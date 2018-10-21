#include <stack.h>
#include <stdio.h>
#include <unittest.h>

#ifndef NDEBUG

int main() {
	stack s = {};
	stackCtor(&s);
	
	for (int i = 0; i < total; i++) {
		stackPush(&s, i);
	}
	for (int i = 0; i < total; i++, testN++) {
		UNITTEST(stackPop(&s), total - i - 1);
	}
	printf("\n");
	
	passed = testN = 0;
	for (int i = 0; i < total / 2; i++, testN++) {
		stackPush(&s, i);
		UNITTEST(stackCheckHash(&s), 1);
	}
	for (int i = total / 2; i < total; i++, testN++) {
		stackPop(&s);
		UNITTEST(stackCheckHash(&s), 1);
	}
	printf("\n");
	
	stackDtor(&s);
}

#endif