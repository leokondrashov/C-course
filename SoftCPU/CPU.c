#include "CPU.h"
#include <textProcessor.h>
#include <stack.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#define BYTES_IN_LINE 16
#define INTS_IN_LINE 4

#define POP_ stackPop(&cpu->st)
#define PUSH_(val) stackPush(&cpu->st, val)

#define GET(name) {\
	(name) = *((int *) &cpu->program[cpu->ip]); \
	cpu->ip += sizeof(int); \
}

enum CPUError {
	CPU_NO_ERROR,
	CPU_STACK_ERROR,
	CPU_ALLOCATION_ERROR,
	CPU_ADDRESS_OUT_OF_MEMORY,
	CPU_UNKNOWN_OPCODE,
	CPU_IP_OUT_OF_PROG,
	CPU_MISSING_PROGRAM
};

struct CPU {
	stack st;
	stack callStack;
	int *r; //registers
	int *mem; //ram
	unsigned int ip; //instuction pointer
	char *program;
	unsigned int programSize;
	int errno;
};
/*!
 * @brief constructor of struct CPU
 * @param cpu
 */
void CPUCtor(struct CPU *cpu) {
	assert(cpu);
	
	cpu->errno = CPU_NO_ERROR;
	
	stackCtor(&(cpu->st));
	if (!stackOk(&(cpu->st)))
		cpu->errno = CPU_STACK_ERROR;
	
	stackCtor(&(cpu->callStack));
	if (!stackOk(&(cpu->callStack)))
		cpu->errno = CPU_STACK_ERROR;
	
	cpu->r = (int *) calloc(REGISTERS_COUNT, sizeof(int));
	cpu->mem = (int *) calloc(MEM_SIZE, sizeof(int));
	if (cpu->r == NULL || cpu->mem == NULL)
		cpu->errno = CPU_ALLOCATION_ERROR;
	
	cpu->ip = 0;
	cpu->program = NULL;
	cpu->programSize = 0;
}

/*!
 * @brief destructor of struct CPU
 * @param cpu
 */
void CPUDtor(struct CPU *cpu) {
	assert(cpu);
	
	stackDtor(&(cpu->st));
	stackDtor(&(cpu->callStack));
	
	free(cpu->r);
	free(cpu->mem);
	
	cpu->ip = 0;
	
	free(cpu->program);
	cpu->programSize = 0;
	
	cpu->errno = CPU_NO_ERROR;
}

/*!
 * @brief checks if cpu is Ok
 * @param cpu
 * @return 1 if cpu is Ok, 0 otherwise
 */
int CPUOk(struct CPU *cpu) {
	if (cpu == NULL)
		return 0;
	
	if (cpu->errno)
		return 0;
	
	if (!stackOk(&(cpu->st))) {
		cpu->errno = CPU_STACK_ERROR;
		return 0;
	}
	
	if (!stackOk(&(cpu->callStack))) {
		cpu->errno = CPU_STACK_ERROR;
		return 0;
	}
	
	if (cpu->ip > cpu->programSize) {
		cpu->errno = CPU_IP_OUT_OF_PROG;
		return 0;
	}
	
	if (cpu->r == NULL || cpu->mem == NULL)
		return 0;
	
	return 1;
}

/*!
 * @brief debug info for cpu
 * @param cpu
 */
void CPUDump(struct CPU *cpu) {
	printf("CPU [%p] {\n", cpu);
	
	printf("errno = %d\n", cpu->errno);
	
	printf("Stack :\n");
	stackDump(&(cpu->st));
	
	printf("Call stack:\n");
	stackDump(&(cpu->callStack));
	
	printf("r[%d]:[%p] {\n", REGISTERS_COUNT, cpu->r);
	for (int i = 0; i < REGISTERS_COUNT; i++)
		printf("\t[%d] = %d\n", i, cpu->r[i]);
	printf("}\n");
	
	printf("mem:[%p] {\n", cpu->mem);
	for (int i = 0; i < MEM_SIZE; i += INTS_IN_LINE) {
		printf("%03x:", i);
		for (int j = 0; j < INTS_IN_LINE; j++)
			printf("%08x ", cpu->mem[i + j]);
		printf("\n");
	}
	printf("}\n");
	
	printf("program[%d]:[%p] {\n", cpu->programSize, cpu->program);
	for (unsigned int i = 0; i < cpu->programSize; i += BYTES_IN_LINE) {
		printf("%08x:", i);
		for (int j = 0; j < BYTES_IN_LINE && (i + j < cpu->programSize); j++)
			printf("%02x ", (unsigned char) cpu->program[i + j]);
		printf("\n");
	}
	printf("ip = %08x\n", cpu->ip);
	
	printf("}\n");
}

/*!
 * @brief loads program to CPU
 * @param cpu
 * @param file
 * @return 1 if loading was successful 0 otherwise
 */
int CPULoadProgramFromFile(struct CPU *cpu, char *file) {
	assert(cpu);
	assert(file);
	
	int size = sizeofFile(file);
	if (size < 0) {
		cpu->errno = CPU_MISSING_PROGRAM;
		return 0;
	}
	
	cpu->programSize = size + 1;
	cpu->program = (char *) calloc(cpu->programSize, sizeof(char));
	if (cpu->program == NULL) {
		cpu->errno = CPU_ALLOCATION_ERROR;
		return 0;
	}
	
	FILE *input = fopen(file, "rb");
	fread(cpu->program, sizeof(char), cpu->programSize - 1, input);
	fclose(input);
	return 1;
}

/*!
 * @brief executes loaded program
 * @param cpu
 * @return 1 if execution was successful, 0 if error occured
 */
int CPURunProgram(struct CPU *cpu) {
	assert(cpu);
	
	if (cpu->program == NULL || !CPUOk(cpu))
		return 0;
	
	cpu->ip = 0;
	int arg1 = 0, arg2 = 0, addr = 0;
	while (cpu->program[cpu->ip]) {
		switch (cpu->program[cpu->ip]) {
		
		#define DEF_CMD(name, code, argc, instr) \
		case CMD_##name: \
			cpu->ip++; \
			instr; \
			break;
		#include "commands.h"
		#undef DEF_CMD
		
		default:
			cpu->errno = CPU_UNKNOWN_OPCODE;
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
//	CPUDump(&cpu);
	CPURunProgram(&cpu);
//	CPUDump(&cpu);
	CPUDtor(&cpu);
}