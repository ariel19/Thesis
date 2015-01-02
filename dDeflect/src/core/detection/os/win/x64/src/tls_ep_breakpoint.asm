[bits 64]

			push	0x60
			pop		rsi
			gs		lodsq						; PEB
			mov		rax, [rax + 0x18]			; PEB.Ldr
			mov		rax, [rax + 0x10]			; PEB.Ldr.InLoadOrderModuleList
			
			mov		rax, [rax + 0x30]			; ImageBase
			
			mov		edx, DWORD [rax + 0x3C]		; e_lfanew
			add		rdx, rax
			
			mov		edx, [rdx + 0x28]
			add		rdx, rax
			
			xor		rax, rax
			cmp		byte [edx], 0xCC
			sete	al
			