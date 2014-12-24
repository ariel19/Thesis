[bits 64]
; exit(1)
; @rax = msvcrt!_exit

		xor		rcx, rcx
		add		rcx, 1
		push	rcx
		call	rax
		pop		rax
		