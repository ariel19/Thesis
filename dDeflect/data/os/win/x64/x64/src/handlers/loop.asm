[bits 64]
; loop

		xor		rcx, rcx
		mov		rax, rcx
		
ll:
		add		rcx, 1
		test	rax, rax
		jnz		ll
		
		call	rdx
		add		rsp, 8