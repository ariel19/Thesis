[bits 64]
; ExitProcess(1)
; @rax = kernel32!ExitProcess

		xor		rdx, rdx
		inc		rdx
		push	rdx
		call	rax
		xor		rax, rax
		