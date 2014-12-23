[bits 32]

l1: call l3
l2: mov al, 0x1 ; debugger is present 
l3: mov al, 0xc3
	mov edi, l3
	or ecx, -1
	rep stosb ; during normal execution exception will be generated
