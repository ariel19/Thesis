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

mov rdi,  0x10f00 ; or'ed flags
mov rsi, 0 ; address of new stack pointer
; TODO: why we don't use rdx???????
;xor r10, r10 ; parent tid
;xor r8, r8 ; child tid
;xor r9, r9 ; regs
xor rdx, rdx
xor r10, r10
xor r8, r8
mov rax, 56 ; syscall clone
syscall ; syscall

or rax, 0
jnz _parent

new_thread:

; PTRACE
xor rdi, rdi
xor rsi, rsi
xor rdx, rdx
inc rdx
xor r10, r10
mov rax, 101 ; ptrace syscall
syscall

cmp rax, 0
jge __nondbg

call _dbg
db 'debugged!', 0xa, 0
_dbg:
mov rdi, 1
pop rsi
mov rdx, 10
mov rax, 1 ; syscall write
syscall

; exit_group call
mov rdi, 127
mov rax, 231 ; exit_group syscall
syscall

__nondbg:
call _nondbg
db 'non-debugged!', 0xa, 0
_nondbg:
mov rdi, 1
pop rsi
mov rdx, 14
mov rax, 1 ; syscall write
syscall

; ==========
; SLEEP.....
; ==========
push 0 ; value should be a parameter
push 5 ; value should be a parameter
mov rdi, rsp
xor rsi, rsi
mov rax, 35 ; syscall nanosleep 
syscall ; syscall
pop rax
pop rax

jmp new_thread

_parent:
pop r15
pop r14
pop r9
pop r8
pop r10
pop rsi
pop rdi
pop rax
