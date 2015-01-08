[bits 32]

push eax
push ebx

mov eax, 45 ; syscall for brk
xor ebx, ebx ; get current break address
int 0x80 ; syscall

mov ebx, eax
add ebx, 0x100 ; should be as parameter for wrapper or whatever
mov eax, 45 ; syscall for brk
int 0x80 ; syscall

; now we have a value in eax ==> write it somewhere (info from wrapper)

pop ebx
pop eax
