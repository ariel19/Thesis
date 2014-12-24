[bits 32]
; undefined instruction

			xor		eax, eax
			add		eax, ecx
			call	aa
			mov		[edx + 4*esi + 16], edi
aa:
			ud2
			add		esp, 64