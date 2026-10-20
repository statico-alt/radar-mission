// Game A board screens: scrolling, the arrange screen and the board rows.
// These read bank 1 tables or call bank 0 routines that need bank 1.
#include "bank1.h"

// Queue a tile strip in g_vramQueue: the BG map address (high, low),
// the mode byte, 16 copies of the tile pair, then 0.
static void queueTileStrip(uint16_t addr, uint8_t mode, const uint8_t *pair)
{
    uint8_t *q = &g_vramQueue;
    q[0] = addr >> 8;
    q[1] = addr;
    q[2] = mode;
    q += 3;
    for (uint8_t n = 16; n; n--) {
        *q++ = pair[0];
        *q++ = pair[1];
    }
    *q = 0;
    g_gameFlags3 |= 0x04;
}

// Move a scroll register one pixel. steps: bit 7 = direction,
// bits 0-6 = steps left. Returns the sprite delta: -1 when the register
// goes up, 1 when it goes down, 0 when no steps are left.
static int8_t scrollStep(uint8_t *steps, volatile uint8_t *reg)
{
    uint8_t v = *steps;
    if (!(v & 0x7f))
        return 0;
    *steps = v - 1;
    if (v & 0x80) {
        (*reg)--;
        return 1;
    }
    (*reg)++;
    return -1;
}

// The index (0-7) of v in a list of 8 scroll triggers, or 8.
// The list is two groups of 4 values, 8 apart.
static uint8_t findTrigger(const uint8_t *list, uint8_t v)
{
    uint8_t i = 0;
    while (i < 8 && list[i] != v)
        i++;
    return i;
}

// Move the 18 sprites at $c058 by dy and dx. This is asm in
// bank_001.asm: it runs every frame in the VBlank, and the C loop is too
// slow (the palette write after it lands 2 lines late).
void moveScrollSprites(uint8_t dy, uint8_t dx);

// Adds $c0 to the sub-step counter $c92d. Runs the rest only when it
// overflows (3 of 4 frames). Scrolls SCY and SCX one pixel with the
// direction and step count in $c927/$c928. At a trigger value it queues
// a strip of edge tiles. The sprites at $c058 move with the scroll.
void updateScroll(void)
{
    uint8_t sub = g_scrollSubStep;
    g_scrollSubStep = sub + 0xc0;
    if (sub < 0x40)
        return;
    // No steps left: skip the scroll steps. This keeps the time close to
    // the original.
    if (!((g_scrollDirSteps | g_scrollXDirSteps) & 0x7f)) {
        g_tmpE8 = g_tmpE9 = 0;
        moveScrollSprites(0, 0);
        return;
    }
    int8_t dy = scrollStep(&g_scrollDirSteps, &rSCY);
    g_tmpE8 = dy;
    if (dy) {
        uint8_t i = findTrigger(GameA_ScrollYRowTriggers, rSCY);
        if (i < 8)
            queueTileStrip((i < 4 ? 0x9820 : 0x9ae0) + ((i & 3) << 5), 0x20,
                           GameA_ScrollRowEdgeTiles + ((i & 1) ? 0 : 2));
    }
    int8_t dx = scrollStep(&g_scrollXDirSteps, &rSCX);
    g_tmpE9 = dx;
    if (dx) {
        uint8_t i = findTrigger(GameA_ScrollXColTriggers, rSCX);
        if (i < 8)
            queueTileStrip((i < 4 ? 0x9801 : 0x9817) + (i & 3), 0xa0,
                           GameA_ScrollColEdgeTiles + ((i & 1) ? 0 : 2));
    }
    moveScrollSprites(dy, dx);
}

// Write the two tiles of one tile row of a ship cell to out. id is the
// grid cell id; bits 6-7 are the direction. $ffab bit 5 selects the
// lower tile row.
void getShipPartTile(uint8_t id, uint8_t *out)
{
    uint8_t dir = id >> 6;
    const uint8_t *t = GameA_ShipPartTiles + (uint8_t)((dir << 1) + dir);
    uint8_t n = id & 0x3f, tile, rowStep = 2, colStep = 1;
    if (n == 0x27) {
        tile = 0xb1;
    } else {
        // A hit cell has the id + $12.
        if (n >= 0x13)
            n -= 0x12;
        if (!n) {
            tile = 0xad;
        } else {
            tile = ((n - 1) << 2) + 2 + t[0];
            rowStep = t[1];
            colStep = t[2];
        }
    }
    if (g_gameFlags3 & 0x20)
        tile += rowStep;
    out[0] = tile;
    out[1] = tile + colStep;
}

// Queue the 2x2 tiles of a ship cell of the large board (2 tiles per
// cell, at $9842). id is the grid cell id. cell = row << 8 | column.
void queueShipCellTiles(uint8_t id, uint16_t cell)
{
    uint16_t addr = 0x9842 + ((uint16_t)(uint8_t)(cell >> 8) << 6) + (uint8_t)(cell << 1);
    uint8_t *q = &g_vramQueue;
    g_gameFlags3 &= ~0x20;
    q[0] = addr >> 8;
    q[1] = addr;
    q[2] = 2;
    getShipPartTile(id, q + 3);
    g_gameFlags3 |= 0x20;
    addr += 0x20;
    q[5] = addr >> 8;
    q[6] = addr;
    q[7] = 2;
    getShipPartTile(id, q + 8);
    q[10] = 0;
    g_gameFlags3 |= 0x04;
}

// Move the 4 sprites at $c000 2 pixels in the direction in bits 6-7
// of $c92b (DirectionDeltas).
void nudgeCellSprites(void)
{
    const int8_t *d = (const int8_t *)DirectionDeltas + (uint8_t)((g_shipDirBits >> 6) << 1);
    uint8_t *s = OAM;
    for (uint8_t n = 4; n; n--, s += 4) {
        s[0] += d[0] << 1;
        s[1] += d[1] << 1;
    }
}

// Build the grid edge strip in $c1bf (sized by the grid) and draw it at
// $c024 with BuildSpriteStrips. Then load the 3 redraw timers from
// GameA_RedrawTimerPresets.
void buildGridEdgeStrip(void)
{
    uint8_t margin = 12 - g_gridSize;
    uint8_t *p = &g_cellList;
    *p++ = (margin << 2) + 0x20;
    *p++ = 0;
    *p++ = g_gridSize | 0x80;
    for (uint8_t n = g_gridSize >> 1; n; n--) {
        *p++ = 1;
        *p++ = 2;
    }
    buildSpriteStrips(&g_cellList, 0x1000 | g_gridSize, OAM + 0x24);
    copyBytes(&g_redrawTimers, GameA_RedrawTimerPresets + (uint8_t)((margin << 1) + margin), 6);
}

// Game A state $1a step 4: draw the enemy aircraft sprites, go to step 3.
void state1AStep4(void)
{
    buildSpriteStrips(GameA_EnemyAircraftSpriteLayout, 0x1027, OAM);
    g_gameAState1AStep = 3;
    NoiseSfx_Queue = 0x0a;
}

// Game A state $1f. Waits for the fade to end, then mutes the music and
// goes to state $0e (3 in link mode). Until then it blinks "START" at
// $9a07 every $18 frames.
void state1FWaitFade(void)
{
    if (!g_soundFadeState) {
        g_gameAState = 0x0e;
        g_gameFlags2 |= 0x02;
        BGM_Queue = 0xff;
        if (g_topModeFlags & 0x80)
            g_gameAState = 0x03;
        return;
    }
    if (--g_gameABlinkTimer)
        return;
    g_gameABlinkTimer = 0x18;
    uint8_t *q = &g_vramQueue;
    q[0] = 0x9a;
    q[1] = 0x07;
    q[2] = 0x05;
    fillBytes(q + 3, 0, 6);
    if (!(g_gameABlinkStep ^= 0x01))
        copyBytes(q + 3, GameA_StartText, 5);
    g_gameFlags3 |= 0x04;
}

// Draw the arrange screen (the player fleet grid) one row per frame
// until all 18 rows are drawn. Then copy it to $9c00 and draw the
// arrange window.
void drawArrangeScreen(void)
{
    do {
        drawStreamToBgMap(&g_playerFleetGrid);
        if (g_gameATurnTimer == 0x11) {
            writeTileRuns(&g_vramQueue, GameA_ArrangeTextRuns);
            writeTileRuns(&g_vramQueue, (g_topModeFlags & 0x80) ? GameA_VersusTextRuns : GameA_VsComTextRuns);
        }
        g_vramCopyCount++;
        VBlank_RunCopyQueue();
    } while (g_gameATurnTimer != 0x12);
    copyBytes((uint8_t *)0x9c00, (const uint8_t *)0x9800, 0x240);
    DrawTilemapCmds(GameA_ArrangeFleetWindowTilemap);
    g_gameABlinkTimer = 0x18;
    g_gameABlinkStep = 0;
    g_gameFlags2 |= 0x01;
    if (g_topModeFlags & 0x80)
        BGM_Queue = 0x03;
}

// Draw row $ffb1 of the board screen to $9800 ($9c00 when $ffad bit 6
// is set) and add 1 to $ffb1. The rows come from the run lists at
// g_mapStreamPtr. The grid rows show the column letters, the row digit
// and the ships of grid (not with $ffad bit 6).
void drawStreamToBgMap(const uint8_t *grid)
{
    if (!g_gameATurnTimer)
        WORD(g_mapStreamPtr) = (uint16_t)GameA_BoardRowRuns;
    QueueTilemapRowCopy((g_gameFlags5 & 0x40) ? 0x9c00 : 0x9800);
    uint8_t *q = &g_vramQueue;
    uint8_t row = g_gameATurnTimer;
    if (row == 0 || row >= 0x0e) {
        unpackRunList(q);
    } else {
        fillBytes(q, 0, 0x14);
        writeTileRuns(q, GameA_BoardSideBorderRuns);
    }
    row = ++g_gameATurnTimer;
    uint8_t first = (0x0f - g_gridSize) >> 1;
    uint8_t *p = q + first;
    first++;
    if (row < first)
        return;
    if (row == first) {
        // The column letters A, B, C...
        p++;
        for (uint8_t i = 0; i < g_gridSize; i++)
            *p++ = 0xe7 + i;
        return;
    }
    if (row >= 0x11 - first)
        return;
    row -= first;
    *p++ = 0xd8 + row;
    grid += multiply8(row - 1, g_gridSize);
    fillBytes(p, 0xb5, g_gridSize);
    if (g_gameFlags5 & 0x40)
        return;
    for (uint8_t n = g_gridSize; n; n--, p++) {
        uint8_t id = *grid++ & 0x3f;
        if (!id)
            continue;
        if (id == 0x27) {
            *p = 0xce;
            continue;
        }
        // A hit cell has the id + the fleet sum.
        uint8_t total = SumFleetBytes();
        if (id > total)
            id -= total;
        *p = 0xbb + id - 1;
    }
}

// Write tile runs to dst. Each run is an offset byte, then tiles.
// $fe starts a new run. $ff ends the data.
void writeTileRuns(uint8_t *dst, const uint8_t *src)
{
    for (;;) {
        uint8_t *p = dst + *src++;
        for (;;) {
            uint8_t v = *src++;
            if (v == 0xff)
                return;
            if (v == 0xfe)
                break;
            *p++ = v;
        }
    }
}

// PixelToBgMapAddr: the $9800 map address of the pixel at $ff9a (y),
// $ff9b (x), in $ff9c (high) and $ff9d (low). Here the pixel is a
// multiple of 8, and y >= 16, x >= 8.
static void pixelToBgMapAddr(void)
{
    uint16_t addr = 0x9800 + ((uint16_t)(uint8_t)(g_pixelY - 0x10) << 2)
                  + (uint8_t)((uint8_t)(g_pixelX - 8) >> 3);
    g_bgMapAddrHi = addr >> 8;
    g_bgMapAddrLo = addr;
}

// Run the 3 redraw timer pairs at $ffd1 (delay, X). After the delay, X
// counts down. Every 8 steps it redraws the board column under X: from
// g_playerShotGrid for timers 1 and 2, blank for timer 3. Skipped while
// a VRAM update is queued ($ffab bit 2).
void tickRedrawTimers(void)
{
    if (g_gameFlags3 & 0x04)
        return;
    uint8_t *t = &g_redrawTimers;
    uint8_t *q = &g_vramQueue;
    for (uint8_t n = 3; n; n--, t += 2) {
        if (t[0]) {
            t[0]--;
            continue;
        }
        uint8_t x = --t[1];
        if (x & 0x07)
            continue;
        fillBytes(q + 3, 0xb5, 12);
        if (n != 1) {
            const uint8_t *tiles = GameA_ShotMarkerTiles + (n == 3 ? 0 : 3);
            const uint8_t *cell = &g_playerShotGrid + (uint8_t)((x >> 3) - ((12 - g_gridSize) >> 1) - 3);
            uint8_t *p = q + 3;
            for (uint8_t i = g_gridSize; i; i--, cell += g_gridSize)
                *p++ = tiles[*cell];
        }
        uint8_t top = (14 - g_gridSize) << 2;
        g_pixelY = top + 0x18;
        g_pixelX = x;
        pixelToBgMapAddr();
        q[0] = g_bgMapAddrHi + 4;
        q[1] = g_bgMapAddrLo;
        q[2] = g_gridSize | 0x80;
        q[3 + g_gridSize] = 0;
        g_gameFlags3 |= 0x04;
        // At the left edge the timer starts again from the right.
        if (x == top + 0x10) {
            t[1] = 0x80 - top;
            t[0] = 0x40;
            uint8_t *s = OAM + 0x25;
            for (uint8_t i = g_gridSize; i; i--, s += 4)
                *s = 0;
        }
    }
    if (t[-6])
        return;
    // The edge strip sprites follow timer 1.
    uint8_t x = t[-5];
    uint8_t *s = OAM + 0x25;
    for (uint8_t i = g_gridSize; i; i--, s += 4)
        *s = x;
    if (x != ((g_gridSize - 8) << 2) + 0x47)
        return;
    if ((g_gameFlags3 & 0x40) || (g_gameFlags9 & 0x20) || (g_gameFlags4 & 0x03))
        return;
    SFX_Queue = 0x0a;
}

// The salvo shot entry of the next shot to launch (2 bytes per shot).
uint8_t *getSalvoShotPtr(void)
{
    return &g_salvoShots + (uint8_t)(g_salvoLaunched << 1);
}
