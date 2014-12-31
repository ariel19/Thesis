[bits 64]

dmmap:
	push rdi 
	push rsi
 	push rdx
	push r10
	push r8
	push r9

	mov rsi, rdi ; rdi - 1st arg (size)
	xor rdi, rdi
	mov rdx, 0x3 ; PROT_READ | PROT_WRITE
	mov r10, 0x122 ; MAP_PRIVATE | MAP_ANON | MAM_GROWSDOWN <--- as stack grows down
	xor r8, r8
	dec r8		; -1
	xor r9, r9
	mov rax, 9 ; syscall for mmap
	syscall ; syscall
	pop r9
	pop r8
	pop r10
	pop rdx
	pop rsi
	pop rdi

	ret
	
