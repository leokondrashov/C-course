#include <list.h>
#include <stdio.h>

int data[] = {1, 2, 3, 4, 5};

int main() {
	list l = {};
	listCtor(&l, 10);
	char op = 0;
	int a = 0, b = 0;
	scanf("%c", &op);
	while (op != '0') {
		if (op == '+') {
			scanf("%d %d", &a, &b);
			listInsert(&l, a, data + b);
		} else if (op == '-') {
			scanf("%d", &a);
			listRemove(&l, a);
		} else if (op == 's') {
			listSort(&l);
		} else if (op == 'd') {
			listDump(&l);
		} else if (op == 'c') {
			scanf("%d %d", &a, &b);
			swap(&l, a, b);
		}
		scanf("%c", &op);
	}
}