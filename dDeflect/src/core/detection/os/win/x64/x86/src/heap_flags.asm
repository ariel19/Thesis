[bits 32]

call 0x12345678 ; resolve GetVersion function address
cmp al, 0x6
mov edx, [ds:0x30] ; PEB
; get heap
mov ecx, [edx + 0x10] ; get proc heap base
sbb edx, edx

xor eax, eax
xor edx, edx
and dl, 0xa4

; ForceFlags
cmp dword [ecx + edx + 0x74], 0x40000060
setz al
