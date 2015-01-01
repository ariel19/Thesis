[bits 64]
; @rbx = kernel32!SetUnhandledExceptionFilter


			call	xxx
			jmp		no_dbg
xxx:
			pop		rcx
			sub		rsp, 0x20
			call	rbx
			add		rsp, 0x20
			
			int		3
			or		rax, 1
			jmp		endf
			
no_dbg:
			mov		rcx, rax
			sub		rsp, 0x20
			call	rbx
			add		rsp, 0x20
			xor		rax, rax
			
endf: