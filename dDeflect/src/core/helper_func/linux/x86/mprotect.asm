[bits 32]

mov ebx, (?^_^vaddr^_^?)
mov ecx, (?^_^vsize^_^?)
mov edx, (?^_^flags^_^?)
mov eax, 125
int 0x80
