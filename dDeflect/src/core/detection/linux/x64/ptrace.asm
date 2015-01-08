mov rax, 101 ; syscall ptrace
mov rdi, 0 ; PTRACE_TRACEME
mov rsi, 0
mov rdx, 1
mov r10, 0
syscall ; syscall
