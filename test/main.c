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
	pickle_test("main.c");
	return 0;
}