[bits 32]
; @eax = kernel32!GetModuleFileNameA
; @edi = kernel32!CreateFileA
; @ebx = kernel32!CloseHandle


			sub		esp, 0x200
			
			mov		edx, esp
			push	esp
			
			push	0x200
			push	edx
			push	0				; Self
			call	eax
			
			xor		edx, edx
			pop		eax
			
			push	edx
			push	edx
			push	3				; OPEN_EXISTING
			push	edx
			push	edx
			inc		edx
			ror		edx, 1
			push	edx
			push	eax
			call	edi
			
			inc		eax
			test	eax, eax
			jz		dbg
			
			dec		eax
			push	eax
			call	ebx
			xor		eax, eax
			jmp		endf
	
dbg:
			or		eax, 0xC8
endf:
			add		esp, 0x200