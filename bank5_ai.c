// The parts of the CPU AI that read bank 5 data, and the Game A random numbers.
// The rest of the AI is in bank13_ai.c. It calls these through __banked.
#include "game.h"

extern const uint8_t ShipCellIdLimits[];
extern const uint8_t NeighborOffsets[];

// Return a pseudo-random byte, mixed from rDIV into g_gameARandomSeed.
uint8_t random(void) __banked
{
    uint8_t div = rDIV;
    return g_gameARandomSeed += (uint8_t)((div << 2) | (div >> 6));
}

// A step nibble: bit 3 is the sign, bits 0-2 the size.
static uint8_t signedStep(uint8_t nibble)
{
    if (nibble & 0x08)
        return -(nibble & 0x07);
    return nibble;
}

// Get the next cell of the AI search pattern into g_targetRow/Col.
// When a run is used up, pick a random unused run: its start cell, length and
// step (signed nibbles) are expanded into g_aiRunCells.
// With no runs left, set g_gameFlags9 bit 3 and do nothing more.
void aiNextPatternCell(void) __banked
{
    if (--g_aiRunCellsLeft == 0) {
        g_aiRunCellIndex = 0;
        uint8_t pick = random() & 0x3f;
        uint8_t runs = g_aiRunsLeft;
        if (!runs) {
            g_gameFlags9 |= 0x08;
            return;
        }
        while (pick >= runs)
            pick -= runs;
        uint8_t *offsets = &g_aiRunOffsets;
        const uint8_t *run = (const uint8_t *)WORD(g_aiPatternPtr) + offsets[pick];
        uint8_t row = run[0] >> 4;
        uint8_t col = run[0] & 0x0f;
        uint8_t n = g_aiRunCellsLeft = run[1];
        g_tmpE8 = signedStep(run[2] >> 4);
        g_tmpE9 = signedStep(run[2] & 0x0f);
        uint8_t *cell = &g_aiRunCells;
        do {
            *cell++ = row;
            *cell++ = col;
            row += g_tmpE8;
            col += g_tmpE9;
        } while (--n);
        // Remove the run from the list of unused runs.
        for (n = runs - pick; n; n--, pick++)
            offsets[pick] = offsets[pick + 1];
        g_aiRunsLeft--;
    }
    uint8_t *cell = &g_aiRunCells + (uint8_t)(g_aiRunCellIndex << 1);
    g_aiRunCellIndex++;
    g_targetRow = cell[0];
    g_targetCol = cell[1];
}

// Grid cell value -> ship index. Returns 1 when that ship has 1 hit left
// (this hit sinks it).
uint8_t isLastHitOnShip(uint8_t cell) __banked
{
    uint8_t id = cell & 0x3f;
    uint8_t ship = 0;
    while (id >= ShipCellIdLimits[ship])
        ship++;
    return g_shipHitsLeft[ship] == 1;
}

// Count the unshot cells around row, col. n = number of offsets to test
// from NeighborOffsets (the first one is the cell itself).
uint8_t countOpenNeighbors(uint8_t row, uint8_t col, uint8_t n) __banked
{
    const uint8_t *offset = NeighborOffsets;
    uint8_t last = g_gridSize - 1;
    uint8_t open = 0;
    do {
        uint8_t r = row + offset[0];
        uint8_t c = col + offset[1];
        if (r <= last && c <= last && !(&g_cpuShotGrid)[multiply8(r, g_gridSize) + c])
            open++;
        offset += 2;
    } while (--n);
    return open;
}
