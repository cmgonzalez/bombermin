; -----------------------------------------------------------------------------
; "Agile" integrated RCS+ZX7 decoder by Einar Saukas (150 bytes)
; -----------------------------------------------------------------------------
; Parameters:
;   HL: source address (compressed data)
;   DE: destination address (decompressing)
; -----------------------------------------------------------------------------

dzx7_agilercs:
        ld      a, $80
dzx7a_copy_byte_loop1:
        ex      af, af'
        call    dzx7a_copy_byte         ; copy literal byte
dzx7a_main_loop1:
        ex      af, af'
        add     a, a                    ; check next bit
        call    z, dzx7a_load_bits      ; no more bits left?
        jr      nc, dzx7a_copy_byte_loop1 ; next bit indicates either literal or sequence
        jr      dzx7a_len_size_start

dzx7a_copy_byte_loop:
        ldi                             ; copy literal byte
dzx7a_main_loop:
        add     a, a                    ; check next bit
        call    z, dzx7a_load_bits      ; no more bits left?
        jr      nc, dzx7a_copy_byte_loop ; next bit indicates either literal or sequence

dzx7a_len_size_start:
; determine number of bits used for length (Elias gamma coding)
        push    de
        ld      bc, 1
        ld      d, b
dzx7a_len_size_loop:
        inc     d
        add     a, a                    ; check next bit
        call    z, dzx7a_load_bits      ; no more bits left?
        jr      nc, dzx7a_len_size_loop
        jp      dzx7a_len_value_start

; determine length
dzx7a_len_value_loop:
        add     a, a                    ; check next bit
        call    z, dzx7a_load_bits      ; no more bits left?
        rl      c
        rl      b
        jr      c, dzx7a_exit           ; check end marker
dzx7a_len_value_start:
        dec     d
        jr      nz, dzx7a_len_value_loop
        inc     bc                      ; adjust length

; determine offset
        ld      e, (hl)                 ; load offset flag (1 bit) + offset value (7 bits)
        inc     hl
        defb    $cb, $33                ; opcode for undocumented instruction "SLL E" aka "SLS E"
        jr      nc, dzx7a_offset_end    ; if offset flag is set, load 4 extra bits
        add     a, a                    ; check next bit
        call    z, dzx7a_load_bits      ; no more bits left?
        rl      d                       ; insert first bit into D
        add     a, a                    ; check next bit
        call    z, dzx7a_load_bits      ; no more bits left?
        rl      d                       ; insert second bit into D
        add     a, a                    ; check next bit
        call    z, dzx7a_load_bits      ; no more bits left?
        rl      d                       ; insert third bit into D
        add     a, a                    ; check next bit
        call    z, dzx7a_load_bits      ; no more bits left?
        ccf
        jr      c, dzx7a_offset_end
        inc     d                       ; equivalent to adding 128 to DE
dzx7a_offset_end:
        rr      e                       ; insert inverted fourth bit into E

; copy previous sequence
        ex      (sp), hl                ; store source, restore destination
        push    hl                      ; store destination
        sbc     hl, de                  ; HL = destination - offset - 1
        pop     de                      ; DE = destination
        ex      af, af'
        ld      a, h                    ; A = 010RRccc
        cp      $58
        jr      c, dzx7a_copy_bytes
        ldir
        pop     hl                      ; restore source address (compressed data)
        ex      af, af'
        jp      dzx7a_main_loop

dzx7a_copy_bytes:
        push    hl
        ex      de, hl
        call    dzx7a_convert
        ex      de, hl
        call    dzx7a_copy_byte
        pop     hl
        inc     hl
        jp      pe, dzx7a_copy_bytes
        pop     hl                      ; restore source address (compressed data)
        jr      dzx7a_main_loop1

dzx7a_load_bits:
        ld      a, (hl)                 ; load another group of 8 bits
        inc     hl
        rla
        ret

dzx7a_copy_byte:
        push    de
        call    dzx7a_convert
        ldi
dzx7a_exit:
        pop     de
        inc     de
        ret

; Convert an RCS address 010RRccc ccrrrppp to screen address 010RRppp rrrccccc
; (Note: replace both EX AF,AF' with PUSH AF/POP AF if you want to preserve AF')
dzx7a_convert:
        ld      a, d                    ; A = 010RRccc
        cp      $58
        ret     nc
        xor     e
        and     $f8
        xor     e                       ; A = 010RRppp
        push    af
        xor     d
        xor     e                       ; A = ccrrrccc
        rlca
        rlca                            ; A = rrrccccc
        pop     de                      ; D = 010RRppp
        ld      e, a                    ; E = rrrccccc
        ret

; -----------------------------------------------------------------------------
