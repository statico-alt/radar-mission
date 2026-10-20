// Game B rank screen, the title cutscene setup and the level select input.
// They read the bank 3 rank data, so they stay in bank 3.
#include "game.h"
#include "bank3.h"

#define OAM (&g_oamBuffer)
#define RANK_TIMER_HI (&g_rankTimer)[1]
#define LINK_GAME (g_topModeFlags & 0x80)

// Every $30 frames: flash OBP1, or blink "NO RANK" at rank 5.
void gameBCutsceneTimerStep(void)
{
    if (--g_tallyStepTimer)
        return;
    g_tallyStepTimer = 0x30;
    if (g_rankIndex != 5) {
        rOBP1 ^= 0x80;
        return;
    }
    uint8_t *p = &g_tileScript;
    g_rankTextScriptEnd = 0;
    p[0] = 0x9e;
    p[1] = 0x0c;
    p[2] = 7;
    fillBytes(p + 3, 2, 7);
    if (g_tallyRowIndex)
        copyBytes(p + 3, GameB_RankText_NoRank, 7);
    g_tallyRowIndex ^= 1;
    g_modeFlags15 |= 0x40;
}

// Clears OAM and the BG map at $9c00. Draws the frame, 5 rank rows, the
// score, the rank names as sprites and the name of the rank in g_rankIndex.
void gameBDrawRankScreen(void)
{
    ClearShadowOAM();
    fillBytes((uint8_t *)0x9c00, 2, 0x240);
    DrawTilemapScript(Rank_FrameStrings);
    uint8_t *row = (uint8_t *)0x9c6c;
    for (uint8_t c = 5; c; c--) {
        copyBytes(row + 1, Rank_RowDotTiles, 5);
        *row = c + 0x84;
        row += 0x40;
    }
    uint8_t *d = (uint8_t *)0x9dd2;
    const uint8_t *s = &g_displayedScoreBCD;
    for (uint8_t c = 6; c; c--)
        *d-- = *s++ + 0x84;
    uint8_t *o = OAM;
    s = GameB_RankStrings;
    for (uint8_t y = 0x28; y != 0x78; y += 0x10) {
        uint8_t n = *s++;
        uint8_t x = 0x18;
        do {
            o[0] = y;
            o[1] = x;
            o[2] = *s++;
            o[3] = 0;
            o += 4;
            x += 8;
        } while (--n);
    }
    uint8_t r = g_rankIndex;
    if (r != 5) {
        s = GameB_RankMedalLayout + (r << 1);
        o = OAM + 3 + (uint8_t)(s[0] << 2);
        uint8_t n = s[1];
        do {
            *o = 0x10;
            o += 4;
        } while (--n);
    }
    s = GameB_RankStrings + GameB_RankStringOffsets[r];
    copyBytes((uint8_t *)0x9e0a + (uint8_t)(9 - s[0]), s + 1, s[0]);
    g_tallyRowIndex = 0;
    g_tallyStepTimer = 0x18;
}

// Sets g_rankIndex: the first rank whose 4-digit threshold the score
// (top 4 digits) reaches. 5 = none.
void gameBSelectRank(void)
{
    const uint8_t *t = GameB_RankThresholds;
    uint8_t r = 0;
    for (; r != 5; r++, t += 4) {
        const uint8_t *s = &g_displayedScoreBCD + 5;
        uint8_t i = 0;
        while (i != 4 && *s == t[i]) {
            s--;
            i++;
        }
        if (i == 4 || *s > t[i])
            break;
    }
    g_rankIndex = r;
}

// Reset the scroll and go to the next substate.
static void rankScreenExit(void)
{
    g_subStateScratch4 &= ~0x02;
    rSCY = 0;
    rSCX = 0;
    g_subState++;
}

// Adds each star speed to its fraction. On a carry the star band SCX
// moves by the step. Records: fraction, speed, step.
void rankScreenMoveStars(void)
{
    uint8_t *s = &g_starRecords;
    uint8_t *x = &g_rasterScx2;
    uint8_t c = g_starCount;
    do {
        uint8_t f = s[0] + s[1];
        if (f < s[1])
            *x += s[2];
        s[0] = f;
        s += 3;
        x++;
    } while (--c);
}

// Every 3 frames (64 after the stop flag) move the 23 sprites at $c000.
void rankScreenScrollSprites(void)
{
    if (--g_levelBlinkCount)
        return;
    uint8_t stop = g_cutsceneFlags & 0x02;
    g_levelBlinkCount = stop ? 0x40 : 3;
    uint8_t *o = OAM;
    for (uint8_t c = 0x17; c; c--) {
        if (!stop)
            o[0]--;
        o[1]--;
        o += 4;
    }
    (&g_rasterScxTable)[6]++;
    if (stop)
        return;
    if (g_tallyRowIndex < 9) {
        (&g_rasterLycTable)[4]--;
        (&g_rasterScyTable)[6]++;
    }
    if (!--g_tallyRowIndex)
        g_cutsceneFlags |= 0x02;
}

// Queue one "CONGRATULATIONS" tile (index i) for the VBlank tilemap script.
static void queueCongratsTile(uint8_t i)
{
    uint8_t *p = &g_tileScript;
    uint16_t a = 0x9882 + i;
    p[0] = a >> 8;
    p[1] = a;
    p[2] = 1;
    p[3] = Rank_CongratulationsTiles[i];
    p[4] = 0;
    g_modeFlags15 |= 0x40;
}

// Go to state 3 (wait).
static void startWait(void)
{
    g_rankScreenState = 3;
    g_cutsceneFlags |= 0x08;
    g_rankTimer = 0x68;
    RANK_TIMER_HI = 1;
}

// State 0: wait for the star band SCX to clear. Copy the star data and
// pick the next state by the rank.
static void rankState00(void)
{
    if (g_rasterScx2)
        return;
    copyBytes(&g_starRecords, Rank_StarRecordsInit, 15);
    WORD(g_rankTimer) = 0;
    g_tallyStepTimer = 1;
    uint8_t r = g_rankIndex;
    if (r == 4) {
        g_rankScreenState = 1;
    } else if (r == 3) {
        g_rankScreenState = 4;
        g_tallyRowIndex = 0xb0;
        RANK_TIMER_HI = 0x64;
        g_tallyStepTimer = 0x20;
        g_rankFlybyPassesLeft = 1;
    } else {
        g_rankScreenState = 2;
        g_rankStripSpriteCount = 5;
        g_tallyRowIndex = r == 1 ? 4 : 1;
    }
}

// State 1: every $18 frames start a sparkle in a free slot (16 in all).
// Each sparkle grows for 3 steps of 8 frames. Its first step shows one
// "CONGRATULATIONS" tile. After 16 tiles go to state 3.
static void rankState01(void)
{
    uint8_t *s;
    uint8_t c;
    if (!(g_cutsceneFlags & 0x04) && !--g_tallyStepTimer) {
        g_tallyStepTimer = 0x18;
        NoiseSfx_Queue = 1;
        s = &g_rankSparkleSlots;
        for (c = 0; c != 4 && *s != 0xff; c++)
            s += 2;
        if (c != 4) {
            s[0] = 8;
            s[1] = 0;
            uint8_t *o = OAM + 0x5c + (uint8_t)(c << 4);
            uint8_t x = (Rank_RevealOrder[g_rankTimer] << 3) + 0x14;
            uint8_t y = 0x2c, t = 0x30;
            for (uint8_t k = 2; k; k--) {
                *o++ = y;
                *o++ = x;
                *o++ = t++;
                *o++ = 0;
                *o++ = y;
                *o++ = x + 8;
                *o++ = t++;
                *o++ = 0;
                y += 8;
            }
            if (++g_rankTimer == 0x10)
                g_cutsceneFlags |= 0x04;
        }
    }
    s = &g_rankSparkleSlots;
    for (c = 0; c != 4; c++, s += 2) {
        if (s[0] == 0xff || --s[0])
            continue;
        s[0] = 8;
        uint8_t f = ++s[1];
        uint8_t *o = OAM + 0x5c + (uint8_t)(c << 4);
        if (f == 3) {
            fillBytes(o, 0, 0x10);
            s[0] = 0xff;
            s[1] = 0xff;
            RANK_TIMER_HI++;
        } else {
            o += 2;
            for (uint8_t k = 4; k; k--) {
                *o += 4;
                o += 4;
            }
            if (f == 1)
                queueCongratsTile(Rank_RevealOrder[RANK_TIMER_HI]);
        }
    }
    if (RANK_TIMER_HI == 0x10)
        startWait();
}

// State 2: move the star strip sprites. At the times in Rank_StarStripTimes
// build the next strip. After $50 frames repeat, or go to state 4.
static void rankState02(void)
{
    uint8_t *o = OAM + 0x5c;
    uint8_t c = g_rankStripSpriteCount;
    do {
        o[0]--;
        o[1] += 2;
        o += 4;
    } while (--c);
    uint8_t t = g_rankTimer;
    if (t == Rank_StarStripTimes[0] || t == Rank_StarStripTimes[1] || t == Rank_StarStripTimes[2]) {
        uint8_t k = RANK_TIMER_HI;
        const uint8_t *e = Rank_StarStripTable + (uint8_t)(k + k + k);
        uint8_t attr = 0;
        g_rankStripSpriteCount = e[2];
        if (!k) {
            SFX_Queue = 0x1c;
            attr = 0x80;
        }
        buildSpriteStrips(*(const uint8_t *const *)e, (attr << 8) | e[2], OAM + 0x5c);
        RANK_TIMER_HI++;
    }
    if (++g_rankTimer != 0x50)
        return;
    WORD(g_rankTimer) = 0;
    g_tallyStepTimer = 0x20;
    if (--g_tallyRowIndex)
        return;
    g_rankScreenState = 4;
    g_tallyStepTimer = 0x60;
    g_rankTimer = 0x40;
    RANK_TIMER_HI = 0x64;
    uint8_t r2 = g_rankIndex == 2;
    g_rankFlybyPassesLeft = r2 ? 1 : 2;
    g_tallyRowIndex = r2 ? 0 : 0xb0;
}

// State 3: wait. A button (bits 0-3) or the end of the timer exits.
static void rankState03(void)
{
    if (g_joyPressed & 0x0f) {
        rankScreenExit();
        return;
    }
    if (--g_rankTimer)
        return;
    if (!RANK_TIMER_HI)
        rankScreenExit();
    else
        RANK_TIMER_HI--;
}

// State 4: two fly-by sprite groups. Then state 1 (rank 2) or state 5.
static void rankState04(void)
{
    if (g_rankTimer) {
        g_rankTimer--;
    } else if (g_tallyRowIndex) {
        if (g_tallyRowIndex == 0xb0) {
            SFX_Queue = 0x1c;
            buildSpriteStrips(Rank_State4SpritesA, 0x8002, OAM + 0x5c);
        }
        g_tallyRowIndex--;
        OAM[0x5d]--;
        OAM[0x61]--;
    }
    if (g_tallyStepTimer) {
        g_tallyStepTimer--;
        return;
    }
    if (RANK_TIMER_HI) {
        if (RANK_TIMER_HI == 0x64) {
            SFX_Queue = 0x1c;
            buildSpriteStrips(Rank_State4SpritesB, 0x300a, OAM + 0x64);
        }
        RANK_TIMER_HI--;
        uint8_t *o = OAM + 0x65;
        for (uint8_t c = 10; c; c--) {
            *o -= 2;
            o += 4;
        }
        return;
    }
    if (g_tallyRowIndex)
        return;
    g_tallyRowIndex = 0xb0;
    RANK_TIMER_HI = 0x64;
    g_tallyStepTimer = 0x20;
    if (--g_rankFlybyPassesLeft)
        return;
    g_tallyStepTimer = 0x40;
    WORD(g_rankTimer) = 0;
    g_rankScreenState = g_rankIndex == 2 ? 1 : 5;
}

// State 5: after the timer build the end sprites. Move them, and show the
// 16 "CONGRATULATIONS" tiles one by one. Then go to state 3.
static void rankState05(void)
{
    if (g_tallyStepTimer) {
        if (!--g_tallyStepTimer) {
            SFX_Queue = 0x1c;
            buildSpriteStrips(Rank_EndSprites, 0x100a, OAM + 0x5c);
        }
        return;
    }
    uint8_t *o = OAM + 0x5d;
    for (uint8_t c = 10; c; c--) {
        *o += 2;
        o += 4;
    }
    if (++g_rankTimer != 4)
        return;
    g_rankTimer = 0;
    uint8_t a = ++RANK_TIMER_HI;
    if (a == 0x19)
        startWait();
    else if (a >= 7 && a < 0x17)
        queueCongratsTile(a - 7);
}

// Entry from bank 0. Start exits. Rank 0 runs the intro screen code.
void rankScreenFrame(void)
{
    if (g_joyPressed & 0x08) {
        rankScreenExit();
        return;
    }
    if (!(g_cutsceneFlags & 0x08))
        rankScreenScrollSprites();
    if (!g_rankIndex) {
        GameB_IntroUpdate();
        return;
    }
    if (!(g_cutsceneFlags & 0x08))
        rankScreenMoveStars();
    switch (g_rankScreenState) {
    case 0: rankState00(); break;
    case 1: rankState01(); break;
    case 2: rankState02(); break;
    case 3: rankState03(); break;
    case 4: rankState04(); break;
    case 5: rankState05(); break;
    }
}

