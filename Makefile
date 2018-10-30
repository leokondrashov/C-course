IDIR=./include
ODIR=./obj
BDIR=./bin
CC=gcc
CFLAGS=-Wall -I$(IDIR)

all:


squareEquation: squareEquation/squareEquation.c
	$(CC) -o $(BDIR)/squareEquation $(CFLAGS) squareEquation/squareEquation.c -lm

Shakespeare: textProcessor Shakespeare/Shakespeare.c
	$(CC) -o $(BDIR)/Shakespeare $(CFLAGS) Shakespeare/Shakespeare.c $(ODIR)/textProcessor.o

stack_hash: stack/stack.c
	$(CC) -c -o $(ODIR)/stack_hash.o $(CFLAGS) stack/stack.c -DNDEBUG -DCHECK_HASH

stack: stack/stack.c
	$(CC) -c -o $(ODIR)/stack.o $(CFLAGS) stack/stack.c -DNDEBUG

stack_dbg: stack/stack.c
	$(CC) -c -o $(ODIR)/stack_dbg.o $(CFLAGS) stack/stack.c -DCHECK_HASH

stack_utest: stack_dbg stack/unittest.c
	$(CC) -o $(BDIR)/stack_utest $(CFLAGS) stack/unittest.c $(ODIR)/stack_dbg.o

map: stack/map.c
	$(CC) -c -o $(ODIR)/map.o $(CFLAGS) stack/map.c -DNDEBUG

asm: textProcessor SoftCPU/asm.c map
	$(CC) -o $(BDIR)/asm $(CFLAGS) SoftCPU/asm.c $(ODIR)/textProcessor.o $(ODIR)/map.o

disasm: textProcessor SoftCPU/disasm.c
	$(CC) -o $(BDIR)/disasm $(CFLAGS) SoftCPU/disasm.c $(ODIR)/textProcessor.o

CPU: stack textProcessor SoftCPU/CPU.c
	$(CC) -o $(BDIR)/CPU $(CFLAGS) SoftCPU/CPU.c $(ODIR)/textProcessor.o $(ODIR)/stack.o -lm

textProcessor: Shakespeare/textProcessor.c
	$(CC) -c -o $(ODIR)/textProcessor.o $(CFLAGS) Shakespeare/textProcessor.c
