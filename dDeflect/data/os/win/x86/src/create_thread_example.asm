[bits 32]

			push	edx
			
			call	xxx
			db		"kernel32.dll", 0x00
xxx:
			call	eax				; LoadLibrary("kernel32.dll");
			
			pop		edx				; GetProcAddr
			call	yyy
			db		"CreateThread", 0x00
yyy:
			push	eax
			call	edx				; GetProcAddr
			
			mov		edx, eax
			mov		edi, 0xDEADBEEF