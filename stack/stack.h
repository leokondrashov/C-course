#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef STACK_H

#define STACK_H

typedef float data_t;
typedef struct stack stack;

enum error {
	NO_ERROR,
	ALLOCATION_ERROR,
	REALLOCATION_ERROR,
	STACK_UNDERFLOW
};

struct stack {
	data_t *data;
	unsigned int size;
	unsigned int capacity;
	enum error errno;
	int hash;
};

void stackCtor(stack *s);
void stackDtor(stack *s);

void stackSetCapacity(stack *s, int c);

int stackOk(stack *s);

int stackPush(stack *s, data_t val);
data_t stackPop(stack *s);

unsigned int stackSize(stack *s);
unsigned int stackCapacity(stack *s);
int stackErrno(stack *s);

void stackClear(stack *s);

int stackCheckHash(stack *s);

#endif