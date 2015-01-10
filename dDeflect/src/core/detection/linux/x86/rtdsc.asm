rdtsc            
mov ecx, eax
rdtsc
sub eax,ecx
xor ecx, ecx
dec ecx
cmp eax,0FFFh
mov eax, 0
cmovae eax, ecx
