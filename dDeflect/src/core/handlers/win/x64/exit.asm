[bits 64]
; exit(1)
; @rax = msvcrt!_exit

		xor		rcx, rcx
		add		rcx, 1
		sub		rsp, 0x20
		call	rax
		add		rsp, 0x20
		