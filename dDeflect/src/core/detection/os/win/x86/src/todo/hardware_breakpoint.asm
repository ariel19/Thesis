[bits 32]

	xor eax, eax
	push l1 ; handler address
	push dword [fs:eax] ; address of SEH structure
	mov [fs:eax], esp
	int 3 ; exception
l1:	; exception handler
