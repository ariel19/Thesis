[bits 32]
; @eax = ntdll!NtQueryInformationProcess


			push	0
			
			push	0
			push	4				; ProcessInformationLength
			push	esp
			add		DWORD [esp], 8
			push	7				; ProcessDebugPort
			push	-1				; Current Process
			call	eax
			
			pop 	eax
			inc		eax
			not		eax
			