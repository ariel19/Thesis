[bits 32]
; METODA WYWO£YWANA TYLKO RAZ!!! OEP
; @ebx = ntdll!DbgBreakPoint
; @edx = kernel32!VirtualProtect


			push	edx
			push	esp
			push	0x40				; PAGE_EXECUTE_READWRITE
			push	1
			push	ebx
			
			call	edx
			mov		BYTE [ebx], 0xC3	; ret
			add		esp, 4
			