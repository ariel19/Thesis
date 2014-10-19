[bits 64]

	xor rax, rax
	mov al, 0x90
	xor rcx, rcx
	inc ecx
	mov rdi, l1
	rep stosb
l1: nop
