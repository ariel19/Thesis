[bits 64]
; @rax = NtSetDebugFilterState


			sub		rsp, 0x20
			xor		rdx, rdx
			mov		rcx, rdx
			mov		r8, rdx
			inc		r8
			call	rax
			add		rsp, 0x20
			not 	rax