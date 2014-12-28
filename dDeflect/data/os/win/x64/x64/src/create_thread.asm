[bits 64]
; Adres CreateThread w R10
; Adres funkcji w¹tku w R8

; HANDLE WINAPI CreateThread(
;   _In_opt_   LPSECURITY_ATTRIBUTES lpThreadAttributes,
;   _In_       SIZE_T dwStackSize,
;   _In_       LPTHREAD_START_ROUTINE lpStartAddress,
;   _In_opt_   LPVOID lpParameter,
;   _In_       DWORD dwCreationFlags,
;   _Out_opt_  LPDWORD lpThreadId
; );

			xor		rcx, rcx
			mov		rdx, rcx
			xor		r9, r9
			
			push	rcx
			push	rdx
			
			sub		rsp, 0x20
			
			call	r10		; CreateThread
			
			add		rsp, 0x20