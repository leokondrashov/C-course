CBtoRealCPU:
	../bin/frontEnd $(if) -o $(if:.cb=.tree)
	../bin/backEndRealCPU $(if:.cb=.tree) -o $(if:.cb=.s)
	as -a $(if:.cb=.s) -o $(if:.cb=.o) > $(if:.cb=.lst)
	ld -s $(if:.cb=.o) stdlib.o -o $(of)

CBtoSoftCPU:
	../bin/frontEnd $(if) -o $(if:.cb=.tree)
	../bin/backEndSoftCPU $(if:.cb=.tree) -o $(if:.cb=.asm)
	../bin/asm $(if:.cb=.asm) -o $(of) -l $(if:.cb=.lst)

runSoftCPU:
	../bin/CPU $(file)
