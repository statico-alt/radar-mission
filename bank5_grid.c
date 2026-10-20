// Game A battle grid: grid drawing, ship sprites, cell markers, small sprite
// effects and the battle setup.
#include "game.h"

extern const uint8_t ShipLengthAndFirstCell[];
extern const uint8_t ShipCellPieceType[];
extern const uint8_t ShipPieceSpriteTiles[];
extern const uint8_t GridRowTileRuns[];
extern const uint8_t GridLevel2RowTiles[];
extern const uint8_t GridHeaderTemplate[];
extern const uint8_t SeaMap_Level1and2[];
extern const uint8_t SeaMap_Level3[];
extern const uint8_t OBP1FlashCycle[];
extern const uint8_t ShipHitPointsByLevel[];
extern const uint8_t GridParamsBySize[];
extern const uint8_t GridParamsByLevel[];
extern const uint8_t *const AiSearchPatternPtrs[];

// bank 0
void QueueTilemapRowCopy(uint16_t tilemap);
// bank_005.asm stubs for bank 0 routines
// bank5_screens.c

#define FLEET_GRID (&g_playerFleetGrid)
#define OAM (&g_oamBuffer)
#define ROW_BUFFER (&g_vramQueue)

// A ship cell id without its direction bits. Ids $13 and up are the same
// ships as ids 1 and up.
static uint8_t shipId(uint8_t cell)
{
    cell &= 0x3f;
    if (cell >= 0x13)
        cell -= 0x12;
    return cell;
}

// Convert grid cell row, col to a screen position: Y << 8 | X in OAM coordinates.
// g_targetInCenter bits 0-1 select the corner of the 2x2 sprite. Bit 2 = centre.
uint16_t gridCellToScreen(uint8_t row, uint8_t col)
{
    uint8_t border = (uint8_t)(g_gridSize - 8) << 3;
    uint8_t y = (row << 4) + 0x10 + (uint8_t)(0x08 - border);
    uint8_t x = (col << 4) + 0x08 + (uint8_t)(0x10 - border);
    uint8_t corner = g_targetInCenter;
    if (!(corner & 0x04)) {
        border += 8;
        y += corner & 0x01 ? border : -border;
        x += corner & 0x02 ? border : -border;
    }
    return (uint16_t)y << 8 | x;
}

// Set Y/X of the four OAM entries at oam as a 2x2 block with step 8.
static void setSprite2x2Pos(uint8_t *oam, uint16_t pos)
{
    uint8_t y = pos >> 8;
    uint8_t x = pos;
    oam[0] = y;
    oam[1] = x;
    oam[4] = y;
    oam[5] = x + 8;
    oam[8] = y + 8;
    oam[9] = x;
    oam[12] = y + 8;
    oam[13] = x + 8;
}

// Put a marker sprite on every cell listed in g_shipCellPos (18 entries, $ff = none).
void drawShipCellMarkers(void)
{
    uint8_t *oam = OAM + 0x58;
    const uint8_t *cell = g_shipCellPos;
    for (uint8_t n = 18; n; n--, cell++) {
        if (*cell == 0xff)
            continue;
        uint16_t pos = gridCellToScreen(*cell >> 4, *cell & 0x0f) + 0x0404;
        *oam++ = pos >> 8;
        *oam++ = pos;
        *oam++ = 0xba;
        *oam++ = 0x10;
    }
}

// Show ship number g_shipNumToShow as sprites over its cells on the grid.
// Uses ShipLengthAndFirstCell to find the ship in g_playerFleetGrid, then
// builds one 2x2 sprite per cell and marks those cells in g_shipCellPos as gone.
void showShipSprite(void)
{
    const uint8_t *ship = ShipLengthAndFirstCell + (uint8_t)(g_shipNumToShow << 1);
    g_shownShipLength = ship[0];
    // Find the first cell of the ship. The ship runs back from it.
    const uint8_t *cell = FLEET_GRID;
    uint8_t col = 0;
    while (shipId(*cell) != ship[1]) {
        cell++;
        col++;
    }
    g_placeDirection = *cell >> 6;
    uint8_t row = 0;
    while (col >= g_gridSize) {
        col -= g_gridSize;
        row++;
    }
    uint8_t *oam = OAM;
    uint8_t n = g_shownShipLength;
    do {
        setSprite2x2Pos(oam, gridCellToScreen(row, col));
        uint8_t id = shipId(FLEET_GRID[multiply8(row, g_gridSize) + col]) - 1;
        g_shipCellPos[id] = 0xff;
        const uint8_t *tiles = ShipPieceSpriteTiles + (uint8_t)(ShipCellPieceType[id] << 5)
                               + (uint8_t)(g_placeDirection << 3);
        for (uint8_t i = 2; i < 16; i += 4) {
            oam[i] = *tiles++;
            oam[i + 1] = *tiles++;
        }
        // Step back along the ship (the opposite direction).
        switch (g_placeDirection) {
        case 0: row++; break;
        case 1: row--; break;
        case 2: col++; break;
        default: col--; break;
        }
        oam += 16;
    } while (--n);
    fillBytes(OAM + 0x58, 0, 0x30);
    drawShipCellMarkers();
    g_gameATurnTimer = 2;
    g_listCount = 0x20;
    NoiseSfx_Queue = 6;
    g_gameACpuState = 0x0d;
    g_sfxBlockNoise06 = 0;
}

// Fill the 32-byte row buffer for level 2 grid art.
static void fillGridRowLevel2(uint8_t *row, uint8_t y)
{
    fillBytes(row, 0, 0x20);
    if (y < 4)
        return;
    if (y == 4) {
        row[2] = 0xa8;
        row[7] = 0xa8;
        row[9] = 0xa8;
        row[13] = 0xa8;
    } else if (y == 5) {
        copyBytes(row, GridLevel2RowTiles, 0x13);
    } else {
        fillBytes(row, 0xa6, 0x20);
    }
}

// Write $14 tiles to row. run = (run length, first tile).
// Each run counts up from the first tile, then repeats.
static void fillRowTileRun(uint8_t *row, const uint8_t *run)
{
    uint8_t tile = run[1];
    uint8_t left = run[0];
    for (uint8_t n = 0x14; n; n--) {
        *row++ = tile++;
        if (!--left) {
            left = run[0];
            tile = run[1];
        }
    }
}

// Build one tile row of the battle grid in the VRAM queue.
// g_gameATurnTimer is the row counter. The glue returns Z when all $12 rows are done.
void drawGridRow(void)
{
    uint8_t *row = ROW_BUFFER;
    uint8_t y = g_gameATurnTimer;
    uint8_t level = g_gameALevel & 0xc0;
    QueueTilemapRowCopy(0x9800);
    if (y < 7) {
        if (level == 0x40) {
            fillGridRowLevel2(row, y);
        } else if (level == 0x80) {
            fillBytes(row, 0x01, 0x20);
            unpackRunList(row);
        } else {
            fillBytes(row, 0x92, 0x20);
        }
        if (y == 0) {
            copyBytes(row + 0x0c, GridHeaderTemplate, 7);
            drawNumber3(row + 0x11, g_turnCount);
        }
        // Row 6: a marker for each ship of player 2 that is still afloat.
        if (y == 6 && !(g_gameFlags4 & 0x20)) {
            if (!(g_gameFlags4 & 0x03)) {
                row[10] = 0xa3;
                row[11] = 0xa4;
            } else {
                row += 3;
                for (uint8_t i = 0; i < 5; i++, row += 3) {
                    if (g_p2ShipHitsLeft[i]) {
                        row[0] = 0xa3;
                        row[1] = 0xa4;
                    }
                }
            }
        }
    } else {
        fillRowTileRun(row, GridRowTileRuns + (uint8_t)((y - 7) << 1));
        if (level == 0x80 ? y == 7 || y >= 0x0a : !(g_gameFlags5 & 0x20) && y >= 9)
            unpackRunList(row);
    }
    g_vramCopyCount = 1;
    g_gameATurnTimer++;
}

// Point g_mapStreamPtr at the sea background map for the current level.
void setSeaMapPtr(void)
{
    WORD(g_mapStreamPtr) = (uint16_t)((g_gameALevel & 0xc0) == 0x80 ? SeaMap_Level3 : SeaMap_Level1and2);
}

// In link play, change OBP1 colour 1 every 16 frames (6-step cycle).
void cyclePaletteOBP1(void)
{
    if (!(g_p2Options & 0x80))
        return;
    if (--g_obp1CycleTimer)
        return;
    g_obp1CycleTimer = 0x10;
    uint8_t *step = &g_obp1CycleTimer + 1;
    uint8_t i = *step;
    if (++*step == 6)
        *step = 0;
    rOBP1 = (rOBP1 & 0xf3) | OBP1FlashCycle[i];
}

// Every 8 frames, flip the palette bit of the ship sprites so they blink.
void blinkShipSprites(void)
{
    if (g_gameFlags5 & 0x01)
        return;
    if (--g_shipSpriteBlinkTimer)
        return;
    g_shipSpriteBlinkTimer = 8;
    uint8_t *attr = OAM + 3;
    for (uint8_t n = 5; n; n--, attr += 4)
        *attr ^= 0x10;
    // The selected ship: 2 sprites, or 1 for ships 4 and up.
    uint8_t n = g_selectedShip < 4 ? 2 : 1;
    attr = OAM + 0x7b + (uint8_t)(g_selectedShip << 3);
    for (; n; n--, attr += 4)
        *attr ^= 0x10;
}

// Set up a new battle from the options: grid size g_gridSize (8/10/12),
// ship hit points for the level, and the AI search pattern for the CPU.
void setupBattle(void)
{
    g_shipSpriteBlinkTimer = 8;
    fillBytes(g_shipCellPos, 0xff, 0x12);
    uint8_t size = g_p1Options & 0x03;
    g_gridSize = (size << 1) + 8;
    uint8_t centre = g_gridSize >> 1;
    g_savedCursorRow = centre;
    g_savedCursorCol = centre;
    (&g_cpuCursorRow)[0] = centre;
    (&g_cpuCursorRow)[1] = centre;
    g_aiRunCellsLeft = 1;
    uint8_t level = g_gameALevel >> 6;
    const uint8_t *params = level ? GridParamsByLevel + (level << 2) : GridParamsBySize + (size << 2);
    copyBytes(&g_luckyCounters, params, 4);
    const uint8_t *hitPoints = ShipHitPointsByLevel + (level << 2) + level;
    copyBytes(g_p2ShipLengths, hitPoints, 5);
    copyBytes(g_p2ShipHitsLeft, hitPoints, 5);
    copyBytes(g_p1ShipLengths, ShipHitPointsByLevel, 5);
    if (!level)
        copyBytes(g_shipHitsLeft, ShipHitPointsByLevel, 5);
    // The CPU search pattern (not in link play, not for commander type 2).
    if (g_topModeFlags & 0x80 || g_p2Commander == 2)
        return;
    const uint8_t *pattern = AiSearchPatternPtrs[(uint8_t)(g_p2Commander << 1) + g_p2Commander + size];
    uint8_t n = g_aiRunsLeft = pattern[0];
    WORD(g_aiPatternPtr) = (uint16_t)(pattern + 1);
    uint8_t *offset = &g_aiRunOffsets;
    uint8_t value = 0;
    do {
        *offset++ = value;
        value += 3;
    } while (--n);
}
