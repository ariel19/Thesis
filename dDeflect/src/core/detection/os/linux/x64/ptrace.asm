[bits 64]

; save registers
push rax
push rdi
push rsi
push rdx
push r10

mov rax, 101 ; syscall ptrace
; ptrace parametres
mov rdi, 0 ; PTRACE_TRACEME
mov rsi, 0
mov rdx, 1
mov r10, 0

syscall ; syscall

; rax == return value of ptrace
; use a wrapper for return value
cmp rax, 0
jge short old_stack
mov rax, 60; exit syscall
mov rdi, 127; error code
syscall ; syscall

old_stack:
pop r10
pop rdx
pop rsi
pop rdi
pop rax
