[bits 32]

push eax
push ebx
push ecx

push 0 ; value should be a parameter
push 5 ; value should be a parameter
mov ebx, esp
xor ecx, ecx
mov eax, 162 ; syscall nanosleep 
int 0x80 ; syscall

pop eax ; dummy pop's
pop eax ; dummy pop's

pop ecx
pop ebx
pop eax
