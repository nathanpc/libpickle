/**
 * libpickle Test Suite
 * A simple test application to ensure our libpickle library is working.
 *
 * @author Nathan Campos <nathan@innoveworkshop.com>
 */

#include <stdlib.h>
#include <stdio.h>

#include "../src/pickle.h"

int main(int argc, char **argv) {
	pickle_err_t err;
	pickle_doc_t *doc;

	/* Quick argument check. */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s pickledoc\n", argv[0]);
		return 1;
	}

	/* Initialize a new document object. */
	doc = pickle_doc_new();

	/* Open a PickLE document. */
	err = pickle_doc_fopen(doc, argv[1], "w+");
	IF_PICKLE_ERROR(err) {
		pickle_error_print();
		return err;
	}

	/* Close everything up. */
	err = pickle_doc_free(doc);
	IF_PICKLE_ERROR(err) {
		pickle_error_print();
		return err;
	}

	return 0;
}