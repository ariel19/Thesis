[bits 32]
; @ebx = kernel32!CheckRemoteDebuggerPresent


			xor		eax, eax
			test	ebx, ebx
			jz		endf
			
			push	eax
			push	esp
			push	-1
			call	ebx
			pop 	eax
endf:
			