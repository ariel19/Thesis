[bits 32]

			xor		ecx, ecx
			inc		ecx
			call	xxx
xxx:
			pop		esi
			add		esi, 9
			lea 	edi, [esi + 1]
			rep 	movsb
l1: 		mov 	dl, 0x90
			xor		eax, eax
l2: 		cmp 	dl, 0xcc
			setz 	al
