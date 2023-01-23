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
#else
	#define EMSG(msg) msg
#endif /* DEBUG */

/* Private variables. */
char *pickle_error_msg_buf = NULL;

/* Private methods. */
size_t pickle_util_strstrcpy(char **dest, const char *start, const char *end);
pickle_err_t pickle_parser_enclstr(const char *delim, const char *buf, const char **start, const char **end);
void pickle_error_free(void);
void pickle_error_msg_set(const char *msg);
void pickle_error_msg_format(const char *format, ...);

/**
 * Initializes a brand new PickLE document object.
 *
 * @return A brand new PickLE document object.
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

	/* Start by closing the file handle. */
	err = pickle_doc_fclose(doc);
	IF_PICKLE_ERROR(err) {
		return err;
	}

	/* TODO: Free the properties, categories, and components. */

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
	pickle_err_t err;
	pickle_property_t *prop;

	/* Check if the file has been opened. */
	if (doc->fh == NULL) {
		pickle_error_msg_set(EMSG(
			"Can't parse a document that hasn't been "
			"opened yet."));
		return PICKLE_ERROR_FILE;
	}

	/* Start by parsing the document's properties. */
	prop = NULL;
	do {
		/* Try to parse a property. */
		err = pickle_parse_property(doc, &prop);
		IF_PICKLE_ERROR(err) {
			return err;
		}
	} while (err != PICKLE_FINISHED_PARSING);

	/* TODO: Parse categories interlaced with components. */

	return PICKLE_OK;
}

/**
 * Parses a property header item at the beginning of the document.
 *
 * @warning prop will be allocated by this function and must be free'd by you.
 *
 * @param doc  PickLE document object.
 * @param prop Property object to be populated by this function. Will be set to
 *             NULL if there isn't a valid property to parse.
 *
 * @return PICKLE_OK if a property was parsed. PICKLE_PARSED_BLANK if a blank
 *         line was found. PICKLE_FINISHED_PARSING when there are no more
 *         properties to be parsed. PICKLE_ERROR_PARSING if the line we tried to
 *         parse was malformed.
 *
 * @see pickle_parse_document
 */
pickle_err_t pickle_parse_property(pickle_doc_t *doc, pickle_property_t **prop) {
	char *buf;
	pickle_err_t err;

	/* Allocate the brand new property. */
	*prop = (pickle_property_t *)malloc(sizeof(pickle_property_t));

	return PICKLE_ERROR_NOT_IMPL;
}

/**
 * Parses a category line in the document.
 *
 * @warning cat will be allocated by this function and must be free'd by you.
 *
 * @param doc PickLE document object.
 * @param cat Category object to be populated by this function. Will be set to
 *            NULL if there isn't a valid category to parse.
 *
 * @return PICKLE_OK if a category was parsed. PICKLE_PARSED_BLANK if a blank
 *         line was found. PICKLE_ERROR_PARSING if the line we tried to
 *         parse was malformed.
 *
 * @see pickle_parse_document
 */
pickle_err_t pickle_parse_category(pickle_doc_t *doc, pickle_category_t **cat) {
	return PICKLE_ERROR_NOT_IMPL;
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
 * @see pickle_parse_document
 */
pickle_err_t pickle_parse_component(pickle_doc_t *doc, pickle_component_t **comp) {
	return PICKLE_ERROR_NOT_IMPL;
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
	va_start(args, format);
	len = vsnprintf(NULL, 0, format, args);
	va_end(args);
	pickle_error_msg_buf = (char *)realloc(pickle_error_msg_buf,
										   (len + 1) * sizeof(char));

	/* Copy our formatted message. */
	va_start(args, format);
	vsprintf(pickle_error_msg_buf, format, args);
	va_end(args);
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
 * Copies one string to another using a start and end portions of an original
 * string. Basically strcpy that accepts substrings. It will always NULL
 * terminate the destination.
 *
 * @warning This function will allocate memory for dest. Make sure you free this
 *          string later.
 *
 * @param dest  Destination string. (Will be allocated by this function.)
 * @param start Start of the string to be copied.
 * @param end   Where should we stop copying the string?
 *
 * @return Number of bytes copied.
 */
size_t pickle_util_strstrcpy(char **dest, const char *start, const char *end) {
	size_t len;
	char *dest_buf;
	const char *src_buf;

	/* Allocate space for the new string. */
	len = end - start + 1;
	*dest = (char *)malloc(len * sizeof(char));

	/* Copy the new string over. */
	dest_buf = *dest;
	src_buf = start;
	len = 0;
	while (src_buf != end) {
		*dest_buf = *src_buf++;
		dest_buf++;
		len++;
	}
	*dest_buf = '\0';

	return len;
}
