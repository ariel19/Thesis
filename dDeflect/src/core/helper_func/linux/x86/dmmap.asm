[bits 32]

dmmap:
	push eax 
	push ebx
 	push ecx
	push edx
	push esi
	push edi
	push ebp

	xor ebx, ebx ; first arg to mmap
	pop ecx ; 1st arg in stack (size)
	mov edx, 0x3 ; PROT_READ | PROT_WRITE
	mov esi, 0x122 ; MAP_PRIVATE | MAP_ANON | MAM_GROWSDOWN <--- as stack grows down
	xor edi, edi
	dec edi ; -1
	xor ebp, ebp
	mov eax, 90 ; syscall for mmap
	int 0x80 ; syscall

	pop ebp
	pop edi
 	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax

	ret
	
