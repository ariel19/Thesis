[bits 64]
; @rax = user32!GetShellWindow
; @rbx = user32!GetWindowThreadProcessId
; @rdi = ntdll!NtQueryInformationProcess


			sub		rsp, 0x60
			
			call	rax
			
			mov		rcx, rax
			mov		rdx, rsp
			add		rdx, 0x20
			call	rbx
			
			add		rsp, 0x20
			
			push	0
			xor		rcx, rcx
			dec		rcx
			xor		rdx, rdx
			mov		r8, rsp
			add		r8, 0x10
			mov		r9, 0x30
			
			sub		rsp, 0x28
			call	rdi
			add		rsp, 0x28
			
			mov		rdx, [rsp + 0x30]
			pop		rcx
			add		rsp, 0x38
			xor		rax, rax
			cmp		rdx, rcx
			setne	al