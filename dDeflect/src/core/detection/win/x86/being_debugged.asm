[bits 32]

			mov		edx, [fs:0x30]
			xor		eax, eax
			cmp		BYTE [edx + 0x02], al
			setne	al
			