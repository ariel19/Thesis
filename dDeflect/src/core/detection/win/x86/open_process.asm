[bits 32]
; @eax = ntdll!CsrGetProcessId
; @ebx = kernel32!OpenProcess

		call	eax
		push	eax
		push	0
		push	0x1F0FFF		; PROCESS_ALL_ACCESS
		call	ebx
		