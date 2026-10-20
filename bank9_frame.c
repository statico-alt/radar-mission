// Per-frame helpers: palette fade and random seed.
// They read no bank 1 data, so they are in bank 9.
#include "bank1.h"

// Fade rBGP, rOBP0 and rOBP1 one step toward the targets at $c0a0.
// Runs every 8 frames while bits 0-1 of $ffaa are set.
// Bit 0 of $ffaa set: fade in toward the targets. Clear: fade out to 0.
// After 3 steps it clears bits 0-1 of $ffaa.
void updatePaletteFade(void) __banked
{
    if (!(g_gameFlags2 & 0x03))
        return;
    if (--g_fadeStepTimer)
        return;
    g_fadeStepTimer = 8;
    volatile uint8_t *pal = &rBGP;
    uint8_t *target = &g_fadeTargetBgp;
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t fadeIn = g_gameFlags2 & 0x01;
        uint8_t want = fadeIn ? target[i] : 0;
        uint8_t cur = pal[i];
        uint8_t out = 0;
        // Each 2-bit shade, high shade first, moves one step.
        for (uint8_t j = 0; j < 4; j++) {
            uint8_t shade = cur >> 6;
            if (shade != want >> 6)
                shade = fadeIn ? shade + 1 : shade - 1;
            out = (out << 2) | (shade & 3);
            cur <<= 2;
            want <<= 2;
        }
        pal[i] = out;
    }
    if (++g_fadeStepCount == 3) {
        g_fadeStepCount = 0;
        g_gameFlags2 &= 0xfc;
    }
}

// Add rDIV (rotated left 2) to the random seed at $ffe0. Returns the new seed.
uint8_t stirRandom(void) __banked
{
    uint8_t d = rDIV;
    uint8_t seed = g_gameARandomSeed;
    seed += (uint8_t)((d << 2) | (d >> 6));
    SET_HRAM(g_gameARandomSeed, seed);
    return seed;
}
