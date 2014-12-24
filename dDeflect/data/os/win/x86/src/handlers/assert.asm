[bits 32]
; assert("Debug!", "main", 0)
; @edx = msvcrt!_assert

		xor		eax, eax
		push	eax
		call	filename
		db		"main", 0x00
filename:
		call	message
		db		"Debug!", 0x00
message:
		call	edx
		add		esp, 12