[bits 32]

			mov		eax, [fs:0x30]				; PEB
			mov		eax, [eax + 0x0C]			; PEB.Ldr
			mov		eax, [eax + 0x0C]			; PEB.Ldr.InLoadOrderModuleList
			
			mov		eax, [eax + 0x18]			; ImageBase
			
			mov		edx, [eax + 0x3C]			; e_lfanew
			add		edx, eax
			
			mov		edx, [edx + 0x28]
			add		edx, eax
			
			xor		eax, eax
			cmp		byte [edx], 0xCC
			sete	al
			