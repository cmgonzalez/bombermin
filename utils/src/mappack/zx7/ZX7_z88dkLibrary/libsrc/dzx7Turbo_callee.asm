; ----------------------------------------------------------------
; Z88DK LIBRARY FOR ZX7 DECOMPRESSORS
;
; See "zx7.h" for further details
; ----------------------------------------------------------------
XLIB dzx7Turbo_callee
XDEF asm_dzx7Turbo

.dzx7Turbo_callee
        pop hl          ; RET address
        pop de          ; dst parameter
        ex (sp),hl      ; src parameter

.asm_dzx7Turbo
        include "dzx7_turbo.asm"
