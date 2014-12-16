[bits 32]

			mov 	ebx, phe_size			; PROCESS_HEAP_ENTRY buffer
			
find_blk:	push	ebx
			mov		eax, [fs:0x30]			; PEB
			push	DWORD [eax + 0x18]		; Process Heap base
			call	HeapWalk/RtlWalkHeap	; TODO: from register?
			
			cmp		WORD [ebx + 0x0A], 4	; Try to find allocated block
			jne		find_blk				; (wFlags != PROCESS_HEAP_ENTRY_BUSY)
			
			mov		edi, [ebx]				; Data ptr
			add		edi, [ebx + 4]			; Data size
			
			mov 	al, 0xAB
			mov		ecx, 8
			repe	scasb
			setz	al
			
			jmp		the_end
			
phe_size:	times	0x1C db 0				; PROCESS_HEAP_ENTRY

the_end:	nop