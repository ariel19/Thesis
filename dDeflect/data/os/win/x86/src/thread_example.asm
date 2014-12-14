[bits 32]

		push 	edx					; GetProcAddr
		push 	eax					; LoadLibrary
		
		sub		esp, 16
		
		call 	xxx
		db 		"user32.dll", 0x00
xxx:
		call	[ebp - 8]			; LoadLibrary
		mov		[ebp - 16], eax
		
		call 	yyy
		db		"kernel32.dll", 0x00
yyy:
		call	[ebp - 8]			; LoadLibrary
		mov		[ebp - 12], eax
		
		call	zzz
		db		"MessageBoxA", 0x00
zzz:
		push	DWORD [ebp - 16]	; user32.dll
		call	[ebp - 4]			; GetProcAddr
		mov		[ebp - 20], eax
		
		call	vvv
		db		"Sleep", 0x00
vvv:
		push	DWORD [ebp - 12]	; kernel32.dll
		call	[ebp - 4]			; GetProcAddr
		mov		[ebp - 24], eax
		
loopt:
		push	10000
		call	[ebp - 24]			; Sleep(10000)
		
		push	0x00
		push	0x00
		call	ttt
		db		":)", 0x00
ttt:
		push	0x00
		call	[ebp - 20]			; MessageBoxA
		
		jmp		loopt
		