[bits 32]
; sigsegv


			xor		eax, esi
			xor		eax, ecx
			call	eax
			xor		ecx, ecx
			sub		ecx, 12
			add		ecx, 4
			mov		[ecx + 8], edx
			