; Disassembly of "rm.gb"
; This file was created with:
; mgbdis v3.0 - Game Boy ROM disassembler by Matt Currie and contributors.
; https://github.com/mattcurrie/mgbdis

; ROM bank 6. Game B support data: the metasprite tables, level layouts,
; tables and graphics. The code is now C (bank6_*.c). Bank 0 calls the
; jp vectors at $4000 + 3 * n, then maps bank 3 again.

; The C code reads these tables.
EXPORT MetaspritePtrs, LevelLayoutPtrs, DifficultyTablePtrs
EXPORT GameB_ScaledTableSource, RandomLimitTable

SECTION "ROM Bank $006", ROMX[$4000], BANK[$6]

; Jump table. Bank 0 switches to bank 6 and calls $4000 + 3 * n.

Vec6_DrawMetasprite:
    jp DrawMetasprite

Vec6_GameB_InitObjectSlot:
    jp GameB_InitObjectSlot

    ds 3, $00 ; unused vector (was GetDifficultyTableByte)

Vec6_GameB_LoadLevelLayout:
    jp GameB_LoadLevelLayout

Vec6_GameB_DrawSixAnimSprites:
    jp GameB_DrawSixAnimSprites

Vec6_GameB_DrawDeathTallies:
    jp GameB_DrawDeathTallies

Vec6_GameB_InitScaledTable:
    jp GameB_InitScaledTable

Vec6_GameB_UpdateScreenEffects:
    jp GameB_UpdateScreenEffects

Vec6_RandomMasked:
    jp RandomMasked

Vec6_CountGroupRun:
    jp CountGroupRun

    ds 3, $00 ; unused vector (was CountGroupSlots)

    ds 3, $00 ; unused vector (was ShiftDELeft4)

    jp Multiply8

Vec6_GameB_InitShipArrays:
    jp GameB_InitShipArrays

Vec6_GameB_PlaceShips:
    jp GameB_PlaceShips

Vec6_GameB_UpdateWeaponFlags:
    jp GameB_UpdateWeaponFlags

; ===========================================================================
; Glue. The asm callers pass values in registers. The C functions use the
; SDCC rules (__sdcccall(1)):
;   - 1st argument: A (byte) or DE (pointer).
;   - 2nd argument: E (byte after a byte) or A (byte after a pointer).
;   - More arguments go on the stack. The C function removes them.
;   - Result: A (byte) or BC (pointer).
; C code can change all registers, so the glue keeps every register that
; is not a result.
; ===========================================================================

; CALL_C and FAR_CALL_C are in glue.inc.

GameB_DrawSixAnimSprites:
    CALL_C drawSixAnimSprites

GameB_DrawDeathTallies:
    CALL_C drawDeathTallies

GameB_InitScaledTable:
    CALL_C initScaledTable

GameB_UpdateScreenEffects:
    CALL_C updateScreenEffects

GameB_InitShipArrays:
    CALL_C initShipArrays

GameB_PlaceShips:
    CALL_C placeShips

GameB_UpdateWeaponFlags:
    CALL_C updateWeaponFlags

; In: HL = metasprite pointer table, C = index, D = X, E = Y.
DrawMetasprite:
    push af
    push bc
    push de
    push hl
    ld a, d ; stack: X, then Y above it
    ld d, e
    ld e, a
    push de
    ld d, h
    ld e, l
    ld a, c
    call drawMetasprite
    pop hl
    pop de
    pop bc
    pop af
    ret

; In: HL = 8-byte slot, DE = source byte, B = slot index.
; Out: HL + 6, DE + 1, B + 1.
GameB_InitObjectSlot:
    push hl
    push de
    push bc
    push de ; stack: source byte pointer
    ld d, h
    ld e, l
    ld a, b
    call initObjectSlot
    pop bc
    pop de
    pop hl
    inc b
    inc de
REPT 6
    inc hl
ENDR
    ret

; Out: DE = rest of the layout.
GameB_LoadLevelLayout:
    push af
    push bc
    push hl
    call loadLevelLayout
    ld d, b
    ld e, c
    pop hl
    pop bc
    pop af
    ret

; In: HL = seed byte, C = range. Out: C = random value.
RandomMasked:
    push de
    push hl
    ld d, h
    ld e, l
    ld a, c
    call randomMasked
    pop hl
    pop de
    ld c, a
    ret

; In: HL = byte list. Out: C = run length, HL = first byte after the run.
CountGroupRun:
    push de
    push hl
    ld d, h
    ld e, l
    call countGroupRun
    pop hl
    pop de
    ld c, a
    push bc
    ld b, $00
    inc hl
    add hl, bc
    pop bc
    ret
; ===========================================================================
; Data. Nothing after this point is code.
; ===========================================================================

; Metasprite format, read by drawMetasprite (bank6_sprites.c):
;   tile, attr  - one 8x8 sprite (tile < $fd), then move right 8 pixels
;   MSPR_ROW    - move down 8 pixels, back to the left edge
;   MSPR_SKIP   - move right 8 pixels with no sprite
;   MSPR_END    - end of the metasprite
DEF MSPR_ROW  EQU $fd
DEF MSPR_SKIP EQU $fe
DEF MSPR_END  EQU $ff

; GameB_WeaponBonusTable moved to C (bank6_level.c).

; Unused bytes.
    db $ff, $55, $79, $95

; 23 tiles (2bpp). Bank 0 copies $170 bytes to VRAM $9590.
Title_GfxExtraTiles:
    db $9f, $90, $8f, $88, $8f, $88, $87, $84, $07, $84, $43, $c0, $c3, $c3, $c3, $c3
    db $48, $a8, $48, $a8, $5f, $a0, $40, $bf, $00, $80, $7f, $ff, $3f, $ff, $0f, $ff
    db $00, $00, $00, $00, $ff, $00, $00, $ff, $00, $00, $ff, $ff, $ff, $ff, $ff, $ff
    db $00, $80, $00, $c0, $0f, $60, $c0, $b7, $a0, $98, $b3, $8f, $b9, $87, $b1, $80
    db $03, $02, $03, $02, $ff, $02, $07, $fa, $03, $02, $ff, $fe, $ff, $fe, $ff, $fe
    db $40, $40, $40, $40, $5f, $40, $50, $4f, $40, $40, $7f, $7f, $7f, $7f, $7f, $7f
    db $00, $01, $00, $03, $f0, $06, $03, $ed, $05, $19, $cd, $f1, $9d, $e1, $8d, $01
    db $12, $15, $12, $15, $fa, $05, $02, $fd, $00, $01, $fe, $ff, $fc, $ff, $f0, $ff
    db $a0, $8f, $a0, $97, $40, $d7, $48, $d3, $44, $c9, $2a, $e4, $25, $e2, $14, $f3
    db $07, $fc, $02, $ff, $01, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $7f, $80, $1f
    db $ff, $e6, $f7, $3a, $ff, $fa, $bf, $da, $5f, $f6, $37, $fe, $1b, $fe, $07, $fe
    db $40, $7f, $40, $7f, $40, $7f, $60, $7f, $7c, $5f, $6e, $7b, $77, $6d, $7d, $76
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $fe, $81, $f8
    db $05, $f1, $05, $e9, $02, $eb, $12, $cb, $22, $93, $54, $27, $a4, $47, $68, $8f
    db $12, $f1, $0b, $f8, $09, $f8, $04, $fc, $02, $fe, $01, $ff, $00, $ff, $00, $ff
    db $60, $83, $1c, $e0, $c3, $3c, $f8, $07, $7f, $00, $3f, $00, $9f, $80, $47, $c0
    db $03, $fe, $03, $3e, $c3, $02, $3f, $c2, $87, $7a, $ff, $02, $ff, $02, $ff, $02
    db $6f, $5f, $77, $6e, $7b, $77, $5f, $58, $5f, $57, $55, $5d, $5f, $54, $7b, $5d
    db $c6, $c1, $59, $c6, $b7, $68, $ef, $a0, $ee, $a0, $ac, $e0, $a9, $e1, $62, $a3
    db $c8, $0f, $d0, $1f, $90, $1f, $20, $3f, $40, $7f, $80, $ff, $00, $ff, $00, $ff
    db $21, $e0, $18, $f8, $06, $fe, $01, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $ff, $02, $7f, $02, $0f, $02, $87, $82, $73, $f2, $0e, $fe, $02, $fe, $01, $ff
    db $6b, $4e, $7b, $4f, $7b, $4f, $6f, $4b, $4f, $4b, $7d, $7b, $47, $7c, $83, $ff

; Unknown bytes.
    db $28, $7f, $88, $00, $50, $00, $68

; 4 bytes. Bank 0 copies them to $d091 when a game starts.
InitD091:
    db $00, $48, $00, $54

; Unknown bytes.
    db $20, $09

; Pointers to 26-byte rows of signed offsets. Bank 0 ($38db) reads
; entry B (B >= 1) and subtracts a byte from $d44a. Entry 0 is unused.
OffsetRowPtrs:
    dw $0028, GameB_DepthChargeOffsets1, GameB_DepthChargeOffsets2, GameB_DepthChargeOffsets3
    dw GameB_DepthChargeOffsets4, GameB_DepthChargeOffsets5
GameB_DepthChargeOffsets1:
    db $00, $01, $02, $03, $04, $05, $06, $07, $08, $09, $0a, $0b, $0c
    db $00, $fe, $fc, $fa, $f8, $f6, $f4, $f2, $f0, $ee, $ec, $ea, $e8
GameB_DepthChargeOffsets2:
    db $00, $02, $04, $06, $08, $09, $0b, $0d, $0f, $11, $13, $15, $16
    db $00, $ff, $fe, $fd, $fc, $fb, $fa, $f9, $f8, $f7, $f6, $f5, $f4
GameB_DepthChargeOffsets3:
    db $00, $02, $04, $06, $08, $0a, $0c, $0e, $10, $12, $14, $16, $18
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
GameB_DepthChargeOffsets4:
    db $00, $02, $04, $06, $08, $09, $0b, $0d, $0f, $11, $13, $15, $16
    db $00, $01, $02, $03, $04, $05, $06, $07, $08, $09, $0a, $0b, $0c
GameB_DepthChargeOffsets5:
    db $00, $01, $02, $03, $04, $05, $06, $07, $08, $09, $0a, $0b, $0c
    db $00, $02, $04, $06, $08, $0a, $0c, $0e, $10, $12, $14, $16, $18

; Metasprite pointer table. Callers pass a base in HL and an index in C
; to DrawMetasprite. Callers use the bases $469b, $46b5, $46d5 and $46f5.
MetaspritePtrs:
    dw Unused_GameBMetasprite_0, Unused_GameBMetasprite_1, Unused_GameBMetasprite_2, Unused_GameBMetasprite_3
    dw Unused_GameBMetasprite_4, GameB_LoadoutBadgeMetasprite_0, GameB_LoadoutBadgeMetasprite_1, GameB_LoadoutBadgeMetasprite_2
    dw GameB_LoadoutBadgeMetasprite_3, GameB_LoadoutBadgeMetasprite_4, GameB_LoadoutBadgeMetasprite_5, GameB_LoadoutBadgeMetasprite_6
    dw GameB_LoadoutBadgeMetasprite_7, GameB_Level3EndMetasprite_0, GameB_Level3EndMetasprite_1, GameB_Level4EndMetasprite_0
    dw GameB_Level4EndMetasprite_1, GameB_Level4EndMetasprite_2, GameB_Level4EndMetasprite_3, GameB_Level4EndMetasprite_4
    dw GameB_Level1EndMetasprite_0, GameB_Level1EndMetasprite_1, GameB_Level2EndMetasprite_0, GameB_Level2EndMetasprite_1
    dw GameB_Level2EndMetasprite_2, GameB_Level3EndMetasprite_2, GameB_Level3EndMetasprite_3, GameB_Level3EndMetasprite_4
    dw GameB_Level3EndMetasprite_5, GameB_OutfitItemMarkerMetasprite, GameB_OutfitCursorMetasprite, GameB_TwoPlayerBadgeMetasprite
    dw GameB_SixAnimMetasprite_0, GameB_SixAnimMetasprite_1, GameB_SixAnimMetasprite_2, GameB_SixAnimMetasprite_1
    dw GameB_SixAnimMetasprite_0, GameB_SinkingSubMetasprite, GameB_GameOverSinkMetasprite_0, GameB_GameOverSinkMetasprite_1
    dw GameB_DepthChargeMetasprite_0, GameB_DepthChargeMetasprite_1, GameB_StageIconMetasprite_0, GameB_StageIconMetasprite_1
    dw GameB_StageIconMetasprite_2, GameB_TallyIconMetasprite, GameB_TallyIconMetasprite, Unused_TallyIconMetasprite_0
    dw Unused_TallyIconMetasprite_1, Unused_TallyIconMetasprite_2, Unused_TallyIconMetasprite_3, GameB_TallyIconMetasprite
    dw GameB_TallyIconBlinkMetasprite, GameB_TallyIconMetasprite, GameB_TallyIconGrowMetasprite
Unused_GameBMetasprite_0:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $49,$00, $4a,$00, $4b,$00, $4c,$00, MSPR_END
Unused_GameBMetasprite_1:
    db MSPR_ROW, MSPR_ROW, MSPR_SKIP, $4d,$00, MSPR_ROW, $4e,$00, $4f,$00, $50,$00, $51,$00, MSPR_ROW, MSPR_SKIP, $52,$00, MSPR_SKIP, $53,$00, MSPR_ROW, MSPR_SKIP, $54,$00, MSPR_END
Unused_GameBMetasprite_2:
    db MSPR_SKIP, $55,$00, MSPR_ROW, MSPR_SKIP, $56,$00, MSPR_ROW, MSPR_SKIP, $57,$00, MSPR_SKIP, $58,$00, MSPR_ROW, $59,$00, $5a,$00, $5b,$00, $5c,$00, MSPR_ROW, MSPR_SKIP, $57,$40, MSPR_SKIP, $5e,$00, MSPR_ROW, MSPR_SKIP, $56,$40, MSPR_ROW, MSPR_SKIP, $5d,$00, MSPR_END
Unused_GameBMetasprite_3:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $5f,$00, $60,$00, $61,$00, $62,$00, MSPR_END
Unused_GameBMetasprite_4:
    db MSPR_SKIP, $55,$00, MSPR_ROW, MSPR_SKIP, $56,$00, MSPR_ROW, MSPR_SKIP, $57,$00, MSPR_SKIP, $58,$00, MSPR_ROW, $63,$00, $64,$00, $65,$00, $66,$00, MSPR_ROW, MSPR_SKIP, $57,$40, MSPR_SKIP, $5e,$00, MSPR_ROW, MSPR_SKIP, $56,$40, MSPR_ROW, MSPR_SKIP, $5d,$00, MSPR_END
GameB_LoadoutBadgeMetasprite_0:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $8a,$00, MSPR_ROW, $81,$00, $82,$00, $83,$00, $84,$00, $85,$00, $86,$00, $87,$00, $88,$00, $88,$00, $89,$00, MSPR_END
GameB_LoadoutBadgeMetasprite_1:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $8a,$00, MSPR_ROW, $8b,$00, $8c,$00, $83,$00, $84,$00, $85,$00, $86,$00, $87,$00, $88,$00, $88,$00, $89,$00, MSPR_END
GameB_LoadoutBadgeMetasprite_2:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $8a,$00, MSPR_ROW, $81,$00, $82,$00, $83,$00, $84,$00, $8d,$00, $8e,$00, $87,$00, $88,$00, $88,$00, $89,$00, MSPR_END
GameB_LoadoutBadgeMetasprite_3:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $8a,$00, MSPR_ROW, $8b,$00, $8c,$00, $83,$00, $84,$00, $8d,$00, $8e,$00, $87,$00, $88,$00, $88,$00, $89,$00, MSPR_END
GameB_LoadoutBadgeMetasprite_4:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $8a,$00, MSPR_ROW, $81,$00, $82,$00, $83,$00, $84,$00, $85,$00, $86,$00, $87,$00, $88,$00, $8f,$00, $90,$00, MSPR_END
GameB_LoadoutBadgeMetasprite_5:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $8a,$00, MSPR_ROW, $8b,$00, $8c,$00, $83,$00, $84,$00, $85,$00, $86,$00, $87,$00, $88,$00, $8f,$00, $90,$00, MSPR_END
GameB_LoadoutBadgeMetasprite_6:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $8a,$00, MSPR_ROW, $81,$00, $82,$00, $83,$00, $84,$00, $8d,$00, $8e,$00, $87,$00, $88,$00, $8f,$00, $90,$00, MSPR_END
GameB_LoadoutBadgeMetasprite_7:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $8a,$00, MSPR_ROW, $8b,$00, $8c,$00, $83,$00, $84,$00, $8d,$00, $8e,$00, $87,$00, $88,$00, $8f,$00, $90,$00, MSPR_END
GameB_Level3EndMetasprite_0:
    db MSPR_SKIP, $80,$00, $81,$00, MSPR_ROW, $82,$00, $83,$00, $84,$00, $85,$00, MSPR_ROW, $86,$00, $87,$00, $88,$00, $89,$00, MSPR_ROW, MSPR_SKIP, $8a,$00, $8b,$00, MSPR_END
GameB_Level3EndMetasprite_1:
    db MSPR_SKIP, $80,$00, $81,$00, MSPR_ROW, $8c,$00, $8d,$00, $84,$00, $8e,$00, MSPR_ROW, $8f,$00, $87,$00, $90,$00, $91,$00, MSPR_ROW, MSPR_SKIP, $8a,$00, $8b,$00, MSPR_END
GameB_Level4EndMetasprite_0:
    db MSPR_SKIP, $2f,$00, $30,$00, MSPR_ROW, $31,$00, $32,$00, $32,$20, $31,$20, MSPR_ROW, $33,$00, $34,$00, $34,$20, $33,$20, MSPR_ROW, MSPR_SKIP, $35,$00, $35,$20, MSPR_END
GameB_Level4EndMetasprite_1:
    db MSPR_SKIP, $2f,$00, $30,$00, MSPR_ROW, MSPR_SKIP, $36,$00, $32,$20, $31,$20, MSPR_ROW, MSPR_SKIP, $37,$00, $38,$00, $33,$20, MSPR_ROW, MSPR_SKIP, $35,$00, $35,$20, MSPR_END
GameB_Level4EndMetasprite_2:
    db MSPR_SKIP, $2f,$00, $30,$00, MSPR_ROW, MSPR_SKIP, $39,$00, $3a,$00, $3b,$00, MSPR_ROW, MSPR_SKIP, $3c,$00, $3d,$00, $3e,$00, MSPR_ROW, MSPR_SKIP, $35,$00, $35,$20, MSPR_END
GameB_Level4EndMetasprite_3:
    db MSPR_SKIP, $2f,$00, $30,$00, $3f,$00, MSPR_ROW, MSPR_SKIP, $40,$00, $41,$00, $42,$00, MSPR_ROW, MSPR_SKIP, $43,$00, $44,$00, MSPR_ROW, MSPR_SKIP, $35,$00, $35,$20, MSPR_END
GameB_Level4EndMetasprite_4:
    db MSPR_SKIP, $2f,$00, $45,$00, $46,$00, MSPR_ROW, MSPR_SKIP, $47,$00, $48,$00, $49,$00, MSPR_ROW, MSPR_SKIP, $4a,$00, $4b,$00, $4c,$00, MSPR_ROW, MSPR_SKIP, $35,$00, $35,$20, MSPR_END
GameB_Level1EndMetasprite_0:
    db MSPR_ROW, MSPR_SKIP, $4d,$00, $4e,$00, $4f,$00, MSPR_ROW, MSPR_SKIP, $50,$00, $51,$00, $52,$00, MSPR_ROW, MSPR_SKIP, $53,$00, $54,$00, $55,$00, MSPR_END
GameB_Level1EndMetasprite_1:
    db MSPR_ROW, MSPR_SKIP, $56,$00, $57,$00, $58,$00, MSPR_ROW, MSPR_SKIP, $50,$00, $51,$00, $52,$00, MSPR_ROW, MSPR_SKIP, $53,$00, $54,$00, $55,$00, MSPR_END
GameB_Level2EndMetasprite_0:
    db MSPR_SKIP, $00,$00, $01,$00, $02,$00, MSPR_ROW, $03,$00, $04,$00, $05,$00, $06,$00, MSPR_ROW, $07,$00, $08,$00, $09,$00, $0a,$00, MSPR_ROW, $0b,$00, $0c,$00, $0d,$00, $0e,$00, MSPR_END
GameB_Level2EndMetasprite_1:
    db MSPR_SKIP, $00,$00, $01,$00, $02,$00, MSPR_ROW, $0f,$00, $10,$00, $05,$00, $06,$00, MSPR_ROW, $11,$00, $12,$00, $09,$00, $0a,$00, MSPR_ROW, $0b,$00, $0c,$00, $0d,$00, $0e,$00, MSPR_END
GameB_Level2EndMetasprite_2:
    db $13,$00, $14,$00, $01,$00, $02,$00, MSPR_ROW, $15,$00, $16,$00, $05,$00, $06,$00, MSPR_ROW, MSPR_SKIP, $17,$00, $09,$00, $0a,$00, MSPR_ROW, $0b,$00, $0c,$00, $0d,$00, $0e,$00, MSPR_END
GameB_Level3EndMetasprite_2:
    db MSPR_ROW, $92,$00, $93,$00, $94,$00, $95,$00, MSPR_ROW, $96,$00, $97,$00, $98,$00, $99,$00, MSPR_ROW, $9a,$00, $9b,$00, $9c,$00, $9d,$00, MSPR_END
GameB_Level3EndMetasprite_3:
    db MSPR_SKIP, $9e,$00, $9f,$00, MSPR_ROW, MSPR_SKIP, $a0,$00, $a1,$00, MSPR_ROW, $a2,$00, $a3,$00, $a4,$00, $a5,$00, MSPR_ROW, MSPR_SKIP, $a6,$00, $a7,$00, MSPR_END
GameB_Level3EndMetasprite_4:
    db MSPR_SKIP, MSPR_SKIP, $a8,$00, $a9,$00, $aa,$00, MSPR_ROW, MSPR_SKIP, MSPR_SKIP, $ab,$00, $ac,$00, $ad,$00, MSPR_ROW, MSPR_SKIP, $ae,$00, $af,$00, $b0,$00, MSPR_ROW, MSPR_SKIP, $b1,$00, $b2,$00, $b3,$00, MSPR_END
GameB_Level3EndMetasprite_5:
    db MSPR_ROW, MSPR_ROW, MSPR_SKIP, MSPR_SKIP, $b4,$00, $b5,$00, $b6,$00, MSPR_ROW, MSPR_SKIP, $b7,$00, $b8,$00, $b9,$00, $ba,$00, MSPR_END
GameB_OutfitItemMarkerMetasprite:
    db $b6,$00, $b7,$00, MSPR_ROW, $b8,$00, $b9,$00, MSPR_END
GameB_OutfitCursorMetasprite:
    db $ba,$00, $bb,$00, MSPR_ROW, $bc,$00, $bd,$00, MSPR_END
GameB_TwoPlayerBadgeMetasprite:
    db $bb,$20, $ba,$20, MSPR_ROW, $bd,$20, $bc,$20, MSPR_END
GameB_SixAnimMetasprite_0:
    db $eb,$10, MSPR_END
GameB_SixAnimMetasprite_1:
    db $ec,$10, MSPR_END
GameB_SixAnimMetasprite_2:
    db $ed,$10, MSPR_END
GameB_SinkingSubMetasprite:
    db $f0,$10, $f0,$10, MSPR_ROW, $f1,$10, $f1,$10, MSPR_END
GameB_GameOverSinkMetasprite_0:
    db $dd,$00, $de,$00, MSPR_ROW, $df,$00, $e0,$00, MSPR_END
GameB_GameOverSinkMetasprite_1:
    db $e1,$00, $e2,$00, MSPR_ROW, $e3,$00, $e4,$00, MSPR_END
GameB_DepthChargeMetasprite_0:
    db $ee,$00, MSPR_END
GameB_DepthChargeMetasprite_1:
    db $ef,$00, MSPR_END
GameB_StageIconMetasprite_0:
    db $e5,$00, $e6,$00, $e7,$00, MSPR_ROW, $e5,$10, $e6,$10, $e8,$10, MSPR_ROW, $e5,$10, $e6,$10, $e9,$10, MSPR_END
GameB_StageIconMetasprite_1:
    db $e5,$10, $e6,$10, $e7,$10, $ea,$00, MSPR_ROW, $e5,$00, $e6,$00, $e8,$00, MSPR_ROW, $e5,$10, $e6,$10, $e9,$10, MSPR_END
GameB_StageIconMetasprite_2:
    db $e5,$10, $e6,$10, $e7,$10, $ea,$00, MSPR_ROW, $e5,$10, $e6,$10, $e8,$10, $ea,$00, MSPR_ROW, $e5,$00, $e6,$00, $e9,$00, MSPR_END
GameB_TallyIconMetasprite:
    db $68,$00, $69,$00, MSPR_ROW, $6a,$00, $6b,$00, MSPR_END
Unused_TallyIconMetasprite_0:
    db $74,$00, $75,$00, MSPR_ROW, $76,$00, $77,$00, MSPR_END
Unused_TallyIconMetasprite_1:
    db $78,$00, $79,$00, MSPR_ROW, $7a,$00, $7b,$00, MSPR_END
Unused_TallyIconMetasprite_2:
    db $7c,$00, $7d,$00, MSPR_ROW, $7e,$00, $7f,$00, MSPR_END
Unused_TallyIconMetasprite_3:
    db $18,$00, $19,$00, MSPR_ROW, $1a,$00, $1b,$00, MSPR_END
GameB_TallyIconBlinkMetasprite:
    db $6c,$00, $6d,$00, MSPR_ROW, $6e,$00, $6f,$00, MSPR_END
GameB_TallyIconGrowMetasprite:
    db $70,$00, $71,$00, MSPR_ROW, $72,$00, $73,$00, MSPR_END

; Second pointer table. Entries 0-3 point to animation records,
; the rest to metasprites. Bank 0 uses the bases $4b1c, $4b24,
; $4b34, $4b44 and $4b4a.
MetaspritePtrs2:
    dw AnimRecord0, AnimRecord1, AnimRecord2, AnimRecord3
GameB_TorpedoShadowBMetaspritePtrs:
    dw GameB_TorpedoBlipBMetasprite_0, GameB_TorpedoBlipBMetasprite_1, GameB_TorpedoBlipBMetasprite_2, GameB_TorpedoBlipBMetasprite_3
    dw GameB_TorpedoBlipBMetasprite_4, GameB_TorpedoBlipBMetasprite_5, GameB_TorpedoBlipBMetasprite_6, GameB_TorpedoBlipBMetasprite_7
GameB_TorpedoShadowAMetaspritePtrs:
    dw GameB_TorpedoBlipAMetasprite_0, GameB_TorpedoBlipAMetasprite_1, GameB_TorpedoBlipAMetasprite_2, GameB_TorpedoBlipAMetasprite_3
    dw GameB_TorpedoBlipAMetasprite_4, GameB_TorpedoBlipAMetasprite_5, GameB_TorpedoBlipAMetasprite_6, GameB_TorpedoBlipAMetasprite_7
GameB_ObjMetaspritePtrs:
    dw GameB_EdgeArrowLeftMetasprite, GameB_EdgeArrowRightMetasprite, GameB_SonarSweepMetasprite, GameB_EnemySubMetasprite_Full
    dw GameB_EnemyShipMetasprite_01, GameB_EnemyShipMetasprite_02, GameB_EnemyShipMetasprite_03, GameB_EnemyShipMetasprite_04
    dw GameB_EnemyShipMetasprite_05, GameB_EnemyShipMetasprite_06, GameB_EnemyShipMetasprite_07, GameB_EnemyShipMetasprite_08
    dw GameB_EnemyShipMetasprite_09, GameB_EnemyShipMetasprite_10, GameB_EnemyShipMetasprite_11, GameB_EnemyShipMetasprite_12
    dw GameB_EnemyShipMetasprite_13, GameB_EnemyShipMetasprite_14, GameB_EnemyShipMetasprite_15, GameB_EnemyShipMetasprite_16
    dw GameB_EnemyShipMetasprite_17, GameB_EnemyShipMetasprite_18, GameB_EnemyShipMetasprite_19, GameB_EnemyShipMetasprite_20
    dw GameB_EnemyShipMetasprite_21, GameB_EnemyShipMetasprite_22, GameB_EnemyShipMetasprite_23, GameB_EnemyShipMetasprite_24
    dw GameB_EnemySubMetasprite_FullFlip, GameB_EnemySubMetasprite_Dive0, GameB_EnemySubMetasprite_Dive1, GameB_EnemySubMetasprite_Fire0
    dw GameB_EnemySubMetasprite_Fire1, GameB_EnemySubMetasprite_Fire2, GameB_EnemySubMetasprite_Fire3, GameB_EnemySubMetasprite_Dive0Flip
    dw GameB_EnemySubMetasprite_Dive1Flip, GameB_EnemySubMetasprite_Fire0Flip, GameB_EnemySubMetasprite_Fire1Flip, GameB_EnemySubMetasprite_Fire2Flip
    dw GameB_EnemySubMetasprite_Fire3Flip, GameB_MuzzleFlashMetasprite_0, GameB_MuzzleFlashMetasprite_1, GameB_MuzzleFlashMetasprite_2
    dw GameB_MuzzleFlashMetasprite_1, GameB_MuzzleFlashMetasprite_0, GameB_Kind19ObjMetasprite_0, GameB_Kind19ObjMetasprite_1
    dw GameB_Kind22ObjMetasprite_0, GameB_Kind22ObjMetasprite_1, GameB_Kind22ObjMetasprite_2, GameB_Kind22ObjMetasprite_3
    dw GameB_Kind22ObjMetasprite_4, GameB_Kind22ObjMetasprite_5, GameB_Kind22ObjMetasprite_Hit, GameB_Kind22ObjMetasprite_7
    dw GameB_Kind22ObjMetasprite_7Flip, GameB_WinMarkerMetasprite_0, GameB_WinMarkerMetasprite_1, GameB_WinMarkerMetasprite_2
    dw GameB_WinMarkerMetasprite_3, GameB_WinMarkerMetasprite_Flip0, GameB_WinMarkerMetasprite_Flip1, GameB_WinMarkerMetasprite_Flip2
    dw GameB_WinMarkerMetasprite_Flip3, GameB_PeriscopeMetasprite_0, GameB_PeriscopeMetasprite_1, GameB_State0dLabelMetasprite
    dw GameB_State13LabelMetasprite, GameB_CountdownLabelMetasprite, GameB_EndBannerMetasprite, GameB_EndBanner2PMetasprite
    dw Unused_ObjMetasprite_0, Unused_ObjMetasprite_1, Unused_ObjMetasprite_2, GameB_DepthChargeBlinkMetasprite_0
    dw GameB_DepthChargeBlinkMetasprite_1, GameB_DepthChargeBlinkMetasprite_2, GameB_DepthChargeBlinkMetasprite_3

; Animation record: 9 bytes (probably frame times), then 9 metasprite pointers.
AnimRecord0:
    db $03, $04, $0a, $07, $07, $03, $03, $03, $03
    dw GameB_Splash0Metasprite_0, GameB_Splash0Metasprite_1, GameB_Splash0Metasprite_2, GameB_Splash0Metasprite_3
    dw GameB_Splash0Metasprite_3, GameB_Splash0Metasprite_5, GameB_Splash0Metasprite_5, GameB_Splash0Metasprite_7
    dw GameB_Splash0Metasprite_7
AnimRecord1:
    db $03, $04, $07, $04, $04, $03, $03, $03, $03
    dw GameB_Splash1Metasprite_0, GameB_Splash1Metasprite_1, GameB_Splash1Metasprite_2, GameB_Splash1Metasprite_3
    dw GameB_Splash1Metasprite_3, GameB_Splash1Metasprite_5, GameB_Splash1Metasprite_5, GameB_Splash1Metasprite_7
    dw GameB_Splash1Metasprite_7
AnimRecord2:
    db $03, $03, $07, $04, $04, $03, $03, $03, $03
    dw GameB_Splash2Metasprite_0, GameB_Splash2Metasprite_1, GameB_Splash2Metasprite_2, GameB_Splash2Metasprite_3
    dw GameB_Splash2Metasprite_3, GameB_Splash2Metasprite_5, GameB_Splash2Metasprite_5, GameB_Splash2Metasprite_0
    dw GameB_Splash2Metasprite_0
AnimRecord3:
    db $02, $02, $04, $02, $02, $02, $02, $02, $02
    dw GameB_Splash3Metasprite_0, GameB_Splash3Metasprite_1, GameB_Splash3Metasprite_2, GameB_Splash3Metasprite_1
    dw GameB_Splash3Metasprite_1, GameB_Splash3Metasprite_5, GameB_Splash3Metasprite_5, GameB_Splash3Metasprite_0
    dw GameB_Splash3Metasprite_0
GameB_Splash0Metasprite_0:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $53,$00, $54,$00, $53,$20, MSPR_END
GameB_Splash0Metasprite_1:
    db MSPR_ROW, MSPR_ROW, MSPR_SKIP, $55,$00, MSPR_ROW, $56,$00, $57,$00, $56,$20, MSPR_END
GameB_Splash0Metasprite_2:
    db MSPR_SKIP, $58,$00, MSPR_ROW, $59,$00, $5a,$00, $59,$20, MSPR_ROW, $5b,$00, $5c,$00, $5b,$20, MSPR_ROW, $5d,$00, $5e,$00, $5d,$20, MSPR_END
GameB_Splash0Metasprite_3:
    db MSPR_ROW, MSPR_SKIP, $5f,$00, MSPR_ROW, $60,$00, $61,$00, $60,$20, MSPR_ROW, $62,$00, $63,$00, $62,$20, MSPR_END
GameB_Splash0Metasprite_5:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $64,$00, $65,$00, $64,$20, MSPR_END
GameB_Splash0Metasprite_7:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $66,$00, $67,$00, $66,$20, MSPR_END
GameB_Splash1Metasprite_0:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $f6,$00, $f7,$00, $f6,$20, MSPR_END
GameB_Splash1Metasprite_1:
    db MSPR_ROW, MSPR_ROW, MSPR_SKIP, $f8,$00, MSPR_ROW, $f9,$00, $fa,$00, $f9,$20, MSPR_END
GameB_Splash1Metasprite_2:
    db MSPR_ROW, MSPR_SKIP, $fb,$00, MSPR_ROW, $4a,$00, $4b,$00, $4a,$20, MSPR_ROW, $4c,$00, $4d,$00, $4c,$20, MSPR_END
GameB_Splash1Metasprite_3:
    db MSPR_ROW, MSPR_ROW, MSPR_SKIP, $4e,$00, MSPR_ROW, $4f,$00, $50,$00, $4f,$20, MSPR_END
GameB_Splash1Metasprite_5:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $51,$00, $52,$00, $51,$20, MSPR_END
GameB_Splash1Metasprite_7:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $66,$00, $67,$00, $66,$20, MSPR_END
GameB_Splash2Metasprite_0:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $e8,$00, $e9,$00, $e8,$20, MSPR_END
GameB_Splash2Metasprite_1:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $ea,$00, $eb,$00, $ea,$20, MSPR_END
GameB_Splash2Metasprite_2:
    db MSPR_ROW, MSPR_SKIP, $ec,$00, MSPR_ROW, $ed,$00, $ee,$00, $ed,$20, MSPR_ROW, $ef,$00, $f0,$00, $ef,$20, MSPR_END
GameB_Splash2Metasprite_3:
    db MSPR_ROW, MSPR_ROW, MSPR_SKIP, $f1,$00, MSPR_ROW, $f2,$00, $f3,$00, $f2,$20, MSPR_END
GameB_Splash2Metasprite_5:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $f4,$00, $f5,$00, $f4,$20, MSPR_END
GameB_Splash3Metasprite_0:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $e3,$00, $e3,$20, MSPR_END
GameB_Splash3Metasprite_1:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $e4,$00, $e4,$20, MSPR_END
GameB_Splash3Metasprite_2:
    db MSPR_ROW, MSPR_ROW, $e5,$00, $e5,$20, MSPR_ROW, $e6,$00, $e6,$20, MSPR_END
GameB_Splash3Metasprite_5:
    db MSPR_ROW, MSPR_ROW, MSPR_ROW, $e7,$00, $e7,$20, MSPR_END
GameB_TorpedoBlipBMetasprite_0:
    db $c0,$00, MSPR_END
GameB_TorpedoBlipBMetasprite_1:
    db $c0,$00, MSPR_ROW, $c1,$00, MSPR_END
GameB_TorpedoBlipBMetasprite_2:
    db $c0,$00, MSPR_ROW, $c1,$00, MSPR_ROW, $c2,$00, MSPR_END
GameB_TorpedoBlipBMetasprite_3:
    db $c3,$00, MSPR_ROW, $c4,$00, MSPR_ROW, $c5,$00, MSPR_END
GameB_TorpedoBlipBMetasprite_4:
    db $c6,$00, MSPR_ROW, $c7,$00, MSPR_END
GameB_TorpedoBlipBMetasprite_5:
    db $c8,$00, MSPR_END
GameB_TorpedoBlipBMetasprite_6:
    db $c9,$00, MSPR_END
GameB_TorpedoBlipBMetasprite_7:
    db $c5,$00, MSPR_END
GameB_TorpedoBlipAMetasprite_0:
    db $c5,$60, MSPR_END
GameB_TorpedoBlipAMetasprite_1:
    db $c9,$60, MSPR_END
GameB_TorpedoBlipAMetasprite_2:
    db $c8,$60, MSPR_END
GameB_TorpedoBlipAMetasprite_3:
    db $c7,$60, MSPR_ROW, $c6,$60, MSPR_END
GameB_TorpedoBlipAMetasprite_4:
    db $c5,$60, MSPR_ROW, $c4,$60, MSPR_ROW, $c3,$60, MSPR_END
GameB_TorpedoBlipAMetasprite_5:
    db $c2,$60, MSPR_ROW, $c1,$60, MSPR_ROW, $c0,$60, MSPR_END
GameB_TorpedoBlipAMetasprite_6:
    db $c2,$60, MSPR_ROW, $c1,$60, MSPR_END
GameB_TorpedoBlipAMetasprite_7:
    db $c2,$60, MSPR_END
GameB_EdgeArrowLeftMetasprite:
    db $ca,$00, MSPR_ROW, $ca,$40, MSPR_END
GameB_EdgeArrowRightMetasprite:
    db $ca,$20, MSPR_ROW, $ca,$60, MSPR_END
GameB_SonarSweepMetasprite:
    db $77,$00, MSPR_END
GameB_EnemySubMetasprite_Full:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $07,$00, MSPR_ROW, $01,$00, $02,$00, $03,$00, $04,$00, $05,$00, $03,$00, $06,$00, MSPR_END
GameB_EnemyShipMetasprite_01:
    db MSPR_SKIP, $24,$00, MSPR_SKIP, $18,$00, MSPR_ROW, $34,$00, $20,$00, $21,$00, $22,$00, $23,$00, MSPR_END
GameB_EnemyShipMetasprite_02:
    db MSPR_SKIP, $18,$20, MSPR_SKIP, $24,$20, MSPR_ROW, $23,$20, $22,$20, $21,$20, $20,$20, $34,$20, MSPR_END
GameB_EnemyShipMetasprite_03:
    db $1c,$00, $1d,$00, $1e,$00, $1f,$00, MSPR_END
GameB_EnemyShipMetasprite_04:
    db $1f,$20, $1e,$20, $1d,$20, $1c,$20, MSPR_END
GameB_EnemyShipMetasprite_05:
    db $19,$00, $1a,$00, $1b,$00, MSPR_END
GameB_EnemyShipMetasprite_06:
    db $1b,$20, $1a,$20, $19,$20, MSPR_END
GameB_EnemyShipMetasprite_07:
    db MSPR_SKIP, $32,$00, MSPR_SKIP, MSPR_SKIP, $33,$00, MSPR_ROW, $25,$00, $2e,$00, $2f,$00, $30,$00, $31,$00, $1f,$00, MSPR_END
GameB_EnemyShipMetasprite_08:
    db MSPR_SKIP, $33,$20, MSPR_SKIP, MSPR_SKIP, $32,$20, MSPR_ROW, $1f,$20, $31,$20, $30,$20, $2f,$20, $2e,$20, $25,$20, MSPR_END
GameB_EnemyShipMetasprite_09:
    db MSPR_SKIP, $18,$00, MSPR_SKIP, $2d,$00, MSPR_ROW, $29,$00, $2a,$00, $2b,$00, $2c,$00, $1f,$00, MSPR_END
GameB_EnemyShipMetasprite_10:
    db MSPR_SKIP, $2d,$20, MSPR_SKIP, $18,$20, MSPR_ROW, $1f,$20, $2c,$20, $2b,$20, $2a,$20, $29,$20, MSPR_END
GameB_EnemyShipMetasprite_11:
    db $25,$00, $26,$00, $27,$00, $28,$00, MSPR_END
GameB_EnemyShipMetasprite_12:
    db $28,$20, $27,$20, $26,$20, $25,$20, MSPR_END
GameB_EnemyShipMetasprite_13:
    db MSPR_SKIP, $47,$00, MSPR_SKIP, $48,$00, $49,$00, MSPR_ROW, $34,$00, $41,$00, $42,$00, $43,$00, $44,$00, $45,$00, $46,$00, MSPR_END
GameB_EnemyShipMetasprite_14:
    db MSPR_SKIP, MSPR_SKIP, $49,$20, $48,$20, MSPR_SKIP, $47,$20, MSPR_ROW, $46,$20, $45,$20, $44,$20, $43,$20, $42,$20, $41,$20, $34,$20, MSPR_END
GameB_EnemyShipMetasprite_15:
    db MSPR_SKIP, $3f,$00, MSPR_SKIP, $40,$00, MSPR_ROW, $34,$00, $3a,$00, $3b,$00, $3c,$00, $3d,$00, $3e,$00, MSPR_END
GameB_EnemyShipMetasprite_16:
    db MSPR_SKIP, MSPR_SKIP, $40,$20, MSPR_SKIP, $3f,$20, MSPR_ROW, $3e,$20, $3d,$20, $3c,$20, $3b,$20, $3a,$20, $34,$20, MSPR_END
GameB_EnemyShipMetasprite_17:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $39,$00, MSPR_ROW, $34,$00, $35,$00, $36,$00, $37,$00, $38,$00, MSPR_END
GameB_EnemyShipMetasprite_18:
    db MSPR_SKIP, $39,$20, MSPR_ROW, $38,$20, $37,$20, $36,$20, $35,$20, $34,$20, MSPR_END
GameB_EnemyShipMetasprite_19:
    db $14,$00, $15,$00, $16,$00, $17,$00, MSPR_END
GameB_EnemyShipMetasprite_20:
    db $17,$20, $16,$20, $15,$20, $14,$20, MSPR_END
GameB_EnemyShipMetasprite_21:
    db $11,$00, $12,$00, $13,$00, MSPR_END
GameB_EnemyShipMetasprite_22:
    db $13,$20, $12,$20, $11,$20, MSPR_END
GameB_EnemyShipMetasprite_23:
    db $0f,$00, $10,$00, MSPR_END
GameB_EnemyShipMetasprite_24:
    db $10,$20, $0f,$20, MSPR_END
GameB_EnemySubMetasprite_FullFlip:
    db MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $07,$20, MSPR_ROW, $06,$20, $03,$20, $05,$20, $04,$20, $03,$20, $02,$20, $01,$20, MSPR_END
GameB_EnemySubMetasprite_Dive0:
    db MSPR_ROW, $08,$00, $09,$00, $0a,$00, $0b,$00, $0c,$00, $0a,$00, $0a,$00, MSPR_END
GameB_EnemySubMetasprite_Dive1:
    db MSPR_ROW, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $0d,$00, $0e,$00, MSPR_END
GameB_EnemySubMetasprite_Fire0:
    db MSPR_ROW, $0b,$00, $0c,$00, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $0c,$20, $0d,$00, MSPR_ROW, $0e,$00, $0f,$00, $10,$00, $11,$00, $12,$00, $13,$00, $14,$00, MSPR_END
GameB_EnemySubMetasprite_Fire1:
    db MSPR_SKIP, $15,$00, $16,$00, MSPR_SKIP, $17,$00, $18,$00, MSPR_ROW, MSPR_SKIP, $19,$00, $1a,$00, MSPR_SKIP, $1b,$00, $19,$20, MSPR_ROW, MSPR_SKIP, MSPR_SKIP, $1c,$00, $1d,$00, $1e,$00, MSPR_END
GameB_EnemySubMetasprite_Fire2:
    db MSPR_ROW, MSPR_SKIP, $15,$00, $16,$00, MSPR_SKIP, $17,$00, $18,$00, MSPR_ROW, MSPR_SKIP, $19,$00, $1a,$00, MSPR_SKIP, $1b,$00, $19,$20, MSPR_END
GameB_EnemySubMetasprite_Fire3:
    db MSPR_ROW, MSPR_ROW, MSPR_SKIP, $15,$00, $16,$00, MSPR_SKIP, $17,$00, $18,$00, MSPR_END
GameB_EnemySubMetasprite_Dive0Flip:
    db MSPR_ROW, $0a,$20, $0a,$20, $0c,$20, $0b,$20, $0a,$20, $09,$20, $08,$20, MSPR_END
GameB_EnemySubMetasprite_Dive1Flip:
    db MSPR_ROW, MSPR_SKIP, MSPR_SKIP, $0e,$20, $0d,$20, MSPR_END
GameB_EnemySubMetasprite_Fire0Flip:
    db MSPR_ROW, $0d,$20, $0c,$00, MSPR_SKIP, MSPR_SKIP, MSPR_SKIP, $0c,$20, $0b,$20, MSPR_ROW, $14,$20, $13,$20, $12,$20, $11,$20, $10,$20, $0f,$20, $0e,$20, MSPR_END
GameB_EnemySubMetasprite_Fire1Flip:
    db MSPR_SKIP, $18,$20, $17,$20, MSPR_SKIP, $16,$20, $15,$20, MSPR_ROW, MSPR_SKIP, $19,$00, $1b,$20, MSPR_SKIP, $1a,$20, $19,$20, MSPR_ROW, MSPR_SKIP, MSPR_SKIP, $1e,$20, $1d,$20, $1c,$20, MSPR_END
GameB_EnemySubMetasprite_Fire2Flip:
    db MSPR_ROW, MSPR_SKIP, $18,$20, $17,$20, MSPR_SKIP, $16,$20, $15,$20, MSPR_ROW, MSPR_SKIP, $19,$00, $1b,$20, MSPR_SKIP, $1a,$20, $19,$20, MSPR_END
GameB_EnemySubMetasprite_Fire3Flip:
    db MSPR_ROW, MSPR_ROW, MSPR_SKIP, $18,$20, $17,$20, MSPR_SKIP, $16,$20, $15,$20, MSPR_END
GameB_MuzzleFlashMetasprite_0:
    db MSPR_ROW, MSPR_SKIP, $1f,$00, $20,$00, MSPR_END
GameB_MuzzleFlashMetasprite_1:
    db MSPR_SKIP, $21,$00, $21,$20, MSPR_ROW, $22,$00, $23,$00, $24,$00, $22,$20, MSPR_END
GameB_MuzzleFlashMetasprite_2:
    db $25,$00, $26,$00, $27,$00, $25,$20, MSPR_ROW, $28,$00, $29,$00, $2a,$00, $28,$20, MSPR_END
GameB_Kind19ObjMetasprite_0:
    db MSPR_ROW, $70,$00, $70,$20, MSPR_END
GameB_Kind19ObjMetasprite_1:
    db $70,$00, $70,$20, MSPR_ROW, $71,$00, $72,$00, MSPR_END
GameB_Kind22ObjMetasprite_0:
    db $cb,$00, MSPR_END
GameB_Kind22ObjMetasprite_1:
    db $cc,$00, MSPR_END
GameB_Kind22ObjMetasprite_2:
    db $cd,$00, MSPR_END
GameB_Kind22ObjMetasprite_3:
    db $ce,$00, MSPR_END
GameB_Kind22ObjMetasprite_4:
    db $cf,$00, $d0,$00, MSPR_END
GameB_Kind22ObjMetasprite_5:
    db MSPR_SKIP, $d3,$00, MSPR_ROW, $d1,$00, $d2,$00, $d1,$20, MSPR_END
GameB_Kind22ObjMetasprite_Hit:
    db $db,$00, MSPR_ROW, $dc,$00, MSPR_END
GameB_Kind22ObjMetasprite_7:
    db MSPR_SKIP, $d5,$00, $d6,$00, $d7,$00, MSPR_ROW, $d8,$00, $d9,$00, $da,$00, MSPR_END
GameB_Kind22ObjMetasprite_7Flip:
    db $d7,$20, $d6,$20, $d5,$20, MSPR_ROW, MSPR_SKIP, $da,$20, $d9,$20, $d8,$20, MSPR_END
GameB_WinMarkerMetasprite_0:
    db $6b,$00, MSPR_END
GameB_WinMarkerMetasprite_1:
    db $6c,$00, MSPR_END
GameB_WinMarkerMetasprite_2:
    db $6c,$20, MSPR_END
GameB_WinMarkerMetasprite_3:
    db $6b,$20, MSPR_END
GameB_WinMarkerMetasprite_Flip0:
    db $6b,$20, MSPR_END
GameB_WinMarkerMetasprite_Flip1:
    db $6c,$20, MSPR_END
GameB_WinMarkerMetasprite_Flip2:
    db $6c,$00, MSPR_END
GameB_WinMarkerMetasprite_Flip3:
    db $6b,$00, MSPR_END
GameB_PeriscopeMetasprite_0:
    db $dd,$00, MSPR_END
GameB_PeriscopeMetasprite_1:
    db $de,$00, MSPR_END
GameB_State0dLabelMetasprite:
    db $80,$00, $81,$00, $82,$00, $83,$00, $84,$00, MSPR_END
GameB_State13LabelMetasprite:
    db $89,$00, $8a,$00, $85,$00, MSPR_END
GameB_CountdownLabelMetasprite:
    db $86,$00, $82,$00, $87,$00, $88,$00, $81,$00, MSPR_END
GameB_EndBannerMetasprite:
    db $df,$00, $e2,$00, $e0,$00, MSPR_ROW, $d4,$00, $e1,$00, $d4,$20, MSPR_ROW, $e0,$60, $e2,$40, $df,$60, MSPR_END
GameB_EndBanner2PMetasprite:
    db $df,$00, $e0,$00, MSPR_ROW, $e0,$60, $df,$60, MSPR_END
Unused_ObjMetasprite_0:
    db $6d,$00, MSPR_END
Unused_ObjMetasprite_1:
    db $6e,$00, MSPR_END
Unused_ObjMetasprite_2:
    db $6f,$00, MSPR_END
GameB_DepthChargeBlinkMetasprite_0:
    db $73,$00, MSPR_END
GameB_DepthChargeBlinkMetasprite_1:
    db $74,$00, MSPR_END
GameB_DepthChargeBlinkMetasprite_2:
    db $76,$00, MSPR_END
GameB_DepthChargeBlinkMetasprite_3:
    db $75,$00, MSPR_END

; 36 bytes. GameB_InitScaledTable copies them to $d3a8, each shifted left
; by (2 - [g_gameBSkill]).
GameB_ScaledTableSource:
    db $00, $28, $28, $32, $32, $32, $32, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00

; 3 pointers. Bank 0 ($392a) returns entry A in HL.
GameB_ObjTypeTablePtrs:
    dw GameB_ObjTypeTableVariant0, GameB_ObjTypeTableVariant1, GameB_ObjTypeTableVariant2

; 3 pointers. GetDifficultyTableByte reads entry A, then byte [g_difficultyColumn] & $0f.
DifficultyTablePtrs:
    dw DifficultyTableSkill0, DifficultyTableSkill1, DifficultyTableSkill2

; Pointers to level layouts. GameB_LoadLevelLayout picks entry
; [g_gameBStage]*3 + [g_gameBLayoutVariant], or entry 9 + min(tally,5)/2 when $d002 bit 3 is set.
LevelLayoutPtrs:
    dw GameB_LevelLayout_Stage1A, GameB_LevelLayout_Stage1B, GameB_LevelLayout_Stage1C, GameB_LevelLayout_Stage2A
    dw GameB_LevelLayout_Stage2B, GameB_LevelLayout_Stage2C, GameB_LevelLayout_Stage3A, GameB_LevelLayout_Stage3B
    dw GameB_LevelLayout_Stage3C, GameB_LevelLayout_Tally1, GameB_LevelLayout_Tally2, GameB_LevelLayout_Tally3

; 3 pointers into the level records below. No reader found in this bank.
GameB_SkillLevelRecordPtrs:
    dw LevelRecord3, LevelRecord4, LevelRecord5
DifficultyTableSkill0:
    db $06, $0c, $12, $06, $12, $1e, $1e, $24, $24, $06
DifficultyTableSkill1:
    db $06, $0c, $12, $06, $12, $1e, $1e, $24, $24, $06
DifficultyTableSkill2:
    db $06, $0c, $12, $06, $12, $1e, $1e, $24, $24, $06
GameB_ObjTypeTableVariant0:
    db $0c, $0c, $09, $09, $07, $07, $0a, $0a, $08, $08, $07, $07, $07, $07, $06, $06
    db $05, $05, $0d, $0d, $0a, $0a, $08, $08, $11, $06, $06, $06, $06, $06, $06, $06
    db $06
GameB_ObjTypeTableVariant1:
    db $0c, $0c, $09, $09, $07, $07, $0a, $0a, $08, $08, $07, $07, $07, $07, $06, $06
    db $05, $05, $0d, $0d, $0a, $0a, $08, $08, $11, $06, $06, $06, $06, $06, $06, $06
    db $06
GameB_ObjTypeTableVariant2:
    db $0c, $0c, $09, $09, $07, $07, $09, $09, $08, $08, $07, $07, $07, $07, $06, $06
    db $05, $05, $0d, $0d, $0a, $0a, $08, $08, $11, $06, $06, $06, $06, $06, $06, $06
    db $06

; Upper limits for random values. GameB_PlaceShips reads entry C+1.
RandomLimitTable:
    db $02, $02, $55, $3f, $33, $2a, $2a, $2a, $2a

; 9 level records of 16 bytes. Each layout below points to one.
LevelRecord0:
    db $04, $0b, $10, $14, $1e, $00, $07, $53, $37, $53, $37, $00, $00, $00, $64, $05
LevelRecord1:
    db $04, $0b, $10, $14, $1e, $01, $07, $53, $37, $53, $37, $01, $00, $01, $64, $05
LevelRecord2:
    db $06, $0b, $10, $14, $1e, $02, $07, $53, $37, $53, $37, $02, $00, $03, $5a, $41
LevelRecord3:
    db $07, $0b, $10, $14, $1e, $04, $07, $53, $37, $53, $37, $03, $00, $03, $50, $41
LevelRecord4:
    db $07, $0b, $10, $14, $1e, $05, $07, $53, $37, $53, $37, $05, $01, $03, $46, $4b
LevelRecord5:
    db $05, $0b, $10, $14, $1e, $06, $07, $53, $37, $53, $37, $06, $02, $04, $3c, $4f
LevelRecord6:
    db $05, $0b, $10, $14, $1e, $06, $06, $53, $37, $53, $37, $07, $04, $04, $3c, $4f
LevelRecord7:
    db $05, $0b, $10, $14, $1e, $07, $06, $53, $37, $53, $37, $07, $05, $05, $32, $4f
LevelRecord8:
    db $07, $0b, $10, $14, $1e, $07, $00, $53, $37, $53, $37, $07, $06, $06, $32, $4f

; Level layouts. Format: width byte W (world width = W * $e0), then ship
; group bytes up to $ff (bank 0 copies them to $d466), then a pointer to
; a LevelRecord. The last 3 layouts have no record pointer.
GameB_LevelLayout_Stage1A:
    db $50, $84, $91, $20, $b3, $b1, $51, $e0, $76, $ff
    dw LevelRecord0
GameB_LevelLayout_Stage2A:
    db $50, $84, $11, $a0, $b3, $40, $d0, $e3, $76, $ff
    dw LevelRecord1
GameB_LevelLayout_Stage3A:
    db $50, $84, $11, $a0, $b0, $43, $d0, $76, $78, $ff
    dw LevelRecord2
GameB_LevelLayout_Stage1B:
    db $50, $84, $13, $10, $a1, $30, $c0, $50, $e3, $73, $78, $ff
    dw LevelRecord3
GameB_LevelLayout_Stage2B:
    db $50, $84, $13, $a0, $b1, $41, $d0, $e2, $73, $76, $78, $ff
    dw LevelRecord4
GameB_LevelLayout_Stage3B:
    db $50, $84, $93, $a3, $30, $c0, $d1, $60, $73, $76, $78, $ff
    dw LevelRecord5
GameB_LevelLayout_Stage1C:
    db $50, $84, $13, $23, $b0, $42, $41, $d3, $d1, $60, $73, $73, $76, $78, $ff
    dw LevelRecord6
GameB_LevelLayout_Stage2C:
    db $50, $84, $13, $13, $a0, $a0, $30, $40, $d3, $e3, $73, $73, $76, $78, $ff
    dw LevelRecord7
GameB_LevelLayout_Stage3C:
    db $50, $84, $10, $23, $b0, $c0, $50, $e0, $e0, $73, $76, $76, $78, $78, $ff
    dw LevelRecord8
GameB_LevelLayout_Tally1:
    db $50, $84, $93, $21, $b1, $c0, $50, $e0, $73, $78, $ff
GameB_LevelLayout_Tally2:
    db $50, $84, $13, $13, $a0, $31, $c0, $e0, $73, $78, $ff
GameB_LevelLayout_Tally3:
    db $50, $84, $10, $b1, $50, $61, $70, $73, $73, $78, $ff

; Unknown or unused bytes.
    db $d0, $20, $09, $00, $80, $0a, $04, $00, $92, $00, $42, $82, $c5, $20, $52, $00
    db $91, $0a, $00, $28, $0a, $00, $a5, $00, $22, $0a, $f7, $20, $b5, $22, $b0, $88
    db $5e, $08, $21, $80, $46, $00, $10, $0a, $20, $80, $02, $20, $2c, $28, $2e, $00
    db $cc, $00, $61, $8a, $69, $08, $48, $00, $a0, $08, $01, $82, $42, $0a, $48, $2a
    db $11, $02, $35, $00, $02, $20, $81, $02, $3c, $00, $00, $82, $42, $00, $00, $82
    db $03, $22, $24, $a2, $1c, $80, $00, $82, $24, $a2, $bb, $aa, $28, $28, $ca, $8a
    db $6a, $82, $2e, $00, $03, $82, $9a, $22, $a4, $00, $00, $02, $06, $a0, $12, $a0
    db $0e, $0a, $00, $00, $20, $a0, $b0, $00, $e6, $22, $13, $02, $10, $00, $22, $2a
    db $01, $00, $31, $88, $03, $02, $06, $02, $80, $00, $42, $8a, $20, $00, $55, $0a
    db $28, $00, $04, $00, $10, $00, $0a, $00, $c0, $a2, $99, $aa, $2a, $02, $a2, $02
    db $92, $a0, $a9, $20, $fa, $00, $ad, $a8, $34, $88, $10, $20, $c2, $00, $88, $0a
    db $73, $20, $80, $08, $26, $80, $5c, $80, $34, $28, $08, $00, $62, $80, $88, $08
    db $40, $80, $ed, $22, $08, $00, $02, $02, $0e, $00, $80, $80, $80, $00, $41, $80
    db $c2, $00, $80, $00, $91, $00, $54, $08, $20, $80, $eb, $88, $92, $02, $87, $8a
    db $ec, $00, $ca, $88, $60, $a0, $14, $82, $49, $00, $10, $00, $06, $20, $05, $8a
    db $a0, $80, $0c, $88, $84, $20, $11, $aa, $1d, $bf, $5f, $7b, $54, $d5, $55, $f7
    db $55, $d6, $45, $b8, $47, $ff, $55, $db, $54, $af, $41, $2c, $77, $be, $55, $77
    db $15, $7c, $50, $df, $54, $fa, $75, $ff, $54, $bf, $5d, $f5, $55, $b3, $50, $7a
    db $55, $fe, $d5, $df, $51, $ef, $11, $ff, $d5, $75, $15, $e3, $11, $fa, $73, $df
    db $41, $ff, $53, $fd, $54, $ef, $55, $7b, $54, $7f, $55, $92, $45, $bb, $55, $f6
    db $55, $79, $d4, $f1, $55, $8f, $55, $ef, $55, $2f, $43, $ff, $55, $fe, $54, $ff
    db $d0, $7e, $45, $dd, $55, $92, $45, $fe, $d5, $57, $55, $f3, $45, $ea, $50, $f6
    db $51, $99, $11, $bb, $60, $9f, $75, $fd, $55, $de, $41, $e5, $57, $fe, $75, $ff
    db $54, $65, $35, $ff, $55, $f6, $d2, $ff, $c5, $ff, $05, $bf, $74, $fb, $4d, $2e
    db $51, $23, $55, $ef, $50, $fe, $44, $76, $41, $ff, $55, $63, $77, $ff, $11, $7b
    db $55, $9f, $f5, $6f, $44, $be, $55, $fb, $d1, $7f, $45, $ff, $05, $f1, $95, $fd
    db $05, $dd, $4d, $fb, $14, $ff, $55, $ff, $c4, $3c, $55, $ff, $19, $cd, $0d, $75
    db $c5, $5b, $59, $5b, $01, $ff, $35, $3d, $53, $f7, $45, $f9, $55, $db, $5c, $9a
    db $23, $77, $55, $03, $54, $ba, $15, $df, $25, $de, $35, $bb, $f5, $57, $4c, $7b
    db $45, $3a, $54, $bf, $55, $e7, $15, $7e, $55, $af, $01, $6d, $51, $96, $d5, $f9
    db $55, $97, $53, $ad, $51, $af, $51, $67, $54, $f9, $15, $ef, $55, $f7, $50, $eb
    db $05, $d6, $54, $ff, $57, $ed, $54, $df, $75

; Ending tiles. GameB_Ending_Load copies them to VRAM.
GameB_EndingGfxTilesB:
    db $00, $00, $00, $00, $3d, $02, $0d, $2f, $dd, $e2, $9d, $62, $1d, $22, $20, $1f
    db $03, $3c, $83, $57, $83, $7f, $ff, $fc, $83, $7c, $3c, $bf, $83, $7c, $00, $ff
    db $00, $00, $e0, $00, $fe, $c0, $ff, $3c, $ff, $03, $ff, $00, $fc, $03, $00, $fc
    db $1e, $00, $1e, $1e, $3e, $10, $fe, $20, $ff, $de, $e1, $1e, $00, $fe, $00, $00
    db $00, $00, $f8, $00, $f8, $40, $fc, $40, $fc, $40, $fc, $60, $fe, $60, $fe, $60
    db $00, $00, $3d, $03, $0d, $2f, $dd, $e2, $9d, $62, $1d, $22, $21, $1e, $00, $00
    db $c3, $7c, $83, $d7, $ab, $fc, $c3, $3c, $ff, $00, $00, $ff, $ff, $00, $ff, $00
    db $00, $00, $e0, $00, $fe, $c0, $ff, $1e, $ff, $01, $7f, $80, $bc, $40, $00, $00
    db $1e, $00, $1e, $1e, $3e, $10, $fe, $20, $ff, $c0, $83, $7c, $7c, $00, $3e, $20
    db $ff, $60, $fe, $60, $fe, $60, $fe, $60, $fe, $40, $fc, $40, $fc, $40, $fc, $40
    db $3e, $10, $1e, $10, $1c, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $fc, $40, $fc, $40, $f8, $00, $f8, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $fc, $00
    db $fc, $40, $fc, $40, $fc, $40, $fc, $40, $fe, $40, $fe, $40, $fe, $40, $fe, $40
    db $fe, $40, $fe, $60, $fe, $60, $ff, $60, $ff, $60, $ff, $60, $ff, $60, $ff, $60
    db $00, $00, $00, $00, $00, $00, $00, $00, $0c, $00, $0e, $04, $1e, $08, $1e, $08
    db $00, $00, $1d, $22, $0d, $2f, $dd, $ff, $9d, $62, $1d, $22, $20, $1f, $00, $00
    db $ff, $60, $ff, $00, $c3, $ff, $83, $d7, $ab, $7c, $c3, $3c, $00, $ff, $ff, $00
    db $80, $00, $e0, $00, $ff, $e0, $ff, $ff, $ff, $00, $ff, $00, $00, $e0, $80, $00
    db $3e, $10, $3e, $10, $fc, $00, $ff, $bf, $ff, $00, $80, $7c, $3e, $00, $3e, $00
    db $fc, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $1e, $08, $1e, $08, $0e, $04, $0c, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $3d, $02, $0d, $2f, $dd, $e2, $9d, $62, $1d, $22, $20, $1f, $00, $00, $00, $00
    db $ff, $00, $01, $01, $7c, $ff, $83, $7c, $ff, $00, $03, $fc, $83, $54, $00, $3f
    db $fe, $00, $ff, $fc, $ff, $1f, $ff, $00, $fe, $01, $e0, $1e, $00, $e0, $00, $00
    db $00, $00, $fe, $00, $ff, $de, $e1, $1e, $3f, $c0, $1e, $20, $1e, $00, $1e, $00
    db $00, $00, $3d, $02, $0d, $2f, $dd, $ff, $9d, $62, $1d, $22, $20, $1f, $00, $00
    db $ff, $60, $ff, $00, $ff, $ff, $ff, $ff, $ff, $00, $ff, $00, $00, $ff, $ff, $00
    db $80, $00, $e0, $00, $ff, $e0, $ff, $ff, $ff, $00, $e0, $1f, $00, $e0, $80, $80
    db $1e, $08, $3e, $10, $fe, $00, $ff, $ff, $ff, $00, $00, $fe, $3e, $00, $1e, $00

; Ending tiles. GameB_Ending_Load copies them to VRAM in two parts.
GameB_EndingGfxTilesA:
    db $00, $00, $00, $00, $00, $1c, $00, $7f, $00, $ff, $00, $7f, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $18, $00, $38, $00, $f8, $00, $fc, $00, $00, $00, $00
    db $00, $00, $00, $00, $3c, $3c, $3c, $20, $3e, $3a, $7e, $7e, $7f, $7d, $70, $70
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $0f, $0f, $ff, $f0, $7f, $00
    db $00, $00, $00, $00, $00, $00, $0e, $0f, $fe, $e1, $fe, $01, $fe, $01, $ff, $00
    db $00, $00, $00, $00, $00, $00, $00, $f8, $00, $2c, $5b, $27, $0f, $f0, $ff, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $7c, $f8, $7c, $8c, $7b, $84
    db $6f, $6f, $40, $7f, $00, $7f, $00, $03, $00, $00, $00, $00, $00, $00, $00, $00
    db $bf, $bf, $3f, $ff, $00, $ff, $00, $ff, $00, $03, $00, $00, $00, $00, $00, $00
    db $fe, $fc, $fd, $fd, $fb, $fb, $03, $ff, $00, $ff, $00, $00, $00, $00, $00, $00
    db $07, $07, $f3, $f3, $fb, $fb, $f8, $ff, $00, $ff, $00, $00, $00, $00, $00, $00
    db $79, $fd, $78, $ff, $70, $fc, $00, $fc, $04, $f8, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $ff, $ff, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $ff, $ff, $ff, $ff, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $00, $00
    db $ff, $00, $ff, $ff, $ff, $00, $ff, $00, $ff, $ff, $ff, $00, $ff, $ff, $ff, $00

; Unknown or unused bytes.
    db $77, $35, $7f, $10, $bf, $e5, $bb, $15, $4d, $57, $df, $45, $c5, $1d, $ef, $14
    db $fd, $55, $fe, $51, $bf, $d5, $cd, $05, $6e, $74, $6d, $44, $be, $51, $f9, $55
    db $ff, $45, $3f, $34, $db, $55, $1f, $95, $fb, $15, $f5, $75, $f9, $55, $fb, $55

; 128 tiles. Bank 0 copies $800 bytes to VRAM $8000.
GameB_GfxSprites:
    db $00, $00, $00, $07, $03, $1f, $0f, $37, $1e, $2f, $18, $7f, $31, $5f, $33, $4d
    db $00, $00, $00, $fe, $fe, $ff, $ff, $ff, $01, $ff, $00, $ff, $74, $8f, $76, $8d
    db $00, $00, $00, $00, $00, $80, $00, $c0, $80, $c0, $c0, $a0, $c0, $a0, $40, $a0
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $01, $02, $00, $02
    db $07, $39, $0f, $10, $06, $09, $03, $1c, $09, $3e, $1a, $ff, $ed, $1f, $ee, $1f
    db $ee, $9d, $ff, $00, $fa, $fd, $06, $f9, $fc, $03, $02, $ff, $b6, $ff, $cd, $ff
    db $00, $c0, $00, $80, $00, $00, $00, $00, $00, $c0, $80, $e0, $c0, $f0, $e0, $fe
    db $00, $02, $00, $03, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00
    db $6f, $1f, $1f, $ff, $ff, $3b, $77, $9b, $0f, $f3, $01, $1f, $1e, $3f, $1f, $3f
    db $6b, $ff, $b7, $ff, $cf, $fe, $ff, $fe, $ff, $fe, $fc, $ff, $02, $ff, $ff, $ff
    db $ee, $f1, $6c, $f1, $70, $bf, $3c, $c2, $00, $bc, $00, $80, $00, $00, $00, $80
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $03, $03, $05, $00, $07
    db $3f, $7f, $3f, $7e, $7e, $ff, $7e, $fd, $fc, $ff, $04, $fa, $e0, $86, $00, $fc
    db $ff, $1f, $1f, $ef, $0f, $97, $07, $0b, $00, $0f, $00, $04, $00, $07, $00, $00
    db $00, $80, $80, $c0, $80, $c0, $c0, $e0, $00, $f8, $f8, $34, $00, $fc, $00, $00
    db $00, $00, $00, $03, $01, $04, $01, $04, $00, $04, $00, $03, $03, $04, $03, $04
    db $07, $39, $0f, $d0, $c6, $29, $c3, $2c, $49, $3e, $1a, $ff, $bd, $ff, $fe, $ff
    db $01, $06, $00, $03, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $ff, $7f, $ef, $1f, $1f, $ef, $3f, $4f, $1f, $6f, $01, $1f, $1e, $3f, $1f, $3f
    db $00, $01, $01, $02, $01, $02, $01, $02, $00, $03, $01, $03, $01, $03, $01, $03
    db $00, $f0, $f0, $0f, $f3, $0f, $07, $3f, $36, $cf, $c8, $3f, $f1, $3f, $f3, $3d
    db $01, $03, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $f7, $39, $fb, $3c, $fa, $3d, $7d, $9e, $3d, $4e, $3d, $4f, $1e, $2f, $1f, $2f
    db $1f, $2f, $1f, $2f, $1f, $2f, $3f, $4f, $1f, $6f, $01, $1f, $1e, $3f, $1f, $3f
    db $00, $00, $00, $00, $00, $00, $00, $01, $01, $03, $01, $02, $00, $01, $00, $01
    db $00, $00, $00, $00, $00, $f0, $f0, $ff, $db, $fc, $e4, $7b, $f4, $7b, $c0, $3f
    db $00, $00, $00, $78, $38, $fc, $7c, $fe, $7c, $f6, $76, $cb, $f8, $4f, $3a, $c5
    db $00, $01, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $01, $00, $01
    db $d6, $39, $55, $bf, $53, $ff, $6c, $ff, $73, $ff, $ff, $ff, $80, $ff, $7f, $80
    db $bc, $e6, $bc, $f6, $7c, $f6, $f8, $f4, $f8, $e4, $f0, $ec, $70, $e8, $90, $68
    db $00, $01, $00, $01, $01, $03, $03, $07, $07, $0f, $07, $0b, $03, $0c, $00, $07
    db $00, $ff, $ff, $ff, $ff, $ff, $ef, $ff, $df, $ff, $df, $ef, $cf, $30, $00, $ff
    db $20, $d8, $c0, $f0, $e0, $de, $d6, $a9, $ba, $5d, $fa, $f5, $f4, $0a, $00, $fc
    db $00, $00, $00, $00, $00, $01, $00, $03, $03, $07, $07, $0f, $0f, $1e, $0e, $1f
    db $00, $61, $61, $9e, $cd, $3e, $3e, $ff, $7e, $ff, $7e, $bf, $3e, $c1, $80, $7f
    db $00, $e0, $e0, $10, $e0, $10, $00, $f0, $f0, $f8, $f0, $f8, $f0, $f8, $f0, $78
    db $0f, $11, $07, $08, $01, $06, $00, $01, $00, $00, $00, $00, $00, $00, $00, $01
    db $c0, $bf, $de, $e1, $c0, $7f, $d5, $2a, $53, $ff, $6c, $ff, $73, $ff, $ff, $ff
    db $f8, $0c, $78, $8c, $78, $cc, $78, $cc, $70, $e8, $f0, $c8, $f0, $c8, $f0, $88
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $03
    db $00, $00, $00, $00, $00, $03, $03, $0f, $00, $3f, $3f, $ff, $07, $ff, $f9, $ff
    db $00, $00, $00, $00, $00, $ff, $fe, $ff, $07, $ff, $f3, $ff, $cf, $ff, $bf, $ff
    db $00, $00, $00, $00, $00, $80, $80, $e0, $60, $f0, $a0, $f0, $b0, $f8, $b0, $f8
    db $01, $07, $03, $0f, $0b, $1f, $0b, $3f, $1b, $3d, $1b, $2d, $1d, $22, $00, $1f
    db $fe, $ff, $f1, $ff, $ee, $f1, $ee, $f1, $ed, $f3, $f1, $fe, $fa, $05, $00, $ff
    db $7f, $ff, $7f, $bf, $3d, $da, $d8, $ff, $fb, $f4, $f3, $ed, $e7, $18, $00, $ff
    db $a0, $f8, $a0, $70, $60, $de, $d6, $a9, $ba, $dd, $fa, $f5, $f4, $0a, $00, $fc
    db $00, $0f, $0f, $3f, $3f, $70, $60, $df, $40, $ff, $4d, $b6, $1d, $66, $1f, $20
    db $00, $f0, $f0, $fc, $fc, $0e, $06, $fb, $02, $ff, $b2, $6d, $b8, $66, $78, $84
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $01, $00, $01
    db $1f, $20, $1c, $23, $0f, $10, $00, $7f, $3b, $e4, $5c, $fb, $ed, $ff, $f6, $ef
    db $00, $01, $00, $01, $00, $01, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00
    db $ba, $d7, $b9, $db, $bc, $dd, $00, $ff, $bf, $5f, $3f, $de, $3e, $5c, $3e, $4d
    db $3c, $5e, $00, $7e, $7c, $82, $00, $fe, $00, $00, $00, $00, $00, $00, $00, $00
    db $1f, $20, $1c, $23, $0f, $10, $00, $7f, $3b, $e4, $4c, $fb, $65, $ff, $7a, $ff
    db $3c, $5f, $0b, $7c, $33, $5c, $00, $7f, $3f, $5f, $3f, $5e, $3e, $5c, $3e, $4d
    db $5d, $ef, $1d, $db, $3d, $bb, $00, $ff, $fd, $fa, $fc, $7b, $7c, $3a, $7c, $b2
    db $1f, $20, $1c, $23, $0f, $10, $00, $3f, $1b, $64, $3c, $7f, $3f, $5f, $2f, $57
    db $f8, $04, $38, $c4, $f0, $08, $00, $fc, $dc, $26, $ba, $5f, $03, $ff, $f5, $fb
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $80, $c0
    db $33, $4d, $38, $5b, $3c, $5d, $00, $7f, $3f, $5f, $3f, $5e, $3e, $5c, $3e, $4d
    db $f5, $fa, $00, $ff, $3c, $ba, $00, $fe, $fc, $fa, $fc, $7a, $7c, $3a, $7c, $b2
    db $c0, $e0, $e0, $70, $50, $a8, $30, $48, $00, $30, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $60, $60, $90, $60, $90, $00, $f0
    db $1f, $20, $1c, $23, $0f, $10, $00, $3f, $1b, $64, $3c, $7f, $3f, $7f, $37, $5b
    db $f8, $04, $38, $c5, $f0, $0f, $0d, $f2, $d5, $2b, $3b, $ff, $7a, $f5, $f4, $ef
    db $60, $b0, $60, $b0, $e0, $70, $c0, $f0, $c0, $a0, $00, $c0, $00, $80, $00, $00
    db $39, $56, $38, $5b, $3c, $5d, $00, $7f, $3f, $5f, $3f, $5e, $3e, $5c, $3e, $4d
    db $cc, $3e, $3c, $fa, $3c, $ba, $00, $fe, $fc, $fa, $fc, $7a, $7c, $3a, $7c, $b2
    db $00, $f0, $f0, $fc, $fc, $0e, $06, $fb, $02, $ff, $b2, $6d, $b8, $66, $48, $85
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $e0, $e0, $10
    db $1b, $24, $1c, $23, $0f, $10, $00, $3f, $19, $66, $3e, $7f, $3f, $7f, $2f, $57
    db $d8, $25, $38, $c4, $f0, $08, $00, $fe, $c6, $39, $3a, $fd, $f9, $77, $f7, $ef
    db $e0, $10, $00, $f8, $30, $78, $30, $78, $30, $f8, $b0, $f8, $f0, $f8, $e0, $f0
    db $33, $5d, $38, $5b, $3c, $5d, $00, $7f, $3f, $5f, $3f, $5e, $3e, $5c, $3e, $4d
    db $cc, $bf, $1c, $fa, $3c, $ba, $00, $fe, $fc, $fa, $fc, $7a, $7c, $3a, $7c, $b2
    db $00, $e0, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $07, $03, $0f, $07, $1f, $0c, $1f, $08, $1f, $03, $0c, $00, $1f
    db $00, $00, $00, $fe, $fc, $fb, $ff, $fc, $07, $fe, $01, $fe, $e0, $07, $00, $ff
    db $00, $00, $00, $00, $00, $00, $00, $80, $00, $c0, $80, $40, $80, $40, $80, $40
    db $1f, $2f, $3f, $5f, $00, $7f, $36, $c9, $79, $ff, $7f, $ff, $7f, $ff, $00, $ff
    db $f6, $e9, $ef, $d6, $0f, $fb, $fd, $0b, $f9, $f6, $fa, $f5, $f3, $e8, $05, $f0
    db $00, $80, $00, $80, $80, $40, $c0, $a0, $e0, $10, $c0, $30, $00, $e0, $80, $40
    db $7f, $ff, $7f, $ff, $3f, $40, $00, $3f, $00, $00, $00, $00, $00, $00, $00, $00
    db $fd, $f2, $f8, $e7, $f0, $0c, $00, $ff, $00, $00, $00, $00, $00, $00, $00, $00
    db $80, $40, $00, $e0, $00, $30, $00, $f0, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $07, $03, $0f, $07, $0f, $04, $0f, $00, $07, $03, $0c, $00, $1f
    db $00, $00, $00, $f8, $f0, $fe, $fc, $fb, $1e, $fd, $07, $fc, $c1, $1e, $00, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $00, $c0, $80, $40, $80, $40
    ds $00f1, $00
    db $0f, $0f, $1f, $00, $0f, $00, $0f, $05, $0a, $0f, $12, $07, $08, $07, $08, $00
    db $f8, $f8, $fc, $00, $f8, $00, $f8, $d0, $28, $f8, $24, $f0, $08, $70, $88, $08
    db $1f, $1a, $3f, $1d, $37, $1e, $37, $00, $1f, $07, $0f, $03, $07, $00, $03, $08
    db $fc, $ac, $7e, $5c, $f6, $3c, $f6, $00, $fc, $f0, $f8, $60, $f0, $00, $e0, $00
    db $0f, $0f, $1f, $00, $0f, $00, $0f, $05, $0a, $0f, $32, $26, $59, $36, $79, $00
    db $f8, $f8, $fc, $00, $f8, $00, $f8, $d0, $28, $f8, $26, $32, $cd, $36, $cf, $1a
    db $3f, $0d, $1f, $0e, $1f, $00, $3f, $2f, $5f, $10, $2f, $00, $18, $00, $00, $2c
    db $fe, $58, $fc, $38, $fc, $00, $fe, $fa, $fd, $04, $fa, $00, $0c, $00, $00, $00
    db $00, $00, $07, $07, $0b, $0f, $17, $0f, $17, $0f, $13, $07, $08, $00, $0f, $00
    db $00, $00, $f0, $f0, $f8, $f8, $fc, $f8, $fc, $f8, $fc, $f0, $18, $00, $f8, $03
    db $08, $08, $1f, $1d, $3f, $1e, $37, $10, $2f, $07, $1f, $03, $07, $00, $03, $e0
    db $08, $08, $fc, $5c, $fe, $3c, $f6, $04, $fa, $f0, $fc, $60, $f0, $00, $e0, $00
    db $0f, $0f, $1f, $00, $0f, $00, $0f, $07, $08, $0f, $10, $07, $08, $00, $0f, $00
    db $f8, $f8, $fc, $00, $f8, $00, $f8, $f0, $08, $f8, $04, $f0, $08, $00, $f8, $0b
    db $1f, $1b, $3f, $18, $37, $1f, $36, $00, $1f, $07, $0f, $03, $07, $00, $03, $e8
    db $fc, $ec, $fe, $0c, $f6, $7c, $b6, $00, $fc, $f0, $f8, $60, $f0, $00, $e0, $00
    db $0f, $0f, $1f, $00, $0f, $00, $0f, $05, $0a, $0f, $12, $06, $09, $00, $07, $00
    db $e0, $e0, $f8, $38, $fe, $0e, $ff, $c0, $3c, $e8, $14, $f0, $08, $00, $f0, $02
    db $07, $05, $0f, $03, $0f, $00, $0f, $07, $0f, $00, $07, $03, $04, $00, $07, $e0
    db $f8, $b0, $f8, $b0, $f8, $00, $f8, $a0, $d0, $00, $f0, $c0, $20, $00, $e0, $00
    db $0f, $0f, $1f, $00, $0f, $00, $0f, $05, $0a, $0f, $32, $26, $59, $30, $7f, $00
    db $e0, $e0, $f8, $38, $fe, $0e, $ff, $c0, $3c, $e8, $16, $f2, $0d, $06, $ff, $1b
    db $3f, $08, $1f, $0f, $1e, $00, $3f, $2f, $5f, $10, $2f, $00, $18, $00, $00, $ec
    db $fe, $08, $fc, $78, $3c, $00, $fe, $fa, $fd, $04, $fa, $00, $0c, $00, $00

; Tiles. No direct reader found.
Unused_GfxTiles:
    db $00, $0f, $0f, $3f, $3f, $70, $60, $df, $40, $ff, $4d, $b6, $1d, $66, $13, $20
    db $00, $f0, $f0, $fc, $fc, $0e, $06, $fb, $02, $ff, $b2, $6d, $b8, $66, $48, $84
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $01, $00, $01
    db $1f, $20, $1c, $23, $0f, $10, $00, $7f, $3b, $e4, $5c, $fb, $ed, $ff, $f6, $ef
    db $00, $01, $00, $01, $00, $01, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00
    db $ba, $d7, $b9, $db, $bc, $dd, $00, $ff, $bf, $5f, $3f, $de, $3e, $5c, $3e, $4d
    db $3c, $5e, $00, $7e, $7c, $82, $00, $fe, $00, $00, $00, $00, $00, $00, $00, $00
    db $1f, $20, $1c, $23, $0f, $10, $00, $7f, $3b, $e4, $4c, $fb, $65, $ff, $7a, $ff
    db $3c, $5f, $0b, $7c, $33, $5c, $00, $7f, $3f, $5f, $3f, $5e, $3e, $5c, $3e, $4d
    db $5d, $ef, $1d, $db, $3d, $bb, $00, $ff, $fd, $fa, $fc, $7b, $7c, $3a, $7c, $b2
    db $1f, $20, $1c, $23, $0f, $10, $00, $3f, $1b, $64, $3c, $7f, $3f, $5f, $2f, $57
    db $f8, $04, $38, $c4, $f0, $08, $00, $fc, $dc, $26, $ba, $5f, $03, $ff, $f5, $fb
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $80, $c0
    db $33, $4d, $38, $5b, $3c, $5d, $00, $7f, $3f, $5f, $3f, $5e, $3e, $5c, $3e, $4d
    db $f5, $fa, $00, $ff, $3c, $ba, $00, $fe, $fc, $fa, $fc, $7a, $7c, $3a, $7c, $b2
    db $c0, $e0, $e0, $70, $50, $a8, $30, $48, $00, $30, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $60, $60, $90, $60, $90, $00, $f0
    db $1f, $20, $1c, $23, $0f, $10, $00, $3f, $1b, $64, $3c, $7f, $3f, $7f, $37, $5b
    db $f8, $04, $38, $c5, $f0, $0f, $0d, $f2, $d5, $2b, $3b, $ff, $7a, $f5, $f4, $ef
    db $60, $b0, $60, $b0, $e0, $70, $c0, $f0, $c0, $a0, $00, $c0, $00, $80, $00, $00
    db $39, $56, $38, $5b, $3c, $5d, $00, $7f, $3f, $5f, $3f, $5e, $3e, $5c, $3e, $4d
    db $cc, $3e, $3c, $fa, $3c, $ba, $00, $fe, $fc, $fa, $fc, $7a, $7c, $3a, $7c, $b2
    db $00, $f0, $f0, $fc, $fc, $0e, $06, $fb, $02, $ff, $b2, $6d, $b8, $66, $48, $85
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $e0, $e0, $10
    db $1b, $24, $1c, $23, $0f, $10, $00, $3f, $19, $66, $3e, $7f, $3f, $7f, $2f, $57
    db $d8, $25, $38, $c4, $f0, $08, $00, $fe, $c6, $39, $3a, $fd, $f9, $77, $f7, $ef
    db $e0, $10, $00, $f8, $30, $78, $30, $78, $30, $f8, $b0, $f8, $f0, $f8, $e0, $f0
    db $33, $5d, $38, $5b, $3c, $5d, $00, $7f, $3f, $5f, $3f, $5e, $3e, $5c, $3e, $4d
    db $cc, $bf, $1c, $fa, $3c, $ba, $00, $fe, $fc, $fa, $fc, $7a, $7c, $3a, $7c, $b2
    db $00, $e0, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $07, $03, $0f, $07, $1f, $0c, $1f, $08, $1f, $03, $0c, $00, $1f, $1f, $2f
    db $00, $fe, $fc, $fb, $ff, $fc, $07, $fe, $01, $fe, $e0, $07, $00, $ff, $f6, $e9
    db $00, $00, $00, $00, $00, $80, $00, $c0, $80, $40, $80, $40, $80, $40, $00, $80
    db $3f, $5f, $00, $7f, $36, $c9, $79, $ff, $7f, $ff, $00, $ff, $3f, $40, $00, $ff
    db $ef, $d6, $0d, $f3, $fb, $0d, $f1, $ee, $f6, $e9, $07, $f8, $87, $18, $0d, $f2
    db $00, $80, $80, $40, $c0, $a0, $e0, $10, $c0, $30, $00, $e0, $00, $80, $80, $40
    db $7f, $ff, $7f, $fb, $3f, $40, $00, $3f, $00, $00, $00, $00, $00, $00, $00, $00
    db $fb, $f4, $f0, $ef, $e0, $18, $00, $ff, $00, $00, $00, $00, $00, $00, $00, $00
    db $80, $40, $00, $e0, $00, $30, $00, $f0, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $07, $03, $0f, $07, $0f, $04, $0f, $00, $07, $03, $0c, $00, $1f, $1f, $2f
    db $00, $f8, $f0, $fe, $fc, $fb, $1e, $fd, $07, $fc, $c1, $1e, $00, $ff, $f6, $e9
    db $00, $00, $00, $00, $00, $00, $00, $80, $00, $c0, $80, $40, $80, $40, $00, $80
    db $00, $00, $00, $00, $00, $01, $01, $03, $01, $03, $00, $01, $00, $03, $01, $06
    db $00, $00, $00, $f8, $f8, $f7, $ff, $fc, $0f, $ff, $01, $ff, $f8, $07, $3e, $c1
    db $00, $00, $00, $00, $00, $00, $00, $c0, $c0, $30, $f0, $c8, $78, $e4, $38, $f4
    db $03, $04, $03, $0c, $06, $09, $06, $09, $02, $0d, $01, $06, $00, $01, $02, $0f
    db $53, $ec, $b5, $4e, $ff, $00, $7b, $84, $03, $fc, $07, $f8, $f8, $07, $e0, $1f
    db $1c, $fa, $8c, $7a, $8c, $3a, $cc, $32, $00, $ec, $80, $40, $00, $80, $00, $00
    db $06, $1f, $02, $1f, $0a, $1f, $19, $2f, $1c, $ff, $5f, $bf, $40, $bf, $17, $78
    db $0f, $fc, $9c, $ff, $73, $ff, $8f, $ff, $7e, $ff, $fe, $fd, $0e, $e5, $e1, $1e
    db $00, $80, $80, $40, $c0, $a0, $e0, $d0, $f0, $e8, $f0, $68, $30, $e8, $d0, $28
    db $18, $3f, $1f, $3f, $1e, $3f, $1e, $3d, $1e, $39, $02, $7d, $70, $83, $00, $ff
    db $1c, $fb, $fc, $fb, $fc, $fa, $fe, $fd, $fe, $7d, $40, $ff, $3f, $c0, $00, $7f
    db $e0, $10, $c0, $20, $00, $c0, $00, $00, $00, $00, $00, $80, $80, $40, $00, $c0
    db $00, $7e, $3c, $ff, $5a, $ff, $5a, $ff, $7e, $ff, $42, $ff, $3c, $ff, $00, $7e
    db $00, $7e, $3c, $ff, $5a, $ff, $5a, $ff, $7e, $ff, $42, $ff, $3c, $ff, $00, $7e
    db $00, $00, $01, $01, $03, $02, $03, $02, $07, $04, $ff, $fc, $ff, $80, $7f, $40
    db $00, $00, $80, $80, $c0, $40, $c0, $40, $e0, $20, $ff, $3f, $ff, $01, $fe, $02
    db $3f, $20, $1f, $10, $3f, $20, $3f, $20, $7f, $41, $7e, $46, $f8, $98, $e0, $e0
    db $fc, $04, $f8, $08, $fc, $04, $fc, $04, $fe, $82, $7e, $62, $1f, $19, $07, $07
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $01, $03, $02
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $80, $c0, $40
    db $ff, $fc, $ff, $80, $3f, $20, $1f, $10, $3f, $20, $7e, $46, $f8, $98, $e0, $e0
    db $ff, $3f, $ff, $01, $fc, $04, $f8, $08, $fc, $04, $7e, $62, $1f, $19, $07, $07
    db $00, $00, $00, $00, $01, $01, $ff, $80, $7f, $70, $3f, $23, $7c, $4c, $e0, $e0
    db $00, $00, $00, $00, $80, $80, $ff, $01, $fe, $0e, $fc, $c4, $3e, $3a, $07, $07
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $ff, $ff, $ff, $ff, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $ff, $ff, $ff, $ff, $ff
    db $00, $03, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00, $00, $c0, $00, $c0
    db $00, $e0, $60, $f8, $38, $7c, $1c, $3e, $1c, $3e, $0e, $1f, $0e, $1f, $1e, $3f
    db $40, $e0, $60, $f8, $38, $7f, $3f, $7f, $1f, $3f, $07, $1f, $00, $07, $00, $00
    db $1e, $3f, $3e, $ff, $fc, $fe, $fc, $fe, $f8, $fc, $e0, $f8, $00, $e0, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $03, $00, $00, $00, $00, $00, $40
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $f0, $30, $fc, $1c, $3e, $0c, $1e
    db $00, $c0, $40, $e0, $60, $f8, $38, $7f, $1f, $7f, $07, $3f, $00, $0f, $00, $00
    db $0e, $1f, $0e, $3f, $1e, $7f, $3c, $ff, $f8, $fe, $e0, $fc, $00, $f0, $00, $00
    db $00, $03, $00, $00, $00, $70, $00, $e0, $60, $ff, $3f, $ff, $0f, $7f, $00, $1f
    db $00, $f8, $30, $fe, $0c, $3f, $0e, $3f, $3c, $ff, $f8, $fe, $e0, $fc, $00, $f0
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $00, $ff, $00, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $00, $ff, $00, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $01, $01, $03, $01, $03
    db $00, $03, $03, $1f, $1f, $7f, $7f, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
    db $00, $ff, $ff, $fe, $ff, $ff, $ff, $ff, $ff, $ff, $c1, $ff, $bc, $fb, $be, $fd
    db $00, $00, $00, $c0, $c0, $20, $e0, $90, $e0, $d0, $f0, $c8, $f0, $e8, $f0, $e8
    db $03, $07, $03, $07, $07, $0f, $0f, $1f, $1f, $3f, $3e, $7f, $7d, $fe, $60, $ff
    db $ff, $ff, $f0, $ff, $cf, $f0, $b0, $cf, $40, $bf, $a0, $7f, $30, $ff, $0f, $bf
    db $be, $fd, $3e, $fd, $be, $41, $00, $ff, $00, $ff, $00, $ff, $7f, $ff, $80, $ff
    db $f0, $c8, $60, $90, $a0, $50, $40, $a0, $00, $e0, $00, $e0, $80, $f0, $70, $f8
    db $5e, $ff, $12, $7f, $19, $77, $1d, $3b, $0f, $1f, $01, $0f, $01, $03, $02, $07
    db $00, $8f, $f3, $70, $ef, $6f, $df, $df, $de, $df, $fd, $fe, $fd, $fe, $fd, $ff
    db $00, $82, $fe, $2f, $ce, $ff, $31, $ce, $ff, $70, $ff, $fe, $ff, $ff, $ff, $ff
    db $00, $78, $00, $20, $60, $d0, $60, $d0, $a0, $70, $a0, $70, $a0, $70, $a0, $50
    db $04, $1f, $04, $fe, $08, $fe, $08, $fc, $08, $fc, $18, $fc, $0c, $ff, $02, $ff
    db $fd, $ff, $3f, $ff, $0f, $3f, $00, $0f, $00, $04, $00, $03, $00, $01, $00, $80
    db $7f, $ff, $81, $fe, $ff, $c0, $00, $ff, $33, $cc, $f3, $39, $f7, $7b, $3b, $fd
    db $e8, $1c, $c4, $2f, $02, $c7, $02, $c7, $01, $a3, $81, $63, $c0, $91, $e0, $d1
    db $00, $00, $00, $00, $00, $01, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00
    db $00, $3f, $3f, $df, $7f, $bf, $e0, $7f, $80, $7f, $1b, $ec, $3f, $4c, $27, $40
    db $00, $f8, $f8, $f6, $fc, $fb, $0e, $fd, $02, $fd, $b0, $6e, $f8, $64, $48, $84
    db $3f, $40, $3f, $40, $3f, $40, $3e, $41, $1e, $21, $07, $18, $1b, $64, $5d, $be
    db $f8, $04, $f8, $04, $f8, $04, $78, $84, $70, $88, $c0, $30, $b0, $4c, $74, $fa
    db $00, $00, $00, $00, $00, $01, $00, $01, $00, $01, $00, $01, $00, $01, $00, $00
    db $6c, $ff, $76, $ff, $db, $7f, $dd, $7f, $de, $7f, $df, $6f, $1f, $ef, $40, $bf
    db $6c, $fe, $dc, $fe, $b6, $fd, $76, $fd, $f6, $fd, $f6, $ed, $f0, $ef, $04, $fa
    db $1f, $6f, $1f, $2f, $1e, $2f, $1e, $2f, $1e, $2f, $00, $3f, $3c, $41, $00, $7f
    db $f0, $ec, $f0, $e8, $f0, $e8, $f0, $e8, $f0, $e8, $00, $f8, $78, $04, $00, $fc
    db $00, $3f, $3f, $df, $7f, $bf, $e0, $7f, $80, $7f, $17, $ec, $3f, $4c, $27, $40
    db $00, $f8, $f8, $f6, $fc, $fb, $0e, $fd, $02, $fd, $b0, $6e, $f8, $64, $48, $84
    db $00, $3f, $3f, $df, $7f, $bf, $e0, $7f, $80, $7f, $17, $ec, $37, $4c, $27, $40
    db $00, $f8, $f8, $f6, $fc, $fb, $0e, $fd, $02, $fd, $b0, $6e, $b8, $64, $48, $84
    db $c3, $3c, $bd, $42, $5a, $a5, $7e, $81, $5a, $a5, $66, $99, $bd, $42, $c3, $3c
    db $c3, $3c, $bd, $42, $5a, $a5, $7e, $81, $5a, $a5, $66, $99, $bd, $42, $c3, $3c
    db $e5, $0b, $e5, $0b, $e5, $0b, $a5, $cb, $05, $cb, $85, $0b, $e5, $0b, $e5, $0b
    db $e5, $0b, $e5, $0b, $e5, $0b, $e5, $0b, $e5, $0b, $e5, $0b, $e5, $0b, $e5, $0b
    db $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $00, $00, $00, $ff
    db $0f, $c0, $c3, $30, $f0, $0c, $fc, $03, $ff, $00, $ff, $00, $ff, $00, $ff, $00
    db $ff, $00, $ff, $00, $ff, $00, $3f, $00, $0f, $c0, $c3, $30, $f0, $0c, $fc, $03
    db $00, $ff, $7e, $fd, $7c, $c1, $7a, $81, $68, $c1, $50, $81, $00, $ff, $ff, $00
    db $e5, $0b, $e5, $0b, $e5, $0b, $e5, $0b, $e5, $0b, $e5, $0b, $05, $0b, $00, $ff
    db $ff, $00, $ff, $00, $81, $7e, $81, $40, $ff, $00, $81, $7e, $81, $40, $ff, $00
    db $f7, $08, $e3, $1c, $80, $7f, $c1, $1c, $c1, $36, $dd, $22, $ff, $00, $ff, $00
    db $ff, $00, $ff, $00, $ff, $00, $9f, $60, $87, $58, $e1, $06, $f9, $00, $3f, $00
    db $f3, $08, $f9, $04, $f9, $06, $b1, $46, $81, $3c, $c3, $18, $ff, $00, $ff, $00
    db $00, $7e, $3c, $ff, $5a, $ff, $5a, $ff, $7e, $ff, $42, $ff, $3c, $ff, $00, $7e
    db $00, $7e, $3c, $ff, $5a, $ff, $5a, $ff, $7e, $ff, $42, $ff, $3c, $ff, $00, $7e
    db $00, $7e, $3c, $ff, $5a, $ff, $5a, $ff, $7e, $ff, $42, $ff, $3c, $ff, $00, $7e
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00

; Tiles. Bank 0 copies $620 bytes to VRAM $9040.
; C build: moved to bank 8 to free space in bank 6 for C code.
SECTION "GameB_CutsceneGfx", ROMX, BANK[8]

GameB_CutsceneGfxTiles:
    db $00, $00, $00, $00, $82, $82, $e9, $e9, $dc, $dc, $16, $16, $4a, $4a, $04, $04
    db $28, $28, $8f, $8f, $6b, $6b, $04, $04, $00, $00, $80, $80, $02, $02, $67, $67
    db $80, $80, $68, $68, $c1, $c1, $00, $00, $00, $00, $38, $38, $c1, $c1, $a7, $a7
    db $03, $03, $0e, $0e, $01, $01, $00, $00, $13, $13, $85, $85, $f4, $f4, $7e, $7e
    db $d7, $d7, $8b, $8b, $77, $77, $fe, $fe, $d3, $d3, $3a, $3a, $7f, $7f, $ff, $ff
    db $f9, $f9, $13, $13, $ee, $ee, $9b, $9b, $74, $74, $ff, $ff, $ff, $ff, $ff, $ff
    db $d4, $d4, $be, $be, $ce, $ce, $79, $79, $bf, $bf, $fb, $fb, $ff, $ff, $ff, $ff
    db $ef, $ef, $f9, $f9, $4f, $4f, $1f, $1f, $ab, $ab, $fc, $fc, $ff, $ff, $ff, $ff
    db $ff, $ff, $ff, $ff, $7f, $7f, $9c, $1c, $41, $00, $aa, $00, $ff, $00, $00, $00
    db $ff, $ff, $ff, $ff, $ff, $ff, $c3, $c3, $14, $00, $aa, $00, $ff, $00, $00, $00
    db $ff, $ff, $ff, $ff, $f0, $f0, $0a, $00, $55, $00, $aa, $00, $ff, $00, $00, $00
    db $ff, $ff, $ff, $ff, $7f, $7f, $83, $03, $54, $00, $aa, $00, $ff, $00, $00, $00
    db $ff, $ff, $ff, $ff, $ff, $ff, $fc, $fc, $71, $70, $8a, $00, $ff, $00, $00, $00
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $7f, $7f, $8b, $03, $ff, $00, $00, $00
    db $ff, $7f, $7f, $03, $fb, $f8, $0f, $ff, $0f, $5f, $0f, $ff, $ff, $ff, $00, $ff
    db $ff, $ff, $ff, $f0, $f7, $07, $60, $00, $df, $cf, $ec, $e0, $f6, $f0, $00, $ff
    db $ff, $00, $ff, $ff, $ff, $ff, $00, $00, $ff, $fe, $06, $00, $0d, $01, $00, $ff
    db $ff, $00, $ff, $fe, $fe, $e0, $6f, $0f, $bf, $3f, $ff, $ff, $ff, $ff, $00, $ff
    db $ff, $00, $ef, $0f, $ff, $ff, $80, $ff, $80, $d5, $80, $ff, $ff, $ff, $00, $ff
    db $fc, $7c, $7e, $02, $fb, $f8, $0f, $ff, $0f, $5f, $0f, $ff, $ff, $ff, $00, $ff
    db $34, $34, $f5, $f5, $cb, $0b, $7d, $1d, $de, $ce, $ef, $e3, $f7, $f1, $00, $ff
    db $03, $0b, $03, $0b, $03, $0b, $03, $0b, $03, $0b, $00, $ff, $00, $00, $ff, $ff
    db $f7, $f8, $f4, $f8, $f4, $f8, $f4, $f8, $f4, $f8, $00, $ff, $00, $00, $ff, $ff
    db $fd, $02, $00, $02, $00, $0a, $00, $02, $00, $02, $00, $ff, $00, $00, $ff, $ff
    db $fe, $01, $02, $01, $02, $81, $02, $01, $02, $01, $00, $ff, $00, $00, $ff, $ff
    db $7e, $7f, $7e, $7f, $7e, $7f, $7e, $7f, $7e, $7f, $00, $ff, $00, $00, $ff, $ff
    db $ff, $ff, $ff, $ff, $7f, $7f, $9c, $1c, $41, $00, $00, $ff, $00, $00, $ff, $ff
    db $ff, $ff, $ff, $ff, $ff, $ff, $c3, $c3, $14, $00, $00, $fc, $02, $06, $ff, $ff
    db $ff, $ff, $ff, $f8, $fc, $f0, $fb, $e0, $f4, $c0, $ef, $80, $c0, $00, $ff, $00
    db $ff, $ff, $ff, $00, $03, $00, $f7, $00, $2d, $00, $d9, $00, $31, $00, $e1, $00
    db $80, $00, $bf, $00, $a0, $00, $a0, $00, $a0, $00, $bf, $00, $80, $00, $ff, $00
    db $21, $00, $a1, $00, $a3, $00, $a7, $00, $ad, $00, $be, $00, $31, $00, $ff, $00
    db $ff, $ff, $ff, $07, $1f, $07, $bf, $07, $6f, $07, $cf, $07, $8f, $07, $0f, $07
    db $04, $00, $f5, $00, $15, $00, $15, $00, $15, $00, $f5, $00, $04, $00, $ff, $00
    db $01, $00, $fd, $00, $05, $00, $05, $00, $05, $00, $fd, $00, $01, $00, $ff, $00
    db $0f, $07, $0f, $07, $1f, $07, $3f, $0f, $7f, $1f, $ff, $3f, $ff, $7f, $ff, $ff
    db $f7, $df, $f7, $df, $b3, $df, $45, $fb, $6d, $e3, $28, $e4, $c0, $f8, $e0, $e0
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $7f, $7f, $3f, $3f, $3f, $3f
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $00, $00
    db $c1, $ff, $b0, $f3, $42, $c1, $02, $85, $82, $f9, $c2, $e1, $c0, $de, $e0, $e0
    db $ff, $00, $00, $00, $ff, $ff, $f8, $f8, $f7, $f7, $ef, $e0, $df, $c0, $e0, $e0
    db $ff, $00, $00, $00, $ff, $ff, $03, $03, $f9, $f9, $f9, $01, $fc, $00, $00, $00
    db $ff, $00, $00, $00, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
    db $fe, $00, $00, $03, $fa, $fd, $f6, $f9, $ee, $f1, $de, $e1, $be, $c1, $7e, $81
    db $00, $00, $00, $00, $00, $00, $20, $20, $88, $88, $f0, $f0, $f4, $f4, $e8, $e8
    db $fe, $ff, $fd, $fe, $fb, $fc, $f7, $f8, $ef, $f0, $df, $e0, $bf, $c0, $7f, $80
    db $ff, $01, $ff, $03, $ff, $07, $fd, $0d, $ff, $1f, $ff, $3f, $fe, $7e, $f7, $f7
    db $f6, $f6, $d8, $d8, $b0, $b0, $f4, $f4, $50, $50, $88, $88, $e0, $e0, $00, $00
    db $c3, $ff, $bd, $ff, $c3, $ff, $81, $c3, $81, $ff, $5a, $ff, $42, $ff, $7e, $ff
    db $00, $bd, $bd, $ff, $bd, $ff, $00, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
    db $c3, $ff, $bd, $ff, $c3, $ff, $81, $c3, $81, $ff, $5a, $ff, $43, $ff, $7e, $ff
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $7f, $ff, $9f, $ff, $4f, $df
    db $00, $bf, $bd, $ff, $bd, $ff, $00, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
    db $9f, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
    db $c3, $ff, $bd, $ff, $c3, $ff, $80, $c3, $80, $fe, $5b, $fd, $43, $ff, $7e, $ff
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $7f, $ff, $bf, $ff, $7f, $ff, $ff, $ff
    db $00, $bf, $bd, $ff, $bd, $ff, $00, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
    db $00, $00, $00, $00, $00, $00, $3f, $2a, $15, $3f, $15, $3f, $15, $3f, $15, $3f
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $7f, $00, $bf, $3f, $80, $00
    db $01, $2b, $01, $2b, $01, $2b, $01, $2b, $c1, $2b, $aa, $14, $81, $81, $7f, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $c0, $00, $e0, $80, $60, $00, $a0, $40
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $0e, $01, $30, $0f
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $c0, $00, $3f, $c0, $1f, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $00, $80, $00, $ff, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $ff, $00, $00, $00, $ff, $ff, $00, $00
    db $00, $00, $00, $00, $00, $00, $ff, $00, $00, $00, $ff, $ff, $fe, $fe, $01, $00
    db $00, $00, $00, $00, $ff, $00, $00, $00, $ff, $ff, $ff, $ff, $00, $00, $ff, $00
    db $00, $00, $00, $00, $ff, $00, $00, $01, $fe, $ff, $fc, $fc, $00, $00, $ff, $00
    db $ff, $00, $80, $7f, $80, $7f, $80, $7f, $00, $ff, $00, $ff, $7f, $00, $ff, $00
    db $ca, $35, $0a, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $ff, $00, $ff, $00
    db $30, $c0, $33, $dc, $14, $e7, $0a, $f3, $06, $f9, $00, $fe, $fc, $00, $ff, $00
    db $00, $00, $ff, $00, $80, $00, $7f, $7f, $7f, $7f, $00, $00, $7f, $00, $d0, $2f
    db $00, $00, $ff, $00, $00, $00, $ff, $ff, $ff, $ff, $00, $00, $ff, $00, $01, $fe
    db $00, $00, $ff, $00, $00, $00, $ff, $ff, $ff, $ff, $00, $00, $ff, $00, $00, $ff
    db $00, $00, $ff, $00, $00, $00, $ff, $ff, $ff, $ff, $00, $00, $ff, $00, $00, $ff
    db $00, $00, $ff, $00, $00, $00, $ff, $ff, $ff, $ff, $00, $00, $ff, $00, $00, $ff
    db $00, $00, $c0, $00, $3c, $00, $c3, $c0, $fc, $fc, $3e, $3e, $87, $07, $79, $81
    db $00, $00, $00, $00, $00, $00, $00, $00, $80, $00, $40, $00, $40, $00, $a0, $80
    db $00, $00, $00, $00, $00, $00, $3f, $3f, $0a, $0a, $01, $01, $00, $00, $00, $00
    db $40, $3f, $81, $70, $1e, $e1, $e0, $1f, $ff, $ff, $55, $55, $0a, $0a, $00, $00
    db $00, $00, $ff, $00, $00, $ff, $ff, $00, $f8, $f7, $ff, $ff, $00, $00, $00, $00
    db $1f, $00, $e0, $1f, $1f, $e0, $fe, $00, $00, $ff, $fe, $ff, $01, $01, $00, $00
    db $ff, $00, $00, $ff, $ff, $00, $00, $00, $00, $ff, $00, $ff, $ff, $ff, $00, $00
    db $ff, $00, $00, $ff, $ff, $00, $00, $00, $00, $ff, $00, $ff, $fe, $ff, $01, $01
    db $00, $ff, $ff, $00, $00, $00, $ff, $00, $00, $ff, $00, $ff, $00, $ff, $ff, $ff
    db $00, $00, $00, $00, $0f, $0f, $00, $0f, $f7, $f0, $00, $ff, $ff, $00, $00, $00
    db $00, $ff, $ff, $00, $7f, $00, $ff, $00, $00, $ff, $00, $ff, $00, $ff, $ff, $ff
    db $00, $ff, $ff, $00, $ff, $00, $ff, $00, $00, $ff, $00, $ff, $00, $ff, $ff, $ff
    db $80, $80, $00, $80, $40, $00, $27, $c7, $b0, $37, $06, $f8, $fc, $00, $00, $00
    db $2f, $d0, $f8, $00, $3c, $03, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $ff, $ff
    db $ff, $00, $00, $00, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $ff, $ff
    db $ff, $00, $c0, $3f, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $ff, $ff
    db $c8, $37, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $ff, $ff
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $03, $fd, $fe, $fe
    db $04, $f8, $03, $fc, $01, $fe, $01, $fe, $01, $fe, $03, $fd, $fe, $fe, $00, $00
    db $a0, $80, $a0, $80, $e0, $a0, $e0, $20, $c0, $40, $80, $80, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $30, $10, $38, $08, $1c, $00, $0c
    db $00, $01, $01, $03, $03, $07, $02, $07, $00, $06, $00, $00, $00, $00, $00, $00
    db $00, $80, $80, $c0, $80, $c0, $00, $80, $00, $18, $18, $3c, $20, $78, $00, $60
    db $00, $00, $07, $00, $1f, $00, $3f, $00, $3f, $00, $3f, $00, $3f, $00, $1f, $00
    db $00, $1f, $0f, $7f, $3f, $ff, $7f, $ff, $7f, $bf, $3f, $47, $0f, $70, $40, $bf
    db $00, $f8, $f0, $fe, $fc, $ff, $fe, $ff, $fe, $fd, $fc, $e2, $f0, $0e, $02, $fd
    db $00, $00, $00, $07, $02, $19, $04, $33, $0c, $21, $0e, $21, $0e, $21, $0e, $11
    db $3b, $c4, $57, $a8, $df, $60, $e6, $79, $e0, $7f, $e0, $70, $ef, $70, $ef, $70
    db $dc, $23, $6a, $95, $fb, $06, $67, $9e, $07, $fe, $07, $0e, $f7, $0e, $f7, $0e
    db $06, $01, $00, $01, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00
    db $ef, $70, $ef, $70, $77, $08, $60, $18, $70, $88, $00, $78, $00, $00, $00, $00
    db $00, $00, $00, $07, $02, $19, $05, $32, $0d, $22, $0d, $22, $0e, $21, $07, $10
    db $20, $df, $13, $ed, $cf, $77, $ff, $df, $ff, $fc, $fc, $73, $f3, $0c, $0f, $f0
    db $1c, $e3, $ea, $17, $13, $0e, $97, $6e, $07, $fe, $07, $0e, $f7, $0e, $f7, $0e
    db $07, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $ff, $00, $ff, $00, $1f, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $ef, $70, $ef, $70, $6f, $10, $60, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $06, $01, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $07, $00, $01, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $ff, $00, $ff, $00, $1f, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $07, $06, $ff, $7e, $ff, $7e, $ff, $78, $fe, $00, $f8
    db $00, $00, $00, $00, $00, $1c, $1c, $ff, $7e, $ff, $7e, $ff, $62, $ff, $00, $e3
    ds $0090, $00
    db $a1, $5e, $3f, $ef, $30, $f8, $10, $18, $07, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $02, $02, $ff, $ff, $10, $10, $10, $10, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $21, $21, $fe, $ff, $00, $00, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $8e, $89, $00, $ff, $ff, $ff, $3f, $3f, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $89, $89, $36, $f9, $89, $89, $09, $09, $f8, $f7, $00, $00, $ff, $ff, $ff, $ff
    db $77, $4a, $80, $7f, $77, $4a, $77, $4a, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $bf, $76, $3f, $c0, $bf, $6e, $bf, $76, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $81, $81, $80, $7f, $97, $94, $97, $94, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $7e, $fe, $7c, $83, $7e, $fe, $7e, $fd, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $00, $00, $00, $ff, $00, $00, $00, $f0, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $00, $00, $00, $ff, $00, $00, $00, $0f, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $bc, $fc, $3c, $c3, $bc, $fc, $3c, $fc, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $0a, $07, $02, $fd, $5a, $57, $5a, $57, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $e6, $e7, $00, $ff, $60, $e0, $b0, $f0, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $e0, $e0, $ff, $3f, $3f, $3f, $3f, $3f, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $ff, $fd, $ff, $fd, $ff, $fd, $ff, $fd, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $f0, $10, $f0, $10, $f0, $10, $f0, $10, $0f, $f1, $00, $00, $ff, $ff, $ff, $ff
    db $b2, $3c, $b1, $3f, $00, $7f, $7c, $0c, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $27, $47, $87, $e7, $00, $ff, $09, $09, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $fc, $f8, $fe, $fc, $00, $ff, $fe, $d1, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $1f, $00, $00, $00, $80, $80, $80, $80, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $00, $00, $00, $00, $07, $07, $04, $04, $f8, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $00, $00, $00, $cc, $33, $ff, $00, $00, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $00, $00, $00, $cc, $33, $ff, $01, $01, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $00, $00, $03, $03, $01, $81, $00, $80, $3f, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $24, $35, $ff, $c8, $00, $00, $ff, $c0, $80, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $0c, $58, $ff, $01, $03, $01, $ff, $01, $03, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $80, $7f, $00, $00, $00, $00, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $00, $ff, $04, $04, $04, $04, $04, $04, $f8, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $00, $ff, $e0, $c0, $e0, $c0, $e0, $c0, $0f, $f3, $00, $00, $ff, $ff, $ff, $ff
    db $01, $ff, $00, $00, $00, $00, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $23, $22, $23, $22, $23, $22, $23, $22, $c0, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $c9, $44, $c7, $48, $ff, $7f, $02, $02, $00, $ff, $07, $46, $c7, $e6, $e3, $f8
    db $08, $00, $08, $00, $f8, $e4, $78, $64, $01, $ff, $eb, $35, $db, $65, $b3, $cd
    db $ff, $ff, $2f, $2f, $3f, $3f, $17, $17, $17, $17, $08, $17, $08, $17, $00, $00
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $00, $ff, $00, $ff, $00, $00
    db $c0, $c0, $df, $df, $df, $df, $df, $df, $df, $df, $20, $ff, $20, $ff, $00, $00
    db $00, $00, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $00, $ff, $00, $ff, $00, $00
    db $0b, $07, $fb, $f7, $fb, $f7, $fb, $f7, $fb, $f7, $08, $f7, $08, $f7, $00, $00
    db $f8, $ff, $ff, $fc, $ff, $ff, $ff, $ff, $ff, $fe, $00, $ff, $00, $ff, $00, $00
    db $f6, $0a, $f6, $4a, $e6, $da, $ec, $94, $cc, $34, $08, $f8, $18, $e8, $00, $00
    ds $0400, $00
    db $89, $89, $22, $22, $5d, $5d, $a3, $a3, $d7, $d7, $7e, $7e, $cd, $cd, $16, $16
    db $19, $19, $ae, $ae, $75, $75, $4a, $4a, $f3, $f3, $b5, $b5, $1e, $1e, $bb, $bb
    db $93, $93, $0f, $0f, $d1, $d1, $aa, $aa, $53, $53, $ec, $ec, $51, $51, $fe, $fe
    db $9c, $9c, $46, $46, $f1, $f1, $c7, $c7, $93, $93, $4d, $4d, $ba, $ba, $e5, $e5
    db $1f, $1f, $40, $40, $ea, $ea, $37, $37, $80, $80, $f9, $f9, $47, $47, $bc, $bc
    db $f0, $f0, $cf, $cf, $1a, $1a, $f5, $f5, $6a, $6a, $ff, $ff, $ca, $ca, $27, $27
    ds $0110, $00
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
    db $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $01
    db $00, $00, $00, $00, $08, $08, $0c, $0c, $0e, $0e, $09, $0b, $09, $0a, $0d, $0e
    db $00, $01, $00, $01, $00, $01, $00, $01, $00, $01, $00, $01, $00, $01, $00, $01
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $01, $01
    db $0c, $03, $0f, $04, $0b, $0b, $02, $02, $04, $07, $00, $00, $00, $c0, $40, $c0
    db $00, $01, $80, $81, $80, $01, $c0, $81, $00, $ff, $20, $30, $28, $38, $20, $38
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $00, $00, $00, $00, $00, $00
    db $03, $03, $01, $03, $02, $02, $02, $02, $02, $02, $1f, $1f, $11, $11, $11, $11
    db $60, $20, $80, $60, $60, $40, $60, $40, $60, $40, $ff, $3f, $e3, $23, $e3, $23
    db $24, $3c, $20, $34, $22, $36, $20, $32, $21, $33, $e0, $31, $e0, $31, $e0, $30
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $80, $40, $c0
    db $00, $00, $00, $03, $00, $01, $00, $01, $00, $01, $00, $01, $00, $01, $00, $03
    db $00, $00, $00, $80, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $f0
    db $00, $1f, $03, $03, $03, $03, $02, $02, $02, $02, $02, $02, $02, $02, $02, $02
    db $00, $ff, $eb, $15, $eb, $14, $ff, $ea, $ff, $ea, $ff, $eb, $ff, $eb, $ff, $eb
    db $c0, $3f, $21, $31, $22, $33, $24, $36, $a8, $bc, $a0, $38, $a0, $30, $e0, $70
    db $00, $c0, $00, $80, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $01, $01, $01, $01, $01, $01, $01, $01, $01, $01, $01, $01, $01, $01, $00, $01
    db $40, $c0, $00, $80, $00, $80, $00, $80, $00, $80, $00, $80, $00, $80, $00, $80
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $10, $10, $18, $10, $18
    db $02, $02, $02, $02, $02, $02, $02, $02, $1f, $1f, $12, $12, $12, $12, $00, $1f
    db $80, $ff, $ed, $92, $ff, $e9, $ff, $db, $fb, $c7, $3b, $3d, $3b, $3c, $00, $ff
    db $00, $f0, $40, $80, $60, $a0, $e1, $41, $e2, $03, $f0, $9f, $f0, $9f, $00, $f0
    db $00, $00, $30, $30, $fc, $80, $7f, $84, $ff, $0b, $7f, $96, $7f, $96, $2a, $55
    db $00, $00, $00, $00, $00, $00, $00, $00, $c0, $00, $a0, $40, $20, $c0, $a0, $40
    db $00, $00, $06, $06, $1f, $10, $3f, $23, $7f, $47, $7f, $0e, $ff, $8e, $f5, $0a
    db $00, $01, $00, $01, $81, $03, $e1, $83, $f9, $47, $f1, $8d, $e1, $9d, $55, $a9
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $3f, $3e
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $02, $00, $06, $cc, $af
    db $10, $1c, $14, $18, $06, $10, $06, $04, $00, $0e, $74, $76, $44, $46, $01, $7e
    db $03, $03, $03, $03, $02, $02, $02, $02, $02, $02, $02, $02, $52, $52, $ad, $fb
    db $ff, $80, $ff, $81, $7f, $7f, $7f, $7f, $7f, $7f, $7f, $7f, $7f, $7f, $43, $7f
    db $70, $df, $f0, $5f, $f6, $51, $fb, $5a, $ff, $50, $f0, $40, $ff, $50, $ff, $5c
    db $00, $ff, $69, $e9, $e9, $e9, $e9, $69, $e9, $69, $69, $29, $e9, $69, $e9, $69
    db $00, $e0, $e0, $20, $e0, $20, $e0, $20, $e0, $20, $e0, $20, $e0, $20, $e0, $20
    db $00, $ff, $d1, $d3, $01, $ff, $c3, $d3, $c3, $d3, $c3, $d3, $c3, $d3, $c0, $d0
    db $01, $fd, $f5, $ad, $ed, $b5, $fd, $a5, $fd, $a5, $fc, $a7, $fc, $a4, $fc, $a4
    db $00, $80, $00, $80, $00, $80, $00, $80, $00, $80, $00, $80, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $01, $01, $01, $01, $09, $01, $1d, $1d, $13, $17
    db $88, $70, $50, $70, $f8, $f8, $0c, $04, $7c, $74, $7c, $74, $7e, $7a, $de, $ea
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $02, $02, $06, $02, $06, $02, $0e
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $01, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $df, $20, $a4, $51, $f4, $8a
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $80, $7e, $82, $91, $42
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $f0, $30
    db $00, $00, $01, $01, $01, $01, $01, $01, $00, $01, $00, $01, $01, $01, $01, $01
    db $23, $22, $e0, $ff, $20, $20, $20, $20, $49, $ff, $49, $ff, $20, $20, $20, $20
    db $ca, $a9, $00, $ff, $00, $00, $00, $00, $24, $ff, $24, $ff, $00, $00, $00, $00
    db $4c, $4b, $0e, $ca, $2e, $6a, $00, $7f, $bf, $c0, $bf, $df, $02, $62, $32, $52
    db $01, $fe, $5e, $59, $5e, $59, $00, $ff, $fd, $02, $fc, $ff, $3d, $02, $3d, $02
    db $5b, $7f, $c3, $ff, $ff, $7f, $ff, $7f, $81, $01, $00, $ff, $ff, $00, $ff, $ff
    db $f0, $5f, $fb, $5a, $f3, $5a, $10, $1f, $18, $18, $00, $ff, $ee, $1f, $fe, $ef
    db $69, $e9, $e9, $69, $fe, $7d, $42, $c3, $43, $42, $00, $ff, $ff, $14, $ff, $14
    db $e0, $20, $e0, $20, $00, $a0, $00, $f0, $f8, $e0, $18, $e0, $20, $e0, $20, $e0
    db $00, $00, $00, $00, $03, $03, $03, $03, $00, $12, $2d, $2d, $00, $7f, $67, $64
    db $00, $00, $00, $00, $33, $7b, $b7, $b3, $00, $fd, $de, $52, $58, $97, $7e, $96
    db $c7, $d7, $c7, $d7, $c0, $d7, $c1, $d2, $c1, $f2, $d1, $d2, $01, $fa, $71, $4a
    db $7c, $a4, $1c, $94, $6c, $f4, $ec, $f4, $0c, $f4, $bc, $c4, $bc, $c4, $bc, $c4
    db $12, $16, $10, $1f, $03, $00, $05, $05, $05, $05, $05, $05, $05, $05, $08, $0f
    db $1e, $2a, $1f, $ed, $ff, $0d, $ff, $fd, $ff, $fe, $ff, $fe, $ff, $fe, $00, $ff
    db $0a, $06, $10, $1a, $10, $18, $02, $1e, $94, $93, $97, $96, $97, $97, $00, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $80, $80, $00, $80, $00, $80, $18, $e8
    db $0b, $0c, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $92, $25, $e2, $98, $0f, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $38, $08, $48, $90, $88, $20, $8c, $44, $bc, $c4, $c4, $a0, $44, $10, $44, $08
    db $08, $08, $08, $08, $08, $08, $08, $08, $7f, $7e, $47, $46, $7c, $7f, $43, $43
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $60, $f0, $20, $b0, $ff, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $01, $fe, $fc, $ff, $01
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $f0, $f0, $0f, $07, $ff, $f0
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $0f, $0f, $e0, $f0, $c0, $20
    db $00, $00, $00, $00, $00, $00, $3f, $1f, $20, $1f, $ff, $c0, $3f, $3f, $3f, $3f
    db $00, $00, $00, $00, $3e, $3e, $c1, $80, $3f, $fe, $fe, $00, $f8, $fc, $f8, $fc
    db $00, $00, $00, $00, $01, $00, $fc, $fe, $f8, $04, $10, $18, $00, $10, $00, $3f
    db $00, $00, $00, $00, $ff, $ff, $07, $07, $07, $07, $07, $07, $07, $07, $00, $ff
    db $00, $00, $07, $07, $fb, $f3, $f8, $f7, $fb, $f3, $fb, $f3, $fb, $f3, $00, $ff
    db $00, $01, $80, $80, $40, $80, $40, $80, $41, $81, $41, $81, $41, $81, $82, $02
    db $00, $ff, $ff, $df, $ff, $c0, $a0, $a0, $20, $20, $20, $20, $20, $20, $21, $21
    db $00, $ff, $ff, $ff, $ff, $00, $00, $00, $00, $00, $28, $28, $d6, $7c, $8e, $89
    db $00, $ff, $7f, $80, $00, $f0, $70, $ff, $7f, $ff, $7c, $87, $04, $07, $09, $09
    db $01, $fe, $ff, $00, $20, $20, $00, $c0, $c0, $c0, $90, $ef, $91, $61, $75, $4d
    db $ff, $7f, $ff, $7f, $be, $ff, $be, $c1, $bf, $ff, $00, $ff, $a0, $40, $a0, $7f
    db $f0, $e0, $e0, $ff, $74, $e5, $74, $85, $f0, $e1, $00, $ff, $41, $41, $01, $81
    db $00, $00, $00, $ff, $81, $01, $80, $fe, $fe, $fe, $00, $ff, $7e, $83, $7e, $ff
    db $58, $58, $fe, $3e, $04, $06, $04, $06, $04, $06, $00, $fe, $66, $ff, $66, $ff
    db $00, $7f, $7f, $40, $19, $7f, $99, $ee, $80, $80, $00, $ff, $66, $99, $66, $ff
    db $00, $ff, $ff, $38, $9f, $fe, $9f, $ef, $0f, $17, $0f, $f7, $7c, $84, $3c, $fc
    db $01, $fe, $f9, $06, $f9, $06, $f8, $f7, $fa, $f7, $fa, $f7, $14, $0c, $10, $1f
    db $bc, $c4, $bc, $c4, $bc, $c4, $07, $03, $84, $07, $e2, $61, $00, $1f, $06, $e7
    db $00, $00, $00, $00, $00, $00, $f0, $f0, $00, $f0, $00, $00, $ff, $ff, $c0, $c0
    db $08, $08, $0b, $0b, $15, $1f, $35, $35, $ff, $ff, $c1, $c1, $00, $ff, $ff, $ff
    db $00, $00, $ff, $ff, $00, $ff, $ff, $80, $90, $70, $98, $78, $00, $ff, $ff, $ff
    db $01, $01, $ff, $ff, $00, $ff, $bf, $40, $7f, $3e, $7f, $3e, $00, $ff, $ff, $fd
    db $f8, $88, $f8, $88, $18, $e8, $80, $40, $80, $40, $80, $40, $30, $d0, $f0, $10
    db $00, $00, $00, $00, $00, $00, $00, $00, $f8, $f8, $97, $9f, $80, $f8, $b4, $bc
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $ff, $07, $07, $07, $07
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $e0, $c0, $ef, $ff, $f7, $f8
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $1f, $1f, $e0, $c0, $ff, $1f
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $7e, $ff, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $07, $07
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $01, $00, $fd, $f8
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $00, $00, $99
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $07, $07, $98
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $ff, $e0, $e0, $19
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $04, $02, $3f, $3e
    db $7a, $46, $22, $04, $22, $08, $2e, $30, $12, $08, $13, $05, $1d, $13, $c9, $42
    db $08, $00, $08, $00, $08, $00, $08, $00, $08, $00, $08, $00, $08, $00, $08, $00
    db $5c, $5c, $7f, $63, $7d, $62, $3c, $23, $3c, $33, $1f, $1c, $07, $06, $01, $01
    db $01, $01, $fe, $fe, $7f, $81, $ff, $00, $7f, $80, $ff, $00, $ff, $00, $ff, $0a
    db $ff, $ff, $00, $00, $ff, $ff, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $5c
    db $c0, $c0, $3f, $3f, $c0, $c0, $ff, $3f, $ff, $00, $ff, $00, $ff, $00, $ff, $bb
    db $00, $00, $fc, $fc, $03, $03, $fc, $fc, $ff, $03, $ff, $00, $ff, $00, $ff, $ff
    db $00, $00, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $ff, $00, $ff, $d4, $ff, $6b
    db $00, $00, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $ff, $00, $ff, $5a, $ff, $ff
    db $00, $00, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $ff, $02, $ff, $ff, $ff, $ff
    db $00, $00, $00, $00, $fe, $fe, $01, $01, $fe, $fe, $ff, $ff, $ff, $ff, $ff, $ff
    db $00, $00, $00, $00, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff, $ff, $ff
    db $00, $00, $00, $00, $00, $00, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $01, $00, $00, $00, $00, $00, $00, $00, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $f0, $00, $00, $00, $01, $00, $01, $01, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $80, $c0, $00, $80, $00, $ff, $d0, $d0, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $3f, $3f, $3f, $3f, $00, $ff, $7f, $58, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $f8, $fc, $f0, $f8, $01, $f1, $c1, $01, $ff, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $3f, $05, $3a, $3a, $eb, $db, $6a, $5a, $fc, $ff, $00, $00, $ff, $ff, $ff, $ff
    db $ff, $ff, $00, $cc, $33, $ff, $00, $00, $00, $ff, $00, $00, $ff, $ff, $ff, $ff
