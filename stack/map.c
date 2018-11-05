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

/*!
 * @brief constructor of struct map
 * @param m
 */
void mapCtor(map *m) {
	assert(m);
	
	m->head = NULL;
	m->size = 0;
	m->errno = MAP_NO_ERROR;
}

/*!
 * @brief destructor of strust map
 * @param m
 */
void mapDtor(map *m) {
	assert(m);
	
	void freeNodes(struct m_node *node);
	
	freeNodes(m->head);
	m->size = 0;
	m->errno = MAP_NO_ERROR;
}

/*!
 * @brief recursively frees nodes starting with node till the NULL pointer to next node
 * @param node
 */
void freeNodes(struct m_node *node) {
	if (node == NULL)
		return;
	
	freeNodes(node->next);
	mNodeDtor(node);
}

/*!
 * @brief checks if m is operable
 * @param m
 * @return 1 if map is operable, 0 otherwise
 */
int mapOk(map *m) {
	if (m == NULL)
		return 0;
	
	if (m->errno != MAP_NO_ERROR)
		return 0;
	
	return 1;
}

/*!
 * @brief debug info for map m
 * @param m
 */
void mapDump(map *m) {
	assert(m);
	
	printf("map [%p] {\n", m);
	printf("\terrno = %d\n", m->errno);
	printf("\tsize = %d\n", m->size);
	printf("\tnodes:\n");
	for (struct m_node *cur = m->head; cur != NULL; cur = cur->next) {
		printf("\t\t\"%s\" -> %d [%p]\n", cur->key, cur->val, cur);
	}
	printf("}\n");
}

/*!
 * @brief adds new node with key, val pair
 * @param m
 * @param key
 * @param val
 * @return 0 if error occured, 1 if addition completed, 2 if node with key is existing
 */
int mapAdd(map *m, const char *key, int val) {
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

/*!
 * @brief gets val by key
 * @param m
 * @param key
 * @return val of node with key or 0 if no such node present
 * @note if there is no such node errno is set to MAP_NO_SUCH_ELEMENT
 */
int mapGet(map *m, const char *key) {
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

/*!
 * @brief removes node with key
 * @param m
 * @param key
 * @return 0 if no such node present, 1 if deletion successful
 */
int mapRemove(map *m, const char *key) {
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

/*!
 * @brief checks if node with key exists
 * @param m
 * @param key
 * @return 1 if such node found, 0 otherwise
 */
int mapFind(map *m, const char *key) {
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

/*!
 *
 * @param m
 * @return size of m
 */
unsigned int mapSize(map *m) {
	assert(m);
	
	return m->size;
}

/*!
 *
 * @param m
 * @return errno of m
 */
int mapErrno(map *m) {
	assert(m);
	
	return m->errno;
}

/*!
 * @brief resets errno of m to 0
 * @param m
 */
void mapResetErrno(map *m) {
	assert(m);
	
	m->errno = 0;
}

/*!
 *
 * @param m
 * @return pointer to head node
 */
struct m_node *mapBegin(map *m) {
	assert(m);
	
	return m->head;
}

/*!
 *
 * @param m
 * @return pointer to next node from last node of list
 */
struct m_node *mapEnd(map *m) {
	assert(m);
	
	return NULL;
}

/*!
 * @brief constructor of struct m_node
 * @param node
 * @param key
 * @param val
 */
void mNodeCtor(struct m_node *node, const char *key, int val) {
	assert(node);
	assert(key);
	
	node->next = NULL;
	
	int len = strlen(key);
	node->key = (char *)calloc(len + 1, sizeof(char));
	strcpy(node->key, key);
	
	node->val = val;
}

/*!
 * @brief destructor of struct m_node
 * @param node
 * @param key
 * @param val
 */
void mNodeDtor(struct m_node *node) {
	assert(node);
	
	node->next = NULL;
	
	free(node->key);
	
	node->val = 0;
}

/*!
 *
 * @param node
 * @return pointer to the next node
 */
struct m_node *mNodeNext(struct m_node *node) {
	assert(node);
		
	return node->next;
}

/*!
 *
 * @param node
 * @return key of current node
 */
char *mNodeKey(struct m_node *node) {
	assert(node);
		
	return node->key;
}

/*!
 *
 * @param node
 * @return val of current node
 */
int mNodeVal(struct m_node *node) {
	assert(node);
		
	return node->val;
}