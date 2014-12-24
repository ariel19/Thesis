[bits 64]
; undefined instruction

			xor		rax, rax
			add		rax, rcx
			call	aa
			mov		[rdx + 4*rsi + 16], rdi
aa:
			ud2
			add		rsp, 64