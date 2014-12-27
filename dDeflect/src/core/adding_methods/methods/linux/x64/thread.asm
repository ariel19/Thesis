mov rdi,  0x10f00 ; or'ed flags
mov rsi, 0 ; address of new stack pointer
xor rdx, rdx
xor r10, r10
xor r8, r8
mov rax, 56 ; syscall clone
syscall ; syscall

or rax, 0
jnz _parent

new_thread:

	mov rdi,  0x10f00 ; or'ed flags
	mov rsi, 0 ; address of new stack pointer
	xor rdx, rdx
	xor r10, r10
	xor r8, r8
	mov rax, 56 ; syscall clone
	syscall ; syscall

	or rax, 0
	jnz __ntroutine

		; debugger detection

		(rsj?^_^ddetectionmethod^_^?rsj)

		cmp (?^_^ddret^_^?), 0
		jge __nondbg

		call _dbg
		db 'debugged!!!!!', 0xa, 0
_dbg:
		mov rdi, 1
		pop rsi
		mov rdx, 14
		mov rax, 1 ; syscall write
		syscall

		(rsj?^_^ddetectionhandler^_^?rsj)

__nondbg:
		call _nondbg
		db 'non-debugged!', 0xa, 0
_nondbg:
		mov rdi, 1
		pop rsi
		mov rdx, 14
		mov rax, 1 ; syscall write
		syscall

		mov rdi, 178
		mov rax, 60 ; syscall exit
		syscall 

__ntroutine:

	mov r15, rax ; save thread id

	push (?^_^sleep1^_^?) ; value should be a parameter
	push (?^_^sleep2^_^?) ; value should be a parameter
	mov rdi, rsp
	xor rsi, rsi
	mov rax, 35 ; syscall nanosleep 
	syscall ; syscall
	pop rax
	pop rax

	mov rax, 39 ; getpid
	syscall

	; kill debugger detection thread
	mov rdi, r15
	mov rsi, 1 ; SIGHUP
	mov rdx, rax ; pid 
	mov rax, 234
	syscall

	jmp new_thread

_parent:
