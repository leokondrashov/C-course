#include "CPU.h"
#include <textProcessor.h>
#include <map.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 100

map labels;

int error = 0;

void processSource(const char *infile, const char *lstfile, const char *outfile);
void compile(const char **source, int nLines, const char *lstfile, const char *outfile);

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("missing file\n");
		printf("using format: asm file [-l listing_file] [-o output_file]\n");
		return 0;
	}

	char *infile = NULL, *lstfile = NULL, *outfile = NULL;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'l' && lstfile == NULL) {
				lstfile = argv[++i];
			} else if (argv[i][1] == 'o' && outfile == NULL) {
				outfile = argv[++i];
			} else {
				printf("Unknown flag: %s", argv[i]);
				return 0;
			}
		} else if (infile == NULL) {
			infile = argv[i];
		} else {
			printf("using format: asm file [-l listing_file] [-o output_file]\n");
			return 0;
		}
	}

	if (infile == NULL) {
		printf("missing file\n");
		printf("using format: asm file [-l listing_file] [-o output_file]\n");
		return 0;
	}

	if (lstfile == NULL) {
		lstfile = "a.out.lst";
	}

	if (outfile == NULL) {
		outfile = "a.out";
	}

	processSource(infile, lstfile, outfile);
}

void writeLabels(FILE *lst) {
	assert(lst);
	
	struct m_node *cur = mapBegin(&labels);
	if (cur == NULL)
		return;
	fprintf(lst, "labels:\n");
	while (cur) {
		fprintf(lst, "\t%s: %08x\n", mNodeKey(cur), mNodeVal(cur));
		cur = mNodeNext(cur);
	}
}

void processSource(const char *infile, const char *lstfile, const char *outfile) {
	assert(infile);
	assert(lstfile);
	assert(outfile);

	char **source = readTextFromFile(infile);
	if (source == NULL) {
		printf("Error reading source file: %s", infile);
		exit(1);
	}
	int nLines = countLines(source);
	
	mapCtor(&labels);

	compile(source, nLines, lstfile, outfile);
	if (!error)
		compile(source, nLines, lstfile, outfile); // for correct labels

	if (error) {
		remove(lstfile);
		remove(outfile);
	}

	mapDtor(&labels);
}

int isLabel(const char *line) {
	assert(line);

	char *tmp = (char *)calloc(1, sizeof(char));
	int val = sscanf(line, "%[a-zA-Z_0-9]%[:]", tmp, tmp);
	free(tmp);
	return val == 2;
}

int getLabelAddress(const char *line) {
	assert(line);
	
	mapResetErrno(&labels);
	int addr = mapGet(&labels, line);
	if (mapErrno(&labels) != 0)
		return -1;
	return addr;
}

void addLabel(char *line, int val) {
	assert(line);
	
	char *str = (char *)calloc(10, sizeof(char));
	sscanf(line, "%[a-zA-Z_0-9]%*[:]", str);
	mapAdd(&labels, str, val);
	free(str);
}

void compile(const char **source, int nLines, const char *lstfile, const char *outfile) {
	assert(source);
	assert(lstfile);
	assert(outfile);

	int pc = 0; // program counter
	FILE *lst = fopen(lstfile, "wb");
	FILE *out = fopen(outfile, "wb");
	char *combuff = (char *)calloc(5, sizeof(char)), *argbuff = (char *)calloc(100, sizeof(char));

	for (int i = 0; i < nLines; i++) {
		combuff[0] = '\0';
		argbuff[0] = '\0';
		sscanf(source[i], "%s %[^;]", combuff, argbuff);
		if (strlen(source[i]) == 0)
			continue;
		if (strcasecmp(combuff, "end") == 0) {
			fprintf(out, "%c", (char)END);
			fprintf(lst, "%08x: %02x -> END\n", pc, END);
			pc++;
		} else if (strcasecmp(combuff, "push") == 0) {
			if (strlen(argbuff) == 0) {
				printf("missing argument on line %d\n", i);
				error = 1;
				continue;
			}

			int arg1 = 0, arg2 = 0;
			char *tmp = NULL;
			switch (sscanf(argbuff, "[r%d + %d]", &arg1, &arg2)) {
			case 2:
				fprintf(out, "%c", (char)PUSHINDIROFFSET);
				fprintf(lst, "%08x: %02x ", pc, PUSHINDIROFFSET);

				tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));

				*((int *)tmp) = arg1;
				tmp[sizeof(int)] = '\0';
				for (int it = 0; it < sizeof(int); it++) {
					fprintf(lst, "%02x ", (unsigned char)tmp[it]);
					fprintf(out, "%c", tmp[it]);
				}

				*((int *)tmp) = arg2;
				tmp[sizeof(int)] = '\0';
				for (int it = 0; it < sizeof(int); it++) {
					fprintf(lst, "%02x ", (unsigned char)tmp[it]);
					fprintf(out, "%c", tmp[it]);
				}

				free(tmp);

				fprintf(lst, "-> PUSH [r%d+%d]\n", arg1, arg2);
				pc += 1 + 2 * sizeof(int);
				break;
			case 1:
				fprintf(out, "%c", (char)PUSHINDIR);
				fprintf(lst, "%08x: %02x ", pc, PUSHINDIR);

				tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));

				*((int *)tmp) = arg1;
				tmp[sizeof(int)] = '\0';
				for (int it = 0; it < sizeof(int); it++) {
					fprintf(lst, "%02x ", (unsigned char)tmp[it]);
					fprintf(out, "%c", tmp[it]);
				}

				free(tmp);

				fprintf(lst, "-> PUSH [r%d]\n", arg1);
				pc += 1 + sizeof(int);
				break;
			case 0:
			default:
				if (sscanf(argbuff, "r%d", &arg1)) {
					fprintf(out, "%c", (char)PUSHR);
					fprintf(lst, "%08x: %02x ", pc, PUSHR);

					tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));

					*((int *)tmp) = arg1;
					tmp[sizeof(int)] = '\0';
					for (int it = 0; it < sizeof(int); it++) {
						fprintf(lst, "%02x ", (unsigned char)tmp[it]);
						fprintf(out, "%c", tmp[it]);
					}

					free(tmp);

					fprintf(lst, "-> PUSH r%d\n", arg1);
					pc += 1 + sizeof(int);
				} else if (sscanf(argbuff, "[%d]", &arg1)) {
					fprintf(out, "%c", (char)PUSHMEM);
					fprintf(lst, "%08x: %02x ", pc, PUSHMEM);

					tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));

					*((int *)tmp) = arg1;
					tmp[sizeof(int)] = '\0';
					for (int it = 0; it < sizeof(int); it++) {
						fprintf(lst, "%02x ", (unsigned char)tmp[it]);
						fprintf(out, "%c", tmp[it]);
					}

					free(tmp);

					fprintf(lst, "-> PUSH [%d]\n", arg1);
					pc += 1 + sizeof(int);
				} else if (sscanf(argbuff, "%d", &arg1)) {
					fprintf(out, "%c", (char)PUSH);
					fprintf(lst, "%08x: %02x ", pc, PUSH);

					tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));

					*((int *)tmp) = arg1;
					tmp[sizeof(int)] = '\0';
					for (int it = 0; it < sizeof(int); it++) {
						fprintf(lst, "%02x ", (unsigned char)tmp[it]);
						fprintf(out, "%c", tmp[it]);
					}

					free(tmp);

					fprintf(lst, "-> PUSH %d\n", arg1);
					pc += 1 + sizeof(int);
				} else {
					printf("Unknown args on line %d\n", i);
					error = 1;
				}
			}
		} else if (strcasecmp(combuff, "pop") == 0) {
			if (strlen(argbuff) == 0) {
				fprintf(out, "%c", (char)POP);
				fprintf(lst, "%08x: %02x -> POP\n", pc, POP);
				pc++;
				continue;
			}

			int arg1 = 0, arg2 = 0;
			char *tmp = NULL;
			switch (sscanf(argbuff, "[r%d + %d]", &arg1, &arg2)) {
			case 2:
				fprintf(out, "%c", (char)POPINDIROFFSET);
				fprintf(lst, "%08x: %02x ", pc, POPINDIROFFSET);

				tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));

				*((int *)tmp) = arg1;
				tmp[sizeof(int)] = '\0';
				for (int it = 0; it < sizeof(int); it++) {
					fprintf(lst, "%02x ", (unsigned char)tmp[it]);
					fprintf(out, "%c", tmp[it]);
				}

				*((int *)tmp) = arg2;
				tmp[sizeof(int)] = '\0';
				for (int it = 0; it < sizeof(int); it++) {
					fprintf(lst, "%02x ", (unsigned char)tmp[it]);
					fprintf(out, "%c", tmp[it]);
				}

				free(tmp);

				fprintf(lst, "-> POP [r%d+%d]\n", arg1, arg2);
				pc += 1 + 2 * sizeof(int);
				break;
			case 1:
				fprintf(out, "%c", (char)POPINDIR);
				fprintf(lst, "%08x: %02x ", pc, POPINDIR);

				tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));

				*((int *)tmp) = arg1;
				tmp[sizeof(int)] = '\0';
				for (int it = 0; it < sizeof(int); it++) {
					fprintf(lst, "%02x ", (unsigned char)tmp[it]);
					fprintf(out, "%c", tmp[it]);
				}

				free(tmp);

				fprintf(lst, "-> POP [r%d]\n", arg1);
				pc += 1 + sizeof(int);
				break;
			case 0:
			default:
				if (sscanf(argbuff, "r%d", &arg1)) {
					fprintf(out, "%c", (char)POPR);
					fprintf(lst, "%08x: %02x ", pc, POPR);

					tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));

					*((int *)tmp) = arg1;
					tmp[sizeof(int)] = '\0';
					for (int it = 0; it < sizeof(int); it++) {
						fprintf(lst, "%02x ", (unsigned char)tmp[it]);
						fprintf(out, "%c", tmp[it]);
					}

					free(tmp);

					fprintf(lst, "-> POP r%d\n", arg1);
					pc += 1 + sizeof(int);
				} else if (sscanf(argbuff, "[%d]", &arg1)) {
					fprintf(out, "%c", (char)POPMEM);
					fprintf(lst, "%08x: %02x ", pc, POPMEM);

					tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));

					*((int *)tmp) = arg1;
					tmp[sizeof(int)] = '\0';
					for (int it = 0; it < sizeof(int); it++) {
						fprintf(lst, "%02x ", (unsigned char)tmp[it]);
						fprintf(out, "%c", tmp[it]);
					}

					free(tmp);

					fprintf(lst, "-> POP [%d]\n", arg1);
					pc += 1 + sizeof(int);
				} else {
					printf("Unknown args on line %d\n", i);
					error = 1;
				}
			}
		} else if (strcasecmp(combuff, "add") == 0) {
			fprintf(out, "%c", (char)ADD);
			fprintf(lst, "%08x: %02x -> ADD\n", pc, ADD);
			pc++;
		} else if (strcasecmp(combuff, "sub") == 0) {
			fprintf(out, "%c", (char)SUB);
			fprintf(lst, "%08x: %02x -> SUB\n", pc, SUB);
			pc++;
		} else if (strcasecmp(combuff, "mul") == 0) {
			fprintf(out, "%c", (char)MUL);
			fprintf(lst, "%08x: %02x -> MUL\n", pc, MUL);
			pc++;
		} else if (strcasecmp(combuff, "div") == 0) {
			fprintf(out, "%c", (char)DIV);
			fprintf(lst, "%08x: %02x -> DIV\n", pc, DIV);
			pc++;
		} else if (strcasecmp(combuff, "sqrt") == 0) {
			fprintf(out, "%c", (char)SQRT);
			fprintf(lst, "%08x: %02x -> SQRT\n", pc, SQRT);
			pc++;
		} else if (strcasecmp(combuff, "in") == 0) {
			fprintf(out, "%c", (char)IN);
			fprintf(lst, "%08x: %02x -> IN\n", pc, IN);
			pc++;
		} else if (strcasecmp(combuff, "out") == 0) {
			fprintf(out, "%c", (char)OUT);
			fprintf(lst, "%08x: %02x -> OUT\n", pc, OUT);
			pc++;
		} else if (strcasecmp(combuff, "jmp") == 0) {
			if (strlen(argbuff) == 0) {
				printf("missing argument on line %d\n", i);
				error = 1;
				continue;
			}

			int addr = 0;
			if (!sscanf(argbuff, "%d", &addr)) { // if nothing converted
				addr = getLabelAddress(argbuff);
			}
			char *tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));
			*((int *)tmp) = addr; // writing addr in binary
			tmp[sizeof(int)] = '\0';

			fprintf(out, "%c", (char)JMP);
			fprintf(lst, "%08x: %02x ", pc, JMP);
			for (int it = 0; it < sizeof(int); it++) {
				fprintf(lst, "%02x ", (unsigned char)tmp[it]);
				fprintf(out, "%c", tmp[it]);
			}
			fprintf(lst, "-> JMP %08x\n", addr);

			pc += 1 + sizeof(int);
			free(tmp);
		} else if (strcasecmp(combuff, "ja") == 0) {
			if (strlen(argbuff) == 0) {
				printf("missing argument on line %d\n", i);
				error = 1;
				continue;
			}

			int addr = 0;
			if (!sscanf(argbuff, "%d", &addr)) { // if nothing converted
				addr = getLabelAddress(argbuff);
			}
			char *tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));
			*((int *)tmp) = addr; // writing addr in binary
			tmp[sizeof(int)] = '\0';

			fprintf(out, "%c", (char)JA);
			fprintf(lst, "%08x: %02x ", pc, JA);
			for (int it = 0; it < sizeof(int); it++) {
				fprintf(lst, "%02x ", (unsigned char)tmp[it]);
				fprintf(out, "%c", tmp[it]);
			}

			fprintf(lst, "-> JA %08x\n", addr);

			pc += 1 + sizeof(int);
			free(tmp);
		} else if (strcasecmp(combuff, "jb") == 0) {
			if (strlen(argbuff) == 0) {
				printf("missing argument on line %d\n", i);
				error = 1;
				continue;
			}

			int addr = 0;
			if (!sscanf(argbuff, "%d", &addr)) { // if nothing converted
				addr = getLabelAddress(argbuff);
			}
			char *tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));
			*((int *)tmp) = addr; // writing addr in binary
			tmp[sizeof(int)] = '\0';

			fprintf(out, "%c", (char)JB);
			fprintf(lst, "%08x: %02x ", pc, JB);
			for (int it = 0; it < sizeof(int); it++) {
				fprintf(lst, "%02x ", (unsigned char)tmp[it]);
				fprintf(out, "%c", tmp[it]);
			}
			fprintf(lst, "-> JB %08x\n", addr);

			pc += 1 + sizeof(int);
			free(tmp);
		} else if (strcasecmp(combuff, "je") == 0) {
			if (strlen(argbuff) == 0) {
				printf("missing argument on line %d\n", i);
				error = 1;
				continue;
			}

			int addr = 0;
			if (!sscanf(argbuff, "%d", &addr)) { // if nothing converted
				addr = getLabelAddress(argbuff);
			}
			char *tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));
			*((int *)tmp) = addr; // writing addr in binary
			tmp[sizeof(int)] = '\0';

			fprintf(out, "%c", (char)JE);
			fprintf(lst, "%08x: %02x ", pc, JE);
			for (int it = 0; it < sizeof(int); it++) {
				fprintf(lst, "%02x ", (unsigned char)tmp[it]);
				fprintf(out, "%c", tmp[it]);
			}
			fprintf(lst, "-> JE %08x\n", addr);

			pc += 1 + sizeof(int);
			free(tmp);
		} else if (strcasecmp(combuff, "jne") == 0) {
			if (strlen(argbuff) == 0) {
				printf("missing argument on line %d\n", i);
				error = 1;
				continue;
			}

			int addr = 0;
			if (!sscanf(argbuff, "%d", &addr)) { // if nothing converted
				addr = getLabelAddress(argbuff);
			}
			char *tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));
			*((int *)tmp) = addr; // writing addr in binary
			tmp[sizeof(int)] = '\0';

			fprintf(out, "%c", (char)JNE);
			fprintf(lst, "%08x: %02x ", pc, JNE);
			for (int it = 0; it < sizeof(int); it++) {
				fprintf(lst, "%02x ", (unsigned char)tmp[it]);
				fprintf(out, "%c", tmp[it]);
			}
			fprintf(lst, "-> JNE %08x\n", addr);

			pc += 1 + sizeof(int);
			free(tmp);
		} else if (strcasecmp(combuff, "call") == 0) {
			if (strlen(argbuff) == 0) {
				printf("missing argument on line %d\n", i);
				error = 1;
				continue;
			}

			int addr = 0;
			if (!sscanf(argbuff, "%d", &addr)) { // if nothing converted
				addr = getLabelAddress(argbuff);
			}
			char *tmp = (char *)calloc(sizeof(int) + 1, sizeof(char));
			*((int *)tmp) = addr; // writing addr in binary
			tmp[sizeof(int)] = '\0';

			fprintf(out, "%c", (char)CALL);
			fprintf(lst, "%08x: %02x ", pc, CALL);
			for (int it = 0; it < sizeof(int); it++) {
				fprintf(lst, "%02x ", (unsigned char)tmp[it]);
				fprintf(out, "%c", tmp[it]);
			}
			fprintf(lst, "-> CALL %08x\n", addr);

			pc += 1 + sizeof(int);
			free(tmp);
		} else if (strcasecmp(combuff, "ret") == 0) {
			fprintf(out, "%c", (char)RET);
			fprintf(lst, "%08x: %02x -> RET\n", pc, RET);
			pc++;
		} else if (isLabel(source[i])) {
				addLabel(source[i], pc);
		} else {
			printf("Unknown command on line %d: %s\n", i, combuff);
			error = 1;
		}
	}

	writeLabels(lst);

	free(combuff);
	free(argbuff);
	fclose(out);
	fclose(lst);
}
