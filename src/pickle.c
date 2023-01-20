/**
 * pickle.c
 * A library for parsing and easily working with PickLE pick list documents.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include "pickle.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Opens an existing or brand new PickLE document file for parsing/saving.
 *
 * @param doc   Pointer to a PickLE document object.
 * @param fname Document file path.
 *
 * @return PICKLE_OK if the operation was successful. PICKLE_ERROR_FILE if an
 *         error occurred while trying to open the file.
 */
pickle_err_t pickle_open(pickle_doc_t *doc, const char *fname) {
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
