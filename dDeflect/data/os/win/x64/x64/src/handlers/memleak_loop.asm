[bits 32]
; memleak loop
; @rdi = msvcrt!malloc

			mov		r14, 0x84A
			sub		rsp, 0x28
lll:
			pop		rax
			mov		rcx, r14
			call	rdi
			call	lll