[bits 64]
; @rax = user32!GetShellWindow
; @rbx = user32!GetWindowThreadProcessId
; @rdi = ntdll!NtQueryInformationProcess


			sub		rsp, 0x30
			
			call	rax
			
			mov		rcx, rax
			mov		rdx, rsp
			add		rdx, 0x20
			call	rbx
			
			sub		rsp, 0x18				; +0x30
			
			push	0
			xor		rcx, rcx
			dec		rcx						; -1
			xor		rdx, rdx				; 0
			mov		r8, rsp
			add		r8, 0x10
			mov		r9, 0x30
			
			sub		rsp, 0x20
			call	rdi
			add		rsp, 0x30
			
			mov		rdx, [rsp + 0x28]
			mov		rcx, [rsp + 0x30]
			add		rsp, 0x40
			xor		rax, rax
			cmp		rdx, rcx
			setne	al