[bits 64]
; HeapFlags
; @r10 = kernel32!GetVersion

			; get heap
			push	0x60
			pop		rsi
			gs		lodsq ; PEB
			xor		rdx, rdx
			mov		edx, [rax + 0x30] 	; get proc heap base

			sub		rsp, 0x20
			call 	r10 				; GetVersion
			add		rsp, 0x20
			cmp 	al, 0x6
			sbb 	rcx, rcx
			and 	cl, 0xa4
			xor 	rax, rax

			; ForceFlags
			; HEAP_TAIL_CHECKING_ENABLED
			cmp 	dword [rdx + rcx + 0x74], 0x400000060
			setz al ; set if being debugged
