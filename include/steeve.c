/*******************************************************************************
 * beepSteve
 *
 * Plays a beep sound effect bases on Steve Turner Code TODO How to create
 * sounds
 *
 * ******************************************************************************/
void beepSteve(unsigned char fx) __z88dk_fastcall {

  // clang-format off
        __asm
          ld a, l                 ;//z88dk passes the L register from unsigned char
          ld (sonreq),a           ;
                                  ;//32991 shows 0
                                  ;//ld (sonnow),a//added
        play_sound:
            call sound            ;
            ld a,(sonnow)         ;
            or a                  ;
            jr nz, play_sound     ;
            ret                   ;//return back to C
    
        sound:
                ld a,(sonreq)     ;//play new sound? ;32777
                                  ;//loads contents of 32991 into register A
                                  ;//32991 shows 0
                and a             ;//32780
                jr z,nonew        ;//NO
                                  ;//jumps to nonew
    
        ;YES1
                ld (sonnow),a
                dec a
                jr z,noise        ;//#01 noise
    
                ld hl,steveData
                dec a
                add a,a
                add a,a
                add a,a
                ld e,a
                xor a
                ld (sonreq),a
                ld d,a
                add hl,de
                ld bc, 8
                ld de,sonfrq      ;//33245    <--start freq
                ldir              ;//32809
                jr process
    
            nonew:
                ld a,(sonnow)     ;//32813
                                  ;//and the old sound is?
                                  ;//loads contents of 32990 into register A
                and a
                ret z
                dec a             ;//continue noise?
                jr nz,process     ;//continue sound
                jr cnois
    
            noise:
                ld a,0xA
                ld (sonlen),a
                xor a
                ld (sonreq),a
    
            cnois:
                ld b,0x30
    
            gain:
                call random
                and 0x10
                out (0xfe),a
                ld c,2
    
            make:
                dec c
                jr nz,make
                djnz gain
                ld hl,sonlen
                dec (hl)
                ret nz
                xor a
                ld (sonnow),a
                ret
    
            process:
                ld a,(sonfrq)       ;//<--start freq
                ld h,a
                ld a,0x10
                ld d,0xff
    
            sonlp:
                ld e,h
                out (0xfe),a
                xor 0x10
    
            freq:
                dec d
                jr z,mod
                dec e
                jr nz,freq
                jr sonlp
    
            mod:
                ld a,(soncfg)
                add a,h
                ld (sonfrq),a       ;//<--start freq
                ld hl,sonmod
                dec (hl)
                ret nz
                ld hl,sonlen
                dec (hl)
                jr nz,modify
                xor a
                ld (sonnow),a
                ld a,(sonnex)
                and a
                ret z
                ld (sonreq),a
                ret
    
            modify:
                ld a,(sobrsf)
                ld c,a
                ld a,(sontyp)
                and a
                jr z,reset
                dec a
                jr z,typ1
                dec a
                jr z,typ2
    
            typoth:
                ld a,(soncfg)
                neg
                ld (soncfg),a
                jr mode
    
            typ2:
                inc c
                inc c
                ld a,c
                ld (sobrsf),a
                jr reset
    
            typ1:
                dec c
                dec c
                ld a,c
                ld (sobrsf),a
                jr reset
    
            reset:
                ld a,c
                ld (sonfrq),a    ;//<--start freq
    
            mode:
                ld a,(sonrnd)
                ld (sonmod),a
                ret
    
            random:
                push hl
                ld hl,(rnseed)
                inc hl
                ld a,h
                and 3
                ld h,a
    
            rok:
                ld (rnseed),a
                ld a,r
                xor (hl)
                pop hl
                ret
    
        rnseed:    defw 0x1000
    
        sonfrq:    defb 00      ;//start frequency
        soncfg:    defb 00      ;//frequency change
        sonmod:    defb 00      ;//change times
        sonlen:    defb 00      ;//repeat times
        sontyp:    defb 00      ;//modulate type
                                    ;//0 sawtooth
                                    ;//1 2nd mod down
                                    ;//2 2nd mod up
                                    ;//3+ triangle
        sobrsf:    defb 00      ;//reset frequency
        sonrnd:    defb 00      ;//change reset temp
        sonnex:    defb 00      ;//linked sfx
    
        sonnow:    defb 0       ;//which effect playing
        sonreq:    defb 0       ;//we have an effect
        steveData:              ;//here all sounds excepts number 1 reserved for random noise
        ;//SFX 1 //Noise Can't be changed
        ;//SFX 2
        defb 0x28,0xFA,0x04,0x01,0x00,0x00,0x00,0x00 ; Jump 1
        ;//SFX 3
        defb 0x28,0xFA,0x06,0x01,0x00,0x00,0x00,0x00 ; Jump 2
        ;//SFX 4
        defb 0x00,0x01,0x05,0x03,0x00,0x00,0x00,0x00 ; Siren
        ;//SFX 5
        defb 0x00,0x02,0x1E,0x01,0x00,0x00,0x00,0x00 ; Dash
        ;//SFX 6
        defb 0x00,0x7D,0x20,0x01,0x00,0x00,0x00,0x00 ; Ice1
        ;//SFX 7
        defb 0xFF,0x59,0x20,0x01,0x00,0x00,0x00,0x00 ; Ice2
        ;//SFX 8
        defb 0xFF,0x83,0x28,0x20,0x01,0x3C,0x01,0x00 ; Obj
        ;//SFX 9
        defb 0xF0,0xF0,0x08,0x03,0x00,0x3C,0x06,0x00 ; Info
        ;//SFX 10
        defb 0x28,0xFA,0x08,0x01,0x00,0x00,0x00,0x00 ; Punch
        ;//SFX 11
        defb 0x3C,0xB4,0x05,0x01,0x01,0x00,0x00,0x00 ; Coin
        ;//SFX 12
        defb 0xFA,0x2C,0x06,0x0A,0x01,0x5A,0x01,0x00 ; Item
        ;//SFX 13
        defb 0x00,0xFC,0x14,0x08,0x01,0x80,0x08,0x00 ; Dead
        ;//SFX 14
        defb 0xE6,0xE6,0x04,0x01,0x01,0x00,0x00,0x00 ; Door
        ;//SFX 15
        defb 0x2D,0x43,0x14,0x01,0x01,0x00,0x00,0x00 ; CheckPoint
        ;//SFX 16
        defb 0x40,0x05,0x0A,0x01,0x00,0x00,0x00,0x00 ; Key

    __endasm;
    // clang-format off

    fx = 0; // to avoid z88dk warnings

}