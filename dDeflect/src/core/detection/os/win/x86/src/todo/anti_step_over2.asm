[bits 32]

	xor eax, eax
	mov al, 0x90
	push 2
	pop ecx
	mov edi, l1
	std
	rep stosb
	nop
l1: nop
