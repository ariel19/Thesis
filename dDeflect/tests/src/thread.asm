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

; xor ebx, ebx ; first arg to mmap
; mov ecx, 0x1000 ; 1st arg in stack (size)
; mov edx, 0x3 ; PROT_READ | PROT_WRITE
; mov esi, 0x122 ; MAP_PRIVATE | MAP_ANON | MAM_GROWSDOWN <--- as stack grows down
; xor edi, edi
; dec edi ; -1
; xor ebp, ebp
; mov eax, 90 ; syscall for mmap
; int 0x80 ; syscall

; xor ebx, ebx ; get current break address
; mov eax, 45 ; syscall for brk
; int 0x80 ; syscall

; mov ebx, eax
; add ebx, 0x1000 ; should be as parameter for wrapper or whatever
; mov eax, 45 ; syscall for brk
; int 0x80 ; syscall

; add eax, 0x1000 ; stack grows downwards

mov ebx, 0x10f00 ; or'ed flags
; mov ecx, eax ; address of new stack pointer
xor ecx, ecx
xor edx, edx ; parent tid
xor esi, esi ; child tid
xor edi, edi ; regs
mov eax, 120 ; syscall clone
int 0x80 ; syscall

or eax, 0
jnz _parent

; push 0xeeeeeeee ; address of thread func
; push 0 ; argument for thread func
; ret

; dummy write for thread work

new_thread:

; process id
;mov eax, 20 ; getpid
;int 0x80
;push eax
;mov ecx, esp
;mov ebx, 1
;mov edx, 4
;mov eax, 4
;int 0x80
;pop eax

; check if is being debugged
; ptrace parametres
mov ebx, 0 ; PTRACE_TRACEME
mov ecx, 0
mov edx, 1
mov esi, 0
mov eax, 26 ; ptrace syscall
int 0x80 ; syscall

; eax == return value of ptrace
; use a wrapper for return value
cmp eax, 0
jge __nondbg

call _dbg
db 'debugged!', 0xa, 0
_dbg:
mov ebx, 1
pop ecx
mov edx, 10
mov eax, 4; write syscall
int 0x80 ;

mov ebx, 0 ; error code
mov ecx, 9 ; SIGKILL
mov eax, 37; kill syscall
int 0x80; syscall

__nondbg:
call _nondbg
db 'non-debugged!', 0xa, 0
_nondbg:
mov ebx, 1
pop ecx
mov edx, 14
mov eax, 4 ; syscall write
int 0x80

; ==========
; SLEEP.....
; ==========
push 0 ; value should be a parameter
push 5 ; value should be a parameter
mov ebx, esp
xor ecx, ecx
mov eax, 162 ; syscall nanosleep 
int 0x80 ; syscall
pop eax
pop eax
jmp new_thread

_parent:
pop edi
pop esi
pop edx
pop ecx
pop ebx
pop eax
