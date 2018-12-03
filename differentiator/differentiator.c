#include <tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <textProcessor.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "recursiveDescent.h"
#include "DSL.h"

#define BUFF_SIZE 100

char **derivativeLines = NULL;
char **simplificationLines = NULL;
int derLinesCount = 0;
int simplLineCount = 0;

enum presence {
	NULL_PRESENCE = -1,
	UNARY_PRESENCE,
	PLUS_PRESENCE,
	SUB_PRESENCE,
	MUL_PRESENCE,
};

void initLatex(const char *file);
void closeLatex();
void printLatex(const t_node *node, char parentOperation);
void printDerivative(const t_node *source, const t_node *derivative);
t_node *differentiate(const t_node *node);
t_node *copy(const t_node *node);
int presence(char operation);
int simplify(const t_node *root, t_node *node);

int main(int argc, char *argv[]) {
	t_node *root = createNode(0, 0, NULL, NULL);
	char *buff = NULL;
	
	if (argc == 1) {
		buff = (char *) calloc(BUFF_SIZE + 1, sizeof(char));
		fgets(buff, BUFF_SIZE, stdin);
	} else {
		int fileSize = sizeofFile(argv[1]);
		if (fileSize < 0) {
			printf("Error reading file\n");
			return 1;
		}
		buff = (char *) calloc(fileSize + 1, sizeof(char));
		FILE *in = fopen(argv[1], "rb");
		fread(buff, sizeof(char), fileSize, in);
		fclose(in);
	}
	
	root = getG(buff);
	if (root == NULL) {
		printf("Error reading input");
		return 1;
	}
	
	initLatex("a.tex");
	t_node *derivative = differentiate(root);
	simplify(derivative, derivative);
//	tree t = {};
//	t.root = derivative;
//	treeDump(&t);
//	printLatex(derivative, "\0");
//	treeSaveToFile(&t, "derivative.txt");
	closeLatex();
	free(buff);
	freeNodes(root);
	freeNodes(derivative);
}

void initLatex(const char *file) {
	assert(file);
	
	freopen(file, "wb", stdout);
	
	printf("\\documentclass{article}\n"
		   "\\usepackage[utf8x]{inputenc}\n"
		   "\\usepackage[russian]{babel}\n"
		   "\\usepackage[T2A] {fontenc}\n"
		   "\\usepackage{letterspace}\n"
		   "\\begin{document}\n"
		   "\\oddsidemargin=-0.5in\n");
	derivativeLines = readTextFromFile("derivative.txt");
	derLinesCount = countLines(derivativeLines);
	simplificationLines = readTextFromFile("simplify.txt");
	simplLineCount = countLines(simplificationLines);
	srand(time(NULL));
}

void closeLatex() {
	printf("\\end{document}\n");
	
	fclose(stdout);
	
	free(derivativeLines[0]);
	free(derivativeLines);
	free(simplificationLines[0]);
	free(simplificationLines);
}

void printDerivative(const t_node *source, const t_node *derivative) {
	if (source == NULL || derivative == NULL)
		return;
	
	printf("%s\n", derivativeLines[rand() % derLinesCount]);
	printf("\\[\\left(");
	printLatex(source, '\0');
	printf("\\right)\\prime = ");
	printLatex(derivative, '\0');
	printf("\\]\n\n");
}

void printSimplified(const t_node *node) {
	if (node == NULL)
		return;
	
	printf("%s\n", simplificationLines[rand() % simplLineCount]);
	printf("\\[");
	printLatex(node, '\0');
	printf("\\]\n\n");
}

void printLatex(const t_node *node, char parentOperation) {
	if (node == NULL)
		return;
	
	int brackets;
	
	switch (node->val->type) {
	case CONST:
		brackets = (node->val->val < 0) && (presence(parentOperation) != UNARY_PRESENCE);
		printf("%s%g%s", brackets ? "(" : "", node->val->val, brackets ? ")" : "");
		return;
	case VAR:
		printf("%c", (char) node->val->val);
		return;
	case OP:
	default:
		switch ((char) node->val->val) {
		case '+':
		case '-':
			brackets = (presence((char) node->val->val) < presence(parentOperation)) && parentOperation != '/';
			printf("%s", brackets ? "\\left(" : "");
			printLatex(node->left, (char) node->val->val);
			printf("%c", (char) node->val->val);
			printLatex(node->right, (char) node->val->val);
			printf("%s", brackets ? "\\right)" : "");
			break;
		case '*':
			brackets = (presence((char) node->val->val) < presence(parentOperation)) && parentOperation != '/';
			printf("%s", brackets ? "\\left(" : "");
			printLatex(node->left, (char) node->val->val);
			printf("\\cdot ");
			printLatex(node->right, (char) node->val->val);
			printf("%s", brackets ? "\\right)" : "");
			break;
		case '/':
			brackets = (presence((char) node->val->val) < presence(parentOperation));
			printf("%s", brackets ? "\\left(" : "");
			printf("\\frac{");
			printLatex(node->left, '/');
			printf("}{");
			printLatex(node->right, '/');
			printf("}");
			printf("%s", brackets ? "\\right)" : "");
			break;
		case 's':
			printf("\\sin\\left(");
			printLatex(node->right, 's');
			printf("\\right)");
			break;
		case 'c':
			printf("\\cos\\left(");
			printLatex(node->right, 'c');
			printf("\\right)");
			break;
		case 'S':
			printf("\\sqrt{");
			printLatex(node->right, 'S');
			printf("}");
			break;
		case 'e':
			printf("e^{");
			printLatex(node->right, 'e');
			printf("}");
			break;
		case 'l':
			printf("\\ln\\left(");
			printLatex(node->right, 'l');
			printf("\\right)");
			break;
		}
	}
}

int presence(char operation) {
	switch (operation) {
	case '\0':
		return 0;
	case '+':
		return PLUS_PRESENCE;
	case '-':
		return SUB_PRESENCE;
	case '*':
	case '/':
		return MUL_PRESENCE;
	default:
		return UNARY_PRESENCE;
	}
}

t_node *differentiate(const t_node *node) {
	assert(node);
	
	switch (node->val->type) {
	case CONST:
		return CONST(0);
	case VAR:
		return CONST(1);
	case OP:
		#define DEF_OP(name, code, diff) \
        if (code == (char) node->val->val) { \
            t_node *derivative = diff; \
            printDerivative(node, derivative); \
            return derivative; \
        }
		#include "operations.h"
		#undef DEF_OP
	default:
		return NULL;
	}
}

t_node *copy(const t_node *node) {
	if (node == NULL)
		return NULL;
	
	return createNode(node->val->type, node->val->val, cL, cR);
}

int simplify(const t_node *root, t_node *node) {
	if (node == NULL)
		return 0;
	
	if (node->val->type == CONST || node->val->type == VAR)
		return 0;
	
	int flag = 0;
	
	flag |= simplify(root, L);
	flag |= simplify(root, R);
	
	if (IS_CONST(node)) {
		flag = 1;
		double res = 0;
		
		switch ((char) node->val->val) {
		case '+':
			res = L->val->val + R->val->val;
			break;
		case '-':
			res = L->val->val - R->val->val;
			break;
		case '*':
			res = L->val->val * R->val->val;
			break;
		case '/':
			res = L->val->val / R->val->val;
			break;
		case 's':
			res = sin(R->val->val);
			break;
		case 'c':
			res = cos(R->val->val);
			break;
		case 'S':
			res = sqrt(R->val->val);
			break;
		case 'e':
			res = exp(R->val->val);
			break;
		case 'l':
			res = log(R->val->val);
			break;
		default:
			res = 0;
		}
		
		freeNodes(L);
		freeNodes(R);
		L = NULL;
		R = NULL;
		
		node->val->type = CONST;
		node->val->val = res;
		printSimplified(root);
		return 1;
	}
	
	if (((IS_OP(node, '*')) && (IS_CONST_VAL(R, 0) || IS_CONST_VAL(L, 0)))
			|| (IS_OP(node, '/') && IS_CONST_VAL(L, 0))) {
		node->val->type = CONST;
		node->val->val = 0;
		
		freeNodes(L);
		freeNodes(R);
		L = NULL;
		R = NULL;
		
		printSimplified(root);
		return 1;
	}
	
	if ((IS_OP(node, '+') && IS_CONST_VAL(L, 0)) || (IS_OP(node, '*') && IS_CONST_VAL(L, 1))) {
		t_node *tmp = R;
		node->val->type = R->val->type;
		node->val->val = R->val->val;
		
		freeNodes(L);
		L = R->left;
		R = R->right;
		tmp->left = NULL;
		tmp->right = NULL;
		freeNodes(tmp);
		
		printSimplified(root);
		return 1;
	}
	
	if ((IS_OP(node, '+') && IS_CONST_VAL(R, 0)) || (IS_OP(node, '*') && IS_CONST_VAL(R, 1)) ||
			(IS_OP(node, '-') && IS_CONST_VAL(R, 0)) || (IS_OP(node, '/') && IS_CONST_VAL(R, 1))) {
		t_node *tmp = L;
		node->val->type = L->val->type;
		node->val->val = L->val->val;
		
		freeNodes(R);
		L = L->left;
		R = L->right;
		tmp->left = NULL;
		tmp->right = NULL;
		freeNodes(tmp);
		
		printSimplified(root);
		return 1;
	}
	
	if (IS_OP(node, '-') && IS_CONST_VAL(L, 0)) {
		node->val->val = (double) '*';
		L->val->val = -1;
		
		return flag;
	}
	
	return flag;
}