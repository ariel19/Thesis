[bits 64]

			xor		rbp, rbp
			enter	0x20, 0
			
			push	4
			pop		r9
			push	rbp
			pop		r8
			push	9							; ThreadQuerySetWin32StartAddress
			pop		rdx
			push	-2							; Current Thread
			pop		rcx
			call 	NtQueryInformationThread	; TODO: from register
			
			leave
			test	rbp, rbp
			setz	al