IDIR=./include
ODIR=./obj
BDIR=./bin
CC=gcc
CFLAGS=-Wall -I$(IDIR)

all: CPU asm disasm


squareEquation: squareEquation/squareEquation.c
	$(CC) -o $(BDIR)/squareEquation $(CFLAGS) squareEquation/squareEquation.c -lm

Shakespeare: textProcessor Shakespeare/Shakespeare.c
	$(CC) -o $(BDIR)/Shakespeare $(CFLAGS) Shakespeare/Shakespeare.c $(ODIR)/textProcessor.o

stack_hash: stack/stack.c
	$(CC) -c -o $(ODIR)/stack_hash.o $(CFLAGS) stack/stack.c -DNDEBUG -DCHECK_HASH

stack.o: stack/stack.c
	$(CC) -c -o $(ODIR)/stack.o $(CFLAGS) stack/stack.c -DNDEBUG

stack_dbg: stack/stack.c
	$(CC) -c -o $(ODIR)/stack_dbg.o $(CFLAGS) stack/stack.c -DCHECK_HASH

stack_utest: stack_dbg stack/unittest.c
	$(CC) -o $(BDIR)/stack_utest $(CFLAGS) stack/unittest.c $(ODIR)/stack_dbg.o

map: stack/map.c
	$(CC) -c -o $(ODIR)/map.o $(CFLAGS) stack/map.c -DNDEBUG

asm: textProcessor SoftCPU/asm.c map
	$(CC) -g -o $(BDIR)/asm $(CFLAGS) SoftCPU/asm.c $(ODIR)/textProcessor.o $(ODIR)/map.o

disasm: textProcessor SoftCPU/disasm.c
	$(CC) -o $(BDIR)/disasm $(CFLAGS) SoftCPU/disasm.c $(ODIR)/textProcessor.o

CPU: stack.o textProcessor SoftCPU/CPU.c
	$(CC) -g -o $(BDIR)/CPU $(CFLAGS) SoftCPU/CPU.c $(ODIR)/textProcessor.o $(ODIR)/stack.o -lm

textProcessor: Shakespeare/textProcessor.c
	$(CC) -c -o $(ODIR)/textProcessor.o $(CFLAGS) Shakespeare/textProcessor.c

list.o: list/list.c
	$(CC) -g -c -o $(ODIR)/list.o $(CFLAGS) list/list.c

list_test: list.o list/unittest.c
	$(CC) -g -o $(BDIR)/list_test $(CFLAGS) list/unittest.c $(ODIR)/list.o