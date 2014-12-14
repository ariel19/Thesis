[bits 64]

; method is responsible for new thread creation

; to create a thread we need to use following values for flags
; CLONE_VM (0x100), CLONE_FS(0x200), CLONE_FILES(0x400), CLONE_SIGHAND(0x800), CLONE_THREAD (0x1000)

; rdi - address of thread_func
; rsi - thread parameter

push rax
push rdi
push rsi
push r10
push r8
push r9
push r14
push r15

; assume that stack grows DOWNWARDS => add to the allocated memory with dmalloc size of allocated memory :)

mov rdi,  0x10f11 ; or'ed flags
mov rsi, 0 ; address of new stack pointer, prbly should be calculated :)
; TODO: why we don't use rdx???????
xor r10, r10 ; parent tid
xor r8, r8 ; child tid
xor r9, r9 ; regs
mov rax, 56 ; syscall clone
syscall ; syscall

or rax, 0
jnz _parent

; ============
; THREAD BODY 
; ============

_parent:
pop r15
pop r14
pop r9
pop r8
pop r10
pop rsi
pop rdi
pop rax

; JUMP HERE
