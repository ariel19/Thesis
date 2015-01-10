mov rcx, (?^_^magic!sec_size^_^?)
call $+5
pop rdi
add rdi, 0xbadadaba ; calculate here an offset to text section begin

xor rax, rax
xor rsi, rsi
xor rdx, rdx

start:
cmp rax, rcx
jae test_checksum
mov dl, byte [rdi + rax]
add rsi, rdx
inc rax
jmp start

test_checksum:

mov rdi, (?^_^magic!sec_checksum^_^?)
xor rax, rax
xor rbx, rbx
dec rbx

cmp rsi, rdi

cmovne rax, rbx
