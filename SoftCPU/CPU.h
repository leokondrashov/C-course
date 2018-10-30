#ifndef CPU_H

#define CPU_H

#define REGISTERS_COUNT 16
#define MEM_SIZE 512

enum opcode {
	END = 0,
	PUSH = 1,
	PUSHR = 2,
	PUSHMEM = 3,
	PUSHINDIR = 4,
	PUSHINDIROFFSET = 5,
	POP = 6,
	POPR = 7,
	POPMEM = 8,
	POPINDIR = 9,
	POPINDIROFFSET = 10,
	ADD = 11,
	SUB = 12,
	MUL = 13,
	DIV = 14,
	SQRT = 15,
	IN = 16,
	OUT = 17,
	JMP = 18,
	JA = 19,
	JB = 20,
	JE = 21,
	JNE = 22,
	CALL = 23,
	RET = 24
};

#endif