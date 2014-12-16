[bits 32]

mov ecx, [fs:0x30] ; PEB
xor eax, eax
mov dl, [ecx + 0x68] ; NtGlobalFlag
and dl, 0x70
cmp dl, 0x70
setz al
