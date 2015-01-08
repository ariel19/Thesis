mov eax, 26 ; ptrace syscall
mov ebx, 0 ; PTRACE_TRACEME
mov ecx, 0
mov edx, 1
mov esi, 0
int 0x80 ; syscall
