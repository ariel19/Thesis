[bits 32]
; @eax = ntdll!NtSetDebugFilterState


			
			xor		edx, edx
			push	1
			push	edx
			push	edx
			call	eax
			not		eax
			