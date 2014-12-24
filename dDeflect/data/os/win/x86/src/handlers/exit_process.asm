[bits 32]
; ExitProcess(1)
; @eax = kernel32!ExitProcess

		xor		edx, edx
		inc		edx
		push	edx
		call	eax
		xor		eax, eax
		