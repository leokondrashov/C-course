#include "stack.h"

#define START_SIZE 10

#ifndef NDEBUG

#define stackDump(s) { \
	printf("stack \"%s\" [%p]:\n", #s, &(s)); \
	printf("\tcapacity = %d\n", (s).capacity); \
	printf("\tsize = %d\n", (s).size); \
	printf("\terrno = %d\n", (s).errno); \
	printf("\tdata[%d]:[%p]\n", (s).size, (s).data); \
	for (int i = 0; i < (s).size; i++) \
		printf("\t\t[%d] = %g\n", i, (s).data[i]); \
	printf("\n"); \
}

#define ASSERT_OK(s) \
	if (!stackOk(s)) { \
		stackDump(*(s)); \
		assert(0); \
	}

#else

#define stackDump(s) ;
#define ASSERT_OK(s) ;

#endif
/*
int main() {
	stack s = {};
	stackCtor(&s);
	ASSERT_OK(&s);
	for (int i = 0; i < 20; i++) {
		stackPush(&s, i);
		printf("%d ", i);
	}
	printf("\n");
	
	stackDump(s);
	
	for (int i = 0; i < 20; i++) {
		printf("%g ", stackPop(&s));
	}
	printf("\n");
	
	stackDump(s);
	
	stackDtor(&s);
}
*/

void stackCtor(stack *s) {
	assert(s);
	
	s->errno = NO_ERROR;
	s->capacity = 0;
	s->data = NULL;
	stackSetCapacity(s, START_SIZE);
	s->size = 0;
	s->hash = 0;
}

void stackDtor(stack *s) {
	assert(s);
	
	memset(s->data, 0, s->capacity * sizeof(data_t));
	free(s->data);
	s->data = NULL;
	s->size = 0;
	s->capacity = 0;
	s->errno = NO_ERROR;
	s->hash = 0;
}

void stackSetCapacity(stack *s, int c) {
	assert(s);
	assert(c);
	
	if (s->capacity == 0) {
		if ((s->data = (data_t *)calloc(c, sizeof(data_t))) == NULL) {
			s->errno = ALLOCATION_ERROR;
		} else {
			s->capacity = c;
		}
		return;
	}
	
	data_t *tmp = realloc(s->data, c * sizeof(data_t));
	if (tmp == NULL) {
		s->errno = REALLOCATION_ERROR;
	} else {
		s->capacity = c;
		s->data = tmp;
	}
}

int stackPush(stack *s, data_t val) {
	assert(s);
	ASSERT_OK(s);
	
	if (s->size == s->capacity) {
		stackSetCapacity(s, s->capacity * 2);
		ASSERT_OK(s);
	}
	
	s->data[s->size++] = val;
	int x = *((int *) &val);
	s->hash += x * s->size;
	
	return s->errno;
}

data_t stackPop(stack *s) {
	assert(s);
	ASSERT_OK(s);
	
	if (s->size == 0) {
		s->errno = STACK_UNDERFLOW;
		return 0;
	}
	
	data_t val = s->data[--(s->size)];
	s->data[s->size] = 0;
	if (s->size < s->capacity / 4) {
		stackSetCapacity(s, s->capacity / 2);
	}
	ASSERT_OK(s);
	
	int x = *((int *) &val);
	s->hash -= x * (s->size + 1);
	
	return val;
}

void stackClear(stack *s) {
	assert(s);
	
	s->size = 0;
	memset(s->data, 0, s->capacity * sizeof(data_t));
	stackSetCapacity(s, START_SIZE);
}

unsigned int stackSize(stack *s) {
	assert(s);
	
	return s->size;
}

unsigned int stackCapacity(stack *s) {
	assert(s);
	
	return s->capacity;
}

int stackErrno(stack *s) {
	assert(s);
	
	return s->errno;
}

int stackOk(stack *s) {
	assert(s);
	
	if (s->errno != 0) return 0;
	
	return s->data && s->capacity && (s->size <= s->capacity);
}

int stackCheckHash(stack *s) {
	int realHash = 0;
	for (int i = 0; i < s->size; i++) {
		realHash += (i + 1) * *((int *)(s->data + i));
	}
	if (s->hash == realHash) {
		return 1;
	} else {
		return 0;
	}
}