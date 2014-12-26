mov ebx, 0x10f11 ; or'ed flags
xor ecx, ecx
xor edx, edx ; parent tid
xor esi, esi ; child tid
xor edi, edi ; regs
mov eax, 120 ; syscall clone
int 0x80 ; syscall

or eax, 0
jnz _parent

new_thread:

__nondbg:
call _nondbg
db 'justntinfo!!!', 0xa, 0
_nondbg:
mov ebx, 1
pop ecx
mov edx, 14
mov eax, 4 ; syscall write
int 0x80

push (?^_^sleep1^_^?) ; value should be a parameter
push (?^_^sleep2^_^?) ; value should be a parameter
mov ebx, esp
xor ecx, ecx
mov eax, 162 ; syscall nanosleep 
int 0x80 ; syscall
pop eax
pop eax
jmp new_thread

_parent:
