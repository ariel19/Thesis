mov eax, 1; exit syscall
mov ebx, (?^_^ret^_^?) ; error code
int 0x80; syscall
