[bits 64]
; @r12 = kernel32!CheckRemoteDebuggerPresent


			xor		rcx, rcx
			push	rcx
			push	rcx
			
			dec		rcx
			mov		rdx, rsp
			
			sub		rsp, 0x20
			call	r12
			add		rsp, 0x20
			pop 	rax
			pop		rcx
			