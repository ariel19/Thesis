[bits 64]
; @rax = kernel32!GetModuleFileNameA
; @rdi = kernel32!CreateFileA
; @rbx = kernel32!CloseHandle


			mov		r8, 0x200
			sub		rsp, r8
			
			xor		rcx, rcx
			mov		rdx, rsp
			
			sub		rsp, 0x20
			call	rax
			add		rsp, 0x20
			
			lea		rcx, [rsp]
			xor		r8, r8
			mov		rdx, r8
			inc		rdx
			ror		rdx, 1
			xor		r9, r9
			
			push	r9
			push	r8
			sub		rsp, 0x20
			call	rdi
			
			inc		rax
			test	rax, rax
			jz		dbg
			
			mov		rcx, rax
			dec		rcx
			call	rbx
			xor		rax, rax
			jmp		endf
			
dbg:
			or		rax, rsp
			
endl:
			add		rsp, 0x230