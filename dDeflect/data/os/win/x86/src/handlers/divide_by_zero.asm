[bits 32]
; divide by zero

			mov		edx, ecx
			mov		esi, ecx
			xor		esi, edx
			div		esi
			