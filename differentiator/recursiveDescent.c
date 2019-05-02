#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <tree.h>
#include "DSL.h"
#include "recursiveDescent.h"

#define sassert(expr) \
if (!(expr)) { \
    printf("unknown symbol %c\n", *s); \
    Error = 1; \
}

#define BUFF_SIZE 100

int Error = 0;
char *s = NULL;

//int main() {
//	char *buff = (char *) calloc(BUFF_SIZE + 1, sizeof(char));
//	fgets(buff, BUFF_SIZE, stdin);
//	int len = strlen(buff);
//	if (buff[len - 1] == '\n')
//		buff[len - 1] = '\0';
//	tree t = {};
//	t_node *root = getG(buff);
//	t.root = root;
//	if (error == 0)
//		treeDump(&t);
//	mFreeNodes(root);
//	free(buff);
//}

t_node *getG(char *str) {
	assert(str);
	
	s = str;
	t_node *val = getE();
	sassert(*s == '\0');
	s++;
	if (Error) {
		freeNodes(val);
		val = NULL;
	}
	return val;
}

t_node *getE() {
	t_node *val = getT();
	
	while (*s == '+' || *s == '-') {
		int op = *s;
		s++;
		t_node *opNode = createNode(OP, op, val, getT());
		
		val = opNode;
	}
	
	return val;
}

t_node *getT() {
	t_node *val = getP();
	
	while (*s == '*' || *s == '/') {
		int op = *s;
		s++;
		t_node *opNode = createNode(OP, op, val, getP());
		
		val = opNode;
	}
	
	return val;
}

t_node *getP() {
	t_node *val = NULL;
	
	if (*s == '(') {
		s++;
		val = getE();
		sassert(*s == ')');
		s++;
	} else if (strncasecmp("sin(", s, 4) == 0) {
		s += 4;
		val = SIN(getE());
		sassert(*s == ')');
		s++;
	} else if (strncasecmp("cos(", s, 4) == 0) {
		s += 4;
		val = COS(getE());
		sassert(*s == ')');
		s++;
	} else if (strncasecmp("sqrt(", s, 5) == 0) {
		s += 5;
		val = SQRT(getE());
		sassert(*s == ')');
		s++;
	} else if (strncasecmp("ln(", s, 3) == 0) {
		s += 3;
		val = LN(getE());
		sassert(*s == ')');
		s++;
	} else if (*s == '.' || (*s < '9' && *s > '0')) {
		val = getN();
	} else {
		val = getV();
	}
	
	return val;
}

t_node *getN() {
	double val = 0;
	int nextPos = 0;
	
	sassert(sscanf(s, "%lg%n", &val, &nextPos) != 0);
	
	s += nextPos;
	t_node *node = CONST(val);
	
	return node;
}

t_node *getV() {
	sassert(*s == 'x');
	s++;
	
	return VAR('x');
}