[bits 32]
; pobiera adresy modu³ów za³adowanych do pamiêci
; pointer na tablice w edi, max ilosc wpisow w ecx
			
find_list:	mov 	eax, [fs:0x30]				; PEB
			mov 	eax, [eax + 0x0C]			; PEB.Ldr
			mov 	eax, [eax + 0x14]			; PEB.Ldr.InMemoryModuleList
			
			mov 	edx, eax
			inc		ecx
			
mod_enum:	mov 	eax, [edx + 0x28]			; module name
			sub		ecx, 1
			jz		end_proc
			test 	eax, eax
			jz		end_proc
			
			mov		[edi], eax
			
			mov		eax, [edx + 0x10]			; Handle
			mov		[edi + 4], eax
			add		edi, 8
			
			mov		edx, [edx]
			jmp		mod_enum
			
end_proc:	nop