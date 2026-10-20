// Game B: the end-of-stage tally (score, time bonus, ranks). Bank 0
// calls these through Bank1_TallyVectors at $7800. The steps that read
// no bank 1 data are in bank9_tally.c.
#include "bank1.h"

// Return v / d and leave v % d in v.
static uint8_t divide8(uint8_t *v, uint8_t d)
{
    uint8_t q = 0;
    for (; *v >= d; *v -= d)
        q++;
    return q;
}

// Tally state 1. Every 8 frames draw the next row: name, point value and
// kill count. Add the row value once for each kill to the row product.
// After 7 rows go to state 2.
void tallyState1RollRanks(void)
{
    if (--g_tallyStepTimer)
        return;
    g_tallyStepTimer = 0x08;
    uint8_t row = g_tallyRowIndex;
    uint16_t dst = 0x9841 + (uint8_t)(row << 5);
    TS[0] = dst >> 8;
    TS[1] = (uint8_t)dst;
    TS[2] = 0x12;
    fillBytes(TS + 3, 0x82, 0x0f);
    copyBytes(TS + 3, GameB_TallyRowNameTiles + (uint8_t)((row << 1) + row), 3);
    copyBytes(TS + 0x0f, GameB_TallyBonusTiles, 5);
    // 4 digits per row, $1c bytes per skill level. $82 is a blank digit.
    const uint8_t *value = GameB_TallyRowPayloads + (uint8_t)((g_gameBSkill << 5) - (g_gameBSkill << 2)) + (uint8_t)(row << 2);
    for (uint8_t i = 0; i != 4; i++) {
        uint8_t d = value[i];
        TS[0x0e - i] = d == 0x82 ? 0x82 : d + 0x84;
    }
    DRAW_TILE_SCRIPT();
    SFX_Queue = 0x19;
    uint8_t kills = g_killCountsByClass[GameB_TallyKillCountPtrTable[row]];
    g_tallyKillCountTile = kills + 0x84;
    if (kills) {
        fillBytes(&g_scoreAddendBCD, 0, 6);
        fillBytes(&g_rowMultiplicandBCD, 0, 6);
        for (uint8_t i = 0; i != 4; i++)
            (&g_rowMultiplicandBCD)[i] = value[i] == 0x82 ? 0 : value[i];
        do
            bcdAdd(&g_scoreAddendBCD, &g_rowMultiplicandBCD, 5);
        while (--kills);
        bcdAdd(&g_rowProductBCD, &g_scoreAddendBCD, 6);
    }
    if (++g_tallyRowIndex == 0x07)
        g_tallyCutsceneSubState++;
}

// Tally state 2: after 8 frames draw the header row with the kill
// count, and add the kill count * 1000 to the row product.
void tallyState2DrawHeader(void)
{
    if (--g_tallyStepTimer)
        return;
    g_tallyStepTimer = 0x08;
    copyBytes(TS, GameB_TallyHeader, 0x16);
    uint8_t kills = g_killCount;
    g_tallyKillCountTile = kills + 0x84;
    fillBytes(&g_rowMultiplicandBCD, 0, 6);
    (&g_rowMultiplicandBCD)[3] = kills;
    bcdAdd(&g_rowProductBCD, &g_rowMultiplicandBCD, 6);
    DRAW_TILE_SCRIPT();
    SFX_Queue = 0x19;
    g_tallyCutsceneSubState++;
}

// Tally state 3: after 8 frames draw the time bonus row. With
// g_sceneFlagsC bit 0 the bonus is the rank floor (in seconds) minus the
// game time, times 100. Add it to the row product, then go to state 4.
void tallyState3ComputeTimeBonus(void)
{
    if (--g_tallyStepTimer)
        return;
    g_tallyStepTimer = 0x08;
    fillBytes(&g_timeBonusDigit1, 0, 5);
    uint8_t *clock = &g_gameClock;
    if (g_sceneFlagsC & 0x01) {
        uint8_t m = clock[2];
        uint16_t t = (m << 6) - (m << 2) + clock[1];
        uint16_t floor = WORD(GameB_TimeBonusRankFloor[g_gameBSkill << 1]);
        if (t <= floor) {
            uint8_t v = floor - t;
            g_timeBonusDigit10000 = divide8(&v, 100);
            g_timeBonusDigit1000 = divide8(&v, 10);
            g_timeBonusDigit100 = v;
        }
    }
    copyBytes(TS, GameB_TimeBonusBanner, 0x16);
    // The minutes, then the seconds: tens digit, then ones digit.
    uint8_t *p = &g_tallyTimeMinutes;
    for (uint8_t i = 2; i; i--, p += 3) {
        uint8_t v = clock[i];
        p[-1] = divide8(&v, 10) + 0x84;
        p[0] = v + 0x84;
    }
    // The 5 bonus digits, high digit first.
    for (uint8_t i = 0; i != 5; i++)
        TS[0x10 + i] = (&g_timeBonusDigit10000)[-i] + 0x84;
    // 6 bytes: the 5 digits and one more byte ($d80c).
    bcdAdd(&g_rowProductBCD, &g_timeBonusDigit1, 6);
    DRAW_TILE_SCRIPT();
    SFX_Queue = 0x19;
    g_tallyCutsceneSubState = 0x04;
    g_tallyRowIndex = 0x02;
    g_tallyStepTimer = 0x20;
}

// Set up the tally screen: row labels and the 12 row sprites. Then clear
// the total and the scroll, go to tally state 1 and stop the music.
void tallyInitScreen(void)
{
    ClearShadowOAM();
    g_tallyRowIndex = 0;
    g_levelBlinkCount = 0;
    fillBytes((uint8_t *)0x9800, 0x82, 0x240);
    DrawTilemapCmds(GameB_TallyRowLabels);
    uint8_t *s = OAM;
    const uint8_t *yx = GameB_TallyRowSprites;
    uint8_t n = 12;
    do {
        s[0] = *yx++;
        s[1] = *yx++;
        s[3] = 0;
        s += 4;
    } while (--n);
    g_sceneFlagsA &= ~0x04;
    fillBytes(&g_scoreTotalBCD, 0, 6);
    rSCY = 0;
    rSCX = 0;
    g_tallyRowIndex = 0;
    g_tallyCutsceneSubState = 1;
    g_tallyStepTimer = 0x18;
    BGM_Queue = 0xff;
}

// Set the X of 6 sprites at s from x, or 0 while the blink is off.
static void setBlinkingSpriteX(uint8_t *s, const uint8_t *x)
{
    uint8_t n = 6;
    do {
        uint8_t v = *x++;
        *s = g_rankTimer ? 0 : v;
        s += 4;
    } while (--n);
}

// Every $14 frames blink the score sprites picked by bits 5-6 of
// g_cutsceneFlags. With both bits a blink round is counted.
void blinkTallySprites(void)
{
    if (--g_tallyBlinkTimer)
        return;
    g_tallyBlinkTimer = 0x14;
    uint8_t f = g_cutsceneFlags;
    if (f & 0x60) {
        g_rankTimer ^= 0x10;
        if ((f & 0x60) == 0x60)
            g_tallyBlinkRoundsLeft--;
    }
    if (f & 0x20)
        setBlinkingSpriteX(OAM + 0x01, GameB_BlinkSpriteXA);
    if (f & 0x40)
        setBlinkingSpriteX(OAM + 0x19, GameB_BlinkSpriteXB);
}
