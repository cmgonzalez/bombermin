; ----------------------------------------------------------------
; Z88DK LIBRARY FOR ZX7 DECOMPRESSORS
;
; See "zx7.h" for further details
; ----------------------------------------------------------------
XLIB dzx7SmartRCS_callee
XDEF asm_dzx7SmartRCS

.dzx7SmartRCS_callee
        pop hl          ; RET address
        pop de          ; dst parameter
        ex (sp),hl      ; src parameter

.asm_dzx7SmartRCS
        include "dzx7_smartRCS.asm"
