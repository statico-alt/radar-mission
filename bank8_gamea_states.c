// Game A states $0e-$1e that load screens, state $13, and the fleet
// panel (was GameA_State0E_LevelTitle .. GameA_State1E_BoardLoad,
// GameA_State13_BattleEnd, GameA_LoadCursorTiles and
// GameA_DrawFleetPanel in bank_000.asm).
#include "bank8_gamea.h"

#define VRAM(a) ((uint8_t *)(a))

// Load the cursor tile and set the cursor anim scripts.
void gameALoadCursorTiles(void)
{
    GA_COPY(2, 0x8ce0, GameA_GfxSprites + 0x6b0, 0x10);
    uint8_t *s = RAM(g_animScriptPtrs);
    s[0] = 0xbe;
    s[1] = 0x6d;
    s[2] = 0xda;
    s[3] = 0x6d;
    g_shipTileSrc[0] = 0xf6;
    g_shipTileSrc[1] = 0x6d;
    g_scriptSpriteSlot = 0;
}

// Draw the fleet panel: the sunk ship tiles of the other side, the
// lucky square markers, the grid rows and the turn count.
void gameADrawFleetPanel(void)
{
    storeCpuShipDirs();
    uint8_t lvl = g_gameALevel & 0xc0;
    gaDrawShipRecords(GameA_VictorySpriteLayout + (lvl == 0 ? 0x0f : lvl == 0x40 ? 0x2d : 0x4b));
    g_currentPlayer = 0x01;
    uint8_t mask[16];
    farCopy(1, mask, GameA_SunkCrossTileMask, 16);
    uint8_t c = multiply8(g_gridSize, g_gridSize);
    uint8_t b = SumFleetBytes() + 1;
    uint8_t *cell = RAM(g_playerShotGrid);
    do {
        if (*cell == 0x01) {
            uint8_t v = cell[0x1b0];  // the CPU fleet grid
            g_placeDirection = v >> 6;
            uint8_t i = v & 0x3f;
            if (i >= b)
                i = i + 1 - b;
            uint16_t off = (uint16_t)(uint8_t)(i - 1) << 4;
            const uint8_t *src = GameA_GfxUiTiles + 0x3b0;
            lvl = g_gameALevel & 0xc0;
            if (lvl == 0x40) {
                if (i >= 0x0f)
                    src = GameA_GfxUiTiles + 0x3e0;
                else if (i >= 0x0c)
                    src = GameA_GfxUiTiles + 0x3c0;
                else if (i >= 0x09)
                    src = GameA_GfxUiTiles + 0x3f0;
                else if (i >= 0x06)
                    src = GameA_GfxUiTiles + 0x420;
            } else if (lvl == 0x80) {
                src = i >= 0x0e ? GameA_GfxUiTiles + 0x3f0 : GameA_GfxAltModeTiles;
            }
            uint16_t *t = (uint16_t *)g_shipTileSrc;
            t[0] = (uint16_t)src + off;
            t[1] = 0x8bb0 + off;
            g_shipTileCount = 0x01;
            rotateShipTileStep();
            uint8_t *q = RAM(g_vramQueue) + 3;
            for (uint8_t k = 0; k < 16; k += 2) {
                q[k] &= mask[k];
                q[k + 1] |= mask[k + 1];
            }
            DrawTilemapScript(RAM(g_vramQueue));
        }
        cell++;
    } while (--c);

    g_gameATurnTimer = 0;
    g_gameFlags5 &= ~0x40;
    for (c = 0; c < 8; c++) {
        uint8_t v = g_luckySquaresP1[c];
        if (v != 0xff)
            *GRID_CELL(RAM(g_cpuFleetGrid), v >> 4, v & 0x0f) = 0x27;
    }
    do {
        gaDrawStreamToBgMap(RAM(g_cpuFleetGrid));
        b = 9 - (g_gridSize >> 1);
        c = 0x12 - b;
        uint8_t t = g_gameATurnTimer;
        if (t >= b && t < c) {
            uint8_t *q = RAM(g_vramQueue) + (uint8_t)(b - 1);
            const uint8_t *s = GRID_CELL(RAM(g_playerShotGrid), t - b, 0);
            for (uint8_t n = g_gridSize; n; n--, q++)
                if (*s++ == 0x02)
                    *q = 0xb8;
        }
        g_vramCopyCount++;
        gaRunCopyQueue();
    } while (g_gameATurnTimer != 0x12);

    farCopy(1, VRAM(0x9a0b), GameA_TurnCounterRuns + 1, 4);
    uint8_t n = GA_LEVEL3 ? g_gameATotalTurns : g_turnCount;
    uint8_t h = 0, t = 0;
    while (n >= 100) {
        n -= 100;
        h++;
    }
    while (n >= 10) {
        n -= 10;
        t++;
    }
    VRAM(0x9a10)[0] = h + 0xd8;
    VRAM(0x9a10)[1] = t + 0xd8;
    VRAM(0x9a10)[2] = n + 0xd8;
}

// Game A state $13: end of the battle. Pick the result music, then go
// to state $14.
void gameAState13BattleEnd(void)
{
    g_gameAWindowY = rWY - 6;
    g_gameFlags8 |= 0x08;
    if (g_gameAEndTimer == 0x18) {
        SFX_Queue = 0x0f;
        g_p2Options &= ~0x80;
        rOBP0 = 0xc6;
        rOBP1 = 0x36;
    }
    if (--g_gameAEndTimer)
        return;
    rSCY = 0;
    rSCX = 0;
    g_gameATurnTimer = 0;
    g_musicCh4Flags &= ~0x80;
    rLCDC = (rLCDC | 0x08) & ~0x20;
    g_gameAState = 0x14;
    if (g_gameFlags4 & 0x03) {
        BGM_Queue = 0x0c;
        g_gameALevel &= ~0x02;
    } else if ((g_gameFlags9 & 0x03) || (g_gameFlags5 & 0x10)) {
        BGM_Queue = 0x11;
        g_gameALevel |= 0x02;
        if ((g_gameFlags3 & 0x40) && !(g_gameFlags4 & 0x40))
            BGM_Queue = 0x1c;
        else if (!(g_gameFlags9 & 0x40) && (g_gameFlags9 & 0x20))
            BGM_Queue = 0x13;
    }
    g_gameFlags9 &= 0xfc;
    g_gameFlags5 &= ~0x10;
}

// State $0e step 0: load the level title tiles and draw the title.
static void gaLevelTitleLoad(void)
{
    LcdOffMaskIrqs();
    GA_COPY(2, 0x8920, GameA_GfxUiTiles + 0x120, 0x3a0);
    GA_COPY(2, 0x89f0, GameA_GfxTiles9F, 0x60);
    if (GA_LEVEL3) {
        GA_COPY(2, 0x8bb0, GameA_GfxAltModeTiles, 0x90);
        GA_COPY(5, 0x9670, NextLevelBgTiles + 0x510, 0x60);
    }
    levelTitleScreenInit();
    LcdOnEnableIrqs();
}

// State $12 step 0: draw both sea grids and the sprites.
static void gaBoardLoad(void)
{
    LcdOffMaskIrqs();
    g_gameFlags6 &= ~0x30;
    g_gameFlags4 &= 0xdc;
    setSeaMapPtr();
    do {
        drawGridRow();
        gaRunCopyQueue();
    } while (g_gameATurnTimer < 0x12);
    copyBytes(VRAM(0x9c00), VRAM(0x9920), 0x120);
    uint8_t lvl = g_gameALevel;
    g_gameALevel = lvl & 0x3f;
    setSeaMapPtr();
    g_gameFlags5 |= 0x20;
    g_gameATurnTimer = 0x09;
    do {
        drawGridRow();
        gaRunCopyQueue();
    } while (g_gameATurnTimer < 0x12);
    g_gameALevel = lvl;
    ClearShadowOAM();
    uint8_t *p = OAM;
    for (uint8_t c = 10; c; c--, p += 4)
        *p = 0x48;
    gaSpriteStrips(GameA_TallyAnimC + 0x51, 0x1001, p);
    rWY = 0x48;
    rWX = 0x07;
    g_gameFlags2 |= 0x01;
    rSCY = 0;
    rSCX = 0;
    g_gameABoardStep = 1;
    g_gameFlags5 &= ~0x40;
    BGM_Queue = 0x0a;
    LcdOnEnableIrqs();
    rLCDC |= 0x20;
}

// State $15 step 0: load the victory tiles and draw the scene.
static void gaState15Load(void)
{
    LcdOffMaskIrqs();
    GA_COPY(2, 0x8920, GameA_GfxUiTiles + 0x120, 0x140);
    if (GA_LEVEL3)
        GA_COPY(4, 0x8010, GameA_VictorySpritesLevel3Gfx, 0x630);
    else
        GA_COPY(2, 0x8010, GameA_GfxTiles6050 + 0x6f0, 0x630);
    drawVictoryScene();
    LcdOnEnableIrqs();
}

// State $17 step 0: load the results tiles and draw the results.
static void gaResultsLoad(void)
{
    LcdOffMaskIrqs();
    GA_COPY(2, 0x9020, GameA_GfxStatusTiles, 0x2c0);
    GA_COPY(2, 0x9010, GameA_GfxBgTiles + 0x10, 0x10);
    GA_COPY(2, 0x8920, GameA_GfxUiTiles + 0x120, 0x140);
    GA_COPY(2, 0x8ce0, GameA_GfxSprites + 0x6b0, 0x10);
    GA_COPY(2, 0x8860, GameA_GfxSprites + 0x640, 0x20);
    // The tiles at $9300 have their low bit plane inverted.
    GA_COPY(2, 0x9300, GameA_GfxUiTiles + 0x670, 0x180);
    uint8_t *p = VRAM(0x9300);
    for (uint8_t c = 0xc0; c; c--, p += 2)
        *p = ~*p;
    GA_COPY(5, 0x8800, Title_ExtraTilesGfx + 0xa0, 0x60);
    resultsScreenInit();
    gameADrawFleetPanel();
    farCopy(1, VRAM(0x9a01), GameA_FleetEnemyHeaderRuns + 0x13, 8);
    farCopy(1, g_nextLevelShipTiming, GameA_NextLevelShipTimingInit, 10);
    LcdOnEnableIrqs();
    rLCDC |= 0x08;
}

// State $1b: load the options screen tiles and draw the menu.
static void gaOptionsLoad(void)
{
    LcdOffMaskIrqs();
    if (g_topModeFlags & 0x80)
        g_gameALinkState = 0x13;
    GA_COPY(2, 0x9000, GameA_GfxBgTiles, 0x800);
    GA_COPY(2, 0x8fe0, GameA_GfxBgTiles, 0x10);
    GA_COPY(2, 0x94d0, GameA_GfxPlayerEnemyTiles, 0x80);
    GA_COPY(2, 0x9490, GameA_GfxUiTiles + 0x230, 0x10);
    GA_COPY(5, 0x8800, CommanderTiles0, 0x100);
    GA_COPY(5, 0x8900, CommanderTiles1, 0x100);
    GA_COPY(5, 0x8a00, CommanderTiles2, 0x100);
    GA_COPY(5, 0x8b00, CommanderTiles3, 0x100);
    GA_COPY(5, 0x8c00, CommanderTiles4, 0x100);
    drawMenuTilemap();
    gaPadCycles(30);
    LcdOnEnableIrqs();
}

// State $1e: load the board tiles for the level, draw the 7 ship
// records and set up the board screen.
void gaBoardLoad1E(void)
{
    LcdOffMaskIrqs();
    GA_COPY(2, 0x94e0, GameA_GfxBgTiles + 0x4e0, 0x320);
    GA_COPY(2, 0x8800, GameA_GfxUiTiles, 0x2a0);
    GA_COPY(2, 0x8d00, GameA_GfxUiTiles + 0x500, 0x140);
    GA_COPY(2, 0x89f0, GameA_GfxTiles9F, 0x60);
    GA_COPY(2, 0x8000, GameA_GfxSprites, 0x800);
    GA_COPY(2, 0x8620, GameA_GfxUiTiles + 0x4d0, 0x10);
    uint8_t lvl = g_gameALevel & 0xc0;
    if (lvl == 0x40) {
        GA_COPY(5, 0x8a30, NextLevelBgTiles + 0x350, 0x20);
    } else if (lvl == 0x80) {
        GA_COPY(5, 0x94d0, NextLevelBgTiles + 0x370, 0x330);
        GA_COPY(5, 0x8800, NextLevelBgTiles + 0x6a0, 0x250);
        GA_COPY(2, 0x8030, GameA_GfxShipSinkSprites, 0x310);
    }
    fillBytes(RAM(g_animScriptCount), 0, 7);
    gaLoadShipTiles();
    initBoardScreen();
    gaPadCycles(174);
    LcdOnEnableIrqs();
    if ((g_topModeFlags & 0x81) == 0x81)
        rLCDC |= 0x08;
}

// States $0e, $0f, $12, $15-$18, $1a-$1c and $1e.
void gameAStates0E(void)
{
    switch (g_gameAState) {
    case 0x0e:
        if (g_gameATitleStep == 0)
            gaLevelTitleLoad();
        else
            levelTitleStep1();
        break;
    case 0x0f:
        tickRedrawTimers();
        if (!(g_gameFlags3 & 0x04))
            blinkEnemyCount();
        break;
    case 0x12:
        switch (g_gameABoardStep) {
        case 0: gaBoardLoad(); break;
        case 1: boardStep1(); break;
        case 2: boardStep2(); break;
        }
        break;
    case 0x15:
        switch (g_gameAState15Step) {
        case 0: gaState15Load(); break;
        case 1: state15Step1(); break;
        case 2: state15Step2(); break;
        }
        break;
    case 0x16:
        if (g_gameAState16Step == 0)
            state16Step0();
        else
            state16Step1();
        break;
    case 0x17:
        if (g_joyPressedA & 0x06) {
            rLCDC ^= 0x0a;
            SFX_Queue = 0x08;
        }
        switch (g_gameAResultsStep) {
        case 0: gaResultsLoad(); break;
        case 1: resultsStep1(); break;
        case 2: resultsStep2Reset(); break;
        }
        break;
    case 0x18: nextLevelScreen(); break;
    case 0x1a:
        switch (g_gameAState1AStep) {
        case 0: state1AStep0(); break;
        case 1: state1AStep1(); break;
        case 2: state1AStep2(); break;
        case 3: state1AStep3(); break;
        case 4: state1AStep4(); break;
        case 5: state1AStep5(); break;
        }
        break;
    case 0x1b: gaOptionsLoad(); break;
    case 0x1c: optionsMenuUpdate(); break;
    case 0x1e: gaBoardLoad1E(); break;
    }
}
