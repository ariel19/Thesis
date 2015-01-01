[bits 64]
; assert("Debug!", "main", 0)
; @r10 = msvcrt!_assert

		xor		rax, rax
		mov		r8, rax
		sub		rsp, 0x20
		call	filename
		db		"main", 0x00
filename:
		pop		rdx
		call	message
		db		"Debug!", 0x00
message:
		pop		rcx
		call	r10
		add		rsp, 0x4A