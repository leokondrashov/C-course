#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef float data_t;
typedef struct stack stack;

struct stack {
	data_t *data;
	unsigned int size;
	unsigned int capacity;
	int errno;
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

void stackDump(stack *s);