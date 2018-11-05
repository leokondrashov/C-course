#include <stdio.h>
#include <assert.h>
#include <textProcessor.h>
#include "CPU.h"

void processFile(const char *infile, const char *outfile);

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("missing file\n");
		printf("using format: disasm file [-l listing_file] [-o output_file]\n");
		return 0;
	}

	char *infile = NULL, *outfile = NULL;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'o' && outfile == NULL) {
				outfile = argv[++i];
			} else {
				printf("Unknown flag: %s", argv[i]);
				return 0;
			}
		} else if (infile == NULL) {
			infile = argv[i];
		} else {
			printf("using format: disasm file [-l listing_file] [-o output_file]\n");
			return 0;
		}
	}

	if (infile == NULL) {
		printf("missing file\n");
		printf("using format: disasm file [-o output_file]\n");
		return 0;
	}

	if (outfile == NULL) {
		outfile = "a.asm";
	}

	processFile(infile, outfile);
}

/*!
 * @brief prints argc arguments from argv to outfile
 * @param argc
 * @param argv
 * @param outfile
 */
void writeArgs(int argc, int *argv, FILE *outfile) {
	assert(argv);
	assert(outfile);
	
	for (int i = 0; i < argc; i++) {
		fprintf(outfile, "%d ", argv[i]);
	}
}

/*!
 * @brief disassemble file infile to outfile
 * @param infile
 * @param outfile
 */
void processFile(const char *infile, const char *outfile) {
	assert(infile);
	assert(outfile);

	int fileSize = sizeofFile(infile);
	if (fileSize < 0) {
		printf("Error reading file %s", infile);
		return;
	}
	char *binaryCode = (char *)calloc(fileSize, sizeof(char));
	FILE *in = fopen(infile, "rb");
	fread(binaryCode, sizeof(char), fileSize, in);
	fclose(in);

	FILE *out = fopen(outfile, "wb");

	int ip = 0;
	while (ip < fileSize) {
		switch (binaryCode[ip]) {
		
		#define DEF_CMD(name, code, argc, instr) \
		case CMD_##name: \
			fprintf(out, "%s ", #name); \
			writeArgs(argc, (int *)(binaryCode + ip + 1), out); \
			fprintf(out, "\n"); \
			ip += 1 + (argc) * sizeof(int); \
			break;
		#include "commands.h"
		#undef DEF_CMD
		
		default:
			return;
		}
	}
	fclose(out);
}