/**
 * libpickle Test Suite
 * A simple test application to ensure our libpickle library is working.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include <stdlib.h>
#include <stdio.h>

#include "../src/pickle.h"

/* Private methods. */
void error_cleanup(pickle_doc_t *doc);

int main(int argc, char **argv) {
	pickle_err_t err;
	pickle_doc_t *doc;
	size_t i;

	/* Quick argument check. */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s pickledoc\n", argv[0]);
		return 1;
	}

	printf("libpickle Test Program\n\n");

	/* Initialize a new document object. */
	doc = pickle_doc_new();
	printf("New document object created.\n");

	/* Open a PickLE document. */
	err = pickle_doc_fopen(doc, argv[1], "r");
	IF_PICKLE_ERROR(err) {
		error_cleanup(doc);
		return err;
	}
	printf("PickLE document \"%s\" opened.\n", argv[1]);

	/* Parse the document. */
	err = pickle_doc_parse(doc);
	IF_PICKLE_ERROR(err) {
		error_cleanup(doc);
		return err;
	}
	printf("Document successfully parsed.\n");

	/* Print the properties. */
	printf("Got %lu properties!\n", doc->len_properties);
	for (i = 0; i < doc->len_properties; i++) {
		const pickle_property_t *prop = doc->properties[i];
		printf("\t%s = %s\n", prop->name, prop->value);
	}

	/* Print the categories. */
	printf("Got %lu categories!\n", doc->len_categories);
	for (i = 0; i < doc->len_categories; i++) {
		const pickle_category_t *cat = doc->categories[i];
		printf("\t- %s\n", cat->name);
	}

	/* Close everything up. */
	err = pickle_doc_free(doc);
	IF_PICKLE_ERROR(err) {
		pickle_error_print();
		return err;
	}
	printf("Document closed and free'd.\n");

	return 0;
}

/**
 * A simple helper function to clean things up after an error occurred and
 * inform the user.
 *
 * @param doc PickLE document object.
 */
void error_cleanup(pickle_doc_t *doc) {
	pickle_error_print();
	pickle_doc_free(doc);
}
