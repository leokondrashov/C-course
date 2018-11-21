#include <tree.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define BUFF_SIZE 100

int treeCtor(tree *t) {
	assert(t);
	
	t->root = NULL;
	t->size = 0;
	t->errno = 0;
	
	return t->errno;
}

void freeNodes(t_node *node) {
	if (node == NULL)
		return;
	
	freeNodes(node->left);
	freeNodes(node->right);
	free(node->val);
	tNodeDtor(node);
	free(node);
}

void treeDtor(tree *t) {
	assert(t);
	
	freeNodes(t->root);
	t->root = NULL;
	t->size = 0;
	t->errno = 0;
}

void treeClear(tree *t) {
	assert(t);
	
	t->root = NULL;
	t->size = 0;
	t->errno = 0;
}

int treeCountNodes(tree *t, t_node *node, int *counter) {
	if (node == NULL)
		return 0;
	
	(*counter)++;
	if (*counter > t->size)
		return -1;
	
	if (treeCountNodes(t, node->left, counter) == -1)
		return -1;
	
	if (treeCountNodes(t, node->right, counter) == -1)
		return -1;
	
	return *counter;
}

int checkLinks(t_node *node) {
	if (node == NULL)
		return 1;
	
	if (node->left != NULL) {
		if (node->left->parent != node || !checkLinks(node->left))
			return 0;
	}
	
	if (node->right != NULL) {
		if (node->right->parent != node || !checkLinks(node->right))
			return 0;
	}
	
	return 1;
}

int treeOk(tree *t) {
	if (t == NULL)
		return 0;
	
	if (t->errno != TREE_NO_ERROR)
		return 0;
	
	int nodeCount = 0;
	treeCountNodes(t, t->root, &nodeCount);
	
	if (nodeCount != t->size) {
		t->errno = TREE_SIZE_ERROR;
		return 0;
	}
	
	if (checkLinks(t->root) || (t->root != NULL && t->root->parent != NULL)) {
		t->errno = TREE_LINK_ERROR;
		return 0;
	}
	
	return 1;
}

int Fopen(const char *file);
int Fclose();
int Fprintf(char *fmt, ...);

void printGraphNode(t_node *node) {
	if (node == NULL)
		return;

//	Fprintf( "\tn%p [shape=record, label=\"{%d|{<l%p> left|<r%p> right}}\"];\n", node, node->val, node, node);
	Fprintf("\tn%p [shape=record, label=\"{%s|{<l%p> left|<r%p> right}}\"];\n", node, node->val, node, node);
	if (node->left)
		Fprintf("\tn%p:l%p -> n%p:n;\n", node, node, node->left);
	if (node->right)
		Fprintf("\tn%p:r%p -> n%p:n;\n", node, node, node->right);
	if (node->parent)
		Fprintf("\tn%p:n -> n%p;\n", node, node->parent);
}

void plotGraph(tree *t) {
	assert(t);
	
	Fopen("tree.dv");
	Fprintf("digraph tree {\n");
	processTree(t, printGraphNode);
	Fprintf("}\n");
	Fclose();
	
	system("dotty tree.dv");
	
	remove("tree.dv");
}

void treeDump(tree *t) {
	assert(t);
	
	printf("tree [%p] {\n", t);
	printf("\terrno = %d\n", t->errno);
	printf("\troot = [%p]\n", t->root);
	printf("\tsize = %d\n", t->size);
	printf("}\n");
	
	plotGraph(t);
}

int treeAddLeft(tree *t, t_node *node, t_node *newLeft) {
	assert(t);
	assert(newLeft);
	
	if (node == NULL) { // inserting as root
		if (t->root != NULL) {
			t->errno = TREE_LINK_ERROR;
			return t->errno;
		}
		
		t->root = newLeft;
		t->size = tNodeCountChildren(t->root);
		t->root->parent = NULL;
		
		return t->errno;
	}
	
	if (node->left != NULL) {
		t->errno = TREE_LINK_ERROR;
		return t->errno;
	}
	
	node->left = newLeft;
	node->left->parent = node;
	t->size += tNodeCountChildren(newLeft);
	
	return t->errno;
}

int treeAddRight(tree *t, t_node *node, t_node *newRight) {
	assert(t);
	assert(newRight);
	
	if (node == NULL) { // inserting as root
		if (t->root != NULL) {
			t->errno = TREE_LINK_ERROR;
			return t->errno;
		}
		
		t->root = newRight;
		t->size = tNodeCountChildren(t->root);
		
		return t->errno;
	}
	
	if (node->right != NULL) {
		t->errno = TREE_LINK_ERROR;
		return t->errno;
	}
	
	node->right = newRight;
	node->right->parent = node;
	t->size += tNodeCountChildren(newRight);
	
	return t->errno;
}

int treeRemove(tree *t, t_node *node) {
	assert(t);
	assert(node);
	
	if (t->root == node) {
		t->root = NULL;
		t->size = 0;
		
		return t->errno;
	}
	
	if (node->parent->left == node)
		node->parent->left = NULL;
	else
		node->parent->right = NULL;
	
	t->size -= tNodeCountChildren(node);
	node->parent = NULL;
	
	return t->errno;
}

t_node *treeRoot(tree *t) {
	assert(t);
	
	return t->root;
}

int treeSize(tree *t) {
	assert(t);
	
	return t->size;
}

int treeErrno(tree *t) {
	assert(t);
	
	return t->errno;
}

void treeResetErrno(tree *t) {
	assert(t);
	
	t->errno = 0;
}

void processTree(tree *t, void (*operation)(t_node *node)) {
	assert(t);
	
	process(t->root, operation);
}

void process(t_node *node, void (*operation)(t_node *node)) {
	if (node == NULL)
		return;
	
	(*operation)(node);
	process(node->left, operation);
	process(node->right, operation);
}

void tNodeCtor(t_node *node, tree_data_t val) {
	assert(node);
	
	node->val = val;
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
}

void tNodeDtor(t_node *node) {
	assert(node);
	
	node->val = 0;
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;
}

tree_data_t tNodeVal(t_node *node) {
	assert(node);
	
	return node->val;
}

t_node *tNodeLeft(t_node *node) {
	assert(node);
	
	return node->left;
}

t_node *tNodeRight(t_node *node) {
	assert(node);
	
	return node->right;
}

int tNodeCountChildren(t_node *node) {
	if (node == NULL)
		return 0;
	
	return tNodeCountChildren(node->left) + tNodeCountChildren(node->right) + 1;
}

FILE *out;

int Fopen(const char *file) {
	assert(file);
	
	out = fopen(file, "wb");
	
	if (out == NULL)
		return EOF;
	
	return 0;
}

int Fclose() {
	if (out == NULL)
		return EOF;
	
	fclose(out);
	return 0;
}

int Fprintf(char *fmt, ...) {
	assert(fmt);
	
	if (out == NULL)
		return 0;
	
	va_list args;
	va_start(args, fmt);
	int res = vfprintf(out, fmt, args);
	va_end(args);
	
	return res;
}

int loadNode(t_node *node, FILE *in) {
	assert(node);
	assert(in);

//	fscanf(in, "%d", &node->val);
	char *buff = (char *) calloc(BUFF_SIZE, sizeof(char));
	fscanf(in, " %*[\n]");
	fgets(buff, BUFF_SIZE - 1, in);
	size_t len = strlen(buff);
	node->val = (char *) calloc(len + 1, sizeof(char));
	strncpy(node->val, buff, len - (buff[len - 1] == '\n' ? 1 : 0));
	free(buff);
	
	char c = 0;
	fscanf(in, " %c", &c);
	if (c == 'n') {
		fscanf(in, "%*s");
		node->left = NULL;
	} else if (c == '{') {
		node->left = (t_node *) calloc(1, sizeof(t_node));
		tNodeCtor(node->left, 0);
		if (loadNode(node->left, in) == 0)
			return 0;
		node->left->parent = node;
	} else {
		return 0;
	}
	
	fscanf(in, " %c", &c);
	if (c == 'n') {
		fscanf(in, "%*s");
		node->right = NULL;
	} else if (c == '{') {
		node->right = (t_node *) calloc(1, sizeof(t_node));
		tNodeCtor(node->right, 0);
		if (loadNode(node->right, in) == 0)
			return 0;
		node->right->parent = node;
	} else {
		return 0;
	}
	
	fscanf(in, " %c", &c);
	if (c != '}')
		return 0;
	
	return 1;
}

int treeLoadFromFile(tree *t, const char *file) {
	assert(t);
	assert(file);
	
	if (t->root != NULL)
		return 0;
	
	FILE *in = fopen(file, "rb");
	if (in == NULL)
		return 0;
	
	char c = 0;
	fscanf(in, " %c", &c);
	if (c != '{') {
		fclose(in);
		return 0;
	}
	
	t->root = (t_node *) calloc(1, sizeof(t_node));
	tNodeCtor(t->root, 0);
	
	if (loadNode(t->root, in) == 0) {
		freeNodes(t->root);
		t->root = NULL;
		fclose(in);
		return 0;
	}
	
	fclose(in);
	t->size = tNodeCountChildren(t->root);
	return 1;
}

void printTextNode(t_node *node) {
	static int indent = 0;
	if (node == NULL) {
		for (int i = 0; i < indent; i++)
			Fprintf("\t");
		Fprintf("nil\n");
		return;
	}
	
	for (int i = 0; i < indent; i++)
		Fprintf("\t");
	Fprintf("{\n");
	indent++;
	for (int i = 0; i < indent; i++)
		Fprintf("\t");
//	Fprintf("%d\n", node->val);
	Fprintf("%s\n", node->val);
	
	printTextNode(node->left);
	printTextNode(node->right);
	
	indent--;
	for (int i = 0; i < indent; i++)
		Fprintf("\t");
	Fprintf("}\n");
}

int treeSaveToFile(tree *t, const char *file) {
	assert(t);
	
	if (Fopen(file) != 0)
		return 0;
	
	printTextNode(t->root);
	
	Fclose();
	return 1;
}

int findNode(t_node *node, char *path, tree_data_t val) {
	assert(path);
	
	if (node == NULL)
		return 0;

//	if (node->val == val)
	if (strcasecmp(node->val, val) == 0)
		return 1;
	
	*path = 'l';
	if (findNode(node->left, path + 1, val) != 0)
		return 1;
	
	*path = 'r';
	if (findNode(node->right, path + 1, val) != 0)
		return 1;
	
	*path = '\0';
	return 0;
}

int treeFind(tree *t, char *path, tree_data_t val) {
	assert(t);
	assert(path);
	
	return findNode(t->root, path, val);
}