[bits 64]
; MessageBox
; @rax = user32!MessageBoxA
; @r11 = kernel32!ExitProcess

		sub		rsp, 0x10
		xor		rcx, rcx
		push	rax
		mov		r8, rcx
		xor		r9, r9
		add		r9, 0x10
		call	msg
		db		"Debugger detected!", 0x00
msg:
		mov		rdx, [rsp]
		call	rax
		
		xor		rcx, rcx
		inc		rcx
		call	r11
		xor		rax, rax
		add		rsp, 0x20
		