[bits 32]
; loop

		xor		ecx, ecx
		mov		eax, ecx
		
ll:
		add		ecx, 1
		test	eax, eax
		jnz		ll
		
		call	edx
		add		esp, 4