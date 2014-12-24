[bits 64]
; abort()
; @rcx = msvcrt!abort

		mov		[rsp], rcx
		push	rdx
		xor		rcx, rcx
		sub		rsp, 0x28
		add		rcx, 0x19C65ADB
		call	[rsp + 0x30]
		call	rcx
		jmp		rcx
		