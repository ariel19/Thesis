[bits 32]
; Adres CreateThread w EDX
; Adres funkcji wątku w EDI

; HANDLE WINAPI CreateThread(
;   _In_opt_   LPSECURITY_ATTRIBUTES lpThreadAttributes,
;   _In_       SIZE_T dwStackSize,
;   _In_       LPTHREAD_START_ROUTINE lpStartAddress,
;   _In_opt_   LPVOID lpParameter,
;   _In_       DWORD dwCreationFlags,
;   _Out_opt_  LPDWORD lpThreadId
; );

			xor		eax, eax
			
			push	eax		; lpThreadId	
			push	eax		; dwCreationFlags
			push	eax		; lpParameter
			push	edi		; lpStartAddress
			push	eax		; dwStackSize
			push	eax		; lpThreadAttributes
			
			call	edx		; CreateThread