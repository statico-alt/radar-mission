// Game A board helpers and simple state steps that read no bank 1 data.
#include "bank1.h"

// Set the Y and X of the 4 OAM entries at oam as a 2x2 metasprite.
// size is the size of one part in pixels.
void setMetaspritePos2x2(uint8_t *oam, uint8_t x, uint8_t y, uint8_t size) __banked
{
    oam[0] = oam[4] = y;
    oam[8] = oam[12] = y + size;
    oam[1] = oam[9] = x;
    oam[5] = oam[13] = x + size;
}

// The first cell of grid whose low 6 bits are a or b.
// Returns row << 8 | column.
uint16_t findGridCellByValue(const uint8_t *grid, uint8_t a, uint8_t b) __banked
{
    uint8_t row = 0, col = 0;
    for (;;) {
        uint8_t v = *grid++ & 0x3f;
        if (v == a || v == b)
            return (row << 8) | col;
        if (++col == g_gridSize) {
            row++;
            col = 0;
        }
    }
}

// 4 + (grid size - 8) / 2: the first tile column and row of the grid on
// the screen.
uint8_t getGridMargin(void) __banked
{
    return ((g_gridSize - 8) >> 1) + 4;
}

// Start the game over once ($ffaf bit 6): the STAT effect, state $0b
// and sound $14. Returns 1 when it started now, 0 when it ran before.
uint8_t beginGameOver(void) __banked
{
    if (g_gameFlags8 & 0x40)
        return 0;
    g_gameFlags8 |= 0x40;
    g_gameALevel |= 0x02;
    rLYC = 0;
    g_statEffect1Arg = 1;
    g_gameAStatEffect = 1;
    g_statEffect2Arg = 3;
    g_statEffectTimer = 0x0f;
    g_gameAState = 0x0b;
    SFX_Queue = 0x14;
    g_gameFlags2 |= 0x04;
    return 1;
}

// Clear the BG map ($9800, or $9c00 with $ff8a bit 0) with tile 1 and
// start a fade in ($ffaa bit 0). Go to Game A state 7 ($10 in link
// mode). Restore the ship hit counts saved at the end of the last battle.
void initBoardScreen(void) __banked
{
    uint8_t *p = (uint8_t *)((g_topModeFlags & 0x01) ? 0x9c00 : 0x9800);
    for (uint16_t n = 0x240; n; n--)
        *p++ = 1;
    g_gameFlags2 |= 0x01;
    g_gameATurnTimer = 0;
    g_listCount = 0;
    g_gameAState = (g_topModeFlags & 0x80) ? 0x10 : 0x07;
    ClearShadowOAM();
    // 6 bytes: the 5 hit counts and $ff97.
    for (uint8_t i = 0; i < 6; i++)
        g_shipHitsLeft[i] = g_savedShipHitsLeft[i];
    g_fadeTargetObp1 = 0x36;
}

// Game A state $1a step 5: when the VRAM copies are done, go to state 8.
void state1AStep5(void) __banked
{
    if (g_vramCopyCount)
        return;
    g_gameATurnTimer = 0;
    g_gameAState = 0x08;
    g_gameFlags6 &= ~0x08;
    g_gameFlags5 |= 0x10;
    g_gameADelay = 0x20;
    g_gameALevel |= 0x02;
}

// Game A state $15 step 2: wait $ffb1 frames, then go to step 0 and
// start a fade ($ffaa bit 1).
void state15Step2(void) __banked
{
    if (--g_gameATurnTimer)
        return;
    g_gameAState15Step = 0;
    g_gameFlags2 |= 0x02;
}

// Board step 2: the end-of-level flash. Every 8 frames it moves the
// window and flips the palettes. After $ffe4 flashes it goes to state
// $17 and adds the turns to the total. The 8 sprites at $c000 blink
// every $20 frames.
void boardStep2(void) __banked
{
    uint8_t t = --g_gameAFxTimer;
    if (!(t & 0x07))
        rWX ^= 0x0e;
    if (!t) {
        g_gameAFxTimer = 8;
        rWY++;
        rBGP ^= 0xf0;
        if (g_gameALevel & 0xc0)
            rOBP0 ^= 0xf0;
        NoiseSfx_Queue = 0x06;
        if (!--g_gameAFxSteps) {
            uint8_t *s = OAM;
            for (uint8_t n = 8; n; n--, s += 4)
                *s = 0x30;
            g_gameFlags2 |= 0x02;
            g_gameAState = 0x17;
            g_gameATotalTurns += g_turnCount;
            return;
        }
    }
    if (--g_gameATurnTimer)
        return;
    g_gameATurnTimer = 0x20;
    uint8_t *s = OAM;
    for (uint8_t n = 8; n; n--, s += 4)
        *s ^= 0x30;
    if (*OAM == 0x30)
        SFX_Queue = 0x0b;
}

// Game A state $20: go to state $1f and start a fast fade out.
void state20FadeOut(void) __banked
{
    g_gameAState = 0x1f;
    g_gameABlinkStep = 0;
    g_gameABlinkTimer = 1;
    SoundCmd_Queue = 0x05;
    g_soundFadeState = 0x05;
}

// Add a copy to the VRAM queue ($c4c0, 7 bytes per entry): src, dst,
// length, repeat count and ROM bank. Count it in $c4bf.
void queueVramCopy(const uint8_t *src, uint16_t dst, uint8_t len, uint8_t rows, uint8_t bank) __banked
{
    uint8_t *e = &g_vramCopyQueue + (uint8_t)((g_vramCopyCount << 3) - g_vramCopyCount);
    *(const uint8_t **)e = src;
    *(uint16_t *)(e + 2) = dst;
    e[4] = len;
    e[5] = rows;
    e[6] = bank;
    g_vramCopyCount++;
}
