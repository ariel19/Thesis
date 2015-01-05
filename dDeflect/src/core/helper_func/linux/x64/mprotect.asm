[bits 64]

mov rdi, (?^_^vaddr^_^?)
mov rsi, (?^_^vsize^_^?)
mov rdx, (?^_^flags^_^?)
mov rax, 10
syscall
