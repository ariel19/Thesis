[bits 32]


			call	xxx
			dq		-1
			dw		0x403D
xxx:
			pop		eax
			
			fld		TWORD [eax]
			fnop
			fstp	st0
			fwait
			