[bits 32]

call 0x12345678 ; TODO: GetVersion address function should be resolved from wrapper
cmp al, 0x6
cmc ; complements a CF flag
sbb edx, edx ; edx - (edx + cf)
and edx, 0x34 
xor eax, eax
; get heap
mov ecx, [fs:0x30] ; PEB
mov ecx, [ecx + 0x18] ; get proc heap base
mov ecx, [ecx + edx + 0xc] ; heap flags

; TODO: conditional jump

; for system version in range 3.10 - 3.50
and ecx, 0xeffeffff ; ~HEAP_CREATE_ALIGH, ~HEAP_SKIP_VALIDATION_CHECK
cmp ecx, 0x40000062 ; HEAP_GROWABLE + HEAP_TAIL_CHECKING_ENABLED + HEAP_FREE_CHECKING_ENABLED + HEAP_VALIDATE_PARAMETRES_ENABLED
setz al

; for system version >= 3.51
bswap ecx ; swap heap flags
and cl, 0xef ; ~HEAP_SKIP_VALIDATION_CHECK
cmp ecx, 0x62000040 ; HEAP_GROWABLE + HEAP_TAIL_CHECKING_ENABLED + HEAP_FREE_CHECKING_ENABLED + HEAP_VALIDATE_PARAMETRES_ENABLED
setz al

