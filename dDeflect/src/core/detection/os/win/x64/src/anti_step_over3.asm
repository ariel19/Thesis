[bits 64]

		xor 	rax, rax
		mov 	al, 0x90
		xor 	rcx, rcx
		inc 	ecx
		call	xxx
xxx:
		pop		rdi
		add 	rdi, 7
		rep 	stosb
l1: 	nop
