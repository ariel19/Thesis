rdtsc            
mov rcx, rax
rdtsc
sub rax,rcx
xor rcx, rcx
dec rcx
cmp rax,0FFFh
mov rax, 0
cmovae rax, rcx
