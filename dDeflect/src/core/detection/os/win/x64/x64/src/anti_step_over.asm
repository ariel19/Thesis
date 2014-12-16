[bits 32]

	xor ecx, ecx
	inc ecx,
	mov esi, l1
	lea edi, [esi + 1]
	rep movsb
l1: mov al, 0x90
l2: cmp al, 0xcc
	setz al
