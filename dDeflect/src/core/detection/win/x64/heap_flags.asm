[bits 64]
; HeapFlags
; @r10 = kernel32!GetVersion

			; get heap
			push	0x60
			pop		rsi
			gs		lodsq ; PEB
			mov		ebx, [rax + 0x30] 	; get proc heap base

			sub		rsp, 0x20
			call 	r10 				; GetVersion
			add		rsp, 0x20
			cmp 	al, 0x6
			sbb 	rax, rax
			and 	al, 0xa4
			xor 	rcx, rcx

			; ForceFlags
			; HEAP_TAIL_CHECKING_ENABLED
			cmp 	dword [rbx + rax + 0x74], 0x40000060
			setz 	cl ; set if being debugged
