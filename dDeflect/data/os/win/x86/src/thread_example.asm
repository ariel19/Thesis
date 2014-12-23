[bits 32]
		
		push	0x00
		push	0x00
		call	ttt
		db		":)", 0x00
ttt:
		push	0x00
		call	eax
		
		