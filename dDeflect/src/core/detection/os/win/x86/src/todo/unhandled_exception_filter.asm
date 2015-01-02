[bits 32]
; @ebx = kernel32!SetUnhandledExceptionFilter


			call	xxx
			jmp		handler
xxx:
			call	ebx
			push	eax
			
			call	value
			nop
value:
			pop		edi
			mov		BYTE [edi], 0x00
			
			int		3
			jmp		endf
			
handler:
			call	yyy
yyy:
			pop		eax
			mov		BYTE [eax - 14], 0x01
			mov		eax, [esp + 4]
			add		DWORD [eax + 0xB8], 1
			mov		eax, -1
			ret		4
			
endf:
			
			call	ebx
			movzx	eax, BYTE [edi]