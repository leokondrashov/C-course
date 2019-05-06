#include <stdio.h>
#include <stdlib.h>
#include <tree.h>
#include <textProcessor.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <map.h>
#include <stack.h>
#include <threads.h>

#define sassert(expr) \
if (!(expr)) { \
    printf("unexpected symbol \'%c\' on line %d(%s)\n", *S, CurLine + 1, __func__); \
    Error = 1; \
}

map Functions, Vars;

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
	
	mapCtor(&Functions);
	mapCtor(&Vars);
	
	tree t = {};
	treeCtor(&t);
	t.root = getProg(infile);
	treeDump(&t);
	
	treeToAsm(t.root, outfile);
	
	treeDtor(&t);
	
	mapDtor(&Functions);
	mapDtor(&Vars);
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
t_node *getArgListDeclaration();
t_node *getReturn();
int getArgCount(t_node *node);

int getVarCount(t_node *node);
void addVars(t_node *node, stack *stk);
void addArgs(t_node *node);

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
		val = createNodeInt(OP, ';', val, funcNode);
	}
	val = createNodeInt(OP, ';', val, createNodeInt(OP, '0', NULL, getOPBlock()));
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
		
		char *funcName = calloc(len + 1, sizeof(char));
		strcpy(funcName, S);
		
		S[len] = savedChar;
		S += len - 1;
		incrementPointer();
		
		mapResetErrno(&Functions);
		mapGet(&Functions, funcName);
		
		if (mapErrno(&Functions) != MAP_NO_SUCH_ELEMENT) {
			printf("redeclaration of function %s on line %d\n", funcName, CurLine);
			sassert(*S == '(');
			incrementPointer();
			
			sassert(*S == ')');
			incrementPointer();
			
			freeNodes(getOPBlock());
			free(funcName);
			return NULL;
		}
		
		sassert(*S == '(');
		incrementPointer();
		
		t_node *argList = NULL;
		if (*S != ')') {
			argList = getArgListDeclaration();
		}
		
		sassert(*S == ')');
		incrementPointer();
		
		mapAdd(&Functions, funcName, getArgCount(argList));
		
		return createNodeStr(FUNCTION_DECLARATION, funcName, argList,
				createNodeInt(OP, ';', getOPBlock(), createNodeInt(OP, 'r', NULL, NULL)));
	}
	
	return NULL;
}

t_node *getArgListDeclaration() {
	t_node *args = getId();
	
	while (*S == ',') {
		incrementPointer();
		args = createNodeInt(OP, ',', args, getId());
	}
	
	return args;
}

int getArgCount(t_node *node) {
	if (node == NULL)
		return 0;
	
	if (node->val->type == OP && node->val->val.i == ',')
		return getArgCount(node->left) + getArgCount(node->right);
	
	return 1;
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
				val = createNodeInt(OP, ';', val, opNode);
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
	
	if (S[getWordLength()] == '(') { //TODO right check for function call
		t_node *func = createNodeInt(OP, ';', getFunctionCall(), createNodeInt(OP, 'p', NULL, NULL));
		sassert(*S == ';');
		incrementPointer();
		return func;
	}
	
	return getAssignment();
}

t_node *getReturn() {
	sassert(isNextWord("return"));
	S += 5;
	incrementPointer();
	
	if (*S == ';') {
		incrementPointer();
		return createNodeInt(OP, 'r', NULL, NULL);
	}
	
	t_node *val = createNodeInt(OP, 'r', NULL, getE());
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
		operation = createNodeInt(OP, '|', operation, getOPBlock());
	}
	
	return createNodeInt(OP, 'i', condition, operation);
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
	
	return createNodeInt(OP, 'w', condition, operation);
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
	
	return createNodeInt(OP, '=', id, expr);
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
			return createNodeInt(OP, 'g', NULL, NULL);
		}
		
		incrementPointer();
		return createNodeInt(OP, 'G', NULL, NULL);
	case '<':
		if (*(S + 1) == '=') {
			S++;
			incrementPointer();
			return createNodeInt(OP, 'l', NULL, NULL);
		}
		
		incrementPointer();
		return createNodeInt(OP, 'L', NULL, NULL);
	case '=':
		if (*(S + 1) == '=') {
			S++;
			incrementPointer();
			return createNodeInt(OP, 'e', NULL, NULL);
		}
		return NULL;
	case '!':
		if (*(S + 1) == '=') {
			S++;
			incrementPointer();
			return createNodeInt(OP, 'n', NULL, NULL);
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
		t_node *opNode = createNodeInt(OP, op, val, getT());
		
		val = opNode;
	}
	
	return val;
}

t_node *getT() {
	t_node *val = getP();
	
	while (*S == '*' || *S == '/') {
		int op = *S;
		incrementPointer();
		t_node *opNode = createNodeInt(OP, op, val, getP());
		
		val = opNode;
	}
	
	return val;
}

t_node *getP() {
	if (*S == '(') {
		incrementPointer();
		t_node *val = getE();
		sassert(*S == ')');
		incrementPointer();
		return val;
	}
	
	if (isdigit(*S) || *S == '+' || *S == '-')
		return getN();
	
	if (S[getWordLength()] == '(') { //TODO right check for function call
		return getFunctionCall();
	}
	
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
	return createNodeInt(CONST, constVal, NULL, NULL);
}

t_node *getId() {
	size_t len = getWordLength();
	
	sassert(len != 0);
	
	char savedChar = S[len];
	S[len] = '\0';
	
	char *varName = calloc(len + 1, sizeof(char));
	strcpy(varName, S);
	
	S[len] = savedChar;
	S += len - 1;
	incrementPointer();
	
	t_node *val = createNodeStr(VAR, varName, NULL, NULL);
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
		
		return createNodeInt(OP, 'I', NULL, NULL);
	} else if (isNextWord("out")) {
		S += 2;
		incrementPointer();
		
		sassert(*S == '(');
		incrementPointer();
		
		t_node *arg = getE();
		
		sassert(*S == ')');
		incrementPointer();
		
		return createNodeInt(OP, 'O', NULL, arg);
	} else {
		
		size_t len = getWordLength();
		if (len == 0)
			return NULL;
		
		char savedChar = S[len];
		S[len] = '\0';
		
		char *funcName = calloc(len + 1, sizeof(char));
		strcpy(funcName, S);
		S[len] = savedChar;
		
		mapResetErrno(&Functions);
		int argNum = mapGet(&Functions, funcName);
		
		if (mapErrno(&Functions) != MAP_NO_SUCH_ELEMENT) {
			t_node *val = createNodeStr(FUNCTION, funcName, NULL, NULL);
			S += len - 1;
			incrementPointer();
			
			sassert(*S == '(');
			incrementPointer();
			
			if (*S != ')') {
				t_node *args = getArgList();
				if (getArgCount(args) != argNum) {
					printf("wrong args count on line %d: expected %d, got %d\n", CurLine, argNum, getArgCount(args));
					Error = 1;
					
					sassert(*S == ')');
					incrementPointer();
					
					freeNodes(args);
					freeNodes(val);
					return NULL;
				}
				val->right = args;
			} else if (argNum != 0) {
				printf("wrong args count on line %d: expected %d, got %d\n", CurLine, argNum, 0);
				Error = 1;
				
				sassert(*S == ')');
				incrementPointer();
				
				freeNodes(val);
				return NULL;
			}
			
			sassert(*S == ')');
			incrementPointer();
			return val;
		}
		printf("unknown function %s on line %d\n", funcName, CurLine);
		Error = 1;
		S += len - 1;
		incrementPointer();
		incrementPointer(); // '('
		
		freeNodes(getArgList());
		
		incrementPointer(); // ')'
		
		free(funcName);
		return NULL;
	}
}

t_node *getArgList() {
	t_node *args = getE();
	
	while (*S == ',') {
		incrementPointer();
		args = createNodeInt(OP, ',', args, getE());
	}
	
	return args;
}

int getVarCount(t_node *node) {
	static int count = 0;
	static int maxChildrenCount = 0;
	static stack *stk = NULL;
	
	char stkCreator = 0;
	
	if (node == NULL)
		return 0;
	
	if (stk == NULL) {
		stkCreator = 1;
		stk = calloc(1, sizeof(stack));
		stackCtor(stk);
	}
	
	if (node->val->type == OP && (node->val->val.i == 'i' || node->val->val.i == 'w')) {
		int countSaved = count;
		count = 0;
		int maxChildrenCountSaved = maxChildrenCount;
		maxChildrenCount = 0;
		stack *stkSaved = stk;
		stk = NULL;
		
		if (node->right->val->type == OP && node->right->val->val.i == '|') {
			int childrenCount = getVarCount(node->right->left);
			maxChildrenCountSaved = (childrenCount > maxChildrenCountSaved) ? (childrenCount) : (maxChildrenCountSaved);
			
			count = 0;
			maxChildrenCount = 0;
			
			childrenCount = getVarCount(node->right->right);
			maxChildrenCount = (childrenCount > maxChildrenCountSaved) ? (childrenCount) : (maxChildrenCountSaved);
		} else {
			int childrenCount = getVarCount(node->right);
			maxChildrenCount = (childrenCount > maxChildrenCountSaved) ? (childrenCount) : (maxChildrenCountSaved);
		}
		
		count = countSaved;
		maxChildrenCount = maxChildrenCountSaved;
		stk = stkSaved;
	} else if (node->val->type == OP && node->val->val.i == '=') {
		mapResetErrno(&Vars);
		mapGet(&Vars, node->left->val->val.s);
		if (mapErrno(&Vars) == MAP_NO_SUCH_ELEMENT) {
			mapAdd(&Vars, node->left->val->val.s, 0);
			stackPush(stk, node->left->val->val.s);
			count++;
		}
	} else {
		getVarCount(node->left);
		getVarCount(node->right);
	}
	
	if (stkCreator) {
		while (stackSize(stk)) {
			mapRemove(&Vars, stackPop(stk));
		}
		stackDtor(stk);
		free(stk);
		stk = NULL;
	}
	
	return count + maxChildrenCount;
}

int curOffset = 0;

void addVars(t_node *node, stack *stk) {
	assert(stk);
	
	if (node == NULL)
		return;
	
	if ((node->val->type == OP && (node->val->val.i == 'i' || node->val->val.i == 'w')) || node->val->type == CONST ||
			node->val->type == FUNCTION || node->val->type == VAR)
		return;
	
	if (node->val->type == OP && node->val->val.i == '=') {
		mapResetErrno(&Vars);
		mapGet(&Vars, node->left->val->val.s);
		if (mapErrno(&Vars) == MAP_NO_SUCH_ELEMENT) {
			mapAdd(&Vars, node->left->val->val.s, curOffset);
			curOffset -= 8;
			stackPush(stk, node->left->val->val.s);
		}
		
		return;
	}
	
	addVars(node->left, stk);
	addVars(node->right, stk);
}

void addArgs(t_node *node) {
	if (node == NULL)
		return;
	
	if (node->val->type == VAR) {
		mapAdd(&Vars, node->val->val.s, curOffset);
		
		curOffset -= 8;
	}
	
	addArgs(node->left);
	addArgs(node->right);
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
	
	stack *stk = NULL;
	
	switch (node->val->type) {
	case CONST:
		fprintf(out, "mov rax, %d\n"
					 "push rax\n", node->val->val.i);
		return 1;
	case VAR:
		fprintf(out, "push [rbp + (%d)]\n", mapGet(&Vars, node->val->val.s));
		return 1;
	case FUNCTION:
		fprintf(out, "sub rsp, 8\n");
		nodeToAsm(node->right, out);
		fprintf(out, "call %s\n"
					 "add rsp, %d\n", node->val->val.s, getArgCount(node->right) * 8);
		return 1;
	case FUNCTION_DECLARATION:
		stk = calloc(1, sizeof(stack));
		stackCtor(stk);
		curOffset = getArgCount(node->left) * 8 + 8;
		mapAdd(&Vars, "_retVal", curOffset + 8);
		addArgs(node->left);
		fprintf(out, "%s:\n"
					 "push rbp\n"
					 "mov rbp, rsp\n"
					 "sub rsp, %d\n", node->val->val.s, getVarCount(node->right) * 8);
		curOffset = -8;
		addVars(node, stk);
		stackDtor(stk);
		free(stk);
		nodeToAsm(node->right, out);
		mapClear(&Vars);
		return 1;
	case OP:
		switch ((char) node->val->val.i) {
		case ',':
			ArgsCounter++;
			nodeToAsm(node->right, out);
			nodeToAsm(node->left, out);
			return 1;
		case 'p':
			fprintf(out, "pop rax\n");
			return 1;
		case '0':
			fprintf(out, "_start:\n"
						 "push rbp\n"
						 "mov rbp, rsp\n"
						 "sub rsp, %d\n", getVarCount(node->right) * 8);
			stk = calloc(1, sizeof(stack));
			stackCtor(stk);
			curOffset = -8;
			addVars(node->right, stk);
			nodeToAsm(node->right, out);
			while (stackSize(stk)) {
				mapRemove(&Vars, stackPop(stk));
			}
			stackDtor(stk);
			free(stk);
			fprintf(out, "leave\n"
						 "mov rax, 0x3c\n"
						 "xor rdi, rdi\n"
						 "syscall\n");
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
			fprintf(out, "pop rbx\n"
						 "pop rax\n"
						 "add rax, rbx\n"
						 "push rax\n");
			return 1;
		case '-':
			nodeToAsm(node->left, out);
			nodeToAsm(node->right, out);
			fprintf(out, "pop rbx\n"
						 "pop rax\n"
						 "sub rax, rbx\n"
						 "push rax\n");
			return 1;
		case '*':
			nodeToAsm(node->left, out);
			nodeToAsm(node->right, out);
			fprintf(out, "pop rbx\n"
						 "pop rax\n"
						 "xor rdx, rdx\n"
						 "imul rbx\n"
						 "push rax\n");
			return 1;
		case '/':
			nodeToAsm(node->left, out);
			nodeToAsm(node->right, out);
			fprintf(out, "pop rbx\n"
						 "pop rax\n"
						 "xor rdx, rdx\n"
						 "idiv rbx\n"
						 "push rax\n");
			return 1;
		
		case 'i':
			if (node->right->val->val.i != '|') {
				nodeToAsm(node->left, out);
				int counter = IfCounter;
				if (node->left->val->type != OP) {
					fprintf(out, "pop rax\n"
								 "test rax, rax\n"
								 "je if%d_end\n", counter);
				} else {
					fprintf(out, "pop rbx\n"
								 "pop rax\n"
								 "cmp rax, rbx\n");
					switch (node->left->val->val.i) {
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
						fprintf(out, "push rax\n"
									 "test rbx, rbx\n"
									 "je if%d_end\n", counter);
					}
				}
				IfCounter++;
				stk = calloc(1, sizeof(stack));
				stackCtor(stk);
				addVars(node->right, stk);
				nodeToAsm(node->right, out);
				while (stackSize(stk)) {
					mapRemove(&Vars, stackPop(stk));
				}
				stackDtor(stk);
				free(stk);
				fprintf(out, "if%d_end:\n", counter);
			} else {
				nodeToAsm(node->left, out);
				int counter = IfCounter;
				if (node->left->val->type != OP) {
					fprintf(out, "pop rax\n"
								 "test rax, rax\n"
								 "je else%d\n", counter);
				} else {
					fprintf(out, "pop rbx\n"
								 "pop rax\n"
								 "cmp rax, rbx\n");
					switch (node->left->val->val.i) {
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
						fprintf(out, "push rax\n"
									 "test rbx, rbx\n"
									 "je else%d\n", counter);
					}
				}
				IfCounter++;
				stk = calloc(1, sizeof(stack));
				stackCtor(stk);
				addVars(node->right->left, stk);
				nodeToAsm(node->right->left, out);
				while (stackSize(stk)) {
					mapRemove(&Vars, stackPop(stk));
					curOffset += 8;
				}
				fprintf(out, "jmp if%d_end\n"
							 "else%d:\n", counter, counter);
				addVars(node->right->right, stk);
				nodeToAsm(node->right->right, out);
				while (stackSize(stk)) {
					mapRemove(&Vars, stackPop(stk));
					curOffset += 8;
				}
				stackDtor(stk);
				free(stk);
				fprintf(out, "if%d_end:\n", counter);
			}
			
			return 1;
		case 'w':
			stk = calloc(1, sizeof(stack));
			stackCtor(stk);
			addVars(node->right, stk);
			
			fprintf(out, "loop%d:\n", LoopCounter);
			int counter = LoopCounter;
			nodeToAsm(node->left, out);
			if (node->left->val->type != OP) {
				fprintf(out, "pop rax\n"
							 "test rax, rax\n"
							 "je loop%d_end\n", counter);
			} else {
				fprintf(out, "pop rbx\n"
							 "pop rax\n"
							 "cmp rax, rbx\n");
				switch (node->left->val->val.i) {
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
					fprintf(out, "push rax\n"
								 "test rbx, rbx\n"
								 "je loop%d_end\n", counter);
				}
			}
			LoopCounter++;
			nodeToAsm(node->right, out);
			fprintf(out, "jmp loop%d\n", counter);
			fprintf(out, "loop%d_end:\n", counter);
			
			while (stackSize(stk)) {
				mapRemove(&Vars, stackPop(stk));
				curOffset += 8;
			}
			stackDtor(stk);
			free(stk);
			return 1;
		
		case '=':
			nodeToAsm(node->right, out);
			fprintf(out, "pop [rbp + (%d)]\n", mapGet(&Vars, node->left->val->val.s));
			return 1;
		
		case 'O':
			nodeToAsm(node->right, out);
			fprintf(out, "call out\n");
			return 1;
		case 'I':
			fprintf(out, "sub rsp, 8\n"
						 "call in\n");
			return 1;
		
		case 'r':
			if (node->right == NULL)
				fprintf(out, "mov rax, 0\n"
							 "push rax\n");
			else
				nodeToAsm(node->right, out);
			fprintf(out, "pop rax\n"
						 "mov [rbp + (%d)], rax\n"
						 "leave\n"
						 "ret\n", mapGet(&Vars, "_retVal"));
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
	
	fprintf(out, ".intel_syntax noprefix\n"
				 "\n"
				 "\t\t.globl _start\n"
				 "\n"
				 ".text\n");
	mapClear(&Vars);
	nodeToAsm(root, out);
	
	fclose(out);
	
	return 1;
}
