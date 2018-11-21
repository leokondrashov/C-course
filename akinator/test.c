#include <tree.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
	tree t = {};
	treeCtor(&t);
	t_node *node = (t_node *) calloc(5, sizeof(t_node));
	for (int i = 0; i < 5; i++)
		tNodeCtor(node + i, i);
	treeAddLeft(&t, t.root, node);
	treeAddLeft(&t, t.root, node + 1);
	treeAddRight(&t, t.root, node + 2);
	treeAddLeft(&t, t.root->right, node + 3);
	treeAddRight(&t, t.root->left, node + 4);
	treeRemove(&t, t.root->left->right);
	treeAddLeft(&t, t.root->left, node + 4);
	treeDump(&t);
	
	char *path = (char *) calloc(5, sizeof(char));
	treeFind(&t, path, 3);
	printf("%s\n", path);
	
	treeClear(&t);
	treeDtor(&t);
	free(node);
}