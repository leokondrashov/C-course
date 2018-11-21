#include <tree.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 100
#define DB_FILE "mipt.db"

void guess(tree *t);
void define(tree *t);
void difference(tree *t);

int main() {
	tree db = {};
	treeCtor(&db);
	treeLoadFromFile(&db, DB_FILE);
	
	printf("Угадать/Определение/Разница? [g/d/i]\n");
	char c = 0;
	while (!feof(stdin)) {
		scanf(" %c", &c);
		switch (c) {
		case 'G':
		case 'g':
			guess(&db);
			break;
		case 'D':
		case 'd':
			define(&db);
			break;
		case 'I':
		case 'i':
			difference(&db);
			break;
		case 'q':
			treeSaveToFile(&db, DB_FILE);
			treeDtor(&db);
			return 0;
		case '~':
			treeDump(&db);
			break;
		default:
			continue;
		}
		printf("Угадать/Определение/Разница? [g/d/i]\n");
	}
	
	treeSaveToFile(&db, DB_FILE);
	treeDtor(&db);
}

void guess(tree *t) {
	assert(t);
	assert(t->root);
	
	char c = 0;
	t_node *cur = t->root;
	while (cur->left != NULL && cur->right != NULL) {
		printf("%s? [y/n]\n", cur->val);
		scanf(" %c", &c);
		if (c == 'y')
			cur = cur->left;
		else
			cur = cur->right;
	}
	
	printf("Это %s? [y/n]\n", cur->val);
	scanf(" %c", &c);
	if (c == 'y') {
		printf("Замечательно\n");
		return;
	}
	
	printf("А кто тогда?\n");
	char *buff = (char *) calloc(BUFF_SIZE, sizeof(char));
	scanf(" %*[\n]");
	fgets(buff, BUFF_SIZE - 1, stdin);
	size_t len = strlen(buff);
	char *newObject = (char *) calloc(len + 1, sizeof(char));
	strncpy(newObject, buff, len - (buff[len - 1] == '\n' ? 1 : 0));
	t_node *objectNode = (t_node *) calloc(1, sizeof(t_node));
	tNodeCtor(objectNode, newObject);
	
	printf("Чем он отличается от %s?\n", cur->val);
	scanf(" %*[\n]");
	fgets(buff, BUFF_SIZE - 1, stdin);
	len = strlen(buff);
	char *newQuestion = (char *) calloc(len + 1, sizeof(char));
	strncpy(newQuestion, buff, len - (buff[len - 1] == '\n' ? 1 : 0));
	t_node *questionNode = (t_node *) calloc(1, sizeof(t_node));
	tNodeCtor(questionNode, newQuestion);
	
	free(buff);
	
	t_node *parent = cur->parent;
	int leftChild = (parent->left == cur);
	treeRemove(t, cur);
	if (leftChild)
		treeAddLeft(t, parent, questionNode);
	else
		treeAddRight(t, parent, questionNode);
	treeAddLeft(t, questionNode, objectNode);
	treeAddRight(t, questionNode, cur);
	
	treeSaveToFile(t, DB_FILE);
}

void define(tree *t) {
	assert(t);
	
	printf("Кого ищем?\n");
	char *buff = (char *) calloc(BUFF_SIZE, sizeof(char));
	scanf(" %*[\n]");
	fgets(buff, BUFF_SIZE - 1, stdin);
	size_t len = strlen(buff);
	if (buff[len - 1] == '\n')
		buff[--len] = '\0';
	
	char *path = (char *) calloc(BUFF_SIZE, sizeof(char));
	if (!treeFind(t, path, buff)) {
		printf("%s не найден в базе\n", buff);
		free(buff);
		free(path);
		return;
	}
	
	t_node *cur = t->root;
	size_t pathLen = strlen(path);
	printf("%s: ", buff);
	for (int i = 0; i < pathLen; cur = (path[i] == 'l' ? cur->left : cur->right), i++) {
		printf("%s%s, ", (path[i] == 'r' ? "не " : ""), cur->val);
	}
	printf("\b\b\n");
	
	free(buff);
	free(path);
}

void difference(tree *t) {
	assert(t);
	
	char *first = (char *) calloc(BUFF_SIZE, sizeof(char));
	scanf(" %*[\n]");
	fgets(first, BUFF_SIZE - 1, stdin);
	size_t firstLen = strlen(first);
	if (first[firstLen - 1] == '\n')
		first[--firstLen] = '\0';
	
	char *firstPath = (char *) calloc(BUFF_SIZE, sizeof(char));
	if (!treeFind(t, firstPath, first)) {
		printf("%s не найден в базе\n", first);
		free(first);
		free(firstPath);
		return;
	}
	
	char *second = (char *) calloc(BUFF_SIZE, sizeof(char));
	scanf(" %*[\n]");
	fgets(second, BUFF_SIZE - 1, stdin);
	size_t secondLen = strlen(second);
	if (second[secondLen - 1] == '\n')
		second[--secondLen] = '\0';
	
	char *secondPath = (char *) calloc(BUFF_SIZE, sizeof(char));
	if (!treeFind(t, secondPath, second)) {
		printf("%s не найден в базе\n", second);
		free(first);
		free(firstPath);
		free(second);
		free(secondPath);
		return;
	}
	
	if (strcasecmp(first, second) == 0) {
		printf("%s и %s не отличаются\n", first, second);
		free(first);
		free(firstPath);
		free(second);
		free(secondPath);
		return;
	}
	
	t_node *cur = t->root;
	int i = 0;
	while (firstPath[i] == secondPath[i]) {
		cur = (firstPath[i] == 'l' ? cur->left : cur->right);
		i++;
	}
	if (firstPath[i] == 'l')
		printf("%s, в отличие от %s, %s\n", first, second, cur->val);
	else
		printf("%s, в отличие от %s, %s\n", second, first, cur->val);
	
	free(first);
	free(firstPath);
	free(second);
	free(secondPath);
}