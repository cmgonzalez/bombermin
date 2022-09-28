; ----------------------------------------------------------------
; Z88DK LIBRARY FOR ZX7 DECOMPRESSORS
;
; See "zx7.h" for further details
; ----------------------------------------------------------------
XLIB dzx7Mega_callee
XDEF asm_dzx7Mega

.dzx7Mega_callee
        pop hl          ; RET address
        pop de          ; dst parameter
        ex (sp),hl      ; src parameter

.asm_dzx7Mega
        include "dzx7_mega.asm"
