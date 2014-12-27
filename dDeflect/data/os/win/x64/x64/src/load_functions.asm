[bits 64]
; Znajdowanie adresów funkcji LoadLibrary i GetProcAddress

			call	load_k_str
			db		"KERNEL"
load_k_str:
			pop		rsi
			
			push	0x60
			pop		rsi
			gs		lodsq					; PEB
			mov		rax, [rax + 0x18]		; PEB.Ldr
			mov		rax, [rax + 0x20]		; PEB.Ldr.InMemoryOrderModuleList
			
enum_dlls:
			mov		rdx, [rax + 0x50]		; Pobranie nazwy
			test	rdx, rdx
			jz		not_found				; Po przejrzeniu calej listy nie znaleziono
			
			xor		rcx, rcx
			
scmp:
			cmp		rcx, 6					; 6 liter sie zgadza = KERNEL pasuje
			je		test32
			
			movzx	rbx, BYTE [rsi + rcx]	; Pobranie litery z KERNEL
			cmp		bl, [rdx + 2 * rcx]		; Pobranie litery z nazwy
			je		l_ok
			
			add		bl, 32					; Zamiana na male litery
			cmp		bl, [rdx + 2 * rcx]		; Pobranie litery z nazwy
			je		l_ok
			jmp		inv_name
			
l_ok:
			inc		rcx						; licznik++
			jmp		scmp
			
test32:
			add		rdx, 12					; 6 * 2
			cmp		BYTE [rdx], 0x33		; Porownanie z '3'
			
			add		rdx, 2
			cmp		BYTE [rdx], 0x32		; Porownanie z '2'
			jne		inv_name
			je		found
			
; ---------------------------------------------
; Parsowanie dll
; ---------------------------------------------

found:
			mov		rax, [rax + 0x20]		; Adres kernel32.dll
			mov		r12, rax				; Zapamietujemy adres kernel32
			
			add		rax, [rax + 0x3C]		; e_lfanew
			mov		rax, [rax + 0x88]		; Export Table offset
			add		rax, r12				; Export Table address
			mov		r13, rax				; Zapamietujemy IMAGE_EXPORT_DIRECTORY
			
			; *****
			; TODO
			; *****
			
			
inv_name:
			mov		eax, [eax]
			jmp		enum_dlls
			
			
not_found:
			xor		rax, rax
			xor		rdx, rdx
			
endf:
			mov		[rbp + 24], rax
			mov		[rbp + 16], rdx