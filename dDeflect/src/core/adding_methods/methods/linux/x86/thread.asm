mov ebx, 0x10f00 ; or'ed flags
xor ecx, ecx
xor edx, edx ; parent tid
xor esi, esi ; child tid
xor edi, edi ; regs
mov eax, 120 ; syscall clone
int 0x80 ; syscall

or eax, 0
jnz _parent

new_thread:

	mov ebx, 0x10f00 ; or'ed flags
	xor ecx, ecx
	xor edx, edx ; parent tid
	xor esi, esi ; child tid
	xor edi, edi ; regs
	mov eax, 120 ; syscall clone
	int 0x80 ; syscall

	or eax, 0
	jnz __ntroutine

		; debugger detection

		(rsj?^_^ddetectionmethod^_^?rsj)

		cmp (?^_^ddret^_^?), 0
		jge __nondbg

		call _dbg
		db 'debugged!!!!!', 0xa, 0
_dbg:
		mov ebx, 1
		pop ecx
		mov edx, 14
		mov eax, 4 ; syscall write
		int 0x80

		(rsj?^_^ddetectionhandler^_^?rsj)

__nondbg:
		call _nondbg
		db 'non-debugged!', 0xa, 0
_nondbg:
		mov ebx, 1
		pop ecx
		mov edx, 14
		mov eax, 4 ; syscall write
		int 0x80

		mov ebx, 178
		mov eax, 1 ; syscall exit
		int 0x80

__ntroutine:

	push eax ; save thread id	

	push (?^_^sleep1^_^?) ; value should be a parameter
	push (?^_^sleep2^_^?) ; value should be a parameter
	mov ebx, esp
	xor ecx, ecx
	mov eax, 162 ; syscall nanosleep 
	int 0x80 ; syscall
	pop eax
	pop eax

	mov eax, 20 ; getpid
	int 0x80

	; kill debugger detection thread
	pop ebx ; thread id
	mov ecx, 1 ; SIGHUP
	mov edx, eax; pid
	mov eax, 270; tgkill
	int 0x80 

jmp new_thread

_parent:
