#include "CPU.h"
#include <textProcessor.h>
#include <stack.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

enum CPUError {
	CPU_NO_ERROR,
	STACK_ERROR,
	CPU_ALLOCATION_ERROR,
	ADDRESS_OUT_OF_MEMORY,
	UNKNOWN_OPCODE,
	IP_OUT_OF_PROG,
	MISSING_PROGRAM
};

struct CPU {
	stack st;
	int *r; //registers
	int *mem; //ram
	unsigned int ip; //instuction pointer
	char *program;
	unsigned int programSize;
	int errno;
};

void CPUCtor(struct CPU *cpu) {
	assert(cpu);

	cpu->errno = CPU_NO_ERROR;
	
//	cpu->st = {};
	stackCtor(&(cpu->st));
	if (!stackOk(&(cpu->st)))
		cpu->errno = STACK_ERROR;
	
	cpu->r = (int *)calloc(REGISTERS_COUNT, sizeof(int));
	cpu->mem = (int *)calloc(MEM_SIZE, sizeof(int));
	if (!(cpu->r && cpu->mem))
		cpu->errno = CPU_ALLOCATION_ERROR;
	
	cpu->ip = 0;
	cpu->program = NULL;
	cpu->programSize = 0;
}

void CPUDtor(struct CPU *cpu) {
	assert(cpu);
	
	stackDtor(&(cpu->st));
	
	free(cpu->r);
	free(cpu->mem);
	
	cpu->ip = 0;
	
	free(cpu->program);
	cpu->programSize = 0;
	
	cpu->errno = CPU_NO_ERROR;
}

int CPUOk(struct CPU *cpu) {
	assert(cpu);
	
	if (cpu->errno)
		return 0;
	
	if (!stackOk(&(cpu->st))) {
		cpu->errno = STACK_ERROR;
		return 0;
	}
	
	if (cpu->ip > cpu->programSize) {
		cpu->errno = IP_OUT_OF_PROG;
		return 0;
	}
	
	return cpu->r && cpu->mem;
}

void CPUDump(struct CPU *cpu) {
	printf("CPU [%p] {\n", cpu);
	
	stackDump(&(cpu->st));
	
	printf("r[%d]:[%p] {\n", REGISTERS_COUNT, cpu->r);
	for (int i = 0; i < REGISTERS_COUNT; i++)
		printf("\t[%d] = %d\n", i, cpu->r[i]);
	printf("}\n");
	
	printf("mem:[%p] {\n", cpu->mem);
	for (int i = 0; i < MEM_SIZE; i += 4) {
		printf("%03x:", i);
		for (int j = 0; j < 4; j++)
			printf("%08x ", cpu->mem[i + j]);
		printf("\n");
	}
	printf("}\n");
	
	printf("program[%d]:[%p] {\n", cpu->programSize, cpu->program);
	for (unsigned int i = 0; i < cpu->programSize; i += 16) {
		printf("%08x:", i);
		for (int j = 0; j < 16 && (i + j < cpu->programSize); j++)
			printf("%02x ", (unsigned char)cpu->program[i + j]);
		printf("\n");
	}
	printf("}\n");
	
	printf("ip = %08x\n", cpu->ip);
	
	printf("errno = %d\n", cpu->errno);
}

int CPULoadProgramFromFile(struct CPU *cpu, char *file) {
	assert(cpu);
	assert(file);
	
	int size = sizeofFile(file);
	if (size < 0) {
		cpu->errno = MISSING_PROGRAM;
		return 0;
	}
	
	cpu->programSize = size + 1;
	cpu->program = (char *)calloc(cpu->programSize, sizeof(char));
	if (cpu->program == NULL) {
		cpu->errno = CPU_ALLOCATION_ERROR;
		return 0;
	}
	
	FILE *input = fopen(file, "rb");
	fread(cpu->program, sizeof(char), cpu->programSize - 1, input);
	fclose(input);
	return 1;
}

int CPURunProgram(struct CPU *cpu) {
	assert(cpu);
	
	if (cpu->program == NULL || !CPUOk(cpu)) 
		return 0;
	
	cpu->ip = 0;
	while (cpu->program[cpu->ip]) {
		int a = 0, b = 0, addr = 0;
		switch(cpu->program[cpu->ip]) {
		case PUSH:
			a = *((int *) &cpu->program[cpu->ip + 1]);
			stackPush(&cpu->st, a);
			cpu->ip += 1 + sizeof(int);
			break;
		case PUSHR:
			a = *((int *) &cpu->program[cpu->ip + 1]);
			if (a >= REGISTERS_COUNT) {
				cpu->errno = ADDRESS_OUT_OF_MEMORY;
				return 0;
			}
			stackPush(&cpu->st, cpu->r[a]);
			cpu->ip += 1 + sizeof(int);
			break;
		case PUSHMEM:
			a = *((int *) &cpu->program[cpu->ip + 1]);
			if (a >= MEM_SIZE) {
				cpu->errno = ADDRESS_OUT_OF_MEMORY;
				return 0;
			}
			stackPush(&cpu->st, cpu->mem[a]);
			sleep(1);
			cpu->ip += 1 + sizeof(int);
			break;
		case PUSHINDIR:
			a = *((int *) &cpu->program[cpu->ip + 1]);
			if (a >= REGISTERS_COUNT || cpu->r[a] >= MEM_SIZE) {
				cpu->errno = ADDRESS_OUT_OF_MEMORY;
				return 0;
			}
			stackPush(&cpu->st, cpu->mem[cpu->r[a]]);
			sleep(1);
			cpu->ip += 1 + sizeof(int);
			break;
		case PUSHINDIROFFSET:
			a = *((int *) &cpu->program[cpu->ip + 1]);
			b = *((int *) &cpu->program[cpu->ip + 1 + sizeof(int)]);
			if (a >= REGISTERS_COUNT || cpu->r[a] + b >= MEM_SIZE) {
				cpu->errno = ADDRESS_OUT_OF_MEMORY;
				return 0;
			}
			stackPush(&cpu->st, cpu->mem[cpu->r[a] + b]);
			sleep(1);
			cpu->ip += 1 + 2 * sizeof(int);
			break;

		case POP:
			stackPop(&cpu->st);
			cpu->ip++;
			break;
		case POPR:
			a = *((int *) &cpu->program[cpu->ip + 1]);
			if (a >= REGISTERS_COUNT) {
				cpu->errno = ADDRESS_OUT_OF_MEMORY;
				return 0;
			}
			cpu->r[a] = stackPop(&cpu->st);
			cpu->ip += 1 + sizeof(int);
			break;
		case POPMEM:
			a = *((int *) &cpu->program[cpu->ip + 1]);
			if (a >= MEM_SIZE) {
				cpu->errno = ADDRESS_OUT_OF_MEMORY;
				return 0;
			}
			cpu->mem[a] = stackPop(&cpu->st);
			sleep(1);
			cpu->ip += 1 + sizeof(int);
			break;
		case POPINDIR:
			a = *((int *) &cpu->program[cpu->ip + 1]);
			if (a >= REGISTERS_COUNT || cpu->r[a] >= MEM_SIZE) {
				cpu->errno = ADDRESS_OUT_OF_MEMORY;
				return 0;
			}
			cpu->mem[cpu->r[a]] = stackPop(&cpu->st);
			sleep(1);
			cpu->ip += 1 + sizeof(int);
			break;
		case POPINDIROFFSET:
			a = *((int *) &cpu->program[cpu->ip + 1]);
			b = *((int *) &cpu->program[cpu->ip + 1 + sizeof(int)]);
			if (a >= REGISTERS_COUNT || cpu->r[a] + b >= MEM_SIZE) {
				cpu->errno = ADDRESS_OUT_OF_MEMORY;
				return 0;
			}
			cpu->mem[cpu->r[a] + b] = stackPop(&cpu->st);
			sleep(1);
			cpu->ip += 1 + 2 * sizeof(int);
			break;

		case ADD:
			a = stackPop(&cpu->st);
			b = stackPop(&cpu->st);
			stackPush(&cpu->st, a + b);
			cpu->ip++;
			break;
		case SUB:
			a = stackPop(&cpu->st);
			b = stackPop(&cpu->st);
			stackPush(&cpu->st, b - a);
			cpu->ip++;
			break;
		case MUL:
			a = stackPop(&cpu->st);
			b = stackPop(&cpu->st);
			stackPush(&cpu->st, a * b);
			cpu->ip++;
			break;
		case DIV:
			a = stackPop(&cpu->st);
			b = stackPop(&cpu->st);
			stackPush(&cpu->st, b / a);
			cpu->ip++;
			break;
		case SQRT:
			stackPush(&cpu->st, (int) sqrt(stackPop(&cpu->st)));
			cpu->ip++;
			break;

		case IN:
			a = 0;
			scanf("%d", &a);
			stackPush(&cpu->st, a);
			cpu->ip++;
			break;
		case OUT:
			printf("%d\n", stackPop(&cpu->st));
//			CPUDump(cpu);
			cpu->ip++;
			break;

		case JMP:
			addr = *((int *) &cpu->program[cpu->ip + 1]);
			if (addr >= cpu->programSize) {
				cpu->errno = IP_OUT_OF_PROG;
				return 0;
			}
			cpu->ip = addr;
			break;
		case JA:
			addr = *((int *) &cpu->program[cpu->ip + 1]);
			if (addr >= cpu->programSize) {
				cpu->errno = IP_OUT_OF_PROG;
				return 0;
			}
			a = stackPop(&cpu->st);
			b = stackPop(&cpu->st);
			if (b > a)
				cpu->ip = addr;
			else
				cpu->ip += 1 + sizeof(int);
			break;
		case JB:
			addr = *((int *) &cpu->program[cpu->ip + 1]);
			if (addr >= cpu->programSize) {
				cpu->errno = IP_OUT_OF_PROG;
				return 0;
			}
			a = stackPop(&cpu->st);
			b = stackPop(&cpu->st);
			if (b < a)
				cpu->ip = addr;
			else
				cpu->ip += 1 + sizeof(int);
			break;
		case JE:
			addr = *((int *) &cpu->program[cpu->ip + 1]);
			if (addr >= cpu->programSize) {
				cpu->errno = IP_OUT_OF_PROG;
				return 0;
			}
			a = stackPop(&cpu->st);
			b = stackPop(&cpu->st);
			if (b == a)
				cpu->ip = addr;
			else
				cpu->ip += 1 + sizeof(int);
			break;

		default:
			cpu->errno = UNKNOWN_OPCODE;
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("missing file\n");
		printf("using format: CPU file\n");
		return 0;
	}
	struct CPU cpu = {};
	CPUCtor(&cpu);
	CPULoadProgramFromFile(&cpu, argv[1]);
	CPURunProgram(&cpu);
	CPUDump(&cpu);
	CPUDtor(&cpu);
}