[bits 32]

		xor 	eax, eax
		mov 	al, 0x90
		push 	2
		pop 	ecx
		call	xxx
xxx:
		pop		edi
		add		edi, 8			; offset l1
		std
		rep 	stosb
		nop
l1: 	nop
		cld
