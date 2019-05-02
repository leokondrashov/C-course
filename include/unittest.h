#include <stdio.h>

#ifndef NDEBUG

int passed = 0, total = 10, testN = 0;

#define UNITTEST(what, ref) { \
	if ((what) != (ref)) \
		printf("\rFAILED: test %d: %s = %d, expected %d\n", testN+1, #what, (what), (ref)); \
	else { \
		passed++; \
		printf("\r[%.*s%.*s]", passed, "||||||||||||", total - passed, "------------"); \
	} \
}

#endif