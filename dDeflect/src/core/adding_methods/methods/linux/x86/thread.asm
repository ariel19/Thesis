[bits 32]

; method responsible for new thread creation

; to create a thread we need to use following values for flags
; CLONE_VM (0x100), CLONE_FS(0x200), CLONE_FILES(0x400), CLONE_SIGHAND(0x800), CLONE_THREAD (0x1000)

; rdi - address of thread_func
; rsi - thread parameter

push eax
push ebx
push ecx
push edx
push esi
push edi

; assume that stack grows DOWNWARDS => add to the allocated memory with dmalloc size of allocated memory :)

mov ebx, 0x1f0 ; or'ed flags
mov ecx, 0xffffffff ; address of new stack pointer
xor edx, edx ; parent tid
xor esi, esi ; child tid
xor edi, edi ; regs
mov eax, 120 ; syscall clone
int 0x80 ; syscall

or eax, 0
jnz _parent

push 0xeeeeeeee ; address of thread func
push 0 ; argument for thread func
ret

_parent:
pop edi
pop esi
pop edx
pop ecx
pop ebx
pop eax
