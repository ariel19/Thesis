[bits 64]

	xor rax, rax
	mov al, 0x90
	push 2
	pop rcx
	mov rdi, l1
	std
	rep stosb
	nop
l1: nop
