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
		int a = 0, b = 0;
		switch (binaryCode[ip]) {
		case END:
			fprintf(out, "END\n");
			ip++;
			break;
		case PUSH:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "PUSH %d\n", a);
			ip += 1 + sizeof(int);
			break;
		case PUSHR:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "PUSH r%d\n", a);
			ip += 1 + sizeof(int);
			break;
		case PUSHMEM:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "PUSH [%d]\n", a);
			ip += 1 + sizeof(int);
			break;
		case PUSHINDIR:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "PUSH [r%d]\n", a);
			ip += 1 + sizeof(int);
			break;
		case PUSHINDIROFFSET:
			a = *((int *) &binaryCode[ip + 1]);
			b = *((int *) &binaryCode[ip + 1 + sizeof(int)]);
			fprintf(out, "PUSH [r%d+%d]\n", a, b);
			ip += 1 + 2 * sizeof(int);
			break;

		case POP:
			fprintf(out, "POP\n");
			ip++;
			break;
		case POPR:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "POP r%d\n", a);
			ip += 1 + sizeof(int);
			break;
		case POPMEM:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "POP [%d]\n", a);
			ip += 1 + sizeof(int);
			break;
		case POPINDIR:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "POP [r%d]\n", a);
			ip += 1 + sizeof(int);
			break;
		case POPINDIROFFSET:
			a = *((int *) &binaryCode[ip + 1]);
			b = *((int *) &binaryCode[ip + 1 + sizeof(int)]);;
			fprintf(out, "POP [r%d+%d]\n", a, b);
			ip += 1 + 2 * sizeof(int);
			break;

		case ADD:
			fprintf(out, "ADD\n");
			ip++;
			break;
		case SUB:
			fprintf(out, "SUB\n");
			ip++;
			break;
		case MUL:
			fprintf(out, "MUL\n");
			ip++;
			break;
		case DIV:
			fprintf(out, "DIV\n");
			ip++;
			break;
		case SQRT:
			fprintf(out, "SQRT\n");
			ip++;
			break;

		case IN:
			fprintf(out, "IN\n");
			ip++;
			break;
		case OUT:
			fprintf(out, "OUT\n");
			ip++;
			break;

		case JMP:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "JMP %d\n", a);
			ip += 1 + sizeof(int);
			break;
		case JA:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "JA %d\n", a);
			ip += 1 + sizeof(int);
			break;
		case JB:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "JB %d\n", a);
			ip += 1 + sizeof(int);
			break;
		case JE:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "JE %d\n", a);
			ip += 1 + sizeof(int);
			break;
		case JNE:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "JNE %d\n", a);
			ip += 1 + sizeof(int);
			break;

		case CALL:
			a = *((int *) &binaryCode[ip + 1]);
			fprintf(out, "CALL %d\n", a);
			ip += 1 + sizeof(int);
			break;
		case RET:
			fprintf(out, "RET\n");
			ip++;
			break;
		default:
			return;
		}
	}
	fclose(out);
}