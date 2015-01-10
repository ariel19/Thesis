mov ecx, (?^_^magic!sec_size^_^?)
call $+5
pop edi
add edi, 0xbadadaba ; calculate here an offset to text section begin

xor eax, eax
xor esi, esi
xor edx, edx

start:
cmp eax, ecx
jae test_checksum
mov dl, byte [edi + eax]
add esi, edx
inc eax
jmp start

test_checksum:

mov edi, (?^_^magic!sec_checksum^_^?)
xor eax, eax
xor ebx, ebx
dec ebx

cmp esi, edi

cmovne eax, ebx
