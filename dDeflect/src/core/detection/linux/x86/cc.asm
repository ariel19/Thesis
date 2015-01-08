cld
mov ecx, (?^_^vsize^_^?)
mov edi, (?^_^vaddr^_^?)
xor eax, eax
mov al, 0xcc
repne scasb

xor eax, eax
xor ebx, ebx
dec ebx

test ecx, ecx
cmovne eax, ebx  
