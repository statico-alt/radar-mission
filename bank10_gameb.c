// Game B sub-states (GameB_SubStateTable in bank_000.asm). The table
// entries of these states point at one ROM0 stub that runs gbRunState.
#include "bank0.h"
#include "bank10_gameb.h"

#define AT(v, i) (((uint8_t *)&(v))[i])

// DecAndCheckTimer: decrement *p unless it is 0. Returns the new value
// (0 = the asm z flag).
static uint8_t decAndCheck(uint8_t *p)
{
    uint8_t v = *p;
    if (v)
        *p = --v;
    return v;
}

static uint8_t tickDeathMarker(void)
{
    return decAndCheck(&g_deathMarker);
}

// GameB_TickLevelClear. 0 = done.
static uint8_t tickLevelClear(void)
{
    if (decAndCheck(&g_levelClearTimer))
        return 1;
    if (!g_soundFadeState)
        return 0;
    return tickDeathMarker();
}

static uint8_t getSideOffset(void)
{
    return (g_sceneFlagsC & 0x04) ? 0 : 4;
}

// GameB_StepD413: step g_cycle11 through 0..10. 1 = carry (no wrap).
static uint8_t stepD413(void)
{
    if (++g_cycle11 < 11)
        return 1;
    g_cycle11 = 0;
    return 0;
}

// GameB_StepCounters. 1 = carry (g_stepCountA was below b).
static uint8_t stepCounters(uint8_t b, uint8_t c)
{
    if (g_stepCountA >= b)
        return 0;
    if (AT(g_stepCountA, 1)++ >= c) {
        AT(g_stepCountA, 1) = 0;
        g_stepCountA++;
    }
    return 1;
}

static void startLevelClear(void)
{
    g_levelClearTimer = 3;
    g_deathMarker = 0xff;
    gbPlayDeb0(5);
}

static void setScrollY5(void)
{
    rSCY = 5;
    rSCX = 0;
}

static void resetScrollCursors(void)
{
    ScrollScript_SlotA_Cursor = 0;
    ScrollScript_SlotB_Cursor = 0;
    g_metaspriteOamSlot = 0;
    g_oamBufToggle &= ~0x01;
}

static void clearModeFlag16_7(void)
{
    g_modeFlags16 &= ~0x80;
}

// Store the next state twice in the g_dispatchTableD346 list, then $ff.
static void setResultStates(uint8_t e)
{
    AT(g_dispatchTableD346, 0) = e;
    AT(g_dispatchTableD346, 1) = e;
    AT(g_dispatchTableD346, 2) = 0xff;
}

static void setSubStateAndNext(uint8_t a, uint8_t c)
{
    g_subState = a;
    AT(g_dispatchTableD346, 0) = c;
    AT(g_dispatchTableD346, 1) = c;
}

static void lcdOnRestoreIE(uint8_t lcdc)
{
    rLCDC = lcdc;
    rIF = 0;
    rIE = g_savedIE;
    __asm__("ei");
}

static void waitLine2AndLcdOn(void)
{
    while (rLY < 2)
        ;
    g_subStateScratch4 = 0x0f;
    rLCDC = 0xe3;
}

static void resetScrollScriptPos(void)
{
    g_linkSyncIndex = 0;
    ScrollScript_LastTileByte = 0;
    AT(ScrollScript_LastTileByte, 1) = 0;
}

static void lcdOffAndReset(void)
{
    GameB_LcdOff();
    gbClearShadowSprites();
}

// Turn the LCD off, clear the scene RAM, fill the BG map with tile $fa.
static void clearSceneRam(void)
{
    lcdOffAndReset();
    gbMemClear(&g_aiScanIndex, 0x172);
    gbMemClear(&g_gameBObjSlots, 0x18);
    gbMemClear(&g_spawnObjSlots, 0x7e);
    GameB_ResetScroll();
    HideWindow();
    gbFillBgFa();
    gbDrawStatusScreen();
    gbClearDamageGauge();
    g_modeFlags16 &= ~0x40;
    if (g_soundFadeState)
        gbPlaySFX(7);
    resetScrollScriptPos();
}

static void drawObjMeta2(uint16_t pos, uint8_t frame, uint8_t attr)
{
    gbDrawObjMeta2(pos, (uint16_t)frame << 8 | attr);
}

// ---- Shared drawing helpers ----

// Every 16 frames step $d091 and flip the scene phase. After 4 steps
// go to the next sub-state (in $0f: set the death marker to 30).
static void blinkPhase(void)
{
    uint8_t b = g_frameCounter;
    if (b & 0x0f)
        return;
    g_gameBLayoutVariant++;
    uint8_t c = (b >> 3) & 0x02;
    if (g_subState != 0x0d)
        c++;
    g_gameBScenePhase = c;
    gbHudPush(0x0e, c);
    if (c & 0x02)
        return;
    gbPlaySFX((c & 1) ? 0x0b : 0x15);
    if (g_gameBLayoutVariant < 4)
        return;
    if (g_subState != 0x0f)
        g_subState++;
    else
        g_deathMarker = 0x1e;
}

static void drawMetasprite4B4A(void)
{
    g_metaspriteOamSlot = 0;
    g_oamBufToggle &= ~0x01;
    gbMetaObj6(0x4473, 0x40);
}

// The pending-kill badge (blinks) and, in 1P, the stage icon.
static void drawKillBadge(void)
{
    uint8_t a = 0;
    g_metaspriteOamSlot = 0;
    if ((g_frameCounter & 0x20) && (g_killFlagsPending & 0xe0)) {
        if (g_subState == 0x08) {
            if (!(g_modeFlags1e & 0x40))
                gbPlayDeb0(0x1e);
            g_modeFlags1e |= 0x40;
        }
        a = g_killFlagsPending & 0xe0;
    } else {
        g_modeFlags1e &= ~0x40;
    }
    gbMeta00(0x3028, (a >> 5) + 5);
    if (g_sceneFlagsC & 0x08)
        return;
    gbMeta3a(0x7c80, g_gameBStage + 0x0d);
}

static void draw2PBadge(void)
{
    if (!(g_sceneFlagsC & 0x08))
        return;
    gbMeta3a(0x4876, 2);
    uint8_t e = g_frameCounter;
    if (e & 0x0f)
        return;
    gbHudPush(0x0b, ((e & 0x10) >> 4) + 2);
}

static void drawEndBanner(void)
{
    uint8_t f = g_sceneFlagsC;
    if (f & 0x01)
        return;
    if (!(g_subStateScratch9 & 0x01))
        return;
    if (rWY != 0x70)
        return;
    uint8_t c = 0x43, a = 9;
    uint16_t de = 0x1888;
    if (f & 0x08) {
        c++;
        a = 4;
        de = 0x1890;
    }
    if (!(g_sceneFlagsB & 0x20))
        return;
    drawObjMeta2(de, a, c);
}

// The sinking sub, only when g_sceneFlagsC & $8b = $8a.
static void drawGameOverSub(void)
{
    g_metaspriteOamSlot = 0;
    if ((g_sceneFlagsC & 0x8b) != 0x8a)
        return;
    gbMeta3a(0x4838, 8);
    uint8_t a = g_soundClock;
    if (a >= 0x0f)
        gbMaybeAddItem();
    if (a >= 0x10) {
        if (g_sixAnimCounter1 >= 0x13) {
            g_modeFlags1e |= 0x04;
            return;
        }
        if (g_modeFlags1e & 0x10) {
            if (!(g_modeFlags1e & 0x20))
                gbPlayDeb8(2);
            g_modeFlags1e |= 0x20;
        }
        if (!(g_frameCounter & 0x03))
            g_sixAnimCounter1++;
        gbDepthCharges(0x50);
        return;
    }
    if (a < 0x0e || (g_modeFlags1e & 0x04))
        return;
    g_modeFlags1e |= 0x10;
    gbMeta3a(0x4800 | ++g_sixAnimCounter3, (g_sceneFlagsC & 0x04) ? 0x0a : 0x09);
}

// The death tallies, the end score and the end jingles, then clear the
// unused sprites.
static void drawEndScoreAndSound(void)
{
    gbDeathTallies();
    uint16_t pos = (uint16_t)g_scratch92 << 8 | AT(g_scratch92, 2);
    const uint8_t *t = GameB_LevelEndAnimPtrs + (uint8_t)((g_gameBLevel - 1) << 1);
    const uint8_t *p = (const uint8_t *)(farByte(3, t) | (uint16_t)farByte(3, t + 1) << 8);
    uint8_t b = farByte(3, p++);
    uint8_t c, a;
    if (g_gameBStateTimer == 0xff) {
        c = b - 1;
    } else if (!(g_modeFlags1d & 0x02)) {
        if (g_soundClock)
            g_modeFlags1d |= 0x02;
        g_soundClock = 0;
        c = 0;
    } else {
        c = g_soundClock;
        if (c >= b)
            c = g_soundClock = 0;
    }
    p += c;
    c = farByte(3, p);
    a = farByte(3, p + 1);
    if (a >= 0xfe)
        g_gameBStateTimer = a;
    if (!c) {
        a = 0x12;
        goto sfx;
    }
    if (g_sceneFlagsC & 0x01) {
        if (c < 0x10)
            goto clear3;
        a = c;
        c = 6;
        if (a == 0x10) {
            a = 0x11;
            goto sfx;
        }
    } else if (g_gameBStateTimer != 0xff) {
        goto clear3;
    }
    if (g_sceneFlagsC & 0x80) {
        if (!(g_modeFlags1d & 0x04))
            gbPlaySFX((g_sceneFlagsC & 0x01) ? 0x05 : 0x17);
        g_modeFlags1d |= 0x04;
    }
    gbSixAnimSprites();
    if ((g_sceneFlagsC & 0x01) && !(g_modeFlags1d & 0x20)) {
        g_modeFlags1d |= 0x20;
        gbHudPush(1, getSideOffset());
    }
clear3:
    g_modeFlags1d &= ~0x08;
    goto draw;
sfx:
    if (!(g_modeFlags1d & 0x08))
        gbPlaySFX(a);
    g_modeFlags1d |= 0x08;
draw:
    gbMeta1a(pos, c);
    a = g_metaspriteOamSlot;
    if (a != 0x28)
        gbClearSpriteEntries(&g_oamBuffer + (uint8_t)(a << 2), 0x28 - a);
}

// ---- The play screen (sub screen) ----

static void enableLcdAfterLine2(void)
{
    GameB_EnableIrqs();
    waitLine2AndLcdOn();
}

static void loadSubResetPlayer(void)
{
    g_subStateScratch4 = 0x0d;
    gbClearShadowSprites();
    gbSetPalettes(1);
    AT(g_mgTrailEmitTimer, 0) = 0;
    AT(g_mgTrailEmitTimer, 1) = 0;
    g_mgBurstCooldownA = 0;
    AT(g_mgBurstCooldownA, 1) = 0;
    g_mgHitMarker = 0;
    AT(g_mgHitMarker, 1) = 0;
    g_weaponEdges &= ~0x02;
    AT(g_weaponEdges, 1) |= 0x01;
    g_subStateScratch9 &= 0x03;
}

// State $2f: open the sub screen.
static void state2f(void)
{
    if (g_modeFlags1a & 0x08)
        return;
    if (!(g_modeFlags17 & 0x40))
        loadSubResetPlayer();
    gbLinkTxPush(0x88, 3);
    gbClearDamageGaugeB();
    farCopy(3, &g_rasterLycTable, GameB_RasterTables + 2, 0x2b);
    g_rasterLine = 0;
    g_d0dbCounter = 0x77;
    g_rasterScy1 = 0x70;
    AT(g_rasterScy1, 1) = 0x87;
    g_sceneFlagsA |= 0x04;
    gbStartDamageGauge();
    gbHudToggleWindow();
    gbPlayDeb8(9);
    g_subStateScratch8 = 3;
    if (g_subState == 0x10)
        return;
    uint8_t a = g_modeFlags17;
    g_modeFlags17 = a & ~0x40;
    if (!(a & 0x40))
        enableLcdAfterLine2();
}

static void resetPlayState(void)
{
    gbClearShadowSprites();
    g_modeFlags10 = (g_modeFlags10 | 0x10) & ~0x40;
    g_modeFlags1b &= ~0x04;
    g_mgTrailPhase &= ~0x10;
    g_modeFlags16 = gbSub52ab();
    gbClearFireFlags();
    gbHudPush(4, g_sideSpeed);
    gbHudPush(6, 0);
    g_subStateScratch9 = (g_subStateScratch9 & 0x03) | 0x04;
    g_mgBurstCooldownA = 0;
    AT(g_mgBurstCooldownA, 1) = 0;
    g_sceneFlagsA &= ~0x04;
    g_subStateScratch4 = 0x0d;
    setScrollY5();
    lcdOnRestoreIE(0x8b);
}

static void returnFromSub(void)
{
    resetPlayState();
    gbSetPalettes(0);
    Vec6_GameB_InitScaledTable();
    gbClearMgTrails();
    g_subStateScratch8 = 0;
}

// State $30: when the sub is hit, go to state $18.
static void state30(void)
{
    if (!(g_sceneFlagsC & 0x02)) {
        returnFromSub();
        return;
    }
    if (g_subState <= 0x18 || (g_sceneFlagsC & 0x01)) {
        g_modeFlags17 |= 0x40;
        g_subState = 0x18;
    }
}

// ---- Scene end ----

static void sceneEndTick(void)
{
    gbEngine();
    g_rasterScx3 = 0x60;
    if (g_sceneFlagsC & 0x01)
        return;
    if (!(g_sceneFlagsB & 0x20)) {
        gbAdvanceScrollScript();
        return;
    }
    if (!(g_modeFlags1e & 0x20))
        gbPlayDeb8(1);
    g_modeFlags1e |= 0x20;
    drawEndBanner();
}

// Bump g_gameBStage up to 3. 1 = it moved.
static uint8_t bumpStage(void)
{
    uint8_t a = g_gameBStage + 1;
    if (a >= 3)
        return 0;
    g_gameBStage = a;
    return 1;
}

// ---- The states ----

static void state00(void)
{
    VOLATILE(g_gameALevel) |= 0x08;
    g_sceneFlagsC = ((g_topModeFlags & 0xc0) >> 4) ^ 0x40;
    GameB_SetRasterFlags(0x0d);
    g_gameBLinkState = 0;
    g_gameBStage = 0;
    g_linkStartMarker = 0xff;
    g_levelClearTimer = 6;
    rLCDC = 0x83;
    g_subState = 4;
}

static void state01(void)
{
    uint16_t p[2];
    gbClearDamageGauge();
    g_gameBInitTimer = 4;
    clearSceneRam();
    gbSetPalettes(0);
    farCopy(6, (uint8_t *)0x8000, GameB_GfxSprites, 0x800);
    farCopy(7, (uint8_t *)0x8800, (g_sceneFlagsC & 0x80) ? GameB_SceneAltTiles : GameB_MenuTiles, 0x800);
    farCopy(7, (uint8_t *)0x9000, GameB_OutfitTiles, 0x800);
    gbMarkLinkEnd();
    g_linkSyncIndex = 0;
    p[0] = 0x9800;
    p[1] = (uint16_t)GameB_SceneTilemap;
    gbRle(p);
    gbRleRows(p);
    p[1] = (uint16_t)GameB_SceneTilemap;
    gbRle(p);
    p[0] = 0x98c2;
    p[1] = (uint16_t)GameB_SceneTilemap + 0x71;
    gbRleTwoParts(p);
    gbDrawSideHeader();
    uint8_t e = getSideOffset();
    if ((g_sceneFlagsC & 0x8b) != 0x8b)
        gbSub4561(e);
    gbPickLevelValue();
    gbPlayMusic(0x19);
    gbDrawTableString(GameB_MessagePtrs, g_gameBLevel - 1);
    g_subState = 2;
    farCopy(6, &g_gameBLayoutVariant, InitD091, 4);
    AT(g_gameBLayoutVariant, 4) = 0;
    AT(g_gameBLayoutVariant, 5) = 0;
    if ((g_sceneFlagsC & 0x81) == 0x80) {
        g_scratch92 = 0x40;
        g_depthChargeOrigin = 0x58;
        g_sixAnimCounter3 = 0x58;
        g_sixAnimCounter1 = 0;
    } else {
        farCopy(3, &g_sixAnimCounter0, GameB_SixAnimCounterDefaults, 6);
    }
    GameB_EnableIrqs();
    rLCDC = 0x83;
    g_subStateScratch4 = 0x0d;
    g_tallyIconGrow = 0;
    g_gameBStateTimer = 0;
    g_modeFlags1d &= ~0x24;
    g_modeFlags1e &= ~0x31;
    if (g_sceneFlagsC & 0x80) {
        g_sixAnimCounter3 = 0x38;
        g_deathMarker = 0xd4;
        g_killFlagsPending &= ~0x10;
        gbPlayMusic(0x15);
        setResultStates(0x2c);
    } else {
        g_deathMarker = 4;
        setResultStates(0x03);
    }
}

static void state02(void)
{
    if (decAndCheck(&g_gameBInitTimer))
        return;
    drawGameOverSub();
    drawEndScoreAndSound();
    if (!(g_modeFlags1e & 0x01)) {
        if (g_gameBStateTimer < 0xfe)
            return;
        g_modeFlags1e |= 0x01;
        if (tickDeathMarker())
            return;
    }
    gbReadPadAndClock();
}

static void state03(void)
{
    g_subStateScratch4 = 0x0d;
    rLCDC = 0x83;
    GameB_ResetScroll();
    ScrollScript_LastTileByte = 1;
    g_levelClearTimer = 0x10;
    g_subState = (g_sceneFlagsC & 0x80) ? 0x2c : 0x07;
}

static void state05(void)
{
    uint8_t a = 3;
    if (g_sceneFlagsC & 0x08) {
        g_gameBLinkState = 4;
        if (!(g_modeFlags16 & 0x80))
            return;
        g_modeFlags16 &= ~0x80;
        g_linkStartMarker = 0x0f;
        a = AT(g_linkSwapPair, 0);
        AT(g_linkSwapPair, 0) = AT(g_linkSwapPair, 1);
        AT(g_linkSwapPair, 1) = a;
        gbMarkLinkEnd();
        a = farByte(3, KillMarkerColumnOffsets + 3 + g_gameBSkill);
    }
    g_killCount = a;
    a <<= 1;
    g_deathMarker = a;
    g_levelClearTimer = a;
    setResultStates(0x07);
    GameB_SetRasterFlags(0x0f);
    g_subState++;
}

static void state06(void)
{
    gbTitleCutscene();
    if (tickDeathMarker())
        return;
    gbReadPadAndClock();
}

// ---- State $08: the outfit menu ----
// ScrollScript_LastTileByte is the cursor: bit 7 = bottom row, bit 6 =
// right side, bits 0-5 = item column (0..2).

static void menuDrawCursor(void)
{
    gbMeta3a(0x3076, 1);
}

// KillFlagMask: the g_killFlagsPending bit of the item column.
static uint8_t killFlagMask(void)
{
    return 0x20 << (ScrollScript_LastTileByte & 0x3f);
}

// A or B on the top row: toggle the item under the cursor.
static void menuToggleItem(uint8_t b, uint8_t col)
{
    uint8_t c, a, d;
    if (!(b & 0x01)) {
        if (!(b & 0x02) || !(g_killFlagsPending & killFlagMask())) {
            gbMeta3a(farByte(3, HudPanelColumnOffsets + 3 + col) << 8 | 0x68, 0);
            return;
        }
    }
    c = killFlagMask();
    a = g_killFlagsPending ^ c;
    g_killFlagsPending = a;
    d = (a & c) ? 1 : 0;
    gbHudPush(0x08, (uint8_t)(ScrollScript_LastTileByte << 1) + d);
    uint8_t l;
    if (!(g_killFlagsPending & c)) {
        a = g_killCount++;
        c = a | 0x80;
        a = 0x01;
        l = 0x08;
    } else {
        if (!g_killCount) {
            gbPlayDeb8(5);
            g_killFlagsPending ^= c;
            return;
        }
        c = --g_killCount;
        a = 0x11;
        l = 0x00;
    }
    gbPlaySFX(a);
    gbHudPush(0x09, c);
    gbHudPush(0x0a, ScrollScript_LastTileByte | l);
}

static void state08(void)
{
    uint8_t *hl = &ScrollScript_LastTileByte;
    uint8_t b, a;
    drawKillBadge();
    b = g_joyEdge;
    if (!(*hl & 0x80)) {
        if (!(b & 0x80)) {
            a = *hl & 0x3f;
            if (b & 0x20) {
                *hl &= ~0x40;
                if (a) {
                    *hl = (*hl & 0xc0) | (a - 1);
                    gbPlaySFX(1);
                    return;
                }
            } else if (b & 0x10) {
                *hl |= 0x40;
                if (a < 2) {
                    *hl = (*hl & 0xc0) | (a + 1);
                    gbPlaySFX(1);
                    return;
                }
            } else {
                menuToggleItem(b, a);
                return;
            }
        }
        *hl |= 0x80;
        gbPlaySFX(1);
        return;
    }
    if (b & 0xf0)
        *hl &= ~0x80;
    if (b & 0x09) {
        menuDrawCursor();
        clearModeFlag16_7();
        g_modeFlags1b |= 0x02;
        SFX_Queue = 5;
        g_savedWeaponFlags = g_killFlagsPending;
        g_sceneFlagsA |= 0x40;
        g_subState++;
        return;
    }
    if (b & 0x20)
        *hl &= ~0x40;
    else if (b & 0x10)
        *hl |= 0x40;
    else {
        menuDrawCursor();
        return;
    }
    *hl = (*hl & 0x40) | ((*hl & 0x40) ? 0 : 2);
    menuDrawCursor();
}

static void state09(void)
{
    drawKillBadge();
    draw2PBadge();
    if (g_sfxSq1Id)
        return;
    startLevelClear();
    g_subState++;
}

static void state0a(void)
{
    drawKillBadge();
    draw2PBadge();
    if (tickLevelClear())
        return;
    g_modeFlags16 &= ~0x80;
    g_gameBLinkState = 1;
    g_levelClearTimer = 6;
    g_subState++;
}

static void state0b(void)
{
    drawKillBadge();
    draw2PBadge();
    if (g_sceneFlagsC & 0x08) {
        if (!(g_modeFlags16 & 0x80))
            return;
        if (decAndCheck(&g_levelClearTimer))
            return;
    }
    g_modeFlags16 &= ~0x40;
    g_gameBLinkState = 5;
    g_levelClearTimer = 6;
    g_subState++;
}

// State $0c: load the scene graphics and reset the per-scene counters.
static void state0c(void)
{
    uint16_t p[2];
    uint8_t *hl;
    if (g_sceneFlagsC & 0x08) {
        if (!(g_modeFlags16 & 0x40))
            return;
        if (--g_levelClearTimer)
            return;
    }
    lcdOffAndReset();
    farCopy(7, (uint8_t *)0x8000, GameB_SceneTiles, 0x1800);
    gbDrawStatusPattern();
    p[0] = 0x9c00;
    p[1] = (uint16_t)GameB_DamageStepTable + 0x0e;
    gbRle(p);
    p[1] = (uint16_t)GameB_DamageStepTable + 0xaa;
    gbRleRowsThenRle(p);
    gbRleRowsThenRle(p);
    gbHudDrawWeaponIcons();
    gbHudCmd14(2);
    gbHudCmd14(3);
    gbDrawSideHeader();
    gbHudDrawCounters();
    gbTilemap3(GameB_DamageStepTable + 0xde);
    uint8_t a = (gbSideAdjustFlipped(0) << 1) + 0x12;
    *(uint8_t *)0x9c09 = a;
    *(uint8_t *)0x9c0a = a + 1;
    Vec6_GameB_InitShipArrays();
    Vec6_GameB_UpdateWeaponFlags();
    returnFromSub();
    gbMemClear(&AT(g_killCountsByClass, 0), 7);
    gbMemClear(&g_hudQueueWrite, 0x2c);
    rWY = 0;
    g_gameBLayoutVariant = 0;
    VOLATILE(g_frameCounter) = 0;
    g_sideSpeed = 0;
    g_linkPad = 0;
    hl = &g_gameClock;
    hl[0] = hl[1] = hl[2] = 0;
    AT(g_startClearPair, 0) = AT(g_startClearPair, 1) = 0;
    g_torpedoFireCount = AT(g_torpedoFireCount, 1) = 0;
    g_kind22ActiveCount = 0;
    hl = &g_screenEffectTimers;
    hl[0] = hl[1] = hl[2] = hl[3] = 0;
    g_livesGameB = 3;
    g_torpedoCooldown = AT(g_torpedoCooldown, 1) = 3;
    g_hpA = AT(g_hpA, 1) = 0x18;
    setScrollY5();
    g_gameBScenePhase = 2;
    g_sceneFlagsA &= ~0x08;
    g_sceneFlagsB &= ~0xa0;
    gbResetSceneFlags();
    clearModeFlag16_7();
    AT(g_dispatchTableD346, 0) = 0x10;
    AT(g_dispatchTableD346, 1) = 0x0e;
    AT(g_dispatchTableD346, 2) = 0xc8;
    gbMarkLinkEnd();
    gbPlayMusic(0xff);
    gbPlayMusic(0x08);
    g_subState++;
}

static void state0d(void)
{
    drawMetasprite4B4A();
    gbDrawPlayerSlots();
    gbDrawPlayerSlotsD1c0();
    blinkPhase();
}

static void state0e(void)
{
    if (tickDeathMarker())
        return;
    gbHudPush(0x0e, 0);
    g_gameBScenePhase = 3;
    g_gameBLayoutVariant = 0;
    VOLATILE(g_frameCounter) = 0;
    g_gameClock = 0;
    g_depthBand = 0x10;
    gbMarkLinkEnd();
    g_subState++;
}

static void state0f(void)
{
    drawMetasprite4B4A();
    uint8_t a = g_gameBScenePhase;
    g_gameBScenePhase = a & ~0x02;
    gbDrawPlayerSlots();
    gbDrawPlayerSlotsD1c0();
    g_gameBScenePhase = a;
    gbDrawEnemySlots();
    gbDrawEnemySlotsD258();
    if (g_gameBLayoutVariant < 4) {
        blinkPhase();
        return;
    }
    if (tickDeathMarker())
        return;
    g_subState++;
}

static void state10(void)
{
    g_sfxBlockNoise09 = 0;
    gbResetPlayerShip();
    gbHudPush(0x0e, 0);
    gbHudPush(0x0e, 1);
    g_gameBLinkState = 2;
    VOLATILE(g_frameCounter) = 0;
    g_depthChargeOrigin = 0x22;
    state2f();
    gbMemClear(&g_linkMsg, 6);
    g_subState++;
    waitLine2AndLcdOn();
}

static void state12(void)
{
    g_weaponEdges &= ~0x01;
    g_stepCountA = 0;
    AT(g_stepCountA, 1) = 0;
    gbClearSpritesFrom(0);
    g_subState++;
}

static void state13(void)
{
    gbSubDispatchD009();
    if (!(g_subStateScratch9 & 0x01))
        return;
    uint8_t e = g_stepCountA;
    if (stepCounters(6, 4)) {
        if (!(e & 0x02))
            drawObjMeta2(0x4c1a, 3, 0x41);
        return;
    }
    g_modeFlags1e &= ~0x04;
    gbPlayMusic(0x18);
    g_sceneFlagsA &= ~0x40;
    VOLATILE(g_topModeFlags) &= ~0x02;
    g_subState++;
}

static void state15(void)
{
    SoundUpdate();
    g_rasterScx3 = 0x60;
    if (!stepCounters(3, 5)) {
        if (!(g_modeFlags1a & 0x01)) {
            g_subState = 0x14;
            gbClearSpritesFrom(0);
            return;
        }
        if (g_joyEdge & 0x08) {
            gbTogglePause();
            return;
        }
    }
    uint16_t de = (g_modeFlags10 & 0x10) ? 0x4473 : 0x441a;
    resetScrollCursors();
    drawObjMeta2(de, 5, 0x42);
}

static void state16(void)
{
    gbEngine();
    gbSendLinkStatus();
    if (!(g_sceneFlagsC & 0x20))
        return;
    if (decAndCheck(&g_levelClearTimer))
        return;
    uint8_t f = g_sceneFlagsC;
    if ((f & 0x01) && (f & 0x10)) {
        g_sceneFlagsC = f & ~0x01;
        g_sceneFlagsB |= 0x80;
    } else if (!(f & 0x01) && !(f & 0x10)) {
        g_sceneFlagsB |= 0x80;
    }
    if (g_modeFlags1d & 0x80)
        g_selectedEnemyShip = 0;
    gbKillConfirm();
}

static void state18(void)
{
    gbSendLinkStatus();
    if (g_modeFlags1a & 0x08) {
        gbEngine();
        return;
    }
    SoundUpdate();
    g_modeFlags16 = 0;
    gbResetPlayerShip();
    state2f();
    g_subState++;
}

static void state1a(void)
{
    uint8_t a, c;
    if (decAndCheck(&g_gameBKillTimer))
        return;
    gbMemClear(&g_spawnObjSlots, 0x40);
    SoundUpdate();
    gbSub5b11();
    gbResetEnemySub();
    g_shipsSpawned = 0;
    VOLATILE(g_frameCounter) = 0;
    c = g_gameBSkill;
    a = g_gameBShipCountM1;
    if (g_sceneFlagsC & 0x01) {
        if (!(g_sceneFlagsB & 0x20)) {
            a = 9;
            c = 1;
        }
    } else if (g_sceneFlagsB & 0x20) {
        c = 3;
    }
    g_waveSpawnCount = a;
    AT(g_waveSpawnCount, 1) = c;
    AT(g_waveSpawnCount, 2) = 0;
    g_killFlagsPending &= ~0x08;
    gbSetLevelClearTimer();
    g_subState++;
}

static void state1b(void)
{
    g_sfxBlockSfx0B = 0;
    g_sfxBlockAll = 0;
    g_sfxNoiseBusy = 0;
    g_sfx04Busy = 0;
    clearModeFlag16_7();
    gbPlayMusic(0xff);
    g_subState++;
}

// Restore the scroll script from the template and pick the next music.
static void restoreScroll(void)
{
    __asm__("di");
    copyBytes(&g_bannerVramPage, &ScrollScript_TemplateFlag, 2);
    copyBytes(&ScrollScript_PrimaryPtr_Lo, &ScrollScript_TemplatePrimaryLo, 6);
    __asm__("ei");
    g_modeFlags15 &= ~0x20;
    if (g_sceneFlagsC & 0x01) {
        g_gameBInitTimer = (g_sceneFlagsB & 0x20) ? 0x0a : 0x4a;
    } else {
        if (!(g_sceneFlagsB & 0x20)) {
            g_modeFlags1e &= ~0x04;
            g_deathMarker = 0x80;
        }
        gbPlayMusic(0x1b);
    }
    g_modeFlags1e &= ~0x08;
    if ((g_sceneFlagsC & 0x01) && !(g_sceneFlagsB & 0x20))
        gbMarkEnemyShip0();
    g_subState++;
}

// State $1c in 2P: spawn ships from the queue at $d450 until it is empty.
static void state1c(void)
{
    if (!(g_sceneFlagsC & 0x01)) {
        restoreScroll();
        return;
    }
    uint8_t *ship = gbMapSelectedShip() + 6;
    if (!g_waveSpawnCount) {
        gbEngine();
        if (g_frameCounter & 0x01)
            return;
        if (g_bannerTileIndex >= 0x36)
            restoreScroll();
        else
            gbHudPush(0x0f, 4);
        return;
    }
    if (!stepD413()) {
        g_waveSpawnCount--;
        uint8_t *s = gbSaveToWreckSlot(ship);
        s[1] |= 0x80;
        uint8_t c = farByte(3, GameB_SpriteHitWidths + gbLookup6e78(s[2])) | 0x01;
        uint16_t x = WORD(g_selectedShipX);
        uint16_t de = x - c;
        if (x < c)
            de += WORD(g_worldWidth);
        uint8_t r = ScrollScript_LastTileByte + g_frameCounter;
        ScrollScript_LastTileByte = r;
        c <<= 1;
        while (r >= c)
            r -= c;
        de += r;
        s[3] = de;
        s[4] = de >> 8;
        s[5] = 1;
        s[6] |= 0x80;
        gbPlayNextTableSound();
    }
    gbEngine();
}

// State $1d: the main play state.
static void state1d(void)
{
    gbEngine();
    if (!(g_sceneFlagsC & 0x01)) {
        if (g_gameBShipCountM1 >= g_shipsSpawned) {
            if (stepD413())
                return;
            uint8_t c = g_shipsSpawned++;
            if (g_sceneFlagsB & 0x20)
                gbHudPush(0x07, c);
            gbPlayNextTableSound();
            return;
        }
    } else {
        if (decAndCheck(&g_gameBInitTimer))
            return;
        if (!(g_modeFlags1e & 0x08))
            gbPlayMusic(0x16);
        g_modeFlags1e |= 0x08;
    }
    gbMapSelectedShip();
    if (!(g_modeFlags15 & 0x20))
        gbHudPush(0x0f, g_sceneScriptIndex);
    if ((g_sceneFlagsC & 0x01) || (g_sceneFlagsB & 0x20)) {
        if (!(g_frameCounter & 0x03))
            gbHudPush(0x0c, 0);
        return;
    }
    if (!gbAdvanceScrollScript())
        return;
    g_sceneFlagsB &= ~0x40;
    gbEndScene();
    g_deathMarker = 0xff;
    if (g_modeFlags17 & 0x20)
        return;
    gbBlockMovement();
}

static void state1e(void)
{
    g_sceneFlagsA &= ~0x04;
    g_rasterScx1 = 0x60;
    sceneEndTick();
    rOBP1 = 0x36;
    if (tickDeathMarker()) {
        // The asm compares the old value with $d8.
        if (!(g_sceneFlagsC & 0x01) && !(g_sceneFlagsB & 0x20) && g_deathMarker == 0xd7)
            gbPlayMusic(0x1a);
        return;
    }
    uint8_t a = 0x21;
    if (!(g_sceneFlagsC & 0x01) && !(g_sceneFlagsB & 0x20)) {
        gbSetPalettesD31f(3);
        gbResetAnimPeriod();
        a = 0x1f;
    }
    g_subState = a;
}

static void state1f(void)
{
    sceneEndTick();
    if (gbAnimTick())
        return;
    g_sceneEndPauseTimer = 0x48;
    g_subState++;
}

static void state20(void)
{
    if (!--g_sceneEndPauseTimer) {
        g_subState++;
        return;
    }
    sceneEndTick();
}

// State $21: the verdict.
static void state21(void)
{
    sceneEndTick();
    if (!(g_sceneFlagsC & 0x08))
        VOLATILE(g_topModeFlags) |= 0x02;
    g_sceneFlagsA &= ~0x0c;
    g_sceneFlagsB &= ~0x40;
    gbTallyKillFlags();
    if (!(g_sceneFlagsB & 0x80)) {
        uint8_t *hl = &g_deathTallyA;
        if (!(g_sceneFlagsC & 0x01))
            hl++;
        if (*hl >= 2)
            g_sceneFlagsC |= 0x80;
        (*hl)++;
    }
    g_subStateScratch9 = 0;
    gbUpdateDistanceCounters();
    uint8_t a = g_sceneFlagsC;
    if (a & 0x08) {
        gbSetLevelClearTimer();
        g_sceneScriptIndex = 0;
        bumpStage();
        g_subState = 0x22;
        return;
    }
    if (!(a & 0x01)) {
        setSubStateAndNext(0x23, 0x23);
        return;
    }
    if (!bumpStage())
        g_sceneFlagsC |= 0x40;
    setSubStateAndNext(0x23, 0x25);
}

static void state22(void)
{
    sceneEndTick();
    g_rasterScx3 = 0x60;
    if (!(g_modeFlags16 & 0x80))
        return;
    if (--g_levelClearTimer)
        return;
    VOLATILE(g_topModeFlags) |= 0x02;
    g_gameBLinkState = 7;
    g_subState = 1;
    clearModeFlag16_7();
}

static void state23(void)
{
    sceneEndTick();
    g_subStateScratch4 = 0x0d;
    lcdOnRestoreIE(0x83);
    setSubStateAndNext(0x24, 0x25);
}

static void state25(void)
{
    uint8_t a = g_sceneFlagsC, c = 0x2b;
    g_modeFlags15 &= ~0x20;
    gbMemClear(&g_rasterScx1, 0x0f);
    if (a & 0x40)
        c = 0x27;
    else if (a & 0x01)
        c = 0x07;
    setSubStateAndNext(0x26, c);
}

static void state27(void)
{
    g_subState = (g_sceneFlagsC & 0x41) == 0x01 ? 0x07 : 0x28;
}

static void state28(void)
{
    gbRankCutscene();
    if (g_padState & 0x09)
        g_subState++;
}

static void state29(void)
{
    gbRankCutscene();
    startLevelClear();
    g_subState++;
}

static void state2a(void)
{
    gbRankCutscene();
    if (!tickLevelClear())
        g_subState++;
}

static void state2c(void)
{
    clearModeFlag16_7();
    startLevelClear();
    g_gameBLayoutVariant = (g_sceneFlagsC & 0x04) ? 2 : 4;
    g_subState++;
}

static void state2d(void)
{
    drawGameOverSub();
    drawEndScoreAndSound();
    if (tickDeathMarker() && tickLevelClear())
        return;
    g_gameBLinkState = 4;
    if (!(g_modeFlags16 & 0x80))
        return;
    if (decAndCheck(&g_gameBLayoutVariant))
        return;
    if ((g_sceneFlagsC & 0x04) || (g_killFlagsPending & 0x10))
        gbSoftReset();
    g_subState = 0x2b;
}

// Run the handler of g_subState (bank_000.asm: the table stub).
void gbRunState(void) __banked
{
    switch (g_subState) {
    case 0x00: state00(); break;
    case 0x01: state01(); break;
    case 0x02: state02(); break;
    case 0x03: state03(); break;
    case 0x04: gbState04(); break;
    case 0x05: state05(); break;
    case 0x06: state06(); break;
    case 0x08: state08(); break;
    case 0x09: state09(); break;
    case 0x0a: state0a(); break;
    case 0x0b: state0b(); break;
    case 0x0c: state0c(); break;
    case 0x0d: state0d(); break;
    case 0x0e: state0e(); break;
    case 0x0f: state0f(); break;
    case 0x10: state10(); break;
    case 0x12: state12(); break;
    case 0x13: state13(); break;
    case 0x15: state15(); break;
    case 0x16: state16(); break;
    case 0x17: gbEngine(); break;
    case 0x18: state18(); break;
    case 0x1a: state1a(); break;
    case 0x1b: state1b(); break;
    case 0x1c: state1c(); break;
    case 0x1d: state1d(); break;
    case 0x1e: state1e(); break;
    case 0x1f: state1f(); break;
    case 0x20: state20(); break;
    case 0x21: state21(); break;
    case 0x22: state22(); break;
    case 0x23: state23(); break;
    case 0x24: gbTallyCutscene(); break;
    case 0x25: state25(); break;
    case 0x26: gbState26(); break;
    case 0x27: state27(); break;
    case 0x28: state28(); break;
    case 0x29: state29(); break;
    case 0x2a: state2a(); break;
    case 0x2c: state2c(); break;
    case 0x2d: state2d(); break;
    }
}

// States $2f and $30 are also in the per-frame table GameB_PerFrameJumpTable_d008.
void gbState2f(void) __banked
{
    state2f();
}

void gbState30(void) __banked
{
    state30();
}

// ---- The intro screen (called from bank 3) ----

void gbIntroInit(void) __banked
{
    uint16_t p[2];
    p[0] = 0x9800;
    p[1] = (uint16_t)GameB_StatusPatternTiles + 0x44;
    gbRle(p);
    gbMemClear((uint8_t *)0x9a60, 0x1a0);
    gbDrawTableString(GameB_MessagePtrs, 5);
    gbClearDamageGaugeB();
    gbMemClear(&g_rasterLine, 0x2b);
    farCopy(3, &g_rasterLycTable, GameB_WaveStepSizes + 10, 0x0b);
    g_sideSpeed = 3;
    g_rasterCount = 0x0a;
    g_d0dbCounter = 0x8f;
    ScrollScript_LastTileByte = 0;
    g_gameBScenePhase = 0;
    g_gameClock = 0xb4;
    farCopy(3, &g_gameBLayoutVariant, GameB_RasterTables + 0xd4, 0x0a);
    g_modeFlags10 &= ~0x11;
    g_sceneFlagsA &= ~0x01;
}

// When $d094 = 1, count $d098 toward the target in the bank 3 table,
// then draw the count.
static void stepScoreCount(void)
{
    if (g_introFadeHi != 1)
        return;
    if (g_modeFlags1e & 0x10)
        return;
    if (!(g_modeFlags1e & 0x08))
        gbPlaySFX(0x1c);
    g_modeFlags1e |= 0x08;
    const uint8_t *t = GameB_RasterTables + 0xde + (uint8_t)(g_gameBAnimSpriteTable << 1);
    if (farByte(3, t) < g_sixAnimCounter0)
        AT(g_sixAnimCounter0, -1)++;
    else
        g_sixAnimCounter0++;
    uint8_t c = farByte(3, t + 1);
    if (c == 0xff) {
        g_modeFlags1e |= 0x10;
        return;
    }
    gbMeta00((uint16_t)g_sixAnimCounter1-- << 8 | 0x28, c);
}

static void introFade(void)
{
    if (!g_rankIndex)
        stepScoreCount();
    uint8_t e = g_gameBScenePhase, d = e - 0xa0;
    if (g_frameCounter & 0x03)
        return;
    if (!++g_gameBScenePhase)
        g_introFadeHi++;
    if (g_introFadeHi < 2 || d < 0xc0) {
        g_rasterScx2 = d;
        if (!(e & 0x07))
            gbHudPush(0x10, e >> 3);
        return;
    }
    g_rasterScx2 = 0;
    g_rasterScy2 = 0x70;
    g_cutsceneFlags |= 0x08;
    g_gameBLayoutVariant++;
}

void gbIntroUpdate(void) __banked
{
    g_metaspriteOamSlot = 0x19;
    if (!(g_frameCounter & 0x03))
        gbAnimateWaves(3);
    switch (g_gameBLayoutVariant) {
    case 0:
        g_rasterLyc2 = 0x77;
        rBGP = 0x78;
        g_gameBLayoutVariant++;
        break;
    case 1:
        introFade();
        break;
    case 2:
        if ((g_frameCounter & 0x01) && !--g_gameClock)
            goto next;
        if (!(g_joyEdge & 0x09))
            break;
    next:
        g_subStateScratch4 = 0x0d;
        GameB_ResetScroll();
        g_subState++;
        break;
    }
    gbClearSpritesFrom(g_metaspriteOamSlot);
}
