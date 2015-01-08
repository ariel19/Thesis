[bits 32]
; @eax = user32!GetShellWindow
; @ebx = user32!GetWindowThreadProcessId
; @edi = ntdll!NtQueryInformationProcess


			call	eax
			
			push	eax
			push	esp
			push	eax
			call	ebx
			
			sub		esp, 0x18
			
			push	0
			push	0x18
			push	esp
			add		DWORD [esp], 8
			push	0
			push	-1
			call	edi
			
			mov		ecx, [esp + 0x14]
			add		esp, 0x18
			pop		edx
			
			cmp		ecx, edx
			setne	al
			