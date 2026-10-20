// Game B frame code: the palette fade, the waves, the MG trails, the object
// sprites, the damage gauge, the sub approach and small helpers.
// They read bank 3 tables, so they stay in bank 3.
#include "game.h"
#include "bank3.h"

// Asm routines. They keep all registers.
void PlaySFX(uint8_t id);
void PlaySound_DEB0(uint8_t id);
void PlaySound_DEB8(uint8_t id);
void PlaySound_DEC8(uint8_t id);
void hudQueuePushFar(uint8_t cmd, uint8_t param) __banked;
void linkTxQueuePushFar(uint8_t cmd, uint8_t param) __banked;
void mgTrailHitFar(uint8_t y, uint8_t side) __banked;
void hudToggleWindow(void);
void enemySubStartBurst(uint8_t flags);
uint8_t mirrorOffset(uint8_t d, uint8_t b);

extern const uint8_t GameB_WaveStepSizes[], GameB_DamageGaugeSteps[], GameB_DamageStepTable[];
extern const uint8_t GameB_ObjSpriteTiles[], GameB_ObjBaseYTable[], GameB_RasterTables[];
extern const uint8_t GameB_MgTrailStepsA[], GameB_MgTrailStepsB[];
extern const uint8_t *const GameB_DepthYOffsetPtrs[];

#define ROUND_OVER (g_sceneFlagsC & 0x02)
#define SHIPS_LOST (&g_playerShipsRemaining)[1]
#define GAUGE_LIMIT (&g_damageGaugeStep)[1]
#define PLAYER_X (&g_shipArray_Player + 2)
#define ENEMY_X WORD((&g_shipArray_Enemy)[2])

// The only rDIV read in this file. compare.py pins it (C_RNG).
uint8_t gameBReadDiv(void)
{
    return rDIV;
}

// ------------------------------------------------------------------
// Palette fade and waves.
// ------------------------------------------------------------------

// Steps the frame timer. Every 8 frames (10 with g_animFlags bit 7) it
// steps the fade one level, 4 steps in all. Returns 0 when the fade ends.
uint8_t gameBAnimTick(void)
{
    if (++g_animFrameTimer < ((g_animFlags & 0x80) ? 10 : 8))
        return 1;
    g_animFrameTimer = 0;
    uint8_t d = ++g_animFlags;
    if ((d & 7) >= 4) {
        g_animFlags = 0;
        g_animFrameTimer = 0;
        return 0;
    }
    volatile uint8_t *r = &rBGP;
    if (d & 0x80) {
        // Fade down BGP and OBP0: each color field steps one shade toward 0.
        for (uint8_t k = 2; k; k--, r++) {
            uint8_t v = *r, c = 0;
            for (uint8_t f = 4; f; f--, v >>= 2) {
                uint8_t a = v & 3;
                if (a)
                    a--;
                c = (c >> 2) | (a << 6);
            }
            *r = c;
        }
        return 1;
    }
    // Fade up toward g_paletteShadow. The asm clears the result byte once,
    // so each register also gets the bits of the registers before it.
    uint8_t n = 3, acc = 0;
    if (g_subState == 0x1f && !(g_sceneFlagsC & 1))
        n = 2;
    const uint8_t *t = g_paletteShadow;
    do {
        for (uint8_t mask = 3; mask; mask <<= 2) {
            uint8_t cur = *r & mask, tgt = *t & mask;
            acc |= cur >= tgt ? tgt : cur + (mask & 0x55);
        }
        *r++ = acc;
        t++;
    } while (--n);
    g_gameBLayoutVariant = acc;
    return 1;
}

// Steps n wave phases by twice the side speed. With speed 0, rDIV bit 0
// adds 1. Each phase wraps at its step size and moves its raster SCX.
void gameBAnimateWaves(uint8_t n)
{
    uint8_t b = g_sideSpeed << 1;
    const uint8_t *step = GameB_WaveStepSizes;
    uint8_t *ph = &g_wavePhases;
    do {
        uint8_t a = b;
        if (!b && (gameBReadDiv() & 1))
            a = 1;
        a += *ph;
        uint8_t q = 0;
        while (a >= *step) {
            a -= *step;
            q++;
        }
        *ph++ = a;
        step++;
        uint8_t *x = &g_rasterScx2 + n;
        if (g_subState != 0x26)
            x++;
        if (g_modeFlags10 & 1)
            *x += q;
        else
            *x -= q;
    } while (--n);
}

// ------------------------------------------------------------------
// Sprite buffers.
// ------------------------------------------------------------------

// Writes 0 to the Y byte of n sprite entries at p. n = 0 clears 256.
void clearSpriteEntries(uint8_t *p, uint8_t n)
{
    do {
        *p = 0;
        p += 4;
    } while (--n);
}

// Clears both sprite buffers from entry a to entry 40.
void clearSpritesFrom(uint8_t a)
{
    uint8_t off = a << 2, n = 0x28 - a;
    clearSpriteEntries(&g_oamBuffer + off, n);
    clearSpriteEntries(&g_oamBufferB + off, n);
}

// Clears the unused tail of each sprite buffer, then moves the slot
// cursors to the sprite cursors.
void gameBClearSpriteTails(void)
{
    uint8_t a = ScrollScript_SlotA_Cursor, b = ScrollScript_SlotB_Cursor;
    if (g_spriteCursorA >= a)
        clearSpriteEntries(&g_oamBuffer + (uint8_t)(a << 2), g_spriteCursorA - a + 1);
    if (g_spriteCursorB >= b)
        clearSpriteEntries(&g_oamBufferB + (uint8_t)(b << 2), g_spriteCursorB - b + 1);
    g_spriteCursorA = a;
    ScrollScript_SlotA_Cursor = 0;
    g_spriteCursorB = b;
    ScrollScript_SlotB_Cursor = 0;
}

// Reserves n sprite entries. The buffer alternates each call. The $c000
// buffer gives way to $c100 when it has no room.
uint8_t *oamAllocSprites(uint8_t n)
{
    uint8_t *cur = &ScrollScript_SlotA_Cursor, *buf = &g_oamBuffer;
    if (((g_oamBufToggle ^= 1) & 1) || (uint8_t)(n + *cur) >= 0x28) {
        cur++;
        buf = &g_oamBufferB;
    }
    uint8_t old = *cur;
    *cur = old + n;
    return buf + (uint8_t)(old << 2);
}

void gameBResetScrollCursors(void)
{
    ScrollScript_SlotA_Cursor = 0;
    ScrollScript_SlotB_Cursor = 0;
    g_metaspriteOamSlot = 0;
    g_oamBufToggle &= ~1;
}

// ------------------------------------------------------------------
// MG trails: 8 slots of 4 bytes per side (flags, steps left, Y, X).
// ------------------------------------------------------------------

// y - GameB_DepthYOffsetPtrs[class][ships / 4], or $b0 past the table.
uint8_t adjustYByDepth(uint8_t cls, uint8_t y)
{
    uint8_t i = g_playerShipsRemaining >> 2;
    if (i >= 0x18)
        return 0xb0;
    return y - GameB_DepthYOffsetPtrs[(cls >> 4) & 7][i];
}

// Starts the burst cooldown of a side. Returns the side, or 1 after
// side 0 sends its link message (the asm leaves c = 1 there).
static uint8_t mgFireGate(uint8_t side)
{
    uint8_t *cd = &g_mgBurstCooldownA + side;
    if (ROUND_OVER || ((&g_modeFlags10)[side] & 0x50) || *cd)
        return side;
    *cd = 8;
    if (side)
        return side;
    linkTxQueuePushFar(0x82, 1);
    return 1;
}

// While the burst cooldown runs, adds one trail particle every 2 frames.
static void mgTrailEmit(uint8_t side)
{
    if ((&g_modeFlags10)[side] & 0x50)
        return;
    uint8_t *t = &g_mgTrailEmitTimer[side];
    if (*t && --*t)
        return;
    *t = 2;
    uint8_t *cd = &g_mgBurstCooldownA + side;
    if (!*cd)
        return;
    (*cd)--;
    uint8_t ph = (&g_mgTrailPhase)[side] ^= 8;
    uint8_t *p = &g_mgTrailA;
    if (side) {
        if (g_enemySubApproachShadow == 0xd0)
            return;
        p = &g_mgTrailB;
    }
    for (uint8_t k = 8; p[0] & 0x80; p += 4)
        if (!--k)
            return;
    p[0] = ph | 0x80;
    p[1] = 0x13;
    if (side) {
        p[2] = adjustYByDepth(0, 0x54);
        PlaySound_DEC8(1);
        p[3] = g_enemySubApproachShadow - 8;
    } else {
        PlaySound_DEC8(1);
        if (g_playerShipsRemaining >= 5)
            return;
        p[2] = adjustYByDepth(0, 0x7a);
        p[3] = (ph & 8) ? 0xa2 : 0x02;
    }
}

// Handles the fire edges (g_weaponEdges bits 1 and 5) and emits the trails.
void gameBUpdateMG(void)
{
    if (ROUND_OVER)
        return;
    uint8_t side = 0;
    if (g_weaponEdges & 0x02) {
        g_weaponEdges &= ~0x02;
        side = mgFireGate(0);
    }
    if (!g_subStateScratch8)
        mgTrailEmit(side);
    if (g_weaponEdges & 0x20) {
        g_weaponEdges &= ~0x20;
        mgFireGate(1);
    }
    mgTrailEmit(1);
}

// Moves the first 6 trail particles of a side and draws each one twice.
void gameBMoveMgTrails(uint8_t side)
{
    if (ROUND_OVER)
        return;
    uint8_t *p = side ? &g_mgTrailB : &g_mgTrailA;
    const uint8_t *steps = side ? GameB_MgTrailStepsB : GameB_MgTrailStepsA;
    for (uint8_t k = 6; k; k--, p += 4) {
        if (!(p[0] & 0x80))
            continue;
        uint8_t n = --p[1];
        if (!n)
            goto end;
        const uint8_t *s = steps + (uint8_t)((n - 1) << 1);
        uint8_t dx = s[0], packed = s[1], dy = packed & 0x0f, y;
        g_gameBLayoutVariant = packed;
        if (p[0] & 8)
            dx = -dx;
        if (side) {
            y = p[2] + dy + ((g_frameCounter >> 4) & 3);
            if (y >= 0x78)
                goto end;
        } else {
            y = p[2] - dy;
        }
        p[2] = y;
        uint8_t x = p[3] += dx;
        uint8_t *o = oamAllocSprites(1);
        uint8_t j = gameBReadDiv() & 3;
        o[0] = j + y;
        o[1] = j + x;
        o[2] = (packed >> 4) + 0xfb;
        o[3] = j << 5;
        uint8_t *o2 = oamAllocSprites(1);
        o2[0] = o[0];
        o2[1] = o[1];
        o2[2] = o[2];
        o2[3] = o[3];
        continue;
end:
        p[0] = 0;
        uint8_t *ph = &g_mgTrailPhase + side;
        if (*ph & 7)
            (*ph)--;
        mgTrailHitFar(p[3], side);
    }
}

// ------------------------------------------------------------------
// Object sprites: 8 slots of 8 bytes. Byte 0 = flags, 1 = class,
// 2-3 = world X, 4 = state, 5 = count, 6 = sprite base.
// ------------------------------------------------------------------

// Writes one sprite at entry g_firstFreeSprite + $d31a - 1 of the $c000 buffer.
static void writeObjSprite(uint8_t c, uint8_t x, uint8_t y)
{
    uint8_t *o = &g_oamBuffer + (uint8_t)((uint8_t)(g_firstFreeSprite + (&g_firstFreeSprite)[1] - 1) << 2);
    uint8_t grow = g_tallyIconGrow, last = ScrollScript_LastTileByte;
    if (!(grow & 0x70))
        c += 0x0c;
    else if (last & 0xc0)
        c += (last & 0x80) ? 8 : 4;
    uint8_t tile = GameB_ObjSpriteTiles[c];
    o[0] = y;
    o[1] = x;
    if (g_subState < 0x13 && (g_gameBScenePhase & 2))
        tile = 0;
    if ((grow & 0x0f) == 9) {
        if (tile < 0x7d)
            tile = (g_frameCounter & 8) ? 0x6a : 0x69;
    } else if (!(g_killFlagsPending & 0x80) && tile == 0x79) {
        tile++;
    }
    o[2] = tile;
    uint8_t attr = (grow & 0x80) ? 0x30 : 0x10;
    if (!(g_difficultyColumn & 0x20))
        attr ^= 0x20;
    o[3] = attr;
}

// Draws the 8 slots at s. A slot in state $40/$80 counts down byte 5. At 0
// it ends the state. State $80 then frees the slot.
static void drawObjectSlots(uint8_t *s)
{
    for (uint8_t k = 8; k; k--, s += 8) {
        if (!(s[0] & 0x80))
            continue;
        g_difficultyColumn = s[0];
        g_tallyIconGrow = s[1];
        if (s[4] & 0xc0) {
            if (s[5] & 0x0f) {
                s[5]--;
            } else {
                s[4] &= ~0x40;
                if (s[4] & 0x80) {
                    s[0] &= ~0x80;
                    g_modeFlags1a &= ~0x08;
                    continue;
                }
            }
        } else if ((s[1] & 0x0f) >= 0x0a) {
            continue;
        }
        uint8_t x = mirrorOffset(relativeX(WORD(s[2]), PLAYER_X) + 0x20, 4);
        uint8_t y = GameB_ObjBaseYTable[s[1] >> 4];
        uint8_t t = ScrollScript_LastTileByte = s[4];
        uint8_t c = (t & 0xc0) ? (s[5] & 0x0c) >> 2 : 3;
        g_firstFreeSprite++;
        writeObjSprite(c, x, y);
    }
}

void gameBDrawObjects(uint8_t *s)
{
    g_spriteDrawMode = 0;
    drawObjectSlots(s);
}

void gameBDrawEnemySlotsD258(void)
{
    gameBDrawObjects(&g_shipArray_Enemy + 0x40);
    clearSpritesFrom(g_firstFreeSprite);
}

void gameBDrawPlayerSlots(void)
{
    g_firstFreeSprite = g_subState < 0x10 ? 5 : 1;
    gameBDrawObjects(&g_shipArray_Player);
}

// ------------------------------------------------------------------
// Player ship loss and the damage gauge.
// ------------------------------------------------------------------

void gameBResetPlayerShip(void)
{
    g_modeFlags16 &= ~2;
    g_playerShipsRemaining = 0x86;
    SHIPS_LOST = 0;
    g_damageGaugeLevel = 1;
    g_damageGaugeStep = 0;
    GAUGE_LIMIT = 7;
    g_d0dbCounter = 0x77;
}

void gameBClearDamageGauge(void)
{
    fillBytes(g_damageGaugeRows, 0, 13);
    fillBytes(&g_rasterScy2, 0, 12);
}

static void setScratch8(uint8_t a)
{
    g_subStateScratch8 = a;
    g_modeFlags10 &= ~0x40;
}

// No ships left: show the window and pick the end sub state.
static void lastShipDestroyed(void)
{
    rWY = 0x70;
    uint8_t s = g_subState;
    if (s == 0x11) {
        g_subState = 0x12;
        g_deathMarker = 0x78;
    } else if (s == 0x17 || s == 0x19) {
        g_subState = 0x1a;
    }
    (&g_rasterLycTable)[2] = 0x17;
    gameBClearDamageGauge();
    g_rasterScy1 = 0;
    g_rasterScx1 = 0x60;
    setScratch8(0);
}

// Spreads the gauge rows over the raster SCY table.
static void damageGaugeShift(void)
{
    uint8_t n = g_rasterCount;
    if (n < 3)
        return;
    n -= 2;
    uint8_t *row = g_damageGaugeRows + 2;
    const uint8_t *step = GameB_DamageGaugeSteps;
    uint8_t k = n;
    do {
        uint8_t a = *row + 0x10, q = 0;
        while (a >= *step) {
            a -= *step;
            q++;
        }
        *row++ = a;
        step++;
        uint8_t *scy = &g_rasterScyTable + 4 + (uint8_t)(n - k);
        if (g_modeFlags16 & 2)
            *scy += q;
        else
            *scy -= q;
    } while (--k);
}

// Stores the raster line of the gauge level. Returns it.
static uint8_t damageGaugeEntry(void)
{
    uint8_t lvl = g_damageGaugeLevel;
    g_rasterCount = lvl;
    return (&g_rasterLine)[lvl] = GameB_RasterTables[lvl] + g_damageGaugeStep;
}

static void updateGauge(void)
{
    damageGaugeShift();
    uint8_t n = g_playerShipsRemaining;
    if (n >= 0x78) {
        g_rasterScy2 = 0x88 - (uint8_t)(0x86 - n);
        return;
    }
    if (g_modeFlags16 & 2) {
        if (!--g_damageGaugeStep) {
            if (!--g_damageGaugeLevel)
                return;
            g_damageGaugeStep = GAUGE_LIMIT = GameB_DamageStepTable[g_damageGaugeLevel];
        }
    } else if (++g_damageGaugeStep >= GAUGE_LIMIT) {
        if (++g_damageGaugeLevel >= 0x0e) {
            lastShipDestroyed();
            return;
        }
        g_damageGaugeStep = 0;
        GAUGE_LIMIT = GameB_DamageStepTable[g_damageGaugeLevel];
    }
    uint8_t e = damageGaugeEntry();
    (&g_rasterScy1)[g_damageGaugeLevel] = 0x78 - e;
}

// Player ship lost. With g_modeFlags16 bit 1 set, the count runs back up.
void gameBPlayerHit(void)
{
    if (!(g_modeFlags16 & 2)) {
        SHIPS_LOST++;
        uint8_t n = --g_playerShipsRemaining;
        if (!n) {
            lastShipDestroyed();
            return;
        }
        if (n == 0x77) {
            g_damageGaugeLevel++;
            g_damageGaugeStep = 0;
            GAUGE_LIMIT = 7;
            g_rasterScy2 = 0;
        } else if (n == 0x71) {
            g_rasterScy3 = 0;
        }
        updateGauge();
        return;
    }
    g_playerShipsRemaining++;
    if (--SHIPS_LOST) {
        updateGauge();
        return;
    }
    if (!(g_padState & 0x40)) {
        setScratch8(2);
        return;
    }
    g_modeFlags10 |= 0x40;
    gameBResetPlayerShip();
    g_subStateScratch8 = 1;
}

void gameBBlockMovement(void)
{
    if (g_modeFlags10 & 0x40)
        return;
    uint8_t a = 1;
    if ((g_modeFlags10 |= 0x40) & 0x10) {
        gameBResetPlayerShip();
    } else {
        g_modeFlags16 |= 2;
        g_playerShipsRemaining = 0;
        SHIPS_LOST = 0x86;
        g_damageGaugeLevel = 0x0d;
        g_damageGaugeStep = 0x17;
        GAUGE_LIMIT = 0x17;
        g_rasterScy1 = 0x70;
        hudToggleWindow();
        linkTxQueuePushFar(0x8c, 3);
        PlaySound_DEB8(9);
        a = 3;
    }
    g_subStateScratch8 = a;
}

// Blocks movement when the pad bit is set ($80, or $40 with g_modeFlags10 bit 4).
void gameBState2eGate(void)
{
    if (ROUND_OVER || (g_modeFlags10 & 0x40))
        return;
    if (g_padState & ((g_modeFlags10 & 0x10) ? 0x40 : 0x80))
        gameBBlockMovement();
}

void gameBStepDelay(void)
{
    if (g_stepDelay) {
        g_stepDelay--;
        return;
    }
    uint8_t a = 0;
    if (g_sideSpeed) {
        if (!ROUND_OVER)
            PlaySound_DEB8(9);
        a = 9;
    }
    g_stepDelay = 0x29 - a;
}

// ------------------------------------------------------------------
// Counters, pause and sounds.
// ------------------------------------------------------------------

void gameBUpdateDistanceCounters(void)
{
    uint8_t *s = &g_playStats;
    if (g_sceneFlagsC & 1)
        s[0] += g_hudCounterB;
    s[2] += 0x0f - ((&g_shipArray_Player)[4] & 0x3f);
    uint8_t a = (&g_gameClock)[1] + s[3], carry = 0;
    if (a >= 0x3c) {
        a -= 0x3c;
        carry = 1;
    }
    s[3] = a;
    a = (&g_gameClock)[2] + s[4] + carry;
    s[4] = a >= 0x63 ? 0x63 : a;
}

void gameBTogglePause(void)
{
    if (ROUND_OVER)
        return;
    g_subState = 0x15;
    uint8_t a = g_modeFlags1a ^= 1;
    if (a & 1) {
        PlaySound_DEB0(1);
        PlaySFX(0x1b);
        if (!(a & 2))
            linkTxQueuePushFar(0x85, 5);
        g_modeFlags1a &= ~2;
        clearSpritesFrom(0);
    } else {
        if (!(a & 2))
            linkTxQueuePushFar(0x84, 5);
        PlaySound_DEB0(2);
        g_modeFlags1a &= ~2;
        g_stepCountA = 0;
    }
}

// Returns 1 (carry) while g_stepCountA < limit. Then steps g_stepCountA
// once every period + 1 calls.
uint8_t gameBStepCounters(uint8_t limit, uint8_t period)
{
    if (g_stepCountA >= limit)
        return 0;
    uint8_t *b = &g_stepCountA + 1;
    if ((*b)++ >= period) {
        *b = 0;
        g_stepCountA++;
    }
    return 1;
}

static const uint8_t soundSequence[] = {1, 1, 1, 1, 4, 3, 1, 1, 1, 1, 4, 3};

// While g_soundStep < 3, plays the next sound of the 12-entry sequence.
void gameBPlayNextTableSound(void)
{
    uint8_t id = 1;
    if (g_soundStep < 3) {
        uint8_t i = g_soundStepIdx++;
        if (i >= 12)
            i = g_soundStepIdx = 0;
        id = soundSequence[i];
    }
    PlaySound_DEB8(id);
}

void gameBMaybeAddItem(void)
{
    if (g_soundFrameCount || ((&g_soundFrameCount)[1] & 1))
        return;
    uint8_t a = g_gameBItemCount;
    if (a >= 4)
        return;
    g_gameBItemCount = a + 1;
    if (!(g_sceneFlagsC & 4))
        a += 4;
    hudQueuePushFar(1, a);
}

void gameBStartLevelClear(void)
{
    g_levelClearTimer = 3;
    g_deathMarker = 0xff;
    PlaySound_DEB0(5);
}

// Counts bits 5-7 of g_killFlagsPending into g_killCount, then clears them.
void gameBTallyKillFlags(void)
{
    uint8_t f = g_killFlagsPending, n = 0;
    for (uint8_t m = 0x80; m != 0x10; m >>= 1)
        if (f & m)
            n++;
    g_killFlagsPending = f & 0x1f;
    g_killCount += n;
}

// ------------------------------------------------------------------
// The enemy sub approach.
// ------------------------------------------------------------------

void gameBResetSubApproach(void)
{
    g_enemySubApproachShadow = 0xd0;
    g_killFlagsPending &= ~1;
}

void gameBSetSubApproach(uint8_t a)
{
    g_enemySubApproachShadow = a + 0x1c;
    g_killFlagsPending |= 1;
}

// The warning sound, when the sub is close or the player is dead.
void gameBSub5127(void)
{
    if (!(g_modeFlags10 & 0x10) && g_enemySubApproachShadow >= 0xaa)
        return;
    if (!ROUND_OVER)
        PlaySFX(0x0c);
}

void gameBSub5111(void)
{
    if (!(g_killFlagsPending & 1)) {
        g_modeFlags1b &= ~0x80;
        return;
    }
    if (g_modeFlags1b & 0x80)
        return;
    g_modeFlags1b |= 0x80;
    gameBSub5127();
}

// Starts the approach when the enemy ship is within $a0 of the player.
void gameBCheckSubRange(void)
{
    if (ROUND_OVER)
        return;
    gameBResetSubApproach();
    g_spriteDrawMode = 0x36;
    uint16_t dist;
    if (rangeCheck(ENEMY_X, 0xa0, &dist, WORD(*PLAYER_X), 0x36)) {
        g_modeFlags1b &= ~0x80;
        return;
    }
    gameBSetSubApproach(divide16(dist, 2));
    gameBSub5111();
}

// Resets the enemy sub for a new approach.
void gameBResetEnemySub(void)
{
    if (g_enemySubFlags & 0x10)
        enemySubStartBurst(g_enemySubFlags & ~0x10);
    g_modeFlags15 &= ~0x20;
    g_modeFlags1d |= 0x40;
    g_modeFlags1e &= ~1;
    g_bannerVramPage = 0;
    (&g_bannerVramPage)[1] = 0x0b;
    ScrollScript_Pad = 0;
    g_enemySubResetByte = 0;
    uint8_t a = g_enemySubApproachShadow;
    if (a != 0xd0) {
        a -= 0x1c;
        if (a < 0xd0 && a >= 0x3c)
            g_modeFlags1e |= 0x80;
        else
            g_modeFlags1e &= ~0x80;
    } else {
        g_spriteDrawMode = 0;
        uint16_t dist;
        rangeCheck(ENEMY_X, WORD(g_worldWidth) >> 1, &dist, WORD(*PLAYER_X), 0);
        if ((uint8_t)(dist >> 8) >= (g_worldWidthHi >> 1)) {
            g_modeFlags1e |= 0x80;
            a = 0xc4;
        } else {
            g_modeFlags1e &= ~0x80;
            a = 0xd0;
        }
    }
    g_enemySubApproachShadow = a;
    g_enemySubApproachCounter = a;
    g_lockOnTargetIdx = 0;
    (&g_weaponFlagsBase)[1] = 8;
    (&g_weaponFlagsBase)[2] = 8;
    g_subAiLevel = 7;
}

// ------------------------------------------------------------------
// Small helpers.
// ------------------------------------------------------------------

// Picks the level value 2..5 from the scene flags.
void gameBPickLevelValue(void)
{
    uint8_t c = 5;
    if (!(g_sceneFlagsB & 0x80)) {
        uint8_t a = g_sceneFlagsC;
        c = (a & 0x80) ? 4 : 2;
        if (!(a & 1))
            c--;
    }
    g_gameBLevel = c;
}

// Three bytes per entry: BGP, OBP0, OBP1.
static const uint8_t paletteTable[] = {
    0x36, 0x36, 0x36,
    0x36, 0x39, 0x39,
    0x00, 0x00, 0x00,
    0xff, 0xff, 0xff,
    0x36, 0x39, 0xb1,
    0x36, 0x36, 0x76,
};

void gameBSetPalettes(uint8_t set)
{
    const uint8_t *p = paletteTable + (uint8_t)((set << 1) + set);
    rBGP = p[0];
    rOBP0 = p[1];
    rOBP1 = p[2];
}

void gameBSetPaletteShadow(uint8_t set)
{
    copyBytes(g_paletteShadow, paletteTable + (uint8_t)((set << 1) + set), 3);
}

// Clears bit 6 of byte 4 and bits 0-4 of byte 5 in 32 slots.
// Returns the last byte 5.
uint8_t gameBSub52ab(void)
{
    uint8_t *p = &g_shipArray_Player + 4, a;
    uint8_t c = 0x20;
    do {
        p[0] &= ~0x40;
        a = p[1] &= 0xe0;
        p += 8;
    } while (--c);
    return a;
}

void gameBClearFireFlags(void)
{
    g_modeFlags17 &= ~4;
    g_modeFlags1b &= ~0x80;
    g_killFlagsPending &= ~1;
}

void gameBSetScrollY5(void)
{
    rSCY = 5;
    rSCX = 0;
}

void gameBSetD0b2To60(void)
{
    g_rasterScx3 = 0x60;
}

void gameBLinkSetState7(void)
{
    g_gameBLinkState = 7;
}

void gameBClearModeFlag16_7(void)
{
    g_modeFlags16 &= ~0x80;
}
