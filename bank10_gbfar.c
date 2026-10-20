// Game B: shims from bank 10 C to asm in other banks (see bank10_gameb.h).
// Each one sets the registers, calls the routine through FarCall (it maps
// the bank given after the call and maps bank 10 again after it) and
// moves the result into the SDCC registers.
#include "bank0.h"
#include "bank10_gameb.h"

// The __naked bodies read the arguments from the registers.
#pragma disable_warning 85

#define FAR(bank, f) "call _FarCall\n.db " #bank "\n.dw _" #f "\n"
#define FAR3(f) FAR(3, f)
#define FAR1(f) FAR(1, f)
#define CARRY "ld a, #0\nret nc\ninc a\nret"
#define VOID3(name, f) void name(void) __naked { __asm__(FAR3(f) "ret"); }
#define AVOID3(name, f) void name(uint8_t a) __naked { __asm__(FAR3(f) "ret"); }
#define VOID1(name, f) void name(void) __naked { __asm__(FAR1(f) "ret"); }

// No registers in or out (A in for the A-argument ones).
VOID3(gbClearDamageGauge, ClearDamageGauge)
VOID3(gbClearDamageGaugeB, GameB_ClearDamageGauge)
VOID3(gbMarkLinkEnd, GameB_MarkLinkEnd)
VOID3(gbDrawSideHeader, GameB_DrawSideHeader)
VOID3(gbPickLevelValue, GameB_PickLevelValue)
VOID3(gbReadPadAndClock, GameB_ReadPadAndClock)
VOID3(gbDrawStatusPattern, GameB_DrawStatusPattern)
VOID3(gbDrawStatusScreen, GameB_DrawStatusScreen)
VOID3(gbHudDrawWeaponIcons, HudCmd_DrawWeaponIcons)
VOID3(gbHudDrawCounters, HudCmd_DrawCounters)
VOID3(gbClearShadowSprites, ClearShadowSprites)
VOID3(gbResetSceneFlags, GameB_ResetSceneFlags)
VOID3(gbDrawPlayerSlots, GameB_DrawPlayerSlots)
VOID3(gbDrawPlayerSlotsD1c0, GameB_DrawPlayerSlotsD1c0)
VOID3(gbDrawEnemySlots, GameB_DrawEnemySlots)
VOID3(gbDrawEnemySlotsD258, GameB_DrawEnemySlotsD258)
VOID3(gbResetPlayerShip, GameB_ResetPlayerShip)
VOID3(gbEngine, GameB_State14_PerFrameEngine)
VOID3(gbSubDispatchD009, GameB_SubDispatch_d009)
VOID3(gbSendLinkStatus, GameB_SendLinkStatus)
VOID3(gbKillConfirm, GameB_KillConfirm)
VOID3(gbSub5b11, GameB_Sub_5b11)
VOID3(gbResetEnemySub, GameB_ResetEnemySub)
VOID3(gbSetLevelClearTimer, GameB_SetLevelClearTimer)
VOID3(gbMarkEnemyShip0, GameB_MarkEnemyShip0)
VOID3(gbPlayNextTableSound, GameB_PlayNextTableSound)
VOID3(gbEndScene, ScrollScript_EndScene)
VOID3(gbBlockMovement, GameB_BlockMovement)
VOID3(gbResetAnimPeriod, GameB_ResetAnimPeriod)
VOID3(gbTallyKillFlags, GameB_TallyKillFlags)
VOID3(gbUpdateDistanceCounters, GameB_UpdateDistanceCounters)
VOID3(gbStartDamageGauge, GameB_StartDamageGauge)
VOID3(gbHudToggleWindow, Hud_ToggleWindow)
VOID3(gbClearMgTrails, ClearMgTrails)
VOID3(gbClearFireFlags, GameB_ClearFireFlags)
VOID3(gbLoadLevel, GameB_LoadLevel)
VOID3(gbMaybeAddItem, GameB_MaybeAddItem)
VOID3(gbDeathTallies, Bank6_DrawDeathTallies)
VOID3(gbSixAnimSprites, Bank6_DrawSixAnimSprites)
VOID3(gbDrawRankScreen, GameB_DrawRankScreen)
VOID3(gbMoveStars, RankScreen_MoveStars)
VOID3(gbScrollSpritesSkip4, gbJpScrollSpritesSkip4)
VOID3(gbCutsceneTimerStep, GameB_CutsceneTimerStep)
VOID3(gbRankScreenFrame, RankScreen_Frame)
VOID3(gbTogglePause, GameB_TogglePause)
VOID3(gbSoftReset, GameB_State2b_SoftReset)
AVOID3(gbPlaySFX, PlaySFX)
AVOID3(gbPlayDeb8, PlaySound_DEB8)
AVOID3(gbPlayMusic, PlayMusic)
AVOID3(gbPlayDeb0, PlaySound_DEB0)
AVOID3(gbSetPalettes, SetPalettes)
AVOID3(gbSetPalettesD31f, gbJpSetPalettesD31f)
AVOID3(gbClearSpritesFrom, ClearSpritesFrom)
void gbTilemap3(const uint8_t *s) __naked { __asm__(FAR3(DrawTilemapScript) "ret"); }

uint8_t gbSub52ab(void) __naked { __asm__(FAR3(GameB_Sub_52ab) "ret"); }
uint8_t gbSideAdjustFlipped(uint8_t c) __naked { __asm__("ld c, a\n" FAR3(GameB_GetSideAdjustFlipped) "ret"); }
uint8_t gbAdvanceScrollScript(void) __naked { __asm__(FAR3(GameB_AdvanceScrollScript) CARRY); }
uint8_t gbAnimTick(void) __naked { __asm__(FAR3(GameB_AnimTick) CARRY); }
uint8_t gbLevelSelectInput(void) __naked { __asm__(FAR3(GameB_LevelSelectInput) CARRY); }
void gbHudPush(uint8_t cmd, uint8_t param) __naked { __asm__("ld c, e\n" FAR3(HudQueue_Push) "ret"); }
void gbLinkTxPush(uint8_t cmd, uint8_t param) __naked { __asm__("ld c, e\n" FAR3(LinkTxQueue_Push) "ret"); }
void gbAnimateWaves(uint8_t c) __naked { __asm__("ld c, a\n" FAR3(GameB_AnimateWaves) "ret"); }
void gbHudCmd14(uint8_t b) __naked { __asm__("ld b, a\n" FAR3(Hud_Cmd14) "ret"); }
void gbSub4561(uint8_t e) __naked { __asm__("ld e, a\n" FAR3(GameB_Sub_4561) "ret"); }
void gbDrawTableString(const uint8_t *table, uint8_t i) __naked { __asm__("ld h, d\nld l, e\n" FAR3(DrawTableString) "ret"); }
void gbMemClear(uint8_t *p, uint16_t n) __naked { __asm__("ld h, d\nld l, e\n" FAR3(MemClear) "ret"); }
void gbFillBgFa(void) __naked { __asm__("ld hl, #0x9800\nld bc, #0x0240\nld e, #0xfa\n" FAR3(MemFill) "ret"); }
void gbClearSpriteEntries(uint8_t *p, uint8_t n) __naked { __asm__("ld h, d\nld l, e\nld c, a\n" FAR3(ClearSpriteEntries) "ret"); }
uint8_t *gbMapSelectedShip(void) __naked { __asm__(FAR3(GameB_MapSelectedShip) "ld b, h\nld c, l\nret"); }
uint8_t gbLookup6e78(uint8_t a) __naked { __asm__(FAR3(GameB_LookupData6e78) "ld a, c\nret"); }
uint8_t *gbSaveToWreckSlot(uint8_t *ship) __naked { __asm__(FAR3(GameB_SaveToWreckSlot) "ld b, h\nld c, l\nret"); }
void gbDrawObjMeta2(uint16_t pos, uint16_t frameAttr) __naked { __asm__("ld a, b\n" FAR3(GameB_DrawObjMeta2) "ret"); }
void gbDepthCharges(uint8_t d) __naked { __asm__("ld d, a\n" FAR3(GameB_DrawDepthChargesAt) "ret"); }

// Bank 6 metasprites: DE = position, A = index into the table.
#define META(name, table) void name(uint16_t pos, uint8_t i) __naked \
    { __asm__("ld c, a\nld hl, #_" table "\n" FAR3(Bank6_DrawMetasprite) "ret"); }
META(gbMeta00, "MetaspritePtrs")
META(gbMeta1a, "MetaspritePtrs+0x1a")
META(gbMeta3a, "MetaspritePtrs+0x3a")
META(gbMetaObj6, "GameB_ObjMetaspritePtrs+6")

// RLE draws: p[0] = map address (HL), p[1] = source (DE). Both move on.
#define RLE(name, f) void name(uint16_t *p) __naked { __asm__( \
    "push de\nld h, d\nld l, e\nld a, (hl+)\nld c, a\nld a, (hl+)\nld b, a\n" \
    "ld a, (hl+)\nld e, a\nld d, (hl)\nld h, b\nld l, c\n" FAR3(f) \
    "pop bc\nld a, l\nld (bc), a\ninc bc\nld a, h\nld (bc), a\ninc bc\n" \
    "ld a, e\nld (bc), a\ninc bc\nld a, d\nld (bc), a\nret"); }
RLE(gbRle, DrawTilesRLE)
RLE(gbRleRows, DrawTilesRLERows)
RLE(gbRleRowsThenRle, DrawRLERowsThenRLE)
RLE(gbRleTwoParts, DrawTilesRLE_TwoParts)

// Bank 1 cutscene code.
VOID1(gbB1Tally00, Bank1_TallyVectors)
VOID1(gbB1Tally03, Bank1_TallyVectors+3)
VOID1(gbB1Tally20, Bank1_TallyVectors+0x20)
VOID1(gbB1Tally23, Bank1_TallyVectors+0x23)
VOID1(gbB1Tally26, Bank1_TallyVectors+0x26)
VOID1(gbB1Tally29, Bank1_TallyVectors+0x29)
VOID1(gbB1TallyState3, Vec1_GameB_TallyState3_ComputeTimeBonus)
VOID1(gbB1TallyState4, Vec1_GameB_TallyState4_AccumulateScore)
VOID1(gbB1TallyState5, Vec1_GameB_TallyState5_WaitInput)
VOID1(gbB1TallyInit, Vec1_GameB_TallyInitScreen)
VOID1(gbB1CutState1, Vec1_GameB_CutsceneState1_DrawScene)
VOID1(gbB1CutState2, Vec1_GameB_CutsceneState2_MoveObjects)
VOID1(gbB1CutState3, Vec1_GameB_CutsceneState3_Scroll)
VOID1(gbB1ScrollTiles, Vec1_GameB_CutsceneScrollTiles)
VOID1(gbB1AnimateSprites, Vec1_GameB_CutsceneAnimateSprites)
