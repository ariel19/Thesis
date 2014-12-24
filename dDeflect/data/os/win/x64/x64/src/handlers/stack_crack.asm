[bits 64]
; stack

			add		rsp, 64
			xor		[rsp], rdx
			xor		[rsp + 8], rsi
			or		[rsp + 16], rdi
			and		[rsp + 24], rsp
			