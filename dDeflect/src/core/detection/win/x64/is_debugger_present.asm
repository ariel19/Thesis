[bits 64]
; @rax = kernel32!IsDebuggerPresent

			sub		rsp, 0x20
			call	rax
			add		rsp, 0x20
			