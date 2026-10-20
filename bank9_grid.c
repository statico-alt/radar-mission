// Game A grid helpers that read no bank 1 data.
#include "bank1.h"

// Write one OAM entry with attributes $10.
static void setSprite(uint8_t *s, uint8_t y, uint8_t x, uint8_t tile)
{
    s[0] = y;
    s[1] = x;
    s[2] = tile;
    s[3] = 0x10;
}

// The entry of the current ship in GameA_ShipSpriteTable (bank 1):
// low byte = tile count, high byte = first tile.
static uint16_t shipSpriteEntry(void)
{
    return peek1w(GameA_ShipSpriteTable + (uint8_t)(g_selectedShip << 1));
}

// Move the grid pointer p one cell in direction dir:
// 0 = down, 1 = up, 2 = right, 3 = left.
uint8_t *gridStep(uint8_t *p, uint8_t dir)
{
    if (dir == 0)
        return p + g_gridSize;
    if (dir == 1)
        return p - g_gridSize;
    if (dir == 2)
        return p + 1;
    return p - 1;
}

// Set the cursor bounds $ffbf..$ffc2 (row end, row start, column end,
// column start) so the current ship fits the grid in direction dir.
void calcPlacementBounds(uint8_t dir) __banked
{
    g_placeBounds[0] = g_placeBounds[2] = g_gridSize;
    g_placeBounds[1] = g_placeBounds[3] = 0;
    uint8_t extra = SHIP_LENGTHS[g_selectedShip] - 1;
    if (!(dir & 0x01))
        extra = -extra;
    g_placeBounds[dir] += extra;
}

// Returns 1 when the cursor is outside the bounds at $ffbf.
uint8_t checkCursorInBounds(void) __banked
{
    return g_cursorRow >= g_placeBounds[0] || g_cursorRow < g_placeBounds[1]
        || g_cursorCol >= g_placeBounds[2] || g_cursorCol < g_placeBounds[3];
}

// Check that all cells under the current ship at the cursor are empty.
// Returns 1 when a cell is used.
uint8_t checkShipFits(uint8_t *grid, uint8_t dir) __banked
{
    uint8_t *p = GRID_CELL(grid, g_cursorRow, g_cursorCol);
    for (uint8_t n = SHIP_LENGTHS[g_selectedShip]; n; n--) {
        if (*p)
            return 1;
        p = gridStep(p, dir);
    }
    return 0;
}

// Write the current ship into the grid at the cursor, one cell per ship
// part. The cell ids of ship n start after those of ships 0..n-1. The
// direction goes in bits 6-7.
void writeShipToGrid(uint8_t *grid) __banked
{
    uint8_t *p = GRID_CELL(grid, g_cursorRow, g_cursorCol);
    const uint8_t *len = SHIP_LENGTHS;
    uint8_t id = 1;
    for (uint8_t i = 0; i < g_selectedShip; i++)
        id += len[i];
    id |= g_placeDirection << 6;
    for (uint8_t n = len[g_selectedShip]; n; n--) {
        *p = id++;
        p = gridStep(p, g_placeDirection);
    }
}

// The ship index for a grid cell id (low 6 bits of id).
// The id after the last ship gives 5. A larger id (a hit cell) reads
// past the 5 lengths, as in the asm.
uint8_t getShipFromCellId(uint8_t id) __banked
{
    id &= 0x3f;
    if (id == SumFleetBytes())
        return 5;
    const uint8_t *len = SHIP_LENGTHS;
    uint8_t end = 1, i = 0;
    for (;; i++) {
        end += len[i];
        if (id < end)
            return i;
    }
}

// Draw the current ship ($ffbe) at the cursor, facing direction $ffc8.
void drawShipSprites(void) __banked
{
    uint16_t e = shipSpriteEntry();
    uint8_t y = CELL_OAM_Y(g_cursorRow), x = CELL_OAM_X(g_cursorCol);
    uint8_t step = (g_placeDirection & 0x01) ? -8 : 8;
    uint8_t tile = e >> 8;
    uint8_t *s = OAM;
    for (uint8_t n = e; n; n--, s += 4) {
        setSprite(s, y, x, tile++);
        if (g_placeDirection & 0x02)
            x += step;
        else
            y += step;
    }
    g_gameAPlaceState = 2;
}

// Set the tile copy source ($ffc4/$ffc5) and VRAM target ($ffc6/$ffc7)
// for the graphics of the current ship ($ffbe).
void setShipTileSource(void) __banked
{
    uint16_t e = shipSpriteEntry();
    g_shipTileCount = e;
    uint16_t offset = (uint16_t)(uint8_t)((e >> 8) - 0x80) << 4;
    *(uint16_t *)g_shipTileSrc = (uint16_t)GameA_GfxUiTiles + offset;
    *(uint16_t *)g_shipTileDst = 0x8800 + offset;
}

// The BG map address of a grid cell. The grid origin depends on
// $ff8d bits 0-1.
uint16_t gridCellToBgAddr(uint8_t row, uint8_t col)
{
    uint8_t size = g_p1Options & 0x03;
    uint16_t base = size == 0 ? 0x9884 : size == 1 ? 0x9863 : 0x9842;
    return base + ((uint16_t)row << 5) + col;
}

// For direction 1 (up) or 3 (left), the first cell of a ship of length
// len that starts at *row/*col. Other directions keep the cell.
void adjustShipOrigin(uint8_t dir, uint8_t len, uint8_t *row, uint8_t *col)
{
    if (dir == 1)
        *row = *row + 1 - len;
    else if (dir == 3)
        *col = *col + 1 - len;
}

// Queue the ship tiles of the placed ship in g_vramQueue as one tilemap
// command (rst $28 format): a row or a column of tiles.
void placeGetShipSprite(void) __banked
{
    uint16_t e = shipSpriteEntry();
    uint8_t count = e, tile = e >> 8;
    int8_t step = 1;
    if (g_placeDirection & 0x01) {
        tile += count - 1;
        step = -1;
    }
    uint8_t *q = &g_vramQueue;
    for (uint8_t i = 0; i < count; i++, tile += step)
        q[3 + i] = tile;
    q[3 + count] = 0;
    // Mode 2 (column) when the ship goes down or up.
    q[2] = ((g_placeDirection & 0x02) ? 0x00 : 0x80) | count;
    uint8_t row = g_cursorRow, col = g_cursorCol;
    adjustShipOrigin(g_placeDirection, count, &row, &col);
    uint16_t addr = gridCellToBgAddr(row, col);
    q[0] = addr >> 8;
    q[1] = addr;
}


// Clear the cells of a picked-up ship from g_playerFleetGrid and queue
// tile $b5 over it in g_vramQueue. The ship has len cells; its first
// cell id is first.
void pickUpShipCells(uint8_t len, uint8_t first) __banked
{
    uint8_t dir = g_placeDirection;
    // Find the first cell of the ship, then clear all its cells.
    uint8_t *p = &g_playerFleetGrid;
    while ((*p & 0x3f) != first)
        p++;
    uint8_t *q = p;
    for (uint8_t n = len; n; n--) {
        *q = 0;
        q = gridStep(q, dir);
    }
    // The cell of p: row = offset / size, column = offset mod size.
    uint16_t offset = p - &g_playerFleetGrid;
    uint8_t row = 0;
    while (offset >= g_gridSize) {
        offset -= g_gridSize;
        row++;
    }
    uint8_t col = offset;
    adjustShipOrigin(dir, len, &row, &col);
    uint16_t addr = gridCellToBgAddr(row, col);
    uint8_t *cmd = &g_vramQueue;
    cmd[0] = addr >> 8;
    cmd[1] = addr;
    cmd[2] = len | 0x40 | ((dir & 0x02) ? 0 : 0x80);
    cmd[3] = 0xb5;
    cmd[4] = 0;
}
