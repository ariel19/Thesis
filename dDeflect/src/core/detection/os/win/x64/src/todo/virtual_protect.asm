[bits 64]
; @rax = kernel32!VirtualAlloc
; @rbx = kernel32!VirtualProtect
; @rdi = kernel32!VirtualFree
; @r12 = kernel32!AddVectoredExceptionHandler
; @r13 = kernel32!RemoveVectoredExceptionHandler

			sub		rsp, 0x20
			xor		rcx, rcx
			mov		rdx, rcx
			inc		rdx
			mov		r8, 0x1000
			mov		r9, 0x40
			call	rax
			add		rsp, 0x20
			
			mov		BYTE [rax], 0xC3
			
			push	rax
			push	rdi
			
			mov		rcx, rax
			mov		rdx, 1
			mov		r8, 0x140
			mov		r9, rsp
			
			sub		rsp, 0x20
			call	rbx
			
			mov		rcx, rax
			call	xxx
			jmp		no_dbg
			
xxx:
			pop		rdx
			call	r12
			add		rsp, 0x20
			mov		[rsp], rax
			
			call	yyy
			jmp		is_dbg
			
yyy:
			jmp		[rsp + 16]
			
is_dbg:
			xor		rax, rax
			add		rax, 7
			jmp		endf
			
no_dbg:
			mov		rcx, [rsp]
			sub		rsp, 0x20
			call	r13
			
			mov		rcx, [rsp + 28]
			xor		rdx, rdx
			mov		r8, 0x8000
			call	rdi
			xor		rax, rax
			add		rsp, 0x30
			
endf: