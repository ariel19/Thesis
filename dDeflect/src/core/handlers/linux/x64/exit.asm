mov rax, 60; exit syscall
mov rdi, (?^_^ret^_^?) ; error code
syscall ; syscall
