[bits 64]
; sigsegv


			xor		rax, rsi
			xor		rax, rcx
			call	rax
			xor		rcx, rcx
			sub		rcx, 12
			add		rcx, 4
			mov		[rcx + 8], rdx
			