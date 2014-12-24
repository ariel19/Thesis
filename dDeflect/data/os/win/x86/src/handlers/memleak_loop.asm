[bits 32]
; memleak loop
; @edi = msvcrt!malloc

			push	0x147
			sub		esp, 4
lll:
			pop		eax
			call	edi
			call	lll