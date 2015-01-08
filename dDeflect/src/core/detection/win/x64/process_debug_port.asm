[bits 64]
; @rax = ntdll!NtQueryInformationProcess


			sub		rsp, 0x08
			or		rcx, -1
			mov		rdx, 7
			mov		r8, rsp
			mov		r9, 4
			push	0
			
			sub		rsp, 0x20
			call	rax
			add		rsp, 0x28
			
			xor		rax, rax
			cmp		DWORD [rsp], -1
			sete	al
			
			pop		rcx
			