// Game A state steps that read no bank 1 data: the end of a CPU turn,
// state $10 and the window slide of state $1d.
#include "bank1.h"

// Bank 2 and 5 graphics (only the address is used).
extern const uint8_t GameA_GfxTiles7330[];
extern const uint8_t NextLevelBgTiles[];

// Game A state $10: draw to map $9c00 ($ffad bit 6) and go to state 8.
// The linked Game Boy with $ff8a bit 0 goes to state $11.
void state10(void) __banked
{
    g_gameFlags5 |= 0x40;
    g_gameAState = 0x08;
    if ((g_topModeFlags & 0x81) == 0x81)
        g_gameAState = 0x11;
}

// CPU step $0b: the end of the CPU turn. When the VRAM copies are done,
// clear the salvo data and go to state 8. With $ffae bit 3 the level is
// won: go to state $1a (the level map) and load its tiles. When no ship
// of the player is left (and no reveal), go to state $12: the player lost.
void cpu0B(void) __banked
{
    if (g_vramCopyCount)
        return;
    g_gameADelay = 0x2f;
    // $24 bytes from $c97a.
    uint8_t *p = &g_salvoCount;
    for (uint8_t n = 0x24; n; n--)
        *p++ = 0;
    g_gameFlags5 |= 0x40;
    g_gameAState = 0x08;
    g_gameATurnState = 0;
    g_gameATurnTimer = 0;
    if (g_gameFlags6 & 0x08) {
        g_gameAState = 0x1a;
        g_gameADelay = 0x5e;
        g_gameFlags2 |= 0x02;
        g_p2Options &= ~0x80;
        g_levelMapStep = 0;
        g_levelMapRows = 0x11;
        // Level 3 has its own tiles in bank 5.
        if ((g_gameALevel & 0xc0) == 0x80) {
            queueVramCopy(NextLevelBgTiles + 0x8f0, 0x94e0, 0x20, 0x19, 5);
            queueVramCopy(NextLevelBgTiles + 0xa10, 0x8940, 0x20, 0x0a, 5);
        } else {
            queueVramCopy(GameA_GfxTiles7330, 0x94e0, 0x20, 0x19, 2);
            queueVramCopy(GameA_GfxTiles7330 + 0x320, 0x8800, 0x20, 0x0a, 2);
        }
        queueVramCopy(GameA_GfxTiles6050, 0x8030, 0x20, 0x24, 2);
    }
    if (countNonZero5(g_shipHitsLeft))
        return;
    if (g_revealFlag)
        return;
    g_gameAState = 0x12;
    g_gameFlags8 |= 0x01;
    g_gameFlags2 |= 0x02;
    g_p2Options &= ~0x80;
    BGM_Queue = 0xff;
    g_gameALevel &= ~0x02;
    if (g_topModeFlags & 0x01)
        return;
    g_turnCount--;
}

// Game A state $1d: the window slides down 6 lines a frame for $ffb1
// frames. Then go to state 9 and pick the next song.
void state1DSlideWindow(void) __banked
{
    rWY += 6;
    if (g_gameATurnTimer == 0x18)
        SFX_Queue = 0x0f;
    if (--g_gameATurnTimer)
        return;
    rLCDC &= ~0x20;
    g_musicCh4Flags &= ~0x80;
    g_gameAState = 0x09;
    if (g_gameFlags4 & 0x0c) {
        BGM_Queue = 0x12;
        g_gameALevel &= ~0x02;
        return;
    }
    if (g_gameFlags4 & 0x40) {
        BGM_Queue = 0x06;
    } else if ((g_gameFlags9 & 0x03) || (g_gameFlags5 & 0x10)) {
        g_gameALevel |= 0x02;
        BGM_Queue = 0x1c;
        if (!(g_gameFlags3 & 0x40)) {
            BGM_Queue = 0x11;
            if ((g_gameFlags9 & 0x60) == 0x20)
                BGM_Queue = 0x13;
        }
    }
    g_gameFlags4 &= ~0x40;
    g_gameFlags9 &= 0x7c;
    g_gameFlags5 &= ~0x10;
}
