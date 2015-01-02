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
			
			call	flag
			nop
flag:
			mov		edx, [esp]
			mov		BYTE [edx], 0x1
			
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
			jmp		endf
yyy:
			jmp		[esp + 12]
			
no_dbg:
			call	get_flag
get_flag:
			pop		edx
			mov		BYTE [edx - 58], 0
			;mov		eax, [esp + 0x0C]
			;sub		edx, 11
			;mov		DWORD [eax + 0xB8], edx
			xor		eax, eax
			ret		4

endf:
			pop		ecx
			mov		[fs:0x00], ecx
			add		esp, 4
			pop		eax
			
			push	0x8000
			push	0
			push	eax
			call	edi
			
			pop		eax
			movzx	eax, BYTE [eax]
			