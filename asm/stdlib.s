.intel_syntax noprefix

		.globl in
		.globl out

.text

in:
		push rbp
		mov rbp, rsp

		xor rax, rax
		xor rbx, rbx
		xor rdi, rdi
		xor r12b, r12b
		mov rsi, offset tmp
		mov rdx, 0x01

		push rax
		syscall
		pop rax

		cmp byte ptr [rsi], '-'
		je minus

		cmp byte ptr [rsi], '+'
		je loop

		cmp byte ptr [rsi], '0'
		jb end

		cmp byte ptr [rsi], '9'
		ja end

		mov bl, [rsi]
		sub bl, '0'
		jmp loop

minus:	mov r12b, 1

loop:	push rax
		syscall
		pop rax

		cmp byte ptr [rsi], '0'
        jb end

        cmp byte ptr [rsi], '9'
        ja end

        lea rbx, [rbx + 4 * rbx]
        shl rbx, 1
        add bl, [rsi]
        sub bl, '0'
        jmp loop


end:
		test r12, r12
		je fin

		neg rbx

fin:	mov [rbp + 0x10], rbx

		leave
		ret

out:
		push rbp
		mov rbp, rsp

		push offset tmp
		push [rbp + 0x10]
		call itos10

		add rsp, 8
		call strlen

		mov rdx, rax
        mov rax, 0x01
		mov rdi, 0x01
		pop rsi
		syscall

		mov rdx, 0x01
        mov rax, 0x01
		mov rdi, 0x01
		mov rsi, offset endl
		syscall

		leave
		ret

strlen:
		push rbp
		mov rbp, rsp
		push rsi
		push rdi

		mov rdi, [rbp+0x10]
		xor rcx, rcx
		dec rcx
		xor al, al
		cld

		repne scasb
		not rcx
		dec rcx
		mov rax, rcx

		pop rdi
		pop rsi
		pop rbp

		ret

itos10:
		push rbp
		mov rbp, rsp

		mov rax, [rbp+0x10]
		mov rdi, [rbp+0x18]

		test rax, rax
		jns positive

		mov byte ptr [rdi], '-'
		inc rdi
		neg rax

positive:
		mov rcx, 0x0a
		xor rdx, rdx
		push cx

1:		div rcx
		push dx
		xor dx, dx
		cmp rax, 0
		jne 1b

		pop ax
2:		add al, '0'
		mov [rdi], al
		inc rdi
		pop ax
		cmp ax, 0x0a
		jne 2b

		mov byte ptr [rdi], 0

		pop rbp

		ret

.comm tmp 20

.data
endl: .string "\n"
