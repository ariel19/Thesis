cld
mov rcx, (?^_^vsize^_^?)
mov rdi, (?^_^vaddr^_^?)
xor rax, rax
mov al, 0xcc
repne scasb

xor rax, rax
xor rbx, rbx
dec rbx

test rcx, rcx
cmovne rax, rbx  
