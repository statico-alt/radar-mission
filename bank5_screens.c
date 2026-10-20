// Game A screens: title, intro text reveal, level title, results and next level.
// Also the byte copy/fill helpers of the bank 5 C code.
#include "game.h"

extern const uint8_t *const CommanderTilesetPtrs[];
extern const uint8_t CommanderTiles0[], CommanderTiles1[], CommanderTiles2[];
extern const uint8_t ResultsScreenTilemap[];
extern const uint8_t ResultsSprites_P1[], ResultsSprites_P2[];
extern const uint8_t ResultsCommanderFaceTiles[];
extern const uint8_t HitRatioRankThresholds[];
extern const uint8_t ResultsMedalSprites_Small[], ResultsMedalSprites_Large[];
extern const uint8_t NextLevelBgTiles[], NextLevelSpriteTiles[];
extern const uint8_t NextLevelScreenMap[], NextLevelRowTiles[];
extern const uint8_t NextLevelShipInit[], NextLevelShipTiming[];
extern const uint8_t TitleScreenMap[], TitleScreenTilemap[], TitleSprites[];
extern const uint8_t TextRevealScreenMap[], TextRevealSprites[], TextRevealTriggers[];
extern const uint8_t LevelTitleExtraTiles[], LevelTitleSprites[];
extern const uint8_t LevelTitleTilemap[], LevelTitleTilemap_Level1[];
extern const uint8_t LevelTitleTilemap_Level2[], LevelTitleTilemap_Level3[];

// bank 0
void DrawTilemapCmds(const uint8_t *cmds);
void LcdOffMaskIrqs(void);
void LcdOnEnableIrqs(void);
void GameA_DrawFleetPanel(void);
void GameA_LoadCursorTiles(void);
// bank_005.asm stubs for bank 0 routines
// attrCount = sprite attributes << 8 | sprite count.
// bank5_ai.c
uint8_t random(void) __banked;

#define OAM (&g_oamBuffer)
#define QUEUE (&g_vramQueue)
#define VRAM(addr) ((uint8_t *)(addr))

static void setMapStream(const uint8_t *map)
{
    WORD(g_mapStreamPtr) = (uint16_t)map;
}

// n / d like bank 0 Divide16By8: the remainder is 8 bits, so it loses its top
// bit when it grows past $ff. d = 0 gives 0.
static uint8_t divide16By8(uint16_t n, uint8_t d)
{
    uint8_t quotient = 0;
    uint8_t rest = 0;
    if (!d)
        return 0;
    for (uint8_t i = 16; i; i--) {
        rest = (rest << 1) | (n >> 15);
        n <<= 1;
        quotient <<= 1;
        if (rest >= d) {
            rest -= d;
            quotient |= 1;
        }
    }
    return quotient;
}

// Hit ratio in percent: shots[1] (hits) * 100 / shots[0] (shots fired).
static uint8_t calcPercent(const uint8_t *shots)
{
    return divide16By8(multiply8(shots[1], 100), shots[0]);
}

// Set up the end-of-battle results screen: both commander portraits,
// hit ratios, scores and a medal picked from HitRatioRankThresholds.
void resultsScreenInit(void)
{
    copyBytes(VRAM(0x8000), CommanderTilesetPtrs[g_p1Commander], 0x400);
    copyBytes(VRAM(0x8400), CommanderTilesetPtrs[g_p2Commander], 0x400);
    fillBytes(VRAM(0x9c20), 0x02, 0x80);
    fillBytes(VRAM(0x9ce0), 0x01, 0xa0);
    DrawTilemapCmds(ResultsScreenTilemap);
    buildSpriteStrips(ResultsSprites_P1, 0x1010, OAM);
    buildSpriteStrips(ResultsSprites_P2, 0x1010, OAM + 0x40);
    g_p1HitRatio = calcPercent(&g_p1ShotsFired);
    g_p2HitRatio = calcPercent(&g_p2ShotsFired);
    drawNumber3(VRAM(0x9c45), g_p1HitRatio);
    drawNumber3(VRAM(0x9c4b), g_p2HitRatio);
    drawNumber3(VRAM(0x9c86), g_linkSendPair[1]);
    drawNumber3(VRAM(0x9c8c), g_linkRecvPair[1]);
    *VRAM(0x9c85) = g_linkSendPair[0] + 0xd8;
    *VRAM(0x9c8b) = g_linkRecvPair[0] + 0xd8;
    drawNumber3(VRAM(0x9e10), g_gameATotalTurns);
    // Face animation: g_gameFlags8 bit 0 picks the winner's face.
    g_p1FaceAnimTimer = 0x20;
    g_p2FaceAnimTimer = 0x20;
    uint8_t p2Won = g_gameFlags8 & 0x01;
    g_p1FaceAnimPeriod = ResultsCommanderFaceTiles[(uint8_t)(g_p1Commander << 1) + (p2Won ? 0 : 1)];
    g_p2FaceAnimPeriod = ResultsCommanderFaceTiles[(uint8_t)(g_p2Commander << 1) + (p2Won ? 1 : 0)];
    // Rank 1-4: 1 + the number of thresholds above the P1 hit ratio.
    uint8_t rank = 1;
    while (rank != 4 && g_p1HitRatio < HitRatioRankThresholds[rank - 1])
        rank++;
    g_p1HitRank = rank;
    if (rank < 3)
        buildSpriteStrips(ResultsMedalSprites_Small, 0x3006, OAM + 0x80);
    else if (rank == 3)
        buildSpriteStrips(ResultsMedalSprites_Small, 0x3002, OAM + 0x80);
    else
        buildSpriteStrips(ResultsMedalSprites_Large, 0x3004, OAM + 0x80);
    WORD(g_idleTimer) = 0x6cd0;
    g_fadeTargetObp0 = 0x27;
    g_gameFlags2 |= 0x01;
    g_gameAResultsStep = 1;
    g_p2Options &= ~0x80;
    g_gameADelay = 0x40;
    BGM_Queue = p2Won ? 0x02 : 0x10;
}

// Next-level screen state 0: turn the LCD off, load the BG and sprite tiles, clear OAM,
// and draw the screen map into $9c00.
static void nextLevelInit(void)
{
    LcdOffMaskIrqs();
    copyBytes(VRAM(0x8800), NextLevelBgTiles, 0x350);
    copyBytes(VRAM(0x8010), NextLevelSpriteTiles, 0x100);
    fillBytes(OAM, 0, 0xa0);
    setMapStream(NextLevelScreenMap);
    uint8_t *vram = VRAM(0x9c00);
    for (uint8_t row = 0; row != 0x12; row++, vram += 0x20) {
        if (row >= 1 && row < 0x0e)
            copyBytes(vram, NextLevelRowTiles, 0x14);
        unpackRunList(vram);
    }
    // The turn count as 3 digits.
    uint8_t n = g_turnCount;
    uint8_t hundreds = 0, tens = 0;
    while (n >= 100) {
        n -= 100;
        hundreds++;
    }
    while (n >= 10) {
        n -= 10;
        tens++;
    }
    *VRAM(0x9e10) = hundreds + 0xd8;
    *VRAM(0x9e11) = tens + 0xd8;
    *VRAM(0x9e12) = n + 0xd8;
    GameA_DrawFleetPanel();
    GameA_LoadCursorTiles();
    copyBytes(&g_animScriptCount, NextLevelShipInit, 0x17);
    copyBytes(g_nextLevelShipTiming, NextLevelShipTiming, 0x0a);
    // Two rows of 10 sprites: only Y is set.
    uint8_t *oam = OAM;
    for (uint8_t i = 0; i < 20; i++, oam += 4)
        oam[0] = i < 10 ? 0x80 : 0x88;
    // 5 ships of 2x2 sprites, tiles 1-4, 16 pixels apart.
    oam = OAM + 0x50;
    for (uint8_t x = 0x1c; x != 0x6c; x += 0x10) {
        uint8_t tile = 1;
        for (uint8_t y = 0x80; y != 0x90; y += 8) {
            for (uint8_t dx = 0; dx != 0x10; dx += 8) {
                *oam++ = y;
                *oam++ = x + dx;
                *oam++ = tile++;
                *oam++ = 0;
            }
        }
    }
    g_gameFlags6 |= 0x40;
    g_gameFlags2 |= 0x01;
    g_nextLevelStep = 1;
    BGM_Queue = 0x14;
    g_fadeTargetObp0 = 0x36;
    g_fadeTargetObp1 = 0xc6;
    LcdOnEnableIrqs();
    rLCDC = 0xcb;
}

// Next-level screen state 1: move the ship sprites (OAM + $50) in.
// g_nextLevelShipTiming holds a start delay and a step count for each ship.
// Each step moves the ship's 4 sprites 1 pixel up.
static void nextLevelSailIn(void)
{
    uint8_t *timing = g_nextLevelShipTiming;
    uint8_t *oam = OAM + 0x50;
    for (uint8_t ship = 0; ship != 5; ship++, timing += 2, oam += 0x10) {
        if (timing[0]) {
            timing[0]--;
            continue;
        }
        if (!timing[1])
            continue;
        for (uint8_t i = 0; i != 0x10; i += 4)
            oam[i]--;
        // At the last step, start the ship's anim script.
        if (!--timing[1])
            (&g_animRecords)[(uint8_t)(ship << 2) - 1] = 1;
    }
    timing = g_nextLevelShipTiming + 1;
    for (uint8_t n = 5; n; n--, timing += 2) {
        if (*timing)
            return;
    }
    g_gameFlags3 |= 0x80;
    g_nextLevelStep = 2;
}

// On A or Start: reset the HRAM work and the VRAM queue area, refill the ship
// hit points, go to the next level (g_gameALevel += $40) and start the battle.
static void nextLevelWaitButton(void)
{
    if (!(g_joyPressedA & 0x09))
        return;
    fillBytes(&g_gameAPlaceState, 0, 0x4e);
    fillBytes(QUEUE, 0, 0x920);
    // Ships still afloat get all hit points back. Only the first sunk ship
    // comes back too.
    uint8_t restored = 0;
    for (uint8_t i = 0; i < 5; i++) {
        if (!g_shipHitsLeft[i]) {
            if (restored)
                continue;
            restored = 1;
        }
        g_shipHitsLeft[i] = g_p1ShipLengths[i];
    }
    g_revealFlag = 0;
    g_gameALevel += 0x40;
    g_p1Options &= 0xfc;
    g_gameFlags2 |= 0x02;
    g_fadeTargetObp0 = 0xb4;
    g_fadeTargetObp1 = 0x36;
    g_gameAState = 0x0e;
    g_gameATitleStep = 0;
    g_turnCount = 1;
    BGM_Queue = 0xff;
}

// Screen between levels: the fleet sails in, then waits for A or Start.
// B or Select switches the window and BG map. Sub-state in g_nextLevelStep.
void nextLevelScreen(void)
{
    if (g_joyPressedA & 0x06)
        rLCDC ^= 0x0a;
    switch (g_nextLevelStep) {
    case 0: nextLevelInit(); break;
    case 1: nextLevelSailIn(); break;
    default: nextLevelWaitButton(); break;
    }
}

// Clear the screen, draw the title screen, and set up sound,
// timer, interrupts and palettes.
void titleScreenInit(void)
{
    fillBytes(VRAM(0x9800), 0, 0x800);
    setMapStream(TitleScreenMap);
    uint8_t *vram = VRAM(0x9800);
    for (uint8_t n = 0x0b; n; n--, vram += 0x20)
        unpackRunList(vram);
    DrawTilemapCmds(TitleScreenTilemap);
    buildSpriteStrips(TitleSprites, 0x0002, OAM);
    if (g_topModeFlags & 0x20)
        OAM[0] = 0x78;
    if (g_topModeFlags & 0x80)
        OAM[5] = 0x5e;
    g_topModeFlags |= 0x0a;
    g_gameFlags2 |= 0x01;
    rSB = 0;
    g_serialSend = 0;
    rBGP = 0;
    rOBP0 = 0;
    rOBP1 = 0;
    rSCY = 0;
    rSCX = 0;
    g_fadeTargetBgp = 0xc6;
    g_fadeTargetObp0 = 0x27;
    g_fadeTargetObp1 = 0x27;
    g_fadeStepTimer = 0x08;
    g_statEffectTimer = 0x24;
    rSTAT |= 0x40;
    rIE = 0x07;
    rNR52 = 0x80;
    rNR50 = 0x77;
    g_ambientSfxTimer = 0x77;
    rNR51 = 0xff;
    rTMA = 0xb8;
    rTIMA = 0xb8;
    rTAC = 0x04;
    g_soundCueRequest = 0x04;
    BGM_Queue = 0x0f;
    rWX = 0x07;
    g_turnCount = 1;
    rLCDC = 0xc3;
    __asm__("ei");
}

// Draw the full-screen text page and set up its sprites.
void textRevealScreenInit(void)
{
    uint8_t *q = QUEUE;
    fillBytes(VRAM(0x9800), 0x01, 0x240);
    setMapStream(TextRevealScreenMap);
    // Rows 5-17: one row command each, drawn now.
    for (uint8_t row = 5; row != 0x12; row++) {
        fillBytes(q + 3, 0x01, 0x14);
        unpackRunList(q + 3);
        uint16_t vram = 0x9800 + ((uint16_t)row << 5);
        q[0] = vram >> 8;
        q[1] = vram;
        q[2] = 0x14;
        q[0x17] = 0;
        DrawTilemapCmds(q);
    }
    buildSpriteStrips(TextRevealSprites, 0x000b, OAM);
    uint8_t *anim = &g_animScriptCount;
    anim[0]++;
    anim[1] = 0x0a;
    anim[2] = 0x0b;
    anim[3] = 0x00;
    anim[4] = 0x01;
    anim[5] = 0x00;
    g_gameFlags6 |= 0x40;
    g_gameFlags2 |= 0x01;
    g_fadeTargetObp0 = 0x27;
    g_fadeTargetObp1 = 0xe4;
    g_gameATurnTimer = 0;
    g_listCount = 0;
    g_gameAState = 1;
    uint8_t *ship = &g_shipRecords;
    ship[5] = ship[0];
    ship[6] = ship[0];
}

// Move the sprites right; when the first one reaches an X in TextRevealTriggers,
// queue the matching two tiles. After $f0 frames go to Game A state $1e.
void textRevealScreenUpdate(void)
{
    uint8_t t = ++g_gameATurnTimer;
    if (t == 0xf0) {
        g_gameAState = 0x1e;
        g_gameFlags2 |= 0x02;
        g_gameFlags6 &= ~0x40;
        return;
    }
    if (t == 0x01)
        BGM_Queue = 0x0e;
    if (t == 0x11)
        NoiseSfx_Queue = 0x0b;
    if (t == 0x74) {
        fillBytes(OAM, 0, 0x2c);
        return;
    }
    if (t > 0x74)
        return;
    uint8_t *oam = OAM + 1;
    for (uint8_t n = 0x0b; n; n--, oam += 4)
        *oam += 2;
    uint8_t x = OAM[1];
    const uint8_t *trigger = TextRevealTriggers;
    for (uint8_t n = 9; *trigger != x; trigger += 3) {
        if (!--n)
            return;
    }
    // Two tiles down a column. The column is X / 8 - 1, with the 9-bit
    // rotate of the asm (rra x3).
    uint8_t *q = QUEUE;
    copyBytes(q + 3, trigger + 1, 2);
    uint16_t vram = 0x9860 + (uint8_t)(((x >> 3) | (uint8_t)(x << 6)) - 1);
    q[0] = vram >> 8;
    q[1] = vram;
    q[2] = 0x82;
    q[6] = 0;
    g_gameFlags3 |= 0x04;
}

static const uint8_t *const levelTitleFleetTiles[] = {
    CommanderTiles0, CommanderTiles1, CommanderTiles2,
};

// Draw the level title screen for the level in g_gameALevel bits 6-7.
void levelTitleScreenInit(void)
{
    copyBytes(VRAM(0x8900), LevelTitleExtraTiles, 0x20);
    copyBytes(VRAM(0x8800), levelTitleFleetTiles[g_p2Commander], 0x100);
    fillBytes(OAM, 0, 0xa0);
    fillBytes(VRAM(0x9820), 0, 0x200);
    fillBytes(VRAM(0x9820), 0xb5, 0x140);
    DrawTilemapCmds(LevelTitleTilemap);
    uint8_t level = g_gameALevel & 0xc0;
    DrawTilemapCmds(!level ? LevelTitleTilemap_Level1
                    : level == 0x40 ? LevelTitleTilemap_Level2 : LevelTitleTilemap_Level3);
    if (level) {
        copyBytes(OAM, LevelTitleSprites, 8);
        g_gameATurnTimer = 0x18;
        g_gameFlags8 |= 0x80;
    }
    g_gameFlags2 |= 0x01;
    g_gameATitleStep = 1;
    g_listCount = 0x20;
}

// When g_gameALevel bit 1 is set, play a random ambient sound every $78-$f7 frames.
void randomAmbientSfx(void)
{
    if (!(g_gameALevel & 0x02))
        return;
    if (--g_ambientSfxTimer)
        return;
    uint8_t r = random();
    NoiseSfx_Queue = (r & 0x01) + 0x0d;
    g_ambientSfxTimer = (r & 0x7f) + 0x78;
}
