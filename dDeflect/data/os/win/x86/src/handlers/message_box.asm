[bits 32]
; ExitProcess(1)
; @edi = kernel32!ExitProcess
; @ecx = user32!MessageBoxA

		xor		edx, edx
		push	0x10
		push	edx
		call	msg
		db		"Debugger detected!", 0x00
msg:
		push	edx
		call	ecx
		xor		edx, edx
		inc		edx
		push	edx
		call	edi
		xor		eax, eax
		