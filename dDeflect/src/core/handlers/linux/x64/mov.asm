mov rcx, 0xffff800000000000 ; kernel space start address
mov rax, 0xcc
mov [ecx], rax
