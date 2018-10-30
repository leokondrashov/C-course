#include <map.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
int main() {
	map m = {};
	mapCtor(&m);
	mapAdd(&m, "wda", 1);
	printf("%d %d\n", mapGet(&m, "wda"), mapGet(&m, "w"));
	mapDump(&m);
	mapDtor(&m);
}*/

void mapCtor(map *m) {
	assert(m);
	
	m->head = NULL;
	m->size = 0;
	m->errno = MAP_NO_ERROR;
}

void mapDtor(map *m) {
	assert(m);
	
	void freeNodes(struct m_node *node);
	
	freeNodes(m->head);
	m->size = 0;
	m->errno = MAP_NO_ERROR;
}

void freeNodes(struct m_node *node) {
	if (node == NULL)
		return;
	
	freeNodes(node->next);
	mNodeDtor(node);
}

int mapOk(map *m) {
	assert(m);
	
	if (m->errno != MAP_NO_ERROR) return 0;
	return 1;
}

void mapDump(map *m) {
	assert(m);
	
	printf("map [%p] {\n", m);
	printf("\tsize = %d\n", m->size);
	printf("\tnodes:\n");
	for (struct m_node *cur = m->head; cur != NULL; cur = cur->next) {
		printf("\t\t\"%s\" -> %d [%p]\n", cur->key, cur->val, cur);
	}
	printf("\terrno = %d\n", m->errno);
	printf("}\n");
}

int mapAdd(map *m, char *key, int val) {
	assert(m);
	assert(key);
	
	if (m->head == NULL) {
		struct m_node *node = (struct m_node *)calloc(1, sizeof(struct m_node));
		if (node == NULL) {
			m->errno = MAP_ALLOCATION_ERROR;
			return 0;
		}
		mNodeCtor(node, key, val);
		m->head = node;
		m->size++;
		return 1;
	}
	
	struct m_node *cur = m->head;
	while (cur->next != NULL) {
		if (strcmp(cur->key, key) == 0)
			return 2;
		cur = cur->next;
	}
	if (strcmp(cur->key, key) == 0)
		return 2;
	struct m_node *node = (struct m_node *)calloc(1, sizeof(struct m_node));
	if (node == NULL) {
		m->errno = MAP_ALLOCATION_ERROR;
		return 0;
	}
	mNodeCtor(node, key, val);
	cur->next = node;
	m->size++;
	return 1;
}

int mapGet(map *m, char *key) {
	assert(m);
	assert(key);
	
	if (m->head == NULL) {
		m->errno = MAP_NO_SUCH_ELEMENT;
		return 0;
	}
	
	struct m_node *cur = m->head;
	while (cur != NULL) {
		if (strcmp(cur->key, key) == 0)
			return cur->val;
		cur = cur->next;
	}
	m->errno = MAP_NO_SUCH_ELEMENT;
	return 0;
}

int mapRemove(map *m, char *key) {
	assert(m);
	assert(key);
	
	if (m->head == NULL)
		return 0;
	
	if (strcmp(m->head->key, key) == 0) {
		struct m_node *newHead = m->head->next;
		mNodeDtor(m->head);
		m->head = newHead;
		m->size--;
		return 1;
	}
	
	struct m_node *cur = m->head;
	while (cur->next != NULL) {
		if (strcmp(cur->next->key, key) == 0) {
			struct m_node *next = cur->next->next;
			mNodeDtor(cur->next);
			cur->next = next;
			m->size--;
			return 1;
		}
		cur = cur->next;
	}
	return 0;
}

int mapFind(map *m, char *key) {
	assert(m);
	assert(key);
	
	if (m->head == NULL) 
		return 0;
	
	struct m_node *cur = m->head;
	while (cur != NULL) {
		if (strcmp(cur->key, key) == 0)
			return 1;
		cur = cur->next;
	}
	return 0;
}

unsigned int mapSize(map *m) {
	assert(m);
	
	return m->size;
}

int mapErrno(map *m) {
	assert(m);
	
	return m->errno;
}

void mapResetErrno(map *m) {
	assert(m);
	
	m->errno = 0;
}

struct m_node *mapBegin(map *m) {
	assert(m);
	
	return m->head;
}

struct m_node *mapEnd(map *m) {
	assert(m);
	
	return NULL;
}

void mNodeCtor(struct m_node *node, char *key, int val) {
	assert(node);
	assert(key);
	
	node->next = NULL;
	
	int len = strlen(key);
	node->key = (char *)calloc(len + 1, sizeof(char));
	strcpy(node->key, key);
	
	node->val = val;
}

void mNodeDtor(struct m_node *node) {
	assert(node);
	
	node->next = NULL;
	
	free(node->key);
	
	node->val = 0;
}

struct m_node *mNodeNext(struct m_node *node) {
	assert(node);
		
	return node->next;
}

char *mNodeKey(struct m_node *node) {
	assert(node);
		
	return node->key;
}

int mNodeVal(struct m_node *node) {
	assert(node);
		
	return node->val;
}