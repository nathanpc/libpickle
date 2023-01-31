/**
 * pickle.c
 * A library for parsing and easily working with PickLE pick list documents.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "pickle.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Decorate the error message with more information. */
#ifdef DEBUG
	#define STRINGIZE(x) STRINGIZE_WRAPPER(x)
	#define STRINGIZE_WRAPPER(x) #x
	#define EMSG(msg) msg " [" __FILE__ ":" STRINGIZE(__LINE__) "]"
	#define DEBUG_LOG(msg) \
		printf("[DEBUG] \"%s\" [" __FILE__ ":" STRINGIZE(__LINE__) "]\n", (msg))
#else
	#define EMSG(msg) msg
	#define DEBUG_LOG(msg) (void)0
#endif /* DEBUG */

/* Work with compilers that don't provide an vsnprintf implementation. */
#ifndef vsnprintf
	#define VSNPRINTF_MAX_LEN 1024 /* Horrible, I know... */
#endif /* vsnprintf */

/* Private definitions. */
#define LINEBUF_MAX_LEN  1024
#define VALID_WHITESPACE " \t"

/* Private variables. */
static char *pickle_error_msg_buf = NULL;

/* Private methods. */
bool pickle_util_iswtspc(const char *buf);
size_t pickle_util_strcpy(char **dest, const char *src);
size_t pickle_util_strstrcpy(char **dest, const char *start, const char *end);
int pickle_util_getline(FILE *fh, char **line, size_t *rlen, size_t mlen);
bool pickle_parser_iscat(const char *line);
pickle_err_t pickle_parser_enclstr(const char *delim, const char *buf, const char **start, const char **end);
void pickle_error_free(void);
void pickle_error_msg_set(const char *msg);
void pickle_error_msg_format(const char *format, ...);

/**
 * Allocates a brand new PickLE document object.
 * @warning This function allocates memory that you are responsible for freeing.
 *
 * @return A brand new allocated PickLE document object.
 * @see pickle_doc_free
 */
pickle_doc_t *pickle_doc_new(void) {
	pickle_doc_t *doc;

	/* Allocate our object. */
	doc = (pickle_doc_t *)malloc(sizeof(pickle_doc_t));

	/* Reset everything. */
	doc->fname = NULL;
	doc->fh = NULL;
	memset(doc->fmode, '\0', 3);
	doc->properties = NULL;
	doc->len_properties = 0;
	doc->categories = NULL;
	doc->len_categories = 0;
	doc->components = NULL;
	doc->len_components = 0;

	return doc;
}

/**
 * Opens an existing or brand new PickLE document file for parsing/saving.
 *
 * @param doc   Pointer to a PickLE document object.
 * @param fname Document file path.
 * @param fmode File opening mode string. (see fopen)
 *
 * @return PICKLE_OK if the operation was successful. PICKLE_ERROR_FILE if an
 *         error occurred while trying to open the file.
 */
pickle_err_t pickle_doc_fopen(pickle_doc_t *doc, const char *fname, const char *fmode) {
	/* Check if a document is still opened. */
	if (doc->fh != NULL) {
		pickle_error_msg_set(EMSG("A document is already open. Close it before "
							 "opening another one."));
		return PICKLE_ERROR_FILE;
	}

	/* Allocate space for the filename and copy it over. */
	doc->fname = (char *)realloc(doc->fname,
								 (strlen(fname) + 1) * sizeof(char));
	strcpy(doc->fname, fname);

	/* Set the file opening mode. */
	strncpy(doc->fmode, fmode, 2);

	/* Finally open the file. */
	doc->fh = fopen(fname, fmode);
	if (doc->fh == NULL) {
		pickle_error_msg_format(EMSG("Couldn't open file \"%s\": %s."), fname,
								strerror(errno));
		return PICKLE_ERROR_FILE;
	}

	return PICKLE_OK;
}

/**
 * Closes the file handle for a PickLE document.
 *
 * @param doc PickLE document object to have its file handle closed.
 *
 * @return PICKLE_OK if the operation was successful. PICKLE_ERROR_FILE if an
 *         error occurred while trying to close the file.
 *
 * @see pickle_doc_free
 */
pickle_err_t pickle_doc_fclose(pickle_doc_t *doc) {
	/* Try to close the file handle. */
	if (fclose(doc->fh) != 0) {
		pickle_error_msg_format(EMSG("Couldn't close file \"%s\": %s."),
								doc->fname, strerror(errno));
		return PICKLE_ERROR_FILE;
	}

	/* NULL out the file handle and return. */
	doc->fh = NULL;
	return PICKLE_OK;
}

/**
 * Frees up everything in the document object and closes the file handle. This
 * is what you want to call for a proper clean up.
 *
 * @param doc Document object to be completely cleaned up.
 */
pickle_err_t pickle_doc_free(pickle_doc_t *doc) {
	pickle_err_t err;
	size_t i;

	/* Start by closing the file handle. */
	err = pickle_doc_fclose(doc);
	IF_PICKLE_ERROR(err) {
		return err;
	}

	/* Free file name. */
	free(doc->fname);

	/* Free the properties. */
	for (i = 0; i < doc->len_properties; i++) {
		pickle_property_free(doc->properties[i]);
	}
	doc->properties = NULL;
	doc->len_properties = 0;

	/* TODO: Free the categories and components. */

	return PICKLE_OK;
}

/**
 * Reads a line from the document file.
 *
 * @warning This function automatically allocates memory for the line. You are
 *          responsible for freeing this later.
 *
 * @param doc  Opened PickLE document object.
 * @param line Line that was read from the file. (Allocated by this function)
 *
 * @return PICKLE_OK if we were able to get a line with contents from the file.
 *         PICKLE_PARSED_BLANK if we got an empty or just whitespace line.
 *         PICKLE_ERROR_FILE if there was an error while trying to read the file.
 */
pickle_err_t pickle_doc_getline(pickle_doc_t *doc, char **line) {
	size_t len;
	int ret;

	/* Read our line. */
	ret = pickle_util_getline(doc->fh, line, &len, LINEBUF_MAX_LEN);
	if (ret != 0) {
		/* Check if we've reached the end of the file. */
		if (ret == -2)
			return PICKLE_FINISHED_PARSING;

		/* Set the error message. */
		pickle_error_msg_set(EMSG("An error occurred while reading a line from "
			"the document."));

		/* Ensure that we free any resources. */
		if (*line != NULL) {
			free(*line);
			*line = NULL;
		}

		return PICKLE_ERROR_FILE;
	}

	/* Check if we have an empty line. */
	if (pickle_util_iswtspc(*line)) {
		free(*line);
		*line = NULL;

		return PICKLE_PARSED_BLANK;
	}

	return PICKLE_OK;
}

/**
 * Parses a whole document at once and populates the contents field of the
 * pickle_doc_t structure.
 *
 * @param doc Opened PickLE document object.
 *
 * @return PICKLE_OK if everything was parsed fine. PICKLE_ERROR_PARSING if
 *         something in the document couldn't be parsed.
 */
pickle_err_t pickle_doc_parse(pickle_doc_t *doc) {
	char *line;
	pickle_err_t err;
	pickle_property_t *prop;
	pickle_category_t *cat;
	pickle_component_t *comp;

	/* Check if the file has been opened. */
	if (doc->fh == NULL) {
		pickle_error_msg_set(EMSG(
			"Can't parse a document that hasn't been opened yet."));
		return PICKLE_ERROR_FILE;
	}

	/* Start by parsing the document's properties. */
	line = NULL;
	prop = NULL;
	do {
		/* Get line from document file. */
		err = pickle_doc_getline(doc, &line);
		IF_PICKLE_ERROR(err) {
			if (line != NULL) {
				free(line);
				line = NULL;
			}

			return err;
		}

		/* Check if we only had whitespace. */
		if (err == PICKLE_PARSED_BLANK)
			continue;

		/* Try to parse a property. */
		err = pickle_property_parse(line, &prop);
		IF_PICKLE_ERROR(err) {
			if (line != NULL) {
				free(line);
				line = NULL;
			}

			return err;
		}

		/* Append property to the collection. */
		if (err == PICKLE_OK)
			pickle_doc_property_add(doc, prop);

		/* Free up our resources. */
		if (line != NULL) {
			free(line);
			line = NULL;
		}
	} while (err != PICKLE_FINISHED_PARSING);

	/* Try to parse categories and components. */
	line = NULL;
	prop = NULL;
	cat = NULL;
	comp = NULL;
	do {
		/* TODO: Check if first parsed thing was a category. */

		/* Get line from document file. */
		err = pickle_doc_getline(doc, &line);
		IF_PICKLE_ERROR(err) {
			if (line != NULL) {
				free(line);
				line = NULL;
			}

			return err;
		}

		/* Check if we only had whitespace. */
		if (err == PICKLE_PARSED_BLANK) {
			continue;
		} else if (err == PICKLE_FINISHED_PARSING) {
			/* Have we reached the end of the file? */
			break;
		}

		/* Check if we have a category. */
		if (pickle_parser_iscat(line)) {
			/* Try to parse a category. */
			err = pickle_category_parse(line, &cat);
			IF_PICKLE_ERROR(err) {
				if (line != NULL) {
					free(line);
					line = NULL;
				}

				return err;
			}

			/* Append category to the collection. */
			if (err == PICKLE_OK)
				pickle_doc_category_add(doc, cat);
		}

		/* Free up our resources. */
		if (line != NULL) {
			free(line);
			line = NULL;
		}
	} while ((err <= PICKLE_OK) && (err != PICKLE_FINISHED_PARSING));

	return PICKLE_OK;
}

/**
 * Appends a property to the document object properties collection.
 *
 * @param doc  PickLE document object.
 * @param prop Property to be appended to the properties collection.
 *
 * @return PICKLE_OK if everything went fine.
 */
pickle_err_t pickle_doc_property_add(pickle_doc_t *doc, pickle_property_t *prop) {
	doc->properties = (pickle_property_t **)realloc(
		doc->properties,
		(doc->len_properties + 1) * sizeof(pickle_property_t *));
	doc->properties[doc->len_properties] = prop;
	doc->len_properties++;

	return PICKLE_OK;
}

/**
 * Appends a category to the document object categories collection.
 *
 * @param doc PickLE document object.
 * @param cat Category to be appended to the categories collection.
 *
 * @return PICKLE_OK if everything went fine.
 */
pickle_err_t pickle_doc_category_add(pickle_doc_t *doc, pickle_category_t *cat) {
	doc->categories = (pickle_category_t **)realloc(
		doc->categories,
		(doc->len_categories + 1) * sizeof(pickle_category_t *));
	doc->categories[doc->len_categories] = cat;
	doc->len_categories++;

	return PICKLE_OK;
}

/**
 * Allocates a brand new property object.
 * @warning This function allocates memory that you are responsible for freeing.
 *
 * @return A brand new allocated property object.
 * @see pickle_property_free
 */
pickle_property_t *pickle_property_new(void) {
	/* Allocate the structure. */
	pickle_property_t *prop =
		(pickle_property_t *)malloc(sizeof(pickle_property_t));

	/* Put it in a default state. */
	prop->name = NULL;
	prop->value = NULL;

	return prop;
}

/**
 * Gets the name of a property.
 *
 * @param prop Property to get the name from.
 *
 * @return Name of the property or NULL if one wasn't defined yet.
 */
const char *pickle_property_name_get(const pickle_property_t *prop) {
	return (const char *)prop->name;
}

/**
 * Sets the name of a property.
 *
 * @param prop Property to set the name of.
 * @param name Name to be set.
 */
void pickle_property_name_set(pickle_property_t *prop, const char *name) {
	pickle_util_strcpy(&prop->name, name);
}

/**
 * Gets the value of a property.
 *
 * @param prop Property to get the value from.
 *
 * @return Value of the property or NULL if one wasn't defined yet.
 */
const char *pickle_property_value_get(const pickle_property_t *prop) {
	return (const char *)prop->value;
}

/**
 * Sets the value of a property.
 *
 * @param prop Property to set the value of.
 * @param value Value to be set.
 */
void pickle_property_value_set(pickle_property_t *prop, const char *value) {
	pickle_util_strcpy(&prop->value, value);
}

/**
 * Frees up the resources allocated by a property object.
 *
 * @param prop Property object to be free'd.
 *
 * @return PICKLE_OK if the operation was successful.
 */
pickle_err_t pickle_property_free(pickle_property_t *prop) {
	/* Check if we have anything to do. */
	if (prop == NULL)
		return PICKLE_OK;

	/* Free up any internal allocations first. */
	if (prop->name != NULL)
		free(prop->name);
	if (prop->value != NULL)
		free(prop->value);

	/* Free up our object. */
	free(prop);
	prop = NULL;

	return PICKLE_OK;
}

/**
 * Parses a property line.
 *
 * @warning prop will be allocated by this function and must be free'd by you.
 *
 * @param line Line to be parsed.
 * @param prop Property object to be populated by this function. Will be set to
 *             NULL if there isn't a valid property to parse.
 *
 * @return PICKLE_OK if a property was parsed. PICKLE_PARSED_BLANK if a blank
 *         line was found. PICKLE_FINISHED_PARSING when there are no more
 *         properties to be parsed. PICKLE_ERROR_PARSING if the line we tried to
 *         parse was malformed.
 *
 * @see pickle_doc_parse
 */
pickle_err_t pickle_property_parse(const char *line, pickle_property_t **prop) {
	const char *cur;

	/* Check if we have finished parsing. */
	if (line[0] == '-') {
		if (strcmp(line, "---") == 0)
			return PICKLE_FINISHED_PARSING;

		/* Invalid property name. */
		pickle_error_msg_set(EMSG("A property can't start with a dash."));
		return PICKLE_ERROR_PARSING;
	}

	/* Check if line starts with a colon. */
	if (line[0] == ':') {
		pickle_error_msg_set(EMSG("Property line must not start with a colon."));
		return PICKLE_ERROR_PARSING;
	}

	/* Allocate the brand new property. */
	*prop = pickle_property_new();

	/* Find the first occurrence of a colon. */
	cur = strpbrk(line, ":");
	if (cur == NULL) {
		pickle_error_msg_set(EMSG("Property line does not contain a colon."));
		goto parsing_error;
	}

	/* Copy the property name over. */
	pickle_util_strstrcpy(&((*prop)->name), line, cur - 1);

	/* Move the cursor over to skip the colon and any whitespace. */
	cur += strspn(cur, ":" VALID_WHITESPACE);
	if (*cur == '\0') {
		pickle_error_msg_set(EMSG("Property line does not contain a value."));
		goto parsing_error;
	}

	/* Copy the property value over and return. */
	pickle_property_value_set(*prop, cur);
	return PICKLE_OK;

parsing_error:
	pickle_property_free(*prop);
	return PICKLE_ERROR_PARSING;
}

/**
 * Allocates a brand new category object.
 * @warning This function allocates memory that you are responsible for freeing.
 *
 * @return A brand new allocated category object.
 * @see pickle_category_free
 */
pickle_category_t *pickle_category_new(void) {
	/* Allocate the structure. */
	pickle_category_t *cat =
		(pickle_category_t *)malloc(sizeof(pickle_category_t));

	/* Put it in a default state. */
	cat->name = NULL;

	return cat;
}

/**
 * Gets the name of a category.
 *
 * @param cat Category to get the name from.
 *
 * @return Name of the category or NULL if one wasn't defined yet.
 */
const char *pickle_category_name_get(const pickle_category_t *cat) {
	return (const char *)cat->name;
}

/**
 * Sets the name of a category.
 *
 * @param cat  Category to set the name of.
 * @param name Name to be set.
 */
void pickle_category_name_set(pickle_category_t *cat, const char *name) {
	pickle_util_strcpy(&cat->name, name);
}

/**
 * Frees up the resources allocated by a category object.
 *
 * @param cat Category object to be free'd.
 *
 * @return PICKLE_OK if the operation was successful.
 */
pickle_err_t pickle_category_free(pickle_category_t *cat) {
	/* Check if we have anything to do. */
	if (cat == NULL)
		return PICKLE_OK;

	/* Free up any internal allocations first. */
	if (cat->name != NULL)
		free(cat->name);

	/* Free up our object. */
	free(cat);
	cat = NULL;

	return PICKLE_OK;
}

/**
 * Parses a category line.
 *
 * @warning cat will be allocated by this function and must be free'd by you.
 *
 * @param line Line to be parsed.
 * @param cat  Category object to be populated by this function. Will be set to
 *             NULL if there isn't a valid category to parse.
 *
 * @return PICKLE_OK if a category was parsed. PICKLE_PARSED_BLANK if a blank
 *         line was found. PICKLE_ERROR_PARSING if the line we tried to
 *         parse was malformed.
 *
 * @see pickle_doc_parse
 */
pickle_err_t pickle_category_parse(const char *line, pickle_category_t **cat) {
	const char *cur;

	/* Check if line starts with a colon. */
	if (line[0] == ':') {
		pickle_error_msg_set(EMSG("Category line must not start with a colon."));
		return PICKLE_ERROR_PARSING;
	}

	/* Allocate the brand new category. */
	*cat = pickle_category_new();

	/* Find the first occurrence of a colon. */
	cur = strpbrk(line, ":");
	if (cur == NULL) {
		pickle_error_msg_set(EMSG("Category line does not contain a colon."));
		goto parsing_error;
	}

	/* Copy the category name over and return. */
	pickle_util_strstrcpy(&((*cat)->name), line, cur - 1);
	return PICKLE_OK;

parsing_error:
	pickle_category_free(*cat);
	return PICKLE_ERROR_PARSING;
}

/**
 * Parses a component item in the document.
 *
 * @warning comp will be allocated by this function and must be free'd by you.
 *
 * @param doc  PickLE document object.
 * @param comp Component object to be populated by this function. Will be set to
 *             NULL if there isn't a valid component to parse.
 *
 * @return PICKLE_OK if a component was parsed. PICKLE_PARSED_BLANK if a blank
 *         line was found. PICKLE_FINISHED_PARSING when there are no more
 *         components to be parsed. PICKLE_ERROR_PARSING if the line we tried to
 *         parse was malformed.
 *
 * @see pickle_doc_parse
 */
pickle_err_t pickle_parse_component(pickle_doc_t *doc, pickle_component_t **comp) {
	return PICKLE_ERROR_NOT_IMPL;
}

/**
 * Checks if a line is a category definition.
 *
 * @param line Line to be checked.
 *
 * @return Is this line a properly-formed category definition?
 */
bool pickle_parser_iscat(const char *line) {
	/* Perform a simple check if the last character in a line is a colon. */
	return line[strlen(line) - 1] == ':';
}

/**
 * Lexer that extracts strings that are enclosed inside a set of tokens (delim).
 *
 * @param delim Tokens that delimit parts of the string.
 * @param str   String to be lexed.
 * @param start Start of a string that was enclosed in one of the tokens.
 * @param end   End of the string that was enclosed in one of the tokens.
 *
 * @return PICKLE_OK if we were able to lex the string. PICKLE_ERROR_PARSING if
 *         no tokens were found in the string. PICKLE_FINISHED_PARSING if the
 *         tokens were found, but nothing was in between them.
 */
pickle_err_t pickle_parser_enclstr(const char *delim, const char *buf, const char **start, const char **end) {
	/* Find the first occurrence of a token. */
	*start = strpbrk(buf, delim);

	/* Check if we've reached the end of the string without finding anything. */
	if (*start[0] == '\0') {
		*start = *end = NULL;
		return PICKLE_ERROR_PARSING;
	}

	/* Skip consecutive tokens. */
	*start += strspn(*start, delim);

	/* Find the end of the string enclosed in the tokens. */
	*end = strpbrk(buf, delim) - 1;

	/* Check if there was nothing in between the tokens. */
	if (*start == *end) {
		*start = *end = NULL;
		return PICKLE_FINISHED_PARSING;
	}

	return PICKLE_OK;
}

/**
 * Sets the error message that the user can recall later.
 *
 * @param msg Error message to be set.
 */
void pickle_error_msg_set(const char *msg) {
	/* Make sure we have enough space to store our error message. */
	pickle_error_msg_buf = (char *)realloc(pickle_error_msg_buf,
										   (strlen(msg) + 1) * sizeof(char));

	/* Copy the error message. */
	strcpy(pickle_error_msg_buf, msg);
}

/**
 * Sets the error message that the user can later recall using a syntax akin to
 * printf.
 *
 * @param format Format string just like in printf.
 * @param ...    Things to place inside the formatted string.
 */
void pickle_error_msg_format(const char *format, ...) {
	size_t len;
	va_list args;

	/* Allocate enough space for our message. */
#ifndef vsnprintf
	len = VSNPRINTF_MAX_LEN;
#else
	va_start(args, format);
	len = vsnprintf(NULL, 0, format, args) + 1;
	va_end(args);
#endif /* vsnprintf */
	pickle_error_msg_buf = (char *)realloc(pickle_error_msg_buf,
										   len * sizeof(char));

	/* Copy our formatted message. */
	va_start(args, format);
	vsprintf(pickle_error_msg_buf, format, args);
	va_end(args);

#ifndef vsnprintf
	/* Ensure that we have a properly sized string. */
	len = strlen(pickle_error_msg_buf) + 1;
	pickle_error_msg_buf = (char *)realloc(pickle_error_msg_buf,
										   len * sizeof(char));
#endif /* vsnprintf */
}

/**
 * Gets the last error message string.
 *
 * @return A read-only pointer to the internal last error message string buffer.
 */
const char *pickle_error_msg(void) {
	return (const char *)pickle_error_msg_buf;
}

/**
 * Prints the last error message thrown by the library to STDERR.
 */
void pickle_error_print(void) {
	fprintf(stderr, "ERROR: %s\n", pickle_error_msg_buf);
}

/**
 * Frees up the internal error message string buffer.
 */
void pickle_error_free(void) {
	/* Do we even have anything to free? */
	if (pickle_error_msg_buf == NULL)
		return;

	/* Free up the error message. */
	free(pickle_error_msg_buf);
	pickle_error_msg_buf = NULL;
}

/**
 * Checks if a string only contains whitespace.
 *
 * @param buf String to be checked.
 *
 * @return Does this string consists only of whitespace?
 */
bool pickle_util_iswtspc(const char *buf) {
	return *(buf + strspn(buf, VALID_WHITESPACE)) == '\0';
}

/**
 * Copies one string to another using a start and end portions of an original
 * string. Basically strcpy that accepts substrings. It will always NULL
 * terminate the destination.
 *
 * @warning This function will allocate memory for dest. Make sure you free this
 *          string later.
 *
 * @param dest  Destination string. (Will be allocated by this function.)
 * @param start Start of the string to be copied.
 * @param end   Where should we stop copying the string? (Inclusive)
 *
 * @return Number of bytes copied.
 */
size_t pickle_util_strstrcpy(char **dest, const char *start, const char *end) {
	size_t len;
	char *dest_buf;
	const char *src_buf;

	/* Allocate space for the new string. */
	len = (end - start) + 1;
	*dest = (char *)malloc(len * sizeof(char));

	/* Copy the new string over. */
	dest_buf = *dest;
	src_buf = start;
	len = 0;
	do {
		*dest_buf = *src_buf;

		src_buf++;
		dest_buf++;
		len++;
	} while (src_buf <= end);
	*dest_buf = '\0';

	return len;
}

/**
 * Similar to strcpy except we allocate (reallocate if needed) the destination
 * string automatically.
 *
 * @warning This function will allocate memory for dest. Make sure you free this
 *          string later.
 *
 * @param dest Destination string. (Will be [re]allocated by this function.)
 * @param src  Source string to be copied.
 *
 * @return Number of bytes copied.
 */
size_t pickle_util_strcpy(char **dest, const char *src) {
	size_t len;
	char *dest_buf;
	const char *src_buf;

	/* Check if we have a valid destination pointer. */
	if (dest == NULL)
		return 0;

	/* Allocate space for the new string. */
	len = strlen(src);
	*dest = (char *)realloc(*dest, (len + 1) * sizeof(char));

	/* Copy the new string over. */
	dest_buf = *dest;
	src_buf = src;
	do {
		*dest_buf = *src_buf;

		src_buf++;
		dest_buf++;
	} while (*src_buf != '\0');

	return len;
}

/**
 * Re-implementation of getline for C89 with some modifications. Reads an entire
 * line from a file, not including the newline separator, also ignores CR
 * characters. Will treat EOF as a pseudo-newline.
 *
 * @warning This function will allocate memory for buf. Make sure you free this
 *          string later.
 *
 * @param fh   File handle to read the line from.
 * @param line Buffer that will store the read line. (ALLOCATED BY THIS FUNCTION)
 * @param rlen Number of bytes copied into the buffer.
 * @param mlen Maximum length of a line. (Used to allocate the internal buffer)
 *
 * @return 0 if the operation was successful. -1 if an error occurred. 1 if mlen
 *         wasn't big enough to hold an entire line. -2 if we've reached EOF.
 */
int pickle_util_getline(FILE *fh, char **line, size_t *rlen, size_t mlen) {
	char *buf;
	char c;

	/* Check if we have a valid pointer to work with. */
	if (line == NULL || rlen == NULL)
		return -1;

	/* Check if our file handle is valid. */
	if ((fh == NULL) || ferror(fh)) {
		*line = NULL;
		*rlen = 0;
		return -1;
	}

	/* Check if we've reached the end of the file. */
	if (feof(fh)) {
		*line = NULL;
		*rlen = 0;
		return -2;
	}

	/* Allocate our line buffer. */
	*line = (char *)malloc(mlen * sizeof(char));
	if (*line == NULL)
		return -1;

	/* Go through the file character by character. */
	*rlen = 0;
	buf = *line;
	while (((c = (char)getc(fh)) != '\n') && (c != EOF)) {
		/* Check if we still have space in our buffer. */
		if (*rlen == (mlen - 1)) {
			free(*line);
			*line = NULL;
			*rlen = 0;

			return 1;
		}

		/* Ignore carriage returns. */
		if (c == '\r')
			continue;

		/* Copy read character over and increase our read counter. */
		*buf = c;
		buf++;
		*rlen += 1;
	}

	/* Properly terminate our buffer. */
	*buf = '\0';

	/* Resize the output string. */
	*line = (char *)realloc(*line, (*rlen + 1) * sizeof(char));
	if (*line == NULL) {
		*rlen = 0;
		return -1;
	}

	DEBUG_LOG(*line);
	return 0;
}
