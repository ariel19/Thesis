[bits 32]
; @ebx = user32!FindWindowA


			call	load_names
			db		"WinDbgFrameClass", 0x00
			db		"OLLYDBG", 0x00
			db		0x00
			
load_names:
			pop		edi

loopn:		
			push	0
			push	edi
			call	ebx
			test	eax, eax
			jnz		found
			
			xor		eax, eax
			or		ecx, -1
			repne	scasb
			cmp		[edi], al
			jne		loopn
			
			xor		eax, eax
found: