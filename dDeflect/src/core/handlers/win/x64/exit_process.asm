[bits 64]
; ExitProcess(1)
; @rax = kernel32!ExitProcess

		xor		rcx, rcx
		inc		rcx
		push	rcx
		sub		rsp, 0x18
		call	rax
		xor		rax, rax
		add		rsp, 0x20
		