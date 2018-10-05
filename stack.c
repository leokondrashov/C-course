#include "stack.h"

#define START_SIZE 10

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

int main() {
	stack s = {};
	stackCtor(&s);
	ASSERT_OK(&s);
	for (int i = 0; i < 20; i++) {
		stackPush(&s, i);
		printf("%d ", i);
	}
	printf("\n");
	
	for (int i = 0; i < 20; i++) {
		printf("%g ", stackPop(&s));
	}
	printf("\n");
	
	stackDtor(&s);
}

void stackCtor(stack *s) {
	assert(s);
	
	s->errno = 0;
	s->capacity = 0;
	s->data = NULL;
	stackSetCapacity(s, START_SIZE);
	s->size = 0;
}

void stackDtor(stack *s) {
	assert(s);
	
	memset(s->data, 0, s->capacity * sizeof(data_t));
	free(s->data);
	s->data = NULL;
	s->size = 0;
	s->capacity = 0;
	s->errno = 0;
}

void stackSetCapacity(stack *s, int c) {
	assert(s);
	assert(c);
	
	if (s->capacity == 0) {
		if ((s->data = (data_t *)calloc(c, sizeof(data_t))) == NULL) {
			s->errno = 1;
		} else {
			s->capacity = c;
		}
		return;
	}
	
	data_t *tmp = realloc(s->data, c * sizeof(data_t));
	if (tmp == NULL) {
		s->errno = 2;
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
		assert(stackOk(s));
	}
	
	s->data[s->size++] = val;
	
	return s->errno;
}

data_t stackPop(stack *s) {
	assert(s);
	ASSERT_OK(s);
	
	if (s->size == 0) {
		s->errno = 3;
		return 0;
	}
	
	data_t val = s->data[--(s->size)];
	s->data[s->size] = 0;
	if (s->size < s->capacity / 4) {
		stackSetCapacity(s, s->capacity / 2);
	}
	assert(stackOk(s));
	
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