#include <stdio.h>
#include <stdlib.h>
#include <tree.h>
#include <textProcessor.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <map.h>

#define sassert(expr) \
if (!(expr)) { \
    printf("unexpected symbol \'%c\' on line %d\n", *S, CurLine + 1); \
    Error = 1; \
}

map functions;

t_node *getProg(const char *file);
int treeToAsm(t_node *root, char *file);

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("missing file\n");
		printf("using format: compiler file [-o output_file]\n");
		return 0;
	}
	
	char *infile = NULL, *outfile = NULL;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'o' && outfile == NULL) {
				outfile = argv[++i];
			} else {
				printf("Unknown flag: %s", argv[i]);
				return 0;
			}
		} else if (infile == NULL) {
			infile = argv[i];
		} else {
			printf("using format: compiler file [-o output_file]\n");
			return 0;
		}
	}
	
	if (infile == NULL) {
		printf("missing file\n");
		printf("using format: asm file [-l listing_file] [-o output_file]\n");
		return 0;
	}
	
	if (outfile == NULL) {
		outfile = "a.out";
	}
	
	mapCtor(&functions);
	
	tree t = {};
	treeCtor(&t);
	t.root = getProg(infile);
	treeDump(&t);
	
	treeToAsm(t.root, outfile);
	
	treeDtor(&t);

//	mapDtor(&functions);
}

char **Lines = NULL;
char *S = NULL;
int CurLine = 0, LinesCount = 0;
int Error = 0;

void incrementPointer() {
	S++;
	
	while (isspace(*S) || (*S == '\0' && CurLine < LinesCount - 1)) {
		S = skipSpaces(S);
		
		if (*S == '\0' && CurLine != LinesCount - 1) {
			CurLine++;
			S = Lines[CurLine];
		}
	}
}

int FunctionCount = 0;

t_node *getG();
t_node *getOPBlock();
t_node *getOP();
t_node *getIf();
t_node *getWhile();
t_node *getAssignment();
t_node *getExpression();
t_node *getComparison();
t_node *getE();
t_node *getT();
t_node *getP();
t_node *getN();
t_node *getId();
t_node *getFunctionCall();
t_node *getArgList();
size_t getWordLength();
int isNextWord(const char *word);
t_node *getFunctionDeclaration();
t_node *getReturn();

t_node *getProg(const char *file) {
	assert(file);
	
	Lines = readTextFromFile(file);
	LinesCount = countLines(Lines);
	
	S = Lines[0] - 1; // done to skip blank lines and space characters
	incrementPointer();
	
	t_node *root = getG();
	
	free(Lines[0]);
	free(Lines);
	
	if (Error) {
		freeNodes(root);
		return NULL;
	}
	
	return root;
}

t_node *getG() {
	t_node *val = NULL;
	while (*S == 'f') {
		t_node *funcNode = getFunctionDeclaration();
		val = createNode(OP, ';', val, funcNode);
	}
	val = createNode(OP, ';', val, createNode(OP, '0', NULL, getOPBlock()));
	sassert(*S == '\0');
	return val;
}

t_node *getFunctionDeclaration() {
	if (isNextWord("function")) {
		S += strlen("function") - 1;
		incrementPointer();
		
		size_t len = getWordLength();
		char savedChar = S[len];
		S[len] = '\0';
		if (mapFind(&functions, S)) {
			printf("redeclaration of function %s on line %d\n", S, CurLine);
			S[len] = savedChar;
			S += len - 1;
			incrementPointer();
			sassert(*S == '(');
			incrementPointer();
			
			sassert(*S == ')');
			incrementPointer();
			
			freeNodes(getOPBlock());
			return NULL;
		}
		
		mapAdd(&functions, S, FunctionCount);
		S[len] = savedChar;
		S += len - 1;
		incrementPointer();
		sassert(*S == '(');
		incrementPointer();
		
		sassert(*S == ')');
		incrementPointer();
		
		return createNode(FUNCTION_DECLARATION, FunctionCount++, NULL,
				createNode(OP, ';', getOPBlock(), createNode(OP, 'r', NULL, NULL)));
	}
	
	return NULL;
}

t_node *getOPBlock() {
	if (*S == '{') {
		incrementPointer();
		
		char *oldS = S;
		t_node *val = getOP();
		while (oldS != S && *S != '}') {
			oldS = S;
			
			t_node *opNode = getOP();
			if (oldS != S)
				val = createNode(OP, ';', val, opNode);
		}
		sassert(*S == '}');
		incrementPointer();
		return val;
	}
	
	return getOP();
}

t_node *getOP() {
	if (isNextWord("if"))
		return getIf();
	
	if (isNextWord("while"))
		return getWhile();
	
	if (isNextWord("return"))
		return getReturn();
	
	t_node *val = getFunctionCall();
	if (val == NULL)
		return getAssignment();
	if (val->val->type == FUNCTION)
		val = createNode(OP, ';', val, createNode(OP, 'p', NULL, NULL)); // to get rid of returning value
	sassert(*S == ';');
	incrementPointer();
	return val;
}

t_node *getReturn() {
	sassert(isNextWord("return"));
	S += 5;
	incrementPointer();
	
	if (*S == ';') {
		incrementPointer();
		return createNode(OP, 'r', NULL, NULL);
	}
	
	t_node *val = createNode(OP, 'r', NULL, getE());
	sassert(*S == ';');
	incrementPointer();
	return val;
}

t_node *getIf() {
	sassert(isNextWord("if"));
	S++;
	incrementPointer();
	
	sassert(*S == '(');
	incrementPointer();
	
	t_node *condition = getExpression();
	
	sassert(*S == ')');
	incrementPointer();
	
	t_node *operation = getOPBlock();
	
	if (isNextWord("else")) {
		S += 3;
		incrementPointer();
		operation = createNode(OP, '|', operation, getOPBlock());
	}
	
	return createNode(OP, 'i', condition, operation);
}

t_node *getWhile() {
	sassert(isNextWord("while"));
	S += 4;
	incrementPointer();
	
	sassert(*S == '(');
	incrementPointer();
	
	t_node *condition = getExpression();
	
	sassert(*S == ')');
	incrementPointer();
	
	t_node *operation = getOPBlock();
	
	return createNode(OP, 'w', condition, operation);
}

t_node *getAssignment() {
	char *oldS = S;
	
	t_node *id = getId();
	if (oldS == S)
		return NULL;
	
	sassert(*S == '=');
	incrementPointer();
	
	t_node *expr = getE();
	
	sassert(*S == ';');
	incrementPointer();
	
	return createNode(OP, '=', id, expr);
}

t_node *getExpression() {
	t_node *left = getE();
	
	t_node *comp = getComparison();
	if (comp == NULL)
		return left;
	
	t_node *right = getE();
	
	comp->left = left;
	comp->right = right;
	
	return comp;
}

t_node *getComparison() {
	switch (*S) {
	case '>':
		if (*(S + 1) == '=') {
			S++;
			incrementPointer();
			return createNode(OP, 'g', NULL, NULL);
		}
		
		incrementPointer();
		return createNode(OP, 'G', NULL, NULL);
	case '<':
		if (*(S + 1) == '=') {
			S++;
			incrementPointer();
			return createNode(OP, 'l', NULL, NULL);
		}
		
		incrementPointer();
		return createNode(OP, 'L', NULL, NULL);
	case '=':
		if (*(S + 1) == '=') {
			S++;
			incrementPointer();
			return createNode(OP, 'e', NULL, NULL);
		}
		return NULL;
	case '!':
		if (*(S + 1) == '=') {
			S++;
			incrementPointer();
			return createNode(OP, 'n', NULL, NULL);
		}
		return NULL;
	
	default:
		return NULL;
	}
}

t_node *getE() {
	t_node *val = getT();
	
	while (*S == '+' || *S == '-') {
		int op = *S;
		incrementPointer();
		t_node *opNode = createNode(OP, op, val, getT());
		
		val = opNode;
	}
	
	return val;
}

t_node *getT() {
	t_node *val = getP();
	
	while (*S == '*' || *S == '/') {
		int op = *S;
		incrementPointer();
		t_node *opNode = createNode(OP, op, val, getP());
		
		val = opNode;
	}
	
	return val;
}

t_node *getP() {
	t_node *val = NULL;
	
	if (*S == '(') {
		incrementPointer();
		val = getE();
		sassert(*S == ')');
		incrementPointer();
		return val;
	}
	
	if (isdigit(*S) || *S == '+' || *S == '-')
		return getN();
	
	val = getFunctionCall();
	if (val != NULL)
		return val;
	
	return getId();
}

t_node *getN() {
	sassert(isdigit(*S) || ((*S == '+' || *S == '-') && isdigit(*(S + 1))));
	
	int constVal = 0, nextPos = 0;
	sscanf(S, "%d%n", &constVal, &nextPos);
	if (nextPos == 0)
		return NULL;
	
	S += nextPos - 1;
	incrementPointer();
	return createNode(CONST, constVal, NULL, NULL);
}

t_node *getId() {
	sassert(*S >= 'a' && *S <= 'o');
	
	if (*S < 'a' || *S > 'o')
		return NULL;
	
	t_node *val = createNode(VAR, *S, NULL, NULL);
	incrementPointer();
	return val;
}

t_node *getFunctionCall() {
	if (isNextWord("in")) {
		S++;
		incrementPointer();
		
		sassert(*S == '(');
		incrementPointer();
		
		sassert(*S == ')');
		incrementPointer();
		
		return createNode(OP, 'I', NULL, NULL);
	} else if (isNextWord("out")) {
		S += 2;
		incrementPointer();
		
		sassert(*S == '(');
		incrementPointer();
		
		t_node *arg = getE();
		
		sassert(*S == ')');
		incrementPointer();
		
		return createNode(OP, 'O', NULL, arg);
	} else if (isNextWord("sqrt")) {
		S += 3;
		incrementPointer();
		
		sassert(*S == '(');
		incrementPointer();
		
		t_node *arg = getE();
		
		sassert(*S == ')');
		incrementPointer();
		
		return createNode(OP, 's', NULL, arg);
	} else if (isNextWord("meow")) {
		S += 3;
		incrementPointer();
		
		sassert(*S == '(');
		incrementPointer();
		
		sassert(*S == ')');
		incrementPointer();
		
		return createNode(OP, 'm', NULL, NULL);
	} else if (isNextWord("dump")) {
		S += 3;
		incrementPointer();
		
		sassert(*S == '(');
		incrementPointer();
		
		sassert(*S == ')');
		incrementPointer();
		
		return createNode(OP, 'd', NULL, NULL);
	} else {
		size_t len = getWordLength();
		if (len == 0)
			return NULL;
		
		char savedChar = S[len];
		S[len] = '\0';
		if (mapFind(&functions, S)) {
			t_node *val = createNode(FUNCTION, mapGet(&functions, S), NULL, NULL);
			S[len] = savedChar;
			S += len - 1;
			incrementPointer();
			
			sassert(*S == '(');
			incrementPointer();
			
			if (*S != ')') {
				t_node *args = getArgList();
				val->right = args;
			}
			
			sassert(*S == ')');
			incrementPointer();
			return val;
		}
		
		S[len] = savedChar;
		return NULL;
	}
}

t_node *getArgList() {
	t_node *args = getE();
	
	while (*S == ',') {
		incrementPointer();
		args = createNode(OP, ',', args, getE());
	}
	
	return args;
}

size_t getWordLength() {
	if (!isalpha(*S) && *S != '_')
		return 0;
	
	size_t len = 0;
	while (isalnum(S[len]) || S[len] == '_')
		len++;
	
	return len;
}

int isNextWord(const char *word) {
	size_t len = getWordLength();
	char savedChar = S[len];
	S[len] = '\0';
	
	int res = strcmp(S, word);
	
	S[len] = savedChar;
	
	return res == 0;
}

int IfCounter = 0, LoopCounter = 0, ArgsCounter = 0;

int nodeToAsm(t_node *node, FILE *out) {
	if (node == NULL)
		return 0;
	
	switch (node->val->type) {
	case CONST:
		fprintf(out, "push %d\n", node->val->val);
		return 1;
	case VAR:
		fprintf(out, "pushr r%d\n", node->val->val - 'a');
		return 1;
	case FUNCTION:
		for (int i = 14; i >= 0; i--) {
			fprintf(out, "pushr r%d\n", i);
		}
		if (node->right != NULL) {
			ArgsCounter = 1;
			nodeToAsm(node->right, out);
			for (int i = 0; i < ArgsCounter; i++) {
				fprintf(out, "popr r%d\n", i > 14 ? 15 : i);
			}
			for (int i = ArgsCounter; i < 15; i++) {
				fprintf(out, "push 0\n"
							 "popr r%d\n", i);
			}
		}
		fprintf(out, "call func%d\n", node->val->val);
		fprintf(out, "popr r15\n");
		for (int i = 0; i < 15; i++) {
			fprintf(out, "popr r%d\n", i);
		}
		fprintf(out, "pushr r15\n");
		return 1;
	case FUNCTION_DECLARATION:
		fprintf(out, "func%d:\n", node->val->val);
		nodeToAsm(node->right, out);
		return 1;
	case OP:
		switch ((char) node->val->val) {
		case ',':
			ArgsCounter++;
			nodeToAsm(node->right, out);
			nodeToAsm(node->left, out);
			return 1;
		case 'p':
			fprintf(out, "pop\n");
			return 1;
		case '0':
			fprintf(out, "main:\n");
			nodeToAsm(node->right, out);
			fprintf(out, "end\n");
			return 1;
		case ';':
		case 'l':
		case 'L':
		case 'g':
		case 'G':
		case 'e':
		case 'n':
			nodeToAsm(node->left, out);
			nodeToAsm(node->right, out);
			return 1;
		
		case '+':
			nodeToAsm(node->left, out);
			nodeToAsm(node->right, out);
			fprintf(out, "add\n");
			return 1;
		case '-':
			nodeToAsm(node->left, out);
			nodeToAsm(node->right, out);
			fprintf(out, "sub\n");
			return 1;
		case '*':
			nodeToAsm(node->left, out);
			nodeToAsm(node->right, out);
			fprintf(out, "mul\n");
			return 1;
		case '/':
			nodeToAsm(node->left, out);
			nodeToAsm(node->right, out);
			fprintf(out, "div\n");
			return 1;
		
		case 'i':
			if (node->right->val->val != '|') {
				nodeToAsm(node->left, out);
				int counter = IfCounter;
				if (node->left->val->type != OP) {
					fprintf(out, "push 0\n"
								 "je if%d_end\n", counter);
				} else {
					switch (node->left->val->val) {
					case 'l':
						fprintf(out, "ja if%d_end\n", counter);
						break;
					case 'L':
						fprintf(out, "jae if%d_end\n", counter);
						break;
					case 'g':
						fprintf(out, "jb if%d_end\n", counter);
						break;
					case 'G':
						fprintf(out, "jbe if%d_end\n", counter);
						break;
					case 'e':
						fprintf(out, "jne if%d_end\n", counter);
						break;
					case 'n':
						fprintf(out, "je if%d_end\n", counter);
						break;
					default:
						fprintf(out, "push 0\n"
									 "je if%d_end\n", counter);
					}
				}
				IfCounter++;
				nodeToAsm(node->right, out);
				fprintf(out, "if%d_end:\n", counter);
				return 1;
			} else {
				nodeToAsm(node->left, out);
				int counter = IfCounter;
				if (node->left->val->type != OP) {
					fprintf(out, "push 0\n"
								 "je else%d\n", counter);
				} else {
					switch (node->left->val->val) {
					case 'l':
						fprintf(out, "ja else%d\n", counter);
						break;
					case 'L':
						fprintf(out, "jae else%d\n", counter);
						break;
					case 'g':
						fprintf(out, "jb else%d\n", counter);
						break;
					case 'G':
						fprintf(out, "jbe else%d\n", counter);
						break;
					case 'e':
						fprintf(out, "jne else%d\n", counter);
						break;
					case 'n':
						fprintf(out, "je else%d\n", counter);
						break;
					default:
						fprintf(out, "push 0\n"
									 "je else%d\n", counter);
					}
				}
				IfCounter++;
				nodeToAsm(node->right->left, out);
				fprintf(out, "jmp if%d_end\n"
							 "else%d:\n", counter, counter);
				nodeToAsm(node->right->right, out);
				fprintf(out, "if%d_end:\n", counter);
				return 1;
			}
		case 'w':
			fprintf(out, "loop%d:\n", LoopCounter);
			int counter = LoopCounter;
			nodeToAsm(node->left, out);
			if (node->left->val->type != OP) {
				fprintf(out, "push 0\n"
							 "je loop%d_end\n", counter);
			} else {
				switch (node->left->val->val) {
				case 'l':
					fprintf(out, "ja loop%d_end\n", counter);
					break;
				case 'L':
					fprintf(out, "jae loop%d_end\n", counter);
					break;
				case 'g':
					fprintf(out, "jb loop%d_end\n", counter);
					break;
				case 'G':
					fprintf(out, "jbe loop%d_end\n", counter);
					break;
				case 'e':
					fprintf(out, "jne loop%d_end\n", counter);
					break;
				case 'n':
					fprintf(out, "je loop%d_end\n", counter);
					break;
				default:
					fprintf(out, "push 0\n"
								 "je loop%d_end\n", counter);
				}
			}
			LoopCounter++;
			nodeToAsm(node->right, out);
			fprintf(out, "jmp loop%d\n", counter);
			fprintf(out, "loop%d_end:\n", counter);
			return 1;
		
		case '=':
			nodeToAsm(node->right, out);
			fprintf(out, "popr r%d\n", node->left->val->val - 'a');
			return 1;
		
		case 'm':
			fprintf(out, "meow\n");
			return 1;
		case 'd':
			fprintf(out, "dump\n");
			return 1;
		
		case 's':
			nodeToAsm(node->right, out);
			fprintf(out, "sqrt\n");
			return 1;
		
		case 'O':
			nodeToAsm(node->right, out);
			fprintf(out, "out\n");
			return 1;
		case 'I':
			fprintf(out, "in\n");
			return 1;
		
		case 'r':
			if (node->right == NULL)
				fprintf(out, "push 0\n");
			else
				nodeToAsm(node->right, out);
			fprintf(out, "ret\n");
			return 1;
		
		default:
			return 0;
		}
	default:
		return 0;
	}
}

int treeToAsm(t_node *root, char *file) {
	if (root == NULL)
		return 0;
	
	FILE *out = fopen(file, "wb");
	
	fprintf(out, "jmp main\n");
	nodeToAsm(root, out);
	
	fclose(out);
	
	return 1;
}