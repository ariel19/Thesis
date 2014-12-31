mov rax, 231; exit syscall
mov rdi, (?^_^ret^_^?) ; error code
syscall ; syscall
