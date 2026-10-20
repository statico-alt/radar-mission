// Game A ship placement and the target cursor. These read bank 1 tables.
#include "bank1.h"

// Write one OAM entry with attributes $10.
static void setSprite(uint8_t *s, uint8_t y, uint8_t x, uint8_t tile)
{
    s[0] = y;
    s[1] = x;
    s[2] = tile;
    s[3] = 0x10;
}

// Add delta to the coordinate v and wrap it to 0..gridSize-1.
static uint8_t wrapCursorAxis(uint8_t v, int8_t delta)
{
    uint8_t n = v + delta;
    if (n == 0xff)
        return g_gridSize - 1;
    if (n == g_gridSize)
        return 0;
    return n;
}

// Move the grid cursor ($ffcc = row, $ffcd = column) with the D-pad.
// $ffd0 is the key-repeat delay. The cursor wraps at the grid size ($ffcb).
// Returns 1 when the cursor moved (a sound is queued).
uint8_t moveGridCursor(void)
{
    if (g_cursorRepeatDelay) {
        g_cursorRepeatDelay--;
        return 0;
    }
    uint8_t held = 0;
    uint8_t keys = g_joyPressedA & 0xf0;
    if (!keys) {
        keys = g_joyRaw & 0xf0;
        if (!keys)
            return 0;
        held = 1;
    }
    g_cursorRepeatDelay = 7;
    SFX_Queue = (g_gameFlags4 & 0x03) ? 0x11 : 0x01;
    // The highest key bit picks the delta pair: Down, Up, Left, Right.
    const int8_t *d = CursorMoveDeltas + 2;
    while (!(keys & 0x80)) {
        keys <<= 1;
        d += 2;
    }
    uint8_t row = wrapCursorAxis(g_cursorRow, d[0]);
    uint8_t col = wrapCursorAxis(g_cursorCol, d[1]);
    uint8_t moved = 1;
    // A held key does not wrap. The cursor stays and the sound is cancelled.
    if (held && (row != (uint8_t)(g_cursorRow + d[0]) || col != (uint8_t)(g_cursorCol + d[1]))) {
        SFX_Queue = 0;
        moved = 0;
    } else {
        SET_HRAM(g_cursorRow, row);
        SET_HRAM(g_cursorCol, col);
    }
    drawTargetCursor();
    // The asm returned SFX_Queue != 0 here. The VBlank sound code can
    // take the sound first, so C returns the move result.
    return moved;
}

// Put the target cursor sprites on each cell of the shot pattern.
// The pattern is GameA_TargetPatternOffsets (1, 5 or 9 cells).
// Cells outside the grid get a hidden sprite.
// With the salvo cursor ($ffac bit 0), cells already shot use palette 0.
void drawTargetCursor(void)
{
    const int8_t *off = GameA_TargetPatternOffsets;
    uint8_t *s = OAM;
    for (uint8_t n = TARGET_CELL_COUNT; n; n--, s += 4, off += 2) {
        s[0] = 0xf0;
        uint8_t row = g_cursorRow + off[0];
        if (row >= g_gridSize)
            continue;
        uint8_t col = g_cursorCol + off[1];
        if (col >= g_gridSize)
            continue;
        s[0] = CELL_OAM_Y(row);
        s[1] = CELL_OAM_X(col);
        if (g_gameFlags4 & 0x01)
            s[3] = *GRID_CELL(&g_playerShotGrid, row, col) ? 0x00 : 0x10;
    }
}

// Draw the ship icon and its 3-letter name with sprites.
// The data comes from GameA_ShipNameplateTable: count, first tile, name.
static void drawShipNameplate(uint8_t ship)
{
    fillBytes(OAM + 0x14, 0, 0x14);
    const uint8_t *p = GameA_ShipNameplateTable + (uint8_t)((ship << 2) + ship);
    uint8_t count = p[0], tile = p[1];
    // The icon goes up from the sprite at $c024, at X = $90.
    uint8_t *s = OAM + 0x24;
    for (uint8_t y = 0x38; count; count--, y -= 8, s -= 4)
        setSprite(s, y, 0x90, tile--);
    s = OAM + 0x28;
    for (uint8_t i = 0; i < 3; i++, s += 4)
        setSprite(s, 0x48, 0x88 + (i << 3), p[2 + i]);
}

// Clear the 40 bytes of OAM at $c078, then draw one icon for each ship
// that still has hits left (g_shipHitsLeft). Ships 0-3 use 2 sprites,
// ship 4 uses 1.
static void drawFleetIcons(void)
{
    uint8_t *s = OAM + 0x78;
    fillBytes(s, 0, 0x28);
    for (uint8_t i = 0; i < 5; i++) {
        uint8_t n = i < 4 ? 2 : 1;
        if (g_shipHitsLeft[i]) {
            uint8_t tile = GameA_FleetIconTiles[i];
            for (uint8_t j = 0; j < n; j++)
                setSprite(s + (j << 2), (i << 3) + 0x50, 0x88 + (j << 3), tile + j);
        }
        s += n << 2;
    }
}

// Select the next ship in $ffbe that is not placed yet.
// A ship is placed when its byte in $ffb8 is $ff.
// Index 5 reads the byte after the table, as in the asm.
void selectNextShip(void)
{
    drawFleetIcons();
    uint8_t ship = g_selectedShip;
    for (uint8_t tries = 6; tries; tries--) {
        if (++ship == 6)
            ship = 0;
        if (g_shipPlaced[ship] != 0xff) {
            g_selectedShip = ship;
            drawShipNameplate(ship);
            return;
        }
    }
}

// Returns 0 (Z) when a cell of the shot pattern at row, col is not shot
// yet. Returns 1 when all are shot. Cells outside the grid are not checked.
uint8_t drawTargetPattern(uint8_t row, uint8_t col)
{
    const int8_t *off = GameA_TargetPatternOffsets;
    for (uint8_t n = TARGET_CELL_COUNT; n; n--, off += 2)
        if (!*GRID_CELL(&g_playerShotGrid, row + off[0], col + off[1]))
            return 0;
    return 1;
}

// Shows the target cursor sprites at $c002. Uses tile $6b when $ffac
// bits 0-1 are set (salvo mode), else $6a.
void showTargetCursor(void)
{
    uint8_t tile = (g_gameFlags4 & 0x03) ? 0x6b : 0x6a;
    uint8_t *s = OAM;
    for (uint8_t n = TARGET_CELL_COUNT; n; n--, s += 4) {
        s[2] = tile;
        s[3] = 0x10;
    }
    g_cursorRow = g_savedCursorRow;
    g_cursorCol = g_savedCursorCol;
    drawTargetCursor();
}

// Ship placement sub-state 0. Waits for A or Start on the placement
// screen. Until then it flips rLCDC bit 3 (the BG map) on a timer:
// $18 frames on one map, $40 frames on the other (with sound $16).
void placeState0WaitButton(void)
{
    if (g_joyPressedA & 0x09) {
        showTargetCursor();
        rLCDC = 0xc3;
        g_gameAPlaceState = 1;
        return;
    }
    if (--g_gameABlinkTimer)
        return;
    rLCDC ^= 0x08;
    g_gameABlinkStep ^= 0x01;
    g_gameABlinkTimer = 0x18;
    if (!g_gameABlinkStep)
        return;
    g_gameABlinkTimer = 0x40;
    SFX_Queue = 0x16;
}

// Ship placement sub-state 2. Turns the picked ship with the D-pad
// (GameA_DpadToDirection). B goes back to sub-state 1, A to sub-state 5.
void placeState2TurnShip(void)
{
    uint8_t keys = g_joyPressedA;
    for (uint8_t i = 0; i < 4; i++, keys <<= 1) {
        if (!(keys & 0x80))
            continue;
        uint8_t dir = GameA_DpadToDirection[i];
        if (dir == g_placeDirection)
            return;
        calcPlacementBounds(dir);
        if (checkCursorInBounds() || checkShipFits(&g_playerFleetGrid, dir)) {
            NoiseSfx_Queue = 0x05;
            return;
        }
        g_placeDirection = dir;
        setShipTileSource();
        fillBytes(OAM, 0, 0x14);
        SFX_Queue = 0x01;
        g_gameAPlaceState = 3;
        g_placeNextState = 4;
        return;
    }
    uint8_t size;
    if (g_joyPressedA & 0x02) {
        g_gameAPlaceState = 1;
        SFX_Queue = 0x08;
        size = 0x14;
    } else if (g_joyPressedA & 0x01) {
        g_gameAPlaceState = 5;
        SFX_Queue = 0x21;
        size = 0x34;
    } else {
        return;
    }
    fillBytes(OAM, 0, size);
    drawTargetCursor();
    OAM[2] = 0x6a;
}

// Builds OAM sprites at $c078 from GameA_LevelSpritesLevel1/2/3,
// chosen by the Game A level. Each row is: count, first tile. $ff ends.
void buildLevelSprites(void)
{
    uint8_t level = g_gameALevel & 0xc0;
    const uint8_t *p = level == 0 ? GameA_LevelSpritesLevel1
                     : level == 0x40 ? GameA_LevelSpritesLevel2
                     : GameA_LevelSpritesLevel3;
    uint8_t *s = OAM + 0x78;
    for (uint8_t y = 0x50; *p != 0xff; y += 8, p += 2) {
        uint8_t tile = p[1], x = 0x88;
        for (uint8_t n = p[0]; n; n--, s += 4, x += 8)
            setSprite(s, y, x, tile++);
    }
}

// Placement B button: pick up the placed ship under the cursor.
// It clears the ship from g_playerFleetGrid, queues tile $b5 over it,
// and selects it again.
void placeOnBPickUpShip(void)
{
    uint8_t id = *GRID_CELL(&g_playerFleetGrid, g_cursorRow, g_cursorCol) & 0x3f;
    if (!id)
        return;
    uint8_t ship = getShipFromCellId(id);
    g_selectedShip = ship;
    g_shipPlaced[ship] = ship;
    uint8_t dir = (&g_shipRecords)[ship];
    g_placeDirection = dir;
    g_shipHitsLeft[ship] = 0xff;
    const uint8_t *range = GameA_ShipCellIdRange + (uint8_t)(ship << 1);
    pickUpShipCells(range[0], range[1]);
    drawShipNameplate(g_selectedShip);
    drawFleetIcons();
    g_placeDirection = 0;
    g_gameAPlaceState = 1;
    g_gameFlags3 |= 0x04;
    SFX_Queue = 0x08;
}

// Draws the coordinate label sprites at $c060. The column letter is
// $e7 + column and the row digit is $d9 + row.
void drawCoordLabels(void)
{
    buildSpriteStrips(GameA_CoordLabelLayout, 0x1003, OAM + 0x60);
    OAM[0x62] = g_cursorCol + 0xe7;
    OAM[0x6a] = g_cursorRow + 0xd9;
}

// Returns 0 when all 5 ships are placed ($ff in each byte at $ffb8).
uint8_t checkAllShipsPlaced(void)
{
    for (uint8_t i = 0; i < 5; i++)
        if (g_shipPlaced[i] != 0xff)
            return 1;
    return 0;
}

// The cursor cell: high byte = row ($ffcc), low byte = column ($ffcd).
uint16_t getCursorCell(void)
{
    return (g_cursorRow << 8) | g_cursorCol;
}

// Returns 1 when v is not a valid grid coordinate.
uint8_t checkCoordOutsideGrid(uint8_t v)
{
    return v >= g_gridSize;
}

// The OAM position of a grid cell: high byte = Y, low byte = X.
uint16_t gridCellToOamXY(uint8_t row, uint8_t col)
{
    return (CELL_OAM_Y(row) << 8) | CELL_OAM_X(col);
}
