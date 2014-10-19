[bits 64]
	
	xor rax, rax
	xor rcx, rcx
	inc rcx
	mov rsi, l1
	lea rdi, [rsi + 1]
	rep movsb
l1:	mov al, 0x90
l2: cmp al, 0xcc
	setz al
