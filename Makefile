IDIR=./include
ODIR=./obj
BDIR=./bin
CC=gcc
CFLAGS=-Wall -I$(IDIR)

all: CPU asm disasm


squareEquation: squareEquation/squareEquation.c
	$(CC) -o $(BDIR)/squareEquation $(CFLAGS) squareEquation/squareEquation.c -lm

Shakespeare: textProcessor.o Shakespeare/Shakespeare.c
	$(CC) -o $(BDIR)/Shakespeare $(CFLAGS) Shakespeare/Shakespeare.c $(ODIR)/textProcessor.o

stack_hash: stack/stack.c
	$(CC) -c -o $(ODIR)/stack_hash.o $(CFLAGS) stack/stack.c -DNDEBUG -DCHECK_HASH

stack.o: stack/stack.c
	$(CC) -c -o $(ODIR)/stack.o $(CFLAGS) stack/stack.c -DNDEBUG

stack_dbg: stack/stack.c
	$(CC) -c -o $(ODIR)/stack_dbg.o $(CFLAGS) stack/stack.c -DCHECK_HASH

stack_utest: stack_dbg stack/unittest.c
	$(CC) -o $(BDIR)/stack_utest $(CFLAGS) stack/unittest.c $(ODIR)/stack_dbg.o

map.o: stack/map.c
	$(CC) -c -o $(ODIR)/map.o $(CFLAGS) stack/map.c -DNDEBUG

asm: textProcessor.o SoftCPU/asm.c map.o
	$(CC) -g -o $(BDIR)/asm $(CFLAGS) SoftCPU/asm.c $(ODIR)/textProcessor.o $(ODIR)/map.o

disasm: textProcessor.o SoftCPU/disasm.c
	$(CC) -o $(BDIR)/disasm $(CFLAGS) SoftCPU/disasm.c $(ODIR)/textProcessor.o

CPU: stack.o textProcessor.o SoftCPU/CPU.c
	$(CC) -g -o $(BDIR)/CPU $(CFLAGS) SoftCPU/CPU.c $(ODIR)/textProcessor.o $(ODIR)/stack.o -lm

textProcessor.o: Shakespeare/textProcessor.c
	$(CC) -c -o $(ODIR)/textProcessor.o $(CFLAGS) Shakespeare/textProcessor.c

list.o: list/list.c
	$(CC) -g -c -o $(ODIR)/list.o $(CFLAGS) list/list.c -DNDEBUG

list_test: list.o list/unittest.c
	$(CC) -g -o $(BDIR)/list_test $(CFLAGS) list/unittest.c $(ODIR)/list.o

tree.o: akinator/tree.c
	$(CC) -g -c -o $(ODIR)/tree.o $(CFLAGS) akinator/tree.c -DNDEBUG

tree_test: tree.o akinator/test.c textProcessor.o
	$(CC) -g -o $(BDIR)/tree_test $(CFLAGS) akinator/test.c $(ODIR)/textProcessor.o $(ODIR)/tree.o

akinator: akinator/akinator.c tree.o
	$(CC) -g -o $(BDIR)/akinator $(CFLAGS) akinator/akinator.c $(ODIR)/tree.o

differentiator: differentiator/differentiator.c tree.o textProcessor.o recursiveDescent.o
	$(CC) -g -o $(BDIR)/differentiator $(CFLAGS) differentiator/differentiator.c $(ODIR)/textProcessor.o $(ODIR)/recursiveDescent.o $(ODIR)/tree.o -lm

recursiveDescent.o: differentiator/recursiveDescent.c tree.o
	$(CC) -g -c -o $(ODIR)/recursiveDescent.o $(CFLAGS) differentiator/recursiveDescent.c -lm