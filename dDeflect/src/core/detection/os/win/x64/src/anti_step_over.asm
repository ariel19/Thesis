[bits 64]
	
		xor 	rax, rax
		xor 	rcx, rcx
		inc 	rcx
		call	xxx
xxx:
		pop		rsi
		add		rsi, 11
		lea 	rdi, [rsi + 1]
		rep 	movsb
l1:		mov 	dl, 0x90
		xor		rax, rax
l2: 	cmp 	dl, 0xcc
		setz 	al
