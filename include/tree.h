#ifndef TREE_H

#define TREE_H

typedef struct tree tree;
typedef struct t_node t_node;
typedef struct token *tree_data_t;

enum treeError {
	TREE_NO_ERROR,
	TREE_ALLOCATION_ERROR,
	TREE_LINK_ERROR,
	TREE_SIZE_ERROR
};

enum tokenType {
	OP = 0,
	VAR = 1,
	CONST = 2,
	MAX_TOKEN_TYPE
};

struct tree {
	t_node *root;
	int size;
	int errno;
};

struct t_node {
	tree_data_t val;
	t_node *parent;
	t_node *left;
	t_node *right;
};

struct token {
	char type;
	double val;
};

int treeCtor(tree *t);
void treeDtor(tree *t);
void treeClear(tree *t);
void freeNodes(t_node *node);

int treeCountNodes(tree *t, t_node *node, int *counter);

int treeOk(tree *t);

void treeDump(tree *t);

int treeAddLeft(tree *t, t_node *node, t_node *newLeft);
int treeAddRight(tree *t, t_node *node, t_node *newRight);
int treeRemove(tree *t, t_node *node);

t_node *treeRoot(tree *t);
int treeSize(tree *t);
int treeErrno(tree *t);
void treeResetErrno(tree *t);

void processTree(tree *t, void (*operation)(t_node *node));
void process(t_node *node, void (*operation)(t_node *node));

void tNodeCtor(t_node *node, tree_data_t val);
void tNodeDtor(t_node *node);
t_node *createNode(char type, double val, t_node *left, t_node *right);

tree_data_t tNodeVal(t_node *node);
t_node *tNodeLeft(t_node *node);
t_node *tNodeRight(t_node *node);

int tNodeCountChildren(t_node *node);

char *loadNode(t_node *node, char *line);
int treeLoadFromFile(tree *t, const char *file);
int treeSaveToFile(tree *t, const char *file);

int treeFind(tree *t, char *path, tree_data_t val);

#endif
