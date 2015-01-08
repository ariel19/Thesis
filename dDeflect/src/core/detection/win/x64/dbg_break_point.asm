[bits 64]
; METODA WYWO£YWANA TYLKO RAZ!!! OEP
; @rbx = ntdll!DbgBreakPoint
; @r12 = kernel32!VirtualProtect


			sub		rsp, 0x30
			mov		rcx, rbx
			xor		rdx, rdx
			add		rdx, 1
			mov		r8, 0x40				; PAGE_EXECUTE_READWRITE
			mov		r9, rsp
			add		r9, 0x20
			call	r12
			add		rsp, 0x30
			
			mov		BYTE [rbx], 0xC3		; ret