[bits 32]
; Znajdowanie adresów funkcji LoadLibrary i GetProcAddress
; out edx = &GetProcAddress
; out eax = &LoadLibrary

			call	load_k_str
			db		"KERNEL"
load_k_str:
			pop		esi
			
			mov		eax, [fs:0x30]				; PEB
			mov		eax, [eax + 0x0C]			; PEB.Ldr
			mov		eax, [eax + 0x14]			; PEB.Ldr.InMemoryOrderModuleList

enum_dlls:	
			mov		edx, [eax + 0x28]			; Pobranie nazwy dll
			test	edx, edx
			jz		not_found					; Jezeli NULL to nie znaleziono - cala lista
			
			xor		ecx, ecx
			
scmp:		
			cmp		ecx, 6						; Jezeli 6 liter sie zgadzalo to KERNEL pasuje
			je		test32
			
			movzx	ebx, BYTE [esi + ecx]		; Pobieranie litery z KERNEL
			cmp		bl, [edx + 2 * ecx]			; Pobieranie litery z nazwy
			je		l_ok
			
			add		bl, 32						; Zamiana na male litery
			cmp		bl, [edx + 2 * ecx]			; Porownanie z litera z nazwy
			je		l_ok
			jmp		inv_name
			
l_ok:
			inc		ecx							; licznik++
			jmp		scmp
			
test32:
			add		edx, 12
			cmp		BYTE [edx], 0x33			; Porownanie z '3'
			jne		inv_name
			
			add		edx, 2
			cmp		BYTE [edx], 0x32			; Porownanie z '2'
			jne		inv_name
			je		found
			
inv_name:
			mov		eax, [eax]
			jmp		enum_dlls
			
; ---------------------------------------------
; Parsowanie dll
; ---------------------------------------------

found:
			mov		eax, [eax + 0x10]			; Adres kernel32.dll
			push	eax							; Zapamietujemy adres
			
			add		eax, [eax + 0x3c]			; e_lfanew
			mov		eax, [eax + 0x78]			; Export table offset
			add		eax, [esp]					; Export table address (IMAGE_EXPORT_DIRECTORY)
			push	eax							; Zapamietujemy IMAGE_EXPORT_DIRECTORY
			
			mov		ecx, [eax + 0x18]			; Liczba nazw funkcji
			mov		eax, [eax + 0x20]			; Offset tablicy nazw
			add		eax, [esp + 4]				; Adres tablicy nazw
			
			call	load_f_str
			db		"GetProcAddress"
load_f_str:
			pop		esi
			
enum_fun:
			mov		edi, [eax + 4 * ecx - 4]	; Offset nazwy
			add		edi, [esp + 4]
			xor		ebx, ebx					; Licznik liter
			
fcmp:
			cmp		ebx, 14						; Sprawdzono caly string, funkcja znaleziona
			je		f_found
			
			movzx	edx, BYTE [edi + ebx]
			cmp		dl, [esi + ebx]
			jne		next_f
			
			inc		ebx
			jmp		fcmp
			
next_f:			
			sub		ecx, 1						; Kolejna funkcja
			jnz		enum_fun
			jmp 	not_found_f					; Funkcja nie znaleziona
			
f_found:
			pop		edx							; IMAGE_EXPORT_DIRECTORY
			pop		eax							; Adres kernel32.dll
			
			mov		edi, [edx + 0x24]			; Offset AddressOfNameOrdinals
			add		edi, eax					; Adres AddressOfNameOrdinals
			
			movzx	edi, WORD [edi + 2 * ecx - 2]	; Index adresu funkcji
			
			mov		ecx, [edx + 0x1C]			; Offset AddressOfFunctions
			add		ecx, eax					; Adres
			
			mov		edx, [ecx + edi * 4]		; Offset adresu funkcji
			add		edx, eax					; Adres GetProcAddress
			
; --------------------------------------------
; Znajdowanie adresu LoadLibrary
; --------------------------------------------

			push	edx
			call	load_l_str					; Odkladanie nazwy funkcji
			db 		"LoadLibraryA", 0x00
load_l_str:
			push	eax							; Odkladanie Handlera do kernel32
			call	edx
			
			pop		edx							; Adres GetProcAddress
			
			jmp		endf			

not_found_f:
			add		esp, 8						; Zmienne na stosie
			
not_found:
			xor		eax, eax
			xor		edx, edx

endf:
			mov		[esp + 4], eax
			mov		[esp], edx
			