// Top state 1: the title menu.
#include "bank0.h"

// Start alone begins the game. 1P sets top state 2 (and Game A state 2).
// 2P starts the link handshake (top state 3).
static void titlePressStart(void)
{
    if (g_joyRaw != 0x08)
        return;
    if (!(g_topModeFlags & 0x80)) {
        g_gameFlags3 |= 0x08;   // the timer IRQ plays g_titleStartSfx
        g_topState = 2;
        g_titleStartSfx = 2;
        SFX_Queue = 7;
        rSB = 0;
        BGM_Queue = 0xff;
        if (!(g_topModeFlags & 0x20)) {
            g_gameAState = 2;
            g_gameALinkState = 8;
        }
        return;
    }
    if (g_topModeFlags & 0x44)
        return;
    g_gameALevel |= 0x08;
    rSC = 0x81;
    rSB = 0xfd;
    g_topState = 3;
}

// Down/Up pick Game B/A (g_topModeFlags bit 5), Left/Right pick 1P/2P
// (bit 7). Only the highest pressed button counts.
void titleUpdate(void) __banked
{
    uint8_t p = g_joyPressed;
    uint8_t *oam = &g_oamBuffer;
    if (p & 0x80) {
        if (g_topModeFlags & 0x20)
            return;
        g_topModeFlags |= 0x20;
        oam[0] = 0x78;
    } else if (p & 0x40) {
        if (!(g_topModeFlags & 0x20))
            return;
        g_topModeFlags &= ~0x20;
        oam[0] = 0x70;
    } else if (p & 0x20) {
        if (!(g_topModeFlags & 0x80))
            return;
        g_topModeFlags &= ~0x80;
        oam[5] = 0x2e;
    } else if (p & 0x10) {
        if (g_topModeFlags & 0x80)
            return;
        g_topModeFlags |= 0x80;
        oam[5] = 0x5e;
    } else {
        if (p & 0x08)
            titlePressStart();
        return;
    }
    SFX_Queue = 1;
}
