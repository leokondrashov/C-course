.intel_syntax noprefix

//	.text
//	.section	.rodata
//fileName:
//	.string	"list/listFind.s"
//varName:
//	.string	"l"
//functionName:
//	.string	"listFindVerySlow"

	.text
	.globl	listFindVerySlow
    .type	listFindVerySlow, @function
listFindVerySlow:
	push	rbp
	mov		rbp, rsp
	push	rbx
	push	r12
	push	r13
	sub		rsp, 0x08
	mov		r12, rdi
	mov		r13, rsi
	cmp		rdi, 0
//	jne	.assert_passed
//	lea		rcx, [rip+functionName]
//	mov		edx, 27
//	lea		rsi, [rip+fileName]
//	lea		rdi, [rip+varName]
//	call	__assert_fail@PLT

//.assert_passed:
//	mov		rax, [rbp-8]
	mov		eax, [r12+0x1c]
	mov		ebx, eax
	jmp .condition

.loop_top:
//	mov		rax, [rbp-8]
	mov		rax, [r12]

//	mov		%ebx, %rbx
//	sal		$3, %rbx
//	add		%rbx, %rax
//	mov		(%rax), %rax

	mov		rsi, [rax+rbx*8]
	mov		rsi, [rsi]

//	mov		-32(%rbp), %rax
//	mov		%rbx, %rsi
//	mov		rsi, [rax]
	mov		rdi, r13
	call	strcmp@PLT

	test	eax, eax
	jne	.continue
	mov		eax, ebx
	jmp	.exit

.continue:
//	mov		rax, [rbp-8]
	mov		rax, [r12+8]
//	mov		-4(%rbp), %ebx
//	mov		%ebx, %rbx
//	sal		$2, %rbx
//	add		%rbx, %rax
//	mov		(%rax), %eax
//	mov		%eax, -4(%rbp)

	mov		ebx, [rax+rbx*4]

.condition:
	test	ebx, ebx
	jne	.loop_top
	mov		eax, 0

.exit:
	pop r13
	pop r12
	pop rbx
	leave
	ret

.size	listFindVerySlow, .-listFindVerySlow
