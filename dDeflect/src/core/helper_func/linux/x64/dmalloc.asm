[bits 64]

push rax
push rdi

mov rax, 12 ; syscall for brk
xor rdi, rdi ; get current break address
syscall ; syscall

mov rdi, rax
add rdi, 0x100 ; should be as parameter for wrapper or whatever
mov rax, 12 ; syscall for brk
syscall ; syscall

; now we have a value in rax ==> write it somewhere (info from wrapper)

pop rdi
pop rax
