/**
 * pickle.c
 * A library for parsing and easily working with PickLE pick list documents.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "pickle.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Private variables. */
char *pickle_error_msg_buf = NULL;

/* Private methods. */
void pickle_error_free(void);
void pickle_error_msg_set(const char *msg);
void pickle_error_msg_format(const char *format, ...);

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
pickle_err_t pickle_open(pickle_doc_t *doc, const char *fname, const char *fmode) {
	/* Check if a document is still opened. */
	if (doc->fh != NULL) {
		pickle_error_msg_set("A document is already open. Close it before "
							 "opening another one.");
		return PICKLE_ERROR_FILE;
	}

	return PICKLE_ERROR_NOT_IMPL;
}

/**
 * Closes the file handle for a PickLE document.
 *
 * @param doc PickLE document object to have its file handle closed.
 *
 * @return PICKLE_OK if the operation was successful. PICKLE_ERROR_FILE if an
 *         error occurred while trying to close the file.
 */
pickle_err_t pickle_close(pickle_doc_t *doc) {
	return PICKLE_ERROR_NOT_IMPL;
}

/**
 * Parses a property header item at the beginning of the document.
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
pickle_err_t pickle_parse_property(pickle_doc_t *doc, pickle_property_t *prop) {
	return PICKLE_ERROR_NOT_IMPL;
}

/**
 * Parses a category line in the document.
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
pickle_err_t pickle_parse_category(pickle_doc_t *doc, pickle_category_t *cat) {
	return PICKLE_ERROR_NOT_IMPL;
}

/**
 * Parses a component item in the document.
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
pickle_err_t pickle_parse_component(pickle_doc_t *doc, pickle_component_t *comp) {
	return PICKLE_ERROR_NOT_IMPL;
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
pickle_err_t pickle_parse_document(pickle_doc_t *doc) {
	return PICKLE_ERROR_NOT_IMPL;
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
