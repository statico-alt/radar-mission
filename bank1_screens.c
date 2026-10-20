// Game A screen states: the options menu, the victory scene, the fleet
// grids after a battle and the "you lost" board. They read bank 1 data.
#include "bank1.h"

// Bank 0 asm wrapper in bank_001.asm.
uint8_t rotateShipTileStep(void);

// True on level 3 (level bits 6-7 = 2).
#define LEVEL3 ((g_gameALevel & 0xc0) == 0x80)

// Write a 4x4 block of tile numbers to the BG map at dst. The tiles start
// at $80 + face * 16.
static void writeTileBlock4x4(uint8_t *dst, uint8_t face)
{
    uint8_t tile = (face << 4) + 0x80;
    for (uint8_t row = 4; row; row--, dst += 0x1c)
        for (uint8_t n = 4; n; n--)
            *dst++ = tile++;
}

// Draw the options menu: the menu text and the two commander faces. P1
// has face 3. P2 has face 0 (face 4 in link play). In link play the
// other Game Boy (no $ff8a bit 6) has the faces swapped. Then set the
// cursor sprites, the default options, and go to state $1c.
void drawMenuTilemap(void)
{
    fillBytes((uint8_t *)0x9800, 0, 0x240);
    DrawTilemapCmds(GameA_OptionsMenuTilemap);
    uint8_t p1 = 3, p2 = 0;
    if (g_topModeFlags & 0x80) {
        p2 = 4;
        if (!(g_topModeFlags & 0x40)) {
            p1 = 4;
            p2 = 3;
        }
    }
    g_p1Commander = p1;
    g_p2Commander = p2;
    writeTileBlock4x4((uint8_t *)0x9961, p1);
    writeTileBlock4x4((uint8_t *)0x996f, p2);
    uint8_t *s = OAM;
    s[0] = 0x18;
    s[1] = 0x2c;
    s[2] = 0x6d;
    s[3] = 0x10;
    if (g_topModeFlags & 0x80) {
        s[4] = 0x18;
        s[5] = 0x7c;
        s[6] = 0x6d;
        s[7] = 0x30;
        g_gameFlags8 |= 0x02;
    }
    g_gameAState = 0x1c;
    g_gameABlinkTimer = 0x18;
    g_gameFlags2 = (g_gameFlags2 & ~0x04) | 0x01;
    BGM_Queue = 0x01;
    g_p1Options = (g_p1Options & 0xe0) | 0x1c;
    g_p2Options = (g_p2Options & 0xe0) | 0x1c;
    g_gameALevel |= 0x08;
    // Hide the 10 option marker sprites.
    s = OAM + 0x08;
    uint8_t n = 10;
    do {
        *s = 0x89;
        s += 4;
    } while (--n);
    g_linkRecvC9d1 = 0;
    (&g_linkRecvC9d1)[1] = 0;
    if (!(g_topModeFlags & 0x80))
        g_p2Options |= 0x40;
}

// Draw the victory scene (state $15 step 0, with the LCD off). One anim
// script of $17 sprites plays the scene. Then fade in and play song 7.
void drawVictoryScene(void)
{
    g_gameFlags6 &= ~0x30;
    ClearShadowOAM();
    fillBytes((uint8_t *)0x9840, 0, 0x1c0);
    DrawTilemapCmds(GameA_VictorySceneTilemap);
    g_animFirstOamSlot = 0;
    g_animSpritesPerScript = 0x17;
    g_animScriptCount = 1;
    WORD(g_animScriptPtrs) = (uint16_t)(LEVEL3 ? GameA_VictorySceneAnimLevel3 : GameA_VictorySceneAnim);
    g_gameFlags6 |= 0x40;
    g_gameATurnTimer = 0;
    g_gameAState15Step = 1;
    g_fadeTargetObp0 = 0x27;
    g_gameFlags2 |= 0x01;
    BGM_Queue = 0x07;
}

// Game A state $15 step 1: the victory scene. $ffb1 counts the frames
// up to $90 and sets BGP at the fade steps. When the anim script ends
// ($ffae bit 6 clear), show the victory sprites and go to state $16.
void state15Step1(void)
{
    uint8_t t = g_gameATurnTimer;
    if (t < 0x90)
        g_gameATurnTimer = ++t;
    if (t == 0x01)
        rOBP0 = 0x27;
    if (t == 0x58)
        NoiseSfx_Queue = 0x0f;
    // 4 pairs: the frame, then the BGP value.
    const uint8_t *f = GameA_VictoryBgpFadeSteps;
    for (uint8_t n = 4; n; n--, f += 2) {
        if (f[0] == t) {
            rBGP = f[1];
            break;
        }
    }
    if (g_gameFlags6 & 0x40)
        return;
    g_gameADelay = 0x20;
    buildSpriteStrips(GameA_VictorySpriteLayout, 0x1012, OAM);
    g_gameAFxTimer = 0x08;
    g_gameAFxSteps = 0x08;
    g_gameAState = 0x16;
    if (!(g_topModeFlags & 0x80))
        return;
    g_serialSend = 0x12;
    g_gameALinkState = 0x0f;
}

// Game A state $14: the enemy fleet screen. When the last row was
// copied, draw the next row ($ffb1). Row 0 also sets up the sprites.
// Row $12 ends it: go to state $0f, or state $0a for a new turn.
void state14(void)
{
    if (g_vramCopyCount)
        return;
    rSCY = 0;
    rSCX = 0;
    uint8_t t = g_gameATurnTimer;
    if (!t) {
        ClearShadowOAM();
        showTargetCursor();
        buildGridEdgeStrip();
        drawCoordLabels();
        buildLevelSprites();
        g_gameFlags5 &= ~0x40;
    } else if (t == 0x12) {
        g_gameATurnTimer = 0;
        if ((g_gameFlags9 & 0x40) || (g_gameALevel & 0x10)) {
            g_gameAState = 0x0f;
            g_gameATurnTimer = 0;
            g_listCount = 0x07;
            return;
        }
        g_gameAState = 0x0a;
        g_gameFlags6 |= 0x10;
        return;
    }
    tickRedrawTimers();
    if (g_gameFlags3 & 0x04)
        return;
    g_currentPlayer = 0;
    drawStreamToBgMap(&g_playerFleetGrid);
    g_currentPlayer = 1;
    // The grid rows are rows first to last - 1 of the map. A cell the
    // CPU hit (2 in $c5b4) shows tile $b8.
    uint8_t first = 9 - (g_gridSize >> 1);
    uint8_t last = 0x12 - first;
    t = g_gameATurnTimer;
    if (t >= first && t < last) {
        uint8_t *q = &g_vramQueue + (uint8_t)(first - 1);
        const uint8_t *shot = GRID_CELL(&g_cpuShotGrid, t - first, 0);
        for (uint8_t n = g_gridSize; n; n--, q++)
            if (*shot++ == 2)
                *q = 0xb8;
    }
    if (t == 0x11) {
        writeTileRuns(&g_vramQueue, GameA_FleetEnemyHeaderRuns);
        uint8_t left = countNonZero5(g_p2ShipHitsLeft);
        if (left)
            (&g_vramQueue)[0x12] = left + 0xd8;
        else
            writeTileRuns(&g_vramQueue, GameA_NoEnemyTextRuns);
    }
    g_vramCopyCount++;
}

// Queue the next tile row ($ffb1) of the player fleet grid at $9800.
// Rows 2 to b - 1 are the grid; each cell is 2 tile rows ($ffab bit 5
// picks the half). Returns 1 while rows are left (row < $20).
static uint8_t drawFleetGridRows(void)
{
    QueueTilemapRowCopy(0x9800);
    uint8_t t = g_gameATurnTimer;
    // b = the map column (and row) after the grid.
    uint8_t b = (g_gridSize + 1) << 1;
    uint8_t inGrid = t >= 2 && t < b;
    const uint8_t *cell = &g_playerFleetGrid;
    if (t >= 2)
        cell += multiply8((t - 2) >> 1, g_gridSize);
    uint8_t *q = &g_vramQueue;
    for (uint8_t c = 0; c != 0x20; c += 2, q += 2) {
        getShipPartTile(inGrid && c >= 2 && c < b ? *cell++ : 0x27, q);
        // The right border.
        if (inGrid && c == b)
            *q = 0x01;
    }
    q = &g_vramQueue;
    if (inGrid) {
        q[1] = 0x01;
    } else if (t == 1 || t == b) {
        // The top and bottom border.
        fillBytes(q + 2, 0x01, (g_gridSize << 1) + 1);
        q[1] = 0x01;
    }
    g_gameFlags3 ^= 0x20;
    g_vramCopyCount++;
    return ++g_gameATurnTimer < 0x20;
}

// Game A state $11: the player fleet after the battle. When the last row
// was copied, draw the next row. After the last row, go to state $1d: a
// new turn ($ffab bit 4) or the game over.
void state11(void)
{
    if (g_vramCopyCount)
        return;
    if (drawFleetGridRows())
        return;
    if (g_gameFlags3 & 0x10) {
        if (g_topModeFlags & 0x80)
            g_gameFlags3 &= ~0x10;
        g_gameATurnTimer = 0x01;
        g_gameFlags5 |= 0x10;
    } else {
        ClearShadowOAM();
        g_currentPlayer = 0;
        uint8_t y = ((g_gridSize - 8) << 3) + 8;
        g_boardScroll[0] = y;
        g_boardScroll[1] = y - 8;
        if (beginGameOver())
            return;
        rSCY = g_boardScroll[0];
        rSCX = g_boardScroll[1];
        rLCDC &= ~0x08;
        g_gameATurnTimer = 0x18;
    }
    g_gameAState = 0x1d;
}

// Game A state $19: a sunk ship turns over. Count 5 starts a ship tile
// from GameA_GfxUiTiles + $3b0, count 4 a tile from GameA_GfxTiles6050 +
// $270. Count 4 also draws a cross on the tile. Wait while a VRAM update
// is queued ($ffab bit 2). After the last tile go to state $0d.
void state19(void)
{
    if (g_gameFlags3 & 0x04)
        return;
    uint8_t n = g_shipTileCount;
    uint8_t part = (g_shipCellId & 0x3f) - 0x13;
    if (n == 5) {
        uint16_t off = (uint16_t)part << 4;
        WORD(g_shipTileSrc[0]) = (uint16_t)GameA_GfxUiTiles + 0x3b0 + off;
        WORD(g_shipTileDst[0]) = 0x8bb0 + off;
    } else if (n == 4) {
        uint16_t off = (uint16_t)part << 6;
        WORD(g_shipTileSrc[0]) = (uint16_t)GameA_GfxTiles6050 + 0x270 + off;
        WORD(g_shipTileDst[0]) = 0x9020 + off;
    }
    uint8_t done = rotateShipTileStep();
    if (g_shipTileCount == 4) {
        // The tile is in g_vramQueue + 3 (8 rows of 2 bytes).
        uint8_t *q = &g_vramQueue + 3;
        const uint8_t *m = GameA_SunkCrossTileMask;
        uint8_t i = 8;
        do {
            *q++ &= *m++;
            *q++ |= *m++;
        } while (--i);
    }
    g_gameFlags3 |= 0x04;
    if (done)
        g_gameAState = 0x0d;
}

// Board step 1: the "you lost" sprite at OAM $28 moves up. At Y 8 show
// the "YOU LOST" sprites and go to step 2 (the flash). OBP0 depends on
// the level.
void boardStep1(void)
{
    uint8_t *y = OAM + 0x28;
    if (*y == 0x48)
        NoiseSfx_Queue = 0x03;
    if (--*y != 0x08)
        return;
    buildSpriteStrips(GameA_YouLostSpriteLayout, 0x0008, OAM);
    // The asm also wrote sound $0b here, but $03 replaced it at once.
    g_gameABoardStep = 2;
    g_gameAFxTimer = 0x08;
    g_gameAFxSteps = 0x48;
    g_gameATurnTimer = 0x20;
    g_gameADelay = 0x40;
    SFX_Queue = 0x03;
    g_sfx04Busy = 0;
    uint8_t level = g_gameALevel & 0xc0;
    SET_HRAM(rOBP0, !level ? 0xd8 : level == 0x40 ? 0x36 : 0xc6);
}
