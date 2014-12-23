[bits 32]

; save registers
push eax
push ebx
push ecx
push edx
push esi

mov eax, 26 ; ptrace syscall
; ptrace parametres
mov ebx, 0 ; PTRACE_TRACEME
mov ecx, 0
mov edx, 1
mov esi, 0

int 0x80 ; syscall

; eax == return value of ptrace
; use a wrapper for return value
cmp eax, 0
jge old_stack
mov eax, 1; exit syscall
mov ebx, 127; error code
int 0x80; syscall

old_stack:
pop esi
pop edx
pop ecx
pop ebx
pop eax
