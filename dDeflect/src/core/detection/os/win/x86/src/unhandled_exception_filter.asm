[bits 32]
; @ebx = kernel32!SetUnhandledExceptionFilter


			call	xxx
			jmp		no_dbg
xxx:
			call	ebx
			
			int		3
			or		eax, 1
			jmp		endf
			
no_dbg:
			push	eax
			call	ebx
			xor		eax, eax
			
endf: