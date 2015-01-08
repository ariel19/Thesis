[bits 32]

		xor		eax, eax
		mov		al, 0x90
		xor		ecx, ecx
		inc		ecx
		call	xxx
xxx:
		pop		edi
		add		edi, 6			; offset l1
		rep stosb
l1:		nop
