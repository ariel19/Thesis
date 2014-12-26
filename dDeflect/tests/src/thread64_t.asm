mov rdi,  0x10f11 ; or'ed flags
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

__nondbg:
call _nondbg
db 'justntinfo!!!', 0xa, 0
_nondbg:
mov rdi, 1
pop rsi
mov rdx, 14
mov rax, 1 ; syscall write
syscall

push (?^_^sleep1^_^?) ; value should be a parameter
push (?^_^sleep2^_^?) ; value should be a parameter
mov rdi, rsp
xor rsi, rsi
mov rax, 35 ; syscall nanosleep 
syscall ; syscall
pop rax
pop rax

jmp new_thread

_parent:
