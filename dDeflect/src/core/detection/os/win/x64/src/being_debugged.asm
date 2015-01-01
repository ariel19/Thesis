[bits 64]

			push	0x60
			pop		rsi
			gs		lodsq
			xor		rcx, rcx
			cmp		BYTE [rax + 0x02], cl
			setne	cl
			