// Game B per-frame screen effects: shake, palette flash and window wobble.
#include "game.h"

// Shake offsets, picked by frame counter bits 2-3.
static const uint8_t shakeWY[4] = {0x70, 0x78, 0x80, 0x74};
static const uint8_t shakeSCY[4] = {0x08, 0x0b, 0x10, 0x0c};

// Shake: g_sceneFlagsB bit 6. Moves rWY (or rSCY when g_modeFlags10 bit 4
// is set) unless g_modeFlags17 bit 5 is set, and cycles rBGP.
static void shake(uint8_t mode)
{
    uint8_t f = g_frameCounter;
    uint8_t i = (f & 0x0c) >> 2;
    if (!(g_modeFlags17 & 0x20)) {
        if (mode & 0x10)
            rSCY = shakeSCY[i];
        else
            rWY = shakeWY[i];
    }

    // rBGP from bits 0-5 of the frame counter.
    uint8_t hi = f & 0x30;
    if (!hi)
        hi = 0x10;
    uint8_t lo = f & 0x03;
    uint8_t c1 = lo ? lo : 1;
    uint8_t c3 = lo == 1 ? 0 : lo;
    rBGP = (c3 << 6) | (c1 << 4) | ((f ^ 0x0c) & 0x0c) | (hi >> 2) | i;
}

// Per-frame screen effects. Counts down the two g_screenEffectTimers.
// Shake when g_sceneFlagsB bit 6 is set. Else g_modeFlags13 bit 6: palette
// flash (g_paletteFlashTimer). Else g_modeFlags13 bit 4: window wobble
// (g_windowWobbleTimer). Flash and wobble move rWY only when g_modeFlags10
// bit 4 is clear.
void updateScreenEffects(void)
{
    uint8_t *timers = &g_screenEffectTimers;
    if (timers[0])
        timers[0]--;
    if (timers[1])
        timers[1]--;

    uint8_t mode = g_modeFlags10;
    uint8_t moveWY = !(mode & 0x10);
    uint8_t bgpHigh;
    if (g_sceneFlagsB & 0x40) {
        shake(mode);
        return;
    }
    if (g_modeFlags13 & 0x40) {
        uint8_t t = --g_paletteFlashTimer;
        if (t) {
            bgpHigh = (t & 3) << 4;
            if (moveWY)
                rWY = ((g_frameCounter + bgpHigh) & 0x1e) + g_windowYTarget;
        } else {
            g_modeFlags13 &= ~0x40;
            bgpHigh = 0x36;
            if (moveWY)
                rWY = 0x70;
        }
    } else if (g_modeFlags13 & 0x10) {
        if (--g_windowWobbleTimer) {
            bgpHigh = (g_frameCounter & 3) << 4;
            if (moveWY)
                rWY = (bgpHigh >> 1) + g_windowYTarget;
        } else {
            g_modeFlags13 &= ~0x10;
            bgpHigh = 0x36;
            rWY = 0x70;
        }
    } else {
        return;
    }
    rBGP = (rBGP & 0x06) | bgpHigh;
}
