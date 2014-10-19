[bits 32]

			call	GetVersion					; TODO: from register
			cmp		al, 6
			jae		vista						; If vista or above do not check!
			
			push	eax							; Allocate 4 bytes
			mov		eax, esp
			
			push	0
			push	4
			push 	eax
			push	9							; ThreadQuerySetWin32StartAddress
			push	-2							; Current Thread
			
			call	NtQueryInformationThread	; TODO: from register
			
			pop		edx							; Get returned value from buffer
			
			test	edx, edx
			je		is_dbg						; Windows XP
			
			; Get kernel32.dll base address
			xor		ecx, ecx
			mov		ecx, fs:[0x30]				; PEB
			mov		ecx, [ecx + 0x0C]			; PEB->Ldr
			mov		ecx, [ecx + 0x1C]			; PEB->Ldr.InInitializationOrderModuleList.Flink (1st entry)
			mov		ecx, [ecx]					; next entry
			mov 	ecx, [ecx + 0x08]			; kernel32.dll Base address
			
			cmp		edx, ecx
			jnb		is_dbg						; Windows NT/2000
			
			jmp 	the_end
			
vista:		xor 	eax, eax
			jmp		the_end
			
is_dbg:		setz 	al
the_end:	nop