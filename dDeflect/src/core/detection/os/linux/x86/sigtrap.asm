call signal_start
nop
nop
nop 
nop

signal_start:
pop esi

call init_signal
signal_handler:
	push ebp
	mov ebp, esp

	xor eax, eax
	mov [esi], eax

	pop ebp
	ret

init_signal:
	pop ecx ; handler pointer
	mov ebx, 5 ; SIGTRAP
	mov eax, 48 ; sys signal
	int 0x80

	int3 ; debugger trap

	xor eax, eax
	xor ecx, ecx
	dec ecx
	mov ebx, [esi]
	cmp ebx, 0x90909090
	cmove eax, ecx
