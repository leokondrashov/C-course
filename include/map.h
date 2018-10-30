#ifndef MAP_H

#define MAP_H

typedef struct map map;

enum mapError {
	MAP_NO_ERROR,
	MAP_ALLOCATION_ERROR,
	MAP_NO_SUCH_ELEMENT
};

struct map {
	struct m_node *head;
	unsigned int size;
	int errno;
};

struct m_node {
	struct m_node *next;
	char *key;
	int val;
};

void mapCtor(map *m);
void mapDtor(map *m);

int mapOk(map *m);

void mapDump(map *m);

int mapAdd(map *m, char *key, int val);
int mapGet(map *m, char *key);
int mapRemove(map *m, char *key);
int mapFind(map *m, char *key);

unsigned int mapSize(map *m);
int mapErrno(map *m);
void mapResetErrno(map *m);

struct m_node *mapBegin(map *m);
struct m_node *mapEnd(map *m);

void mNodeCtor(struct m_node *node, char *key, int val);
void mNodeDtor(struct m_node *node);
struct m_node *mNodeNext(struct m_node *node);
char *mNodeKey(struct m_node *node);
int mNodeVal(struct m_node *node);

#endif