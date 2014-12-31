mov eax, 252 ; exit syscall
mov ebx, (?^_^ret^_^?) ; error code
int 0x80 ; syscall
