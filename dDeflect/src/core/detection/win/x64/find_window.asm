[bits 64]
; @rbx = user32!FindWindowA


			call	load_names
			db		"WinDbgFrameClass", 0x00
			db		"OLLYDBG", 0x00
			db		0x00
			
load_names:
			pop		rdi
			sub		rsp, 0x20

loopn:		
			xor		rdx, rdx
			mov		rcx, rdi
			call	rbx
			test	rax, rax
			jnz		found
			
			xor		rax, rax
			or		rcx, -1
			repne	scasb
			cmp		[rdi], al
			jne		loopn
			
			xor		rax, rax
found:
			add		rsp, 0x20