[bits 64]
; assert("Debug!", "main", 0)
; @rdx = msvcrt!_assert

		xor		rax, rax
		push	rax
		call	filename
		db		"main", 0x00
filename:
		call	message
		db		"Debug!", 0x00
message:
		call	rdx
		add		rsp, 24