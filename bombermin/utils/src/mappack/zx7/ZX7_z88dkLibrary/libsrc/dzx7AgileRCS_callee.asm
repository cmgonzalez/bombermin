; ----------------------------------------------------------------
; Z88DK LIBRARY FOR ZX7 DECOMPRESSORS
;
; See "zx7.h" for further details
; ----------------------------------------------------------------
XLIB dzx7AgileRCS_callee
XDEF asm_dzx7AgileRCS

.dzx7AgileRCS_callee
        pop hl          ; RET address
        pop de          ; dst parameter
        ex (sp),hl      ; src parameter

.asm_dzx7AgileRCS
        include "dzx7_agileRCS.asm"
