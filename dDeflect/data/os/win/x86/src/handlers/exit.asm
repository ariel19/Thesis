[bits 32]
; exit(1)
; @eax = msvcrt!_exit

		xor		ecx, ecx
		add		ecx, 1
		push	ecx
		call	eax
		pop		eax
		