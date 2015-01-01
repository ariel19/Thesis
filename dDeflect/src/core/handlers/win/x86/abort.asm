[bits 32]
; abort()
; @ecx = msvcrt!abort

		mov		[esp], ecx
		push	edx
		xor		ecx, ecx
		add		ecx, 0x19C65ADB
		call	[esp + 4]
		call	ecx
		jmp		ecx
		