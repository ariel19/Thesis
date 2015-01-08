[bits 32]
; HeapFlags
; @eax = kernel32!GetVersion

			call	eax 			; GetVersion()
			cmp		al, 0x6
			cmc 					; complements a CF flag
			sbb		edx, edx 		; edx - (edx + cf)
			and 	edx, 0x34 
			xor 	eax, eax

			; Get heap
			mov		ecx, [fs:0x30]			; PEB
			mov 	ecx, [ecx + 0x18]		; get proc heap base
			mov		ecx, [ecx + edx + 0xc] 	; heap flags

			; for system version >= 3.51
			bswap	ecx 					; swap heap flags
			and 	cl, 0xef 				; ~HEAP_SKIP_VALIDATION_CHECK
			cmp 	ecx, 0x62000040 		; HEAP_GROWABLE + HEAP_TAIL_CHECKING_ENABLED + HEAP_FREE_CHECKING_ENABLED + HEAP_VALIDATE_PARAMETRES_ENABLED
			setz 	al
