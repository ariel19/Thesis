[bits 64]
; Znajdowanie adresów funkcji LoadLibrary i GetProcAddress
			
			push	0x60
			pop		rsi
			gs		lodsq							; PEB
			
			call	load_k_str
			db		"KERNEL"
load_k_str:
			pop		rsi
			
			mov		rax, [rax + 0x18]				; PEB.Ldr
			mov		rax, [rax + 0x20]				; PEB.Ldr.InMemoryOrderModuleList
			
enum_dlls:
			mov		rdx, [rax + 0x50]				; Pobranie nazwy
			test	rdx, rdx
			jz		not_found						; Po przejrzeniu calej listy nie znaleziono
			
			xor		rcx, rcx
			
scmp:
			cmp		rcx, 6							; 6 liter sie zgadza = KERNEL pasuje
			je		test32
			
			movzx	rbx, BYTE [rsi + rcx]			; Pobranie litery z KERNEL
			cmp		bl, [rdx + 2 * rcx]				; Pobranie litery z nazwy
			je		l_ok
			
			add		bl, 32							; Zamiana na male litery
			cmp		bl, [rdx + 2 * rcx]				; Pobranie litery z nazwy
			je		l_ok
			jmp		inv_name
			
l_ok:
			inc		rcx								; licznik++
			jmp		scmp
			
test32:
			add		rdx, 12							; 6 * 2
			cmp		BYTE [rdx], 0x33				; Porownanie z '3'
			jne		inv_name
			
			add		rdx, 2
			cmp		BYTE [rdx], 0x32				; Porownanie z '2'
			jne		inv_name
			je		found
			
inv_name:
			mov		eax, [eax]
			jmp		enum_dlls
			
; ---------------------------------------------
; Parsowanie dll
; ---------------------------------------------

found:
			mov		rax, [rax + 0x20]				; Adres kernel32.dll
			mov		r12, rax						; Zapamietujemy adres kernel32
			
			mov		r8d, [rax + 0x3C]				; e_lfanew
			add		rax, r8
			mov		eax, [rax + 0x88]				; Export Table offset
			add		rax, r12						; Export Table address
			mov		r13, rax						; Zapamietujemy IMAGE_EXPORT_DIRECTORY
			
			mov		ecx, [rax + 0x18]				; Liczba nazw funkcji
			mov		eax, [rax + 0x20]				; Offset tablicy nazw
			add		rax, r12						; Adres tablicy nazw
			
			call	load_f_str
			db		"GetProcAddress"
load_f_str:
			pop		rsi
			
enum_fun:
			mov		edi, [rax + 4 * rcx - 4]		; Offset nazwy
			add		rdi, r12
			xor		rbx, rbx						; Licznik liter
			
fcmp:
			cmp		rbx, 14							; Caly string sie zgadza, funkcja odnaleziona
			je		f_found
			
			movzx	rdx, BYTE [rdi + rbx]
			cmp		dl, [rsi + rbx]
			jne		next_f
			
			inc		rbx
			jmp		fcmp
			
next_f:
			sub		rcx, 1							; Kolejna funkcja
			jnz		enum_fun
			jmp		not_found						; Funkcja nie znaleziona
			
f_found:
			mov		edi, [r13 + 0x24]				; Offset AddressOfNameOrdinals
			add		rdi, r12						; Adres AddressOfNameOrdinals
			
			movzx	rdi, WORD [rdi + 2 * rcx - 2]	; Index adresu funkcji
			
			mov		ecx, [r13 + 0x1C]				; Offset AddressOfFunctions
			add		rcx, r12						; Adres
			
			mov		r13d, DWORD [rcx + 4 * rdi]		; Offset adresu funkcji
			add		r13, r12						; Adres GetProcAddr
			
; --------------------------------------------
; Znajdowanie adresu LoadLibrary
; --------------------------------------------

			mov		rcx, r12
			call	load_l_str
			db		"LoadLibraryA", 0x00
load_l_str:
			pop		rdx
			sub		rsp, 0x20
			call	r13
			add		rsp, 0x20
			
			jmp		endf
			
			
not_found:
			xor		rax, rax
			xor		r13, r13
			
endf:
			mov		[rbp + 24], rax
			mov		[rbp + 16], r13