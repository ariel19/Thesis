[bits 32]
; @eax = msvcrt!printf

			push	esp
			call	msg
			db		"hook! [%x]", 0x0A, 0x0D, 0x00
msg:
			call	eax
			add		esp, 8
			