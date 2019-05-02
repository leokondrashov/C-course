#include "CPU.h"
#include <textProcessor.h>
#include <map.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LABEL_SIZE 16

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

FILE *lstFile = NULL;

void writeLabel(struct l_node *node) {
	fprintf(lstFile, "\t%s: %08x\n", node->key, node->val);
}

/*!
 * @brief write labels to listing file lst
 * @param lst
 */
void writeLabels(FILE *lst) {
	assert(lst);
	
	lstFile = lst;
	
	fprintf(lst, "labels:\n");
	mapIterate(&labels, writeLabel);
	
	lstFile = NULL;
}

/*!
 *
 * @param line
 * @return 1 if line contains label in right format, 0 otherwise
 */
int isLabel(const char *line) {
	assert(line);
	
	char *tmp = (char *) calloc(MAX_LABEL_SIZE, sizeof(char));
	int val = sscanf(line, "%[a-zA-Z_0-9]%[:]", tmp, tmp);
	free(tmp);
	return val == 2;
}

/*!
 *
 * @param line
 * @return address of this label, -1 if no such label present
 */
int getLabelAddress(const char *line) {
	assert(line);
	
	mapResetErrno(&labels);
	int addr = mapGet(&labels, line);
	if (mapErrno(&labels) != 0) {
		mapResetErrno(&labels);
		return -1;
	}
	return addr;
}

/*!
 * @brief adds label to labels
 * @param line name of label
 * @param val address of label line
 */
void addLabel(const char *line, int val) {
	assert(line);
	
	char *str = (char *) calloc(MAX_LABEL_SIZE, sizeof(char));
	sscanf(line, "%[a-zA-Z_0-9]%*[:]", str);
	mapAdd(&labels, str, val);
	free(str);
}

/*!
 * @brief process file infile, writing listing to lstfile, binary to outfile
 * @param infile
 * @param lstfile
 * @param outfile
 */
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

/*!
 * @brief parse arguments from line
 * @param [in] args
 * @param [out] argv
 * @param [in] maxArgc
 * @return number of parsed arguments
 */
int getArgs(const char *args, int *argv, int maxArgc) {
	assert(args);
	assert(argv);
	
	int argc = 0, nextPos = 0, arg = 0;
	char *label = (char *) calloc(MAX_LABEL_SIZE, sizeof(char));
	
	do {
		nextPos = 0;
		if (sscanf(args, "%d%n", &arg, &nextPos) && nextPos != 0) {
			if (argc < maxArgc)
				argv[argc] = arg;
			argc++;
		} else if (sscanf(args, " r%d%n", &arg, &nextPos) && nextPos != 0) {
			if (argc < maxArgc)
				argv[argc] = arg;
			argc++;
		} else if (sscanf(args, " %[a-zA-Z_0-9]%n", label, &nextPos) && nextPos != 0) {
			if (argc < maxArgc)
				argv[argc] = getLabelAddress(label);
			argc++;
		}
		
		args += nextPos;
	} while (nextPos != 0 && args[0] != ';' && args[0] != '\0');
	
	return argc;
}

/*!
 * @brief writes argc arguments from argv in binary form to outfile and lstfile
 * @param argc
 * @param argv
 * @param lstfile listing file
 * @param outfile binary file
 */
void writeBinArgs(int argc, int *argv, FILE *lstfile, FILE *outfile) {
	assert(argv);
	assert(lstfile);
	assert(outfile);
	
	char *chars = (char *) argv;
	for (int i = 0; i < argc * sizeof(int); i++) {
		fprintf(lstfile, "%02x ", (unsigned char) chars[i]);
		fprintf(outfile, "%c", chars[i]);
	}
}

/*!
 * @brief writes argc arguments from argv in hexadecimal form to and lstfile
 * @param argc
 * @param argv
 * @param lstfile listing file
 */
void writeHexArgs(int argc, int *argv, FILE *lstfile) {
	assert(argv);
	assert(lstfile);
	
	for (int i = 0; i < argc; i++) {
		fprintf(lstfile, "%x ", argv[i]);
	}
}

/*!
 * @brief compiles lines from source to listing file lstfile and binary file outfile
 * @param source
 * @param nLines
 * @param lstfile
 * @param outfile
 */
void compile(const char **source, int nLines, const char *lstfile, const char *outfile) {
	assert(source);
	assert(lstfile);
	assert(outfile);
	
	int pc = 0; // program counter
	FILE *lst = fopen(lstfile, "wb");
	FILE *out = fopen(outfile, "wb");
	
	int maxCmdLen = 0, len = 0;
	int maxArgc = 0;
	
	#define DEF_CMD(name, code, argc, instr) \
	len = strlen(#name); \
	if (len > maxCmdLen) \
		maxCmdLen = len; \
	if (argc > maxArgc) \
		maxArgc = argc;
	#include "commands.h"
	#undef DEF_CMD
	
	char *combuff = (char *) calloc(maxCmdLen, sizeof(char));
	int *argbuff = (int *) calloc(maxArgc, sizeof(int));
	int argsPos = 0;
	
	for (int i = 0; i < nLines; i++) {
		combuff[0] = '\0';
		
		sscanf(source[i], "%[^;\n ]%n", combuff, &argsPos);
		if (strlen(source[i]) == 0 || strlen(combuff) == 0)
			continue;
		
		#define DEF_CMD(name, code, argc, instr) \
		if (strcasecmp(combuff, #name) == 0) { \
			if (getArgs(source[i] + argsPos, argbuff, maxArgc) != (argc)) { \
				error = 1; \
				printf("Wrong arguments count on line %d: expected %d arguments, got %d\n", i + 1, argc, getArgs(source[i] + argsPos, argbuff, maxArgc)); \
			} \
			fprintf(out, "%c", (char)(code)); \
			fprintf(lst, "%08x: %02x ", pc, CMD_##name); \
			writeBinArgs((argc), argbuff, lst, out); \
			fprintf(lst, "-> %s ", #name); \
			writeHexArgs((argc), argbuff, lst); \
			fprintf(lst, "\n"); \
			pc += 1 + (argc) * sizeof(int); \
		} else
		#include "commands.h"
		#undef DEF_CMD
		
		if (isLabel(source[i])) {
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
