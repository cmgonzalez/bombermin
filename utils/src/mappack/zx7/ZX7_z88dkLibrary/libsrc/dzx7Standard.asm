; ----------------------------------------------------------------
; Z88DK LIBRARY FOR ZX7 DECOMPRESSORS
;
; See "zx7.h" for further details
; ----------------------------------------------------------------
XLIB dzx7Standard
LIB dzx7Standard_callee
XREF asm_dzx7Standard

.dzx7Standard
        pop bc          ; RET address
        pop de          ; dst parameter
   	pop hl          ; src parameter
   	push hl
   	push de
   	push bc         ; put everything back
   	jp asm_dzx7Standard
