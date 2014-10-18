[bits 64]

push 0x60
pop rsi
gs lodsq ; PEB stores value into RAX
mov dl, [rsi * 2 + rax - 0x14] ; NtGlobalFlag
xor rax, rax
and dl, 0x70
cmp dl, 0x70
setz al
