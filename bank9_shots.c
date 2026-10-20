// Game A shots: the shot pattern, Lucky Shot squares, touching ships and
// the CPU ship placement. They read bank 1 tables through peek1w.
#include "bank1.h"

// A packed cell: row in the high nibble, column in the low nibble.
#define PACK(row, col) ((uint8_t)(((row) << 4) | (col)))

// The number of nonzero bytes in the 5 bytes at p.
uint8_t countNonZero5(const uint8_t *p) __banked
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < 5; i++)
        if (p[i])
            n++;
    return n;
}

// Move the cell *row, *col one step in direction dir (DirectionDeltas).
// Returns 1 when the new cell is outside the grid.
// The column is not changed when the row is already outside.
static uint8_t getNeighborCell(uint8_t dir, uint8_t *row, uint8_t *col)
{
    uint16_t d = peek1w(DirectionDeltas + (uint8_t)(dir << 1));
    *row += (uint8_t)d;
    if (*row >= g_gridSize)
        return 1;
    *col += (uint8_t)(d >> 8);
    return *col >= g_gridSize;
}

// Move the cell *row, *col one step in direction dir:
// 0 = up, 1 = down, 2 = left, 3 = right.
// Returns 1 when the cell leaves the grid.
static uint8_t stepCellInDirection(uint8_t dir, uint8_t *row, uint8_t *col)
{
    if (dir == 0)
        return (*row)-- == 0;
    if (dir == 1)
        return ++*row >= g_gridSize;
    if (dir == 2)
        return (*col)-- == 0;
    return ++*col >= g_gridSize;
}

// stepCellInDirection for the asm glue: cell = row << 8 | col.
// Returns the new cell in the same form.
uint16_t stepCellInDirectionAsm(uint8_t dir, uint16_t cell) __banked
{
    uint8_t row = cell >> 8, col = cell;
    stepCellInDirection(dir, &row, &col);
    return (row << 8) | col;
}

// Collect the cells of the ship at row, col and of all ships that touch
// it into the list at $c1bf (2 bytes each: packed cell, ship index;
// $ffb2 = count). Bit 7 of the ship index marks a cell that is not hit.
// AI hit-list entries of hit cells get state 8. Then each cell not hit
// goes to the salvo list at $c97c.
void collectTouchingShips(uint8_t *grid, uint8_t row, uint8_t col) __banked
{
    WORD(g_tmpE8) = (uint16_t)grid;
    g_gameATurnTimer = 0;
    g_listCount = 1;
    uint8_t *list = &g_cellList;
    uint8_t total = SumFleetBytes();
    list[0] = PACK(row, col);
    list[1] = getShipFromCellId(*GRID_CELL(grid, row, col) - total) | 0x80;
    uint8_t *entry = list;
    do {
        for (uint8_t dir = 0; dir < 4; dir++) {
            uint8_t r = entry[0] >> 4, c = entry[0] & 0x0f;
            if (getNeighborCell(dir, &r, &c))
                continue;
            uint8_t v = *GRID_CELL(grid, r, c) & 0x3f;
            if (!v || v == total)
                continue;
            uint8_t cell = PACK(r, c);
            uint8_t notHit = 0x80;
            if (v > total) {
                // A hit cell. Mark its AI hit-list entry (sorted by cell).
                notHit = 0;
                if (!(g_topModeFlags & 0x80) && !g_currentPlayer && g_aiHitCount) {
                    uint8_t *h = &g_aiHitList + 1;
                    while (cell > *h)
                        h += 2;
                    if (cell == *h)
                        h[-1] = 0x08;
                }
            }
            // Add the cell when it is not in the list yet.
            uint8_t *p = list;
            uint8_t n = g_listCount;
            for (; n; n--, p += 2)
                if (*p == cell)
                    break;
            if (n)
                continue;
            p[0] = cell;
            p[1] = getShipFromCellId(v) | notHit;
            g_listCount++;
        }
        entry += 2;
        SET_HRAM(g_gameATurnTimer, g_gameATurnTimer + 1);
    } while (g_listCount != g_gameATurnTimer);

    uint8_t *shot = &g_salvoShots;
    entry = list;
    for (uint8_t n = g_listCount; n; n--, entry += 2) {
        if (entry[1] & 0x80) {
            shot[0] = entry[0];
            shot[1] = entry[1] & 0x7f;
            shot += 2;
            g_salvoCount++;
        }
    }
}

// Check whether the packed cell is one of the Lucky Shot squares of player
// ($c950 for player 1, $c958 for player 2; 2 groups of 4). A found
// square takes 1 from the counter of its group ($c94c, 2 per player).
// Returns 1 (group 1), 2 (group 2) or 0. $ffb1 counts the groups.
uint8_t checkLuckySquare(uint8_t player, uint8_t cell) __banked
{
    uint8_t *sq = player ? g_luckySquaresP2 : g_luckySquaresP1;
    uint8_t *counter = &g_luckyCounters + (player ? 2 : 0);
    g_gameATurnTimer = 2;
    uint8_t group = 1;
    do {
        for (uint8_t i = 0; i < 4; i++, sq++) {
            if (*sq == cell) {
                (*counter)--;
                return group;
            }
        }
        group <<= 1;
        counter++;
        SET_HRAM(g_gameATurnTimer, g_gameATurnTimer - 1);
    } while (g_gameATurnTimer);
    return 0;
}

// Apply the shot pattern (GameA_TargetPatternOffsets) at the cursor.
// For each new cell in the grid it marks g_playerShotGrid (1 = hit,
// 2 = miss) and writes 2 bytes to the list at $c966: flags and a value.
// Flags: bits 2-3 Lucky Shot group, bit 1 near miss, bit 4 ship sunk,
// bit 7 hit. The value is ship * 2 for a hit, else a splash slot * 2 + 1.
// $c963 = number of new cells.
void resolveShots(void) __banked
{
    const uint8_t *off = GameA_TargetPatternOffsets;
    uint8_t *out = &g_shotCells;
    uint8_t total = SumFleetBytes();
    g_listCount = 0;
    g_tmpE8 = 0;
    g_tmpEA = 0;
    for (uint8_t n = TARGET_CELL_COUNT; n; n--, off += 2) {
        g_tmpE9 = 0;
        uint16_t d = peek1w(off);
        uint8_t row = g_cursorRow + (uint8_t)d;
        uint8_t col = g_cursorCol + (uint8_t)(d >> 8);
        if (row >= g_gridSize || col >= g_gridSize)
            continue;
        // Only the first Lucky Shot square of the pattern counts.
        if (!g_tmpEA) {
            uint8_t group = checkLuckySquare(0, PACK(row, col));
            if (group) {
                g_tmpE9 |= group << 2;
                g_tmpEA++;
            }
        }
        uint8_t *shot = GRID_CELL(&g_playerShotGrid, row, col);
        if (*shot)
            continue;
        g_tmpE8++;
        uint8_t *cell = GRID_CELL(&g_cpuFleetGrid, row, col);
        uint8_t v = *cell;
        uint8_t result = 2;
        if (!v) {
            // A miss. A ship that is not hit next to it is a near miss.
            for (uint8_t dir = 0; dir < 4; dir++) {
                uint8_t r = row, c = col;
                if (stepCellInDirection(dir, &r, &c))
                    continue;
                uint8_t w = *GRID_CELL(&g_cpuFleetGrid, r, c) & 0x3f;
                if (w && w < total)
                    g_tmpE9 |= 0x02;
            }
        } else {
            // A hit. The cell value goes up by total.
            *cell = v + total;
            result = 1;
            if ((v & 0x3f) == total)
                g_tmpE9 |= 0x20;
            uint8_t ship = getShipFromCellId(v);
            g_gameATurnTimer = ship;
            if (!--g_p2ShipHitsLeft[ship] && !(g_tmpE9 & 0x20))
                g_tmpE9 |= 0x10;
        }
        *shot = result;
        uint8_t value;
        if (!(g_tmpE9 & 0x20) && result == 1) {
            value = g_gameATurnTimer << 1;
            g_tmpE9 |= 0x80;
        } else {
            value = g_listCount;
            if (value >= 4)
                value -= 4;
            g_listCount++;
            value = (value << 1) + 1;
        }
        out[0] = value;
        out[-1] = g_tmpE9;
        out += 2;
    }
    g_salvoResultCount = g_tmpE8;
}

// Pick count random empty cells for player and store them packed in
// the list at sq. A cell must have no ship, must not be shot, and must
// not be in the 16 bytes of the player's square table yet.
// The squares of player 0 are on the CPU grid (the grid player 0 shoots).
static void placeRandomLuckySquares(uint8_t *sq, uint8_t player, uint8_t count)
{
    uint8_t *fleet = player ? &g_playerFleetGrid : &g_cpuFleetGrid;
    uint8_t *shots = player ? &g_cpuShotGrid : &g_playerShotGrid;
    uint8_t *table = player ? g_luckySquaresP2 : g_luckySquaresP1;
    for (; count; count--) {
        uint8_t cell;
        for (;;) {
            uint8_t r = stirRandom();
            uint8_t row = r >> 4, col = r & 0x0f;
            if (row >= g_gridSize)
                row -= g_gridSize;
            if (col >= g_gridSize)
                col -= g_gridSize;
            if (*GRID_CELL(fleet, row, col) || *GRID_CELL(shots, row, col))
                continue;
            cell = PACK(row, col);
            uint8_t i = 0;
            for (; i < 16; i++)
                if (table[i] == cell)
                    break;
            if (i == 16)
                break;
        }
        *sq++ = cell;
    }
}

// Fill the Lucky Shot square table $c950 (16 bytes) with $ff, then place
// new squares for each player with the Lucky Shot option (bit 4 of
// $ff8d / $ff8e). In link mode it first starts sending them (state $11).
void initLuckySquares(void) __banked
{
    if (g_topModeFlags & 0x80) {
        g_linkByteIndex = 0;
        g_serialSend = 0x10;
        g_gameALinkState = 0x11;
    }
    uint8_t *counters = &g_luckyCounters;
    for (uint8_t i = 0; i < 16; i++)
        g_luckySquaresP1[i] = 0xff;
    if (g_p1Options & 0x10) {
        placeRandomLuckySquares(g_luckySquaresP1, 0, counters[0]);
        placeRandomLuckySquares(g_luckySquaresP1 + 4, 0, counters[1]);
    }
    if (g_p2Options & 0x10) {
        placeRandomLuckySquares(g_luckySquaresP2, 1, counters[2]);
        placeRandomLuckySquares(g_luckySquaresP2 + 4, 1, counters[3]);
    }
}

// Write the cells of the salvo pattern (5 or 9 cells, by the grid
// option) around row, col to the list at $c0bf ($ffea = write pointer).
// It keeps the cells in the grid that are 0 in the grid at $ffe8.
// Returns the number of cells.
static uint8_t buildSalvoPattern(uint8_t row, uint8_t col)
{
    uint8_t n = (g_p1Options & 0x03) == 2 ? 9 : 5;
    const uint8_t *off = GameA_TargetPatternOffsets;
    uint8_t count = 0;
    WORD(g_tmpEA) = (uint16_t)&g_vramQueue;
    for (; n; n--, off += 2) {
        uint16_t d = peek1w(off);
        uint8_t r = row + (uint8_t)d, c = col + (uint8_t)(d >> 8);
        if (r >= g_gridSize || c >= g_gridSize)
            continue;
        if (*GRID_CELL((uint8_t *)WORD(g_tmpE8), r, c))
            continue;
        *(uint8_t *)WORD(g_tmpEA) = PACK(r, c);
        WORD(g_tmpEA)++;
        count++;
    }
    return count;
}

// Set $c92c: bit 2 when the target cell ($ffdd, $ffde) is in the 4x4
// center of the grid. Else bit 0 when the row is in the top half, and
// bit 1 when the column is in the left half.
static void isTargetInCenter(void)
{
    uint8_t flags = 0;
    uint8_t lo = ((g_gridSize - 8) >> 1) + 2;
    uint8_t hi = lo + 4;
    if (g_targetRow >= lo && g_targetCol >= lo && g_targetRow < hi && g_targetCol < hi) {
        flags = 0x04;
    } else {
        uint8_t half = g_gridSize >> 1;
        if (g_targetRow < half)
            flags |= 0x01;
        if (g_targetCol < half)
            flags |= 0x02;
    }
    g_targetInCenter = flags;
}

// CPU turn step 6: fire at the target cell ($ffdd, $ffde).
// The Radar option ($ffac bit 3) hits all touching ships of a hit ship.
// The salvo option ($ffac bit 2) fires the salvo pattern.
// Then it starts the shot animation and the scroll toward the target.
void turn06FireAtTarget(void) __banked
{
    uint8_t row = g_targetRow, col = g_targetCol;
    uint8_t count = 1;
    if (g_gameFlags4 & 0x08) {
        uint8_t v = *GRID_CELL(&g_playerFleetGrid, row, col) & 0x3f;
        if (v && v != SumFleetBytes()) {
            collectTouchingShips(&g_playerFleetGrid, row, col);
            goto launch;
        }
        g_vramQueue = PACK(row, col);
    } else if (g_gameFlags4 & 0x04) {
        WORD(g_tmpE8) = (uint16_t)&g_cpuShotGrid;
        count = buildSalvoPattern(row, col);
    } else {
        g_vramQueue = PACK(row, col);
    }
    g_salvoCount = count;
    {
        // The count 0 copies 256 cells, as in the asm.
        uint8_t *shot = &g_salvoShots;
        const uint8_t *cell = &g_vramQueue;
        do {
            *shot = *cell++;
            shot += 2;
        } while (--count);
    }
launch:
    for (uint8_t i = 0; i < 0x58; i++)
        (&g_oamBuffer)[i] = 0;
    g_gameACpuState = 2;
    g_gameATurnTimer = 0xa0;
    g_salvoLaunchSfxTimer = 0x5c;
    g_salvoLaunchSfxCount = 0;
    g_gameFlags6 &= ~0x20;
    g_scrollDirSteps = 0;
    g_scrollXDirSteps = 0;
    isTargetInCenter();
    uint8_t c = g_targetInCenter;
    if (c & 0x04)
        return;
    uint8_t steps = ((g_gridSize - 8) << 3) + 8;
    g_scrollDirSteps = ((c & 0x01) << 7) | steps;
    g_scrollXDirSteps = ((c & 0x02) << 6) | steps;
}

// The cell of the grid pointer p in the grid at base:
// high byte = row, low byte = column.
static uint16_t gridPtrToCell(const uint8_t *p, const uint8_t *base)
{
    uint16_t offset = p - base;
    uint8_t row = 0;
    while (offset >= g_gridSize) {
        offset -= g_gridSize;
        row++;
    }
    return (row << 8) | (uint8_t)offset;
}

// CPU aim after a hit on the carrier (ship 0) of the grid at $c810.
// It collects the carrier cells (hit or not) at $c86a, then the open
// neighbor cells (not shot, no other ship) at $c816. It picks one at
// random and marks it in the grid with the value total and the
// direction bits. The cell goes to $c814/$c815 ($ff, $ff when none).
void processShipList(void) __banked
{
    const uint8_t *len = SHIP_LENGTHS;
    uint8_t total = SumFleetBytes();
    uint8_t idEnd = len[0] + 1;
    uint8_t hitStart = total + 1, hitEnd = hitStart + len[0];
    uint8_t *grid = (uint8_t *)WORD(g_activeFleetGridPtr);
    uint8_t *cells = &g_shipCellList;
    const uint8_t *p = grid;
    for (uint8_t found = 0; found + 1 != idEnd; found++) {
        uint8_t v;
        do {
            v = *p++ & 0x3f;
        } while (!v || (v >= idEnd && (v < hitStart || v >= hitEnd)));
        g_placeDirection = p[-1] & 0xc0;
        uint16_t cell = gridPtrToCell(p - 1, grid);
        cells[0] = cell >> 8;
        cells[1] = cell;
        cells += 2;
    }

    g_listCount = 0;
    WORD(g_tmpE8) = (uint16_t)&g_openNeighborCells;
    cells = &g_shipCellList;
    for (uint8_t n = len[0]; n; n--, cells += 2) {
        for (uint8_t dir = 0; dir < 4; dir++) {
            uint8_t r = cells[0], c = cells[1];
            if (getNeighborCell(dir, &r, &c))
                continue;
            if (*GRID_CELL((uint8_t *)WORD(g_activeShotGridPtr), r, c))
                continue;
            uint8_t *f = GRID_CELL(grid, r, c);
            uint8_t v = *f & 0x3f;
            if (v) {
                if (v != total)
                    continue;
                *f = 0;
            }
            uint8_t *out = (uint8_t *)WORD(g_tmpE8);
            out[0] = r;
            out[1] = c;
            WORD(g_tmpE8) += 2;
            g_listCount++;
        }
    }
    if (!g_listCount) {
        g_linkCellRow = 0xff;
        g_linkCellCol = 0xff;
        return;
    }
    uint8_t pick = stirRandom() & 0x3f;
    while (pick >= g_listCount)
        pick -= g_listCount;
    const uint8_t *open = &g_openNeighborCells + (uint8_t)(pick << 1);
    uint8_t row = open[0], col = open[1];
    *GRID_CELL(grid, row, col) = g_placeDirection | total;
    g_linkCellRow = row;
    g_linkCellCol = col;
}

// Place the fleet of player at random cells and directions.
// Player 0 uses g_playerFleetGrid and g_shipHitsLeft, player 1 uses
// g_cpuFleetGrid and $ffd7. A ship with 0 hits left is skipped. Index 5
// reads the byte after the hit table, as in the asm.
void loadPlayerFleet(uint8_t player) __banked
{
    g_currentPlayer = player;
    uint8_t *grid = player ? &g_cpuFleetGrid : &g_playerFleetGrid;
    const uint8_t *hits = player ? g_p2ShipHitsLeft : g_shipHitsLeft;
    for (g_selectedShip = 0; g_selectedShip != 6; g_selectedShip++) {
        if (!hits[g_selectedShip])
            continue;
        for (;;) {
            uint8_t r = stirRandom() >> 4;
            while (r >= g_gridSize)
                r -= g_gridSize;
            SET_HRAM(g_cursorRow, r);
            r = stirRandom() & 0x0f;
            while (r >= g_gridSize)
                r -= g_gridSize;
            SET_HRAM(g_cursorCol, r);
            SET_HRAM(g_placeDirection, (stirRandom() & 0x08) ? 2 : 0);
            // Try the 4 directions, from the random one.
            uint8_t tries = 4;
            for (;;) {
                calcPlacementBounds(g_placeDirection);
                if (!checkCursorInBounds() && !checkShipFits(grid, g_placeDirection)) {
                    writeShipToGrid(grid);
                    goto next;
                }
                if (!--tries)
                    break;
                SET_HRAM(g_placeDirection, (g_placeDirection + 1) & 0x03);
            }
        }
next:;
    }
}
