#ifndef CPU_H

#define CPU_H

#define REGISTERS_COUNT 16
#define MEM_SIZE 512

enum opcode {
	#define DEF_CMD(name, code, argc, instr) \
	CMD_##name = (code),
	#include "commands.h"
	#undef DEF_CMD
	CMD_MAX
};

#endif