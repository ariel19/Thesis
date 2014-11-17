[bits 64]

			xor		ebx, ebx
			call	[rcx]						; TODO: GetVersion address function should be resolved from wrapper
			cmp		al, 0x06
			sbb		rbp, rbp
			jb		check_dbg					; If version below Windows Vista
		
			; Check for heap protection for Vista and later
check_prot:	mov		rax, [gs:0x60]		; PEB
			mov		eax, [rax + 0x30]			; Process Heap base TODO: test if RAX or EAX!
			mov		ecx, [rax + 0x40]
			jrcxz	check_dbg					; If no heap protection check for debugger
			
get_prot:	mov		ecx, [rcx + 8]
			test	[rax + 0x7c], ecx
			cmovne	ebx, [rax + 0x88]			; Get heap key
			
			; Check for 0xABABABABABABABAB, 0xABABABABABABABAB at the end of block
check_dbg:	mov		eax, <heap ptr> 			; TODO: heap ptr
			movzx	edx, WORD [rax - 8]			; size
			xor		dx, bx
			add		edx, edx
			movzx	ecx, BYTE [rax + rbp - 1]	; overhead
			sub		eax, ecx
			lea		edi, [rdx * 8 + rax]
			mov 	al, 0xAB
			mov		cl, 16
			repe	scasb
			setz	al							; TODO: check setz/setnz