// Game A turn and state steps that read bank 1 data or call bank 1 asm.
#include "bank1.h"

// Bank 2 and 5 graphics (only the address is used).
extern const uint8_t GameA_GfxBgTiles[];
extern const uint8_t GameA_GfxSprites[];
extern const uint8_t GameA_GfxShipSinkSprites[];
extern const uint8_t NextLevelBgTiles[];

// True on level 3 (level bits 6-7 = 2).
#define LEVEL3 ((g_gameALevel & 0xc0) == 0x80)

// Write 20 rising tile numbers to dst. run[0] = the run length,
// run[1] = the first tile. The run starts again every run[0] tiles.
static void fillTileRun20(uint8_t *dst, const uint8_t *run)
{
    uint8_t i = 0;
    for (uint8_t n = 20; n; n--) {
        *dst++ = run[1] + i;
        if (++i == run[0])
            i = 0;
    }
}

// Queue the next row of the level map at $9800. The rows go up from row
// $c869. Step 0 picks the map stream (level 3 has its own). The row is
// 20 tiles of sea, then the map runs (up to step $32).
static void drawLevelMap(void)
{
    if (!g_levelMapStep)
        WORD(g_mapStreamPtr) = (uint16_t)(LEVEL3 ? GameA_LevelMapRuns + 4 : GameA_LevelMapStream);
    g_gameATurnTimer = g_levelMapRows;
    QueueTilemapRowCopy(0x9800);
    // The row is $14 tiles, not $20.
    (&g_vramCopyQueue)[(uint8_t)((g_vramCopyCount << 3) - g_vramCopyCount) + 4] = 0x14;
    fillTileRun20(&g_vramQueue, GameA_LevelMapRuns + ((g_levelMapStep & 1) << 1));
    if (g_levelMapStep < 0x32)
        unpackRunList(&g_vramQueue);
    g_vramCopyCount++;
    g_levelMapStep++;
    if (--g_levelMapRows == 0xff)
        g_levelMapRows = 0x1f;
}

// Split a packed cell byte: returns row (high nibble) << 8 | column (low nibble).
uint16_t splitCellByte(uint8_t v)
{
    return ((uint16_t)(v >> 4) << 8) | (v & 0x0f);
}

// Game A state $1a step 0: draw the level map. When it is done, start
// the "enemy aircraft launched" cutscene and the fade in.
void state1AStep0(void)
{
    if (g_vramCopyCount)
        return;
    drawLevelMap();
    if (g_levelMapStep < 0x13)
        return;
    rSCY = 0;
    rSCX = 0;
    g_scriptSpriteSlot = 0;
    *(uint16_t *)g_shipTileSrc = (uint16_t)GameA_CutsceneEnemyAircraftLaunched;
    g_gameFlags3 |= 0x80;
    g_gameAState1AStep = 4;
    g_fadeTargetObp0 = 0xe4;
    g_gameFlags2 |= 0x01;
    BGM_Queue = 0x09;
    ClearShadowOAM();
    g_gameALevel &= ~0x02;
}

// Game A state $1a step 3: the aircraft come down 1 line a frame. They
// slow down near Y $48. At Y $48 go to step 1.
void state1AStep3(void)
{
    uint8_t *s = OAM;
    for (uint8_t n = 0x27; n; n--, s += 4)
        (*s)--;
    g_gameADelay = 1;
    uint8_t y = *OAM;
    if (y == 0x48) {
        g_gameADelay = 0x40;
        g_gameATurnTimer = 0;
        g_gameAState1AStep = 1;
        return;
    }
    if (y < 0x51)
        g_gameADelay = GameA_AircraftDescentDelays[(uint8_t)(0x50 - y)];
}

// Game A state $1a step 1: the level map scrolls up 8 lines a frame
// while the rows are drawn. At step $50 queue the tiles of the cutscene
// and go to step 2.
void state1AStep1(void)
{
    if (g_vramCopyCount)
        return;
    rSCY -= 8;
    uint8_t step = g_levelMapStep;
    if (step == 0x13)
        NoiseSfx_Queue = 0x0b;
    if (step != 0x50) {
        drawLevelMap();
        return;
    }
    if (LEVEL3) {
        queueVramCopy(NextLevelBgTiles + 0x380, 0x94e0, 0x20, 0x19, 5);
        queueVramCopy(NextLevelBgTiles + 0x7b0, 0x8910, 0x20, 0x0a, 5);
    } else {
        queueVramCopy(GameA_GfxBgTiles + 0x4e0, 0x94e0, 0x20, 0x19, 2);
        queueVramCopy(GameA_GfxUiTiles, 0x8800, 0x20, 0x0a, 2);
    }
    queueVramCopy(GameA_GfxSprites + 0x30, 0x8030, 0x20, 0x24, 2);
    if (LEVEL3)
        queueVramCopy(GameA_GfxShipSinkSprites, 0x8030, 0x10, 0x31, 2);
    g_gameAState1AStep = 2;
}
