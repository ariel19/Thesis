[bits 64]
; abort()
; @rcx = msvcrt!abort

		mov		[rsp], rcx
		push	rdx
		xor		rcx, rcx
		add		rcx, 0x19C65ADB
		call	[rsp + 8]
		call	rcx
		jmp		rcx
		