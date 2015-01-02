[bits 64]

		xor 	rax, rax
		mov 	al, 0x90
		push 	2
		pop 	rcx
		call	xxx
xxx:
		pop		rdi
		add		rdi, 9
		std
		rep 	stosb
		nop
l1: 	nop
		cld
