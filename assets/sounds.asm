;// Byte 0 Start Frequency
;// Byte 1 Frequency Change
;// Byte 2 Sound Mods
;// Byte 3 Sound Repeats
;// Byte 4 Module Type
;//  0 sawtooth
;//  1 2nd mod down
;//  2 2nd mod up
;//  3 triangle
;// Byte 5 Reset Frequency
;// Byte 6 Change Temp
;// Byte 7 Link to sfx (noise?)

steveData:  
    ;//SFX 1
    ;//Noise Can't be changed
    ;//SFX 2
    defb $28,$FA,$04,$01,$00,$00,$00,$00 ; Jump 1
    ;//SFX 3
    defb $28,$FA,$06,$01,$00,$00,$00,$00 ; Jump 2
    ;//SFX 4
    defb $00,$05,$05,$03,$00,$00,$00,$00 ; Siren
    ;//SFX 5
    defb $00,$02,$1E,$01,$00,$00,$00,$00 ; Dash
    ;//SFX 6
    defb $00,$7D,$20,$01,$00,$00,$00,$00 ; Ice1
    ;//SFX 7
    defb $FF,$59,$20,$01,$00,$00,$00,$00 ; Ice2
    ;//SFX 8
    defb $FF,$83,$28,$20,$01,$3C,$01,$00 ; Obj
    ;//SFX 9
    defb $F0,$F0,$08,$03,$00,$3C,$06,$00 ; Info
    ;//SFX 10
    defb $28,$FA,$08,$01,$00,$00,$00,$00 ; Punch
    ;//SFX 11
    defb $3C,$B4,$05,$01,$01,$00,$00,$00 ; Coin
    ;//SFX 12
    defb $FA,$2C,$06,$0A,$01,$5A,$01,$00 ; Item
    ;//SFX 13
    defb $00,$FC,$14,$08,$01,$80,$08,$00 ; Dead
    ;//SFX 14
    defb $E6,$E6,$04,$01,$01,$00,$00,$00 ; Door
    ;//SFX 15
    defb $2D,$43,$14,$01,$01,$00,$00,$00 ; CheckPoint
    ;//SFX 16
    defb $40,$05,$0A,$01,$00,$00,$00,$00 ; Key
    