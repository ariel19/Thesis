[bits 32]
; memleak loop
; @rdi = msvcrt!malloc

			push	0x1A8547
			sub		rsp, 8
lll:
			pop		rax
			call	rdi
			call	lll