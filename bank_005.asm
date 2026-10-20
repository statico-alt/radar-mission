; Disassembly of "rm.gb"
; This file was created with:
; mgbdis v3.0 - Game Boy ROM disassembler by Matt Currie and contributors.
; https://github.com/mattcurrie/mgbdis

; ROM bank 5. Game A support data: the tilemaps, sprite tables, AI search
; patterns and the commander graphics. The code is now C: bank5_*.c in this
; bank, and bank13_*.c (link play and most of the CPU AI) in bank 13.
; Other banks call the jp vectors at $4000 + 3 * n.

; The C code reads these tables.
EXPORT GridSizeDigitTiles, GridSizeTileQueueTemplate, AiSearchPatternPtrs
EXPORT ShipCellIdLimits, ResultsScreenTilemap, CommanderTilesetPtrs
EXPORT ShipLengthAndFirstCell, ShipCellPieceType, ShipPieceSpriteTiles
EXPORT GridRowTileRuns, GridLevel2RowTiles, SeaMap_Level1and2, SeaMap_Level3
EXPORT NextLevelRowTiles, NextLevelScreenMap, NextLevelShipInit, NextLevelShipTiming
EXPORT TitleSprites, TitleScreenMap, TitleScreenTilemap
EXPORT TextRevealScreenMap, TextRevealSprites, TextRevealTriggers
EXPORT ShipHitPointsByLevel, GridParamsBySize, GridParamsByLevel
EXPORT OptionsCursorPositions, OBP1FlashCycle
EXPORT LevelTitleTilemap, LevelTitleTilemap_Level1, LevelTitleTilemap_Level2
EXPORT LevelTitleTilemap_Level3, LevelTitleSprites
EXPORT ResultsSprites_P1, ResultsSprites_P2, ResultsCommanderFaceTiles
EXPORT NeighborOffsets, GridHeaderTemplate
EXPORT ResultsMedalSprites_Small, ResultsMedalSprites_Large, HitRatioRankThresholds
EXPORT CommanderTiles0, CommanderTiles1, CommanderTiles2
EXPORT NextLevelBgTiles, LevelTitleExtraTiles, NextLevelSpriteTiles

; The C code calls these bank 0 routines directly.
EXPORT DrawTilemapCmds, QueueTilemapRowCopy, LcdOffMaskIrqs, LcdOnEnableIrqs
EXPORT GameA_DrawFleetPanel, GameA_LoadCursorTiles

SECTION "ROM Bank $005", ROMX[$4000], BANK[$5]


; Entry vector. Other banks switch to bank 5 and call $4000 + 3 * n.
Bank5_EntryVector:
    jp Link_SerialHandler


Vec5_GameA_BlinkShipSprites:
    jp GameA_BlinkShipSprites


Vec5_GameA_OptionsMenuUpdate:
    jp GameA_OptionsMenuUpdate


Vec5_GameA_CpuTurn_Type0:
    jp GameA_CpuTurn_Type0


Vec5_GameA_CpuTurn_Type1:
    jp GameA_CpuTurn_Type1


Vec5_GameA_ResultsScreenInit:
    jp GameA_ResultsScreenInit


Vec5_GameA_ShowShipSprite:
    jp GameA_ShowShipSprite


Vec5_GameA_GridCellToScreen:
    jp GameA_GridCellToScreen


Vec5_GameA_CpuTurn_Type2:
    jp GameA_CpuTurn_Type2


Vec5_GameA_DrawGridRow:
    jp GameA_DrawGridRow


Vec5_GameA_SetSeaMapPtr:
    jp GameA_SetSeaMapPtr


Vec5_GameA_NextLevelScreen:
    jp GameA_NextLevelScreen


Vec5_TitleScreenInit:
    jp TitleScreenInit


Vec5_TextRevealScreenInit:
    jp TextRevealScreenInit


Vec5_TextRevealScreenUpdate:
    jp TextRevealScreenUpdate


Vec5_GameA_SetupBattle:
    jp GameA_SetupBattle


Vec5_GameA_AiPickNextState:
    jp GameA_AiPickNextState


Vec5_GameA_DrawShipCellMarkers:
    jp GameA_DrawShipCellMarkers


Vec5_GameA_CyclePaletteOBP1:
    jp GameA_CyclePaletteOBP1


Vec5_GameA_AiMarkSunkLine:
    jp GameA_AiMarkSunkLine


Vec5_GameA_LevelTitleScreenInit:
    jp GameA_LevelTitleScreenInit


Vec5_GameA_RandomAmbientSfx:
    jp GameA_RandomAmbientSfx

; ===========================================================================
; Glue. The asm callers pass values in registers. The C functions use the
; SDCC rules (__sdcccall(1)):
;   - 1st argument: A (byte) or DE (pointer).
;   - 2nd argument: E (byte after a byte) or A (byte after a pointer).
;   - More arguments go on the stack. The C function removes them.
;   - Result: A (byte) or BC (pointer).
; C code can change all registers, so the glue keeps every register that
; is not a result. FAR_CALL_C calls the C code in bank 13.
; ===========================================================================

; CALL_C and FAR_CALL_C are in glue.inc.

Link_SerialHandler:
    FAR_CALL_C linkSerialHandler

GameA_BlinkShipSprites:
    CALL_C blinkShipSprites

GameA_OptionsMenuUpdate:
    CALL_C optionsMenuUpdate

GameA_CpuTurn_Type0:
    FAR_CALL_C cpuTurnType0

GameA_CpuTurn_Type1:
    FAR_CALL_C cpuTurnType1

GameA_CpuTurn_Type2:
    FAR_CALL_C cpuTurnType2

GameA_AiPickNextState:
    FAR_CALL_C aiPickNextState

GameA_ResultsScreenInit:
    CALL_C resultsScreenInit

GameA_ShowShipSprite:
    CALL_C showShipSprite

GameA_SetSeaMapPtr:
    CALL_C setSeaMapPtr

GameA_NextLevelScreen:
    CALL_C nextLevelScreen

TitleScreenInit:
    CALL_C titleScreenInit

TextRevealScreenInit:
    CALL_C textRevealScreenInit

TextRevealScreenUpdate:
    CALL_C textRevealScreenUpdate

GameA_SetupBattle:
    CALL_C setupBattle

GameA_DrawShipCellMarkers:
    CALL_C drawShipCellMarkers

GameA_CyclePaletteOBP1:
    CALL_C cyclePaletteOBP1

GameA_LevelTitleScreenInit:
    CALL_C levelTitleScreenInit

GameA_RandomAmbientSfx:
    CALL_C randomAmbientSfx

; In: D = row, E = column. Out: D = Y, E = X. Changes A, BC and HL, like the asm.
GameA_GridCellToScreen:
    ld a, d
    call gridCellToScreen
    ld d, b
    ld e, c
    ret

; Out: A = g_gameATurnTimer (the row counter), carry while rows are left.
; The callers read only A and the flags.
GameA_DrawGridRow:
    call drawGridRow
    ldh a, [g_gameATurnTimer]
    cp $12 ; 18 = all screen rows drawn
    ret

; In: D/E = cell of a sunk ship. Out: A = count of AI list entries marked done.
GameA_AiMarkSunkLine:
    push bc
    push de
    push hl
    ld hl, g_aiSunkLineCell
    ld [hl], d
    inc hl
    ld [hl], e
    ld e, BANK(aiMarkSunkLine)
    ld hl, aiMarkSunkLine
    call ___sdcc_bcall_ehl
    pop hl
    pop de
    pop bc
    ldh a, [g_gameATurnTimer]
    ret



; ---------------------------------------------------------------------------
; Data tables
; ---------------------------------------------------------------------------

GridSizeDigitTiles:
    db $e0, $e2, $e4

GridSizeTileQueueTemplate:
    db $98, $21, $03, $00, $49, $00, $98, $30, $03, $00, $49, $00, $00

; Index: commander type ($c0b5) * 3 + grid size. Each pattern: count, then
; runs of 3 bytes: start cell, length, step (signed row/column nibbles).
AiSearchPatternPtrs:
    dw AiSearchPattern_T0_8
    dw AiSearchPattern_T0_10
    dw AiSearchPattern_T0_12
    dw AiSearchPattern_T1_8
    dw AiSearchPattern_T1_10
    dw AiSearchPattern_T1_12

AiSearchPattern_T0_8:
    db $14, $02, $03, $19, $31, $02, $11, $51, $02, $19, $71, $01, $11, $06, $03, $19
    db $35, $02, $11, $55, $02, $19, $75, $01, $11, $33, $01, $11, $73, $01, $11, $00
    db $03, $11, $13, $02, $91, $15, $02, $11, $17, $01, $11, $40, $03, $11, $53, $02
    db $91, $55, $02, $11, $57, $01, $11, $37, $01, $11, $77, $01, $11

AiSearchPattern_T0_10:
    db $09, $40, $06, $11, $04, $06, $11, $60, $04, $11, $06, $04, $11, $80, $02, $11
    db $08, $02, $11, $00, $0a, $11, $02, $08, $11, $20, $08, $11

AiSearchPattern_T0_12:
    db $1d, $00, $03, $11, $02, $02, $2a, $04, $02, $19, $40, $02, $91, $0a, $04, $19
    db $17, $02, $91, $39, $03, $11, $2a, $02, $91, $48, $02, $19, $3b, $01, $11, $bb
    db $05, $99, $b9, $02, $a2, $a8, $02, $19, $8a, $02, $91, $97, $02, $a2, $a0, $04
    db $91, $b1, $01, $11, $a2, $02, $11, $80, $01, $11, $93, $03, $11, $71, $01, $22
    db $84, $02, $11, $75, $01, $11, $53, $03, $91, $55, $01, $11, $59, $03, $11, $06
    db $07, $19, $62, $05, $02, $26, $05, $20

AiSearchPattern_T1_8:
    db $0f, $02, $03, $19, $60, $02, $11, $75, $03, $91, $17, $02, $99, $00, $01, $11
    db $77, $01, $11, $31, $02, $19, $51, $03, $11, $46, $02, $91, $26, $03, $99, $44
    db $02, $99, $42, $02, $a2, $35, $02, $2a, $13, $02, $19, $64, $02, $91

AiSearchPattern_T1_10:
    db $1d, $27, $03, $91, $72, $03, $19, $23, $03, $99, $67, $03, $11, $05, $02, $11
    db $38, $02, $11, $94, $02, $99, $61, $02, $99, $85, $02, $91, $58, $02, $91, $41
    db $02, $91, $14, $01, $11, $69, $02, $19, $87, $02, $19, $30, $02, $91, $03, $02
    db $19, $29, $02, $99, $07, $01, $11, $70, $02, $22, $92, $01, $11, $10, $01, $11
    db $98, $01, $11, $25, $01, $11, $47, $01, $11, $74, $01, $11, $52, $01, $11, $36
    db $04, $19, $65, $03, $99, $34, $03, $11

AiSearchPattern_T1_12:
    db $24, $00, $01, $11, $02, $03, $19, $04, $05, $19, $24, $03, $19, $44, $01, $11
    db $a0, $02, $11, $80, $04, $11, $82, $02, $11, $bb, $01, $11, $b9, $03, $91, $b7
    db $05, $91, $97, $03, $91, $77, $01, $11, $1b, $02, $99, $3b, $04, $99, $39, $02
    db $99, $51, $01, $11, $60, $02, $11, $62, $02, $91, $64, $02, $11, $6a, $01, $11
    db $5b, $02, $99, $59, $02, $19, $57, $02, $99, $a4, $01, $11, $b5, $02, $91, $95
    db $02, $99, $75, $02, $91, $17, $01, $11, $06, $02, $19, $26, $02, $11, $46, $02
    db $19, $73, $01, $11, $86, $01, $11, $48, $01, $11, $35, $01, $11

; Upper limit (exclusive) of each ship's cell ids, used by GameA_CheckLastHitOnShip.
ShipCellIdLimits:
    db $06, $0a, $0d, $10, $12, $13, $14

; RST $28 tilemap script. Each record: VRAM addr (big endian), mode<<6|count, data. $00 ends.
ResultsScreenTilemap:
    db $9c, $01, $52, $d1
    db $9c, $a6, $58, $d6
    db $9c, $c0, $54, $d1
    db $9c, $00, $01, $d0
    db $9c, $13, $01, $d2
    db $9c, $20, $c4, $d3
    db $9c, $33, $c4, $d4
    db $9c, $a0, $06, $d5, $04, $05, $06, $07, $08
    db $9c, $ae, $06, $0e, $0f, $10, $11, $12, $d7
    db $9c, $c1, $05, $09, $0a, $0b, $0c, $0d
    db $9c, $ce, $05, $13, $14, $15, $16, $17
    db $9c, $25, $0a, $18, $19, $1a, $00, $00, $00, $18, $19, $1a, $00
    db $9c, $45, $0a, $00, $00, $d8, $a4, $00, $00, $00, $00, $d8, $a4
    db $9c, $65, $0a, $1b, $1c, $1d, $00, $00, $00, $1b, $1c, $1d, $00
    db $9c, $85, $0a, $d8, $a5, $d8, $d8, $00, $00, $d8, $a5, $d8, $d8
    db $9d, $63, $0e, $1e, $1f, $01, $1e, $1f, $01, $1e, $1f, $01, $1e, $1f, $01, $1e, $1f
    db $9d, $80, $54, $03
    db $9d, $83, $0e, $20, $21, $03, $20, $21, $03, $20, $21, $03, $20, $21, $03, $20, $21
    db $9d, $a0, $54, $92
    db $9d, $c0, $54, $d6
    db $9d, $e1, $52, $d1
    db $9e, $21, $52, $d6
    db $9d, $e0, $83, $d0, $d3, $d5
    db $9d, $e9, $83, $d2, $d4, $d7
    db $9d, $ea, $83, $d0, $d3, $d5
    db $9d, $f3, $83, $d2, $d4, $d7
    db $9e, $01, $12, $ed, $e7, $f3, $eb, $f5, $fb, $eb, $f7, $d4, $d3, $f9, $fa, $f7, $f4, $00, $d8, $d8, $d8
    db $00 ; end

; $400 bytes of tiles for each of the 5 commander types.
CommanderTilesetPtrs:
    dw CommanderTiles0
    dw CommanderTiles1
    dw CommanderTiles2
    dw CommanderTiles3
    dw CommanderTiles4

; Per ship: length, first cell id. Carrier 5, battleship 4, cruiser 3, sub 3, destroyer 2.
ShipLengthAndFirstCell:
    db $05, $01, $04, $06, $03, $0a, $03, $0d, $02, $10

ShipCellPieceType:
    db $00, $01, $01, $01, $02, $00, $01, $01, $02, $00, $01, $02, $00, $01, $02, $00
    db $02

ShipPieceSpriteTiles:
    db $67, $28, $67, $08, $68, $28, $68, $08, $68, $28, $68, $08, $67, $68, $67, $48
    db $67, $28, $69, $08, $67, $68, $69, $48, $69, $08, $67, $08, $69, $48, $67, $48
    db $68, $28, $68, $08, $68, $28, $68, $08, $68, $28, $68, $08, $68, $28, $68, $08
    db $69, $08, $69, $08, $69, $48, $69, $48, $69, $08, $69, $08, $69, $48, $69, $48
    db $68, $28, $68, $08, $67, $68, $67, $48, $67, $28, $67, $08, $68, $28, $68, $08
    db $69, $08, $67, $08, $69, $48, $67, $48, $67, $28, $69, $08, $67, $68, $69, $48

GridRowTileRuns:
    db $02, $9f, $02, $a1, $02, $9b, $02, $9d, $04, $93, $04, $97, $04, $93, $04, $97
    db $04, $93, $04, $97, $04, $93

GridLevel2RowTiles:
    db $a7, $00, $a9, $a7, $a7, $a7, $00, $a9, $00, $a9, $a7, $a7, $a7, $a9, $00, $a7
    db $a7, $00, $a7

SeaMap_Level1and2:
    db $13, $01, $4d, $ff, $10, $44, $4e, $ff, $10, $44, $52, $ff, $0f, $45, $56, $ff
    db $08, $01, $5b, $0e, $46, $5c, $ff, $08, $42, $62, $0e, $46, $64, $ff, $01, $50
    db $6a, $11, $83, $00, $ff, $01, $50, $7a, $11, $83, $00, $ff, $01, $43, $8a, $04
    db $83, $8d, $07, $83, $8e, $0a, $01, $8f, $0b, $85, $90, $10, $01, $91, $11, $83
    db $00, $ff

SeaMap_Level3:
    db $09, $42, $4d, $ff, $09, $42, $4f, $ff, $09, $42, $51, $ff, $09, $42, $54, $ff
    db $06, $01, $53, $07, $46, $56, $0d, $01, $53, $ff, $01, $42, $5c, $03, $4e, $5c
    db $11, $42, $68, $ff, $00, $a0, $6a, $ff, $00, $14, $6b, $6c, $6b, $6c, $6b, $6c
    db $6b, $6c, $6b, $6c, $6b, $6c, $6b, $6c, $6b, $6c, $6b, $6c, $6b, $6c, $ff, $0a
    db $01, $6d, $ff, $08, $44, $6e, $ff, $08, $44, $72, $ff, $08, $45, $76, $ff, $07
    db $46, $7b, $ff, $07, $46, $81, $ff, $07, $46, $87, $ff, $07, $46, $8d, $ff

NextLevelRowTiles:
    db $d3, $b3, $b3, $b3, $b3, $b3, $b3, $b3, $b3, $b3, $b3, $b3, $b3, $b3, $d4, $d3
    db $00, $00, $00, $d4

NextLevelScreenMap:
    db $01, $92, $d1, $00, $01, $d0, $0e, $02, $d2, $d0, $13, $01, $d2, $ff, $09, $42
    db $b1, $ff, $09, $42, $af, $ff, $09, $42, $ad, $ff, $09, $42, $ab, $ff, $01, $0d
    db $a7, $a8, $a7, $a8, $a7, $a8, $a7, $a8, $a9, $aa, $a7, $a8, $a7, $ff, $01, $8d
    db $01, $06, $43, $80, $ff, $01, $8d, $01, $05, $45, $83, $ff, $01, $8d, $01, $04
    db $07, $88, $89, $b4, $8a, $b4, $8b, $8c, $ff, $01, $8d, $01, $03, $09, $88, $89
    db $8d, $8e, $b3, $8f, $90, $8b, $8c, $ff, $01, $0d, $01, $88, $89, $91, $b3, $93
    db $94, $95, $b3, $92, $8b, $8c, $01, $ff, $01, $0d, $88, $89, $91, $96, $97, $98
    db $b3, $99, $9a, $9b, $92, $8b, $8c, $ff, $01, $0d, $89, $91, $b3, $b3, $9c, $9d
    db $9e, $9f, $a0, $b3, $b3, $92, $8b, $ff, $01, $0d, $91, $b3, $a1, $b3, $a2, $a3
    db $b3, $a4, $a5, $b3, $a6, $b3, $92, $ff, $00, $01, $d5, $01, $92, $d6, $0e, $02
    db $d7, $d5, $13, $01, $d7, $ff, $00, $01, $d0, $01, $92, $d1, $09, $02, $d2, $d0
    db $13, $01, $d2, $ff, $00, $01, $d3, $01, $88, $00, $09, $0b, $d4, $d3, $f9, $fa
    db $f7, $f4, $00, $00, $00, $00, $d4, $ff, $00, $01, $d5, $01, $92, $d6, $09, $02
    db $d7, $d5, $13, $01, $d7, $ff

NextLevelShipInit:
    db $05, $04, $14, $00, $03, $00, $00, $00, $01, $00, $00, $00, $04, $00, $00, $00
    db $02, $00, $00, $00, $05, $00, $00

NextLevelShipTiming:
    db $05, $20, $00, $28, $06, $30, $03, $28, $08, $20

TitleSprites:
    db $70, $36, $01, $ce, $88, $2e, $01, $ce

TitleScreenMap:
    db $02, $42, $01, $09, $42, $03, $10, $42, $05, $ff, $02, $43, $07, $09, $42, $0a
    db $0f, $43, $0c, $ff, $03, $43, $0f, $08, $44, $12, $0e, $43, $16, $ff, $00, $01
    db $19, $01, $88, $1a, $08, $43, $1b, $0b, $88, $1a, $13, $01, $1e, $ff, $00, $54
    db $1f, $ff, $00, $54, $33, $ff, $00, $53, $47, $13, $01, $46, $ff, $00, $01, $5a
    db $01, $86, $5b, $07, $06, $5c, $5b, $5d, $5e, $5b, $5f, $0d, $86, $5b, $13, $01
    db $60, $ff, $04, $42, $05, $07, $46, $61, $0e, $42, $01, $12, $42, $78, $ff, $03
    db $43, $0c, $07, $46, $67, $0e, $43, $07, $ff, $02, $43, $16, $08, $44, $6d, $0f
    db $43, $0f, $ff

TitleScreenTilemap:
    db $99, $87, $06, $ed, $e7, $f3, $eb, $00, $e7
    db $99, $a7, $06, $ed, $e7, $f3, $eb, $00, $e8
    db $99, $e6, $02, $d9, $f6
    db $99, $ec, $02, $da, $f6
    db $9a, $24, $0c, $71, $d9, $e1, $e1, $d8, $00, $72, $73, $74, $75, $76, $77
    db $00 ; end

TextRevealScreenMap:
    db $0b, $42, $4e, $ff, $0b, $42, $50, $ff, $0a, $44, $52, $ff, $0a, $46, $56, $ff
    db $0a, $44, $5c, $ff, $0a, $45, $60, $ff, $09, $47, $65, $ff, $09, $49, $6c, $ff
    db $04, $01, $75, $08, $4a, $76, $ff, $03, $43, $80, $08, $4b, $83, $ff, $00, $83
    db $a6, $03, $50, $8e, $13, $01, $a6, $ff, $00, $83, $a7, $03, $42, $9e, $08, $42
    db $a0, $0c, $42, $a2, $0f, $42, $a4, $13, $01, $a7, $ff, $00, $94, $a8, $ff

TextRevealSprites:
    db $20, $c0, $01, $23, $28, $c0, $45, $01, $30, $c0, $45, $06

TextRevealTriggers:
    db $18, $d0, $d9, $20, $d1, $da, $28, $d2, $db, $30, $d3, $dc, $38, $d4, $dd, $40
    db $d5, $de, $48, $d6, $df, $50, $d7, $e0, $58, $d8, $e1

; 5 ship hit-point counts per level.
ShipHitPointsByLevel:
    db $05, $04, $03, $03, $02, $05, $03, $03, $03, $00, $07, $02, $02, $02, $00

GridParamsBySize:
    db $01, $00, $01, $00, $02, $02, $02, $02

GridParamsByLevel:
    db $03, $03, $03, $03, $04, $04, $04, $04, $04, $04, $04, $04

OptionsCursorPositions:
    db $18, $2c, $10, $28, $2c, $10, $38, $2c, $10, $50, $2c, $10, $74, $30, $10, $90
    db $34, $30, $18, $7c, $30, $28, $7c, $30, $38, $7c, $30, $50, $7c, $30, $74, $7a
    db $30, $90, $6c, $10

OBP1FlashCycle:
    db $08, $04, $00, $04, $08, $0c

LevelTitleTilemap:
    db $98, $01, $52, $d1
    db $99, $61, $52, $d6
    db $99, $81, $52, $d1
    db $9a, $21, $52, $d6
    db $98, $20, $d0, $d3
    db $98, $2c, $ca, $d4
    db $98, $2d, $ca, $d3
    db $98, $33, $d0, $d4
    db $98, $00, $01, $d0
    db $98, $0c, $02, $d2, $d0
    db $98, $13, $01, $d2
    db $99, $60, $82, $d5, $d0
    db $99, $6c, $02, $d7, $d5
    db $99, $73, $82, $d7, $d2
    db $9a, $20, $01, $d5
    db $9a, $33, $01, $d7
    db $98, $2e, $45, $00
    db $98, $4e, $05, $f8, $f9, $e7, $ed, $eb
    db $98, $6e, $45, $00
    db $98, $8e, $45, $00
    db $98, $ae, $45, $00
    db $99, $af, $04, $80, $81, $82, $83
    db $99, $c2, $09, $f9, $ee, $eb, $00, $eb, $f4, $eb, $f3, $fd
    db $99, $cf, $04, $84, $85, $86, $87
    db $99, $ef, $04, $88, $89, $8a, $8b
    db $9a, $04, $06, $ec, $f5, $f7, $e9, $eb, $f8
    db $9a, $0f, $04, $8c, $8d, $8e, $8f
    db $00 ; end

LevelTitleTilemap_Level1:
    db $98, $90, $01, $d9
    db $98, $ce, $45, $92
    db $98, $ee, $45, $92
    db $99, $0e, $45, $92
    db $99, $2e, $05, $9f, $a0, $9f, $a0, $9f
    db $99, $4e, $05, $a1, $a2, $a1, $a2, $a1
    db $98, $a2, $85, $bb, $bc, $bd, $be, $bf
    db $98, $c4, $84, $c0, $c1, $c2, $c3
    db $98, $e6, $83, $c4, $c5, $c6
    db $98, $e8, $83, $c7, $c8, $c9
    db $99, $0a, $82, $ca, $cb
    db $00 ; end

LevelTitleTilemap_Level2:
    db $98, $90, $01, $da
    db $98, $ce, $45, $00
    db $98, $ee, $05, $00, $00, $00, $a8, $00
    db $99, $0e, $05, $a7, $a7, $00, $a9, $a7
    db $99, $2e, $45, $a6
    db $99, $4e, $45, $01
    db $98, $a2, $85, $bb, $bc, $bd, $be, $bf
    db $98, $e4, $83, $c7, $c8, $c9
    db $98, $e6, $83, $c7, $c8, $c9
    db $98, $e8, $83, $c7, $c8, $c9
    db $00 ; end

LevelTitleTilemap_Level3:
    db $98, $90, $01, $db
    db $98, $ce, $45, $01
    db $98, $ee, $05, $67, $68, $69, $68, $69
    db $99, $0e, $45, $6a
    db $99, $2e, $05, $6b, $6c, $6b, $6c, $6b
    db $99, $4e, $45, $01
    db $98, $62, $87, $bb, $bc, $bd, $be, $bf, $c0, $c1
    db $99, $04, $82, $c2, $c3
    db $99, $06, $82, $c2, $c3
    db $99, $08, $82, $c2, $c3
    db $00 ; end

LevelTitleSprites:
    db $76, $00, $90, $10, $7e, $00, $91, $10

ResultsSprites_P1:
    db $18, $10, $44, $00, $20, $10, $44, $04, $28, $10, $44, $08, $30, $10, $44, $0c

ResultsSprites_P2:
    db $18, $80, $44, $40, $20, $80, $44, $44, $28, $80, $44, $48, $30, $80, $44, $4c

ResultsCommanderFaceTiles:
    db $28, $08, $28, $28, $28, $08, $28, $08, $28, $08

; Row/column offsets: self, then 8 neighbours.
NeighborOffsets:
    db $00, $00, $ff, $ff, $01, $01, $ff, $01, $01, $ff, $fe, $fe, $02, $02, $fe, $02
    db $02, $fe

GridHeaderTemplate:
    db $f9, $fa, $f7, $f4, $00, $d8, $d8

ResultsMedalSprites_Small:
    db $54, $f8, $02, $87, $86

ResultsMedalSprites_Large:
    db $50, $e0, $02, $87, $86, $58, $d0, $02, $87, $86

; Hit ratio (%) limits for medals: 60, 55, 45.
HitRatioRankThresholds:
    db $3c, $37, $2d

Unused_Bank5SpriteGfx:
    db $10, $14, $11, $40, $50, $14, $10, $28, $40, $74, $10, $45, $01, $40, $41, $03
    db $01, $42, $40, $3a, $00, $0d, $44, $ce, $41, $39, $11, $c6, $10, $4c, $01, $b5
    db $00, $92, $15, $b9, $05, $82, $04, $21, $05, $60, $14, $00, $55, $2b, $50, $1b
    db $10, $bf, $44, $d2, $04, $4a, $55, $28, $84, $a2, $14, $09, $04, $95, $01, $00
    db $00, $c6, $00, $ec, $40, $64, $05, $e3, $01, $a8, $11, $c1, $00, $21, $40, $c1
    db $10, $70, $00, $70, $41, $40, $40, $81, $40

; ---------------------------------------------------------------------------
; Graphics data
; ---------------------------------------------------------------------------

CommanderTiles0:
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $f9
    db $1f, $f0, $77, $e7, $e2, $82, $c0, $1f, $7f, $ad, $00, $80, $00, $bf, $40, $5f
    db $00, $3f, $40, $07, $38, $01, $06, $c0, $f9, $10, $04, $02, $00, $f8, $03, $c3
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $7f, $00, $ff, $00, $ff, $00, $63
    db $06, $f0, $06, $f0, $02, $f0, $05, $f0, $05, $f0, $02, $f8, $01, $fc, $01, $fc
    db $a0, $80, $bf, $80, $5f, $66, $7b, $66, $7f, $00, $9d, $00, $6f, $00, $7f, $00
    db $2f, $0f, $f2, $02, $fc, $00, $ff, $00, $bf, $00, $ff, $00, $5f, $00, $ff, $00
    db $1c, $41, $b4, $01, $ac, $01, $dc, $01, $d8, $03, $f8, $03, $c0, $07, $80, $3f
    db $01, $fc, $01, $fc, $01, $fc, $00, $fe, $00, $fe, $00, $fe, $00, $ff, $00, $ff
    db $9f, $00, $ff, $00, $f8, $00, $07, $00, $f7, $f0, $f7, $f0, $0f, $00, $1f, $c0
    db $fb, $00, $e7, $00, $1f, $00, $ff, $00, $ff, $00, $fe, $00, $fe, $00, $fc, $01
    db $c0, $1f, $c0, $1f, $80, $3f, $80, $3f, $00, $7f, $00, $ff, $00, $ff, $00, $ff
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $0f, $f0, $0f, $f7, $07, $f0, $08, $e8
    db $0f, $e0, $07, $f0, $03, $f8, $03, $f8, $9b, $60, $bb, $58, $3b, $38, $bd, $bc
    db $f8, $03, $f0, $07, $e0, $0f, $e0, $0f, $ec, $03, $df, $1c, $de, $1e, $be, $3e
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $7c, $83, $7c, $bb, $38, $03, $c4, $c5
    db $9f, $00, $ff, $00, $ff, $00, $07, $00, $f1, $f0, $f0, $f6, $00, $0f, $00, $fe
    db $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $fe, $00, $7e, $00, $fc, $01
    db $07, $f0, $03, $f8, $03, $f8, $03, $f8, $9b, $60, $bb, $58, $3b, $38, $bd, $bc
    db $f0, $07, $e0, $0f, $e0, $0f, $e0, $0f, $ec, $03, $df, $1c, $de, $1e, $be, $3e
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $f1
    db $00, $ff, $07, $ff, $1f, $f8, $38, $e0, $67, $c2, $08, $d0, $00, $c7, $30, $b0
    db $fc, $81, $ba, $38, $11, $10, $00, $fe, $ff, $2d, $00, $00, $00, $ff, $00, $fe
    db $00, $ff, $00, $7f, $80, $3f, $c0, $3f, $80, $7f, $00, $7f, $00, $7f, $80, $a3
    db $0e, $e0, $0b, $e0, $0d, $e0, $0e, $e0, $06, $f0, $07, $f0, $00, $f8, $00, $ff
    db $3d, $bc, $53, $10, $4f, $00, $ff, $00, $ff, $00, $ff, $00, $fe, $00, $7f, $00
    db $01, $00, $ff, $00, $fe, $19, $f7, $19, $7f, $00, $ee, $00, $bd, $00, $ff, $00
    db $58, $43, $50, $43, $88, $83, $a8, $83, $a8, $03, $50, $07, $a0, $0f, $a0, $0f
    db $00, $fe, $00, $fe, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $ff, $00, $ff, $00, $7f, $00, $7f, $00, $3f, $80, $1f, $c0, $1f, $c0, $0f, $e0
    db $fe, $00, $ff, $00, $ff, $00, $f8, $00, $e3, $03, $c3, $1b, $80, $3c, $c0, $0f
    db $60, $0f, $e0, $0f, $e0, $0f, $00, $1f, $c0, $df, $c0, $df, $00, $3f, $00, $ff
    db $a0, $80, $bf, $80, $7f, $66, $5d, $66, $9f, $00, $6d, $00, $7f, $00, $9f, $00
    db $01, $fc, $00, $fe, $00, $fe, $00, $fe, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $f8, $00, $00, $07, $f0, $f7, $f0, $f7, $00, $1f, $00, $ff, $00, $ff, $00, $e0
    db $03, $00, $01, $fc, $03, $f8, $03, $f8, $07, $f0, $0e, $e0, $1e, $80, $7c, $01
    db $1f, $c0, $07, $e0, $03, $f8, $03, $f8, $9b, $60, $bb, $58, $3b, $38, $bd, $bc
    db $f8, $03, $f0, $07, $e0, $0f, $e0, $0f, $ec, $03, $df, $1c, $de, $1e, $be, $3e
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $0f, $fc, $1b, $f3, $31, $e0, $2f, $cb, $10, $ef, $00, $c0, $2f, $a0, $5f, $d9
    db $c0, $07, $b8, $81, $1e, $00, $e3, $a0, $1c, $e4, $02, $05, $f9, $02, $fc, $80
    db $00, $ff, $00, $ff, $00, $7f, $80, $3f, $c0, $1f, $40, $1f, $00, $bf, $40, $5f
    db $00, $fc, $02, $f8, $02, $f8, $00, $f8, $02, $f8, $01, $fc, $00, $fe, $00, $ff
    db $37, $99, $4f, $00, $b7, $00, $bf, $00, $cf, $00, $ff, $00, $00, $00, $78, $7b
    db $7b, $83, $ec, $00, $7f, $00, $d7, $00, $ff, $00, $ff, $00, $01, $00, $00, $fe
    db $c0, $d3, $8c, $a1, $54, $01, $d4, $01, $ec, $01, $ec, $01, $f8, $03, $c0, $07
    db $78, $7b, $00, $87, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $e0, $1f, $c0
    db $01, $fc, $01, $fc, $03, $f8, $07, $f0, $0e, $e0, $1e, $80, $7c, $01, $f8, $03
    db $c0, $1f, $80, $3f, $80, $3f, $00, $7f, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $00, $ff, $00, $ff, $00, $ff, $0f, $f0, $0f, $f7, $07, $f0, $08, $e8, $0f, $ef
    db $07, $e0, $03, $f8, $03, $f8, $9b, $60, $bb, $58, $3b, $38, $bb, $b8, $bd, $bc
    db $f0, $07, $e0, $0f, $e0, $0f, $ec, $03, $ee, $0d, $de, $1e, $de, $1e, $be, $3e
    db $00, $ff, $00, $ff, $00, $ff, $7c, $83, $7c, $bb, $38, $03, $c4, $c5, $fc, $fd

CommanderTiles1:
    db $03, $f9, $07, $f0, $06, $f0, $06, $f0, $03, $f8, $00, $fc, $00, $fe, $00, $fd
    db $ff, $3f, $df, $c0, $ef, $60, $ef, $20, $00, $00, $00, $ff, $ff, $2e, $00, $ff
    db $00, $ff, $f0, $ff, $fe, $0f, $ff, $01, $1f, $00, $03, $e0, $fc, $d0, $03, $fe
    db $00, $ff, $00, $ff, $00, $ff, $80, $ff, $e0, $7f, $fe, $1d, $fc, $01, $00, $03
    db $02, $fa, $00, $fe, $01, $fc, $01, $fc, $01, $fc, $01, $f8, $07, $f0, $07, $f0
    db $7e, $7e, $d7, $00, $f7, $00, $f8, $07, $f7, $00, $fb, $00, $ff, $00, $fd, $00
    db $c8, $c8, $bf, $00, $bc, $00, $db, $c3, $f7, $07, $37, $07, $f7, $07, $f7, $07
    db $e0, $8f, $80, $07, $78, $7b, $fc, $fd, $fc, $fd, $f8, $fb, $f8, $fb, $90, $97
    db $07, $f0, $07, $f0, $03, $f8, $04, $e4, $17, $d7, $1f, $df, $0f, $ef, $0f, $ef
    db $f6, $00, $e9, $08, $ee, $0e, $1e, $1e, $7f, $7f, $7f, $7f, $bf, $bf, $df, $df
    db $f7, $07, $ef, $0f, $cf, $0f, $bf, $3f, $1f, $1f, $4e, $4e, $d1, $c0, $df, $c0
    db $60, $0f, $20, $0f, $66, $03, $eb, $01, $5b, $01, $b7, $03, $f7, $03, $cf, $07
    db $03, $f3, $00, $fc, $00, $fe, $00, $ff, $00, $fe, $01, $fc, $03, $f9, $07, $f3
    db $a7, $87, $78, $00, $ff, $00, $7f, $00, $7f, $00, $bf, $80, $cf, $c0, $e3, $c0
    db $bf, $80, $7f, $00, $fe, $00, $fe, $00, $fd, $00, $fb, $01, $f7, $03, $cf, $07
    db $bf, $0f, $46, $04, $aa, $38, $92, $38, $aa, $b8, $ba, $80, $c5, $c0, $fd, $f8
    db $03, $f9, $07, $f0, $06, $f0, $05, $f0, $01, $f8, $01, $fc, $02, $f8, $07, $f0
    db $ff, $3f, $1f, $00, $ef, $00, $f7, $00, $fa, $00, $7c, $01, $3c, $01, $98, $03
    db $07, $f0, $0f, $e0, $ce, $a0, $9f, $40, $1f, $c0, $39, $80, $3e, $80, $3f, $80
    db $e0, $04, $fb, $00, $7d, $00, $bc, $00, $de, $00, $ef, $00, $f7, $00, $7a, $00
    db $08, $08, $bf, $00, $bc, $00, $5b, $03, $f7, $07, $37, $07, $77, $07, $f7, $07
    db $27, $80, $3b, $80, $3d, $80, $9a, $c0, $83, $e5, $c7, $bb, $ff, $07, $0f, $ef
    db $bd, $00, $d9, $00, $c6, $06, $3e, $3e, $7f, $ff, $7f, $7f, $bf, $bf, $df, $df
    db $f7, $07, $ef, $0f, $cf, $0f, $bf, $3f, $1f, $1f, $4e, $4e, $d1, $c0, $cf, $c0
    db $e3, $13, $fc, $e0, $fe, $fc, $ff, $fe, $fe, $fc, $fd, $f8, $fb, $f1, $f7, $e3
    db $a7, $87, $78, $00, $ef, $00, $71, $00, $7e, $00, $bf, $80, $cf, $c0, $e3, $c0
    db $b7, $90, $77, $10, $36, $30, $ce, $00, $7d, $00, $fb, $01, $f7, $03, $cf, $07
    db $90, $ff, $47, $f2, $0e, $e0, $1d, $c0, $9b, $c0, $0b, $e0, $02, $f0, $04, $f0
    db $00, $ff, $fe, $7f, $3f, $01, $df, $00, $ef, $00, $f4, $00, $f8, $03, $79, $03
    db $00, $ff, $00, $ff, $f8, $ff, $ff, $07, $ff, $00, $0f, $00, $00, $f0, $ff, $49
    db $66, $ff, $46, $ff, $08, $ff, $c0, $ff, $fc, $3b, $f0, $07, $01, $0f, $02, $1f
    db $0f, $e0, $0f, $e0, $1f, $c0, $9c, $40, $3f, $80, $3f, $80, $73, $00, $7d, $00
    db $30, $07, $c0, $08, $f6, $00, $fa, $00, $79, $00, $bd, $00, $de, $00, $ee, $00
    db $01, $f8, $26, $20, $f8, $00, $f7, $07, $6f, $0f, $df, $1f, $5f, $1f, $df, $1f
    db $80, $3f, $00, $1f, $e0, $e7, $f8, $fb, $f8, $fb, $f0, $f7, $f0, $f7, $f0, $f7
    db $7e, $00, $4f, $00, $77, $00, $7b, $00, $34, $80, $06, $cb, $8f, $77, $ff, $0f
    db $f5, $00, $7b, $00, $b3, $00, $8d, $0c, $7d, $7c, $fe, $fe, $7f, $7f, $3f, $3f
    db $ee, $0e, $ee, $0e, $ee, $0e, $de, $1e, $3f, $3e, $9e, $9e, $ad, $8c, $93, $80
    db $20, $2f, $c6, $03, $5b, $01, $db, $01, $d7, $03, $b7, $03, $4f, $07, $bf, $0f
    db $1f, $df, $c7, $27, $f8, $c0, $fd, $f8, $fe, $fc, $fd, $f9, $fb, $f1, $f7, $e3
    db $df, $df, $af, $8f, $70, $00, $ef, $00, $f1, $00, $7e, $00, $bf, $80, $cf, $c0
    db $af, $80, $6e, $20, $ee, $60, $1d, $00, $fb, $01, $77, $03, $cf, $07, $3f, $0f
    db $46, $04, $aa, $38, $92, $38, $aa, $b8, $ba, $80, $c5, $c0, $fd, $f8, $fd, $f8
    db $c8, $c8, $bf, $00, $bc, $00, $db, $c3, $f7, $07, $37, $07, $8f, $07, $77, $67
    db $f6, $00, $e9, $08, $ee, $0e, $1e, $1e, $7f, $7f, $7f, $7f, $b8, $b8, $c7, $e0
    db $77, $67, $77, $07, $77, $07, $77, $07, $77, $07, $76, $06, $71, $00, $f3, $00
    db $9c, $80, $53, $00, $df, $00, $5f, $00, $5f, $00, $af, $80, $cf, $c0, $ef, $c0
    db $75, $00, $f6, $00, $f6, $00, $fe, $00, $fe, $00, $fe, $00, $fd, $01, $fb, $03
    db $7e, $7e, $d7, $00, $f7, $00, $fe, $07, $f7, $03, $f9, $01, $fc, $00, $fd, $00
    db $c8, $c8, $bf, $00, $bc, $00, $5b, $c3, $d7, $c7, $97, $87, $17, $07, $37, $07
    db $f6, $00, $e9, $08, $ee, $0e, $1e, $1e, $7e, $7e, $7f, $7f, $bf, $bf, $df, $df
    db $8f, $07, $77, $67, $77, $67, $77, $07, $77, $07, $76, $06, $71, $00, $77, $00
    db $03, $f3, $00, $fc, $03, $f0, $0f, $e1, $1e, $c2, $1c, $dc, $03, $e1, $07, $f3
    db $88, $a8, $67, $00, $dc, $00, $93, $80, $1f, $40, $5f, $80, $5f, $00, $af, $80
    db $77, $00, $f3, $00, $74, $00, $f6, $00, $f6, $00, $fe, $00, $fe, $00, $fe, $00

CommanderTiles2:
    db $0f, $f7, $0b, $f0, $05, $fc, $06, $fe, $02, $fe, $00, $ff, $01, $fe, $02, $fe
    db $fe, $ff, $ff, $7f, $ff, $00, $00, $ff, $ff, $36, $00, $00, $04, $72, $0b, $c4
    db $3f, $bf, $ff, $f9, $ff, $00, $04, $f8, $fc, $61, $00, $01, $f8, $03, $78, $03
    db $00, $ff, $80, $bf, $00, $7f, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $07, $fc, $00, $ff, $00, $fc, $03, $f8, $07, $f0, $07, $f0, $07, $f0, $03, $f8
    db $a3, $18, $57, $20, $39, $07, $f7, $00, $f9, $00, $ff, $00, $f9, $00, $f7, $00
    db $7c, $01, $f8, $00, $37, $00, $f4, $00, $fc, $00, $fe, $00, $f5, $00, $fb, $00
    db $00, $ff, $00, $ff, $00, $7f, $80, $3f, $80, $03, $bc, $01, $9d, $18, $6e, $0c
    db $00, $fc, $00, $ff, $00, $ff, $00, $fe, $01, $fc, $00, $fe, $00, $ff, $00, $ff
    db $0e, $00, $7f, $00, $7f, $00, $07, $00, $f8, $00, $ff, $00, $7f, $00, $7f, $00
    db $fc, $00, $1d, $00, $ed, $00, $fe, $00, $fe, $00, $6e, $00, $9e, $00, $ee, $00
    db $ee, $0c, $df, $1e, $df, $1f, $df, $1f, $df, $1f, $df, $1f, $bf, $3f, $bf, $3e
    db $00, $fe, $00, $fe, $00, $fe, $00, $fe, $00, $ff, $01, $fd, $02, $fa, $02, $fa
    db $ff, $00, $ff, $00, $ff, $00, $ff, $00, $7f, $00, $7f, $00, $bf, $00, $c0, $00
    db $ee, $00, $fd, $00, $fd, $00, $fa, $00, $f6, $00, $cd, $01, $3b, $03, $f7, $07
    db $bf, $3e, $7f, $7e, $7e, $7c, $c2, $c0, $aa, $90, $aa, $b8, $56, $24, $56, $74
    db $0f, $f7, $0f, $f0, $05, $fa, $04, $fb, $02, $fc, $01, $fe, $02, $fd, $04, $f9
    db $3f, $ff, $ff, $ff, $ff, $00, $00, $ff, $ff, $36, $00, $00, $7f, $00, $7f, $00
    db $ff, $fe, $ff, $f9, $ff, $00, $00, $fe, $fc, $61, $00, $01, $f8, $03, $f8, $03
    db $80, $7f, $80, $3f, $00, $7f, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $00, $fe, $00, $fe, $01, $fc, $01, $fc, $01, $fc, $01, $fc, $01, $fc, $01, $fc
    db $ff, $00, $7f, $00, $bf, $00, $fd, $00, $ff, $00, $bf, $00, $be, $00, $b9, $01
    db $fc, $01, $c2, $00, $3e, $00, $ff, $00, $e0, $00, $9f, $1f, $7f, $7f, $fe, $fe
    db $00, $ff, $00, $ff, $00, $ff, $00, $7f, $00, $1f, $e0, $83, $fc, $e1, $0c, $09
    db $00, $fe, $00, $ff, $00, $fe, $00, $fe, $00, $fe, $0f, $f5, $1f, $e2, $03, $e0
    db $77, $07, $6f, $0f, $dc, $1c, $b3, $30, $0f, $03, $ef, $4f, $f7, $a7, $ef, $0f
    db $e1, $e0, $1f, $01, $ff, $1f, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
    db $f2, $00, $fd, $d0, $ff, $ed, $ff, $ef, $ff, $f7, $ff, $f7, $ff, $f7, $ff, $fb
    db $1c, $dc, $1e, $de, $3f, $bf, $3f, $bf, $3f, $bf, $7f, $7f, $7f, $7f, $7f, $7f
    db $1f, $1f, $ff, $ff, $7f, $7f, $7f, $7f, $bf, $bf, $bf, $bf, $df, $df, $df, $df
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff
    db $ff, $fb, $ff, $fb, $ff, $fb, $ff, $fd, $ff, $fd, $ff, $fd, $ff, $fd, $ff, $fd
    db $40, $ff, $20, $ff, $0f, $f7, $0b, $f0, $c5, $fa, $24, $fb, $02, $fc, $01, $fe
    db $00, $ff, $00, $ff, $3f, $ff, $ff, $ff, $ff, $00, $00, $ff, $ff, $36, $00, $00
    db $00, $ff, $00, $ff, $ff, $fe, $ff, $f9, $ff, $00, $00, $fe, $fc, $61, $00, $01
    db $10, $ff, $20, $ff, $80, $7f, $86, $3f, $08, $7f, $00, $ff, $00, $ff, $00, $ff
    db $02, $fd, $04, $f9, $20, $fe, $c0, $fe, $01, $fc, $01, $fc, $01, $fc, $01, $fc
    db $7f, $00, $7f, $00, $ff, $00, $7f, $00, $bd, $00, $ff, $00, $fc, $00, $b3, $03
    db $f8, $03, $fc, $01, $c4, $01, $3e, $00, $e0, $00, $9f, $1f, $7f, $7f, $fe, $fe
    db $10, $ff, $0c, $ff, $00, $ff, $00, $ff, $00, $1f, $e0, $83, $fc, $e1, $0c, $09
    db $01, $fc, $00, $fe, $00, $ff, $0f, $f5, $1f, $e2, $03, $e0, $1c, $dc, $3e, $be
    db $6f, $0f, $d0, $10, $0f, $00, $ef, $4f, $f7, $a7, $ef, $0f, $1f, $1f, $ff, $ff
    db $3f, $bf, $3f, $bf, $3f, $bf, $7f, $7f, $7f, $7f, $7f, $7f, $7f, $7f, $7f, $7f
    db $7f, $7f, $7f, $7f, $bf, $bf, $bf, $bf, $df, $df, $df, $df, $df, $df, $df, $df
    db $00, $fc, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $0f, $00, $7c, $00, $03, $83, $7f, $db, $7b, $d8, $5f, $83, $7e, $da, $01, $00
    db $8c, $00, $6d, $40, $cd, $40, $6e, $00, $de, $40, $be, $00, $76, $00, $f6, $00
    db $01, $fc, $00, $fe, $00, $fe, $00, $fe, $00, $ff, $01, $fd, $02, $fa, $02, $fa
    db $00, $ff, $0f, $f7, $0b, $f0, $05, $fc, $06, $fe, $02, $fe, $00, $ff, $01, $fe
    db $00, $ff, $fe, $ff, $ff, $7f, $ff, $00, $00, $ff, $ff, $36, $00, $00, $04, $72
    db $00, $ff, $3f, $bf, $ff, $f9, $ff, $00, $04, $f8, $fc, $61, $00, $01, $f8, $03
    db $00, $ff, $00, $ff, $80, $bf, $00, $7f, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $02, $fe, $07, $fc, $00, $ff, $00, $fc, $03, $f8, $07, $f0, $07, $f0, $07, $f0
    db $0b, $c4, $a3, $18, $57, $20, $39, $07, $f7, $00, $f9, $00, $ff, $00, $f9, $00
    db $78, $03, $7c, $01, $f8, $00, $37, $00, $f4, $00, $fc, $00, $fe, $00, $f5, $00
    db $00, $ff, $00, $ff, $00, $ff, $00, $43, $bc, $01, $9d, $18, $ae, $0c, $ae, $0c
    db $03, $f8, $00, $fc, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $f7, $00, $0f, $00, $7c, $00, $03, $03, $7f, $db, $7b, $d8, $5f, $83, $7e, $da
    db $fb, $00, $8c, $00, $6d, $40, $cd, $40, $6e, $00, $de, $40, $be, $00, $76, $00
    db $5f, $1e, $df, $1f, $df, $1f, $df, $1f, $df, $1f, $bf, $3f, $bf, $3e, $bf, $3e
    db $00, $fe, $01, $fc, $00, $fe, $00, $fe, $00, $fe, $03, $fb, $03, $fb, $06, $f6
    db $01, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $7f, $00, $7f, $00, $bf, $00
    db $f6, $00, $ee, $00, $fc, $00, $fc, $00, $f9, $01, $f3, $03, $c7, $07, $2e, $0e
    db $7f, $7e, $7e, $7c, $c2, $c0, $aa, $90, $aa, $b8, $56, $24, $56, $74, $ae, $4c

CommanderTiles3:
    db $00, $ff, $00, $ff, $01, $fe, $06, $f9, $08, $f7, $00, $ef, $00, $ef, $00, $f6
    db $07, $f8, $39, $c5, $c3, $3b, $07, $f6, $07, $f6, $07, $e3, $07, $18, $78, $b0
    db $e0, $1f, $9c, $a3, $c3, $dc, $e0, $ef, $e0, $6f, $e0, $c7, $e0, $18, $1c, $0b
    db $00, $ff, $00, $ff, $80, $7f, $60, $9f, $10, $ef, $00, $f7, $00, $f7, $00, $6f
    db $01, $f8, $00, $fc, $00, $fd, $00, $fe, $00, $fe, $00, $fe, $02, $fe, $06, $fc
    db $80, $ff, $00, $00, $40, $ff, $3b, $3f, $c0, $00, $87, $78, $c1, $3e, $f7, $1d
    db $03, $fc, $00, $00, $00, $ff, $90, $fc, $03, $00, $e1, $1e, $83, $7c, $6f, $38
    db $80, $1f, $00, $3f, $00, $bf, $00, $7f, $00, $7f, $00, $7f, $40, $7f, $60, $3f
    db $06, $f8, $06, $f8, $05, $f8, $01, $f8, $01, $fc, $01, $fc, $01, $fc, $00, $fe
    db $ff, $01, $ff, $42, $ff, $84, $ff, $84, $fb, $07, $f0, $8f, $f0, $0f, $f8, $00
    db $7f, $00, $bf, $00, $df, $00, $df, $00, $df, $e0, $0f, $f0, $0f, $f0, $1f, $00
    db $60, $1f, $60, $1f, $a0, $1f, $80, $1f, $80, $3f, $80, $3f, $80, $3f, $00, $7f
    db $00, $fe, $00, $ff, $00, $ff, $00, $ff, $fc, $53, $fe, $f9, $f8, $02, $00, $07
    db $e7, $00, $7e, $00, $0f, $82, $73, $83, $1c, $00, $03, $60, $20, $5c, $18, $46
    db $e7, $00, $7e, $00, $f0, $01, $ce, $01, $38, $00, $c0, $06, $04, $3a, $18, $62
    db $00, $7f, $00, $ff, $00, $ff, $00, $ff, $3f, $ca, $7f, $9f, $1f, $40, $00, $e0
    db $01, $f8, $00, $fc, $00, $fd, $00, $fe, $00, $fe, $00, $fe, $00, $fe, $07, $f9
    db $80, $ff, $00, $00, $40, $ff, $3b, $3f, $c0, $00, $87, $78, $c4, $38, $fc, $fd
    db $03, $fc, $00, $00, $00, $ff, $90, $fc, $03, $00, $e1, $1e, $00, $00, $05, $fa
    db $80, $1f, $00, $3f, $00, $bf, $00, $7f, $00, $7f, $00, $7f, $00, $3f, $00, $c0
    db $0f, $f7, $1f, $e9, $1f, $ed, $1f, $ec, $1f, $ea, $1f, $eb, $1f, $eb, $1f, $eb
    db $fc, $f9, $fc, $f9, $fc, $f9, $fc, $f9, $fc, $f9, $fc, $b9, $f8, $11, $f8, $01
    db $05, $0a, $05, $ff, $05, $fa, $05, $ff, $05, $ff, $05, $ff, $05, $fa, $05, $ff
    db $00, $ff, $00, $80, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $0f, $f3, $07, $f9, $03, $fc, $00, $ff, $fc, $53, $fe, $f9, $f8, $02, $00, $07
    db $c4, $c1, $fc, $c0, $8f, $02, $73, $83, $1c, $00, $03, $60, $20, $5c, $18, $46
    db $05, $fa, $00, $00, $f0, $01, $ce, $01, $38, $00, $c0, $06, $04, $3a, $18, $62
    db $00, $ff, $00, $0f, $30, $d0, $78, $a3, $70, $c7, $40, $8f, $00, $5f, $00, $fe
    db $31, $ff, $30, $ff, $08, $ff, $00, $ff, $03, $fc, $0c, $f3, $00, $ef, $80, $ef
    db $80, $ff, $80, $ff, $0f, $f0, $71, $8d, $83, $7a, $07, $f6, $07, $f3, $07, $c0
    db $01, $ff, $01, $ff, $f0, $0f, $8e, $b1, $c1, $de, $e0, $6f, $e0, $cf, $e0, $03
    db $8c, $ff, $0c, $ff, $10, $ff, $00, $ff, $c0, $3f, $30, $cf, $00, $f7, $01, $f7
    db $c0, $f6, $01, $f8, $00, $fc, $00, $fd, $00, $fe, $00, $fe, $00, $fe, $00, $fe
    db $00, $3f, $f8, $b0, $00, $7f, $00, $80, $40, $ff, $38, $3f, $c3, $07, $80, $78
    db $00, $fc, $1f, $08, $00, $fe, $00, $01, $00, $ff, $10, $fc, $83, $e0, $00, $00
    db $03, $6f, $80, $1f, $00, $3f, $00, $bf, $00, $7f, $00, $7f, $00, $7f, $00, $3f
    db $07, $f9, $0f, $f7, $1f, $e9, $1f, $ed, $1f, $ec, $1f, $ea, $1f, $eb, $1f, $eb
    db $fc, $fd, $fc, $f9, $fc, $f9, $fc, $f9, $fc, $f9, $fc, $f9, $fc, $b9, $f8, $11
    db $05, $fa, $05, $0a, $05, $ff, $05, $fa, $05, $ff, $05, $ff, $05, $ff, $05, $fa
    db $00, $c0, $00, $ff, $00, $80, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $1f, $eb, $0f, $f3, $07, $f9, $fd, $52, $fe, $f9, $f8, $02, $00, $07, $00, $ff
    db $f8, $01, $c4, $c1, $cc, $c0, $a3, $00, $7c, $80, $1f, $00, $03, $60, $20, $5c
    db $05, $ff, $05, $fa, $00, $00, $c4, $01, $3e, $01, $f8, $00, $c0, $06, $04, $3a
    db $00, $ff, $00, $ff, $00, $0f, $30, $d0, $78, $a3, $70, $c7, $40, $8f, $00, $5f
    db $ff, $01, $ff, $02, $bf, $04, $5f, $44, $6b, $67, $60, $4f, $70, $5f, $bf, $16
    db $7f, $00, $bf, $00, $dd, $00, $da, $02, $d6, $e6, $06, $f6, $0e, $f6, $fd, $d4
    db $df, $16, $60, $00, $0f, $82, $73, $83, $1c, $00, $03, $60, $20, $5c, $18, $46
    db $fb, $d0, $06, $00, $f0, $01, $ce, $01, $38, $00, $c0, $06, $04, $3a, $18, $62
    db $00, $ff, $00, $ff, $00, $ff, $01, $fe, $06, $f9, $08, $f7, $00, $ef, $00, $ef
    db $00, $ff, $07, $f8, $39, $c5, $c3, $3b, $07, $f6, $07, $f6, $07, $e3, $07, $18
    db $00, $ff, $e0, $1f, $9c, $a3, $c3, $dc, $e0, $ef, $e0, $6f, $e0, $c7, $e0, $18
    db $00, $ff, $00, $ff, $00, $ff, $80, $7f, $60, $9f, $10, $ef, $00, $f7, $00, $f7
    db $00, $f6, $01, $f8, $00, $fc, $00, $fd, $00, $fe, $00, $fe, $00, $fe, $02, $fe
    db $78, $b0, $80, $ff, $00, $00, $40, $ff, $3b, $3f, $c0, $00, $83, $7c, $fd, $0a
    db $1c, $0b, $03, $fc, $00, $00, $00, $ff, $90, $fc, $03, $00, $c1, $3e, $bf, $50
    db $00, $6f, $80, $1f, $00, $3f, $00, $bf, $00, $7f, $00, $7f, $00, $7f, $40, $7f
    db $06, $fc, $06, $f8, $06, $f8, $05, $f8, $01, $f8, $01, $fc, $01, $fc, $01, $fc
    db $f7, $1d, $ff, $01, $ff, $02, $bf, $04, $5f, $44, $6b, $67, $60, $4f, $70, $5f
    db $6f, $38, $7f, $00, $bf, $00, $dd, $00, $da, $02, $d6, $e6, $06, $f6, $0e, $f6
    db $60, $3f, $60, $1f, $60, $1f, $a0, $1f, $80, $1f, $80, $3f, $80, $3f, $80, $3f
    db $00, $fe, $00, $ff, $00, $ff, $fc, $53, $fe, $f9, $f8, $02, $00, $07, $00, $ff
    db $bf, $16, $df, $16, $60, $00, $0f, $82, $73, $83, $1c, $00, $03, $60, $20, $5c
    db $fd, $d4, $fb, $d0, $06, $00, $f0, $01, $ce, $01, $38, $00, $c0, $06, $04, $3a
    db $00, $7f, $00, $ff, $00, $ff, $3f, $ca, $7f, $9f, $1f, $40, $00, $e0, $00, $ff

CommanderTiles4:
    db $00, $ff, $00, $ff, $03, $ff, $0f, $fc, $1f, $f0, $3f, $e0, $3f, $e0, $3e, $80
    db $0f, $ff, $7c, $f0, $fb, $83, $fb, $02, $fb, $02, $e1, $01, $00, $1e, $0f, $f7
    db $f0, $ff, $3e, $0f, $df, $c1, $df, $c0, $df, $40, $87, $80, $00, $78, $f0, $ef
    db $00, $ff, $00, $ff, $c0, $ff, $f0, $3f, $f8, $0f, $fc, $07, $fc, $07, $7c, $01
    db $3c, $81, $0c, $c1, $01, $f0, $00, $fc, $03, $ff, $03, $ff, $02, $ff, $01, $ff
    db $01, $fd, $fe, $68, $00, $ff, $00, $3f, $c0, $00, $81, $7e, $00, $ff, $01, $ff
    db $80, $3f, $7f, $0e, $00, $ff, $00, $fc, $03, $00, $81, $7e, $00, $ff, $00, $7f
    db $3c, $81, $30, $83, $80, $0f, $00, $3f, $c0, $ff, $c0, $ff, $40, $ff, $80, $ff
    db $01, $fd, $03, $fd, $02, $fc, $02, $fc, $02, $fc, $03, $fc, $01, $fc, $00, $fe
    db $43, $fe, $a7, $7a, $cf, $32, $ff, $04, $bf, $08, $59, $00, $7e, $00, $5f, $00
    db $90, $3f, $a9, $1e, $b3, $0c, $df, $00, $ed, $00, $9a, $00, $7e, $00, $fa, $00
    db $80, $bf, $c0, $bf, $40, $3f, $40, $3f, $40, $3f, $c0, $3f, $80, $3f, $00, $7f
    db $03, $fc, $04, $fb, $07, $fb, $e4, $1f, $f4, $4f, $ec, $87, $dc, $1f, $b4, $37
    db $58, $00, $db, $8c, $57, $98, $2c, $b0, $57, $e0, $28, $c0, $3f, $80, $5f, $c0
    db $1a, $00, $ea, $00, $fa, $00, $34, $00, $ea, $00, $15, $01, $fd, $01, $fb, $03
    db $00, $ff, $00, $ff, $00, $ff, $07, $f8, $0f, $72, $d7, $81, $db, $98, $ed, $cc
    db $3c, $81, $0c, $c1, $01, $f0, $00, $fc, $03, $ff, $03, $ff, $02, $ff, $01, $f8
    db $01, $fd, $fe, $68, $00, $ff, $00, $3f, $c0, $00, $81, $7e, $00, $fc, $f9, $00
    db $80, $3f, $7f, $0e, $00, $ff, $00, $fc, $03, $00, $81, $7e, $00, $00, $fd, $00
    db $3c, $81, $30, $83, $80, $0f, $00, $3f, $c0, $ff, $c0, $ff, $00, $1f, $60, $00
    db $07, $f0, $09, $e0, $0d, $e0, $0c, $e0, $0a, $e0, $0b, $e0, $0b, $e0, $0b, $e0
    db $fd, $01, $fd, $01, $fd, $01, $fd, $01, $fd, $01, $bd, $01, $19, $01, $01, $01
    db $fe, $fe, $fe, $fe, $ff, $fe, $ff, $fe, $fe, $fe, $ff, $fe, $fe, $fe, $fe, $fe
    db $bf, $a0, $bf, $bf, $ff, $bf, $ff, $bf, $bf, $bf, $ff, $bf, $bf, $bf, $bf, $bf
    db $03, $f0, $01, $f8, $00, $fc, $e4, $1f, $f4, $4f, $ec, $87, $dc, $1f, $b4, $37
    db $c5, $00, $d9, $0c, $14, $18, $ec, $30, $57, $e0, $28, $c0, $3f, $80, $5f, $c0
    db $fe, $00, $fd, $00, $00, $00, $34, $00, $ea, $00, $15, $01, $fd, $01, $fb, $03
    db $bf, $1f, $7f, $00, $3f, $00, $40, $80, $77, $27, $cf, $8f, $df, $9f, $ef, $cf
    db $c6, $ff, $c2, $ff, $20, $ff, $00, $ff, $83, $ff, $cf, $fc, $3f, $f0, $3f, $80
    db $00, $ff, $00, $ff, $0f, $ff, $7c, $f0, $fb, $83, $fb, $02, $fb, $02, $e1, $01
    db $00, $ff, $00, $ff, $f0, $ff, $3e, $0f, $df, $c1, $df, $c0, $df, $40, $87, $80
    db $33, $ff, $23, $ff, $04, $ff, $00, $ff, $c1, $ff, $f3, $3f, $fc, $0f, $fc, $01
    db $3f, $80, $3c, $80, $08, $c3, $03, $f1, $00, $fd, $00, $fc, $03, $ff, $03, $ff
    db $00, $1f, $0f, $f7, $01, $fd, $fe, $68, $00, $ff, $00, $ff, $00, $0f, $80, $70
    db $00, $f8, $f0, $ef, $80, $3f, $7f, $0e, $00, $ff, $00, $ff, $00, $f0, $01, $0e
    db $fc, $01, $3c, $01, $10, $c3, $c0, $0f, $00, $bf, $00, $3f, $c0, $ff, $c0, $ff
    db $01, $f8, $07, $f0, $09, $e0, $0d, $e0, $0c, $e0, $0a, $e0, $0b, $e0, $0b, $e0
    db $f9, $00, $fd, $01, $fd, $01, $fd, $01, $fd, $01, $fd, $01, $bd, $01, $19, $01
    db $fd, $00, $fe, $fe, $fe, $fe, $ff, $fe, $ff, $fe, $fe, $fe, $ff, $fe, $fe, $fe
    db $60, $00, $bf, $a0, $bf, $bf, $ff, $bf, $ff, $bf, $bf, $bf, $ff, $bf, $bf, $bf
    db $0b, $e0, $03, $f0, $e1, $18, $f0, $4c, $e8, $83, $d4, $17, $b4, $37, $74, $77
    db $01, $01, $c5, $00, $d9, $0c, $10, $18, $e4, $30, $48, $e0, $3f, $c0, $3f, $80
    db $fe, $fe, $fe, $00, $fd, $00, $00, $00, $14, $00, $0a, $00, $fd, $01, $fd, $01
    db $bf, $bf, $bf, $1f, $7f, $00, $3f, $00, $40, $80, $77, $27, $cf, $8f, $df, $9f
    db $43, $fe, $a7, $7a, $8f, $32, $7f, $04, $bf, $08, $59, $40, $66, $20, $78, $28
    db $90, $3f, $a9, $1e, $b1, $0c, $de, $00, $ed, $00, $9a, $02, $66, $04, $1e, $14
    db $3f, $2e, $bf, $ae, $57, $9e, $27, $b6, $58, $e0, $2f, $c0, $30, $80, $5f, $c0
    db $fd, $d4, $fd, $d4, $fa, $d0, $e4, $c0, $1e, $00, $f5, $01, $0d, $01, $fb, $03
    db $00, $ff, $00, $ff, $00, $ff, $03, $ff, $0f, $fc, $1f, $f0, $3f, $e0, $3f, $e0
    db $00, $ff, $0f, $ff, $7c, $f0, $fb, $83, $fb, $02, $fb, $02, $e1, $01, $00, $1e
    db $00, $ff, $f0, $ff, $3e, $0f, $df, $c1, $df, $c0, $df, $40, $87, $80, $00, $78
    db $00, $ff, $00, $ff, $00, $ff, $c0, $ff, $f0, $3f, $f8, $0f, $fc, $07, $fc, $07
    db $3e, $80, $3c, $81, $0c, $c1, $01, $f0, $00, $fc, $03, $ff, $03, $ff, $02, $ff
    db $0f, $f7, $01, $fd, $fe, $68, $00, $ff, $00, $3f, $c0, $00, $81, $7e, $00, $ff
    db $f0, $ef, $80, $3f, $7f, $0e, $00, $ff, $00, $fc, $03, $00, $81, $7e, $00, $ff
    db $7c, $01, $3c, $81, $30, $83, $80, $0f, $00, $3f, $c0, $ff, $c0, $ff, $40, $ff
    db $01, $ff, $01, $fd, $03, $fd, $02, $fc, $02, $fc, $02, $fc, $03, $fc, $01, $fc
    db $01, $ff, $43, $fe, $a7, $7a, $8f, $32, $7f, $04, $bf, $08, $59, $40, $66, $20
    db $00, $7f, $90, $3f, $a9, $1e, $b1, $0c, $de, $00, $ed, $00, $9a, $02, $66, $04
    db $80, $ff, $80, $bf, $c0, $bf, $40, $3f, $40, $3f, $40, $3f, $c0, $3f, $80, $3f
    db $00, $fe, $07, $f8, $e4, $1b, $f7, $4f, $ec, $87, $dc, $1f, $34, $37, $f4, $f7
    db $78, $28, $3f, $2e, $bf, $ae, $57, $9e, $27, $b6, $58, $e0, $2f, $c0, $30, $80
    db $1e, $14, $fd, $d4, $fd, $d4, $fa, $d0, $e4, $c0, $1e, $00, $f5, $01, $0d, $01
    db $00, $ff, $00, $ff, $07, $f8, $0f, $72, $d7, $81, $db, $98, $ec, $cc, $ef, $cf
    db $ff, $00, $ff, $52, $ff, $2c, $5f, $12, $2f, $0c, $1f, $0a, $1f, $0c, $17, $04
    db $2f, $06, $17, $04, $2f, $00, $0f, $04, $0f, $02, $17, $04, $0f, $02, $1f, $04
    db $ff, $00, $ff, $8a, $ff, $d5, $bb, $00, $45, $00, $a0, $00, $00, $00, $00, $00
    db $ff, $00, $ff, $d2, $ff, $6c, $7f, $32, $7f, $14, $3f, $0e, $3f, $1a, $3f, $0c
    db $1f, $0a, $1f, $08, $1f, $04, $3f, $0c, $1f, $06, $1f, $08, $3f, $06, $1f, $04
    db $ff, $00, $ff, $8a, $ff, $3b, $ff, $d4, $ff, $00, $12, $00, $00, $00, $00, $00
    db $ff, $00, $ff, $44, $ff, $a8, $ff, $56, $ff, $74, $ff, $28, $ff, $34, $7f, $14
    db $3f, $1a, $7f, $0a, $3f, $0c, $3f, $14, $3f, $08, $3f, $0c, $3f, $0a, $3f, $04
    db $ff, $00, $ff, $c2, $ff, $35, $ff, $ee, $ff, $90, $ff, $00, $40, $00, $00, $00

NextLevelBgTiles:
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $fc, $fc, $f3, $f1
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $81, $81, $7e, $3c, $ff, $c3
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $3f, $3f, $cf, $8f
    db $ff, $ff, $ff, $ff, $fc, $fc, $f3, $f1, $ef, $e6, $de, $c8, $b8, $90, $70, $23
    db $cf, $c6, $3e, $18, $f8, $60, $e0, $83, $80, $0c, $00, $30, $00, $c0, $00, $00
    db $c3, $00, $00, $24, $00, $e7, $00, $24, $00, $24, $00, $24, $00, $24, $00, $24
    db $f3, $63, $7c, $18, $1f, $06, $07, $c1, $01, $30, $00, $0c, $00, $03, $00, $00
    db $ff, $ff, $ff, $ff, $3f, $3f, $cf, $8f, $f7, $67, $7b, $13, $1d, $09, $0e, $c4
    db $fe, $fe, $fd, $fc, $fb, $f9, $f7, $f2, $ee, $e4, $dc, $c8, $b8, $91, $70, $22
    db $e0, $44, $c0, $8c, $80, $14, $10, $24, $10, $44, $10, $84, $10, $04, $10, $04
    db $00, $24, $00, $24, $00, $24, $00, $24, $00, $24, $00, $24, $00, $24, $00, $e7
    db $07, $22, $03, $31, $01, $28, $08, $24, $08, $22, $08, $21, $08, $20, $08, $20
    db $7f, $7f, $bf, $3f, $df, $9f, $ef, $4f, $77, $27, $3b, $13, $1d, $89, $0e, $44
    db $10, $04, $10, $04, $10, $05, $10, $07, $13, $04, $07, $30, $1f, $e0, $7f, $80
    db $00, $0f, $07, $78, $3f, $c0, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00
    db $00, $f0, $e0, $1e, $fc, $03, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00
    db $08, $20, $08, $20, $08, $a0, $08, $e0, $c8, $20, $e0, $0c, $f8, $07, $fe, $01
    db $10, $07, $11, $06, $13, $04, $17, $00, $0f, $30, $1f, $60, $3f, $c0, $7f, $80
    db $08, $e0, $88, $60, $c8, $20, $e8, $00, $f0, $0c, $f8, $06, $fc, $03, $fe, $01
    db $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $78, $fb, $f0, $fb, $f0, $03, $00
    db $ff, $00, $ff, $3c, $ff, $7e, $bd, $3c, $c3, $00, $ff, $00, $ff, $00, $ff, $00
    db $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $1e, $df, $0f, $df, $0f, $c0, $00
    db $ff, $0f, $f0, $00, $f0, $00, $f9, $79, $86, $07, $85, $06, $cb, $cc, $07, $f8
    db $ff, $01, $ff, $01, $ff, $83, $7f, $82, $fc, $00, $fc, $01, $fc, $01, $ff, $00
    db $fb, $f0, $fb, $f0, $fb, $00, $0b, $00, $03, $b0, $43, $b8, $43, $d8, $bf, $e0
    db $df, $0f, $df, $0f, $df, $00, $d0, $00, $c0, $0f, $c2, $1d, $c2, $1b, $fd, $07
    db $ff, $80, $ff, $80, $ff, $c1, $fe, $41, $3f, $00, $3f, $80, $3f, $80, $ff, $00
    db $ff, $f0, $0f, $00, $0f, $00, $9f, $9e, $61, $e0, $a1, $60, $d3, $33, $e0, $1f
    db $ff, $01, $f0, $00, $e1, $11, $e1, $10, $c2, $23, $c2, $21, $c1, $20, $c2, $23
    db $7f, $c0, $87, $00, $43, $c4, $43, $85, $83, $85, $82, $04, $03, $04, $83, $84
    db $ff, $00, $ff, $3c, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $ff, $3c, $3c, $c3, $00
    db $fe, $03, $e1, $00, $c2, $23, $c2, $a1, $c1, $a1, $41, $20, $c0, $20, $c1, $21
    db $ff, $80, $0f, $00, $87, $88, $87, $08, $43, $c4, $43, $84, $83, $04, $43, $c4
    db $fe, $3f, $fd, $7e, $fb, $fc, $07, $f8, $ff, $00, $ff, $00, $ff, $00, $ff, $00
    db $82, $41, $81, $40, $82, $43, $82, $41, $81, $40, $02, $83, $02, $81, $01, $80
    db $83, $04, $03, $04, $87, $88, $87, $08, $07, $08, $87, $88, $87, $08, $07, $08
    db $c1, $20, $c0, $20, $e1, $11, $e1, $10, $e0, $10, $e1, $11, $e1, $10, $e0, $10
    db $41, $82, $81, $02, $41, $c2, $41, $82, $81, $02, $40, $c1, $40, $81, $80, $01
    db $7f, $fc, $bf, $7e, $df, $3f, $e0, $1f, $ff, $00, $ff, $00, $ff, $00, $ff, $00
    db $ff, $dd, $ff, $77, $ff, $ff, $ff, $73, $ff, $df, $ff, $ff, $ff, $ff, $ff, $ff
    db $ff, $3b, $ff, $ef, $ff, $ff, $ff, $bd, $ff, $ff, $ff, $f7, $ff, $ff, $ff, $ff
    db $d9, $40, $f6, $80, $ff, $64, $ff, $f9, $ff, $df, $ff, $ff, $ff, $ff, $ff, $ff
    db $d7, $03, $2f, $05, $ff, $1f, $ff, $7d, $ff, $ff, $ff, $f7, $ff, $ff, $ff, $ff
    db $fc, $03, $fe, $01, $ff, $01, $7f, $86, $3f, $cd, $1f, $f8, $9f, $7d, $ff, $7f
    db $5f, $60, $cf, $90, $4f, $f0, $8f, $90, $2d, $02, $f8, $b7, $fd, $fe, $fb, $ff
    db $f8, $06, $f9, $04, $f8, $04, $fc, $02, $f8, $07, $f9, $05, $f8, $06, $fc, $02
    db $e3, $04, $43, $04, $87, $08, $8f, $30, $1f, $20, $3f, $40, $3f, $40, $9f, $a0
    db $f0, $0c, $f0, $08, $f8, $04, $fc, $02, $fc, $02, $fe, $01, $fc, $03, $f8, $06
    db $23, $04, $73, $04, $69, $02, $31, $02, $11, $02, $31, $06, $23, $0c, $43, $04
    db $e3, $10, $f1, $08, $e1, $10, $c1, $38, $c2, $30, $c1, $38, $e0, $1c, $f0, $0e
    db $e1, $02, $c3, $0c, $c7, $08, $c7, $08, $cf, $10, $e7, $08, $e3, $0c, $63, $04
    db $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $ff, $00, $00, $00, $00, $00, $ff, $ff, $fe, $ff, $fc, $ff, $c0, $ff, $e0, $ff
    db $ff, $00, $00, $00, $00, $40, $bf, $ff, $2f, $ff, $07, $ff, $00, $ff, $00, $ff
    db $ff, $ff, $ff, $ff, $ff, $fe, $fe, $fe, $fe, $fe, $fe, $fe, $fe, $fe, $fe, $fe
    db $ff, $ff, $ff, $ff, $ff, $ff, $3f, $ff, $07, $ff, $07, $ff, $07, $ff, $c7, $ff
    db $fe, $fe, $fe, $fe, $fe, $fe, $fe, $fc, $fc, $fc, $fc, $fc, $fc, $fd, $fc, $ff
    db $ff, $ff, $ff, $ff, $ff, $ff, $ff, $7f, $7f, $7f, $7f, $7f, $7f, $7f, $7f, $ff
    db $f8, $f8, $ef, $e6, $b7, $92, $51, $46, $50, $43, $54, $41, $56, $42, $57, $42
    db $3f, $3f, $cf, $8f, $d3, $83, $11, $c5, $11, $85, $51, $05, $d1, $85, $d1, $85
    db $ef, $ff, $c7, $ff, $ef, $ff, $83, $ff, $ef, $ff, $ef, $ff, $c7, $ff, $c7, $ff
    db $57, $42, $57, $42, $57, $42, $57, $42, $57, $42, $57, $42, $57, $42, $57, $42
    db $d1, $85, $d1, $85, $d1, $85, $d1, $85, $d1, $85, $d1, $85, $d1, $85, $d1, $85
    db $ff, $ff, $80, $80, $7f, $7f, $7f, $40, $00, $00, $00, $7f, $7f, $40, $7f, $40
    db $00, $00, $ff, $ff, $ff, $00, $00, $00, $00, $ff, $ff, $00, $ff, $00, $00, $00
    db $55, $40, $55, $42, $55, $42, $57, $42, $57, $42, $55, $40, $55, $42, $55, $02
    db $50, $04, $50, $84, $50, $84, $d0, $84, $d0, $85, $50, $04, $50, $84, $50, $84
    db $01, $01, $fe, $1e, $ff, $01, $01, $00, $00, $fe, $fe, $01, $ff, $00, $01, $00
    db $ff, $ff, $03, $03, $fd, $fd, $fd, $05, $01, $01, $01, $fd, $fd, $05, $fd, $05
    db $fb, $fd, $fb, $fa, $fd, $fd, $fe, $fe, $fd, $fc, $fd, $fc, $fd, $fc, $fd, $fc
    db $ff, $ff, $ff, $ff, $ff, $7f, $c3, $83, $6d, $5d, $0d, $b9, $fd, $19, $fd, $19
    db $ff, $ff, $80, $80, $7f, $7f, $55, $6a, $7f, $40, $7f, $40, $55, $6a, $7f, $40
    db $00, $00, $ff, $ff, $55, $aa, $ff, $00, $ff, $00, $55, $aa, $ff, $00, $ff, $00
    db $00, $00, $00, $7f, $7e, $40, $7e, $40, $00, $00, $00, $7e, $7e, $40, $7e, $40
    db $07, $ff, $ff, $f8, $fc, $00, $84, $00, $84, $01, $84, $31, $84, $31, $84, $31
    db $ff, $ff, $ff, $00, $21, $00, $21, $00, $21, $8c, $21, $8c, $21, $8c, $21, $8c
    db $ff, $ff, $ff, $00, $08, $00, $08, $00, $08, $63, $08, $63, $08, $63, $08, $63
    db $c0, $fe, $fe, $3f, $7e, $00, $40, $00, $42, $00, $42, $18, $42, $18, $42, $18
    db $00, $00, $00, $fc, $fc, $04, $fc, $04, $00, $00, $00, $fc, $fc, $04, $fc, $04
    db $01, $01, $fe, $1e, $55, $ab, $ff, $00, $ff, $00, $55, $aa, $ff, $00, $ff, $00
    db $ff, $ff, $03, $03, $fd, $fd, $55, $ad, $fd, $05, $fd, $05, $55, $ad, $fd, $05
    db $ff, $ff, $ff, $ff, $ff, $fe, $c3, $c1, $b6, $ba, $b0, $9d, $bf, $98, $bf, $98
    db $df, $bf, $df, $5f, $bf, $bf, $7f, $7f, $bf, $3f, $bf, $3f, $bf, $3f, $bf, $3f
    db $00, $00, $ff, $00, $ff, $ff, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $00, $00
    db $00, $00, $61, $04, $04, $db, $00, $ff, $00, $ff, $06, $f9, $ff, $06, $ff, $d9
    db $00, $00, $22, $d0, $10, $6d, $00, $ff, $00, $ff, $e3, $1c, $ff, $63, $ff, $3c
    db $df, $2a, $df, $a3, $df, $6c, $9f, $60, $9f, $fb, $9f, $7e, $0f, $fa, $9f, $f0
    db $ff, $a0, $ff, $00, $fe, $0c, $fd, $f0, $fb, $58, $fa, $30, $fa, $c8, $fa, $00
    db $ff, $36, $fc, $c3, $1f, $05, $ef, $0b, $17, $09, $00, $0f, $07, $cb, $07, $45
    db $9f, $f8, $03, $fd, $0f, $fe, $0f, $f5, $9e, $6f, $00, $ff, $0e, $fd, $9e, $f2
    db $ff, $08, $ff, $e7, $87, $80, $7b, $01, $8d, $00, $05, $01, $05, $31, $05, $20
    db $fc, $04, $fe, $93, $ff, $0f, $fc, $60, $f9, $04, $f8, $54, $f0, $e3, $f3, $c0
    db $00, $df, $1f, $72, $27, $79, $00, $64, $03, $e4, $00, $3c, $00, $ff, $f0, $0c
    db $00, $ff, $9f, $6d, $9f, $e2, $00, $60, $0e, $f1, $00, $f2, $00, $fe, $00, $f3
    db $03, $30, $87, $a0, $cf, $6d, $03, $23, $09, $b2, $01, $42, $00, $7c, $fc, $00
    db $e7, $a3, $e7, $03, $c3, $03, $c1, $c2, $83, $04, $83, $04, $30, $0f, $70, $0f
    db $f8, $f4, $fb, $f4, $f0, $ff, $3f, $40, $3f, $40, $ff, $00, $00, $ff, $00, $ff
    db $01, $f2, $0d, $f2, $00, $ff, $ff, $00, $ff, $00, $80, $3f, $00, $3f, $00, $3f
    db $fe, $fc, $fe, $fc, $fc, $fc, $c8, $24, $cc, $22, $00, $fe, $00, $ff, $00, $ff
    db $7f, $20, $7f, $00, $3f, $0c, $3f, $31, $1f, $1c, $1f, $12, $cf, $08, $ef, $00
    db $ff, $04, $ff, $00, $ff, $1d, $ff, $f3, $fe, $a8, $fe, $30, $fe, $03, $fc, $39
    db $00, $1f, $00, $1f, $00, $11, $00, $3f, $00, $31, $00, $3f, $1f, $20, $00, $3f
    db $00, $ff, $00, $ff, $00, $ff, $55, $d5, $55, $d5, $55, $d5, $ff, $00, $00, $ff
    db $00, $3f, $00, $3f, $15, $2a, $00, $2a, $20, $0a, $1f, $61, $1f, $61, $7f, $a1
    db $00, $ff, $00, $ff, $55, $aa, $00, $aa, $00, $aa, $ff, $ff, $00, $ff, $ff, $ff
    db $0f, $84, $0f, $83, $0f, $cf, $0f, $c2, $87, $41, $87, $c7, $87, $ce, $83, $c8
    db $fc, $09, $fc, $e5, $fc, $f9, $fc, $c1, $fc, $15, $fc, $2d, $fc, $3c, $fc, $7c
    db $80, $7f, $80, $3f, $80, $3f, $80, $1f, $80, $40, $c0, $3f, $40, $bf, $60, $9f
    db $aa, $aa, $a8, $a8, $ab, $a8, $a7, $a3, $07, $03, $07, $f3, $07, $f3, $07, $f3
    db $60, $bf, $60, $23, $6c, $21, $6e, $2c, $6e, $0c, $0e, $2c, $0e, $6c, $0e, $6c
    db $00, $ff, $00, $e3, $00, $ff, $00, $ff, $00, $00, $00, $ff, $00, $ff, $00, $ff
    db $13, $e8, $13, $c8, $13, $c8, $13, $09, $13, $2a, $33, $ca, $23, $d3, $63, $93
    db $fc, $2c, $fc, $1c, $fc, $1c, $fc, $7c, $fc, $ac, $fc, $3c, $fe, $0e, $fe, $3e
    db $7f, $80, $bb, $43, $5c, $20, $60, $1f, $2e, $11, $00, $1f, $00, $1f, $00, $1f
    db $f7, $03, $f8, $c0, $1f, $00, $00, $ff, $ff, $00, $00, $ff, $00, $ff, $00, $ff
    db $0e, $6c, $61, $00, $ff, $00, $00, $ff, $7f, $80, $00, $ff, $00, $ff, $00, $ff
    db $ff, $00, $fd, $7c, $83, $00, $00, $ff, $7f, $80, $00, $ff, $00, $ff, $00, $ff
    db $e3, $13, $d3, $23, $a3, $43, $63, $83, $43, $83, $03, $83, $03, $83, $07, $87
    db $fe, $0e, $ff, $af, $ff, $ef, $ff, $cf, $ff, $1f, $ff, $3d, $ff, $3e, $ff, $7f
    db $00, $1f, $00, $1f, $80, $9f, $81, $9f, $c7, $df, $ff, $ff, $ff, $fe, $ff, $df
    db $00, $ff, $00, $ff, $00, $ff, $f0, $ff, $ff, $ff, $ff, $ff, $ff, $7f, $ff, $ca
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $07, $ff, $ff, $ff, $ff, $ff, $ff, $87
    db $00, $ff, $00, $ff, $00, $ff, $78, $ff, $fe, $ff, $ff, $ff, $ff, $1f, $ff, $e8
    db $07, $87, $0f, $8e, $0f, $8f, $1f, $9e, $3f, $bf, $ff, $ff, $ff, $ff, $ff, $5f
    db $ff, $a0, $ff, $00, $ff, $0c, $ff, $f1, $ff, $5c, $ff, $32, $ff, $c8, $ff, $00
    db $ff, $37, $ff, $00, $ff, $65, $ff, $9b, $ff, $31, $ff, $c0, $ff, $00, $ff, $20
    db $ff, $d8, $ff, $b5, $ff, $fe, $ff, $95, $ff, $66, $ff, $00, $ff, $31, $ff, $09
    db $ff, $08, $ff, $e7, $ff, $f8, $ff, $c5, $ff, $32, $ff, $2f, $ff, $ff, $ff, $2a
    db $ff, $04, $ff, $93, $ff, $0f, $ff, $62, $ff, $01, $ff, $57, $ff, $ee, $ff, $c0
    db $ff, $be, $ff, $f1, $ff, $db, $ff, $04, $ff, $80, $ff, $eb, $ff, $5c, $ff, $86
    db $ff, $60, $ff, $c8, $ff, $e6, $ff, $6f, $ff, $fe, $ff, $0a, $ff, $01, $ff, $00
    db $ff, $04, $ff, $00, $ff, $1d, $ff, $f3, $ff, $a8, $ff, $30, $ff, $02, $ff, $38
    db $ff, $34, $ff, $ea, $ff, $22, $ff, $0f, $ff, $d5, $ff, $77, $ff, $8d, $ff, $30
    db $ff, $d2, $ff, $2f, $ff, $cb, $ff, $b0, $ff, $45, $ff, $2b, $ff, $c0, $ff, $26
    db $ff, $0a, $ff, $83, $ff, $6c, $ff, $40, $ff, $bb, $ff, $1e, $ff, $6a, $ff, $f0
    db $ff, $91, $ff, $7b, $ff, $64, $ff, $01, $ff, $06, $ff, $f1, $ff, $48, $ff, $01
    db $ff, $ff, $ff, $ff, $ff, $bd, $ff, $ff, $ff, $db, $ff, $7f, $ff, $f6, $ff, $ff
    db $ff, $ff, $ff, $ff, $ff, $d7, $ff, $ff, $ff, $bd, $ff, $ff, $ff, $6c, $ff, $ff
    db $ff, $66, $ff, $9b, $ff, $7f, $ff, $d2, $ff, $bf, $ff, $eb, $ff, $4d, $ff, $d2
    db $ff, $d3, $ff, $3f, $ff, $d6, $ff, $ff, $ff, $12, $ff, $e7, $ff, $5d, $ff, $0b
    db $00, $00, $ff, $00, $ff, $ff, $c0, $3f, $fe, $01, $e0, $1f, $c0, $3f, $00, $1f
    db $00, $00, $ff, $00, $03, $ff, $03, $fc, $07, $f8, $00, $ff, $00, $ff, $00, $fe
    db $ff, $ff, $ff, $ff, $c0, $ff, $c0, $ff, $c0, $ff, $c0, $ff, $c0, $ff, $c0, $ff
    db $ff, $ff, $ff, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $c0, $ff, $c0, $ff, $c0, $ff, $c0, $ff, $c0, $ff, $c0, $ff, $c0, $ff, $c0, $ff
    db $ff, $ff, $ff, $ff, $03, $ff, $03, $ff, $03, $ff, $03, $ff, $03, $ff, $03, $ff
    db $03, $ff, $03, $ff, $03, $ff, $03, $ff, $03, $ff, $03, $ff, $03, $ff, $03, $ff
    db $00, $e7, $10, $e7, $00, $c3, $3c, $99, $3c, $99, $3c, $99, $00, $c3, $00, $ff
    db $c0, $ff, $c0, $ff, $c6, $f9, $f7, $86, $87, $86, $c0, $f9, $c0, $ff, $c0, $ff
    db $03, $ff, $03, $ff, $63, $9f, $ef, $61, $e1, $61, $03, $9f, $03, $ff, $03, $ff
    db $03, $ff, $01, $ff, $01, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $7e, $6a, $f5, $91, $ef, $ac, $ff, $c9, $f6, $e6, $7e, $f0, $3f, $fc, $0f, $ff
    db $03, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $01, $ff, $01, $ff, $03, $ff
    db $0f, $ff, $3f, $fd, $77, $f0, $ff, $e5, $de, $ca, $fb, $93, $ef, $8a, $df, $51
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $03, $ff
    db $f7, $94, $ef, $2d, $ff, $c3, $7b, $29, $ff, $16, $ed, $a8, $ff, $11, $ff, $ff
    db $ff, $ff, $fb, $28, $f7, $c1, $7e, $14, $fd, $e9, $b7, $22, $6f, $4c, $ef, $69
    db $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $ff, $ff
    db $ff, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff, $00, $ff
    db $de, $52, $5f, $41, $ff, $5a, $fa, $e0, $f7, $42, $ff, $33, $fd, $c8, $bb, $11
    db $fb, $91, $db, $0a, $77, $41, $fe, $14, $ff, $ea, $bb, $22, $77, $50, $df, $49
    db $db, $89, $7e, $22, $7f, $56, $dd, $01, $bb, $12, $ff, $2a, $df, $88, $be, $34
    db $bd, $28, $6f, $0c, $dd, $01, $ff, $aa, $b6, $92, $6f, $04, $fb, $aa, $ff, $55
    db $77, $62, $ff, $53, $ef, $ac, $9f, $09, $73, $02, $ef, $a0, $ff, $65, $ef, $85
    db $bf, $84, $fd, $2d, $eb, $c3, $bb, $09, $77, $16, $de, $8a, $ff, $41, $5f, $0a
    db $bd, $89, $ff, $16, $f7, $b2, $b6, $04, $6e, $60, $ff, $4c, $fd, $19, $eb, $20
    db $ff, $a1, $f6, $12, $6f, $25, $ff, $56, $ee, $20, $dd, $85, $df, $1a, $fe, $52

LevelTitleExtraTiles:
    db $00, $18, $18, $3c, $18, $3c, $04, $1e, $00, $64, $60, $f0, $70, $f8, $00, $70
    db $00, $00, $00, $00, $00, $70, $70, $f8, $60, $f0, $00, $60, $00, $00, $00, $00

NextLevelSpriteTiles:
    db $00, $0f, $0f, $1f, $1f, $3f, $1f, $2f, $0e, $11, $11, $2e, $11, $2e, $01, $1e
    db $00, $f0, $f0, $f8, $f8, $fc, $f8, $f4, $70, $88, $08, $f4, $08, $f4, $00, $f8
    db $0f, $3e, $10, $3f, $17, $3f, $10, $2f, $0f, $1f, $00, $0f, $00, $00, $00, $00
    db $70, $f8, $08, $fc, $e8, $fc, $00, $f8, $f0, $f8, $60, $f0, $00, $78, $00, $00
    db $00, $00, $00, $0f, $0f, $1f, $1f, $3f, $1f, $2f, $0e, $11, $11, $2e, $11, $2e
    db $00, $00, $00, $f0, $f0, $f8, $f8, $fc, $f8, $f4, $70, $88, $08, $f4, $08, $f4
    db $01, $1e, $0f, $1e, $10, $3f, $17, $3f, $00, $1f, $0f, $1f, $06, $0f, $00, $1e
    db $00, $f8, $70, $fc, $08, $fc, $e8, $fc, $08, $f4, $f0, $f8, $00, $f0, $00, $00
    db $00, $00, $00, $1f, $1f, $3f, $3f, $7f, $3f, $5f, $1c, $23, $22, $5d, $22, $5d
    db $00, $00, $00, $e0, $e0, $f0, $f0, $f8, $f0, $e8, $e0, $10, $10, $e8, $10, $e8
    db $02, $1d, $1e, $3d, $20, $7f, $2f, $7f, $20, $5f, $1f, $3f, $0c, $1f, $00, $3c
    db $00, $e0, $e0, $f0, $10, $f8, $d0, $f8, $10, $e8, $e0, $f0, $c0, $e0, $00, $f0
    db $00, $1f, $1f, $3f, $3f, $7f, $3f, $5f, $1c, $23, $22, $5d, $22, $5d, $02, $1d
    db $00, $e0, $e0, $f0, $f0, $f8, $f0, $e8, $e0, $16, $16, $e9, $16, $e9, $0c, $fe
    db $1e, $3d, $20, $7f, $2f, $7f, $20, $5f, $1f, $3f, $0c, $1f, $00, $3c, $00, $00
    db $fc, $fe, $1c, $f2, $d0, $fc, $10, $e8, $e0, $f0, $c0, $e0, $00, $f0, $00, $00

Title_ExtraTilesGfx:
    db $c4, $47, $d8, $df, $e0, $7f, $c0, $7f, $40, $ff, $c0, $7f, $80, $ff, $00, $ff
    db $38, $ff, $44, $ff, $ba, $ff, $a2, $ff, $ba, $ff, $44, $ff, $38, $ff, $00, $ff
    db $c6, $ff, $e6, $ff, $e6, $ff, $d6, $ff, $d6, $ff, $ce, $ff, $ce, $ff, $c6, $ff
    db $c0, $ff, $c0, $ff, $00, $ff, $db, $ff, $dd, $ff, $d9, $ff, $d9, $ff, $d9, $ff
    db $00, $ff, $30, $ff, $78, $ff, $33, $ff, $b6, $ff, $b7, $ff, $b6, $ff, $b3, $ff
    db $00, $ff, $00, $ff, $00, $ff, $cd, $ff, $6e, $ff, $ec, $ff, $0c, $ff, $ec, $ff
    db $01, $ff, $01, $ff, $01, $ff, $8f, $ff, $d9, $ff, $d9, $ff, $d9, $ff, $cf, $ff
    db $80, $ff, $80, $ff, $80, $ff, $9e, $ff, $b3, $ff, $b3, $ff, $b3, $ff, $9e, $ff
    db $00, $ff, $fa, $ff, $23, $ff, $22, $ff, $22, $ff, $22, $ff, $00, $ff, $00, $ff
    db $00, $ff, $20, $ff, $60, $ff, $a0, $ff, $20, $ff, $20, $ff, $00, $ff, $00, $ff
    db $00, $f8, $7c, $84, $7c, $84, $04, $ff, $7f, $84, $78, $88, $7f, $88, $10, $f0
    db $4f, $7f, $00, $07, $00, $0c, $00, $ff, $fe, $01, $1e, $01, $e0, $00, $1f, $00
    db $ff, $ff, $00, $fc, $3f, $40, $40, $80, $00, $ff, $7f, $00, $00, $80, $80, $7f
    db $40, $c0, $3c, $08, $4c, $39, $99, $7e, $7f, $9c, $9f, $10, $00, $60, $7f, $80
    db $00, $00, $00, $80, $00, $f0, $f0, $78, $f8, $1e, $ff, $01, $01, $01, $fe, $00
    db $40, $c0, $3c, $00, $44, $39, $91, $7e, $7f, $9c, $9f, $18, $08, $68, $7f, $88
