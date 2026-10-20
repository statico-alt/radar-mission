; Disassembly of "rm.gb"
; This file was created with:
; mgbdis v3.0 - Game Boy ROM disassembler by Matt Currie and contributors.
; https://github.com/mattcurrie/mgbdis

; ROM bank 3. Game B (submarine battle) code and data: the HUD command
; queue, scroll scripts, objects, weapons, enemy AI, link cable messages,
; the sound request queue and the rank screen. Bank 0 maps this bank for
; most of Game B. The jp at $4000 goes to GameB_LinkUpdate.

SECTION "ROM Bank $003", ROMX[$4000], BANK[$3]

; Data that the C files read.
EXPORT HudStringPtrs, HudIconPtrs, KillMarkerColumnOffsets, Hud2x2IconTiles
EXPORT GameB_StatusPatternTiles, GameB_StatusScreenTilemap, GameB_StatusScreenStrings
EXPORT WeaponIconLowerHudCells, WeaponIconUpperHudCells, WeaponIconTiles
EXPORT HudPanelColumnOffsets, HudPanelTiles, HudPanelBlankTiles
EXPORT HudBarCellsA, HudBarCellsA_LinkLast, HudBarCellsA_Last
EXPORT HudBarCellsB, HudBarCellsB_LinkLast, HudBarCellsB_Last
EXPORT HudColumnFallbackData
EXPORT SetPalettes, PlaySFX, PlayMusic, PlaySound_DEB0, PlaySound_DEB8, PlaySound_DEC8
EXPORT GameB_TogglePause
EXPORT GameB_WaveStepSizes, GameB_DamageGaugeSteps, GameB_DamageStepTable, GameB_RasterTables
EXPORT GameB_ObjSpriteTiles, GameB_ObjBaseYTable, GameB_DepthYOffsetPtrs
EXPORT GameB_MgTrailStepsA, GameB_MgTrailStepsB
EXPORT GameB_UnknownCurve6e10, GameB_TorpedoRangeDelays
EXPORT GameB_HitRangeBounds, GameB_SpriteHitWidths, GameB_DepthScaleDivisors, GameB_ObjSpriteIdPtrs
EXPORT ScrollScript_Scene0_SideA, ScrollScript_Scene1, ScrollScript_Scene2, ScrollScript_Scene3
EXPORT Rank_FrameStrings, Rank_RowDotTiles, GameB_RankStrings, GameB_RankText_NoRank
EXPORT GameB_RankStringOffsets, GameB_RankMedalLayout, Rank_FramePattern, Rank_LycTable
EXPORT Rank_ScyTable, Rank_ScxTable, Rank_StarCountInit, Rank_StarRecordsInit, Rank_LabelStrings
EXPORT Rank_SpriteStrips, Rank_RevealOrder, Rank_CongratulationsTiles, Rank_StarStripTable
EXPORT Rank_StarStripTimes, Rank_State4SpritesA, Rank_State4SpritesB, Rank_EndSprites
EXPORT TitleCutscene_LycTable, TitleCutscene_ScyTable, TitleCutscene_StarInit, TitleCutscene_ObjMotion
EXPORT TitleCutscene_PatternA, TitleCutscene_PatternB, TitleCutscene_PatternC
EXPORT TitleCutscene_SpritesA, TitleCutscene_SpritesB, TitleCutscene_SpritesC
EXPORT GameB_RankThresholds, LevelSelect_PushStartSprites

Vec3_GameB_LinkUpdate:
    jp GameB_LinkUpdate

; Glue to the bank 11 C code. FAR11 calls a C function with no arguments.
; FAR11_AC passes a and c as two byte arguments. Both keep all registers.
MACRO FAR11
    push af
    push bc
    push de
    push hl
    ld hl, \1
    jp FarCall11
ENDM
MACRO FAR11_AC
    push af
    push bc
    push de
    push hl
    ld h, c
    ld l, a
    push hl
    ld hl, \1
    jp FarCall11_Args
ENDM

; Input: hl = function, 2 argument bytes on the stack top. Removes them.
FarCall11_Args:
    ld e, BANK(soundQueuePushFar)
    call ___sdcc_bcall_ehl
    pop hl
    jr FarCall11_Pop

; Input: hl = function. Pops the registers that FAR11 pushed.
FarCall11:
    ld e, BANK(soundQueuePushFar)
    call ___sdcc_bcall_ehl
FarCall11_Pop:
    pop hl
    pop de
    pop bc
    pop af
    ret




; ==================================================================
; HUD command queue, HUD commands, scroll script, blink timers and the
; window slide: C in bank3_hud.c. The glue moves the asm input registers
; to the SDCC registers.
; ==================================================================

; Add a command to the HUD queue (hudQueuePush in bank11_queues.c).
; Input: a = command (1..16), c = parameter. Keeps all registers.
HudQueue_Push:
    FAR11_AC hudQueuePushFar

; Output: hl = queue entry at the read index ($d035 + 2 * index).
; Stays in asm: it returns in hl, and VBlank calls it every frame.
HudQueue_ReadSlot:
    ld hl, g_hudQueueRead
    ld e, [hl]
    sla e
    ld d, $00
    ld hl, g_hudQueue
    add hl, de
    ret

; Input: hl = pointer table, a = index. Bank 0 reads no register after it.
DrawTableString:
    ld d, h
    ld e, l
    jp drawTableString

; The HUD commands. Bank 0 runs them through HudCmdTable and reads no
; register after the call.
HudCmd_DrawPair:
    jp hudCmdDrawPair
HudCmd_DrawGauge:
    jp hudCmdDrawGauge
HudCmd_DrawCounters:
    jp hudCmdDrawCounters
HudCmd_DrawD2ec:
    jp hudCmdDrawD2ec
HudCmd_DrawHeader:
    jp hudCmdDrawHeader
HudCmd_DrawIcon:
    jp hudCmdDrawIcon
HudCmd_DrawMark:
    jp hudCmdDrawMark
HudCmd_DrawWeaponIcons:
    jp hudCmdDrawWeaponIcons
HudCmd_DrawKillMarker:
    jp hudCmdDrawKillMarker
HudCmd_DrawPanel:
    jp hudCmdDrawPanel
HudCmd_DrawString:
    jp hudCmdDrawString
; Not unused: this is HUD command 12.
ScrollScript_StreamToVram_Unused:
    jp scrollScriptStreamToVram
HudCmd_ClearIcon:
    jp hudCmdClearIcon
HudCmd_DrawBars:
    jp hudCmdDrawBars
HudCmd_DrawColumn:
    jp hudCmdDrawColumn

; Input: b = flags (bit 0 = bar A, bit 1 = empty tile).
Hud_Cmd14:
    ld a, b
    jp hudCmd14

; Input: e = parameter.
GameB_Sub_4561:
    ld a, e
    jp gameBDrawPair

; Input: c. Output: a. Bank 0 reads only a.
GameB_GetSideAdjustFlipped:
    ld a, c
    jp gameBGetSideAdjustFlipped

; Output: carry set = the script is done.
GameB_AdvanceScrollScript:
    call gameBAdvanceScrollScript
    rra
    ret

GameB_DrawStatusPattern:
    jp gameBDrawStatusPattern
GameB_DrawStatusScreen:
    jp gameBDrawStatusScreen
ScrollScript_EndScene:
    jp scrollScriptEndScene

GameB_DrawSideHeader:
    CALL_C gameBDrawSideHeader
GameB_ResetSceneFlags:
    CALL_C gameBResetSceneFlags
ClearDamageGauge:
    CALL_C clearDamageGauge
ClearMgTrails:
    CALL_C clearMgTrails
GameB_HudBlinkUpdate:
    CALL_C gameBHudBlinkUpdate
Hud_ToggleWindow:
    CALL_C hudToggleWindow
Hud_WindowSlideTick:
    CALL_C hudWindowSlideTick
GameB_ResetAnimPeriod:
    CALL_C gameBResetAnimPeriod

; Clear the shadow OAM ($c000, $200 bytes). Falls into MemClear.
ClearShadowSprites:
    ld bc, $0200
    ld hl, _RAM
; Fill bc bytes at hl with 0 (MemClear) or with e (MemFill).
; Output: hl = end, bc = 0, a = 0. Keeps de.
; Stays in asm: callers use the registers it leaves, and its speed sets
; how long the LCD-off loads take (the timer IRQ phase depends on that).
MemClear:
    ld e, $00
MemFill:
    ld a, e
    ld [hl+], a
    dec bc
    ld a, c
    or b
    jr nz, MemFill
    ret

; Input: hl = VRAM dest, de = tile row, a = row width, b = times, c = groups.
DrawRepeatedPattern:
    ld [g_gameBLayoutVariant], a
    ld a, b
    ld [g_scratch92], a
    ld a, c
    push af
    inc sp
    ld b, d
    ld c, e
    ld d, h
    ld e, l
    call drawRepeatedPattern
    ret

; C entries for the RLE routines. Input: de = dest, bc = data.
drawTilesRLE::
    ld h, d
    ld l, e
    ld d, b
    ld e, c
    jr DrawTilesRLE
drawTilesRLETwoParts::
    ld h, d
    ld l, e
    ld d, b
    ld e, c
    jr DrawTilesRLE_TwoParts

; The RLE routines stay in asm: C was about 70% slower here, and the
; LCD-off load time sets the timer IRQ phase that the sound races depend on.
; Draw an RLE string at hl, then a block of RLE rows one map row lower.
DrawTilesRLE_TwoParts:
    push hl
    call DrawTilesRLE
    pop hl
    ld bc, $0020
    add hl, bc
DrawRLERowsThenRLE:
    call DrawTilesRLERows

; Copy tiles from de to hl until the end mark.
; $ff, n, tile = write tile n times. $ff, $ff = end.
DrawTilesRLE:
    ld a, [de]
    inc de
    cp $ff
    jr nz, .skip

    ld a, [de]
    inc de
    cp $ff
    ret z

    ld c, a
    ld a, [de]
    inc de

.loop:
    ld [hl+], a
    dec c
    jr nz, .loop

    jr DrawTilesRLE

.skip:
    ld [hl+], a
    jr DrawTilesRLE

; Draw RLE rows. First byte at de = number of rows.
; Each row starts one map row ($20) lower.
DrawTilesRLERows:
    ld a, [de]
    inc de

.loop:
    push af
    push de
    push hl
    call DrawTilesRLE
    pop hl
    ld bc, $0020
    add hl, bc
    ld c, e
    ld b, d
    pop de
    pop af
    dec a
    jr nz, .loop

    ld e, c
    ld d, b
    ret

; Copy c bytes from hl to de.
CopyBytesHLtoDE:
    ld a, [hl+]
    ld [de], a
    inc de
    dec c
    jr nz, CopyBytesHLtoDE
    ret



; ==================================================================
; Game B frame code: C in bank3_game.c. Bank 0 reads no register after
; a "jp" entry. The other entries keep or set the registers that bank 0
; reads.
; ==================================================================

; Output: carry clear = the fade has ended.
GameB_AnimTick:
    call gameBAnimTick
    rra
    ret

; Input: c = number of wave bands.
GameB_AnimateWaves:
    ld a, c
    jp gameBAnimateWaves

GameB_ResetPlayerShip:
    jp gameBResetPlayerShip
GameB_State2e_D02eSamePriorityGate:
    jp gameBState2eGate
GameB_BlockMovement:
    jp gameBBlockMovement
GameB_StepDelay:
    jp gameBStepDelay
GameB_ClearSpriteTails:
    jp gameBClearSpriteTails
GameB_PlayerHit:
    jp gameBPlayerHit
GameB_ClearDamageGauge:
    jp gameBClearDamageGauge
GameB_UpdateMG:
    jp gameBUpdateMG
GameB_ClearFireFlags:
    jp gameBClearFireFlags
GameB_UpdateDistanceCounters:
    jp gameBUpdateDistanceCounters
; bank11_link.c also calls it.
GameB_TogglePause:
    jp gameBTogglePause
GameB_CheckSubRange:
    jp gameBCheckSubRange
GameB_MaybeAddItem:
    jp gameBMaybeAddItem
GameB_ResetEnemySub:
    jp gameBResetEnemySub
GameB_PickLevelValue:
    jp gameBPickLevelValue
GameB_PlayNextTableSound:
    jp gameBPlayNextTableSound
GameB_TallyKillFlags:
    jp gameBTallyKillFlags
GameB_StartLevelClear:
    jp gameBStartLevelClear
GameB_SetScrollY5:
    jp gameBSetScrollY5
GameB_SetD0b2To60:
    jp gameBSetD0b2To60
GameB_LinkSetState7:
    jp gameBLinkSetState7
GameB_ClearModeFlag16_7:
    jp gameBClearModeFlag16_7
; Output: a = the last byte 5. Bank 0 stores it.
GameB_Sub_52ab:
    jp gameBSub52ab

; Move the MG trail A points. The next entry does trail B.
GameB_MoveMgTrails:
    xor a
    jp gameBMoveMgTrails
.trailB:
    ld a, $01
    jp gameBMoveMgTrails

; Draw the object slots.
GameB_DrawEnemySlots:
    ld de, g_shipArray_Enemy
    jp gameBDrawObjects
GameB_DrawEnemySlotsD258:
    jp gameBDrawEnemySlotsD258
GameB_DrawPlayerSlots:
    jp gameBDrawPlayerSlots
GameB_DrawPlayerSlotsD1c0:
    ld de, g_shipArray_Player + $40
    jp gameBDrawObjects

; Input: a = first entry. Clear both sprite buffers from entry a to 40.
ClearSpritesFrom:
    jp clearSpritesFrom

; Input: hl = first entry, c = count. Write 0 to every 4th byte.
ClearSpriteEntries:
    ld d, h
    ld e, l
    ld a, c
    jp clearSpriteEntries

; Input: a = count. Output: hl = first entry, a = its index (40 more in
; the $c100 buffer). Keeps bc and de.
Oam_AllocSprites:
    push bc
    push de
    call oamAllocSprites
    pop de
    ld h, b
    ld l, c
    ld a, c
    rrca
    rrca
    bit 0, h
    jr z, .done

    add $28

.done:
    pop bc
    ret

; Input: a = class, e = Y. Output: e = Y moved by the depth. Keeps bc, d.
GameB_AdjustYByDepth:
    push bc
    push de
    call adjustYByDepth
    pop de
    ld e, a
    pop bc
    ret

; Input: a = class. Output: a = GameB_ObjSpriteIdPtrs[(a >> 4) & 7][a & 15],
; c = a with bits 6 and 7 cleared. Keeps de.
GameB_LookupData6e78:
    push de
    call objSpriteByte
    pop de
    ld c, a
    res 7, c
    res 6, c
    ret

; Input: d = depth. Set the approach from d. Keeps all registers.
GameB_EnemySub_TickInit:
    ld a, d
    CALL_C gameBSetSubApproach

GameB_ResetSubApproach:
    CALL_C gameBResetSubApproach
GameB_Sub_5111:
    CALL_C gameBSub5111
GameB_Sub_5127:
    CALL_C gameBSub5127
; Keeps all registers.
GameB_ResetScrollCursors:
    CALL_C gameBResetScrollCursors

; Input: b = limit, c = step period. Output: carry set while $d447 < b.
; Then e = the old $d447. Keeps bc and d.
GameB_StepCounters:
    push bc
    push de
    ld a, [g_stepCountA]
    push af
    ld a, b
    ld e, c
    call gameBStepCounters
    pop bc
    pop de
    rra
    jr nc, .done

    ld e, b

.done:
    pop bc
    ret

; Copy 3 palette bytes from entry a to rBGP, rOBP0, rOBP1. The next entry
; copies them to g_paletteShadow. Both keep all registers.
SetPalettes:
    CALL_C gameBSetPalettes
.d31fEntry:
    CALL_C gameBSetPaletteShadow

; Stays in asm: these routines take and return only registers and flags.
; Glue for C would be as long as the routine.

; Input: de < 0. Output: de = world width + de.
GameB_WorldWrapNegDE:
    sub a
    sub e
    ld e, a
    ld a, $00
    sbc d
    ld d, a
    ld hl, g_worldWidth
    ld a, [hl+]
    ld h, [hl]
    sub e
    ld e, a
    ld a, h
    sbc d
    ld d, a
    ret

; Shift de left by 4 bits (de = de << 4, keeping 16 bits).
GameB_ShiftDELeft4:
    swap d
    ld a, d
    and $f0
    ld d, a
    swap e
    ld a, e
    and $0f
    or d
    ld d, a
    ld a, e
    and $f0
    ld e, a
    ret

; Build the bit mask c = $20 << (last tile & $3f).
; Output: d = kill flag bits 0..2, a = kill flag bits 5..7, hl = flags.
GameB_KillFlagMask:
    ld a, [ScrollScript_LastTileByte]
    and $3f
    ld c, $20

.loop:
    or a
    jr z, .skip

    sla c
    dec a
    jr .loop

.skip:
    ld hl, g_killFlagsPending
    ld a, [hl]
    and $07
    ld d, a
    ld a, [hl]
    and $e0
    ret

; Input: c = index. Output: a = GameB_SpriteParamTable[c].
GameB_GetTable71c9:
    ld b, $00
    ld hl, GameB_SpriteParamTable
    add hl, bc
    ld a, [hl]
    ret

; Output: c = b when $d011 bit 0 is set. Keeps a.
GameB_SelectByD011:
    ld hl, g_enemySubFlags
    bit 0, [hl]
    ret z

    ld c, b
    ret

; Output: a = $d002, z = bit 7 clear.
GameB_TestSceneFlagC7:
    ld a, [g_sceneFlagsC]
    bit 7, a
    ret

; Count down g_levelClearTimer. Output: z set when it is 0.
GameB_TickLevelClearTimer:
    ld hl, g_levelClearTimer
    jr DecAndCheckTimer

; Count down g_deathMarker. Output: z set when it is 0.
GameB_TickDeathMarker:
    ld hl, g_deathMarker

; Decrement [hl] unless it is 0. Output: z set when it was already 0.
; a = the old value. Keeps hl.
DecAndCheckTimer:
    ld a, [hl]
    or a
    ret z

    dec [hl]
    ret

; Tick the level-clear timer. When it ends and a sound fade is running,
; tick the death marker. Output: z.
GameB_TickLevelClear:
    call GameB_TickLevelClearTimer
    ret nz

    ld a, [g_soundFadeState]
    or a
    ret z

    jp GameB_TickDeathMarker

; Output: a = 0 if $d002 bit 2 is set, else 4.
GameB_GetSideOffset:
    sub a
    ld hl, g_sceneFlagsC
    bit 2, [hl] ; link role: other side
    ret nz

    ld a, $04
    ret

; Step the counter at $d413 through 0..10. Output: carry clear when it
; wraps to 0. Keeps hl.
GameB_StepD413:
    push hl
    ld hl, g_cycle11
    inc [hl]
    ld a, [hl]
    cp $0b
    jr c, .done

    ld [hl], $00

.done:
    pop hl
    ret

; Jump through a table of addresses. Input: hl = table, a = index.
; Stays in asm: it jumps, so it has no C form.
DispatchJumpTable:
    ld d, $00
    ld e, a
    sla e
    add hl, de
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    jp hl

; Entry from bank 0. Runs only when $d002 bit 0 is set. Map the selected
; enemy ship X to the screen, then fall through to get its ship entry.
GameB_MapSelectedShip:
    ld a, [g_sceneFlagsC]
    bit 0, a ; scene parity
    ret z

    call mapSelectedShip
    ld a, [g_selectedEnemyShip]

; Output: hl = $d219 + a * 8 (8-byte ship entries).
; Stays in asm: callers read hl.
GetShipEntryD219:
    ld c, a
    ld b, $00
    sla c
    sla c
    sla c
    ld hl, g_shipArray_Enemy + $1
    add hl, bc
    ret


; ==================================================================
; Glue to C in bank 11.
; ==================================================================

GameB_StartDamageGauge:
    FAR11 gameBStartDamageGauge
GameB_TickGameClock:
    FAR11 gameBTickGameClock
GameB_UpdateEnemyShips:
    FAR11 gameBUpdateEnemyShips
GameB_UpdateEnemyObjects:
    FAR11 gameBUpdateEnemyObjects
GameB_UpdatePlayerOrLink:
    FAR11 gameBUpdatePlayerOrLink
GameB_RunObjectHandlers:
    FAR11 gameBRunObjectHandlers
GameB_FireTorpedo:
    FAR11 fireTorpedo
GameB_FireMachineGun:
    FAR11 fireMachineGun
; Bank 0 jumps to .skip: set the death marker and the end sub state.
GameB_KillConfirm:
.skip:
    FAR11 killConfirmEndFar
GameB_Sub_5b11:
    FAR11 sub5b11Far
GameB_LinkUpdate:
    FAR11 gameBLinkUpdate
GameB_ReadPadAndClock:
    FAR11 gameBReadPadAndClock
GameB_MarkLinkEnd:
    FAR11 gameBMarkLinkEnd
GameB_SetLevelClearTimer:
    FAR11 gameBSetLevelClearTimer
GameB_SendLinkStatus:
    FAR11 gameBSendLinkStatus

; Add a 2-byte message to the link send ring. Input: a = command,
; c = parameter. Keeps all registers.
LinkTxQueue_Push:
    FAR11_AC linkTxQueuePushFar

; Called by bank 0.
SoundQueue_RunNext:
    CALL_C soundQueueRunNext

; Queue sound a. The Play* functions add (type, id) to a 16-entry ring at
; $d071 (soundQueuePush in bank11_queues.c). Keep all registers.
; Stays in asm: bank 0 calls them with a in many places and reads the
; registers after the call.
PlaySFX:
    push bc
    ld c, $01
    jr SoundQueue_Push
PlaySound_DEB8:
    push bc
    ld c, $02
    jr SoundQueue_Push
PlayMusic:
    push bc
    ld c, $03
    jr SoundQueue_Push
PlaySound_DEB0:
    push bc
    ld c, $04
    jr SoundQueue_Push
PlaySound_DEC8:
    push bc
    ld c, $05

; Input: c = type, a = id. Pops bc that the caller pushed.
SoundQueue_Push:
    push af
    push de
    push hl
    ld h, a
    ld l, c
    push hl
    ld e, BANK(soundQueuePushFar)
    ld hl, soundQueuePushFar
    call ___sdcc_bcall_ehl
    pop hl
    pop hl
    pop de
    pop af
    pop bc
    ret

; Input: de, c. b is 0 for every caller. Output: a = de / c (low 8 bits),
; de = 0. Keeps bc.
Divide16:
    push bc
    ld a, c
    call divide16
    pop bc
    ld de, $0000
    ret

; Move a projectile (projectileTick in bank11_weapons.c).
; Input: hl = slot. Output: hl = slot + 3, de = new X, a = e.
GameB_ProjectileTick:
    push hl
    ld hl, projectileTickFar
    ld e, BANK(projectileTickFar)
    call ___sdcc_bcall_ehl
    pop hl
    inc hl
    inc hl
    ld e, [hl]
    inc hl
    ld d, [hl]
    ld a, e
    ret

; enemyFireGate in bank11_weapons.c.
; Input: hl = enemy ship, a = fire delay, d = range. Keeps de, bc, hl.
GameB_EnemyFireGate:
    push de
    push bc
    push hl
    ld e, a
    push de
    push hl
    ld hl, enemyFireGateFar
    ld e, BANK(enemyFireGateFar)
    call ___sdcc_bcall_ehl
    add sp, 4
    pop hl
    pop bc
    pop de
    ret

; The enemy ship with byte 6 at hl fires (enemyShipFire in
; bank11_weapons.c). Keeps de, bc, hl.
GameB_EnemyShipFire:
    push de
    push bc
    push hl
    push hl
    ld hl, enemyShipFireFar
    ld e, BANK(enemyShipFireFar)
    call ___sdcc_bcall_ehl
    pop de
    pop hl
    pop bc
    pop de
    ret

; Input: de = last byte of the ship, c = type bits. Save the ship to a
; wreck slot and turn the slot into a wreck. Output: de = de - 8. Keeps bc.
GameB_MakeWreck:
    push bc
    push bc
    push de
    ld hl, makeWreckFar
    ld e, BANK(makeWreckFar)
    call ___sdcc_bcall_ehl
    pop de
    pop hl
    ld a, e
    sub $08
    ld e, a
    jr nc, .noBorrow

    dec d

.noBorrow:
    pop bc
    ret

; Input: de = last byte of the ship. Copy it to the next wreck slot.
; Output: hl = slot - 1.
GameB_SaveToWreckSlot:
    push de
    ld hl, saveToWreckSlotFar
    ld e, BANK(saveToWreckSlotFar)
    call ___sdcc_bcall_ehl
    pop de
    ld h, b
    ld l, c
    dec hl
    ret


; ==================================================================
; Range and hit tests (bank3_range.c).
; ==================================================================

; Input: de = X, bc = half width. Measure from the player X ($d182).
; Output: carry set = out of range. de = distance, $d092 bit 0 = side.
; bc = the window extra ($d32d).
GameB_RangeCheckPlayer:
    ld hl, g_shipArray_Player + $2
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    ld a, [g_spriteDrawMode]
    push af
    add sp, -2 ; the distance
    push af
    inc sp
    push hl
    ld hl, sp+3
    push hl
    call rangeCheck
    pop de
    ld b, a
    pop af
    ld c, a
    ld a, b
    ld b, $00
    rra ; carry = out of range
    ret

; Input: c = index. b = GameB_SpriteOffsetTable[c], then fall into
; GameB_MirrorOffset.
GameB_MirrorFromTable7121:
    ld hl, GameB_SpriteOffsetTable
GameB_MirrorFromTableHL:
    add hl, bc
    ld b, [hl]

; If $d092 bit 0 is set, negate d. Then d -= b. $d092 bit 1 = borrow.
; Output: a = d, hl = $d092.
GameB_MirrorOffset:
    push bc
    push de
    ld a, d
    ld e, b
    call mirrorOffset
    pop de
    ld d, a
    pop bc
    ld hl, g_scratch92
    ret

; Input: c = index, de = value. Divide by GameB_DepthScaleDivisors[c],
; add 8, then mirror the offset by 4.
; Output: a = d = the offset, e = 0, b = 0, hl = $d092.
GameB_ScaledMirrorOffset:
    ld b, $00
    push bc
    ld a, c
    call scaledMirrorOffset
    ld d, a
    ld e, $00
    pop bc
    ld hl, g_scratch92
    ret

; Input: hl = projectile + 3, de = position. Test the projectile against
; the ships. Output: hl = projectile + 6, de = position.
GameB_ProjectileCollide:
    push hl
    push de
    ld b, d
    ld c, e
    ld de, -3
    add hl, de
    ld d, h
    ld e, l
    call projectileCollide
    pop de
    pop hl
    inc hl
    inc hl
    inc hl
    ret


; ==================================================================
; Lock-on AI, enemy sub AI and the Game B joypad (bank3_ai.c).
; ==================================================================

GameB_WeaponAiTick:
    jp weaponAiTick

; Input: d. Output: d = the approach counter when the AI ran. Keeps bc and e.
GameB_EnemySub_ApproachAI:
    push bc
    push de
    ld a, d
    call enemySubApproachAI
    pop de
    ld d, a
    pop bc
    ret

GameB_ProcessInput:
    jp gameBProcessInput

; Input: hl = source + 6. Output: carry set when all slots are in use.
GameB_FindFreeSlotD298:
    ld d, h
    ld e, l
    call findFreeSlotD298
    rra
    ret

GameB_MarkEnemyShip0:
    jp markEnemyShip0



; ==================================================================
; Data tables: tilemaps, HUD strings, scroll scripts and lookup tables.
; ==================================================================

; The data stays at its original address: some of it holds numeric
; pointers into bank 3.
SECTION "Bank 3 data", ROMX[$6911], BANK[$3]

GameB_SceneTilemap:
    db $1b, $ff, $05, $79, $37, $38, $38, $39
    db $37, $38, $38, $39, $37, $38, $38, $39
    db $79, $1c, $ff, $0c, $00, $1b, $ff, $05
    db $79, $65, $7a, $7a, $66, $65, $7a, $7a
    db $66, $65, $7a, $7a, $66, $79, $1c, $ff
    db $0c, $00, $1b, $ff, $05, $79, $65, $7a
    db $7a, $66, $65, $7a, $7a, $66, $65, $7a
    db $7a, $66, $79, $1c, $ff, $0c, $00, $1b
    db $ff, $05, $79, $3a, $3b, $3b, $3c, $3a
    db $3b, $3b, $3c, $3a, $3b, $3b, $3c, $79
    db $1c, $ff, $0c, $00, $1b, $ff, $12, $19
    db $1c, $ff, $0c, $00, $1c, $ff, $12, $18
    db $1c, $ff, $0c, $00, $ff, $ff, $08, $1b
    db $ff, $12, $79, $1c, $ff, $0c, $00, $ff
    db $ff, $37, $ff, $0e, $38, $39, $ff, $ff
    db $06, $65, $ff, $0e, $7a, $66, $ff, $ff
    db $3a, $ff, $0e, $3b, $3c, $ff, $ff, $06
    db $65, $ff, $0e, $79, $66, $ff, $ff

GameB_SixAnimCounterDefaults:
    db $00, $0c, $12, $18, $1e, $24

GameB_LevelEndAnim1:
    db $10, $07, $07, $08, $08, $07, $07, $08
    db $08, $07, $07, $08, $08, $07, $07, $08
    db $08, $fe

GameB_LevelEndAnim2:
    db $10, $0b, $09, $0a, $09, $0a, $09, $0a
    db $09, $0a, $09, $0a, $09, $0a, $09, $09
    db $0b, $fe

GameB_LevelEndAnim3:
    db $18, $00, $01, $00, $01, $00, $01, $00
    db $01, $00, $01, $00, $01, $00, $01, $00
    db $01, $0c, $0c, $0d, $0d, $0e, $0e, $0f
    db $0f, $ff

GameB_LevelEndAnim4:
    db $18, $02, $02, $03, $04, $05, $06, $10
    db $11, $11, $11, $11, $11, $11, $11, $11
    db $11, $11, $11, $11, $11, $11, $11, $11
    db $11, $ff

GameB_LevelEndAnim5:
    db $10, $09, $09, $09, $09, $09, $09, $09
    db $09, $09, $09, $09, $09, $09, $09, $09
    db $09, $fe

GameB_LevelEndAnimPtrs:
    dw GameB_LevelEndAnim1, GameB_LevelEndAnim2, GameB_LevelEndAnim3, GameB_LevelEndAnim4, GameB_LevelEndAnim5

Hud2x2IconTiles:
    db $fa, $fa, $fa, $fa, $7c, $7d, $7e, $7f
    db $57, $5b, $5f, $69, $7c, $7d, $7e, $7f
    db $67, $68, $69, $6a, $6b, $6c, $6d, $6e
    db $dd, $de, $df, $e0, $d5, $d6, $d7, $d8
    db $7a, $7a, $d9, $da, $7a, $7a, $db, $db
    db $e1, $e2, $e3, $e4, $dc, $e5, $e6, $e7
    db $7a, $7a, $e8, $e9, $7a, $7a, $ea, $ea
    db $02, $02, $02, $02, $79, $79, $79, $79
    db $df, $e0, $e1, $e2

GameB_StatusScreenTilemap:
    db $1d, $ff, $12, $1e, $1f, $ff, $ff, $10
    db $75, $ff, $12, $7a, $76, $ff, $ff, $77
    db $ff, $12, $20, $78, $ff, $ff

GameB_StatusScreenStrings:
    db $98, $22, $10, $52, $44, $4b, $44, $42
    db $53, $fa, $fa, $4f, $4e, $56, $44, $51
    db $fa, $54, $4f, $99, $22, $10, $2e, $2f
    db $30, $31, $7a, $7a, $21, $22, $23, $7a
    db $7a, $7a, $27, $28, $29, $2a, $99, $42
    db $10, $2b, $32, $33, $34, $7a, $7a, $7a
    db $24, $25, $26, $7a, $7a, $2b, $28, $2c
    db $2d, $99, $a2, $03, $5d, $5e, $5f, $00

WeaponIconLowerHudCells:
    db $ac, $a9, $a6

WeaponIconUpperHudCells:
    db $cf, $c9, $c3

WeaponIconTiles:
    db $80, $8c, $84, $90, $88, $94

HudPanelColumnOffsets:
    db $01, $07, $0d, $20, $50, $80

HudPanelTiles:
    db $3d, $3e, $3e, $3e, $3e, $3f, $63, $00
    db $00, $00, $00, $64, $60, $61, $61, $61
    db $61, $62

HudPanelBlankTiles:
    db $7a, $7a, $7a, $7a, $7a, $7a, $7a, $00
    db $00, $00, $00, $7a, $7a, $7a, $7a, $7a
    db $7a, $7a

KillMarkerColumnOffsets:
    db $0c, $0a, $08, $03, $02, $01

GameB_StatusPatternTiles:
    db $a0, $a1, $a2, $a5, $a3, $a4, $a3, $a4
    db $a5, $a8, $a6, $a7, $a6, $a7, $a8, $ab
    db $a9, $aa, $a9, $aa, $ab, $ac, $ad, $ae
    db $af, $b0, $b1, $b2, $b3, $b4, $b5, $b6
    db $b7, $ff, $40, $01, $ff, $20, $b8, $ff
    db $20, $b9, $ff, $20, $ba, $ff, $20, $bb
    db $ff, $20, $bc, $ff, $40, $02, $ff, $20
    db $bd, $ff, $20, $be, $ff, $20, $bf, $ff
    db $c0, $03, $ff, $ff, $ff, $20, $02, $ff
    db $20, $b5, $ff, $20, $01, $ff, $20, $b4
    db $ff, $20, $b3, $ff, $a0, $00, $ff, $a0
    db $00, $ff, $ff

GameB_DamageScrollOffsets:
    db $00, $00, $00, $30, $34, $38, $40, $48
    db $50, $60, $78, $80, $90, $a0, $b0

GameB_WaveStepSizes:
    db $02, $03, $04, $05, $06, $07, $08, $0a
    db $0c, $10, $00, $07, $57, $59, $5c, $5f
    db $67, $6f, $77, $7f, $8e

GameB_DamageGaugeSteps:
    db $22, $20, $1e, $1c, $1a, $18, $16, $14
    db $12, $10

GameB_DamageStepTable:
    db $0e, $00, $07, $10, $03, $04, $05, $06
    db $07, $08, $0a, $0e, $10, $17, $0e, $0f
    db $10, $10, $10, $11, $2a, $2a, $2a, $2a
    db $2a, $2a, $2a, $2a, $1f, $10, $10, $10
    db $20, $0e, $ff, $0c, $00, $22, $23, $1c
    db $1d, $1e, $25, $26, $27, $28, $29, $28
    db $2b, $2c, $26, $2e, $42, $43, $44, $30
    db $31, $ff, $0c, $00, $23, $ff, $04, $03
    db $32, $ff, $08, $37, $38, $ff, $04, $03
    db $30, $ff, $0c, $00, $ff, $05, $03, $39
    db $ff, $08, $3a, $3b, $ff, $05, $03, $ff
    db $0c, $2f, $ff, $04, $3c, $3d, $3e, $02
    db $02, $48, $49, $4a, $4b, $02, $02, $3f
    db $40, $ff, $04, $3c, $ff, $0c, $00, $ff
    db $04, $41, $45, $46, $ff, $08, $02, $47
    db $4e, $04, $04, $4f, $50, $ff, $0c, $00
    db $ff, $04, $51, $52, $53, $ff, $08, $02
    db $54, $55, $ff, $04, $51, $ff, $0c, $00
    db $ff, $05, $56, $58, $ff, $08, $59, $5a
    db $ff, $05, $56, $ff, $0c, $00, $5c, $5c
    db $60, $ff, $0e, $5e, $5d, $ff, $0e, $5c
    db $ff, $ff, $04, $01, $01, $61, $ff, $0e
    db $62, $63, $01, $01, $ff, $ff, $64, $64
    db $65, $ff, $0e, $66, $67, $64, $64, $ff
    db $0c, $00, $ff, $ff, $04, $01, $01, $61
    db $ff, $0e, $62, $63, $01, $01, $ff, $ff
    db $64, $64, $65, $ff, $0e, $6a, $67, $64
    db $64, $ff, $20, $01, $ff, $ff, $9d, $52
    db $02, $6c, $6d, $9d, $f2, $02, $6e, $6f
    db $00

GameB_ObjSpriteTiles:
    db $79, $79, $79, $79, $79, $7b, $7c, $7b
    db $7d, $7e, $7f, $7b, $7c, $7f, $7c, $78
    db $01, $8c, $8d, $01, $01, $01, $8e, $8f
    db $90, $01, $01, $91, $92, $93, $94, $01
    db $9a, $95, $96, $97, $98, $99, $01, $9b
    db $9c, $9d, $9e, $9f, $01, $8b, $01, $01
    db $01, $01, $01, $01

HudIconPtrs:
    dw GameB_ObjSpriteTiles + $2e, GameB_ObjSpriteTiles + $10, GameB_ObjSpriteTiles + $15, GameB_ObjSpriteTiles + $1a
    dw GameB_ObjSpriteTiles + $20, GameB_ObjSpriteTiles + $20, GameB_ObjSpriteTiles + $26, GameB_ObjSpriteTiles + $26
    dw GameB_ObjSpriteTiles + $2c

HudBarCellsB:
    db $60, $61, $62, $63, $64, $41, $42, $43

HudBarCellsB_LinkLast:
    db $44, $22, $23

HudBarCellsB_Last:
    db $24

HudBarCellsA:
    db $73, $72, $71, $70, $6f, $52, $51, $50

HudBarCellsA_LinkLast:
    db $4f, $31, $30

HudBarCellsA_Last:
    db $2f

; Pointers to VRAM tile strings: dest (big-endian), length, tiles..., $00.
HudStringPtrs:
    dw HudStr_TopLogo, HudStr_TopLogoClear, HudStr_Waiting, HudStr_WaitingClear, HudStr_WindowLabelClear, HudStr_WindowLabel

HudStr_TopLogo:
    db $98, $27, $06, $80, $81, $82, $83, $84
    db $85, $00

HudStr_TopLogoClear:
    db $98, $27, $06, $01, $01, $01, $01, $01
    db $01, $00

HudStr_Waiting:
    db $99, $aa, $07, $56, $40, $48, $53, $48
    db $4d, $46, $00

HudStr_WaitingClear:
    db $99, $aa, $07, $fa, $fa, $fa, $fa, $fa
    db $fa, $fa, $00

HudStr_WindowLabelClear:
    db $9c, $27, $06, $26, $26, $26, $26, $26
    db $26, $00

HudStr_WindowLabel:
    db $9c, $27, $06, $27, $28, $29, $28, $2b
    db $2c, $00

; Pointers to VRAM text strings: dw dest (big-endian), db length, text..., $00.
GameB_MessagePtrs:
    dw GameB_Msg_DoYourBest, GameB_Msg_NiceFight, GameB_Msg_Defeat, GameB_Msg_BigBanner, GameB_Msg_ADraw, GameB_Msg_Congratulations

GameB_Msg_DoYourBest:
    db $98, $e4, $0c, $43, $4e, $7a, $58, $4e
    db $54, $51, $7a, $41, $44, $52, $53, $00

GameB_Msg_NiceFight:
    db $98, $e5, $0a, $4d, $48, $42, $44, $7a
    db $45, $48, $46, $47, $53, $00

GameB_Msg_Defeat:
    db $98, $e7, $06, $43, $44, $45, $44, $40
    db $53, $00

GameB_Msg_BigBanner:
    db $98, $e3, $0e, $bb, $bc, $bd, $be, $bf
    db $c0, $c1, $c2, $bf, $c3, $c4, $c5, $c6
    db $c7, $99, $03, $0e, $c8, $c9, $ca, $cb
    db $cc, $cd, $ce, $cf, $cc, $d0, $d1, $d2
    db $d3, $d4, $00

GameB_Msg_ADraw:
    db $98, $e7, $06, $40, $7a, $43, $51, $40
    db $56, $00

GameB_Msg_Congratulations:
    db $9a, $c2, $10, $c2, $ce, $cd, $c6, $d1
    db $c0, $d3, $d4, $d1, $c0, $d3, $c8, $ce
    db $cd, $d2, $da, $00

; Scroll scripts: tile numbers, $fe = next row, $ff = end.
ScrollScript_Scene0_SideA:
    db $80, $81, $82, $82, $81, $83, $84, $01
    db $85, $83, $80, $86, $87, $88, $89, $88
    db $ff

; Scroll script tile lists. $00/$01 = control codes, $fe = break, $ff = end.
ScrollScript_Scene1:
    db $80, $81, $82, $82, $83, $01, $84, $85
    db $01, $86, $87, $85, $83, $ff

ScrollScript_Scene2:
    db $80, $81, $82, $83, $00, $84, $85, $83
    db $86, $ff

ScrollScript_Scene3:
    db $80, $81, $82, $00, $83, $84, $85, $fe
    db $86, $87, $88, $89, $87, $88, $8a, $ff

; Banners: first byte is a column, then tiles, $ff = end.
ScrollScript_Banner_A:
    db $37, $4c, $48, $52, $4e, $4d, $42, $4f
    db $4b, $44, $53, $ff

ScrollScript_Banner_B:
    db $37, $45, $4b, $44, $53, $48, $52, $4b
    db $4e, $ff

ScrollScript_Banner_C:
    db $47, $46, $40, $4c, $44, $4e, $55, $51
    db $ff

ScrollScript_Banner_D:
    db $47, $58, $4e, $54, $40, $51, $44, $52
    db $48, $4d, $4a, $46, $ff

ScrollScript_Banner_E:
    db $37, $16, $17, $18, $19, $1a, $1b, $1c
    db $1d, $1e, $1f, $20, $21, $22, $23, $24
    db $25, $26, $27, $28, $29, $2a, $2b, $2c
    db $2d, $2e, $2f, $30, $31, $32, $33, $34
    db $35, $ff

; Used by bank 1 ($6280).
ScrollScript_BannerPtrTable:
    dw ScrollScript_Banner_A, ScrollScript_Banner_B, ScrollScript_Banner_C, ScrollScript_Banner_D, ScrollScript_Banner_E

; A falling curve of 8 values ($38 down to 0).
GameB_UnknownCurve6e10:
    db $38, $2c, $20, $18, $10, $0a, $04, $00

GameB_ObjSpriteIds_Class0:
    db $ff, $ff, $ff, $ff, $00, $ff, $ff, $ff, $ff, $ff, $22, $23

GameB_ObjSpriteIds_Class1:
    db $81, $87, $8d, $93, $ff, $ff, $ff, $ff, $ff, $ff, $22, $23

GameB_ObjSpriteIds_Class2:
    db $42, $48, $4e, $54, $ff, $ff, $ff, $ff, $ff, $ff, $22, $23

GameB_ObjSpriteIds_Class3:
    db $83, $89, $8f, $95, $ff, $ff, $ff, $ff, $ff, $ff, $22, $23

GameB_ObjSpriteIds_Class4:
    db $44, $4a, $50, $56, $ff, $ff, $ff, $ff, $ff, $ff, $22, $23

GameB_ObjSpriteIds_Class5:
    db $85, $8b, $91, $97, $ff, $ff, $ff, $ff, $ff, $ff, $22, $23

GameB_ObjSpriteIds_Class6:
    db $46, $4c, $52, $58, $ff, $ff, $ff, $ff, $ff, $ff, $22, $23

GameB_ObjSpriteIds_Class7:
    db $1a, $1b, $1c, $21, $ff, $1d, $1e, $1f, $20, $19, $22, $23

GameB_ObjSpriteIdPtrs:
    dw GameB_ObjSpriteIds_Class0, GameB_ObjSpriteIds_Class1, GameB_ObjSpriteIds_Class2, GameB_ObjSpriteIds_Class3
    dw GameB_ObjSpriteIds_Class4, GameB_ObjSpriteIds_Class5, GameB_ObjSpriteIds_Class6, GameB_ObjSpriteIds_Class7

GameB_DepthYOffsets_Class0:
    db $02, $05, $07, $09, $0b, $0e, $10, $12
    db $15, $17, $19, $1c, $1e, $20, $22, $25
    db $27, $29, $2c, $2e, $30, $32, $35, $37

GameB_DepthYOffsets_Class1:
    db $02, $03, $05, $06, $08, $09, $0b, $0c
    db $0e, $0f, $11, $13, $14, $16, $17, $19
    db $1a, $1c, $1e, $1f, $20, $22, $23, $25

GameB_DepthYOffsets_Class2:
    db $01, $02, $03, $04, $05, $06, $08, $09
    db $0a, $0b, $0c, $0d, $0e, $0f, $10, $11
    db $12, $14, $15, $16, $17, $18, $19, $1a

GameB_DepthYOffsets_Class3:
    db $01, $01, $02, $03, $03, $04, $05, $05
    db $06, $07, $07, $08, $09, $09, $0a, $0b
    db $0b, $0c, $0d, $0d, $0e, $0f, $0f, $10

GameB_DepthYOffsets_Class4:
    db $00, $01, $01, $01, $02, $02, $02, $03
    db $03, $03, $04, $04, $04, $05, $05, $05
    db $06, $06, $06, $07, $07, $07, $08, $08

GameB_DepthYOffsets_Class5:
    db $00, $00, $01, $01, $01, $01, $01, $01
    db $02, $02, $02, $02, $02, $02, $03, $03
    db $03, $03, $03, $03, $04, $04, $04, $04

GameB_DepthYOffsets_Class6:
    db $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $01, $01, $01, $01
    db $01, $01, $01, $01, $01, $01, $01, $01

GameB_DepthYOffsets_Class7:
    db $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00

GameB_DepthYOffsetPtrs:
    dw GameB_DepthYOffsets_Class0, GameB_DepthYOffsets_Class1, GameB_DepthYOffsets_Class2, GameB_DepthYOffsets_Class3
    dw GameB_DepthYOffsets_Class4, GameB_DepthYOffsets_Class5, GameB_DepthYOffsets_Class6, GameB_DepthYOffsets_Class7

GameB_DepthBandThresholds:
    db $0c, $0e, $10, $12, $14, $18, $18, $18
    db $18, $10, $18, $20, $30, $4e, $50, $54
    db $58, $08, $0c, $10, $18, $36, $46, $4e
    db $5e

GameB_HitRangeBounds:
    db $59, $6a, $76, $80, $87, $8b, $8d, $8f

GameB_TorpedoRangeDelays:
    db $01, $01, $01, $02, $01, $01, $01, $02
    db $01, $01, $01, $02, $01, $01, $01, $02
    db $01, $01, $01, $02, $01, $01, $01, $02
    db $01, $01, $01, $02, $01, $01, $01, $02
    db $01, $01, $01, $02, $01, $01, $01, $02
    db $01, $01, $01, $02, $01, $01, $01, $02
    db $01, $01, $01, $02, $01, $01, $01, $02
    db $00, $00, $00, $01, $00, $00, $00, $01
    db $00, $00, $01, $00, $00, $01, $00, $00
    db $00, $01, $00, $00, $01, $00, $00, $01
    db $00, $00, $01, $00, $00, $01, $00, $01
    db $00, $01, $00, $01, $00, $01, $00, $01
    db $00, $01, $00, $01, $00, $01, $00, $01
    db $01, $01, $01, $00, $01, $00, $01, $01
    db $01, $01, $01, $01, $01, $01, $01, $01
    db $01, $01, $01, $01, $01, $01, $01, $01
    db $01, $01, $01, $01, $01, $01, $02, $01
    db $02, $01, $02, $03, $04, $04, $06, $0a
GameB_TorpedoRangeDistances:
    db $18, $00, $1c, $00, $20, $00, $24, $00
    db $28, $00, $2c, $00, $30, $00, $34, $00
    db $38, $00, $3c, $00, $40, $00, $44, $00
    db $48, $00, $4c, $00, $50, $00, $54, $00
    db $58, $00, $5c, $00, $60, $00, $68, $00
    db $6c, $00, $70, $00, $74, $00, $78, $00
    db $7c, $00, $80, $00, $84, $00, $88, $00
    db $8c, $00, $90, $00, $94, $00, $98, $00
    db $9c, $00, $a0, $00, $a4, $00, $a9, $00
    db $ad, $00, $b2, $00, $b6, $00, $bb, $00
    db $bf, $00, $c4, $00, $c8, $00, $cd, $00
    db $d1, $00, $d6, $00, $da, $00, $df, $00
    db $e3, $00, $e7, $00, $ec, $00, $f0, $00
    db $f6, $00, $fd, $00, $03, $01, $0a, $01
    db $10, $01, $17, $01, $1d, $01, $24, $01
    db $2b, $01, $31, $01, $38, $01, $40, $01
    db $48, $01, $50, $01, $58, $01, $60, $01
    db $68, $01, $70, $01, $78, $01, $80, $01
    db $88, $01, $90, $01, $9a, $01, $a4, $01
    db $ae, $01, $b8, $01, $c2, $01, $cc, $01
    db $d6, $01, $e0, $01, $f4, $01, $08, $02
    db $1c, $02, $30, $02, $58, $02, $80, $02

GameB_DepthScaleDivisors:
    db $05, $06, $06, $03, $07, $09, $0a, $0a
    db $0b, $0c, $0d, $0e, $0e, $0f, $10, $11
    db $12, $12, $13, $14, $15, $16, $17, $18
    db $18, $19, $1a, $1b, $1c, $1d, $1e, $1e
    db $1f, $20, $21, $22, $23, $24, $25, $26
    db $27, $28, $29, $2a, $2b, $2c, $2c, $2d
    db $2e, $2e, $2f, $30, $32, $33, $35, $36
    db $38, $39, $3b, $3c, $3d, $3e, $3f, $40
    db $42, $43, $45, $46, $48, $49, $4b, $4d
    db $4e, $50, $52, $54, $56, $58, $5a, $5c
    db $5e, $60, $64, $68, $6c, $70, $78, $80

GameB_ObjBaseYTable:
    db $78, $80, $84, $88, $8c, $90, $94, $98
    db $70, $68, $64, $60, $5c, $58, $54, $50

GameB_SpriteOffsetTable:
    db $14, $0c, $0c, $08, $08, $04, $04, $10
    db $10, $0c, $0c, $08, $08, $14, $14, $10
    db $10, $0c, $0c, $08, $08, $04, $04, $00
    db $00, $00, $00, $04, $08, $09, $0b, $09
    db $0b, $fc, $04, $04, $04, $04, $04, $00
    db $00, $00, $00, $01, $01, $12, $12, $23
    db $23, $01, $01, $12, $12, $23, $23, $01
    db $01, $12, $12, $23, $23, $01, $01, $12
    db $12, $23, $23, $44, $23, $23, $23, $13
    db $13, $13, $13, $23, $4f, $3d, $31, $2f
    db $27, $23, $21, $3d, $31, $27, $1f, $23
    db $21, $3d, $31, $27, $1f, $1b, $19, $45
    db $39, $2f, $27, $23, $21, $18, $3f, $2d
    db $21, $17, $0f, $0b, $09, $08

GameB_SpriteHitWidths:
    db $38, $3c, $50, $50, $60, $54, $60, $48
    db $60, $64, $78, $66, $80, $54, $70, $78
    db $90, $8c, $a0, $30, $40, $3c, $48, $38
    db $40, $48, $48, $6c, $90, $a7, $a7, $b1
    db $b1, $1a

; One byte per enemy class.
GameB_EnemyClassRankTable:
    db $36, $4e, $6e, $70, $8a, $80, $98, $b8

; One word per enemy class ($00a0..$02d0, step $50).
GameB_EnemyClassHandlerPtrs:
    dw $00a0, $00f0, $0140, $0190, $01e0, $0230, $0280, $02d0

GameB_SpriteParamTable:
    db $08, $07, $07, $04, $04, $03, $03, $08
    db $08, $07, $07, $04, $04, $0a, $0a, $08
    db $08, $06, $06, $04, $04, $03, $03, $02
    db $02, $08, $07, $02, $0b, $0b, $08, $04
    db $07, $02, $0b, $0b, $08, $04, $02, $06
    db $08, $06, $02, $02, $04, $0f, $0f, $0e
    db $0e, $0d, $0d, $0c, $0c, $0b, $0b, $0a
    db $0a, $09, $09, $08, $08, $07, $07, $06
    db $06, $05, $05, $04, $04, $03, $03, $02
    db $02, $01, $01, $01, $01, $02, $03, $03
    db $02, $01, $01, $01, $02, $03, $03, $02
    db $01, $01, $01, $fc, $fc, $fc, $fc, $f8
    db $f4, $f4

GameB_MgTrailStepsA:
    db $01, $10, $01, $11, $01, $10, $02, $11
    db $02, $10, $02, $11, $02, $11, $03, $11
    db $03, $11, $04, $12, $04, $12, $04, $21
    db $05, $22, $05, $23, $06, $23, $08, $24
    db $0e, $37, $01, $31

GameB_MgTrailStepsB:
    db $00, $41, $09, $44, $07, $33, $04, $32
    db $03, $32, $03, $21, $02, $20, $03, $21
    db $02, $21, $02, $20, $02, $10, $01, $10
    db $01, $10, $01, $10, $01, $10, $00, $10
    db $00, $11, $00

GameB_RasterTables:
    db $10, $00, $00, $07, $17, $1a, $1e, $23
    db $29, $30, $38, $42, $50, $60, $77, $88
    db $28, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $18, $1d, $10, $7d
    db $30, $1c, $10, $7e, $34, $1c, $11, $7f
    db $36, $1b, $12, $7f, $37, $1a, $12, $7f
    db $38, $19, $13, $7f, $39, $18, $13, $7e
    db $3a, $17, $13, $7d, $3c, $16, $13, $fa
    db $3f, $16, $13, $78, $42, $17, $13, $76
    db $45, $17, $13, $65, $49, $19, $24, $63
    db $4e, $1a, $24, $52, $52, $1b, $24, $51
    db $55, $1c, $24, $c0, $58, $1d, $24, $42
    db $5b, $1e, $24, $34, $5c, $16, $45, $36
    db $5e, $17, $45, $38, $60, $18, $45, $2a
    db $61, $19, $45, $2b, $62, $19, $45, $1c
    db $63, $18, $45, $1c, $64, $16, $46, $1c
    db $65, $12, $46, $1d, $66, $0f, $67, $1d
    db $67, $0b, $67, $1e, $68, $07, $67, $1e
    db $69, $03, $67, $1f, $6a, $fe, $67, $1f
    db $6b, $f9, $67, $1f, $6c, $f1, $67, $1f
    db $ff, $e9, $67, $1f, $02, $22, $28, $40
    db $04, $24, $28, $40, $06, $26, $28, $40
    db $07, $2a, $28, $40, $08, $2e, $28, $40
    db $09, $32, $28, $40, $0a, $3a, $29, $40
    db $ff, $42, $28, $40, $00, $5e, $73, $00
    db $20, $00, $00, $00, $a8, $00, $18, $00
    db $28, $01, $38, $02, $48, $03, $58, $04
    db $b2, $00, $b4, $ff, $07, $45, $6e, $00
    db $07, $46, $6f, $a3, $08, $70, $a4, $08
    db $71, $a4, $08, $72, $a4, $08, $73, $a4
    db $08, $74, $a4, $08, $75, $a4, $08, $77
    db $a4, $08, $77, $a4, $08, $77, $a4, $08
    db $78, $a4, $07, $47, $79, $a4, $07, $48
    db $7a, $a4, $07, $49, $7b, $a4, $07, $4a
    db $7c, $a6, $07, $4b, $7d, $a6, $07, $4c
    db $7e, $a6, $07, $4d, $7f, $a6, $07, $4e
    db $80, $a6, $06, $31, $4f, $81, $a6, $05
    db $1f, $32, $50, $82, $a6, $05, $20, $33
    db $51, $83, $a6, $04, $17, $21, $34, $52
    db $84, $a6, $01, $07, $0b, $11, $18, $22
    db $35, $53, $85, $a6, $00, $05, $08, $0c
    db $12, $19, $23, $36, $54, $86, $a6, $00
    db $06, $09, $0d, $13, $1a, $24, $37, $55
    db $87, $a6, $01, $0a, $0e, $14, $1b, $25
    db $38, $56, $88, $a6, $04, $1c, $26, $39
    db $57, $89, $a6, $06, $3a, $58, $8a, $a6
    db $06, $3b, $59, $8b, $a6, $04, $1d, $27
    db $3c, $5a, $8c, $a6, $01, $0f, $15, $15
    db $1e, $28, $3d, $5b, $8d, $a6, $01, $10
    db $16, $16, $16, $29, $00, $5c, $8e, $a6
    db $07, $5d, $8f, $a6, $05, $2a, $3e, $5e
    db $8f, $a6, $05, $2b, $3f, $5f, $8f, $a6
    db $05, $2c, $40, $5f, $90, $a6, $06, $41
    db $60, $91, $a6, $07, $61, $92, $a6, $07
    db $62, $93, $a6, $07, $63, $94, $a6, $07
    db $64, $95, $a6, $07, $65, $96, $a7, $08
    db $97, $a4, $08, $98, $a4, $08, $99, $a4
    db $07, $66, $9a, $a4, $07, $67, $9b, $a4
    db $07, $68, $9c, $a4, $07, $69, $9d, $a4
    db $05, $2d, $00, $6a, $9e, $a4, $05, $2e
    db $42, $68, $9f, $a4, $05, $2f, $43, $6b
    db $a0, $a4, $05, $30, $44, $6c, $a1, $a8
    db $07, $6d, $a2, $a9

HudColumnFallbackData:
    db $0a, $0a, $0a, $0a, $0a, $0a, $0a, $0a


; Pinned: bank 0 GameB_CutsceneDispatch_d824 maps bank 1 and jumps to
; RankScreen_State04 ($7803), which is bank 1 GameB_TallyState2_DrawHeader.
SECTION "Bank 3 rank code", ROMX, BANK[$3]

; The rank screen, the title cutscene build and the level select input
; are C code in bank3_rank.c. Bank 0 calls these labels.
GameB_CutsceneTimerStep:
    jp gameBCutsceneTimerStep
GameB_DrawRankScreen:
    jp gameBDrawRankScreen
GameB_SelectRank:
    jp gameBSelectRank
RankScreen_Frame:
    jp rankScreenFrame
RankScreen_Build:
    jp rankScreenBuild
RankScreen_MoveStars:
    jp rankScreenMoveStars
RankScreen_ScrollSprites:
    jp rankScreenScrollSprites
.skip4: ; the title cutscene build
    jp titleCutsceneBuild

; Output: carry set when the level is confirmed.
GameB_LevelSelectInput:
    call gameBLevelSelectInput
    rra
    ret


; ==================================================================
; Rank screen and level select data.
; ==================================================================

SECTION "Bank 3 rank data", ROMX[$7cd1], BANK[$3]

Rank_FrameStrings:
    db $9c, $20, $01, $bd, $9c, $33, $01, $bf
    db $9d, $a0, $01, $e0, $9d, $b3, $01, $e2
    db $9c, $21, $52, $be, $9c, $22, $04, $d1
    db $c0, $cd, $ca, $9c, $2c, $05, $d2, $c2
    db $ce, $d1, $c4, $9c, $40, $cb, $e3, $9c
    db $53, $cb, $e4, $9d, $a1, $52, $e1, $9d
    db $c1, $0a, $d8, $ce, $d4, $d1, $02, $d2
    db $c2, $ce, $d1, $c4, $9e, $01, $04, $d1
    db $c0, $cd, $ca, $00

Rank_RowDotTiles:
    db $84, $84, $84, $84, $b6

; Rank names: db length, tiles. ADMIRAL, CAPTAIN, COMMANDER, OFFICER, CADET, NO RANK.
; Tile $c0 = "A".
GameB_RankStrings:
    db $07, $c0, $c3, $cc, $c8, $d1, $c0, $cb
    db $07, $c2, $c0, $cf, $d3, $c0, $c8, $cd
    db $09, $c2, $ce, $cc, $cc, $c0, $cd, $c3
    db $c4, $d1, $07, $ce, $c5, $c5, $c8, $c2
    db $c4, $d1, $05, $c2, $c0, $c3, $c4, $d3
    db $07

; The text of "NO RANK" (its length byte is just before).
GameB_RankText_NoRank:
    db $cd, $ce, $02, $d1, $c0, $cd, $ca

; Offset of each rank name in GameB_RankStrings.
GameB_RankStringOffsets:
    db $00, $08, $10, $1a, $22, $28

; Two bytes per rank, indexed by $d82c * 2.
GameB_RankMedalLayout:
    db $00, $07, $07, $07, $0e, $09, $17, $07
    db $1e, $05

Rank_FramePattern:
    db $dd, $de, $df, $df, $dd, $de, $de, $df
    db $dd, $dd, $de, $df

Rank_LycTable:
    db $27, $73, $7a, $83, $8f

Rank_ScyTable:
    db $00, $00, $00, $00, $00

Rank_ScxTable:
    db $70, $00, $00, $00, $c8

Rank_StarCountInit:
    db $05, $20, $20, $ff, $40, $40, $ff, $70
    db $70, $ff, $c0, $c0, $ff, $00, $00, $00

Rank_StarRecordsInit:
    db $02, $02, $ff, $08, $08, $ff, $14, $14
    db $ff, $24, $24, $ff, $00, $00, $00, $0f
    db $44, $f6, $ff, $05, $04, $f4, $f5, $fa
    db $fb, $ff, $03, $44, $f6, $07, $42, $fc
    db $ff, $0c, $42, $fa, $ff, $0c, $42, $fc
    db $ff, $04, $01, $04, $ff, $03, $44, $05
    db $ff, $03, $44, $09, $ff, $03, $44, $0d
    db $ff, $03, $44, $11, $ff, $03, $4c, $15
    db $0f, $83, $21, $ff, $02, $48, $22, $0a
    db $82, $01, $0c, $46, $2a, $ff, $03, $4f
    db $30, $ff, $00, $a0, $48, $04, $42, $3f
    db $06, $84, $41, $0a, $46, $42, $ff

Rank_LabelStrings:
    db $9a, $40, $60, $f1, $9a, $49, $09, $ee
    db $ef, $03, $03, $03, $f3, $fe, $03, $f0
    db $00

Rank_SpriteStrips:
    db $a0, $a0, $42, $e0, $a8, $a0, $42, $e0
    db $b0, $98, $43, $e2, $b8, $98, $07, $e5
    db $e6, $e7, $e8, $e8, $e8, $e9, $c0, $88
    db $09, $ea, $eb, $ec, $3c, $3c, $f2, $f2
    db $3c, $ed

Rank_RevealOrder:
    db $01, $09, $05, $0d, $03, $0e, $00, $0c
    db $07, $02, $0b, $04, $0a, $06, $0f, $08

Rank_CongratulationsTiles:
    db $c2, $ce, $cd, $c6, $d1, $c0, $d3, $d4
    db $cb, $c0, $d3, $c8, $ce, $cd, $d2, $da

Rank_StarStripTable:
    db $40, $7e, $03, $44, $7e, $06, $4c, $7e
    db $07, $68, $30, $43, $01, $49, $65, $42
    db $04, $51, $5d, $44, $06, $38, $8a, $42
    db $0a, $40, $7a, $45, $0c

Rank_StarStripTimes:
    db $00, $18, $28

Rank_State4SpritesA:
    db $54, $a8, $42, $3d

Rank_State4SpritesB:
    db $38, $a8, $05, $43, $42, $41, $40, $3f
    db $40, $a8, $05, $48, $47, $46, $45, $44

Rank_EndSprites:
    db $2c, $e0, $45, $3f, $34, $e0, $45, $44
    db $08, $02, $07, $04, $ff, $00, $42, $05
    db $04, $03, $07, $04, $05, $0b, $42, $05
    db $16, $43, $04, $1b, $01, $05, $ff, $ff
    db $00, $47, $12, $1f, $01, $08, $ff, $00
    db $42, $1c, $02, $47, $19, $09, $43, $0e
    db $0c, $45, $0c, $11, $45, $0c, $16, $86
    db $0c, $19, $01, $11, $1a, $86, $2a, $ff
    db $11, $02, $59, $5c, $ff, $06, $02, $59
    db $5c, $ff, $0c, $02, $59, $5c, $ff, $0a
    db $01, $3d, $ff, $09, $43, $3e, $ff, $01
    db $46, $41, $07, $4c, $46, $ff, $00, $47
    db $52, $07, $05, $58, $58, $5a, $5b, $58
    db $0c, $47, $5d, $ff, $18, $43, $2e, $ff
    db $00, $04, $20, $21, $2b, $29, $04, $94
    db $00, $11, $02, $2b, $29, $18, $43, $31
    db $ff, $00, $43, $22, $03, $94, $00, $0a
    db $01, $34, $17, $43, $31, $ff, $00, $43
    db $25, $03, $93, $00, $0a, $01, $35, $16
    db $43, $31, $ff, $00, $95, $00, $04, $05
    db $34, $00, $34, $00, $34, $0c, $05, $34
    db $00, $34, $00, $34, $15, $43, $31, $ff
    db $00, $94, $00, $04, $05, $35, $00, $35
    db $00, $35, $0c, $05, $35, $00, $35, $00
    db $35, $14, $43, $31, $ff

TitleCutscene_LycTable:
    db $00, $27, $2f, $37, $3f, $5f

TitleCutscene_ScyTable:
    db $00, $00, $00, $00, $90, $00

TitleCutscene_StarInit:
    db $06, $00, $00, $01, $00, $08, $ff, $00
    db $10, $ff, $00, $20, $ff, $00, $00, $01
    db $00, $00, $01

TitleCutscene_ObjMotion:
    db $20, $40, $60, $80, $00, $ff, $a0

TitleCutscene_PatternA:
    db $04, $05, $06, $07

TitleCutscene_PatternB:
    db $08, $09, $0a, $0b

TitleCutscene_PatternC:
    db $2e, $2c, $2d

TitleCutscene_SpritesA:
    db $4e, $30, $44, $89, $56, $30, $44, $8d

TitleCutscene_SpritesB:
    db $62, $8d, $01, $86, $5e, $85, $43, $80
    db $66, $85, $43, $83, $58, $7d, $85, $87
    db $88, $88, $88, $88

TitleCutscene_SpritesC:
    db $80, $80, $42, $92

; Score needed for each rank, ADMIRAL first.
; Four BCD digits each, high digit first: 500000, 400000, 300000, 200000, 100000.
GameB_RankThresholds:
    db $00, $05, $00, $00
    db $00, $04, $00, $00
    db $00, $03, $00, $00
    db $00, $02, $00, $00
    db $00, $01, $00, $00

LevelSelect_PushStartSprites:
    db $90, $30, $04, $cf, $d4, $d2, $c7, $90
    db $58, $05, $d2, $d3, $c0, $d1, $d3, $00
    db $01, $00, $00, $90, $30, $04, $cf, $d4
    db $d2, $c7, $90, $58, $05, $d2, $d3, $c0
    db $d1, $d3, $d2, $c7, $90, $58, $05, $d2
    db $d3, $c0, $d1, $d3, $40, $9f, $54, $71
    db $11, $7d, $55, $cf, $54, $a5, $55, $5d
    db $45, $5e, $45, $ee, $55, $5f, $15, $b6
    db $54, $75, $44, $59, $45, $7d, $40, $67
    db $51, $d3, $50, $b0, $01, $d7, $05, $aa
    db $15, $8b, $51, $a3, $51, $3e, $05, $06
    db $50, $a7, $85, $bb, $00, $3c, $45, $f2
    db $14, $9f, $55, $bf, $05, $be, $15, $f2
    db $40, $ff, $50


