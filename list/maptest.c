#include <stdio.h>
#include <map.h>
#include <textProcessor.h>
#include <string.h>
#include <stdlib.h>

int main() {
	char **dictionary = readTextFromFile("list/dictfull.txt");
	int entries = countLines(dictionary);
	
	map dict;
	mapCtor(&dict);
	
	for (int i = 0; i < entries && dictionary[i][0] != '\0'; i++) {
		char *position = strchr(dictionary[i], ':');
		*position = '\0';
		mapAdd(&dict, dictionary[i], position + 2);
	}
//	printDistribution(&dict);
	
	FILE *out = fopen("/dev/null", "wb");

//	printf("enter word: ");
//
//	char *buff = calloc(20, sizeof(char));
//	scanf("%s", buff);
//
//	char *response = mapGet(&dict, buff);
//	if (mapErrno(&dict) != MAP_NO_SUCH_ELEMENT) {
//		printf("translation: %s\n", response);
//	} else {
//		printf("no such word in dictionary\n");
//	}
	
	char **queries = readTextFromFile("list/dict.txt");
	
	for (int j = 0; j < 1000; j++) {
		int i = 0;
		while (queries[i][0] != '\0') {
			mapResetErrno(&dict);
			char *response = mapGet(&dict, queries[i]);
			if (mapErrno(&dict) != MAP_NO_SUCH_ELEMENT) {
				fprintf(out, "%s: %s\n", queries[i], response);
			} else {
				fprintf(out, "%s: no such word in dictionary\n", queries[i]);
			}
			i++;
		}
	}
	
	fclose(out);
	mapDtor(&dict);
//	free(buff);
	free(dictionary[0]);
	free(dictionary);
}