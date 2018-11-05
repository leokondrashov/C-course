DEF_CMD(END, 0, 0, {
	return 1;
})

DEF_CMD(PUSH, 1, 1, {
	GET(arg1);
	PUSH_(arg1);
})
DEF_CMD(PUSHR, 2, 1, {
	GET(arg1);
	if (arg1 >= REGISTERS_COUNT) {
		cpu->errno = CPU_ADDRESS_OUT_OF_MEMORY;
		return 0;
	}
	PUSH_(cpu->r[arg1]);
})
DEF_CMD(PUSHM, 3, 1, {
	GET(arg1);
	if (arg1 >= MEM_SIZE) {
		cpu->errno = CPU_ADDRESS_OUT_OF_MEMORY;
		return 0;
	}
	sleep(1);
	PUSH_(cpu->mem[arg1]);
})
DEF_CMD(PUSHI, 4, 1, {
	GET(arg1);
	if (arg1 >= REGISTERS_COUNT || cpu->r[arg1] >= MEM_SIZE) {
		cpu->errno = CPU_ADDRESS_OUT_OF_MEMORY;
		return 0;
	}
	sleep(1);
	PUSH_(cpu->mem[cpu->r[arg1]]);
})
DEF_CMD(PUSHIOFFSET, 5, 2, {
	GET(arg1);
	GET(arg2);
	if (arg1 >= REGISTERS_COUNT || cpu->r[arg1] + arg2 >= MEM_SIZE) {
		cpu->errno = CPU_ADDRESS_OUT_OF_MEMORY;
		return 0;
	}
	sleep(1);
	PUSH_(cpu->mem[cpu->r[arg1] + arg2]);
})

DEF_CMD(POP, 6, 0, {
	POP_;
})
DEF_CMD(POPR, 7, 1, {
	GET(arg1);
	if (arg1 >= REGISTERS_COUNT) {
		cpu->errno = CPU_ADDRESS_OUT_OF_MEMORY;
		return 0;
	}
	cpu->r[arg1] = POP_;
})
DEF_CMD(POPM, 8, 1, {
	GET(arg1);
	if (arg1 >= MEM_SIZE) {
		cpu->errno = CPU_ADDRESS_OUT_OF_MEMORY;
		return 0;
	}
	sleep(1);
	cpu->mem[arg1] = POP_;
})
DEF_CMD(POPI, 9, 1, {
	GET(arg1);
	if (arg1 >= REGISTERS_COUNT || cpu->r[arg1] >= MEM_SIZE) {
		cpu->errno = CPU_ADDRESS_OUT_OF_MEMORY;
		return 0;
	}
	sleep(1);
	cpu->mem[cpu->r[arg1]] = POP_;
})
DEF_CMD(POPIOFFSET, 10, 2, {
	GET(arg1);
	GET(arg2);
	if (arg1 >= REGISTERS_COUNT || cpu->r[arg1] + arg2 >= MEM_SIZE) {
		cpu->errno = CPU_ADDRESS_OUT_OF_MEMORY;
		return 0;
	}
	sleep(1);
	cpu->mem[cpu->r[arg1] + arg2] = POP_;
})

DEF_CMD(ADD, 11, 0, {
	arg1 = POP_;
	arg2 = POP_;
	PUSH_(arg2 + arg1);
})
DEF_CMD(SUB, 12, 0, {
	arg1 = POP_;
	arg2 = POP_;
	PUSH_(arg2 - arg1);
})
DEF_CMD(MUL, 13, 0, {
	arg1 = POP_;
	arg2 = POP_;
	PUSH_(arg2 * arg1);
})
DEF_CMD(DIV, 14, 0, {
	arg1 = POP_;
	arg2 = POP_;
	PUSH_(arg2 / arg1);
})
DEF_CMD(SQRT, 15, 0, {
	PUSH_((int)sqrt(POP_));
})

DEF_CMD(IN, 16, 0, {
	arg1 = 0;
	scanf("%d", &arg1);
	PUSH_(arg1);
})
DEF_CMD(OUT, 17, 0, {
	printf("%d\n", POP_);
})

DEF_CMD(JMP, 18, 1, {
	GET(addr);
	if (addr >= cpu->programSize) {
		cpu->errno = CPU_IP_OUT_OF_PROG;
		return 0;
	}
	cpu->ip = addr;
})
DEF_CMD(JA, 19, 1, {
	GET(addr);
	if (addr >= cpu->programSize) {
		cpu->errno = CPU_IP_OUT_OF_PROG;
		return 0;
	}
	arg1 = POP_;
	arg2 = POP_;
	if (arg2 > arg1)
		cpu->ip = addr;
})
DEF_CMD(JB, 20, 1, {
	GET(addr);
	if (addr >= cpu->programSize) {
		cpu->errno = CPU_IP_OUT_OF_PROG;
		return 0;
	}
	arg1 = POP_;
	arg2 = POP_;
	if (arg2 < arg1)
	cpu->ip = addr;
})
DEF_CMD(JE, 21, 1, {
	GET(addr);
	if (addr >= cpu->programSize) {
		cpu->errno = CPU_IP_OUT_OF_PROG;
		return 0;
	}
	arg1 = POP_;
	arg2 = POP_;
	if (arg2 == arg1)
	cpu->ip = addr;
})
DEF_CMD(JNE, 22, 1, {
	GET(addr);
	if (addr >= cpu->programSize) {
		cpu->errno = CPU_IP_OUT_OF_PROG;
		return 0;
	}
	arg1 = POP_;
	arg2 = POP_;
	if (arg2 != arg1)
	cpu->ip = addr;
})

DEF_CMD(CALL, 23, 1, {
	GET(addr);
	if (addr >= cpu->programSize) {
		cpu->errno = CPU_IP_OUT_OF_PROG;
		return 0;
	}
	stackPush(&cpu->callStack, cpu->ip);
	cpu->ip = addr;
})
DEF_CMD(RET, 24, 0, {
	cpu->ip = stackPop(&cpu->callStack);
})

DEF_CMD(DUMP, 25, 0, {
	CPUDump(cpu);
})