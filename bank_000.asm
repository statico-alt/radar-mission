; Disassembly of "rm.gb"
; This file was created with:
; mgbdis v3.0 - Game Boy ROM disassembler by Matt Currie and contributors.
; https://github.com/mattcurrie/mgbdis

; ROM bank 0 (fixed, $0000-$3FFF). It holds the RST and interrupt
; vectors, the cartridge header, the main loop and bank switching, shared
; helpers (copy, fill, numbers, tilemaps, VRAM queue) and most of the
; Game A and Game B state machines. Code here selects banks 1, 3, 5 and 6
; with SwitchROMBank and then calls into them.

SECTION "ROM Bank $000", ROM0[$0]


    ; ---- RST vectors and interrupt vectors ----
    ; rst $00: jump table. A = index; the dw table follows the rst.
RST_00::
    jp RST00_JumpTableDispatch


    nop
    nop
    nop
    nop
    nop

    ; rst $08: bit dispatch. B = mask, A = bits; calls the dw entry
    ; for each set bit, high bit first.
RST_08::
    jp RST08_BitDispatch


    nop
    nop
    nop
    nop
    nop

    ; rst $10: copy BC bytes from HL to DE.
CopyBytes::
    jp CopyMem


    nop
    nop
    nop
    nop
    nop

    ; rst $18: fill BC bytes at HL with A.
FillBytes::
    jp FillMem


    nop
    nop
    nop
    nop
    nop

    ; rst $20: grid cell address. HL += D * g_gridSize + E.
GridCellAddr::
    jp RST20_GridOffset


    nop
    nop
    nop
    nop
    nop

    ; rst $28: run the tilemap script at DE (DrawTilemapScript).
DrawTilemapScript::
    jp DrawTilemapCmds


    nop

    nop
    nop
    nop
    nop

    ; rst $30: set or clear one HRAM flag bit. A = %SBBBIIII:
    ; S = 1 set / 0 clear, BBB = bit, IIII = index into HramFlagPtrs.
RST_30::
    jp RST30_SetResFlagBit


    ret


    nop
    nop
    nop
    nop

    ; rst $38: unused, returns.
RST_38::
    ret


    nop
    nop
    nop

    nop
    nop
    nop
    nop

    ; Hardware interrupt vectors.
VBlankInterrupt::
    jp IRQ_VBlank


    nop
    nop
    nop
    nop
    nop

LCDCInterrupt::
    jp IRQ_LCDStat


    nop
    nop
    nop
    nop
    nop

TimerOverflowInterrupt::
    jp IRQ_Timer


    nop
    nop
    nop
    nop
    nop

SerialTransferCompleteInterrupt::
    jp IRQ_Serial


    nop
    nop
    nop

    nop
    nop

JoypadTransitionInterrupt::
    reti


    nop
    nop
    nop
    nop
    nop
    nop
    nop

; rst $00: jump through the word table that follows the rst instruction.
; In: A = table index. Does not return to the caller.
RST00_JumpTableDispatch:
    pop hl
    push de
    add a
    ld e, a
    ld d, $00
    add hl, de
    call DerefHL
    pop de
    jp hl


; rst $08: jump by button/flag bits. In: B = mask, A = bits.
; The table after the rst has one word per set mask bit (MSB first).
; Jumps to the first entry whose mask bit and A bit are both set;
; if none match, returns to the caller's caller.
RST08_BitDispatch:
    pop hl
    ld c, $08

.loop:
    sla b
    jr c, .maskBitSet

    rla
    jr .next

.maskBitSet:
    and a

    rla
    jr c, .jump

    inc hl
    inc hl

.next:
    dec c
    jr nz, .loop

    ret


.jump:
    call DerefHL
    jp hl


; rst $30: set or clear one bit of an HRAM flag byte.
; In: A = %SBBBIIII. S = 1 sets, 0 clears; BBB = bit; IIII = index into HramFlagPtrs.
RST30_SetResFlagBit:
    push hl
    push bc
    push af
    and $0f
    add a
    ld c, a
    ld b, $00
    ld hl, HramFlagPtrs

    add hl, bc
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    pop af
    push af
    ld b, $01
    swap a
    bit 3, a
    jr nz, .setMaskReady

    ld b, $fe

.setMaskReady:
    and $07

.shiftLoop:
    and a
    jr z, .apply

    rlc b
    dec a
    jr .shiftLoop

.apply:
    pop af
    rla
    jr c, .setBit

    ld a, [hl]
    and b
    ld [hl], a
    jr .done

.setBit:
    ld a, [hl]
    or b
    ld [hl], a

.done:
    pop bc
    pop hl
    ret


; Flag-byte pointers for RST_30, indexed by the low nibble of A.
HramFlagPtrs:
    dw g_topModeFlags, g_gameALevel, g_gameFlags2, g_gameFlags3, g_gameFlags4, g_gameFlags5, g_gameFlags6, g_p1Options
    dw $ffaf, $ffb0, $ff8e

    ds 41, $00

    ; ---- Cartridge header ----
Boot::
    nop
    jp Boot_Jump


HeaderLogo::
    db $ce, $ed, $66, $66, $cc, $0d, $00, $0b, $03, $73, $00, $83, $00, $0c, $00, $0d
    db $00, $08, $11, $1f, $88, $89, $00, $0e, $dc, $cc, $6e, $e6, $dd, $dd, $d9, $99
    db $bb, $bb, $67, $63, $6e, $0e, $ec, $cc, $dd, $dc, $99, $9f, $bb, $b9, $33, $3e

HeaderTitle::
    db "RADARMISSION", $00, $00, $00, $00

HeaderNewLicenseeCode::
    db $00, $00

HeaderSGBFlag::
    db $00

HeaderCartridgeType::
    db $01

HeaderROMSize::
    db $02

HeaderRAMSize::
    db $00

HeaderDestinationCode::
    db $01

HeaderOldLicenseeCode::
    db $01

HeaderMaskROMVersion::
    db $00

HeaderComplementCheck::
    db $56

HeaderGlobalChecksum::
    db $3a, $8e

    ; ---- Fixed entry points used by other banks ----
    ; Boot entry jumps here.
Boot_Jump:
    jp Main


    ; Game B state $2b: soft reset back to the title.
GameB_State2b_SoftReset:
    jp SoftReset


    ; Far jump vectors so other banks can call these ROM0 helpers.
Vec_GameA_LoadCursorTiles:
    jp GameA_LoadCursorTiles


Vec_Multiply16:
    jp Multiply16


Vec_Divide16By8:
    jp Divide16By8


Vec_UnpackRunList:
    jp UnpackRunList


Vec_DrawNumber3:
    jp DrawNumber3


Vec_GameA_DrawFleetPanel:
    jp GameA_DrawFleetPanel


Vec_ClearShadowOAM:
    jp ClearShadowOAM


Vec_QueueTilemapRowCopy:
    jp QueueTilemapRowCopy


Vec_LcdOffMaskIrqs:
    jp LcdOffMaskIrqs


Vec_LcdOnEnableIrqs:
    jp LcdOnEnableIrqs


Vec_BuildSpriteStrips:
    jp BuildSpriteStrips


Vec_SumFleetBytes:
    jp SumFleetBytes


EXPORT SoftReset, GameA_Frame, GameB_Frame, Vec_GameB_VBlank, SwitchROMBank
EXPORT Title_GfxLogoTiles, Title_ExtraTiles2Gfx, Title_GfxExtraTiles, Title_ExtraTilesGfx
EXPORT Vec5_TitleScreenInit, Bank5_EntryVector, Vec3_GameB_LinkUpdate, SoundUpdate
EXPORT UpdatePaletteFade, StirRandom, CopyOamDmaToHram
EXPORT GameB_SelectRank, RankScreen_Build, GameB_EndingGfxTilesA, GameB_EndingGfxTilesB
EXPORT GameB_CutsceneGfxTiles, LcdOffMaskIrqs, LcdOnEnableIrqs
EXPORT BuildSpriteStrips, VBlank_RunCopyQueue, ClearShadowOAM, SumFleetBytes
; Data in banks 1, 2, 4 and 5 that the Game A C in bank 8 reads.
EXPORT CommanderTiles3, CommanderTiles4, GameA_CutsceneAircraftLaunching, GameA_GfxAirStrikeBgTiles
EXPORT GameA_GfxAirStrikeBgTiles2, GameA_GfxAirStrikeSprites, GameA_GfxAltModeTiles, GameA_GfxPlayerEnemyTiles
EXPORT GameA_GfxReadyBanner, GameA_GfxStatusTiles, GameA_GfxTiles9F, GameA_NextLevelShipTimingInit
EXPORT GameA_PlacementSpriteLayout, GameA_ShotAnimSpriteStrip, GameA_TallyAnimC, GameA_VictorySpritesLevel3Gfx
; Used by the Game B C code (bank10_gameb*.c, bank10_gbfar.c).
EXPORT Bank1_TallyVectors, Bank6_DrawDeathTallies, Bank6_DrawMetasprite, Bank6_DrawSixAnimSprites, ClearDamageGauge
EXPORT ClearMgTrails, ClearShadowSprites, ClearSpriteEntries, ClearSpritesFrom
EXPORT DrawRLERowsThenRLE, DrawTableString, DrawTilesRLE, DrawTilesRLE_TwoParts, DrawTilesRLERows
EXPORT GameB_AdvanceScrollScript, GameB_AnimateWaves, GameB_AnimTick, GameB_BlockMovement, GameB_ClearDamageGauge
EXPORT GameB_ClearFireFlags, GameB_CutsceneTimerStep, GameB_DamageStepTable, GameB_DrawDepthChargesAt, GameB_DrawEnemySlots
EXPORT GameB_DrawEnemySlotsD258, GameB_DrawObjMeta2, GameB_DrawPlayerSlots, GameB_DrawPlayerSlotsD1c0, GameB_DrawRankScreen
EXPORT GameB_DrawSideHeader, GameB_DrawStatusPattern, GameB_DrawStatusScreen, GameB_EnableIrqs, GameB_GetSideAdjustFlipped
EXPORT GameB_GfxSprites, GameB_KillConfirm, GameB_LcdOff, GameB_LevelEndAnimPtrs, GameB_LevelNames
EXPORT GameB_LevelSelectInput, GameB_LevelSelectScoreTilemap, GameB_LevelSelectSprites, GameB_LevelSelectSpritesA, GameB_LevelSelectSpritesB
EXPORT GameB_LevelSelectSpritesC, GameB_LevelSelectSpritesD, GameB_LevelSelectTilemap, GameB_LoadLevel, GameB_LookupData6e78
EXPORT GameB_MapSelectedShip, GameB_MarkEnemyShip0, GameB_MarkLinkEnd, GameB_MaybeAddItem, GameB_MessagePtrs
EXPORT GameB_ObjMetaspritePtrs, GameB_PickLevelValue, GameB_PlayNextTableSound, GameB_RankStrings, GameB_RasterTables
EXPORT GameB_ReadPadAndClock, GameB_ResetAnimPeriod, GameB_ResetEnemySub, GameB_ResetPlayerShip, GameB_ResetSceneFlags
EXPORT GameB_ResetScroll, GameB_SaveToWreckSlot, GameB_SceneTilemap, GameB_SendLinkStatus, GameB_SetLevelClearTimer
EXPORT GameB_SetRasterFlags, GameB_SixAnimCounterDefaults, GameB_StartDamageGauge, GameB_State14_PerFrameEngine, GameB_State2b_SoftReset
EXPORT GameB_Sub_4561, GameB_Sub_52ab, GameB_Sub_5b11, GameB_SubDispatch_d009, GameB_TallyKillFlags
EXPORT GameB_UpdateDistanceCounters, GameB_WaveStepSizes, gbJpScrollSpritesSkip4, gbJpSetPalettesD31f, HideWindow
EXPORT Hud_Cmd14, Hud_ToggleWindow, HudCmd_DrawCounters, HudCmd_DrawWeaponIcons, HudQueue_Push
EXPORT InitD091, LinkTxQueue_Push, MemClear, MemFill, PlaySound_DEB0
EXPORT PlaySound_DEB8, RankScreen_Frame, RankScreen_MoveStars, ScrollScript_EndScene, Vec1_GameB_CutsceneAnimateSprites
EXPORT Vec1_GameB_CutsceneScrollTiles, Vec1_GameB_CutsceneState1_DrawScene, Vec1_GameB_CutsceneState2_MoveObjects, Vec1_GameB_CutsceneState3_Scroll, Vec1_GameB_TallyInitScreen
EXPORT Vec1_GameB_TallyState3_ComputeTimeBonus, Vec1_GameB_TallyState4_AccumulateScore, Vec1_GameB_TallyState5_WaitInput, Vec6_GameB_InitScaledTable, Vec6_GameB_InitShipArrays
EXPORT Vec6_GameB_UpdateWeaponFlags

    ; Power on: clear g_topModeFlags, then fall into SoftReset.
Main:
    xor a
    ldh [g_topModeFlags], a

    ; Soft reset: set up the stack and bank 1, turn the LCD off, clear
    ; HRAM and WRAM, then run softReset (bank0_main.c). Asm: it sets SP,
    ; and the WRAM clear covers the stack.
SoftReset:
    di
    ld sp, $dfff
    ld a, $01
    call SwitchROMBank
    call WaitVBlankLcdOff
    ldh a, [g_topModeFlags]
    and $a0
    ldh [g_topModeFlags], a
    ld hl, g_gameALevel
    ld bc, $0072
    xor a
    rst $18
    ld hl, g_oamBuffer

    ld bc, $1fff

.clearWram:
    ld [hl], $00
    inc hl
    dec bc
    ld a, b
    or c
    jr nz, .clearWram

    jp softReset

    ; Interrupt entry and exit code. The bodies are C (bank0_irq.c).
IRQ_VBlank::
    push af
    push bc
    push hl
    push de
    call irqVBlank
    pop de
    pop hl
    pop bc
    pop af
    reti

    ; Timer: only a nested serial IRQ can come in (IE bit 2 off).
IRQ_Timer::
    push hl
    ld hl, rIE
    res 2, [hl]
    ei
    push af
    push bc
    push de
    call irqTimer
    pop de
    pop bc
    pop af
    ld hl, rIE
    set 2, [hl]
    pop hl
    reti

    ; LCD STAT: Game B raster effects (asm: the raster deadline), or the
    ; Game A effects (irqStatGameA).
IRQ_LCDStat:
    push af
    push hl
    push de
    push bc
    ldh a, [g_topModeFlags]
    bit 5, a
    jr nz, .gameB

    call irqStatGameA
    jr IRQ_LCDStat_Return

.gameB:
    call Vec_GameB_StatRasterFx

IRQ_LCDStat_Return:
    pop bc
    pop de
    pop hl
    pop af
    reti

IRQ_Serial:
    push af
    push hl
    push bc
    push de
    call irqSerial
    pop de
    pop bc
    pop hl
    pop af
    reti

    ; Far call for C: "call FarCall / db bank / dw routine". Maps the bank,
    ; calls the routine and maps the old bank again. A, BC, DE and HL go
    ; into the routine. A, F, BC, DE and HL come back from it.
FarCall::
    add sp, -6
    push hl
    push de
    push af
    ; Stack: AF, DE, HL, routine, .tail, old bank, R (R = the db byte).
    ld hl, sp+12
    ld e, [hl]
    inc hl
    ld d, [hl]
    inc de
    inc de
    inc de
    ld [hl], d
    dec hl
    ld [hl], e              ; return to R+3
    dec hl
    ldh a, [g_currentBankShadow]
    ld [hl-], a             ; old bank (high byte of its slot)
    dec hl
    ld a, HIGH(.tail)
    ld [hl-], a
    ld a, LOW(.tail)
    ld [hl-], a
    dec de
    ld a, [de]              ; routine high
    ld [hl-], a
    dec de
    ld a, [de]              ; routine low
    ld [hl], a
    dec de
    ld a, [de]              ; bank
    ldh [g_currentBankShadow], a
    ld [rROMB0], a
    pop af
    pop de
    pop hl
    ret

.tail:
    push af
    push hl
    ld hl, sp+5
    ld a, [hl]
    ldh [g_currentBankShadow], a
    ld [rROMB0], a
    pop hl
    pop af
    inc sp
    inc sp
    ret

    ; ======== Game A ========
    ; Game A per-frame entry. The body is C (gameAFrame, bank8_gamea_frame.c).
    ; Asm: callers expect bank 1 mapped after it. The play clock, the
    ; scroll and the OBP1 cycle run here first: from bank 8 they cost three
    ; bcalls, and an LCD-off load must still start in the same VBlank as
    ; the original.
GameA_Frame:
    call UpdatePlayClock
    call GameA_UpdateScroll
    ld a, $05
    call SwitchROMBank
    call Vec5_GameA_CyclePaletteOBP1
    ld a, BANK(gameAFrame)
    call SwitchROMBank
    call gameAFrame
    ld a, $01
    jp SwitchROMBank

    ; Copy BC bytes from HL in ROM bank A to DE, then map bank 8 again.
    ; Asm: the bank 8 Game A C cannot map another bank under itself.
gaFarCopyRegs::
    ldh [g_currentBankShadow], a
    ld [$2000], a
    call CopyMem
    ld a, BANK(gameAFrame)
    ldh [g_currentBankShadow], a
    ld [$2000], a
    ret

    ; The 7-ship tile loop of the state $1e load (gaBoardLoad1E).
    ; Asm: it calls bank 1 in a tight loop with the LCD off. The load
    ; time sets the timer IRQ phase, and FarCalls from bank 8 are slower.
gaLoadShipTiles::
    ld a, $01
    call SwitchROMBank
    xor a
    ldh [g_listCount], a
.loop:
    ldh a, [g_listCount]
    ld de, GameA_PlacementSpriteLayout + $15
    call GameA_LoadShipRecord
.loop2:
    call GameA_RotateShipTileStep
    push af
    ld de, g_vramQueue
    rst $28
    pop af
    jr nc, .loop2
    ld hl, g_listCount
    inc [hl]
    ld a, [hl]
    cp $07
    jr nz, .loop
    ld a, BANK(gameAFrame)
    jp SwitchROMBank


; Copy one ship tile, turned to the placement direction, to VRAM.
; In: $ffc4 = source tile in bank 2, $ffc6 = VRAM target,
;     $ffc8 = direction (0 as is, 1 flip, 2/3 turn 90 degrees),
;     $ffc3 = tiles left.
; The tile goes through g_vramQueue (drawn in VBlank).
; Advances $ffc4 and $ffc6 by 16 bytes.
; Out: carry set when $ffc3 reaches 0.
; Asm: bank 1 asm calls it in a tight LCD-off loop; TileXform_* and
; TurnTile_* below are its inner loops.
GameA_RotateShipTileStep:
    ld hl, g_shipTileSrc ; source tile pointer
    call ReadWordDE
    ld a, $02
    call SwitchROMBank
    ld hl, .copyToQueue
    push hl
    ldh a, [g_placeDirection] ; direction
    rst $00
    dw TileXform_Copy
    dw TileXform_FlipV
    dw TileXform_TurnLeft
    dw TileXform_TurnRight

.copyToQueue:
    ld hl, g_tileWorkBuf ; transformed tile


    ld de, g_vramQueue + $3 ; queue data area
    ld bc, $0010
    push bc
    rst $10
    ld a, $01
    call SwitchROMBank
    ld hl, g_shipTileDst ; VRAM target pointer
    call DerefHL
    push hl
    ld a, h
    ld [g_vramQueue], a
    ld a, l
    ld [g_vramQueue + 1], a
    ld a, $10 ; 16 bytes
    ld [g_vramQueue + 2], a
    xor a
    ld [g_vramQueue + $13], a
    pop hl
    pop bc
    add hl, bc
    ld a, l
    ldh [g_shipTileDst], a
    ld a, h
    ldh [g_shipTileDst + 1], a
    ld hl, g_shipTileSrc
    call DerefHL
    add hl, bc
    ld a, l
    ldh [g_shipTileSrc], a
    ld a, h
    ldh [g_shipTileSrc + 1], a
    ld hl, g_shipTileCount ; tiles left
    and a
    dec [hl]
    ret nz

    scf
    ret


; Direction 0: copy the 16-byte tile at DE to the work buffer $c0df.
TileXform_Copy:
    ld l, e
    ld h, d
    ld de, g_tileWorkBuf
    ld bc, $0010
    rst $10
    ret


; Direction 1: copy the tile at DE to $c0df with the 8 rows in
; reverse order (vertical flip).
TileXform_FlipV:
    ld l, e
    ld h, d
    ld de, g_tileWorkBuf + $e
    ld c, $08

.loop:
    ld a, [hl+]
    ld [de], a
    inc de
    ld a, [hl+]
    ld [de], a
    dec de
    dec de
    dec de
    dec c
    jr nz, .loop

    ret


; Direction 2: turn the tile at DE 90 degrees one way.
TileXform_TurnLeft:
    xor a
    call TurnTile90
    ret


; Direction 3: turn the tile at DE 90 degrees the other way.
TileXform_TurnRight:
    ld a, $01
    call TurnTile90
    ret


; Turn the 2bpp tile at DE by 90 degrees into $c0df.
; In: A = 0 shifts bits out to the right, A = 1 to the left.
; Uses $c0ef as a copy of the source tile.
TurnTile90:
    ld l, e
    ld h, d
    push af
    push hl
    ld hl, g_tileWorkBuf
    ld bc, $0010
    push bc
    xor a
    rst $18
    pop bc
    pop hl
    ld de, g_tileSrcBuf
    rst $10
    ld hl, g_tileWorkBuf
    ld de, g_tileSrcBuf
    ld bc, $0808 ; b = 8 bits, c = 8 rows
    pop af

.rowLoop:
    push de

.byteLoop:
    push af

    and a
    jr nz, .shiftLeft

    call TurnTile_ShiftRight
    jr .nextByte

.shiftLeft:
    call TurnTile_ShiftLeft

.nextByte:
    pop af
    inc de
    dec hl
    dec b
    jr nz, .byteLoop

    ld b, $08
    inc hl
    inc hl
    pop de
    dec c
    jr nz, .rowLoop

    ret


; Move the low bit of both bit planes at DE into the row at HL.
TurnTile_ShiftRight:
    ld a, [de]
    srl a
    ld [de], a
    rl [hl]
    inc de
    inc hl
    ld a, [de]
    srl a
    ld [de], a
    rl [hl]
    ret


; Move the high bit of both bit planes at DE into the row at HL.
TurnTile_ShiftLeft:
    ld a, [de]
    sla a
    ld [de], a
    rr [hl]
    inc de
    inc hl
    ld a, [de]
    sla a
    ld [de], a
    rr [hl]
    ret


    ; ---- Memory and math helpers ----
; Fill BC bytes at HL with A.
FillMem:
    ld [hl+], a
    ld d, a
    dec bc
    ld a, b
    or c
    ld a, d
    jr nz, FillMem

    ret


    ; Load HL from the word at HL.
DerefHL:
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    ret


; Load the word at HL into DE.
ReadWordDE:
    ld e, [hl]
    inc hl
    ld d, [hl]
    dec hl
    ret


; VBlank: run the queued VRAM copies (queue count at $c4bf). TSV name: VBlank_RunCopyQueue.
VBlank_RunCopyQueue:
    ld a, [g_vramCopyCount] ; number of queued copies
    and a
    ret z

    ld hl, g_vramCopyQueue + $4 ; entry 0: +0 src, +2 dst, +4 length, +5 repeat count, +6 bank
    ld a, [hl-]
    ld c, a
    ld b, $00
    dec hl
    call ReadWordDE
    dec hl
    dec hl
    call DerefHL
    push hl
    push de
    push bc
    ld a, [g_vramCopyQueue + 6]
    call SwitchROMBank
    rst $10
    ld a, $01
    call SwitchROMBank
    pop bc
    pop hl
    add hl, bc
    ld e, l
    ld d, h
    pop hl
    add hl, bc
    push bc
    ld a, l
    ld b, h
    ld hl, g_vramCopyQueue
    ld [hl+], a
    ld [hl], b
    inc hl
    ld [hl], e
    inc hl
    ld [hl], d
    pop bc
    ld hl, g_vramCopyQueue + $5 ; repeat count
    dec [hl]
    ret nz

    ld a, [g_vramCopyCount]
    cp $01
    jr z, .lastChunk

    dec a
    ld e, a
    ld d, $00
    ld hl, $0007 ; 7 bytes per entry
    call Multiply16
    ld c, l
    ld b, $00
    ld hl, g_vramCopyQueue + $7 ; move the next entries down by one
    ld de, g_vramCopyQueue
    rst $10

.lastChunk:
    ld hl, g_vramCopyCount
    dec [hl]
    ret


; HL += D * [g_gridSize] + E (row/column offset into a grid of width [g_gridSize]).
RST20_GridOffset:
    push hl
    push de
    ldh a, [g_gridSize]
    ld l, a
    ld h, $00
    ld e, d
    ld d, $00
    call Multiply16
    pop de
    ld d, $00
    add hl, de
    ld e, l
    ld d, h
    pop hl
    add hl, de
    ret


    ; HL = HL * DE (16-bit, low word).
Multiply16:
    ld c, l
    ld b, h
    ld hl, $0000
    ld a, $10

.loop:
    srl d
    rr e
    jr nc, .noAdd

    add hl, bc

.noAdd:
    sla c
    rl b
    dec a
    jr nz, .loop

    ret


    ; HL = HL / B. Out: HL = quotient, A = remainder.
Divide16By8:
    xor a
    ld e, a
    ld d, a
    inc b
    dec b
    jr z, .done

    ld c, $10

.loop:
    sla l
    rl h
    rla
    sub b
    jr nc, .noRestore

    add b

.noRestore:
    ccf
    rl e
    rl d
    dec c
    jr nz, .loop

.done:
    ld l, e
    ld h, d
    ret


    ; Unpack a run list from the stream pointer g_mapStreamPtr into HL.
    ; Record: offset, then %RIcccccc (R = repeat byte, I = count up),
    ; then data. $ff ends the list. Saves the new stream pointer.
UnpackRunList:
    push hl
    ld hl, g_mapStreamPtr
    call ReadWordDE
    pop hl

.nextRecord:
    ld a, [de]
    cp $ff
    jr z, .end

    push hl
    ld c, a
    ld b, $00
    add hl, bc
    inc de
    ld a, [de]
    and $c0
    ld b, a
    ld a, [de]
    and $3f
    ld c, a
    inc de

.nextByte:
    ld a, [de]

.store:
    ld [hl+], a
    dec c
    jr z, .recordDone

    bit 7, b
    jr nz, .store

    inc a
    bit 6, b
    jr nz, .store

    inc de
    jr .nextByte

.recordDone:
    inc de
    pop hl
    jr .nextRecord

.end:
    inc de
    ld hl, g_mapStreamPtr
    ld [hl], e
    inc hl
    ld [hl], d
    ret


    ; ---- Bank switch and Game A drawing helpers ----
SwitchROMBank:
    ld [g_currentBankShadow], a
    ld [$2000], a
    ret


    ; Load the Game A cursor tile and set the cursor anim scripts.
GameA_LoadCursorTiles:
    call FarCall
    db BANK(gameALoadCursorTiles)
    dw gameALoadCursorTiles
    ret


    ; Draw A (0..255) as 3 digit tiles at HL, without leading zeros.
DrawNumber3:
    ld de, $d8d8 ; d = e = $d8: two digit counters, not an address

.hundreds:
    cp $64
    jr c, .tens

    inc d
    sub $64
    jr .hundreds

.tens:
    cp $0a
    jr c, .ones

    inc e
    sub $0a
    jr .tens

.ones:
    add $d8
    ld b, a

    ld a, $d8
    cp d
    jr z, .skipHundreds

    ld [hl], d

.skipHundreds:
    inc hl
    cp e
    jr nz, .storeTens

    cp d
    jr z, .storeOnes

.storeTens:
    ld [hl], e

.storeOnes:
    inc hl
    ld [hl], b
    ret


    ; Draw the Game A fleet status panel.
    ; Shows the sunk ship tiles of the other side on the panel, marks
    ; the cells listed at $c950, then draws the grid rows and the
    ; 3-digit value at $9a10. Keeps the current ROM bank.
GameA_DrawFleetPanel:
    call FarCall
    db BANK(gameADrawFleetPanel)
    dw gameADrawFleetPanel
    ret


    ; Clear the shadow OAM at $c000 (160 bytes).
ClearShadowOAM:
    ld hl, g_oamBuffer
    ld bc, $00a0
    xor a
    rst $18
    ret


    ; Queue a copy of one 32-byte tilemap row.
    ; In: DE = tilemap base, $ffb1 = row. Source is g_vramQueue.
QueueTilemapRowCopy:
    push de
    ldh a, [g_gameATurnTimer]
    ld e, a
    ld d, $00
    ld hl, $0020
    call Multiply16
    pop de
    add hl, de
    ld c, l
    ld b, h
    ld de, g_vramQueue
    call QueueVramCopy
    ld [hl], $20
    inc hl
    ld [hl], $01
    inc hl
    ld [hl], $02
    ret


    ; Add a VRAM copy to the queue at $c4c0 (7 bytes per entry).
    ; In: DE = source, BC = target.
    ; Out: HL = entry +4; the caller stores length, repeat count, bank.
QueueVramCopy:
    push de
    ld hl, g_vramCopyCount
    ld e, [hl]
    ld d, $00
    inc hl
    push hl
    push bc
    ld hl, $0007
    call Multiply16
    pop bc
    ld e, l
    ld d, $00
    pop hl
    add hl, de
    pop de
    ld [hl], e
    inc hl
    ld [hl], d
    inc hl
    ld [hl], c
    inc hl
    ld [hl], b
    inc hl
    ret


; Turn the LCD off and mask interrupts.
LcdOffMaskIrqs:
    di
    call WaitVBlankLcdOff
    ldh a, [rIE]
    and $fc
    ldh [rIE], a
    ei
    ret


; Turn the LCD on and re-enable interrupts.
LcdOnEnableIrqs:
    ldh a, [rIE]
    or $03
    ldh [rIE], a
    ld a, $c3
    ldh [rLCDC], a
    ret


    ; In: HL = OAM target, B = sprite attributes, C = sprite count.
    ; Each strip header is Y, X, flags, tile. Flags: bit 7 = strip goes
    ; down (else right), bit 6 = tile number counts up, bits 0-5 = length.
    ; When a strip ends, the next header follows in the list.
; Build shadow-OAM sprite strips from the list at DE.
BuildSpriteStrips:
    ld a, [de]
    ld [g_stripY], a
    inc de
    ld a, [de]
    ld [g_stripX], a
    inc de
    ld a, [de]
    ld [g_stripFlags], a
    inc de
    ld a, [de]
    ld [g_stripTile], a
    inc de

.loop:
    ld a, [g_stripY]
    ld [hl+], a
    ld a, [g_stripX]
    ld [hl+], a
    ld a, [g_stripTile]
    ld [hl+], a
    ld [hl], b
    inc hl
    dec c
    ret z

    ld a, [g_stripFlags]
    dec a
    ld [g_stripFlags], a
    and $3f
    jr z, BuildSpriteStrips

    ld a, [g_stripFlags]
    push af
    rla
    jr c, .stepDown

    ld a, [g_stripX]
    add $08
    ld [g_stripX], a
    jr .nextTile

.stepDown:
    ld a, [g_stripY]
    add $08
    ld [g_stripY], a

.nextTile:
    pop af
    bit 6, a
    jr z, .readTile

    push hl
    ld hl, g_stripTile
    inc [hl]
    pop hl
    jr .loop

.readTile:
    push af
    ld a, [de]
    ld [g_stripTile], a
    inc de
    pop af
    jr .loop

    ; Out: A = 1 + sum of the 5 ship lengths of the current player
    ; ($c0a5 for player 1, $c76c for player 2).
SumFleetBytes:
    push hl
    push bc
    ld hl, g_p1ShipLengths
    ld a, [g_currentPlayer]
    and a
    jr z, .sum

    ld hl, g_p2ShipLengths

.sum:
    ld a, $01
    ld c, $05

.loop:
    add [hl]
    inc hl
    dec c
    jr nz, .loop

    pop bc
    pop hl
    ret


; Wait for VBlank, then turn the LCD off.
WaitVBlankLcdOff:
    ldh a, [rIE]
    ldh [g_savedIE], a ; save IE
    res 0, a
    ldh [rIE], a

.waitVBlank:
    ldh a, [rLY]
    cp $91 ; line $91 = in VBlank
    jr c, .waitVBlank

    ld hl, rLCDC
    res 7, [hl]
    ldh a, [g_savedIE]
    ldh [rIE], a
    ret


; Copy BC bytes from HL to DE.
CopyMem:
    ld a, [hl+]
    ld [de], a
    inc de

    dec bc
    ld a, b
    or c
    jr nz, CopyMem

    ret


; Read one tilemap command (hi, lo, mode/count) at DE, draw it, then
; go on with DrawTilemapCmds.
DrawTilemapCmds_Next:
    inc de
    ld h, a
    ld a, [de]
    ld l, a
    inc de
    ld a, [de]
    inc de
    call DrawTilemapCmd

; Draw a list of tilemap commands at DE, ended by $00.
; Record: [hi][lo][mode<<6 | count] data. Modes: 0 copy row, 1 fill row,
; 2 copy column, 3 fill column.
DrawTilemapCmds:
    ld a, [de]
    cp $00
    jr nz, DrawTilemapCmds_Next

    ret


    ; Draw one tilemap command at HL. In: A = mode<<6 | count, DE = data.
DrawTilemapCmd:
    push af
    and $3f
    ld b, a
    pop af
    rlca
    rlca
    and $03
    jr z, .copyRow

    dec a
    jr z, .fillRow

    dec a
    jr z, .copyColumn

    jr .fillColumn

.copyRow:
    ld a, [de]
    ld [hl+], a
    inc de
    dec b
    jr nz, .copyRow

    ret


.fillRow:
    ld a, [de]
    inc de

.fillRowLoop:
    ld [hl+], a
    dec b
    jr nz, .fillRowLoop

    ret


.copyColumn:
    ld a, [de]
    ld [hl], a
    inc de
    ld a, b
    ld bc, $0020
    add hl, bc
    ld b, a
    dec b
    jr nz, .copyColumn

    ret


.fillColumn:
    ld a, [de]
    ld [hl], a
    ld a, b
    ld bc, $0020
    add hl, bc
    ld b, a
    dec b
    jr nz, .fillColumn

    inc de
    ret


    ; ======== Game B ========
    ; Game B (submarine) runs from here to the end of the bank.
    ; GameB_Frame runs each frame with bank 3 mapped and calls the
    ; handler for g_subState from GameB_SubStateTable.
    ds 30, $00 ; padding up to $2000

    ; Fixed entry points at $2000 used by other banks.
Vec_GameB_StatRasterFx:
    jp GameB_StatRasterFx


    nop
    nop
    ret


    ; Fixed entry: Game B VBlank work (run the next HUD command).
Vec_GameB_VBlank:
    jp HudQueue_RunNext


    ; Run one frame of Game B: input, sound queue, frame counter,
    ; then the handler for g_subState.
GameB_Frame:
    ld a, $03
    call SwitchROMBank_FromA
    call GameB_ProcessInput
    call SoundQueue_RunNext
    ld hl, g_frameCounter
    inc [hl]
    ld hl, GameB_SubStateTable
    ld a, [g_subState]
    jp DispatchJumpTable


    ; Game B states in C (bank10_gameb.c, bank10_gbcut.c). gbRunState
    ; runs the handler for g_subState.
GameB_State00_Reset:
GameB_State01_SceneLoader:
GameB_State02_InitCheck:
GameB_State03_LevelIntroSetup:
GameB_State04_LevelSelect:
GameB_State05:
GameB_State06:
GameB_State08_MenuInput:
GameB_State09_MenuWaitDea1:
GameB_State0a_MenuFadeTimer:
GameB_State0b_PostMenuTimer:
GameB_State0c_SceneLoadGraphics:
GameB_State0d_SceneInit0:
GameB_State0e_SceneInit1:
GameB_State0f_SceneD091Loop:
GameB_State10_PrepPlay:
GameB_State12_ResetCounters:
GameB_State13_D009InputWait:
GameB_State15_PrePlayCountdown:
GameB_State16_LevelClearGate:
GameB_State17_EngineTail:
GameB_State18_HitGate:
GameB_State1a_KillBookkeeping:
GameB_State1b_EndClear:
GameB_State1c:
GameB_State1d_Underwater:
GameB_State1e_SceneEndAnim:
GameB_State1f_SceneEndTimer:
GameB_State20_SceneEndPause:
GameB_State21_Verdict:
GameB_State22_GameOver:
GameB_State23_NextScene:
GameB_State24_TrampolineTo3ace:
GameB_State25:
GameB_State26_CutsceneDispatchD826:
GameB_State27_SelectPath2801:
GameB_State28_CutsceneD02eGate:
GameB_State29_CutsceneStep533a:
GameB_State2a_CutsceneStep532e:
GameB_State2c_PostKillD091:
GameB_State2d_GameOverVerdict:
    FAR_CALL_C gbRunState

GameB_State2f_LoadSubroutine:
    FAR_CALL_C gbState2f

GameB_State30_CheckSubKill:
    FAR_CALL_C gbState30

    ; Game B state $07 stays in asm. With it in C, the Game B race at
    ; frame 4616 drew the sea one pixel off (timing; padding the C did
    ; not fix it).
GameB_State07_MenuInit:
    call GameB_ClearSceneRam
    call GameB_LoadLevel
    call GameB_ResetScrollScriptPos
    ld a, $55
    ld [g_shipArray_Player + 2], a
    ld a, $05
    call SetPalettes
    ld hl, GameB_MenuTiles
    ld de, $8800
    ld bc, $1000
    ld a, $07
    call SwitchROMBank_FromA
    rst $10
    ld a, $03
    call SwitchROMBank_FromA
    ld a, [g_gameBStage]
    or a
    ld a, $17
    call z, PlayMusic
    ld hl, g_gameBLayoutVariant
    ld bc, $0007
    call MemClear
    ld a, $83
    call GameB_LcdOnRestoreIE
    and a
    ld hl, g_subState
    jp GameB_AdvanceCounter

GameB_LcdOnRestoreIE:
    ldh [rLCDC], a
    sub a
    ldh [rIF], a
    ldh a, [g_savedIE]
    ldh [rIE], a
    ei
    ret

GameB_LcdOffAndReset:
    call GameB_LcdOff
    jp ClearShadowSprites ; bank 3

; Turn the LCD off and clear the scene RAM ($d00e, $d200, $d298),
; reset the scroll and window, fill the BG map with tile $fa.
; Plays sound 7 when g_soundFadeState is set. Falls into the next.
GameB_ClearSceneRam:
    call GameB_LcdOffAndReset
    ld hl, g_aiScanIndex
    ld bc, $0172
    call MemClear
    ld hl, g_gameBObjSlots
    ld bc, $0018
    call MemClear
    ld hl, g_spawnObjSlots
    ld bc, $007e
    call MemClear
    call GameB_ResetScroll
    call HideWindow
    ld hl, $9800
    ld bc, $0240
    ld e, $fa
    call MemFill
    call GameB_DrawStatusScreen ; bank 3
    call ClearDamageGauge ; bank 3
    ld hl, g_modeFlags16
    res 6, [hl]
    ld a, [g_soundFadeState]
    or a
    ld a, $07
    call nz, PlaySFX

; Clear $d2e3 and the scroll script position bytes.
GameB_ResetScrollScriptPos:
    sub a
    ld [g_linkSyncIndex], a
    ld hl, ScrollScript_LastTileByte
    ld [hl+], a
    ld [hl], a
    ret

    ; Entries for the C shims (local labels cannot be exported).
gbJpSetPalettesD31f:
    jp SetPalettes.d31fEntry

gbJpScrollSpritesSkip4:
    jp RankScreen_ScrollSprites.skip4


; HUD command $0e: copy one 16-byte tile from a ROM bank into VRAM.
; The HUD slot gives the bank (low nibble) and a table index. When
; the source byte is $ff, sets g_modeFlags15 bit 5 (done).
Hud_CopySlotTile:
    call HudQueue_ReadSlot
    inc hl
    ld a, [hl]
    ld hl, ScrollScript_BannerPtrTable ; bank 3 pointer table
    sla a
    ld c, a
    ld b, $00
    add hl, bc
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    ld a, [hl+]
    ld b, a
    and $0f
    ld [g_bannerSrcBank], a
    ld a, [ScrollScript_Pad]

    ld e, a
    inc a
    ld [ScrollScript_Pad], a
    ld d, $00
    add hl, de
    ld e, l
    ld d, h
    ld a, b
    and $f0
    srl a
    ld b, a
    ld c, $00
    ld hl, $4000 ; tile source base in the selected bank
    add hl, bc
    ld a, [de]
    cp $ff
    jr z, .skip

    push hl
    push de
    ld e, a
    ld d, $00
    call GameB_ShiftDELeft4 ; bank 3
    add hl, de
    push hl
    ld a, [g_bannerVramPage]
    sla a
    sla a
    sla a
    ld b, a
    ld c, $00
    ld hl, $8000 ; VRAM tile base
    add hl, bc
    ld a, [g_bannerTileIndex]
    ld e, a
    inc a
    ld [g_bannerTileIndex], a
    ld d, $00
    call GameB_ShiftDELeft4 ; bank 3
    add hl, de
    ld e, l
    ld d, h
    pop hl
    ld a, [g_bannerSrcBank]
    call SwitchROMBank_FromA ; bank from the HUD slot (low nibble)
    ld bc, $0010 ; one tile
    rst $10
    ld a, $03 ; back to the Game B bank
    call SwitchROMBank_FromA
    pop de
    pop hl
    ret


.skip:
    ld hl, g_modeFlags15
    set 5, [hl]
    ret
    ; HUD command handlers, indexed by the command in the HUD queue.
HudCmdTable: ; HUD command table, indexed by the queued command
    dw HudCmd_DrawPair, HudCmd_DrawGauge, HudCmd_DrawCounters, HudCmd_DrawD2ec
    dw HudCmd_DrawHeader, HudCmd_DrawIcon, HudCmd_DrawMark, HudCmd_DrawWeaponIcons
    dw HudCmd_DrawKillMarker, HudCmd_DrawPanel, HudCmd_DrawString, ScrollScript_StreamToVram_Unused
    dw HudCmd_ClearIcon, HudCmd_DrawBars, Hud_CopySlotTile, HudCmd_DrawColumn


    ; Run one queued HUD command from the ring of $14 slots ($d033 =
    ; read index). With the queue empty, runs the $d878 tilemap
    ; script once when g_modeFlags15 bit 6 is set.
HudQueue_RunNext:
    call HudQueue_ReadSlot
    ld a, [hl]
    or a
    jr z, .skip

    ld [hl], $00
    dec a
    ld hl, HudCmdTable
    call DispatchJumpTable
    ld hl, g_hudQueueRead
    inc [hl]
    ld a, [hl]
    cp $14 ; $14 slots in the ring
    ret c

    ld [hl], $00
    ret


.skip:
    ld hl, g_modeFlags15
    bit 6, [hl]
    ret z

    push hl
    ld de, g_tileScript ; bank 3 tilemap script
    rst $28
    pop hl
    res 6, [hl]
    ret


    ; Game B intro screen (called from bank 3, bank10_gameb.c).
GameB_IntroInit::
    FAR_CALL_C gbIntroInit

GameB_IntroUpdate::
    FAR_CALL_C gbIntroUpdate


    ; Step g_subStateScratch9. Carry set = reset to 0 first.
GameB_Advance_d009:
    ld hl, g_subStateScratch9

; Shared tail. In: HL = counter, carry set = reset to 0 first.
; Out: [HL] incremented.
GameB_AdvanceCounter:
    jr nc, .skip

    ld [hl], $ff

.skip:
    inc [hl]
    ret
; Game B substate handlers, indexed by [g_subState] ($d005).
; Dispatched by the bank 3 helper at $4f97 (HL = table, A = index).
; The table continues into the two tables below: entries $2e-$39.
; GameB_PerFrameJumpTable_d008 and _d009 index into its tail.
GameB_SubStateTable:
    dw GameB_State00_Reset ; $00
    dw GameB_State01_SceneLoader ; $01
    dw GameB_State02_InitCheck ; $02
    dw GameB_State03_LevelIntroSetup ; $03
    dw GameB_State04_LevelSelect ; $04
    dw GameB_State05 ; $05
    dw GameB_State06 ; $06
    dw GameB_State07_MenuInit ; $07
    dw GameB_State08_MenuInput ; $08
    dw GameB_State09_MenuWaitDea1 ; $09
    dw GameB_State0a_MenuFadeTimer ; $0a
    dw GameB_State0b_PostMenuTimer ; $0b
    dw GameB_State0c_SceneLoadGraphics ; $0c
    dw GameB_State0d_SceneInit0 ; $0d
    dw GameB_State0e_SceneInit1 ; $0e
    dw GameB_State0f_SceneD091Loop ; $0f
    dw GameB_State10_PrepPlay ; $10
    dw GameB_State11_D008Dispatch ; $11
    dw GameB_State12_ResetCounters ; $12
    dw GameB_State13_D009InputWait ; $13
    dw GameB_State14_PerFrameEngine ; $14
    dw GameB_State15_PrePlayCountdown ; $15
    dw GameB_State16_LevelClearGate ; $16
    dw GameB_State17_EngineTail ; $17
    dw GameB_State18_HitGate ; $18
    dw GameB_State14_PerFrameEngine ; $19
    dw GameB_State1a_KillBookkeeping ; $1a
    dw GameB_State1b_EndClear ; $1b
    dw GameB_State1c ; $1c
    dw GameB_State1d_Underwater ; $1d
    dw GameB_State1e_SceneEndAnim ; $1e
    dw GameB_State1f_SceneEndTimer ; $1f
    dw GameB_State20_SceneEndPause ; $20
    dw GameB_State21_Verdict ; $21
    dw GameB_State22_GameOver ; $22
    dw GameB_State23_NextScene ; $23
    dw GameB_State24_TrampolineTo3ace ; $24
    dw GameB_State25 ; $25
    dw GameB_State26_CutsceneDispatchD826 ; $26
    dw GameB_State27_SelectPath2801 ; $27
    dw GameB_State28_CutsceneD02eGate ; $28
GameB_SubStateTable_29: ; mid-table label, entry $29 at $3000
    dw GameB_State29_CutsceneStep533a ; $29
    dw GameB_State2a_CutsceneStep532e ; $2a
    dw GameB_State2b_SoftReset ; $2b
    dw GameB_State2c_PostKillD091 ; $2c
    dw GameB_State2d_GameOverVerdict ; $2d

GameB_PerFrameJumpTable_d008:
    dw GameB_State2e_D02eSamePriorityGate ; $2e
    dw GameB_State2f_LoadSubroutine ; $2f
    dw GameB_State30_CheckSubKill ; $30
    dw GameB_PlayerHit ; $31
GameB_PerFrameJumpTable_d009:
    dw GameB_State32_Engine32 ; $32
    dw GameB_State33_Engine33 ; $33
    dw GameB_State34_Engine34 ; $34
    dw GameB_State35_Engine35 ; $35
    dw GameB_State36_Engine36 ; $36
    dw GameB_State37_Engine37 ; $37
    dw GameB_State38_Engine38 ; $38
    dw GameB_State39_Engine39 ; $39

    ; Engine state $32: bank 3 steps, then the engine at $4809.
GameB_State32_Engine32:
    call GameB_UpdateEnemyShips ; bank 3
    call GameB_State34_Engine34.resetScroll ; skip its first three calls
    jp GameB_StepDelay ; bank 3

    ; Engine state $34: move, collide and draw everything.
GameB_State34_Engine34:
    call GameB_HudBlinkUpdate ; bank 3
    call GameB_RunObjectHandlers ; bank 3
    call GameB_UpdateMG ; bank 3
.resetScroll:
    call GameB_ResetScrollCursors ; bank 3
    call GameB_UpdateTorpedoes
    call GameB_EnemyObjectScan
    call GameB_MoveMgTrails ; bank 3
    and a
    jp GameB_Advance_d009


    ; Engine state $33: torpedo engine step.
GameB_State33_Engine33:
    call GameB_UpdateEnemyObjects ; bank 3
    ld c, $0a
    call GameB_AnimateWaves ; bank 3
    call GameB_FireTorpedo

    and a

; Shared tail of engine states $33 and $35. In: carry as for
; GameB_Advance_d009.
GameB_Engine33Tail:
    call GameB_Advance_d009
    call GameB_MoveMgTrails.trailB ; bank 3
    call GameB_UpdateD2F7Counter

    jp GameB_ClearSpriteTails ; bank 3


    ; Engine state $35: machine gun engine step.
GameB_State35_Engine35:
    call GameB_UpdatePlayerOrLink
    call GameB_FireMachineGun
    scf
    call GameB_Engine33Tail
    call GameB_DrawEdgeWarnings
    jp GameB_TickGameClock ; bank 3


    ; Engine state $36: play sound $0a at times, then the shared tail.
GameB_State36_Engine36:
    ldh a, [g_frameCounter]
    and $7c ; bits 2-6 of the frame counter
    ld a, $0a
    call z, PlaySFX

    call GameB_UpdateEnemyShips ; bank 3
    call GameB_DrawPlayerSlots ; bank 3
    jr GameB_Engine38Tail

    ; Engine state $38: bank 3 steps, then the shared tail.
GameB_State38_Engine38:
    call GameB_HudBlinkUpdate ; bank 3
    call GameB_RunObjectHandlers ; bank 3
    call GameB_DrawEnemySlots ; bank 3

; Shared tail of engine states $36 and $38.
GameB_Engine38Tail:
    and a

    call GameB_Advance_d009
    call GameB_DrawSonarSweep
    call GameB_FireTorpedo
    jp GameB_UpdateTorpedoes


    ; Engine state $37: count g_d2f7Counter, advance, then run the
    ; bank 3 steps at $4b2b and $4e17.
GameB_State37_Engine37:
    call GameB_UpdateD2F7Counter
    and a
    call GameB_Advance_d009
    call GameB_UpdateEnemyObjects ; bank 3
    jp GameB_DrawPlayerSlotsD1c0 ; bank 3


    ; Engine state $39: run the bank 3 steps, fire the machine gun and
    ; set g_subStateScratch9 = 4.
GameB_State39_Engine39:
    call GameB_UpdatePlayerOrLink
    call GameB_DrawEnemySlotsD258 ; bank 3
    call GameB_FireMachineGun
    ld a, $04
    ld [g_subStateScratch9], a
    call GameB_CheckSubRange ; engine state $36
    jp GameB_TickGameClock ; bank 3
 ; bank 3

    ; Step g_d2f7Counter. $d019 bit 5 counts it up to $11, bit 6
    ; counts it down to 0. At the end clears the bit and runs the
    ; bank 3 step at $4f87. $d011 bit 4 = counter full.
GameB_UpdateD2F7Counter:
    ld a, [g_subFireMode]
    bit 6, a ; bit 6 = count down
    jr nz, .skip

    bit 5, a ; bit 5 = count up
    ret z

    ld a, [g_d2f7Counter]
    inc a
    cp $11 ; count up to $11
    jr c, .skip3

    ld hl, g_enemySubFlags
    set 4, [hl]
    ld hl, g_subFireMode
    res 5, [hl]
    jp GameB_ClearFireFlags ; bank 3


.skip:
    ld hl, g_enemySubFlags
    res 4, [hl]
    ld a, [g_d2f7Counter]
    or a
    jr nz, .skip2

    ld hl, g_subFireMode
    res 6, [hl]
    jp GameB_ClearFireFlags ; bank 3


.skip2:
    dec a

.skip3:
    ld [g_d2f7Counter], a
    ret


    ; Run the sound driver in bank 4, then go back to bank 3.
GameB_TickSoundEngine:
    ld a, $04 ; sound driver bank
    call SwitchROMBank_FromA
    call SoundUpdate
    jp SwitchToBank3


    ; Game B state $14: the main play frame. Runs the sound, Start
    ; pauses, then runs the engine state and the screen effects.
GameB_State14_PerFrameEngine:
    call GameB_TickSoundEngine
    ld a, [g_joyEdge]
    bit 3, a ; Start pauses
    jp nz, GameB_TogglePause

    ld a, [g_sceneFlagsC]
    bit 3, a
    call z, GameB_WeaponAiTick ; bank 3, 1-player only

    ; Run the engine state in g_subStateScratch9.
GameB_SubDispatch_d009:
    ld a, [g_subStateScratch9]
    ld hl, GameB_PerFrameJumpTable_d009
    call DispatchJumpTable
    ; Game B state $11: screen effects (bank 6) and the
    ; g_subStateScratch8 per-frame state.
GameB_State11_D008Dispatch:
    ld a, $06
    call SwitchROMBank_FromA
    call Vec6_GameB_UpdateScreenEffects ; bank 6 vector -> GameB_UpdateScreenEffects
    ld a, $03
    call SwitchROMBank_FromA
    ld a, [g_subStateScratch8]
    cp $03
    jr nz, .skip3

    ld a, [g_sceneFlagsC]
    and $03
    cp $02 ; bit 1 set, bit 0 clear
    jr nz, .skip

    ld a, [g_subState]
    cp $1e
    jr c, .skip

    ldh a, [rDIV] ; skip on random half of frames
    and $01
    call z, Hud_WindowSlideTick
    ldh a, [g_frameCounter]
    and $01
    ret nz

    jr .skip2

.skip:
    call Hud_WindowSlideTick
    ld a, [g_subStateScratch8]
    ld hl, GameB_PerFrameJumpTable_d008
    call DispatchJumpTable

.skip2:
    ld a, [g_subStateScratch8]

.skip3:
    ld hl, GameB_PerFrameJumpTable_d008
    jp DispatchJumpTable


    ; Scan the 8-byte object slots at $d200 and handle each active
    ; one (bit 7). B = slot count: 4 when g_sceneFlagsC bit 1 is set,
    ; else $1b. Then runs the bank 3 step at $5111.
GameB_EnemyObjectScan:
    ld hl, g_modeFlags1b
    res 6, [hl]
    res 2, [hl]
    ld a, [g_sceneFlagsC]
    bit 1, a
    jr z, .skip

    ld b, $04
    jr .skip2

.skip:
    ld b, $1b
    call GameB_ResetSubApproach

.skip2:
    ld hl, g_gameBObjSlots ; object table, 8 bytes per slot
    ld de, $0008

; Test one slot. In: HL = slot, DE = 8, B = slots left.
GameB_ObjScan_Loop:
    bit 7, [hl] ; bit 7 = slot active
    jr nz, GameB_ObjScan_Active

; Go to the next slot. When none are left, finish the scan.
GameB_ObjScan_Next:
    add hl, de
    dec b
    jr nz, GameB_ObjScan_Loop

    ld hl, g_sceneFlagsC
    bit 1, [hl]
    jr nz, .skip

    ld hl, g_modeFlags1b
    bit 6, [hl]
    jr nz, .skip2

.skip:
    ld a, $60
    ld [g_rasterScx3], a

.skip2:
    jp GameB_Sub_5111 ; bank 3


; Slot is active. Save its type byte in $d3ff. A dead slot is
; cleared; a live one goes to GameB_EnemyObjectDispatch.
GameB_ObjScan_Active:
    push bc
    ld a, [hl+]
    ld [g_scanSlotType], a
    ld c, l
    inc l
    inc l
    inc l
    bit 7, [hl]
    ld l, c
    jr z, GameB_EnemyObjectDispatch

.loop:
    dec l
    ld [hl], $00
    jr GameB_ObjScan_ContinueDE

    pop hl
    jr .loop

; Done with this slot. Restore the scan registers and go on.
GameB_ObjScan_Continue:
    pop hl
    dec l

; Same, with BC and HL already restored.
GameB_ObjScan_ContinueDE:
    pop bc
    ld de, $0008
    jr GameB_ObjScan_Next

    ; Handle one active object slot by its type. In: HL = slot.
GameB_EnemyObjectDispatch:
    push hl
    ld a, [hl+]
    ld [g_scanSlotClass], a
    and $70
    swap a
    ld c, a
    ld b, d
    ld a, [hl+]
    ld d, [hl]
    ld e, a
    push bc
    ld hl, GameB_EnemyClassRankTable ; bank 3 table
    add hl, bc
    ld a, [hl]
    ld [g_spriteDrawMode], a
    sla c
    ld hl, GameB_EnemyClassHandlerPtrs ; bank 3 table
    add hl, bc
    ld a, [hl+]
    ld b, [hl]
    ld c, a
    call GameB_RangeCheckPlayer ; bank 3
    pop bc
    jr nc, .skip2

    ld a, [g_scanSlotClass]
    and $70
    jp nz, .skip

    ld a, [g_sceneFlagsC]
    bit 1, a
    jr z, GameB_ObjScan_Continue

    ld a, [g_subState]
    cp $1b
    jr nc, .skip2

    jr GameB_ObjScan_Continue

.skip:
    cp $70
    jp nz, GameB_ObjScan_Continue

    pop hl
    push hl
    inc l
    inc l
    inc l
    inc l
    inc l
    bit 3, [hl]
    ld c, $00
    ld a, $06
    call nz, HudQueue_Push
    res 3, [hl]
    jr GameB_ObjScan_Continue

.skip2:
    inc c
    inc c
    call Divide16
    ld d, a
    ld a, [g_scanSlotClass]
    call GameB_LookupData6e78 ; bank 3
    ld [g_objShapeAttr], a
    pop hl
    push hl
    inc l
    inc l
    inc l
    inc l
    bit 7, [hl]
    jp nz, GameB_DrawShapedObj

    ld hl, GameB_SpriteOffsetTable + $4c ; bank 3 table
    add hl, bc
    ld e, [hl]
    ld a, [g_scanSlotClass]
    call GameB_AdjustYByDepth ; bank 3
    call GameB_MirrorFromTable7121 ; bank 3
    ld b, $00
    ld hl, g_scaledTable
    add hl, bc
    ld a, [hl]
    or a
    jr z, .skip3

    pop hl
    push hl
    call GameB_EnemyFireGate

.skip3:
    ld a, c
    cp $19
    jr c, .skip8

    jr z, .skip6

    sub $19
    ld c, a
    cp $09
    jp z, GameB_ObjKind22

    ld hl, g_modeFlags1b
    cp $06
    jr c, .skip4

    cp $08
    jr nc, .skip4

    set 2, [hl]

.skip4:
    set 6, [hl]
    cp $08
    jr nz, .skip5

    dec d
    dec d

.skip5:
    ld a, [g_subState]
    cp $16
    jp nc, GameB_ObjScan_Continue

    ld a, $08
    sub d
    ld [g_rasterScx3], a

    ldh a, [g_frameCounter]
    and $06
    jp nz, GameB_ObjScan_Continue

    ld a, $06
    call HudQueue_Push
    pop hl
    push hl
    inc l
    inc l
    inc l
    inc l
    inc l
    set 3, [hl]
    jp GameB_ObjScan_Continue


.skip6:
    pop hl
    push hl
    inc l
    inc l
    inc l
    inc l
    ld a, [hl]
    ld c, a
    and $c0
    ld b, a
    ld a, c
    inc a
    and $3f
    ld c, a
    or b
    ld [hl], a
    ld a, c
    srl a
    srl a
    srl a
    and $03
    jp z, GameB_ObjScan_Continue

    dec a
    cp $02
    jr c, .skip7

    sub a

.skip7:
    add $2b
    ld c, a
    jr .skip9

.skip8:
    ld a, c
    or a
    jr z, .skip12

.skip9:
    ld a, [g_objShapeAttr]
    and $c0
    jr z, .skip11

    ld hl, g_scanSlotType
    sla a
    jr c, .skip10

    bit 5, [hl]
    jr nz, .skip11

    dec c
    jr .skip11

.skip10:
    bit 5, [hl]
    jr z, .skip11

    inc c

.skip11:
    call GameB_GetTable71c9 ; bank 3
    call GameB_DrawObjMeta
    jp GameB_ObjScan_Continue


.skip12:
    call GameB_EnemySub_TickInit
    ld hl, g_modeFlags1d
    bit 6, [hl]
    call nz, GameB_EnemySub_ApproachAI
    ld a, [g_shipArray_Enemy]
    bit 6, a
    jp z, GameB_EnemySub_PickFrame

    push de
    call GameB_EnemySub_DrawFireFlash
    pop de
    ld a, e
    sub $08
    ld e, a
    ld a, [g_enemySubFirePhase]
    and $70
    swap a
    cp $04
    jr c, .skip13

    ld hl, g_shipArray_Enemy + $4
    set 7, [hl]
    jp GameB_ObjScan_Continue


.skip13:
    ld b, $1c
    ld c, $22
    call GameB_SelectByD011 ; bank 3
    add c
    ld c, a
    jp GameB_EnemySub_Draw


; Step the enemy sub fire phase every 4 frames (sound 1 at phase
; $0c), and draw the muzzle flash frame ($26-$2a) while it lasts.
GameB_EnemySub_DrawFireFlash:
    ld a, d
    add $0c
    ld d, a
    ldh a, [g_frameCounter]
    and $0c
    ld hl, g_enemySubFirePhase
    jr z, .skip

    inc [hl]
    ld a, [hl]
    and $7f
    cp $0c
    ld a, $01
    call z, PlaySound_DEB8

.skip:
    ld a, [hl]
    and $38
    srl a
    srl a
    srl a
    add $26
    cp $2b
    ret nc

    ld c, a
    call GameB_GetTable71c9
    jp GameB_DrawObjMeta


; Pick the enemy sub sprite frame from g_d2f7Counter. Skips the draw
; when $d011 bit 4 is set or the frame is past $0e.
GameB_EnemySub_PickFrame:
    ld hl, g_enemySubFlags
    bit 4, [hl]
    jp nz, GameB_ObjScan_Continue

    ld a, [g_d2f7Counter]
    or a
    jr nz, .skip

    ld c, $19
    call GameB_SelectByD011 ; bank 3
    jr GameB_EnemySub_Draw

.skip:
    ld b, $1a
    ld c, $20
    call GameB_SelectByD011 ; bank 3
    cp $08
    jr c, GameB_EnemySub_Draw

    inc c
    cp $0f
    jp nc, GameB_ObjScan_Continue

; Draw the enemy sub. In a won 1-player scene also draws the extra
; marker. Then draws the $d42c periscope if bit 7 is set.
GameB_EnemySub_Draw:
    call GameB_GetTable71c9 ; bank 3
    push af
    ld a, [g_subState]
    cp $18
    jr c, .skip

    ld a, [g_sceneFlagsC]
    bit 0, a
    jr z, .skip

    ld a, [g_sceneFlagsB]
    bit 5, a
    jr z, .skip

    push de
    push bc
    ld bc, $131c ; two offsets, picked by $d011 bit 0; not an address
    call GameB_SelectByD011 ; bank 3
    ld a, d
    add c
    ld d, a
    inc e
    inc e
    ld bc, $363a ; two offsets, picked by $d011 bit 0; not an address
    call GameB_SelectByD011 ; bank 3
    ldh a, [g_frameCounter]
    and $30
    swap a
    add c
    ld c, a
    ld a, $01
    call GameB_DrawObjMeta2
    pop bc
    pop de

.skip:
    ld hl, g_periscopeFlags
    bit 7, [hl]
    jr z, .skip4

    push de
    push bc
    ld a, [hl]
    and $3f
    add d
    ld d, a
    ld c, $3e
    bit 6, [hl]
    jr z, .skip2

    inc c
    res 7, [hl]
    res 6, [hl]
    jr .skip3

.skip2:
    set 6, [hl]

.skip3:
    ld a, e
    add $0a
    ld e, a
    ld a, $01
    call GameB_DrawObjMeta2
    pop bc
    pop de

.skip4:
    pop af
    call GameB_DrawObjMeta2
    jp GameB_ObjScan_Continue


; Draw the object metasprite twice: once at the given place, once
; with the $d3f8 offset. In: A = frame, DE = position.
GameB_DrawObjMeta2:
    call GameB_DrawObjMeta
    push bc
    ld b, c
    ld hl, g_tallyIconGrow
    ld a, [hl]
    jr GameB_DrawObjMetaAt

; Draw the object metasprite. In: A = frame, DE = position.
; Stores the frame in g_tallyIconGrow.
GameB_DrawObjMeta:
    ld b, a
    ld [g_tallyIconGrow], a
    push bc
    ld b, c

; Shared tail: find a free sprite slot and draw the bank 6
; metasprite at $4b4a.
GameB_DrawObjMetaAt:
    call Oam_AllocSprites ; bank 3
    ld [g_metaspriteOamSlot], a
    ld c, b
    push de
    ld hl, GameB_ObjMetaspritePtrs + 6 ; bank 6 metasprite
    call Bank6_DrawMetasprite
    pop de
    pop bc
    ret


; Draw an object whose type byte has bit 7 set. Looks up its shape
; in bank 3 tables and its frame list in the bank 6 table at $4b1c.
; Frame 8 means the object is gone.
GameB_DrawShapedObj:
    dec l
    ld a, [hl+]
    ld [g_sixAnimCounter1], a
    push hl
    ld hl, GameB_SpriteOffsetTable + $2a ; bank 3 table
    add hl, bc
    ld a, [hl]
    push af
    ld a, [g_scanSlotClass]
    ld e, a
    and $70
    swap a
    ld c, a
    ld hl, GameB_SpriteOffsetTable + $66 ; bank 3 table
    add hl, bc
    ld a, e
    ld e, [hl]
    call GameB_AdjustYByDepth ; bank 3
    ld hl, GameB_SpriteOffsetTable + $22 ; bank 3 table
    call GameB_MirrorFromTableHL ; bank 3
    pop af
    pop hl
    inc l
    inc l
    bit 7, [hl]
    jr z, .skip

    swap a

.skip:
    bit 5, [hl]
    jr z, .skip2

    ld a, $02

.skip2:
    and $0f
    sla a
    ld c, a
    ld b, $00
    ld hl, MetaspritePtrs2 ; bank 6 table
    add hl, bc
    ld a, $06 ; read the table from bank 6
    call SwitchROMBank_FromA
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    push hl
    ld a, c
    cp $08
    jr nz, .skip3

    call SwitchToBank3
    pop hl
    call GameB_DrawDepthCharges
    jp GameB_ObjScan_Continue


.skip3:
    ld a, [g_sixAnimCounter1]
    and $f0
    swap a
    ld c, a
    add hl, bc
    push af
    ld c, [hl]
    call SwitchToBank3
    ld a, c
    call Oam_AllocSprites ; bank 3
    ld [g_metaspriteOamSlot], a
    pop af
    pop hl
    ld bc, $0009
    add hl, bc
    ld c, a
    call Bank6_DrawMetasprite
    jp GameB_ObjScan_Continue


; Handle object kind $22. Ignored after sub-state $1c.
GameB_ObjKind22:
    ld a, [g_subState]
    cp $1c
    jp nc, GameB_ObjScan_Continue

    pop hl
    push hl
    inc l
    inc l
    inc l
    push hl
    ld a, [hl+]
    ld c, a
    ld a, [hl+]
    sla c
    sla c
    push af
    push bc
    bit 4, [hl]
    jr nz, .skip

    set 4, [hl]
    jr .skip2

.skip:
    bit 5, [hl]
    jr nz, .skip4

    ld a, d
    cp $a0
    jr nc, .skip4

.skip2:
    push hl
    ld hl, g_kind22ActiveCount
    ld a, [hl]
    cp $02
    jr nc, .skip3

    inc [hl]
    pop hl
    set 5, [hl]
    jr .skip4

.skip3:
    pop hl

.skip4:
    ld a, [hl+]
    bit 5, [hl]
    ld hl, GameB_RasterTables + $2c ; bank 3 table
    jr z, .skip5

    ld hl, GameB_RasterTables + $b4 ; bank 3 table, alt set

.skip5:
    pop bc
    pop af
    add hl, bc
    cp [hl]
    jr c, .skip6

    pop bc
    ld a, [bc]
    inc a
    ld [bc], a
    inc hl
    inc hl
    inc hl
    inc hl
    jr .skip7

.skip6:
    pop bc
    cp $39
    jr nz, .skip7

    ld a, d
    cp $a8
    ld a, $1c
    call c, PlaySFX ; sound $1c

.skip7:
    inc hl
    ld a, [hl+]
    ld e, a
    ld a, [hl+]
    ld c, a
    ld b, [hl]
    ld a, [g_modeFlags10]
    bit 6, a
    jr z, .skip8

    ld a, b
    and $f0
    push bc
    ld b, $00
    call GameB_AdjustYByDepth ; bank 3
    pop bc

.skip8:
    pop hl
    push hl
    push de
    dec l
    ld a, [hl]
    and $e0
    ld e, a
    ld a, b
    and $0f
    or e
    ld e, a
    bit 7, b
    jr z, .skip10

    ld a, d
    cp $58
    jr c, .skip9

    set 5, e
    cp $78
    jr c, .skip10

    jr .skip10

.skip9:
    res 5, e
    cp $30
    jr nc, .skip10

    jr .skip10

.skip10:
    ld a, e
    ld [hl+], a
    ld [ScrollScript_LastTileByte], a
    ld a, [hl]
    and $8f
    ld e, a
    ld a, b
    and $70
    or e
    ld [hl+], a
    ld a, c
    ld b, a
    and $0f
    ld c, a
    inc l
    inc l
    inc l
    inc l
    cp $08
    jp nc, .skip13

    cp $06
    jr z, .skip11

    cp $07
    jr nz, .skip12

    ld a, [ScrollScript_LastTileByte]
    bit 5, a
    jr nz, .skip12

    inc c
    jr .skip12

.skip11:
    ld c, $05
    ld b, $40
    bit 7, [hl]
    jr nz, .skip12

    set 7, [hl]
    push bc
    push hl
    call GameB_FindFreeSlotD298 ; bank 3
    pop hl
    pop bc
    ld a, $03
    call nc, PlaySFX

.skip12:
    pop de
    push bc
    ld b, $00
    ld hl, GameB_SpriteParamTable + $5b ; bank 3 table
    add hl, bc
    ld a, [hl]
    add d
    ld d, a
    pop bc
    ld a, c
    add $2d
    ld c, a
    ld a, b
    and $f0
    swap a
    call GameB_DrawObjMeta2
    jp GameB_ObjScan_Continue


.skip13:
    cp $09
    jr nz, .skip14

    call GameB_EnemyShipFire ; bank 3
    push bc
    ld b, $00
    inc hl
    ld a, [g_enemyBulletSpeed]
    and $70
    add $10
    or $80
    ld c, a
    ld e, l
    ld d, h
    call GameB_MakeWreck ; bank 3
    inc de
    sub a
    ld [de], a
    pop bc

.skip14:
    pop de
    ld a, d
    sub $04
    ld d, a
    ld c, $33
    ld a, $02
    call GameB_DrawObjMeta2
    jp GameB_ObjScan_Continue


; Draw the radar blip for a torpedo slot on the scope. Picks the
; size from the depth and the frame from the bank 3 tables, then
; draws the bank 6 metasprite at $4b34.
; In: HL = slot. Uses $d401, g_enemyFireGate.
GameB_DrawTorpedoShadowA:
    push hl
    call GameB_ScaledMirrorOffset ; bank 3
    pop hl
    ld a, [hl-]
    and $d0
    cp $c0
    jr nz, .skip7

    push bc
    ld a, [hl]
    ld b, a
    and $0f
    ld c, a
    ld a, b
    and $f0
    add $10
    ld b, a
    ld a, [g_enemyFireGate]
    ld e, a
    ld a, $07
    sub e
    swap a
    cp b
    jr nc, .skip

    ld [hl], c
    pop bc
    push de
    ld a, d
    ld de, -5
    cp $40
    jr c, .skip3

    cp $70
    jr nc, .skip2

    set 4, [hl]
    add hl, de
    ld a, [hl]
    and $e0
    or $08
    jr .skip6

.skip:
    ld a, b
    or c
    ld [hl], a
    pop bc
    jr .skip7

.skip2:
    add hl, de
    res 5, [hl]
    jr .skip4

.skip3:
    add hl, de
    set 5, [hl]

.skip4:
    ld a, [hl]
    ld d, a
    and $e0
    ld e, a
    ld a, d
    and $1f
    inc a
    cp $1f
    jr c, .skip5

    ld a, $1f

.skip5:
    or e

.skip6:
    ld [hl], a
    pop de

.skip7:
    ld a, [g_dispatchTableD346]
    add $20
    ld e, a
    ld b, $00
    ld a, [g_playerShipsRemaining]
    cp $40
    ret nc

    ld a, [g_modeFlags10]
    bit 6, a
    jr z, .skip8

    call GameB_DepthBand
    call GameB_AdjustYByDepth ; bank 3

.skip8:
    ld a, $54
    sub c
    ld hl, GameB_DepthBandThresholds + $11 ; bank 3 threshold list
    call GameB_FindRangeIndex
    ld a, [g_modeFlags10]
    bit 6, a
    jr z, .skip9

    ld b, $00
    ld hl, GameB_DepthBandThresholds ; bank 3 table
    add hl, bc
    ld a, [hl]
    add e
    ld e, a

.skip9:
    ld a, [g_firedTorpedoSlot]
    and $60
    cp $60
    jr nz, .skip10

    ld c, $00
    ld a, [g_dispatchTableD346]
    add $20
    ld e, a

.skip10:
    ld hl, GameB_SpriteParamTable + $51 ; bank 3 table
    add hl, bc
    ld a, [hl]
    ld b, c
    call Oam_AllocSprites ; bank 3, find a free sprite slot
    ld [g_metaspriteOamSlot], a
    ld c, b
    ld hl, GameB_TorpedoShadowAMetaspritePtrs ; bank 6 metasprite
    jp Bank6_DrawMetasprite


; Draw the other torpedo blip (bank 6 metasprite $4b24). Not drawn
; when the scene is over (g_sceneFlagsC bit 0) or off screen.
GameB_DrawTorpedoShadowB:
    call GameB_ScaledMirrorOffset ; bank 3
    ld a, $7f
    sub c
    cp $88
    ret nc

    ld e, a
    ld b, $00
    ld a, [g_sceneFlagsC]
    bit 0, a
    ret nz

    ld a, [g_modeFlags10]
    bit 6, a
    jr z, .skip

    call GameB_DepthBand
    call GameB_AdjustYByDepth ; bank 3

.skip:
    ld hl, GameB_DepthBandThresholds + 9 ; bank 3 threshold list
    call GameB_FindRangeIndexB
    ld hl, GameB_SpriteParamTable + $4c ; bank 3 table
    add hl, bc
    ld a, [hl]
    ld b, c
    call Oam_AllocSprites ; bank 3, find a free sprite slot
    ld [g_metaspriteOamSlot], a
    ld c, b
    ld hl, GameB_TorpedoShadowBMetaspritePtrs ; bank 6 metasprite
    jp Bank6_DrawMetasprite


; Set B from HL first, then fall into GameB_FindRangeIndex.
GameB_FindRangeIndexB:
    ld a, c

; Count entries in the list at HL that are <= A.
; In: A = value, HL = sorted list. Out: C = index.
GameB_FindRangeIndex:
    ld c, $00

.loop:
    cp [hl]
    ret c

    inc hl
    inc c
    jr .loop

; Out: A = min($d347 + 3, 7) << 4.
GameB_DepthBand:
    ld a, [g_depthBand]
    add $03
    cp $07 ; cap at 7
    jr c, .skip

    ld a, $07

.skip:
    swap a
    ret


; Move, collide and draw all 8 torpedo slots (g_torpedoSlots,
; 8 bytes each, bit 7 = in use). Skipped after sub-state $1c.
; A torpedo is freed when its range passes $90.
GameB_UpdateTorpedoes:
    ld a, [g_subState]
    cp $1c
    ret nc ; no torpedoes after sub-state $1c

    sub a
    ld [g_spriteDrawMode], a
    ld hl, g_torpedoSlots ; 8 slots, 8 bytes each
    ld de, $0008
    sub a

.loop:
    ld [g_torpedoSlotIdx], a
    bit 7, [hl] ; bit 7 = slot in use
    jr nz, .skip

.loop2:
    add hl, de
    ld a, [g_torpedoSlotIdx]
    inc a
    cp $08 ; 8 slots
    jr c, .loop

    ret


.skip:
    push hl
    call GameB_ProjectileTick
    call GameB_ProjectileCollide
    push hl
    ld a, [g_dispatchTableD346]
    bit 7, [hl]
    ld hl, g_oamBufToggle ; $d014 bit 6 = hit this frame
    jp nz, .skip8

    bit 6, [hl]
    jr z, .skip2

.loop3:
    res 6, [hl]
    pop hl
    jp .skip7


.skip2:
    cp $38
    jp c, .skip3

    sub $38
    sla a
    ld hl, GameB_TorpedoRangeDistances ; bank 3 table
    ld c, a
    ld b, $00
    add hl, bc
    ld a, [hl+]
    ld b, [hl]
    ld c, a
    call GameB_RangeCheckPlayer ; bank 3
    jr c, .skip3

    call GameB_ShiftDELeft4 ; bank 3
    ld a, [g_dispatchTableD346]
    sub $38
    ld c, a
    ld a, [g_modeFlags10]
    bit 4, a
    call z, GameB_DrawTorpedoShadowB

.skip3:
    pop hl
    dec hl
    dec hl
    ld a, [hl]
    or a
    jr z, .skip4

    dec [hl]
    jp nz, .skip7

.skip4:
    dec hl
    dec hl
    dec hl

.loop4:
    inc [hl]
    ld a, [hl]
    cp $90 ; range $90 = gone
    jr nc, .skip6

    push hl
    ld hl, GameB_TorpedoRangeDelays ; bank 3 table
    ld e, a
    ld d, $00
    add hl, de
    ld a, [hl]
    or a
    jr nz, .skip5

    pop hl
    jr .loop4

.skip5:
    pop hl
    inc hl
    inc hl
    inc hl
    ld [hl], a
    jr .skip7

.skip6:
    dec hl
    ld [hl], $00

.skip7:
    pop hl
    ld de, $0008
    jp .loop2


.skip8:
    bit 6, [hl]
    jp nz, .loop3

    cp $58
    jp nc, .skip9

    pop hl
    push hl
    ld c, a
    ld a, [hl]
    ld [g_firedTorpedoSlot], a
    ld a, $58
    sub c
    sla a
    ld hl, GameB_TorpedoRangeDistances ; bank 3 table
    ld c, a
    ld b, $00
    add hl, bc
    ld a, [hl+]
    ld b, [hl]
    ld c, a
    call GameB_RangeCheckPlayer ; bank 3
    jr c, .skip9

    call GameB_ShiftDELeft4 ; bank 3
    ld a, [g_dispatchTableD346]
    cp $58
    jr nc, .skip9

    ld hl, g_dispatchTableD346
    ld a, $58
    sub [hl]
    ld c, a
    ld hl, g_modeFlags10
    bit 4, [hl]
    pop hl
    push hl
    call z, GameB_DrawTorpedoShadowA

.skip9:
    pop hl
    bit 6, [hl]
    jr z, .skip11

    bit 5, [hl]
    jr z, .skip11

    inc hl
    ld a, [hl]
    or a
    jr z, .skip10

    dec [hl]
    jp nz, .skip7

    dec hl
    res 5, [hl]
    ld a, $06
    call PlaySFX
    jp .skip7


.skip10:
    dec hl

.skip11:
    dec hl
    dec hl
    ld a, [hl]
    or a
    jr z, .skip12

    dec [hl]
    jp nz, .skip7

.skip12:
    dec hl
    dec hl
    dec hl

.loop5:
    inc [hl]
    ld a, [hl]
    cp $90 ; range $90 = gone
    jr nc, .skip15

    ld e, a
    ld a, [g_firedTorpedoSlot]
    bit 6, a
    jr z, .skip13

    ld a, e
    cp $5c
    jr nc, .skip15

.skip13:
    ld a, $90
    sub [hl]
    push hl
    ld hl, GameB_TorpedoRangeDelays ; bank 3 table
    ld e, a
    ld d, $00
    add hl, de
    ld a, [hl]
    or a
    jr nz, .skip14

    pop hl
    jr .loop5

.skip14:
    pop hl
    inc hl
    inc hl
    inc hl
    jr .skip16

.skip15:
    dec hl
    sub a
    ld [hl], a
    ld bc, $0006
    add hl, bc

.skip16:
    ld [hl], a
    jp .skip7


; From sub-state $14 on, draw the left and right edge arrows that
; point at the enemy sub. Blinks them and plays the warning sound
; once (g_modeFlags18 bits 3 and 5). Clears the sprites when off.
GameB_DrawEdgeWarnings:
    ld a, [g_subState]
    cp $14 ; only from sub-state $14 on
    ret c

    ld hl, g_shipArray_Enemy + $2
    ld a, [hl+]
    ld d, [hl]
    ld e, a
    ld hl, g_worldWidth
    ld a, [hl+]
    ld b, [hl]
    push bc
    ld c, a
    srl b
    rr c
    call GameB_RangeCheckPlayer ; bank 3
    pop hl
    srl h
    ld a, d
    cp h
    ld d, $98 ; X = $98, right edge
    ld c, $01
    jr nc, .skip

    ld d, $10 ; X = $10, left edge
    dec c

.skip:
    ld a, [g_sceneFlagsC]
    bit 1, a
 ; bit 1 = sub killed
    jr nz, GameB_ClearEdgeArrows

    ld a, [g_killFlagsPending]
    bit 0, a ; bit 0 = enemy sub pending
    jr z, .skip2

    ld a, [g_enemySubApproachShadow]
    cp $08
    jr c, .skip2

    cp $a8
    jr c, .skip3

.skip2:
    ldh a, [g_frameCounter]
    and $40 ; blink every 64 frames
    jr z, GameB_EdgeWarnOff

    ld hl, g_modeFlags17
    res 3, [hl]
    bit 2, [hl]
    jr nz, .skip5

    set 2, [hl]
    jp .skip5


.skip3:
    ldh a, [g_frameCounter]
    and $08 ; blink every 8 frames
    jr z, GameB_ClearEdgeArrows

    ld hl, g_modeFlags18
    bit 5, [hl]
    jr nz, .skip4

    set 5, [hl]
    dec hl
    bit 3, [hl]
    jr nz, .skip4

    set 3, [hl]
    call GameB_Sub_5127 ; bank 3
    ld a, $0c ; warning sound
    call PlaySFX

.skip4:
    ld c, $00
    ld d, $10
    ld a, $4c
    call GameB_DrawEdgeArrow
    ld c, $01
    ld d, $98

.skip5:
    ld a, $4e

; Draw the bank 6 edge arrow metasprite at $4b44.
; In: A = sprite slot, C = attr, D = X. Uses Y = $14.
GameB_DrawEdgeArrow:
    ld [g_metaspriteOamSlot], a
    ld e, $14
    ld hl, GameB_ObjMetaspritePtrs ; bank 6 metasprite
    jp Bank6_DrawMetasprite


; Clear g_modeFlags17 bits 2 and 3, then fall into the next.
GameB_EdgeWarnOff:
    ld hl, g_modeFlags17
    res 2, [hl]
    res 3, [hl]

; Clear the edge arrow sprites at $c190 and $c198.
GameB_ClearEdgeArrows:
    ld hl, g_modeFlags18
    ld a, [hl]
    res 5, [hl]
    bit 5, a
    ld hl, g_oamBufferB + $90 ; shadow OAM sprite 50
    call nz, GameB_Clear2Sprites
    ld hl, g_oamBufferB + $98 ; shadow OAM sprite 51

; Clear 2 shadow OAM entries. In: HL = first entry.
GameB_Clear2Sprites:
    ld c, $02
    jp ClearSpriteEntries


; Step the $d35d sweep counter (up 0-8 or down 8-0 by
; g_modeFlags10 bit 0) and draw the bank 6 metasprite at $4b44.
GameB_DrawSonarSweep:
    ld hl, g_sonarSweepStep
    ld a, [hl]
    or a
    jr nz, .skip4

    ld a, [g_sideSpeed]
    ld c, a
    ld b, $00
    ld hl, GameB_SpriteParamTable + $2d ; bank 3 table
    add hl, bc
    ld a, [hl]
    ld hl, g_sonarSweepStep
    ld [hl+], a
    ld a, [g_modeFlags10]
    bit 0, a
    jr nz, .skip2

    ld a, [hl]
    or a
    jr z, .skip

    dec [hl]
    jr .skip5

.skip:
    ld [hl], $08 ; 8 steps
    jr .skip3

.skip2:
    inc [hl]
    ld a, [hl]
    cp $09 ; 9 steps
    jr c, .skip5

    ld [hl], $00

.skip3:
    jr .skip5

.skip4:
    dec [hl]
    inc hl

.skip5:
    ld a, [hl]
    sla a
    sla a
    add $08
    ld d, a
    ld e, $33
    sub a
    ld [g_metaspriteOamSlot], a
    ld c, $02
    ld hl, GameB_ObjMetaspritePtrs ; bank 6 metasprite
    jp Bank6_DrawMetasprite


; Get a free sprite slot, then fall into the next.
GameB_DrawDepthCharges:
    ld a, $05
    call Oam_AllocSprites ; bank 3, find a free sprite slot
    ld [g_metaspriteOamSlot], a

; Draw $d099/2 sprites from the bank 6 table at $460d. Blinks in
; play (sub-state >= $14).
GameB_DrawDepthChargesAt:
    ld hl, g_sixAnimCounter1
    ld c, [hl]
    srl c
    ld b, $05

.loop:
    push bc
    push de
    push bc
    ld c, b
    sla c
    ld b, $00
    ld hl, OffsetRowPtrs ; bank 6 table
    add hl, bc
    ld a, $06 ; read the table from bank 6
    call SwitchROMBank_FromA
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    pop bc
    ld b, $00
    add hl, bc
    ld a, [g_depthChargeOrigin]
    sub [hl]
    ld e, a
    ld bc, $000d
    add hl, bc
    ld a, [hl]
    add d
    ld d, a

    ld a, [g_subState]
    cp $14 ; sub-state $14 = playing
    jr c, .skip

    ldh a, [g_frameCounter]
    and $0c
    srl a
    srl a
    ld hl, MetaspritePtrs2 + $be
    jr .skip2

.skip:
    ld hl, MetaspritePtrs + $3a
    ld a, [g_sceneFlagsC]
    bit 2, a
    ld a, $0b
    jr z, .skip2

    inc a

.skip2:
    ld c, a
    call Bank6_DrawMetasprite
    pop de
    pop bc
    dec b
    jr nz, .loop

    ret


    ; Read the word at GameB_ObjTypeTablePtrs + 2 * A in bank 6. Leaves bank 6
    ; selected. Out: HL = word.
GetBank6Table5121Ptr:
    ld c, a
    sla c
    ld a, $06
    call SwitchROMBank_FromA
    ld hl, GameB_ObjTypeTablePtrs
    add hl, bc
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    ret


; Build the object type byte for the next level slot from the
; bank 6 table at GameB_ObjTypeTablePtrs. Out: $d3fa = flags.
GameB_SetupObjectSlot:
    push bc
    push hl
    push af
    ld [g_setupObjType], a
    call GameB_LookupData6e78 ; bank 3
    ld b, $00
    push bc
    ld a, [g_gameBLayoutVariant]
    call GetBank6Table5121Ptr
    pop bc
    dec c
    add hl, bc
    ld a, [hl]
    and $1f
    ld b, a
    set 7, b
    pop af
    ld c, a
    and $80
    srl a
    srl a
    or b
    pop hl
    ld [hl+], a
    ld a, c
    pop bc
    res 7, a
    ld [hl+], a
    ld [g_difficultyColumn], a
    ret


; Load the level for g_gameBSkill. Copies the group list from bank 6
; into $d466, shuffles each group in 1-player games, then fills the
; object slots. Copies the per-level limits to $d437 and $d42f.
; Ends in bank 6 with the ships placed.
GameB_LoadLevel:
    ld a, [g_gameBSkill] ; g_gameBSkill
    ld [g_gameBLayoutVariant], a ; level index
    ld a, $06
    call SwitchROMBank_FromA
    call Vec6_GameB_LoadLevelLayout ; bank 6 vector -> GameB_LoadLevelLayout
    ld hl, g_levelGroupList ; group list
    push hl

.loop:
    ld a, [de]
    ld [hl+], a
    inc de
    cp $ff ; $ff ends a group
    jr nz, .loop

    ld [g_slotInitByte], a
    pop hl
    push de
    push hl
    ld a, [g_sceneFlagsC]
    bit 3, a
    jr z, .skip

    ld l, e
    ld h, d
    jr .skip3

.skip:
    push hl
    call Vec6_CountGroupRun ; bank 6 vector -> CountGroupRun
    ld a, c
    or a
    jr nz, .skip2

.loop2:
    pop de
    ld a, [hl]
    cp $ff
    jr z, .skip3

    jr .skip

.skip2:
    pop hl
    push de
    ld e, l
    ld d, h

.loop3:
    push bc
    call Vec6_RandomMasked ; bank 6 vector -> RandomMasked
    ld b, $00
    add hl, bc
    ld a, [de]
    push af
    ld a, [hl]
    ld [de], a
    inc de
    pop af
    ld [hl], a
    ld l, e
    ld h, d
    pop bc
    dec c
    jr nz, .loop3

    jr .loop2

.skip3:
    pop de
    push hl
    ld b, $00
    ld hl, g_shipArray_Player ; slot table, 8 bytes per slot

.loop4:
    ld a, [de]
    cp $ff ; $ff ends the list
    jr z, .skip4

    push af
    call SwitchToBank3 ; the layout is read from bank 3
    pop af
    push af
    push de
    call GameB_SetupObjectSlot
    pop de
    call Vec6_GameB_InitObjectSlot ; bank 6 vector -> GameB_InitObjectSlot
    pop af
    ld [g_slotInitByte], a
    jr .loop4

.skip4:
    ld a, b
    dec a
    ld hl, g_hudCounterA ; bank 3
    ld [hl+], a
    ld [hl], a
    ld [g_gameBShipCountM1], a
    ld a, [ScrollScript_LastTileByte]
    sla a
    sla a
    ld [g_shipArray_Player + 3], a
    ld a, [g_sceneFlagsC]
    bit 3, a
    jr z, .skip5

    ld a, [g_gameBSkill]
    sla a
    ld c, a
    ld b, $00
    ld hl, GameB_SkillLevelRecordPtrs
    add hl, bc
    pop de
    pop de
    jr .skip6

.skip5:
    pop hl
    pop hl

.skip6:
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    ld de, g_weaponFlagsBase
    ld bc, $0010
    rst $10
    ld hl, RandomLimitTable
    ld de, g_randomLimits
    ld bc, $0002
    rst $10
    call Vec6_GameB_PlaceShips ; bank 6 vector -> GameB_PlaceShips

; Switch the ROM bank to 3.
SwitchToBank3:
    ld a, $03
    jp SwitchROMBank_FromA


; Far call: bank 6 $400f (draw Game B death tallies), then back to bank 3.
Bank6_DrawDeathTallies:
    ld a, $06
    call SwitchROMBank_FromA
    call Vec6_GameB_DrawDeathTallies ; bank 6 vector -> GameB_DrawDeathTallies
    jr SwitchToBank3_Tail

; Far call: bank 6 $400c (draw six animated sprites), then back to bank 3.
Bank6_DrawSixAnimSprites:
    ld a, $06
    call SwitchROMBank_FromA
    call Vec6_GameB_DrawSixAnimSprites ; bank 6 vector -> GameB_DrawSixAnimSprites
    jr SwitchToBank3_Tail

; Far call: bank 6 $4000 (draw a metasprite), then back to bank 3.
Bank6_DrawMetasprite:
    ld a, $06
    call SwitchROMBank_FromA
    call Vec6_DrawMetasprite ; bank 6 vector -> DrawMetasprite

; Shared tail of the bank 6 far calls: go back to bank 3.
SwitchToBank3_Tail:
    ld a, $03
    jp SwitchROMBank_FromA


    ; Switch the ROM bank. In: A = bank. Keeps a copy in HRAM.
SwitchROMBank_FromA:
    ldh [g_currentBankShadow], a
    ld [$2100], a
    ret


    ; Set SCY and SCX to 0 and clear g_sceneFlagsA bit 2.
GameB_ResetScroll:
    sub a
    ld hl, rSCY
    ld [hl+], a
    ld [hl], a
    ld hl, g_sceneFlagsA
    res 2, [hl]
    ret


    ; Hide the window (WY = $90, WX = 7).
HideWindow:
    ld a, $90
    ldh [rWY], a
    ld [g_windowYTarget], a
    ld a, $07
    ldh [rWX], a
    ret


    ; STAT raster effect for Game B: per-line SCX, SCY and LYC from
    ; the tables at $d0af, $d0bd and $d0a3.
GameB_StatRasterFx:
    ld a, [g_subStateScratch4]
    bit 1, a
    ret z

    ld b, $00
    ld hl, g_rasterLine
    inc [hl]
    ld a, [g_rasterCount]
    cp [hl]
    ld c, [hl]
    jr nc, .readTable

    ld [hl], b
    ld a, [g_d0dbCounter]
    ldh [rLYC], a
    jr .setScroll

.readTable:
    add hl, bc
    ld a, [hl]
    ldh [rLYC], a

.setScroll:
    ld hl, g_rasterScxTable
    add hl, bc
    ld a, [hl]
    ldh [rSCX], a
    ld hl, g_rasterScyTable
    add hl, bc
    ld a, [hl]
    ldh [rSCY], a
    ret


    ; Set g_subStateScratch4 with interrupts off. In: A = flags.
GameB_SetRasterFlags:
    di
    ld [g_subStateScratch4], a
    ei
    ret


    ; Turn the LCD off safely and stop the STAT raster effect.
    ; Masks the VBlank and STAT interrupts.
GameB_LcdOff:
    di
    call GameB_WaitVBlankLcdOff
    ld hl, g_gameALevel ; $ff8c bit 1 = STAT raster effect on
    res 1, [hl]
    ldh a, [rIE]
    and $fc ; mask off VBlank and STAT
    ldh [rIE], a
    ei
    ret


    ; Wait for VBlank, then turn the LCD off. Keeps IE in $ff9e and
    ; restores it after.
GameB_WaitVBlankLcdOff:
    ldh a, [rIE]
    ldh [g_savedIE], a
    res 0, a
    ldh [rIE], a

.waitVBlank:
    ldh a, [rLY]
    cp $90 ; wait for LY >= $90 (VBlank)
    jr c, .waitVBlank

    ld hl, rLCDC
    res 7, [hl]
    ldh a, [g_savedIE]
    ldh [rIE], a
    ret


    ; Enable the STAT raster effect and the VBlank, STAT, timer and
    ; serial interrupts.
GameB_EnableIrqs:
    ld hl, g_gameALevel
    set 1, [hl] ; $ff8c bit 1 = STAT raster effect on
    ldh a, [rIE]
    or $0f ; enable VBlank, STAT, timer, serial
    ldh [rIE], a
    ret


; Unknown tile-index table (no direct reference found).
Unused_TileIndexList:
    db $3c, $3d, $3d, $3f, $40, $41, $40, $41, $3f, $42, $43, $44, $43, $44, $42, $45
    db $46, $47, $46, $47, $45, $48, $49, $4a, $4b, $4c, $4d, $4e, $4f, $50, $51, $52
    db $53

; Text uses tiles $c0-$d9 for A-Z and $02 for space.
    PUSHC
    NEWCHARMAP RadarText
    CHARMAP " ", $02
    CHARMAP "A", $c0
    CHARMAP "B", $c1
    CHARMAP "C", $c2
    CHARMAP "D", $c3
    CHARMAP "E", $c4
    CHARMAP "F", $c5
    CHARMAP "G", $c6
    CHARMAP "H", $c7
    CHARMAP "I", $c8
    CHARMAP "J", $c9
    CHARMAP "K", $ca
    CHARMAP "L", $cb
    CHARMAP "M", $cc
    CHARMAP "N", $cd
    CHARMAP "O", $ce
    CHARMAP "P", $cf
    CHARMAP "Q", $d0
    CHARMAP "R", $d1
    CHARMAP "S", $d2
    CHARMAP "T", $d3
    CHARMAP "U", $d4
    CHARMAP "V", $d5
    CHARMAP "W", $d6
    CHARMAP "X", $d7
    CHARMAP "Y", $d8
    CHARMAP "Z", $d9

; Tilemap command lists for DrawTilemapScript: VRAM addr (hi, lo), then mode<<6|count,
; then data. Mode 0 = copy row, 1 = fill row, 2 = copy column, 3 = fill column.
; A $00 byte ends the list.
GameB_LevelSelectTilemap:
    db $98, $21, $12, "SELECT GAME LEVEL", $da
    db $98, $41, $52, $e1
    db $98, $a6, $08, "PRACTICE"
    db $99, $27, $06, "NORMAL"
    db $99, $a8, $04, "HARD"
    db $00

GameB_LevelSelectScoreTilemap:
    db $9a, $07, $06, $f1, "SCORE"
    db $99, $e1, $52, $11
    db $00

; Sprite strips for the sprite builder at BuildSpriteStrips.
GameB_LevelSelectSprites:
    db $54, $20, $02, $3b, $3a, $5c, $20, $02, $3d, $3c

GameB_LevelSelectSpritesA:
    db $60, $20, $42, $76

GameB_LevelSelectSpritesB:
    db $60, $80, $42, $78

GameB_LevelSelectSpritesC:
    db $50, $20, $42, $5d
    db $58, $20, $42, $5f
    db $50, $80, $42, $61
    db $58, $80, $42, $63

GameB_LevelSelectSpritesD:
    db $50, $20, $42, $61
    db $58, $20, $42, $63
    db $50, $80, $42, $5d
    db $58, $80, $42, $5f

; Level names, 8 tiles each, indexed by [g_gameBSkill].
GameB_LevelNames:
    db "PRACTICE"
    db " NORMAL "
    db "  HARD  "
    POPC

