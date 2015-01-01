[bits 64]
; @rax = msvcrt!printf

			sub		rsp, 0x20
			call	msg
			db		"hook! [%x]", 0x0a, 0x0d, 0x00
msg:
			pop		rcx
			mov		rdx, rsp
			call	rax
			add		rsp, 0x20