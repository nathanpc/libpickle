/**
 * pickle.h
 * A library for parsing and easily working with PickLE pick list documents.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#ifndef _PICKLE_H
#define _PICKLE_H

#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Utility macros. */
#define IF_PICKLE_ERROR(err) if ((err) > PICKLE_OK)

/* Ensure that we have ssize_t defined in non-POSIX-compliant systems. */
#ifndef ssize_t
	#if SIZE_MAX == UINT_MAX
		typedef int ssize_t; /* General 32-bit case. */
		#define SSIZE_MIN INT_MIN
		#define SSIZE_MAX INT_MAX
	#elif SIZE_MAX == ULONG_MAX
		typedef long ssize_t; /* Linux 64 bits */
		#define SSIZE_MIN LONG_MIN
		#define SSIZE_MAX LONG_MAX
	#elif SIZE_MAX == ULLONG_MAX
		typedef long long ssize_t;  /* Windows 64 bits. */
		#define SSIZE_MIN LLONG_MIN
		#define SSIZE_MAX LLONG_MAX
	#elif SIZE_MAX == USHRT_MAX
		typedef short ssize_t;  /* Is this even possible? */
		#define SSIZE_MIN SHRT_MIN
		#define SSIZE_MAX SHRT_MAX
	#elif SIZE_MAX == UINTMAX_MAX
		typedef intmax_t ssize_t;  /* Last resort. */
		#define SSIZE_MIN INTMAX_MIN
		#define SSIZE_MAX INTMAX_MAX
	#else
		#error platform has exotic SIZE_MAX
	#endif
#endif /* ssize_t */

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
	size_t length;
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
	char *fname;
	FILE *fh;
	char fmode[3];

	pickle_property_t **properties;
	size_t len_properties;

	pickle_category_t **categories;
	size_t len_categories;

	pickle_component_t **components;
	size_t len_components;
} pickle_doc_t;

/* PickLE document operations. */
pickle_doc_t *pickle_doc_new(void);
pickle_err_t pickle_doc_fopen(pickle_doc_t *doc, const char *fname, const char *fmode);
pickle_err_t pickle_doc_fclose(pickle_doc_t *doc);
pickle_err_t pickle_doc_free(pickle_doc_t *doc);
pickle_err_t pickle_doc_parse(pickle_doc_t *doc);
pickle_err_t pickle_doc_getline(pickle_doc_t *doc, char **line);
pickle_err_t pickle_doc_property_add(pickle_doc_t *doc, pickle_property_t *prop);
pickle_err_t pickle_doc_category_add(pickle_doc_t *doc, pickle_category_t *cat);

/* PickLE parsing operations. */
pickle_err_t pickle_parse_component(pickle_doc_t *doc, pickle_component_t **comp);

/* PickLE property operations. */
pickle_property_t *pickle_property_new(void);
const char *pickle_property_name_get(const pickle_property_t *prop);
void pickle_property_name_set(pickle_property_t *prop, const char *name);
const char *pickle_property_value_get(const pickle_property_t *prop);
void pickle_property_value_set(pickle_property_t *prop, const char *value);
pickle_err_t pickle_property_free(pickle_property_t *prop);
pickle_err_t pickle_property_parse(const char *line, pickle_property_t **prop);

/* PickLE category operations. */
pickle_category_t *pickle_category_new(void);
const char *pickle_category_name_get(const pickle_category_t *cat);
void pickle_category_name_set(pickle_category_t *cat, const char *name);
pickle_err_t pickle_category_free(pickle_category_t *cat);
pickle_err_t pickle_category_parse(const char *line, pickle_category_t **cat);

/* Error handling. */
const char *pickle_error_msg(void);
void pickle_error_print(void);

#ifdef __cplusplus
}
#endif

#endif /* _PICKLE_H */
