; ----------------------------------------------------------------
; Z88DK LIBRARY FOR ZX7 DECOMPRESSORS
;
; See "zx7.h" for further details
; ----------------------------------------------------------------
XLIB dzx7Standard_callee
XDEF asm_dzx7Standard

.dzx7Standard_callee
        pop hl          ; RET address
        pop de          ; dst parameter
        ex (sp),hl      ; src parameter

.asm_dzx7Standard
        include "dzx7_standard.asm"
