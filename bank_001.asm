; Disassembly of "rm.gb"
; This file was created with:
; mgbdis v3.0 - Game Boy ROM disassembler by Matt Currie and contributors.
; https://github.com/mattcurrie/mgbdis

; ROM bank 1. Game A (Battleship) code: ship placement, turns, shots,
; results and level screens, animation scripts, palette fades and the
; joypad read. It also holds the Game B tally and cutscene code (reached
; through Bank1_TallyVectors at $7800), and Game A tilemaps, tile runs
; and sprite layouts.

SECTION "Bank1 code", ROMX, BANK[$1]

; Data and labels that the C code uses.
EXPORT CursorMoveDeltas, GameA_TargetPatternOffsets
EXPORT GameA_ShipNameplateTable, GameA_ShipSpriteTable, GameA_CoordLabelLayout
EXPORT GameA_FleetIconTiles, GameA_DpadToDirection, GameA_ShipCellIdRange
EXPORT GameA_LevelSpritesLevel1, GameA_LevelSpritesLevel2, GameA_LevelSpritesLevel3
EXPORT GameA_GfxUiTiles, DirectionDeltas, SumFleetBytes
EXPORT GameA_ScrollYRowTriggers, GameA_ScrollXColTriggers, GameA_ScrollRowEdgeTiles
EXPORT GameA_ScrollColEdgeTiles, GameA_ShipPartTiles, GameA_RedrawTimerPresets
EXPORT GameA_EnemyAircraftSpriteLayout, GameA_StartText, GameA_ArrangeTextRuns
EXPORT GameA_VsComTextRuns, GameA_VersusTextRuns, GameA_ArrangeFleetWindowTilemap
EXPORT GameA_BoardRowRuns, GameA_BoardSideBorderRuns, GameA_ShotMarkerTiles
EXPORT ClearShadowOAM, VBlank_RunCopyQueue, DrawTilemapCmds, QueueTilemapRowCopy
EXPORT GameA_CutsceneEnemyAircraftLaunched, GameA_ShipBurningLoopAnim
EXPORT GameA_ShipTileCopyEntries, GameA_AircraftDirOffsets, GameA_AircraftDirTiles
EXPORT GameA_AircraftDescentDelays, GameA_EnemyFleetGridPtrs, GameA_TallyAnimSetup
EXPORT GameA_WaitingText, GameA_CpuCursorTiles
EXPORT GameA_AnimSlotFrameTiles, GameA_ResultsBlinkTiles, GameA_CongratsBannerSprite
EXPORT GameA_CongratulationsText, GameA_EnemyCountText, GameA_NoEnemyText
EXPORT GameA_BoardIconRuns, GameA_TurnCounterRuns, GameA_EnemyCountRuns
EXPORT GameA_NoEnemyTextRuns, SoftReset
EXPORT GameA_OptionsMenuTilemap, GameA_VictorySceneTilemap, GameA_VictorySceneAnim
EXPORT GameA_VictorySceneAnimLevel3, GameA_VictoryBgpFadeSteps, GameA_VictorySpriteLayout
EXPORT GameA_FleetEnemyHeaderRuns, GameA_SunkCrossTileMask, GameA_YouLostSpriteLayout
EXPORT GameA_LevelMapRuns, GameA_LevelMapStream, GameA_GfxTiles6050, GameA_GfxTiles7330
EXPORT GameA_GfxBgTiles, GameA_GfxSprites, GameA_GfxShipSinkSprites, NextLevelBgTiles
EXPORT GameB_TallyKillCountPtrTable, GameB_TallyRowSprites, GameB_TallyRowLabels
EXPORT GameB_TallyRowPayloads, GameB_TallyRowNameTiles, GameB_TallyBonusTiles
EXPORT GameB_TallyHeader, GameB_TimeBonusBanner, GameB_TimeBonusRankFloor
EXPORT GameB_CutsceneSceneSteps, GameB_CutsceneAttrFlips, GameB_CutsceneSpriteScript
EXPORT GameB_CutsceneFlashPtrs, GameB_BlinkSpriteXA, GameB_BlinkSpriteXB

; ---------------------------------------------------------------------------
; Shared helpers: animation scripts and palette fades
; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; Glue: asm calls the C functions in bank1_*.c and bank9_*.c.
; ---------------------------------------------------------------------------

; A glue entry is "call GlueC" (or GlueCA, GlueCC) with "db BANK(f)" and
; "dw f" after it. The macros below make them. The glue calls f, then
; returns to the caller of the entry.
MACRO C_GLUE
    call GlueC
    db BANK(\1)
    dw \1
ENDM

MACRO C_GLUE_A
    call GlueCA
    db BANK(\1)
    dw \1
ENDM

MACRO C_GLUE_C
    call GlueCC
    db BANK(\1)
    dw \1
ENDM

; Keeps AF, BC, DE and HL.
GlueC:
    push af
    push bc
    push de
    push hl
    ld hl, sp+8
    call GlueCall
    pop hl
    pop de
    pop bc
    pop af
    add sp, 2
    ret

; Keeps BC, DE and HL. Out: A = the C result, Z set when A = 0.
GlueCA:
    push bc
    push de
    push hl
    ld hl, sp+6
    call GlueCall
    pop hl
    pop de
    pop bc
    add sp, 2
    or a
    ret

; Keeps BC, DE and HL. Out: A = the C result, carry = bit 0 of A.
GlueCC:
    push bc
    push de
    push hl
    ld hl, sp+6
    call GlueCall
    pop hl
    pop de
    pop bc
    add sp, 2
    rrca
    rlca
    ret

; In: HL = the address of the return address of the glue entry. It
; points to "db BANK(f)", "dw f". Calls f in its bank.
GlueCall:
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    ld e, [hl]
    inc hl
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    jp ___sdcc_bcall_ehl

; The entries keep all registers, unless the comment names a result.
UpdateAnimScripts:
    C_GLUE updateAnimScripts

UpdatePaletteFade:
    C_GLUE updatePaletteFade

; Out: carry set when the cursor moved.
GameA_MoveGridCursor:
    C_GLUE_C moveGridCursor

GameA_SelectNextShip:
    C_GLUE selectNextShip

; In: A = direction.
GameA_CalcPlacementBounds:
    push af
    push bc
    push de
    push hl
    push af
    inc sp
    ld e, BANK(calcPlacementBounds)
    ld hl, calcPlacementBounds
    call ___sdcc_bcall_ehl
    inc sp
    pop hl
    pop de
    pop bc
    pop af
    ret

; Out: carry set when the cursor is outside the bounds.
GameA_CheckCursorInBounds:
    C_GLUE_C checkCursorInBounds

GameA_DrawShipSprites:
    C_GLUE drawShipSprites

GameA_SetShipTileSource:
    C_GLUE setShipTileSource

GameA_Place_GetShipSprite:
    C_GLUE placeGetShipSprite

; In: HL = grid.
GameA_WriteShipToGrid:
    push af
    push bc
    push de
    push hl
    push hl
    ld e, BANK(writeShipToGrid)
    ld hl, writeShipToGrid
    call ___sdcc_bcall_ehl
    pop hl
    pop hl
    pop de
    pop bc
    pop af
    ret

; In: D = row, E = column. Out: Z set when a cell of the pattern is not shot yet.
GameA_DrawTargetPattern:
    push bc
    push de
    push hl
    ld a, d
    call drawTargetPattern
    pop hl
    pop de
    pop bc
    or a
    ret

; Out: Z set when all 5 ships are placed.
GameA_CheckAllShipsPlaced:
    C_GLUE_A checkAllShipsPlaced

; In: HL = grid, C = direction. Out: carry set when a cell is used.
GameA_CheckShipFits:
    push bc
    push de
    push hl
    ld a, c
    push af
    inc sp
    push hl
    ld e, BANK(checkShipFits)
    ld hl, checkShipFits
    call ___sdcc_bcall_ehl
    add sp, 3
    pop hl
    pop de
    pop bc
    rrca
    ret

GameA_Place_State0_WaitButton:
    C_GLUE placeState0WaitButton

GameA_Place_State2_TurnShip:
    C_GLUE placeState2TurnShip

GameA_Place_OnB_PickUpShip:
    C_GLUE placeOnBPickUpShip

GameA_ShowTargetCursor:
    C_GLUE showTargetCursor

GameA_DrawCoordLabels:
    C_GLUE drawCoordLabels

GameA_BuildLevelSprites:
    C_GLUE buildLevelSprites

GameA_ResolveShots:
    C_GLUE resolveShots

GameA_Turn06_FireAtTarget:
    C_GLUE turn06FireAtTarget

; In: HL = grid, D = row, E = column.
GameA_CollectTouchingShips:
    ld a, e
    push af
    inc sp
    ld a, d
    push af
    inc sp
    push hl
    ld e, BANK(collectTouchingShips)
    ld hl, collectTouchingShips
    call ___sdcc_bcall_ehl
    add sp, 4
    ret

; In: A = player, D = row, E = column.
; Out: A = the Lucky Shot group (1 or 2), or 0.
GameA_CheckLuckySquare:
    ld b, a
    ld a, d
    swap a
    or e
    push af
    inc sp
    ld a, b
    push af
    inc sp
    ld e, BANK(checkLuckySquare)
    ld hl, checkLuckySquare
    call ___sdcc_bcall_ehl
    add sp, 2
    ret

; In: A = player.
GameA_LoadPlayerFleet:
    push af
    inc sp
    ld e, BANK(loadPlayerFleet)
    ld hl, loadPlayerFleet
    call ___sdcc_bcall_ehl
    inc sp
    ret

; In: HL = 5 bytes. Out: A = the number of nonzero bytes.
CountNonZero5:
    push hl
    ld e, BANK(countNonZero5)
    ld hl, countNonZero5
    call ___sdcc_bcall_ehl
    add sp, 2
    ret

; In: A = direction, D = row, E = column. Out: D, E = the next cell,
; carry set when it is outside the grid. Keeps BC and HL.
StepCellInDirection:
    push hl
    push bc
    push de
    push af
    inc sp
    ld e, BANK(stepCellInDirectionAsm)
    ld hl, stepCellInDirectionAsm
    call ___sdcc_bcall_ehl
    add sp, 3
    ld d, b
    ld e, c
    ldh a, [g_gridSize]
    dec a
    cp d
    jr c, .done
    cp e
.done:
    pop bc
    pop hl
    ret

; Out: D = row, E = column, A = column.
GetCursorCell:
    push bc
    push hl
    call getCursorCell
    ld d, b
    ld e, c
    ld a, c
    pop hl
    pop bc
    ret

; In: A = coordinate. Out: carry set when it is outside the grid.
CheckCoordOutsideGrid:
    push bc
    push de
    push hl
    call checkCoordOutsideGrid
    pop hl
    pop de
    pop bc
    rrca
    ret

GridCellToOamXY:
    push bc
    push hl
    ld a, d
    call gridCellToOamXY
    ld d, b
    ld e, c
    pop hl
    pop bc
    ret

; In: HL = OAM, D = Y, E = X, B = the size of one part.
SetMetaspritePos2x2:
    push af
    push bc
    push de
    push hl
    push bc
    inc sp
    push de
    push hl
    ld e, BANK(setMetaspritePos2x2)
    ld hl, setMetaspritePos2x2
    call ___sdcc_bcall_ehl
    add sp, 5
    pop hl
    pop de
    pop bc
    pop af
    ret

; Out: A = the new random seed. Keeps BC, DE and HL.
StirRandom:
    C_GLUE_A stirRandom

GameA_UpdateScroll:
    jp updateScroll

; In: HL = grid, B, C = two cell ids. Out: D = row, E = column,
; HL = the cell. Keeps BC.
FindGridCellByValue:
    push bc
    push hl
    push bc
    push hl
    ld e, BANK(findGridCellByValue)
    ld hl, findGridCellByValue
    call ___sdcc_bcall_ehl
    add sp, 4
    ld d, b
    ld e, c
    pop hl
    push de
    rst $20
    pop de
    pop bc
    ret

; Out: A = the first tile column and row of the grid.
GameA_GetGridMargin:
    C_GLUE_A getGridMargin

; In: A = cell id, D = row, E = column.
GameA_Sub_4677:
    jp queueShipCellTiles

GameA_Sub_46c2:
    jp nudgeCellSprites

; Out: carry set when the game over started now.
GameA_BeginGameOver:
    C_GLUE_C beginGameOver

GameA_InitBoardScreen:
    C_GLUE initBoardScreen

GameA_State1A_Step4:
    jp state1AStep4

GameA_State1A_Step5:
    C_GLUE state1AStep5

; Game A state $1f (bank 0 jumps here by raw address $4859).
; .loop draws the arrange screen (bank 0 calls it).
GameA_State1F_WaitFade:
    jp state1FWaitFade
.loop:
    jp drawArrangeScreen

GameA_State15_Step2:
    C_GLUE state15Step2

GameA_Board_Step2:
    C_GLUE boardStep2

GameA_State20_FadeOut:
    C_GLUE state20FadeOut

GameA_TickRedrawTimers:
    jp tickRedrawTimers

GameA_BuildGridEdgeStrip:
    jp buildGridEdgeStrip

; In: A = cell id, HL = output. Out: HL + 2. Keeps DE.
GameA_GetShipPartTile:
    push de
    push hl
    ld d, h
    ld e, l
    call getShipPartTile
    pop hl
    inc hl
    inc hl
    pop de
    ret

; In: A = cell id. Out: A = ship index. Keeps DE (bank 0 $3604 uses it).
GameA_GetShipFromCellId:
    push de
    push af
    inc sp
    ld e, BANK(getShipFromCellId)
    ld hl, getShipFromCellId
    call ___sdcc_bcall_ehl
    inc sp
    pop de
    ret

; Out: HL = the salvo shot entry of the next shot. Keeps BC.
GameA_Sub_5023:
    push bc
    call getSalvoShotPtr
    ld h, b
    ld l, c
    pop bc
    ret

; In: DE = grid.
GameA_DrawStreamToBgMap:
    jp drawStreamToBgMap

; In: HL = destination, DE = tile runs.
WriteTileRuns:
    ld b, d
    ld c, e
    ld d, h
    ld e, l
    jp writeTileRuns


GameA_State1A_Step0:
    jp state1AStep0


UpdatePlayClock:
    jp updatePlayClock


ClearAnimScripts:
    CALL_C clearAnimScripts


; Output: HL = $c883 + g_animScriptCount*4, the next free anim script record.
; Keeps BC and DE.
AnimGetNextRecordPtr:
    push bc
    push de
    call animGetNextRecordPtr
    ld h, b
    ld l, c
    pop de
    pop bc
    ret


; Out: HL = the first OAM entry of the next new anim script.
GameA_Sub_503e:
    call animOamPtr
    ld h, b
    ld l, c
    ret

GameA_Turn01_PlayerAim:
    C_GLUE turn01PlayerAim


GameA_State16_Step0:
    C_GLUE state16Step0


GameA_State16_Step1:
    C_GLUE state16Step1


GameA_Turn08_ResetScroll:
    C_GLUE turn08ResetScroll


GameA_ResetStatEffectA:
    C_GLUE resetStatEffectA


GameA_ResetStatEffectB:
    C_GLUE resetStatEffectB


GameA_Turn06_ClearOAM:
    C_GLUE turn06ClearOAM


; In: A = row << 4 | column. Out: D = row, E = column. Keeps BC and HL.
GameA_Sub_52ca:
    push bc
    push hl
    call splitCellByte
    ld d, b
    ld e, c
    pop hl
    pop bc
    ret


GameA_State1A_Step2:
    C_GLUE state1AStep2


GameA_State1A_Step3:
    jp state1AStep3


GameA_LevelTitle_Step1:
    C_GLUE levelTitleStep1


GameA_Results_Step1:
    jp resultsStep1


GameA_Results_Step2_Reset:
    jp resultsStep2Reset


GameA_Sub_5563:
    jp blinkEnemyCount


GameA_State02_Init:
    C_GLUE state02Init


GameA_State08:
    jp state08


GameA_Turn08_AltHandler:
    jp turn08AltHandler


GameA_Cpu10_SetDelay:
    C_GLUE cpu10SetDelay


; In: A = ship, DE = the table of 5-byte ship records.
GameA_LoadShipRecord:
    jp loadShipRecord

; C: moveScrollSprites(dy, dx). In: A = dy, E = dx. Moves the 18 sprites
; at $c058. This stays asm, because it runs in the VBlank and the C loop
; makes the OBP1 palette write after it land 2 lines late (visible tearing).
moveScrollSprites::
    ld d, a
    ld c, $12
    ld hl, g_oamBuffer + $58

.loop:
    ld a, [hl]
    add d
    ld [hl+], a
    ld a, [hl]
    add e
    ld [hl+], a
    inc hl
    inc hl
    dec c
    jr nz, .loop

    ret

; Out: A = 1 when GameA_RotateShipTileStep copied the last tile (carry).
rotateShipTileStep::
    call GameA_RotateShipTileStep
    ld a, 0
    rla
    ret


GameA_Sub_57bb:
    C_GLUE storeCpuShipDirs


GameA_RunCutsceneScript:
    jp runCutsceneScript


; In: DE = the table of 5-byte ship records.
GameA_Sub_59d2:
    jp drawShipRecords


; In: A = cell, B = margin. Out: A = scroll steps. Keeps DE.
GameA_Sub_5a00:
    push de
    ld e, b
    call scrollStepsTo
    pop de
    ret


GameA_DrawMenuTilemap:
    jp drawMenuTilemap


; Game A state $14 (bank 0 table, raw $5acf). Waits until the last row was copied ($c4bf = 0), then draws the next row (row count $ffb1).
GameA_State14:
    jp state14


GameA_State15_Step1:
    jp state15Step1


GameA_Sub_5bf1:
    jp drawVictoryScene


; Game A state $10 (bank 0 table, raw $5cda). Sets $ffad bit 6, then runs the state.
GameA_State10:
    C_GLUE state10

; Game B tally and cutscene steps in bank9_tally.c (jumped to from
; Bank1_TallyVectors).
GameB_TallyState4_AccumulateScore:
    C_GLUE tallyState4AccumulateScore
GameB_TallyState5_WaitInput:
    C_GLUE tallyState5WaitInput
GameB_TallyState6_SelectFanfare:
    C_GLUE tallyState6SelectFanfare
GameB_TallyFrameRefresh_CopyDigitsToTiles:
    C_GLUE tallyCopyDigitsToTiles


GameA_Cpu0B:
    C_GLUE cpu0B


GameA_BlinkLuckyMarkers:
    C_GLUE blinkLuckyMarkers


; Game A state $1d (bank 0 table, raw $5e1e). Slides the window down 6 lines a frame and counts down $ffb1 with SFX $0f.
GameA_State1D_SlideWindow:
    C_GLUE state1DSlideWindow


GameA_State1A_Step1:
    jp state1AStep1


; Game A state $19 (bank 0 table, raw $5fd9). Waits while a VRAM update is queued ($ffab bit 2).
GameA_State19:
    jp state19


GameA_State04_Delay:
    C_GLUE state04Delay


GameA_Turn02:
    C_GLUE turn02


GameA_Turn03:
    C_GLUE turn03


GameA_Turn04_ClearOAM:
    C_GLUE turn04ClearOAM


GameA_Cpu12_SetDelay:
    C_GLUE cpu12SetDelay


GameA_Board_Step1:
    jp boardStep1


; Game A state $11 (bank 0 table, raw $62c3). Waits until the last row was copied ($c4bf = 0), then draws the next row (row count $ffb1).
GameA_State11:
    jp state11


; Called from VBlank in bank 0.
ReadJoypad:
    jp readJoypad

CopyOamDmaToHram:
    CALL_C copyOamDmaToHram

; Copied to $ff80 by copyOamDmaToHram. Starts OAM DMA from $c000 and waits for it to end.
; This stays asm, because it runs from HRAM.
OamDmaRoutine::
    ld a, $c0
    ldh [rDMA], a
    ld a, $28

.loop:
    dec a
    jr nz, .loop

    ret


; Bank 0, bank 5 and the code use raw addresses of this data, so it stays
; at its original address.
SECTION "Bank1 data A", ROMX[$63a9], BANK[$1]

; ---------------------------------------------------------------------------
; Game A data: anim scripts, sprite layouts, tilemaps and tables
; ---------------------------------------------------------------------------

; Sprite layouts for bank 0 $1e7a (BuildSpriteRuns). Each run is 4 bytes:
; y, x, count (bits 6-7 = direction), first tile.
GameA_PlacementSpriteLayout:
    db $5f, $88, $01, $63, $50, $88, $03, $f5, $f1, $62, $68, $88, $03, $fd, $eb, $f8
    db $78, $88, $02, $f4, $f5, $14, $20, $40, $20, $90, $10, $60, $41, $60, $91, $0c
    db $60, $42, $60, $92, $0c, $20, $43, $20, $93, $08, $e0, $43, $e0, $93, $04, $60
    db $44, $60, $94, $04, $90, $5c, $00, $87, $40, $88, $03, $6e, $6f, $fe, $90, $70
    db $82, $86, $90

; Sprite animation scripts for UpdateAnimScripts. One command per line.
GameA_ShotFlightAnim:
    db $fd, $02, $5f                ; sprite 2 tile = $5f
    db $fe, $06                     ; wait 6
    db $fd, $02, $60                ; sprite 2 tile = $60
    db $fa, $06                     ; restart, wait 6

GameA_ShotExplosionFlashAnim:
    db $f7, $32                     ; rBGP = $32
    db $fd, $00, $00                ; sprite 0 tile = $00
    db $fd, $01, $0f                ; sprite 1 tile = $0f
    db $fd, $02, $10                ; sprite 2 tile = $10
    db $fd, $03, $11                ; sprite 3 tile = $11
    db $fe, $07                     ; wait 7
    db $fb, $01                     ; $deb8 sound = $01
    db $fd, $00, $12                ; sprite 0 tile = $12
    db $fd, $01, $13                ; sprite 1 tile = $13
    db $fd, $02, $14                ; sprite 2 tile = $14
    db $fd, $03, $15                ; sprite 3 tile = $15
    db $fe, $05                     ; wait 5
    db $fb, $04                     ; $deb8 sound = $04
    db $fe, $02                     ; wait 2
    db $fd, $00, $16                ; sprite 0 tile = $16
    db $fd, $01, $17                ; sprite 1 tile = $17
    db $fd, $02, $18                ; sprite 2 tile = $18
    db $fd, $03, $19                ; sprite 3 tile = $19
    db $fe, $03                     ; wait 3
    db $fb, $02                     ; $deb8 sound = $02
    db $fe, $04                     ; wait 4
    db $fd, $00, $1a                ; sprite 0 tile = $1a
    db $fd, $01, $1b                ; sprite 1 tile = $1b
    db $fd, $02, $1c                ; sprite 2 tile = $1c
    db $fd, $03, $1d                ; sprite 3 tile = $1d
    db $fe, $01                     ; wait 1
    db $fb, $03                     ; $deb8 sound = $03
    db $fe, $05                     ; wait 5
    db $fb, $04                     ; $deb8 sound = $04
    db $fe, $01                     ; wait 1
    db $fd, $00, $1e                ; sprite 0 tile = $1e
    db $fd, $01, $1f                ; sprite 1 tile = $1f
    db $fd, $02, $20                ; sprite 2 tile = $20
    db $fd, $03, $21                ; sprite 3 tile = $21
    db $fe, $04                     ; wait 4
    db $fb, $01                     ; $deb8 sound = $01
    db $f7, $36                     ; rBGP = $36
    db $fe, $03                     ; wait 3
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $ff                          ; end

GameA_ShotExplosionAnim:
    db $fd, $00, $00                ; sprite 0 tile = $00
    db $fd, $01, $0f                ; sprite 1 tile = $0f
    db $fd, $02, $10                ; sprite 2 tile = $10
    db $fd, $03, $11                ; sprite 3 tile = $11
    db $fe, $07                     ; wait 7
    db $fb, $01                     ; $deb8 sound = $01
    db $fd, $00, $12                ; sprite 0 tile = $12
    db $fd, $01, $13                ; sprite 1 tile = $13
    db $fd, $02, $14                ; sprite 2 tile = $14
    db $fd, $03, $15                ; sprite 3 tile = $15
    db $fe, $07                     ; wait 7
    db $fd, $00, $16                ; sprite 0 tile = $16
    db $fd, $01, $17                ; sprite 1 tile = $17
    db $fd, $02, $18                ; sprite 2 tile = $18
    db $fd, $03, $19                ; sprite 3 tile = $19
    db $fe, $07                     ; wait 7
    db $fd, $00, $1a                ; sprite 0 tile = $1a
    db $fd, $01, $1b                ; sprite 1 tile = $1b
    db $fd, $02, $1c                ; sprite 2 tile = $1c
    db $fd, $03, $1d                ; sprite 3 tile = $1d
    db $fe, $07                     ; wait 7
    db $fd, $00, $1e                ; sprite 0 tile = $1e
    db $fd, $01, $1f                ; sprite 1 tile = $1f
    db $fd, $02, $20                ; sprite 2 tile = $20
    db $fd, $03, $21                ; sprite 3 tile = $21
    db $fe, $07                     ; wait 7
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $ff                          ; end

GameA_ShotSplashAnim:
    db $fd, $00, $00                ; sprite 0 tile = $00
    db $fd, $01, $00                ; sprite 1 tile = $00
    db $fd, $02, $22                ; sprite 2 tile = $22
    db $fd, $03, $23                ; sprite 3 tile = $23
    db $fe, $07                     ; wait 7
    db $fb, $04                     ; $deb8 sound = $04
    db $fd, $00, $24                ; sprite 0 tile = $24
    db $fd, $01, $25                ; sprite 1 tile = $25
    db $fd, $02, $26                ; sprite 2 tile = $26
    db $fd, $03, $27                ; sprite 3 tile = $27
    db $fe, $07                     ; wait 7
    db $fd, $00, $28                ; sprite 0 tile = $28
    db $fd, $01, $29                ; sprite 1 tile = $29
    db $fd, $02, $2a                ; sprite 2 tile = $2a
    db $fd, $03, $2b                ; sprite 3 tile = $2b
    db $fe, $07                     ; wait 7
    db $fd, $00, $2c                ; sprite 0 tile = $2c
    db $fd, $01, $2d                ; sprite 1 tile = $2d
    db $fd, $02, $2e                ; sprite 2 tile = $2e
    db $fd, $03, $2f                ; sprite 3 tile = $2f
    db $fe, $07                     ; wait 7
    db $fd, $00, $30                ; sprite 0 tile = $30
    db $fd, $01, $31                ; sprite 1 tile = $31
    db $fd, $02, $32                ; sprite 2 tile = $32
    db $fd, $03, $33                ; sprite 3 tile = $33
    db $fe, $07                     ; wait 7
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $ff                          ; end

GameA_ShotSplashAnimLevel3:
    db $fd, $00, $00                ; sprite 0 tile = $00
    db $fd, $01, $00                ; sprite 1 tile = $00
    db $fd, $02, $22                ; sprite 2 tile = $22
    db $fd, $03, $23                ; sprite 3 tile = $23
    db $fe, $07                     ; wait 7
    db $fb, $02                     ; $deb8 sound = $02
    db $fd, $00, $24                ; sprite 0 tile = $24
    db $fd, $01, $25                ; sprite 1 tile = $25
    db $fd, $02, $26                ; sprite 2 tile = $26
    db $fd, $03, $27                ; sprite 3 tile = $27
    db $fe, $07                     ; wait 7
    db $fd, $00, $28                ; sprite 0 tile = $28
    db $fd, $01, $29                ; sprite 1 tile = $29
    db $fd, $02, $2a                ; sprite 2 tile = $2a
    db $fd, $03, $2b                ; sprite 3 tile = $2b
    db $fe, $07                     ; wait 7
    db $fd, $00, $2c                ; sprite 0 tile = $2c
    db $fd, $01, $2d                ; sprite 1 tile = $2d
    db $fd, $02, $2e                ; sprite 2 tile = $2e
    db $fd, $03, $2f                ; sprite 3 tile = $2f
    db $fe, $07                     ; wait 7
    db $fd, $00, $30                ; sprite 0 tile = $30
    db $fd, $01, $31                ; sprite 1 tile = $31
    db $fd, $02, $32                ; sprite 2 tile = $32
    db $fd, $03, $33                ; sprite 3 tile = $33
    db $fe, $07                     ; wait 7
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $ff                          ; end

GameA_ShipBurningLoopAnim:
    db $fb, $06                     ; $deb8 sound = $06
    db $fd, $01, $03                ; sprite 1 tile = $03
    db $fd, $02, $04                ; sprite 2 tile = $04
    db $fd, $03, $05                ; sprite 3 tile = $05
    db $fe, $08                     ; wait 8
    db $fd, $01, $06                ; sprite 1 tile = $06
    db $fd, $02, $07                ; sprite 2 tile = $07
    db $fd, $03, $08                ; sprite 3 tile = $08
    db $fe, $08                     ; wait 8
    db $fd, $01, $09                ; sprite 1 tile = $09
    db $fd, $02, $0a                ; sprite 2 tile = $0a
    db $fd, $03, $0b                ; sprite 3 tile = $0b
    db $fe, $08                     ; wait 8
    db $fd, $01, $0c                ; sprite 1 tile = $0c
    db $fd, $02, $0d                ; sprite 2 tile = $0d
    db $fd, $03, $0e                ; sprite 3 tile = $0e
    db $fa, $08                     ; restart, wait 8
GameA_ShotAnimSpriteStrip:
    db $28, $98, $02, $64, $65

GameA_TallyAnimA:
    db $fb, $01                     ; $deb8 sound = $01
    db $fd, $00, $34                ; sprite 0 tile = $34
    db $fd, $01, $35                ; sprite 1 tile = $35
    db $fd, $02, $36                ; sprite 2 tile = $36
    db $fd, $03, $37                ; sprite 3 tile = $37
    db $fe, $07                     ; wait 7
    db $fd, $00, $38                ; sprite 0 tile = $38
    db $fd, $01, $39                ; sprite 1 tile = $39
    db $fd, $02, $3a                ; sprite 2 tile = $3a
    db $fd, $03, $3b                ; sprite 3 tile = $3b
    db $fe, $07                     ; wait 7
    db $fd, $00, $3c                ; sprite 0 tile = $3c
    db $fd, $01, $3d                ; sprite 1 tile = $3d
    db $fd, $02, $3e                ; sprite 2 tile = $3e
    db $fd, $03, $3f                ; sprite 3 tile = $3f
    db $fe, $07                     ; wait 7
    db $fd, $00, $40                ; sprite 0 tile = $40
    db $fd, $01, $41                ; sprite 1 tile = $41
    db $fd, $02, $42                ; sprite 2 tile = $42
    db $fd, $03, $43                ; sprite 3 tile = $43
    db $fe, $07                     ; wait 7
    db $fd, $00, $44                ; sprite 0 tile = $44
    db $fd, $01, $45                ; sprite 1 tile = $45
    db $fd, $02, $46                ; sprite 2 tile = $46
    db $fd, $03, $47                ; sprite 3 tile = $47
    db $fe, $07                     ; wait 7
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $ff                          ; end

GameA_TallyAnimB:
    db $f8, $13                     ; sound effect $13
    db $fd, $00, $48                ; sprite 0 tile = $48
    db $fd, $01, $49                ; sprite 1 tile = $49
    db $fd, $02, $4a                ; sprite 2 tile = $4a
    db $fd, $03, $4b                ; sprite 3 tile = $4b
    db $fe, $07                     ; wait 7
    db $fd, $00, $4c                ; sprite 0 tile = $4c
    db $fd, $01, $4d                ; sprite 1 tile = $4d
    db $fd, $02, $4e                ; sprite 2 tile = $4e
    db $fd, $03, $4f                ; sprite 3 tile = $4f
    db $fe, $07                     ; wait 7
    db $fd, $00, $50                ; sprite 0 tile = $50
    db $fd, $01, $51                ; sprite 1 tile = $51
    db $fd, $02, $52                ; sprite 2 tile = $52
    db $fd, $03, $53                ; sprite 3 tile = $53
    db $fe, $07                     ; wait 7
    db $fd, $00, $54                ; sprite 0 tile = $54
    db $fd, $01, $55                ; sprite 1 tile = $55
    db $fd, $02, $56                ; sprite 2 tile = $56
    db $fd, $03, $57                ; sprite 3 tile = $57
    db $fe, $07                     ; wait 7
    db $fd, $00, $58                ; sprite 0 tile = $58
    db $fd, $01, $59                ; sprite 1 tile = $59
    db $fd, $02, $5a                ; sprite 2 tile = $5a
    db $fd, $03, $5b                ; sprite 3 tile = $5b
    db $fe, $07                     ; wait 7
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $ff                          ; end

GameA_TallyAnimC:
    db $fb, $04                     ; $deb8 sound = $04
    db $fd, $00, $48                ; sprite 0 tile = $48
    db $fd, $01, $49                ; sprite 1 tile = $49
    db $fd, $02, $4a                ; sprite 2 tile = $4a
    db $fd, $03, $4b                ; sprite 3 tile = $4b
    db $fe, $07                     ; wait 7
    db $fd, $00, $4c                ; sprite 0 tile = $4c
    db $fd, $01, $4d                ; sprite 1 tile = $4d
    db $fd, $02, $4e                ; sprite 2 tile = $4e
    db $fd, $03, $4f                ; sprite 3 tile = $4f
    db $fe, $07                     ; wait 7
    db $fd, $00, $50                ; sprite 0 tile = $50
    db $fd, $01, $51                ; sprite 1 tile = $51
    db $fd, $02, $52                ; sprite 2 tile = $52
    db $fd, $03, $53                ; sprite 3 tile = $53
    db $fe, $07                     ; wait 7
    db $fd, $00, $54                ; sprite 0 tile = $54
    db $fd, $01, $55                ; sprite 1 tile = $55
    db $fd, $02, $56                ; sprite 2 tile = $56
    db $fd, $03, $57                ; sprite 3 tile = $57
    db $fe, $07                     ; wait 7
    db $fd, $00, $58                ; sprite 0 tile = $58
    db $fd, $01, $59                ; sprite 1 tile = $59
    db $fd, $02, $5a                ; sprite 2 tile = $5a
    db $fd, $03, $5b                ; sprite 3 tile = $5b
    db $fe, $07                     ; wait 7
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $ff                          ; end
    db $48, $5d, $01, $5c

; Tilemap streams for bank 0 $1fa1 / rst $28:
; BG address (high, low), byte count, tiles. A $00 address byte ends the stream.
GameA_VictorySceneTilemap:
    db $98, $00, $54, $92, $9a, $20, $54, $92, $98, $20, $d0, $92, $98, $33, $d0, $92
    db $98, $21, $01, $d0, $98, $32, $01, $d2, $9a, $01, $01, $d5, $9a, $12, $01, $d7
    db $98, $22, $50, $d1, $9a, $02, $50, $d6, $98, $41, $ce, $d3, $98, $52, $ce, $d4
    db $99, $02, $01, $a1, $99, $ea, $01, $a2, $98, $63, $cc, $9f, $99, $c4, $4d, $a0
    db $00

GameA_VictorySceneAnim:
    db $fc, $05, $48, $50, $01, $00 ; sprite 5: y=$48 x=$50 tile=$01 attr=$00
    db $fc, $08, $50, $40, $02, $00 ; sprite 8: y=$50 x=$40 tile=$02 attr=$00
    db $fc, $09, $50, $48, $03, $00 ; sprite 9: y=$50 x=$48 tile=$03 attr=$00
    db $fc, $0a, $50, $50, $04, $00 ; sprite 10: y=$50 x=$50 tile=$04 attr=$00
    db $fc, $0b, $50, $58, $05, $00 ; sprite 11: y=$50 x=$58 tile=$05 attr=$00
    db $fc, $0c, $50, $60, $06, $00 ; sprite 12: y=$50 x=$60 tile=$06 attr=$00
    db $fe, $a0                     ; wait 160
    db $fb, $01                     ; $deb8 sound = $01
    db $fc, $04, $48, $48, $07, $00 ; sprite 4: y=$48 x=$48 tile=$07 attr=$00
    db $fd, $05, $08                ; sprite 5 tile = $08
    db $fd, $09, $09                ; sprite 9 tile = $09
    db $fd, $0a, $0a                ; sprite 10 tile = $0a
    db $fd, $0b, $0b                ; sprite 11 tile = $0b
    db $fc, $0e, $58, $48, $59, $00 ; sprite 14: y=$58 x=$48 tile=$59 attr=$00
    db $fc, $0f, $58, $50, $5a, $00 ; sprite 15: y=$58 x=$50 tile=$5a attr=$00
    db $fc, $10, $58, $58, $5b, $00 ; sprite 16: y=$58 x=$58 tile=$5b attr=$00
    db $fe, $0b                     ; wait 11
    db $fd, $04, $0c                ; sprite 4 tile = $0c
    db $fd, $05, $0d                ; sprite 5 tile = $0d
    db $fc, $06, $48, $58, $0e, $00 ; sprite 6: y=$48 x=$58 tile=$0e attr=$00
    db $fd, $09, $0f                ; sprite 9 tile = $0f
    db $fd, $0a, $10                ; sprite 10 tile = $10
    db $fd, $0b, $11                ; sprite 11 tile = $11
    db $fd, $0e, $5c                ; sprite 14 tile = $5c
    db $fd, $0f, $5d                ; sprite 15 tile = $5d
    db $fd, $10, $5e                ; sprite 16 tile = $5e
    db $fe, $0b                     ; wait 11
    db $fc, $01, $40, $50, $12, $00 ; sprite 1: y=$40 x=$50 tile=$12 attr=$00
    db $fd, $04, $13                ; sprite 4 tile = $13
    db $fd, $05, $14                ; sprite 5 tile = $14
    db $fd, $06, $15                ; sprite 6 tile = $15
    db $fd, $09, $16                ; sprite 9 tile = $16
    db $fd, $0a, $17                ; sprite 10 tile = $17
    db $fd, $0b, $18                ; sprite 11 tile = $18
    db $fc, $0d, $58, $40, $5f, $00 ; sprite 13: y=$58 x=$40 tile=$5f attr=$00
    db $fd, $0e, $60                ; sprite 14 tile = $60
    db $fd, $0f, $61                ; sprite 15 tile = $61
    db $fd, $10, $62                ; sprite 16 tile = $62
    db $fc, $11, $58, $60, $63, $00 ; sprite 17: y=$58 x=$60 tile=$63 attr=$00
    db $fe, $0b                     ; wait 11
    db $fc, $00, $40, $48, $19, $00 ; sprite 0: y=$40 x=$48 tile=$19 attr=$00
    db $fd, $01, $1a                ; sprite 1 tile = $1a
    db $fc, $02, $40, $58, $1b, $00 ; sprite 2: y=$40 x=$58 tile=$1b attr=$00
    db $fd, $04, $1c                ; sprite 4 tile = $1c
    db $fd, $05, $1d                ; sprite 5 tile = $1d
    db $fd, $06, $1e                ; sprite 6 tile = $1e
    db $fc, $07, $48, $60, $1f, $00 ; sprite 7: y=$48 x=$60 tile=$1f attr=$00
    db $fd, $08, $20                ; sprite 8 tile = $20
    db $fd, $09, $21                ; sprite 9 tile = $21
    db $fd, $0a, $22                ; sprite 10 tile = $22
    db $fd, $0b, $23                ; sprite 11 tile = $23
    db $fd, $0c, $24                ; sprite 12 tile = $24
    db $fe, $0d                     ; wait 13
    db $fd, $00, $25                ; sprite 0 tile = $25
    db $fd, $01, $26                ; sprite 1 tile = $26
    db $fd, $02, $27                ; sprite 2 tile = $27
    db $fc, $03, $48, $40, $28, $00 ; sprite 3: y=$48 x=$40 tile=$28 attr=$00
    db $fd, $04, $29                ; sprite 4 tile = $29
    db $fd, $05, $2a                ; sprite 5 tile = $2a
    db $fd, $06, $2b                ; sprite 6 tile = $2b
    db $fd, $07, $2c                ; sprite 7 tile = $2c
    db $fd, $08, $2d                ; sprite 8 tile = $2d
    db $fd, $09, $2e                ; sprite 9 tile = $2e
    db $fd, $0a, $2f                ; sprite 10 tile = $2f
    db $fd, $0b, $30                ; sprite 11 tile = $30
    db $fd, $0c, $31                ; sprite 12 tile = $31
    db $fe, $10                     ; wait 16
    db $fd, $00, $32                ; sprite 0 tile = $32
    db $fd, $01, $33                ; sprite 1 tile = $33
    db $fd, $02, $34                ; sprite 2 tile = $34
    db $fd, $03, $35                ; sprite 3 tile = $35
    db $fd, $04, $36                ; sprite 4 tile = $36
    db $fd, $05, $37                ; sprite 5 tile = $37
    db $fd, $06, $38                ; sprite 6 tile = $38
    db $fd, $07, $39                ; sprite 7 tile = $39
    db $fd, $08, $3a                ; sprite 8 tile = $3a
    db $fd, $09, $3b                ; sprite 9 tile = $3b
    db $fd, $0a, $3c                ; sprite 10 tile = $3c
    db $fd, $0b, $3d                ; sprite 11 tile = $3d
    db $fd, $0c, $3e                ; sprite 12 tile = $3e
    db $fe, $10                     ; wait 16
    db $f9, $00                     ; hide sprite 0
    db $fd, $01, $3f                ; sprite 1 tile = $3f
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $fd, $04, $40                ; sprite 4 tile = $40
    db $fd, $05, $41                ; sprite 5 tile = $41
    db $fd, $06, $42                ; sprite 6 tile = $42
    db $f9, $07                     ; hide sprite 7
    db $fd, $08, $43                ; sprite 8 tile = $43
    db $fd, $09, $44                ; sprite 9 tile = $44
    db $fd, $0a, $45                ; sprite 10 tile = $45
    db $fd, $0b, $46                ; sprite 11 tile = $46
    db $fd, $0c, $47                ; sprite 12 tile = $47
    db $fe, $10                     ; wait 16
    db $f9, $01                     ; hide sprite 1
    db $fd, $04, $48                ; sprite 4 tile = $48
    db $fd, $05, $49                ; sprite 5 tile = $49
    db $fd, $06, $4a                ; sprite 6 tile = $4a
    db $fd, $08, $4b                ; sprite 8 tile = $4b
    db $fd, $09, $4c                ; sprite 9 tile = $4c
    db $fd, $0a, $4d                ; sprite 10 tile = $4d
    db $fd, $0b, $4e                ; sprite 11 tile = $4e
    db $fd, $0c, $4f                ; sprite 12 tile = $4f
    db $f9, $0d                     ; hide sprite 13
    db $fd, $0e, $5c                ; sprite 14 tile = $5c
    db $fd, $0f, $5d                ; sprite 15 tile = $5d
    db $fd, $10, $5e                ; sprite 16 tile = $5e
    db $f9, $11                     ; hide sprite 17
    db $fe, $10                     ; wait 16
    db $fc, $04, $48, $4a, $50, $00 ; sprite 4: y=$48 x=$4a tile=$50 attr=$00
    db $fc, $05, $48, $52, $51, $00 ; sprite 5: y=$48 x=$52 tile=$51 attr=$00
    db $f9, $06                     ; hide sprite 6
    db $f9, $08                     ; hide sprite 8
    db $fc, $09, $50, $4a, $52, $00 ; sprite 9: y=$50 x=$4a tile=$52 attr=$00
    db $fc, $0a, $50, $52, $53, $00 ; sprite 10: y=$50 x=$52 tile=$53 attr=$00
    db $fc, $0b, $50, $5a, $54, $00 ; sprite 11: y=$50 x=$5a tile=$54 attr=$00
    db $f9, $0c                     ; hide sprite 12
    db $f9, $0d                     ; hide sprite 13
    db $fd, $0e, $59                ; sprite 14 tile = $59
    db $fd, $0f, $5a                ; sprite 15 tile = $5a
    db $fd, $10, $5b                ; sprite 16 tile = $5b
    db $fe, $11                     ; wait 17
    db $f9, $04                     ; hide sprite 4
    db $f9, $05                     ; hide sprite 5
    db $fc, $09, $50, $4b, $55, $00 ; sprite 9: y=$50 x=$4b tile=$55 attr=$00
    db $fc, $0a, $50, $53, $56, $00 ; sprite 10: y=$50 x=$53 tile=$56 attr=$00
    db $f9, $0b                     ; hide sprite 11
    db $fe, $13                     ; wait 19
    db $fc, $09, $50, $4c, $57, $00 ; sprite 9: y=$50 x=$4c tile=$57 attr=$00
    db $fc, $0a, $50, $54, $58, $00 ; sprite 10: y=$50 x=$54 tile=$58 attr=$00
    db $fe, $15                     ; wait 21
    db $f9, $09                     ; hide sprite 9
    db $f9, $0a                     ; hide sprite 10
    db $f9, $0e                     ; hide sprite 14
    db $f9, $0f                     ; hide sprite 15
    db $f9, $10                     ; hide sprite 16
    db $fe, $40                     ; wait 64
    db $ff                          ; end

GameA_VictorySpriteLayout:
    db $58, $a8, $4a, $00, $58, $38, $08, $fb, $ef, $e9, $f9, $f5, $f7, $fd, $fe, $05
    db $b0, $4b, $b0, $8b, $04, $00, $4c, $00, $8c, $03, $40, $4c, $40, $8c, $03, $70
    db $4c, $70, $8c, $02, $a0, $4c, $a0, $8c, $01, $c0, $4c, $c0, $8c, $05, $b0, $4b
    db $b0, $8b, $03, $70, $4c, $00, $8c, $03, $70, $4c, $30, $8c, $03, $70, $4c, $60
    db $8c, $01, $00, $40, $a0, $8c, $01, $c0, $4c, $90, $8c, $07, $b0, $7e, $b0, $8b
    db $02, $20, $7f, $20, $8c, $02, $40, $7f, $40, $8c, $02, $60, $7f, $60, $8c, $01
    db $b0, $7e, $90, $8c, $01, $c0, $4c, $80, $8c, $9a, $22, $08, $f9, $fa, $f7, $f4
    db $ff

GameA_EnemyAircraftSpriteLayout:
    db $a0, $54, $c2, $03, $b0, $4c, $43, $05, $b8, $4c, $43, $08, $c0, $4c, $43, $0b
    db $c8, $34, $49, $0e, $d0, $34, $49, $17, $d8, $44, $45, $20, $e0, $44, $45, $25

; Cutscene script: "ENEMY AIRCRAFT LAUNCHED". GameA_RunCutsceneScript runs it.
GameA_CutsceneEnemyAircraftLaunched:
    db $01, $38, $44, $eb, $f4, $eb, $f3, $fd, $03, $10, $01, $48, $38, $e7, $ef, $f7
    db $e9, $f7, $e7, $ec, $f9, $03, $10, $01, $58, $34, $f2, $e7, $fa, $f4, $e9, $ee
    db $eb, $ea, $fe, $03, $40, $00

GameA_OptionsMenuTilemap:
    db $98, $01, $52, $d1, $98, $02, $04, $4d, $4e, $4f, $50, $98, $0e, $04, $51, $52
    db $53, $54, $98, $41, $52, $4b, $98, $81, $52, $4b, $98, $e1, $52, $4b, $99, $41
    db $52, $4b, $99, $e1, $52, $4b, $9a, $21, $52, $d6, $98, $00, $8b, $d0, $d3, $4a
    db $d3, $4a, $d3, $d3, $4a, $d3, $d3, $4a, $99, $60, $c4, $d3, $99, $e0, $83, $4a
    db $d3, $d5, $98, $13, $8b, $d2, $d4, $4c, $d4, $4c, $d4, $d4, $4c, $d4, $d4, $4c
    db $99, $73, $c4, $d4, $99, $f3, $83, $4c, $d4, $d7, $98, $21, $03, $e0, $49, $e0
    db $98, $28, $04, $ed, $f7, $ef, $ea, $98, $30, $03, $e0, $49, $e0, $98, $62, $10
    db $f5, $f4, $00, $00, $e7, $ef, $f7, $e9, $f7, $e7, $ec, $f9, $00, $00, $f5, $f4
    db $98, $a2, $08, $f5, $f4, $00, $00, $f4, $eb, $e7, $f7, $98, $b0, $02, $f5, $f4
    db $98, $ca, $04, $f3, $ef, $f8, $f8, $99, $02, $09, $f5, $f4, $00, $00, $f2, $fa
    db $e9, $f1, $fd, $99, $10, $02, $f5, $f4, $99, $2a, $04, $f8, $ee, $f5, $f9, $99
    db $66, $07, $f5, $ec, $ec, $ef, $e9, $eb, $f7, $99, $89, $02, $ef, $f4, $99, $a7
    db $07, $e9, $f5, $f3, $f3, $e7, $f4, $ea, $9a, $07, $05, $f8, $f9, $e7, $f7, $f9
    db $00, $d1, $00, $d6, $98, $22, $0e, $e7, $e8, $e9, $ea, $eb, $ec, $ed, $ee, $ef
    db $f0, $f1, $f2, $f3, $f4

GameA_ShipNameplateTable:
    db $05, $84, $fb, $ff, $d9, $04, $88, $e8, $ff, $da, $03, $8b, $e9, $ff, $db, $03
    db $8e, $f8, $ff, $dc, $02, $90, $ea, $ff, $dd

GameA_ShipSpriteTable:
    db $05, $bb, $04, $c0, $03, $c4, $03, $c7, $02, $ca, $00, $00, $20, $00, $00, $00
    db $01, $00

GameA_CoordLabelLayout:
    db $30, $88, $03, $00, $ff, $00

GameA_RedrawTimerPresets:
    db $00, $78, $79, $78, $8a, $78, $00, $70, $69, $70, $7a, $70, $00, $68, $59, $68
    db $6a, $68

GameA_ShipPartTiles:
    db $00, $02, $01, $02, $fe, $01, $01, $ff, $02, $02, $01, $fe

GameA_FleetIconTiles:
    db $74, $76, $78, $7a, $7c, $06, $0a, $0d, $10, $12

CursorMoveDeltas:
    db $13, $14, $01, $00, $ff, $00, $00, $ff, $00, $01

GameA_ShotMarkerTiles:
    db $b5, $b6, $b8, $b5, $b7, $b9

; The run lists of the board rows (UnpackRunList format).
GameA_BoardRowRuns:
    db $01, $92, $d1, $00, $01, $d0, $0e, $02, $d2, $d0
    db $13, $01, $d2, $ff, $01, $92, $d6, $00, $01, $d5, $0e, $02, $d7, $d5, $13, $01
    db $d7, $ff, $01, $92, $d1, $00, $01, $d0, $09, $02, $d2, $d0, $13, $01, $d2, $ff
    db $01, $92, $00, $00, $01, $d3, $09, $02, $d4, $d3, $13, $01, $d4, $ff, $01, $92
    db $d6, $00, $01, $d5, $09, $02, $d7, $d5, $13, $01, $d7, $ff

GameA_BoardSideBorderRuns:
    db $00, $d3, $fe, $0e, $d4, $d3, $fe, $13, $d4, $ff

GameA_VsComTextRuns:
    db $0c, $fb, $f8, $00, $e9, $f5, $f3, $ff

GameA_ArrangeTextRuns:
    db $01, $e7, $f7, $f7, $e7, $f4, $ed, $eb, $ff

GameA_FleetEnemyHeaderRuns:
    db $02, $ec, $f2, $eb, $eb, $f9, $00, $00, $d4, $d3, $eb, $f4, $eb, $f3, $fd, $00
    db $d8, $ff, $01, $ed, $e7, $f3, $eb, $f5, $fb, $eb, $f7, $ff

GameA_VersusTextRuns:
    db $0c, $fb, $eb, $f7, $f8, $fa, $f8, $ff

GameA_TurnCounterRuns:
    db $01, $f9, $fa, $f7, $f4, $00, $d8, $d8, $ff

GameA_NoEnemyTextRuns:
    db $0b, $00

GameA_NoEnemyText:
    db $cd, $00, $f5, $f4, $f2, $fd, $ff

GameA_EnemyCountRuns:
    db $0b

GameA_EnemyCountText:
    db $eb, $f4, $eb, $f3, $fd, $00, $d8, $ff

GameA_AircraftDirTiles:
    db $70, $00, $71, $00, $72, $00, $73, $00, $72, $00, $73, $00, $70, $00, $71, $00
    db $71, $00, $73, $00, $70, $00, $72, $00, $72, $00, $70, $00, $73, $00, $71, $00

; Cutscene script: "AIRCRAFT LAUNCHING". GameA_RunCutsceneScript runs it.
GameA_CutsceneAircraftLaunching:
    db $01, $4c, $38, $e7, $ef, $f7, $e9, $f7, $e7, $ec, $f9, $03, $10, $01, $5c, $34
    db $f2, $e7, $fa, $f4, $e9, $ee, $ef, $f4, $ed, $00

GameA_AircraftDirOffsets:
    db $02, $00, $fe, $00, $00, $02, $00, $fe, $ff, $fe, $fe, $ff, $ff, $00, $fe, $01
    db $ff, $02, $00, $30, $00, $38, $00, $80, $00, $38, $00, $30, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $f2, $fa, $e9, $f1, $fd, $fe

GameA_TargetPatternOffsets:
    db $00, $00, $ff, $ff, $01, $01, $ff, $01, $01, $ff, $fe, $fe, $02, $02, $fe, $02
    db $02, $fe, $98, $c3, $4e, $ec, $00

GameA_CpuCursorTiles:
    db $61, $00, $61, $20, $61, $40, $61, $60

GameA_ScrollYRowTriggers:
    db $77, $7f, $87, $8f, $c1, $c9, $d1, $d9

GameA_ScrollXColTriggers:
    db $68, $70, $78, $80, $c0, $c8, $d0, $d8

GameA_ScrollRowEdgeTiles:
    db $b1, $b2, $b3, $b4

GameA_ScrollColEdgeTiles:
    db $b1, $b3, $b2, $b4

GameA_BoardIconRuns:
    db $10, $aa, $ab, $ac, $ff, $99, $e2, $09, $da, $f6, $ff, $ea, $ea, $ea, $ea, $ea
    db $ea, $99, $af, $04, $b0, $b1, $b2, $b3, $99, $cf, $04, $b4, $b5, $b6, $b7, $99
    db $ef, $04, $b8, $b9, $ba, $bb, $9a, $0f, $04, $bc, $bd, $be, $bf, $00, $99, $e2
    db $09, $da, $f6, $ff, $eb, $eb, $eb, $eb, $eb, $eb, $99, $af, $04, $c0, $c1, $c2
    db $c3, $99, $cf, $04, $c4, $c5, $c6, $c7, $99, $ef, $04, $c8, $c9, $ca, $cb, $9a
    db $0f, $04, $cc, $cd, $ce, $cf, $00

GameA_AnimSlotFrameTiles:
    db $00, $01, $02, $03, $04, $05, $06, $07, $08, $10, $11, $0b, $0c, $12, $13, $0f
    db $14, $15, $16, $17, $18, $19, $1a, $1b, $1c, $1d, $1e, $1f, $0c, $12, $13, $0f
    db $00, $01, $02, $03, $04, $20, $06, $07, $21, $22, $23, $0b, $0c, $24, $25, $0f
    db $26, $27, $28, $29, $2a, $2b, $2c, $2d, $26, $2e, $2f, $30, $31, $32, $33, $34
    db $10, $11, $02, $03, $12, $13, $14, $07, $15, $16, $17, $0b, $18, $19, $1a, $0f
    db $1b, $1c, $1d, $1e, $1f, $20, $21, $22, $23, $24, $25, $26, $27, $28, $29, $2a
    db $00, $01, $02, $03, $04, $05, $2b, $07, $08, $2c, $2d, $0b, $0c, $2e, $2f, $0f
    db $00, $01, $02, $03, $04, $30, $31, $07, $08, $32, $33, $0b, $34, $35, $36, $0f
    db $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $1a, $1b, $1c, $1d, $1e, $1f
    db $20, $21, $22, $23, $24, $25, $26, $27, $28, $29, $1a, $1b, $2a, $2b, $1e, $1f
    db $00, $01, $02, $03, $04, $05, $06, $07, $2c, $2d, $2e, $0b, $2f, $0d, $0e, $0f
    db $30, $31, $32, $33, $34, $35, $36, $37, $38, $39, $3a, $3b, $3c, $3d, $3e, $3f
    db $00, $01, $02, $03, $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $1a, $1b
    db $1c, $1d, $1e, $1f, $20, $21, $22, $23, $24, $25, $26, $27, $28, $29, $2a, $2b
    db $00, $01, $02, $03, $04, $05, $06, $07, $08, $2c, $2d, $0b, $0c, $2e, $2f, $0f
    db $30, $31, $32, $33, $34, $35, $36, $37, $38, $39, $3a, $3b, $3c, $3d, $3e, $3f
    db $00, $01, $02, $03, $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $1a, $1b
    db $1c, $1d, $1e, $1f, $20, $21, $22, $23, $24, $25, $26, $27, $28, $29, $2a, $2b
    db $00, $01, $02, $03, $04, $05, $06, $07, $08, $2c, $2d, $0b, $0c, $2e, $2f, $0f
    db $30, $31, $32, $33, $34, $35, $36, $37, $38, $39, $3a, $3b, $3c, $3d, $3e, $3f
GameA_NextLevelShipTimingInit:
    db $0b, $00, $11, $00, $07, $00, $14, $00, $0e, $00

GameA_ResultsBlinkTiles:
    db $01, $01, $1e, $1f, $20, $21, $22, $23, $24, $25, $03, $03, $01, $01, $26, $27
    db $28, $29, $01, $01, $2a, $2b, $2c, $2d

GameA_ShipTileCopyEntries:
    db LOW(CommanderTiles4 + $3f0), HIGH(CommanderTiles4 + $3f0), $60, $86, $20, $05, $05, $f0, $5b, $60, $86, $20, $05, $02, $00, $01
    db $01, $01, $02, $00, $01, $01, $02, $00, $01, $02, $00, $01, $02, $00, $02

GameA_ShipCellIdRange:
    db $05, $01, $04, $06, $03, $0a, $03, $0d, $02, $10, $90, $60, $82, $8b, $91, $00
    db $40, $06, $ea, $e7, $f4, $ed, $eb, $f7, $fd, $00, $01, $fd, $01, $02, $fd, $02
    db $03, $fd, $03, $04, $fe, $08, $fd, $00, $05, $fd, $01, $06, $fd, $02, $07, $fd
    db $03, $08, $fa, $08, $fd, $00, $09, $fd, $01, $0a, $fd, $02, $0b, $fd, $03, $0c
    db $fe, $08, $fd, $00, $0d, $fd, $01, $0e, $fd, $02, $0f, $fd, $03, $10, $fa, $08
    db $01, $3c, $1c, $f4, $eb, $cf, $f9, $03, $20, $02, $f8, $f9, $e7, $ed, $eb, $00

GameA_YouLostSpriteLayout:
    db $00, $38, $08, $fd, $f5, $fa, $00, $f2, $f5, $f8, $f9

GameA_VictoryBgpFadeSteps:
    db $58, $06, $70, $16, $7c, $26, $88, $36

GameA_TextRevealAnim:
    db $fc, $00, $90, $30, $0b, $10 ; sprite 0: y=$90 x=$30 tile=$0b attr=$10
    db $fc, $01, $90, $38, $0c, $10 ; sprite 1: y=$90 x=$38 tile=$0c attr=$10
    db $fc, $02, $90, $40, $0d, $10 ; sprite 2: y=$90 x=$40 tile=$0d attr=$10
    db $fc, $03, $90, $58, $0e, $10 ; sprite 3: y=$90 x=$58 tile=$0e attr=$10
    db $fc, $04, $90, $60, $0f, $10 ; sprite 4: y=$90 x=$60 tile=$0f attr=$10
    db $fc, $05, $90, $78, $10, $10 ; sprite 5: y=$90 x=$78 tile=$10 attr=$10
    db $fc, $06, $90, $90, $11, $10 ; sprite 6: y=$90 x=$90 tile=$11 attr=$10
    db $fc, $07, $90, $98, $12, $10 ; sprite 7: y=$90 x=$98 tile=$12 attr=$10
    db $fe, $1a                     ; wait 26
    db $fd, $00, $13                ; sprite 0 tile = $13
    db $fd, $01, $14                ; sprite 1 tile = $14
    db $fd, $02, $15                ; sprite 2 tile = $15
    db $fd, $03, $16                ; sprite 3 tile = $16
    db $fd, $04, $17                ; sprite 4 tile = $17
    db $fd, $05, $18                ; sprite 5 tile = $18
    db $fd, $06, $19                ; sprite 6 tile = $19
    db $fd, $07, $1a                ; sprite 7 tile = $1a
    db $fe, $1a                     ; wait 26
    db $fd, $00, $1b                ; sprite 0 tile = $1b
    db $fd, $01, $1c                ; sprite 1 tile = $1c
    db $fd, $02, $1d                ; sprite 2 tile = $1d
    db $fd, $03, $1e                ; sprite 3 tile = $1e
    db $fd, $04, $1f                ; sprite 4 tile = $1f
    db $fd, $05, $20                ; sprite 5 tile = $20
    db $fd, $06, $21                ; sprite 6 tile = $21
    db $fd, $07, $22                ; sprite 7 tile = $22
    db $fe, $1a                     ; wait 26
    db $fd, $00, $13                ; sprite 0 tile = $13
    db $fd, $01, $14                ; sprite 1 tile = $14
    db $fd, $02, $15                ; sprite 2 tile = $15
    db $fd, $03, $16                ; sprite 3 tile = $16
    db $fd, $04, $17                ; sprite 4 tile = $17
    db $fd, $05, $18                ; sprite 5 tile = $18
    db $fd, $06, $19                ; sprite 6 tile = $19
    db $fd, $07, $1a                ; sprite 7 tile = $1a
    db $fa, $1a                     ; restart, wait 26

GameA_VictorySceneAnimLevel3:
    db $fc, $01, $28, $50, $01, $00 ; sprite 1: y=$28 x=$50 tile=$01 attr=$00
    db $fc, $02, $28, $58, $02, $00 ; sprite 2: y=$28 x=$58 tile=$02 attr=$00
    db $fc, $05, $30, $50, $03, $00 ; sprite 5: y=$30 x=$50 tile=$03 attr=$00
    db $fc, $06, $30, $58, $04, $00 ; sprite 6: y=$30 x=$58 tile=$04 attr=$00
    db $fc, $09, $38, $50, $05, $00 ; sprite 9: y=$38 x=$50 tile=$05 attr=$00
    db $fc, $0a, $38, $58, $06, $00 ; sprite 10: y=$38 x=$58 tile=$06 attr=$00
    db $fc, $0d, $40, $50, $07, $00 ; sprite 13: y=$40 x=$50 tile=$07 attr=$00
    db $fc, $0e, $40, $58, $08, $00 ; sprite 14: y=$40 x=$58 tile=$08 attr=$00
    db $fc, $10, $48, $38, $09, $00 ; sprite 16: y=$48 x=$38 tile=$09 attr=$00
    db $fc, $11, $48, $40, $0a, $00 ; sprite 17: y=$48 x=$40 tile=$0a attr=$00
    db $fc, $12, $48, $48, $0b, $00 ; sprite 18: y=$48 x=$48 tile=$0b attr=$00
    db $fc, $13, $48, $50, $0c, $00 ; sprite 19: y=$48 x=$50 tile=$0c attr=$00
    db $fc, $14, $48, $58, $0d, $00 ; sprite 20: y=$48 x=$58 tile=$0d attr=$00
    db $fc, $15, $48, $60, $0e, $00 ; sprite 21: y=$48 x=$60 tile=$0e attr=$00
    db $fc, $16, $48, $68, $0f, $00 ; sprite 22: y=$48 x=$68 tile=$0f attr=$00
    db $fc, $17, $48, $70, $09, $00 ; sprite 23: y=$48 x=$70 tile=$09 attr=$00
    db $fc, $18, $50, $30, $10, $00 ; sprite 24: y=$50 x=$30 tile=$10 attr=$00
    db $fc, $19, $50, $38, $11, $00 ; sprite 25: y=$50 x=$38 tile=$11 attr=$00
    db $fc, $1a, $50, $40, $12, $00 ; sprite 26: y=$50 x=$40 tile=$12 attr=$00
    db $fc, $1b, $50, $48, $13, $00 ; sprite 27: y=$50 x=$48 tile=$13 attr=$00
    db $fc, $1c, $50, $50, $14, $00 ; sprite 28: y=$50 x=$50 tile=$14 attr=$00
    db $fc, $1d, $50, $58, $15, $00 ; sprite 29: y=$50 x=$58 tile=$15 attr=$00
    db $fc, $1e, $50, $60, $16, $00 ; sprite 30: y=$50 x=$60 tile=$16 attr=$00
    db $fc, $1f, $50, $68, $17, $00 ; sprite 31: y=$50 x=$68 tile=$17 attr=$00
    db $fc, $20, $50, $70, $18, $00 ; sprite 32: y=$50 x=$70 tile=$18 attr=$00
    db $fc, $21, $50, $78, $19, $00 ; sprite 33: y=$50 x=$78 tile=$19 attr=$00
    db $fe, $a0                     ; wait 160
    db $fb, $01                     ; $deb8 sound = $01
    db $fd, $09, $1a                ; sprite 9 tile = $1a
    db $fd, $0a, $1b                ; sprite 10 tile = $1b
    db $fd, $0d, $1c                ; sprite 13 tile = $1c
    db $fd, $0e, $1d                ; sprite 14 tile = $1d
    db $fe, $0a                     ; wait 10
    db $f9, $01                     ; hide sprite 1
    db $f9, $02                     ; hide sprite 2
    db $fc, $04, $30, $48, $1e, $00 ; sprite 4: y=$30 x=$48 tile=$1e attr=$00
    db $fd, $05, $1f                ; sprite 5 tile = $1f
    db $fd, $06, $20                ; sprite 6 tile = $20
    db $fc, $07, $30, $60, $21, $00 ; sprite 7: y=$30 x=$60 tile=$21 attr=$00
    db $fc, $08, $38, $48, $22, $00 ; sprite 8: y=$38 x=$48 tile=$22 attr=$00
    db $fd, $09, $23                ; sprite 9 tile = $23
    db $fd, $0a, $24                ; sprite 10 tile = $24
    db $fc, $0b, $38, $60, $25, $00 ; sprite 11: y=$38 x=$60 tile=$25 attr=$00
    db $fc, $0c, $40, $48, $26, $00 ; sprite 12: y=$40 x=$48 tile=$26 attr=$00
    db $fd, $0d, $27                ; sprite 13 tile = $27
    db $fd, $0e, $28                ; sprite 14 tile = $28
    db $fc, $0f, $40, $60, $29, $00 ; sprite 15: y=$40 x=$60 tile=$29 attr=$00
    db $fd, $13, $2a                ; sprite 19 tile = $2a
    db $fd, $14, $2b                ; sprite 20 tile = $2b
    db $fe, $0a                     ; wait 10
    db $fc, $01, $28, $50, $2c, $00 ; sprite 1: y=$28 x=$50 tile=$2c attr=$00
    db $fc, $02, $28, $58, $2d, $00 ; sprite 2: y=$28 x=$58 tile=$2d attr=$00
    db $fd, $04, $2e                ; sprite 4 tile = $2e
    db $fd, $05, $2f                ; sprite 5 tile = $2f
    db $fd, $06, $30                ; sprite 6 tile = $30
    db $fd, $07, $31                ; sprite 7 tile = $31
    db $fd, $08, $32                ; sprite 8 tile = $32
    db $fd, $09, $33                ; sprite 9 tile = $33
    db $fd, $0a, $34                ; sprite 10 tile = $34
    db $fd, $0b, $35                ; sprite 11 tile = $35
    db $fd, $0c, $36                ; sprite 12 tile = $36
    db $fd, $0d, $37                ; sprite 13 tile = $37
    db $fd, $0e, $38                ; sprite 14 tile = $38
    db $fd, $0f, $39                ; sprite 15 tile = $39
    db $fd, $13, $3a                ; sprite 19 tile = $3a
    db $fd, $14, $3b                ; sprite 20 tile = $3b
    db $fe, $0a                     ; wait 10
    db $fc, $00, $28, $48, $3c, $00 ; sprite 0: y=$28 x=$48 tile=$3c attr=$00
    db $fd, $01, $3d                ; sprite 1 tile = $3d
    db $fd, $02, $3e                ; sprite 2 tile = $3e
    db $fc, $03, $28, $60, $3f, $00 ; sprite 3: y=$28 x=$60 tile=$3f attr=$00
    db $fd, $04, $40                ; sprite 4 tile = $40
    db $fd, $05, $41                ; sprite 5 tile = $41
    db $fd, $06, $42                ; sprite 6 tile = $42
    db $fd, $07, $43                ; sprite 7 tile = $43
    db $fd, $08, $44                ; sprite 8 tile = $44
    db $fd, $09, $45                ; sprite 9 tile = $45
    db $fd, $0a, $46                ; sprite 10 tile = $46
    db $fd, $0b, $47                ; sprite 11 tile = $47
    db $fd, $0c, $48                ; sprite 12 tile = $48
    db $fd, $0d, $49                ; sprite 13 tile = $49
    db $fd, $0e, $4a                ; sprite 14 tile = $4a
    db $fd, $0f, $4b                ; sprite 15 tile = $4b
    db $fe, $0a                     ; wait 10
    db $fd, $00, $4c                ; sprite 0 tile = $4c
    db $fd, $01, $4d                ; sprite 1 tile = $4d
    db $fd, $02, $4e                ; sprite 2 tile = $4e
    db $fd, $03, $4f                ; sprite 3 tile = $4f
    db $fd, $04, $50                ; sprite 4 tile = $50
    db $fd, $05, $51                ; sprite 5 tile = $51
    db $fd, $06, $52                ; sprite 6 tile = $52
    db $fd, $07, $53                ; sprite 7 tile = $53
    db $f9, $08                     ; hide sprite 8
    db $fd, $09, $54                ; sprite 9 tile = $54
    db $fd, $0a, $55                ; sprite 10 tile = $55
    db $f9, $0b                     ; hide sprite 11
    db $f9, $0c                     ; hide sprite 12
    db $fd, $0d, $56                ; sprite 13 tile = $56
    db $fd, $0e, $57                ; sprite 14 tile = $57
    db $f9, $0f                     ; hide sprite 15
    db $fe, $10                     ; wait 16
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $f9, $04                     ; hide sprite 4
    db $f9, $05                     ; hide sprite 5
    db $f9, $06                     ; hide sprite 6
    db $f9, $07                     ; hide sprite 7
    db $f9, $09                     ; hide sprite 9
    db $f9, $0a                     ; hide sprite 10
    db $f9, $0d                     ; hide sprite 13
    db $fd, $0e, $00                ; sprite 14 tile = $00
    db $fd, $13, $58                ; sprite 19 tile = $58
    db $fd, $14, $59                ; sprite 20 tile = $59
    db $fe, $20                     ; wait 32
    db $fc, $09, $38, $50, $5a, $00 ; sprite 9: y=$38 x=$50 tile=$5a attr=$00
    db $fc, $0a, $38, $58, $5b, $00 ; sprite 10: y=$38 x=$58 tile=$5b attr=$00
    db $fc, $0d, $40, $50, $5c, $00 ; sprite 13: y=$40 x=$50 tile=$5c attr=$00
    db $fe, $0d                     ; wait 13
    db $fd, $0a, $5d                ; sprite 10 tile = $5d
    db $fe, $14                     ; wait 20
    db $fd, $09, $61                ; sprite 9 tile = $61
    db $fd, $0a, $62                ; sprite 10 tile = $62
    db $fd, $0d, $63                ; sprite 13 tile = $63
    db $fe, $14                     ; wait 20
    db $fd, $09, $5e                ; sprite 9 tile = $5e
    db $fd, $0a, $5f                ; sprite 10 tile = $5f
    db $fd, $0d, $00                ; sprite 13 tile = $00
    db $fd, $0e, $60                ; sprite 14 tile = $60
    db $fe, $14                     ; wait 20
    db $fd, $09, $61                ; sprite 9 tile = $61
    db $fd, $0a, $62                ; sprite 10 tile = $62
    db $fd, $0d, $63                ; sprite 13 tile = $63
    db $fd, $0e, $00                ; sprite 14 tile = $00
    db $fe, $14                     ; wait 20
    db $fd, $09, $5e                ; sprite 9 tile = $5e
    db $fd, $0a, $5f                ; sprite 10 tile = $5f
    db $fd, $0d, $00                ; sprite 13 tile = $00
    db $fd, $0e, $60                ; sprite 14 tile = $60
    db $fe, $14                     ; wait 20
    db $fd, $09, $61                ; sprite 9 tile = $61
    db $fd, $0a, $62                ; sprite 10 tile = $62
    db $fd, $0d, $63                ; sprite 13 tile = $63
    db $fd, $0e, $00                ; sprite 14 tile = $00
    db $fe, $14                     ; wait 20
    db $fd, $09, $5e                ; sprite 9 tile = $5e
    db $fd, $0a, $5f                ; sprite 10 tile = $5f
    db $fd, $0d, $00                ; sprite 13 tile = $00
    db $fd, $0e, $60                ; sprite 14 tile = $60
    db $fe, $08                     ; wait 8
    db $f9, $09                     ; hide sprite 9
    db $f9, $0a                     ; hide sprite 10
    db $f9, $0e                     ; hide sprite 14
    db $f9, $10                     ; hide sprite 16
    db $f9, $11                     ; hide sprite 17
    db $f9, $12                     ; hide sprite 18
    db $f9, $13                     ; hide sprite 19
    db $f9, $14                     ; hide sprite 20
    db $f9, $15                     ; hide sprite 21
    db $f9, $16                     ; hide sprite 22
    db $f9, $17                     ; hide sprite 23
    db $f9, $18                     ; hide sprite 24
    db $f9, $19                     ; hide sprite 25
    db $f9, $1a                     ; hide sprite 26
    db $f9, $1b                     ; hide sprite 27
    db $f9, $1c                     ; hide sprite 28
    db $f9, $1d                     ; hide sprite 29
    db $f9, $1e                     ; hide sprite 30
    db $f9, $1f                     ; hide sprite 31
    db $f9, $20                     ; hide sprite 32
    db $f9, $21                     ; hide sprite 33
    db $ff                          ; end

GameA_ShotShortSplashAnim:
    db $fe, $07                     ; wait 7
    db $f8, $13                     ; sound effect $13
    db $fd, $00, $00                ; sprite 0 tile = $00
    db $fd, $01, $00                ; sprite 1 tile = $00
    db $fd, $02, $22                ; sprite 2 tile = $22
    db $fd, $03, $23                ; sprite 3 tile = $23
    db $fe, $0b                     ; wait 11
    db $fd, $00, $24                ; sprite 0 tile = $24
    db $fd, $01, $25                ; sprite 1 tile = $25
    db $fd, $02, $26                ; sprite 2 tile = $26
    db $fd, $03, $27                ; sprite 3 tile = $27
    db $fe, $03                     ; wait 3
    db $fe, $06                     ; wait 6
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $fd, $02, $32                ; sprite 2 tile = $32
    db $fd, $03, $33                ; sprite 3 tile = $33
    db $fe, $08                     ; wait 8
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $ff                          ; end

GameA_ShotShortSplashAnimLevel3:
    db $fe, $09                     ; wait 9
    db $fd, $00, $00                ; sprite 0 tile = $00
    db $fd, $01, $00                ; sprite 1 tile = $00
    db $fd, $02, $22                ; sprite 2 tile = $22
    db $fd, $03, $23                ; sprite 3 tile = $23
    db $fe, $09                     ; wait 9
    db $fd, $00, $24                ; sprite 0 tile = $24
    db $fd, $01, $25                ; sprite 1 tile = $25
    db $fd, $02, $26                ; sprite 2 tile = $26
    db $fd, $03, $27                ; sprite 3 tile = $27
    db $fe, $03                     ; wait 3
    db $f8, $10                     ; sound effect $10
    db $fe, $06                     ; wait 6
    db $f9, $00                     ; hide sprite 0
    db $f9, $01                     ; hide sprite 1
    db $fd, $02, $32                ; sprite 2 tile = $32
    db $fd, $03, $33                ; sprite 3 tile = $33
    db $fe, $08                     ; wait 8
    db $f9, $02                     ; hide sprite 2
    db $f9, $03                     ; hide sprite 3
    db $ff                          ; end
; The level map run lists for levels 1 and 2 (level 3 starts at
; GameA_LevelMapRuns + 4). GameA_DrawLevelMap unpacks one row a frame.
GameA_LevelMapStream:
    db $00, $8f, $00, $0f, $01, $7b, $10, $84, $00, $ff, $00, $88, $00, $08, $0c, $58
    db $53, $53, $59, $00, $00, $00, $7b, $00, $58, $53, $53, $ff, $00, $89, $00, $09
    db $0b, $93, $7d, $00, $7b, $00, $00, $7b, $7b, $00, $93, $7d, $ff, $00, $89, $00
    db $09, $0b, $93, $7d, $00, $7b, $00, $00, $7b, $7b, $00, $93, $7d, $ff, $00, $85
    db $57, $05, $0f, $55, $5b, $5c, $00, $93, $7d, $00, $7b, $00, $00, $7b, $7b, $00
    db $93, $7d, $ff, $06, $0e, $7e, $5c, $00, $8b, $8c, $00, $7b, $00, $00, $7b, $7b
    db $00, $8b, $8c, $ff, $05, $0f, $85, $86, $5c, $00, $8b, $8c, $00, $00, $7b, $00
    db $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $83, $84, $5c, $00, $8b, $8c, $00, $00
    db $00, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $83, $84, $5c, $00, $8b, $8c
    db $00, $00, $7b, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $81, $82, $5c, $00
    db $8b, $8c, $00, $00, $00, $00, $7b, $88, $00, $8b, $8c, $ff, $06, $0e, $7e, $5c
    db $00, $8b, $8c, $00, $00, $7b, $00, $7b, $87, $00, $8b, $8c, $ff, $01, $13, $89
    db $8a, $8a, $8a, $7f, $80, $5c, $00, $8b, $8c, $00, $00, $00, $00, $7b, $00, $00
    db $8b, $8c, $ff, $05, $0f, $85, $86, $5c, $00, $8b, $8c, $00, $00, $7b, $00, $7b
    db $00, $00, $8b, $8c, $ff, $05, $0f, $83, $84, $5c, $00, $8b, $8c, $00, $00, $00
    db $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $81, $82, $5c, $00, $8b, $8c, $00
    db $00, $7b, $00, $7b, $00, $00, $8b, $8c, $ff, $06, $0e, $7e, $5c, $00, $8b, $8c
    db $00, $00, $00, $00, $7b, $00, $00, $8b, $8c, $ff, $01, $13, $89, $8a, $8a, $8a
    db $7f, $80, $5c, $00, $8b, $8c, $00, $00, $7b, $00, $7b, $00, $00, $8b, $8c, $ff
    db $06, $0e, $7e, $5c, $00, $8b, $8c, $00, $00, $00, $00, $7b, $00, $00, $8b, $8c
    db $ff, $05, $0f, $81, $82, $5c, $00, $8b, $8c, $00, $00, $00, $00, $7b, $88, $00
    db $8b, $8c, $ff, $06, $0e, $7e, $5c, $00, $8b, $8c, $00, $00, $7b, $00, $7b, $87
    db $00, $8b, $8c, $ff, $01, $13, $89, $8a, $8a, $8a, $7f, $80, $5c, $00, $8b, $8c
    db $00, $00, $00, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $85, $86, $5c, $00
    db $8b, $8c, $00, $00, $7b, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $83, $84
    db $5c, $00, $8b, $8c, $00, $00, $00, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f
    db $81, $82, $5c, $00, $8b, $8c, $00, $00, $7b, $00, $7b, $00, $00, $8b, $8c, $ff
    db $06, $0e, $7e, $5c, $00, $8b, $8c, $00, $00, $00, $00, $7b, $00, $00, $8b, $8c
    db $ff, $01, $13, $89, $8a, $8a, $8a, $7f, $80, $5c, $00, $8b, $8c, $00, $00, $7b
    db $00, $7b, $00, $00, $8b, $8c, $ff, $06, $0e, $7e, $5c, $00, $8b, $8c, $00, $00
    db $00, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $85, $86, $5c, $00, $8b, $8c
    db $00, $00, $7b, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $83, $84, $5c, $00
    db $8b, $8c, $00, $00, $00, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $83, $84
    db $5c, $00, $8b, $8c, $00, $00, $7b, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f
    db $83, $84, $5c, $00, $8b, $8c, $00, $00, $00, $00, $7b, $88, $00, $8b, $8c, $ff
    db $05, $0f, $81, $82, $5c, $00, $8b, $8c, $00, $00, $7b, $00, $7b, $87, $00, $8b
    db $8c, $ff, $06, $0e, $7e, $5c, $00, $8b, $8c, $00, $00, $00, $00, $7b, $00, $00
    db $8b, $8c, $ff, $01, $13, $89, $8a, $8a, $8a, $7f, $80, $5c, $00, $8b, $8c, $00
    db $00, $7b, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $85, $86, $5c, $00, $8b
    db $8c, $00, $00, $00, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $0f, $83, $84, $5c
    db $00, $8b, $8c, $00, $00, $7b, $00, $7b, $00, $00, $8b, $8c, $ff, $05, $06, $83
    db $84, $5c, $00, $8b, $8c, $0b, $87, $00, $12, $02, $8b, $8c, $ff, $05, $0f, $81
    db $82, $5c, $00, $8b, $8c, $76, $77, $77, $77, $78, $79, $7a, $8b, $8c, $ff, $01
    db $13, $89, $8a, $8a, $8a, $7f, $80, $5c, $00, $8b, $8c, $67, $73, $74, $75, $72
    db $8f, $90, $8b, $8c, $ff, $06, $0e, $7e, $5c, $00, $8b, $8c, $67, $6b, $00, $91
    db $72, $8f, $90, $8b, $8c, $ff, $01, $13, $89, $8a, $8a, $8a, $7f, $80, $5c, $00
    db $8b, $8c, $67, $6f, $70, $71, $72, $8f, $90, $8b, $8c, $ff, $06, $0e, $7e, $5c
    db $00, $8b, $8c, $67, $01, $01, $01, $66, $8f, $90, $8b, $8c, $ff, $06, $0e, $7e
    db $5c, $00, $8b, $8c, $67, $6c, $92, $92, $6d, $8f, $90, $8b, $8c, $ff, $06, $0e
    db $7e, $5c, $00, $8b, $8c, $67, $6b, $00, $00, $00, $8f, $90, $8b, $8c, $ff, $06
    db $0e, $7e, $5c, $00, $8b, $8c, $67, $6b, $00, $69, $6a, $8f, $90, $8b, $8c, $ff
    db $06, $0e, $7e, $5c, $00, $5d, $5e, $62, $63, $64, $65, $66, $8f, $90, $5d, $5e
    db $ff, $06, $0e, $52, $5a, $00, $00, $00, $5f, $60, $60, $60, $61, $8d, $8e, $00
    db $00, $ff, $07, $04, $54, $55, $56, $56, $0b, $86, $57, $11, $03, $55, $56, $56
    db $ff, $09, $42, $50, $12, $42, $50, $ff, $09, $42, $4e, $12, $42, $4e, $ff

GameA_LevelMapRuns:
    db $04, $98, $04, $94, $02, $01, $50, $03, $8e, $00, $11, $03, $59, $5a, $5d, $ff
    db $02, $01, $50, $03, $8f, $00, $09, $02, $52, $50, $12, $02, $5b, $5e, $ff, $02
    db $01, $54, $03, $91, $00, $09, $02, $52, $50, $ff, $02, $01, $50, $03, $91, $00
    db $09, $02, $52, $50, $ff, $02, $01, $50, $03, $91, $00, $09, $02, $52, $50, $ff
    db $02, $01, $54, $03, $91, $00, $ff, $02, $01, $50, $03, $91, $00, $ff, $02, $01
    db $50, $03, $91, $00, $ff, $02, $01, $54, $03, $91, $00, $09, $02, $52, $50, $ff
    db $02, $01, $50, $03, $8f, $00, $09, $02, $52, $50, $12, $02, $58, $5f, $ff, $02
    db $01, $50, $03, $8e, $00, $09, $02, $52, $50, $11, $03, $56, $57, $5c, $ff, $02
    db $01, $54, $03, $8e, $00, $09, $02, $52, $50, $11, $01, $55, $ff, $02, $01, $50
    db $03, $8e, $00, $11, $01, $52, $ff, $02, $01, $50, $03, $8e, $00, $11, $01, $52
    db $ff, $02, $01, $54, $03, $8e, $00, $11, $01, $55, $ff, $02, $01, $50, $03, $8e
    db $00, $09, $02, $52, $50, $11, $01, $52, $ff, $02, $01, $50, $03, $8e, $00, $09
    db $02, $52, $50, $11, $01, $52, $ff, $02, $01, $54, $03, $8e, $00, $09, $02, $52
    db $50, $11, $01, $55, $ff, $02, $01, $50, $03, $8e, $00, $09, $02, $52, $50, $11
    db $03, $59, $5a, $5d, $ff, $02, $01, $50, $03, $8f, $00, $12, $02, $5b, $5e, $ff
    db $02, $01, $54, $03, $91, $00, $ff, $02, $01, $50, $03, $91, $00, $ff, $02, $01
    db $50, $03, $91, $00, $09, $02, $52, $50, $ff, $02, $01, $54, $03, $91, $00, $09
    db $02, $52, $50, $ff, $02, $01, $50, $03, $91, $00, $09, $02, $52, $50, $ff, $02
    db $01, $50, $03, $91, $00, $09, $02, $52, $50, $ff, $02, $01, $54, $03, $91, $00
    db $ff, $02, $01, $50, $03, $8f, $00, $12, $02, $58, $5f, $ff, $02, $01, $50, $03
    db $8e, $00, $11, $03, $56, $57, $5c, $ff, $02, $01, $54, $03, $8e, $00, $09, $02
    db $52, $50, $11, $01, $55, $ff, $02, $01, $50, $03, $8e, $00, $09, $02, $52, $50
    db $11, $01, $52, $ff, $02, $01, $50, $03, $8e, $00, $09, $02, $52, $50, $11, $01
    db $52, $ff, $02, $01, $54, $03, $8e, $00, $09, $02, $52, $50, $11, $01, $55, $ff
    db $02, $01, $50, $03, $8e, $00, $11, $01, $52, $ff, $02, $01, $50, $03, $8e, $00
    db $11, $01, $52, $ff, $02, $01, $54, $03, $8e, $00, $11, $01, $55, $ff, $02, $01
    db $50, $03, $8e, $00, $09, $02, $52, $50, $11, $01, $52, $ff, $02, $01, $50, $03
    db $8e, $00, $09, $02, $52, $50, $11, $01, $52, $ff, $02, $01, $54, $03, $8e, $00
    db $09, $02, $52, $50, $11, $01, $55, $ff, $02, $01, $50, $03, $8e, $00, $09, $02
    db $52, $50, $11, $01, $52, $ff, $02, $01, $50, $03, $8e, $00, $11, $01, $52, $ff
    db $02, $01, $54, $03, $8e, $00, $11, $01, $55, $ff, $02, $01, $50, $03, $8e, $00
    db $11, $01, $52, $ff, $02, $01, $50, $03, $8e, $00, $09, $02, $52, $50, $11, $01
    db $52, $ff, $02, $01, $54, $03, $8e, $00, $09, $02, $52, $50, $11, $01, $55, $ff
    db $02, $01, $50, $03, $8e, $00, $09, $02, $52, $50, $11, $01, $52, $ff, $02, $01
    db $4e, $03, $8e, $4f, $09, $02, $51, $4e, $11, $01, $51, $ff, $02, $01, $50, $03
    db $8e, $00, $11, $01, $52, $ff, $02, $10, $50, $53, $00, $53, $00, $53, $00, $00
    db $00, $00, $53, $00, $53, $00, $53, $52, $ff, $02, $01, $4e, $03, $8e, $4f, $11
    db $01, $51, $ff

GameA_SunkCrossTileMask:
    db $7e, $81, $bd, $42, $db, $24, $e7, $18, $e7, $18, $db, $24, $bd, $42, $7e, $81

GameA_TallyAnimSetup:
    db $d6, $65, $85, $65, $27, $66, $00, $00, $00, $04, $00

GameA_EnemyFleetGridPtrs:
    db $d4, $c6, $24, $c5

GameA_AircraftDescentDelays:
    db $05, $06, $06, $06, $07, $07, $08, $08

GameA_StartText:
    db $f8, $f9, $e7, $f7, $f9

GameA_ArrangeFleetWindowTilemap:
    db $9c, $e4, $84, $d0, $d3, $d3, $d5, $9c, $f0, $84, $d2, $d4, $d4, $d7, $9c, $e5
    db $4b, $d1, $9d, $05, $0b, $00, $00, $e7, $f7, $f7, $e7, $f4, $ed, $eb, $00, $00
    db $9d, $25, $0b, $fd, $f5, $fa, $f7, $00, $ec, $f2, $eb, $eb, $f9, $fe, $9d, $45
    db $4b, $d6, $00, $eb, $cf, $f6, $f2, $f5, $ea, $ef, $f4, $ed

GameA_WaitingText:
    db $fc, $e7, $ef, $f9, $ef, $f4, $ed

GameA_LevelSpritesLevel1:
    db $02, $74, $02, $76, $02, $78, $02, $7a, $01, $7c, $ff

GameA_LevelSpritesLevel2:
    db $02, $74, $02, $7a, $02, $7a, $02, $7a, $ff

GameA_LevelSpritesLevel3:
    db $03, $7d, $01, $6c, $01, $6c, $01, $6c, $ff

DirectionDeltas:
    db $ff, $00, $01, $00, $00, $ff, $00, $01

GameA_CongratsBannerSprite:
    db $50, $e0, $45, $80

GameA_CongratulationsText:
    db $32, $3e, $3d, $36, $40, $30, $42, $43, $3b, $30, $42, $38, $3e, $3d, $41, $47

GameA_DpadToDirection:
    db $00, $01, $03, $02, $04, $bb, $00, $e0, $40, $53, $01, $80, $50, $88, $00, $09
    db $14, $8b, $50, $13, $51, $4b, $45, $96, $40, $ba, $00, $c0, $05, $4c, $40, $84
    db $04, $08, $03, $00, $41, $83, $00, $9e, $40, $2d, $01, $14, $54, $32, $15, $80
    db $4c, $72, $45, $79, $01, $1f, $11, $1b, $10, $9a, $45, $65, $11, $d6, $55, $18
    db $00, $4c, $05, $ea, $55, $89, $44, $7d, $45, $17, $40, $6d, $05, $c2, $45, $2f
    db $14, $a8, $44


; ---------------------------------------------------------------------------
; Game B: end-of-stage tally (score, time bonus, ranks)
; ---------------------------------------------------------------------------

SECTION "Bank1 tally vectors", ROMX[$7800], BANK[$1]

; Bank 1 entry vectors at $7800. Bank 0 calls these by raw address
; (tables at bank0 $3ad8 for $d824 and $3ba7 for $d828).
Bank1_TallyVectors:
    jp tallyState1RollRanks                      ; $7800
    jp tallyState2DrawHeader                     ; $7803
Vec1_GameB_TallyState3_ComputeTimeBonus:
    jp tallyState3ComputeTimeBonus               ; $7806
Vec1_GameB_TallyState4_AccumulateScore:
    jp GameB_TallyState4_AccumulateScore         ; $7809
Vec1_GameB_TallyInitScreen:
    jp tallyInitScreen                           ; $780c
Vec1_GameB_TallyState5_WaitInput:
    jp GameB_TallyState5_WaitInput               ; $780f
Vec1_GameB_CutsceneState1_DrawScene:
    jp cutsceneState1DrawScene                   ; $7812
Vec1_GameB_CutsceneState2_MoveObjects:
    jp cutsceneState2MoveObjects                 ; $7815
Vec1_GameB_CutsceneState3_Scroll:
    jp cutsceneState3Scroll                      ; $7818
Vec1_GameB_CutsceneAnimateSprites:
    jp cutsceneAnimateSprites                    ; $781b
Vec1_GameB_CutsceneScrollTiles:
    jp cutsceneScrollTiles                       ; $781e
    jp GameB_TallyState6_SelectFanfare           ; $7821
    jp cutsceneState4WaitThenExit                ; $7824
    jp GameB_TallyFrameRefresh_CopyDigitsToTiles ; $7827
    jp blinkTallySprites                         ; $782a

; Bank 0 also enters at $7820, $7823, $7826 and $7829. Each runs the high
; byte of a jp target as one opcode ($40-$7f = "ld r, r"), then the next jp.
; No caller needs a register after the call, so any "ld" is fine. $76
; is "halt".
ASSERT HIGH(cutsceneScrollTiles) != $76
ASSERT HIGH(GameB_TallyState6_SelectFanfare) != $76
ASSERT HIGH(cutsceneState4WaitThenExit) != $76
ASSERT HIGH(GameB_TallyFrameRefresh_CopyDigitsToTiles) != $76

SECTION "Bank1 data B", ROMX[$7d63], BANK[$1]

; ---------------------------------------------------------------------------
; Game B data: tally and cutscene tables
; ---------------------------------------------------------------------------

; Game B tally tables. One entry per tally row (7 rows).
; Offset into $d800 of the kill count for each row.
GameB_TallyKillCountPtrTable:
    db $04, $06, $05, $02, $01, $00, $03

; Y/X of the 12 tally row sprites.
GameB_TallyRowSprites:
    db $80, $70, $80, $78, $80, $80, $80, $88, $80, $90, $80, $98, $90, $70, $90, $78
    db $90, $80, $90, $88, $90, $90, $90, $98

; Tilemap stream (rst $28) for the tally row labels.
GameB_TallyRowLabels:
    db $98, $01, $12, $d3, $c0, $d1, $c6, $c4, $d3, $d2, $82, $82, $c3, $c4, $d2, $d3
    db $d1, $ce, $d8, $c4, $c3, $99, $a1, $52, $11, $99, $c1, $05, $d2, $c2, $ce, $d1
    db $c4, $9a, $01, $05, $d3, $ce, $d3, $c0, $cb, $00

; Point value of each row, 4 digits, high digit first. $82 = blank digit.
; Indexed by $d337 * $1c + row * 4.
GameB_TallyRowPayloads:
    ; $d337 = 0
    db $00, $00, $00, $01
    db $00, $00, $06, $82
    db $00, $00, $04, $82
    db $00, $00, $03, $82
    db $00, $00, $01, $82
    db $00, $00, $03, $82
    db $00, $00, $01, $82
    ; $d337 = 1
    db $00, $00, $00, $02
    db $00, $00, $05, $01
    db $00, $00, $00, $01
    db $00, $00, $08, $82
    db $00, $00, $04, $82
    db $00, $00, $08, $82
    db $00, $00, $04, $82
    ; $d337 = 2
    db $00, $00, $00, $04
    db $00, $00, $08, $01
    db $00, $00, $02, $01
    db $00, $00, $00, $01
    db $00, $00, $05, $82
    db $00, $00, $00, $01
    db $00, $00, $05, $82

GameB_TallyRowNameTiles:
    db $ee, $ef, $f0, $e3, $e4, $e5, $e0, $e1, $e2, $e6, $e7, $e8, $e9, $ea, $82, $eb
    db $ec, $82, $ed, $82, $82

GameB_TallyBonusTiles:
    db $f1, $f2, $f3, $82, $84

GameB_TallyHeader:
    db $99, $41, $12, $c1, $ce, $cd, $d4, $d2, $82, $b5, $82, $85, $84, $84, $84, $f1
    db $f2, $f3, $82, $84, $84, $00

GameB_TimeBonusBanner:
    db $99, $81, $12, $d3, $c8, $cc, $c4, $82, $82, $82, $84, $84, $f4, $84, $84, $82
    db $84, $84, $84, $84, $84, $00

GameB_TimeBonusRankFloor:
    db $64, $00, $82, $00, $82, $00

; Steps of GameB_CutsceneState1_DrawScene: wait frames, then tilemap stream.
GameB_CutsceneSceneSteps:
    db $01
    dw GameB_CutsceneSceneTilemap1
    db $17
    dw GameB_CutsceneSceneTilemap2
    db $01
    dw GameB_CutsceneSceneTilemap3
    db $17
    dw GameB_CutsceneSceneTilemap4

GameB_CutsceneSceneTilemap1:
    db $99, $ca, $02, $36, $37, $9a, $04, $06, $36, $37, $36, $37, $36, $37, $9a, $0c
    db $06, $36, $37, $36, $37, $36, $37, $00

GameB_CutsceneSceneTilemap2:
    db $99, $ea, $02, $38, $39, $9a, $24, $06, $38, $39, $38, $39, $38, $39, $9a, $2c
    db $06, $38, $39, $38, $39, $38, $39, $00

GameB_CutsceneSceneTilemap3:
    db $99, $ca, $02, $3a, $3b, $9a, $04, $06, $3a, $3b, $3a, $3b, $3a, $3b, $9a, $0c
    db $06, $3a, $3b, $3a, $3b, $3a, $3b, $00

GameB_CutsceneSceneTilemap4:
    db $99, $ea, $02, $3c, $00, $9a, $24, $06, $3c, $00, $3c, $00, $3c, $00, $9a, $2c
    db $06, $3c, $00, $3c, $00, $3c, $00, $00

GameB_CutsceneAttrFlips:
    db $00, $03, $05, $07, $03, $05, $07

GameB_CutsceneSpriteScript:
    db $fe, $00, $67, $98, $10, $fe, $01, $68, $90, $0f, $fe, $02, $68, $60, $20, $fe
    db $03, $68, $18, $27, $ff, $60, $fe, $04, $68, $88, $0c, $fe, $02, $68, $60, $22
    db $fe, $05, $68, $58, $20, $fe, $06, $68, $10, $26, $ff, $58, $fe, $04, $68, $88
    db $0e, $fe, $07, $69, $80, $0c, $fe, $02, $68, $60, $22, $fe, $05, $68, $58, $21
    db $fe, $08, $68, $50, $20, $ff, $40, $fe, $07, $69, $80, $0d, $fe, $09, $69, $78
    db $0c, $ff, $80, $fe, $00, $6b, $98, $1a, $fe, $01, $6b, $90, $19, $fe, $02, $6b
    db $88, $18, $fe, $03, $6b, $80, $17, $fe, $04, $6b, $78, $16, $fe, $05, $63, $60
    db $01, $fe, $06, $6b, $60, $05, $fe, $07, $6b, $58, $04, $fe, $08, $6b, $50, $03
    db $fe, $09, $6b, $48, $02, $fe, $0a, $6b, $40, $00, $ff, $10, $fd, $00, $fd, $01
    db $fd, $02, $fd, $03, $fd, $04, $ff, $08, $fe, $00, $6a, $58, $22, $fe, $01, $6a
    db $50, $21, $fe, $02, $6a, $48, $20, $fd, $05, $fd, $06, $fd, $07, $fd, $08, $fd
    db $09, $fd, $0a, $ff, $16, $fe, $00, $6a, $58, $27, $fe, $01, $6a, $50, $26, $fd
    db $02, $ff, $40, $fd, $00, $fd, $01, $ff, $f0

; Pointers to sprite flash lists: count, then XOR values for OAM attributes.
GameB_CutsceneFlashPtrs:
    dw GameB_CutsceneFlashList1, GameB_CutsceneFlashList2, GameB_CutsceneFlashList3, GameB_CutsceneFlashList4, GameB_CutsceneFlashList5, GameB_CutsceneFlashList6, GameB_CutsceneFlashList7, GameB_CutsceneFlashList8

GameB_CutsceneFlashList1:
    db $04, $05, $1b, $03, $0e

GameB_CutsceneFlashList2:
    db $07, $05, $1b, $07, $0e, $1d, $03, $0e

GameB_CutsceneFlashList3:
    db $09, $05, $1b, $07, $0e, $1d, $05, $0e, $1d, $03

GameB_CutsceneFlashList4:
    db $0a, $05, $1b, $07, $0e, $1d, $05, $0e, $1f, $03, $1d

GameB_CutsceneFlashList5:
    db $0b, $05, $07, $05, $0b, $0d, $07, $0e, $0e, $0a, $0a, $07

GameB_CutsceneFlashList6:
    db $06, $07, $0e, $0e, $0a, $0a, $07

GameB_CutsceneFlashList7:
    db $03, $07, $05, $03

GameB_CutsceneFlashList8:
    db $02, $0e, $0e

GameB_BlinkSpriteXA:
    db $20, $28, $20, $28, $20, $28

GameB_BlinkSpriteXB:
    db $80, $88, $80, $88, $80, $88

; Not referenced. Probably leftover bytes at the end of the bank.
Unused_Bank1TailBytes:
    db $7b, $04, $7b, $43, $d6, $95, $be, $55, $ef, $7f, $ff, $d5, $ef, $6d, $ff, $fd
    db $ff, $37, $ed, $45, $bf, $5d, $f7, $d5, $bd, $5d, $f5, $15, $ff, $55, $47, $04

