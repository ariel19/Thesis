[bits 64]

push rax
push rdi
push rsi

push 0 ; value should be a parameter
push 1 ; value should be a parameter
mov rdi, rsp
xor rsi, rsi
mov rax, 35 ; syscall nanosleep 
syscall ; syscall

pop rax ; dummy pop's
pop rax ; dummy pop's

pop rsi
pop rdi
pop rax
