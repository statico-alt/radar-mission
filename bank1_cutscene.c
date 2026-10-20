// Game B title cutscene steps that run every frame. Bank 0 calls these
// through Bank1_TallyVectors at $7800. They are in bank 1, so no glue
// slows them down.
#include "bank1.h"

// Cutscene state 1: draw the 4 steps of GameB_CutsceneSceneSteps (wait
// frames, then a tilemap script of $18 bytes). Then go to state 2.
void cutsceneState1DrawScene(void)
{
    if (--g_tallyStepTimer)
        return;
    const uint8_t *step = GameB_CutsceneSceneSteps + (uint8_t)((g_tallyRowIndex << 1) + g_tallyRowIndex);
    g_tallyStepTimer = step[0];
    copyBytes(TS, (const uint8_t *)WORD(step[1]), 0x18);
    DRAW_TILE_SCRIPT();
    if (++g_tallyRowIndex != 0x04)
        return;
    g_cutsceneD828Step = 0x02;
    g_tallyRowIndex = 0;
    g_levelBlinkCount = 0;
}

// Cutscene state 2: move the stars for 256 frames. Each star has a
// fraction and a speed in g_cutsceneObjMotion. In a star record (3
// bytes) byte 1 is the position and byte 2 the direction (bit 7 = left).
// One more pair slows the scene sprites. Then go to state 3.
void cutsceneState2MoveObjects(void)
{
    if (!++g_tallyRowIndex) {
        g_cutsceneD828Step = 0x03;
        g_tallyRowIndex = 0x03;
        g_tallyStepTimer = 0x80;
        return;
    }
    uint8_t *m = &g_cutsceneObjMotion;
    uint8_t *star = &g_starRecords + 1;
    for (uint8_t n = g_starCount; n; n--, m += 2, star += 3) {
        uint8_t f = m[0] + m[1];
        uint8_t over = f < m[0];
        m[0] = f;
        if (over && !(star[0] += (star[1] & 0x80) ? 0xff : 0x01))
            star[1] = 0x01;
    }
    uint8_t f = m[0] + m[1];
    uint8_t over = f < m[0];
    m[0] = f;
    if (!over)
        return;
    // Slow the sprites down; at speed 0 they turn left.
    if (!(g_cutsceneSpriteSpeed += (g_cutsceneSpriteStep & 0x80) ? 0x01 : 0xff))
        g_cutsceneSpriteStep = 0xff;
}

// Cutscene state 3: after the timer, every 3 frames scroll the scene band
// up one line. At LYC $5e go to state 4.
void cutsceneState3Scroll(void)
{
    if (g_tallyStepTimer) {
        g_tallyStepTimer--;
        return;
    }
    if (--g_tallyRowIndex)
        return;
    g_tallyRowIndex = 0x03;
    (&g_rasterScyTable)[6]--;
    if (++(&g_rasterLycTable)[4] != 0x5e)
        return;
    g_tallyStepTimer = 0x78;
    g_cutsceneD828Step = 0x04;
}

// Cutscene state 4: wait for the timer, then go to the next sub-state.
void cutsceneState4WaitThenExit(void)
{
    if (!--g_tallyStepTimer)
        g_subState++;
}

// The cutscene sprites. g_cutsceneSpriteSpeed adds to the fraction. On
// overflow the 8 sprites at $c000 move by g_cutsceneSpriteStep. Every
// $10 frames the 7 sprites at $c020 bob and flip. The sprite script
// places or hides the sprites from $c058. The flash lists XOR the
// attributes from $c05a.
void cutsceneAnimateSprites(void)
{
    uint8_t f = g_cutsceneSpriteFrac + g_cutsceneSpriteSpeed;
    uint8_t over = f < g_cutsceneSpriteFrac;
    g_cutsceneSpriteFrac = f;
    if (over) {
        uint8_t *s = OAM + 1;
        uint8_t n = 8;
        do {
            *s += g_cutsceneSpriteStep;
            s += 4;
        } while (--n);
    }
    if (!--g_cutsceneFlipTimer) {
        g_cutsceneFlipTimer = 0x10;
        // $d832 is the bob direction.
        uint8_t dy = ((&g_cutsceneFlipTimer)[1] ^= 0x01) ? 0xff : 0x01;
        uint8_t *s = OAM + 0x20;
        const uint8_t *flip = GameB_CutsceneAttrFlips;
        for (uint8_t n = 7; n; n--, s += 4) {
            s[0] += dy;
            s[2] ^= *flip++;
            // All but the first sprite also flip in X.
            if (n != 7)
                s[3] ^= 0x40;
        }
    }
    if (!--g_cutsceneScriptDelay) {
        // Steps: $fe slot y x tile = show a sprite; $fd slot = hide it;
        // $ff delay = wait.
        uint8_t pos = g_cutsceneScriptPos;
        for (;;) {
            const uint8_t *op = GameB_CutsceneSpriteScript + pos;
            if (op[0] == 0xff) {
                g_cutsceneScriptDelay = op[1];
                g_cutsceneScriptPos = pos + 2;
                g_cutsceneFlashIndex++;
                g_cutsceneFlashPeriod--;
                break;
            }
            uint8_t *s = OAM + 0x58 + (uint8_t)(op[1] << 2);
            if (op[0] == 0xfe) {
                s[0] = op[2];
                s[1] = op[3];
                s[2] = op[4];
                s[3] = 0x10;
                pos += 5;
            } else {
                s[0] = 0xf0;
                pos += 2;
            }
        }
    }
    if (--g_cutsceneFlashTimer)
        return;
    g_cutsceneFlashTimer = g_cutsceneFlashPeriod;
    const uint8_t *list = (const uint8_t *)WORD(GameB_CutsceneFlashPtrs[(uint8_t)(g_cutsceneFlashIndex << 1)]);
    uint8_t *a = OAM + 0x5a;
    for (uint8_t n = *list++; n; n--, a += 4)
        *a ^= *list++;
}

// Follow the scroll position g_cutsceneScrollX. Every 8 pixels queue a
// new tile column at $9980 (and every 8 columns noise $0e). The 14 sprites
// at $c020 move with the scroll. When they are off screen, clear them.
void cutsceneScrollTiles(void)
{
    uint8_t x = g_cutsceneScrollX;
    if (x == g_cutsceneScrollPrevX)
        return;
    uint8_t dx = x - g_cutsceneScrollPrevX;
    g_cutsceneScrollPrevX = x;
    if ((g_rankTimer += dx) >= 0x08) {
        g_rankTimer = 0;
        uint16_t dst = 0x9980 + g_levelBlinkCount++;
        TS[0] = dst >> 8;
        TS[1] = (uint8_t)dst;
        TS[2] = 0xc6;
        TS[3] = 0x02;
        TS[4] = 0x00;
        DRAW_TILE_SCRIPT();
        if (++g_scrollNoiseCounter == 0x08) {
            g_scrollNoiseCounter = 0;
            NoiseSfx_Queue = 0x0e;
        }
    }
    if (g_cutsceneFlags & 0x10)
        return;
    uint8_t *s = OAM + 0x21;
    uint8_t n = 0x0e;
    do {
        *s -= dx;
        s += 4;
    } while (--n);
    if (OAM[0x21] != 0xf8)
        return;
    fillBytes(OAM + 0x20, 0, 0x38);
    g_cutsceneFlags |= 0x10;
}
