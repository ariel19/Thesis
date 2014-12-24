[bits 64]
; stack

			add		rsp, 64
			xor		[rsp], rdx
			xor		[rsp + 8], rsi
			or		[rsp + 16], rdi
			and		[rsp + 24], rsp
			xor		[rsp + 28], rdx
			xor		[rsp + 32], rsi
			or		[rsp + 36], rdi
			and		[rsp + 40], rsp
			xor		[rsp + 44], rdx
			xor		[rsp + 48], rsi
			or		[rsp + 52], rdi
			and		[rsp + 56], rsp
			