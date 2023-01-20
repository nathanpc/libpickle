/**
 * pickle.h
 * A library for parsing and easily working with PickLE pick list documents.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _PICKLE_H
#define _PICKLE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/* PickLE parser status codes. */
typedef enum {
	PICKLE_FINISHED_PARSING = -2,
	PICKLE_PARSED_BLANK,
	PICKLE_OK,
	PICKLE_ERROR_FILE,
	PICKLE_ERROR_PARSING,
	PICKLE_ERROR_UNKNOWN,
	PICKLE_ERROR_NOT_IMPL
} pickle_err_t;

/* Reference designator list. */
typedef struct {
	int length;
	char **refdes;
} refdes_list_t;

/* PickLE category object. */
typedef struct {
	char *name;
} pickle_category_t;

/* PickLE component object. */
typedef struct {
	bool picked;
	char *name;
	char *value;
	char *description;
	char *package;
	refdes_list_t refdes;

	pickle_category_t *category;
} pickle_component_t;

/* PickLE property item object. */
typedef struct {
	char *name;
	char *value;
} pickle_property_t;

/* PickLE document handle. */
typedef struct {
	const char *fname;
	FILE *fh;

	pickle_property_t **properties;
	int len_properties;

	pickle_category_t **categories;
	int len_categories;

	pickle_component_t **components;
	int len_components;
} pickle_doc_t;

/* PickLE document operations. */
pickle_err_t pickle_open(pickle_doc_t *doc, const char *fname);
pickle_err_t pickle_close(pickle_doc_t *doc);

/* PickLE parsing operations. */
pickle_err_t pickle_parse_property(pickle_doc_t *doc, pickle_property_t *prop);
pickle_err_t pickle_parse_category(pickle_doc_t *doc, pickle_category_t *cat);
pickle_err_t pickle_parse_component(pickle_doc_t *doc, pickle_component_t *comp);
pickle_err_t pickle_parse_document(pickle_doc_t *doc);

#endif /* _PICKLE_H */
