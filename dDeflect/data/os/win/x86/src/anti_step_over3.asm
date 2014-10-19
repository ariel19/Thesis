[bits 32]

	xor eax, eax
	mov al, 0x90
	xor ecx, ecx
	inc ecx
	mov edi, l1
	rep stosb
l1: nop
