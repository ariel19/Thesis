[bits 32]

			xor		ebx, ebx
			call	[ecx]						; TODO: GetVersion address function should be resolved from wrapper
			cmp		al, 0x06
			sbb		ebp, ebp
			jb		check_dbg					; If version below Windows Vista
		
			; Check for heap protection for Vista and later
check_prot:	mov		eax, [fs:0x30]		; PEB
			mov		eax, [eax + 0x18]			; Process Heap base
			mov		ecx, [eax + 0x24]
			jecxz	check_dbg					; If no heap protection check for debugger
			
get_prot:	mov		ecx, [ecx]
			test	[eax + 0x4c], ecx
			cmovne	ebx, [eax + 0x50]			; Get heap key
			
			; Check for 0xABABABAB, 0xABABABAB at the end of block
check_dbg:	mov		eax, <heap ptr> 			; TODO: heap ptr
			movzx	edx, WORD [eax - 8]			; size
			xor		dx, bx
			movzx	ecx, BYTE [eax + ebp - 1]	; overhead
			sub		eax, ecx
			lea		edi, [edx * 8 + eax]
			mov 	al, 0xAB
			mov		cl, 8
			repe	scasb
			setz	al							; TODO: check setz/setnz