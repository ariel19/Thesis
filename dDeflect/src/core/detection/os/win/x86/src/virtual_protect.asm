[bits 32]
; @eax = kernel32!VirtualAlloc
; @ebx = kernel32!VirtualProtect
; @edi = kernel32!VirtualFree


			push	0x40				; PAGE_EXECUTE_READWRITE
			push	0x1000				; MEM_COMMIT
			push	1
			push	0
			call	eax
			
			mov		BYTE [eax], 0xC3	; ret
			
			push	eax
			push	eax
			
			push	esp
			push	0x140
			push	1
			push	eax
			call	ebx
			pop		ebx
			
			call	xxx
			jmp		no_dbg
xxx:
			push	DWORD [fs:0x00]
			mov		[fs:0x00], esp
			call	yyy
			jmp		is_dbg
yyy:
			jmp		[esp + 12]
		
is_dbg:
			xor		eax, eax
			add		eax, 1
			jmp		endf
			
no_dbg:
			pop		eax
			push	0x8000				; MEM_RELEASE
			push	0
			push	eax
			call	edi
			
			xor		eax, eax

endf: