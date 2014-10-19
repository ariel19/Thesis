[bits 64]

			mov 	rbx, phe_size			; PROCESS_HEAP_ENTRY buffer
			
find_blk:	push	rbx
			pop		rdx
			push	0x60
			pop		rsi
			gs:lodsq						; PEB
			mov		ecx, [rax + 0x30]		; Process heap base
			call	HeapWalk/RtlWalkHeap	; TODO: from register?
			
			cmp		WORD [rbx + 0x0E], 4	; Try to find allocated block
			jne		find_blk				; (wFlags != PROCESS_HEAP_ENTRY_BUSY)
			
			mov		edi, [rbx]				; Data ptr
			add		edi, [rbx + 8]			; Data size
			
			mov 	al, 0xAB
			mov		r	cx, 16
			repe	scasb
			setz	al
			
			jmp		the_end
			
phe_size:	times	0x28 db 0				; PROCESS_HEAP_ENTRY

the_end:	nop