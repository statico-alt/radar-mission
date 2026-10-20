// Game A turn sub-states and state steps. Bank 1 data comes through
// copyFromBank1, peek1w and setMetaspriteTiles4.
#include "bank1.h"

// 1 on the turn that the enemy aircraft come (the Aircraft option, bit 3).
// That turn is 27 - 3 * (12 - grid size).
static uint8_t isAircraftTurn(void)
{
    uint8_t d = 12 - g_gridSize;
    return g_turnCount == (uint8_t)(27 - d - (d << 1));
}

// Blink the 6 Lucky Shot markers (sprites 4-9) every 24 frames, while
// $ffac bits 2-3 are set. Sound $0b when they show.
void blinkLuckyMarkers(void) __banked
{
    if (!(g_gameFlags4 & 0x0c))
        return;
    if (--g_blinkTimer)
        return;
    g_blinkTimer = 0x18;
    uint8_t *s = OAM + 0x10;
    for (uint8_t n = 6; n; n--, s += 4)
        *s ^= 0x54;
    if (OAM[0x10] == 0x54)
        SFX_Queue = 0x0b;
}

// The steps from one cursor coordinate to another. Bit 7 set = go down.
static uint8_t cursorSteps(uint8_t from, uint8_t to)
{
    return from >= to ? (uint8_t)(from - to) | 0x80 : to - from;
}

// Move a cursor coordinate 1 step. Count down the steps.
static uint8_t stepCursor(uint8_t *steps, uint8_t v)
{
    uint8_t n = *steps & 0x7f;
    if (n) {
        n--;
        v++;
        if (*steps & 0x80) {
            v -= 2;
            n |= 0x80;
        }
        *steps = n;
    }
    return v;
}

// Move v 1 cell up or down (down when dec is set). Stay in the grid.
static uint8_t nudgeCoord(uint8_t v, uint8_t dec)
{
    if (dec)
        return v ? v - 1 : v;
    v++;
    return v == g_gridSize ? v - 1 : v;
}

// Turn sub-state 1: the CPU aims. Blink the Lucky Shot markers. When
// $ff8d bit 7 is set, aim at the cell from link byte $c9dd, moved by up
// to 1 cell at random. Move the CPU cursor 1 cell a frame to it.
void turn01PlayerAim(void) __banked
{
    blinkLuckyMarkers();
    if (g_p1Options & 0x80) {
        g_p1Options &= ~0x80;
        if (!(g_gameFlags4 & 0x0c))
            SFX_Queue = 0x01;
        uint8_t row = g_linkRecvC9dd >> 4;
        uint8_t col = g_linkRecvC9dd & 0x0f;
        uint8_t r = stirRandom() & 0xc3;
        if (!(r & 0x40))
            row = nudgeCoord(row, r & 0x01);
        if (!(r & 0x80))
            col = nudgeCoord(col, r & 0x02);
        g_cpuCursorRowSteps = cursorSteps(g_cpuCursorRow, row);
        g_cpuCursorColSteps = cursorSteps(g_cpuCursorCol, col);
    }
    uint8_t row = stepCursor(&g_cpuCursorRowSteps, g_cpuCursorRow);
    uint8_t col = stepCursor(&g_cpuCursorColSteps, g_cpuCursorCol);
    // Keep the cursor off the grid edge.
    if (!row)
        row++;
    if (!col)
        col++;
    uint8_t last = g_gridSize - 1;
    if (row == last)
        row--;
    if (col == last)
        col--;
    g_cpuCursorRow = row;
    g_cpuCursorCol = col;
    setMetaspritePos2x2(OAM, CELL_SCREEN(col) - 12, CELL_SCREEN(row) - 12, 0x20);
    setMetaspriteTiles4(OAM, GameA_CpuCursorTiles);
}

// Turn sub-state 2: play the salvo launch sounds, 1 every $20 frames.
// After $ffb1 frames start the tally anim scripts and go to sub-state 5.
void turn02(void) __banked
{
    uint8_t shots = (g_gameFlags4 & 0x04) ? g_salvoCount : 1;
    if (g_salvoLaunchSfxCount != shots && !--g_salvoLaunchSfxTimer) {
        g_salvoLaunchSfxTimer = 0x20;
        g_salvoLaunchSfxCount++;
        SFX_Queue = 0x03;
        g_sfx04Busy = 0;
    }
    if (--g_gameATurnTimer)
        return;
    g_shotTallyTimer = 1;
    g_gameACpuState = 0x05;
    copyFromBank1(&g_animScriptPtrs, GameA_TallyAnimSetup, 0x0b);
}

// Turn sub-state 3: wait for the anim scripts ($ffae bit 6). Count the
// turn. With the Aircraft option, the enemy aircraft can attack the
// player 1 fleet. Then go to sub-state $0b, or 9 to redraw hit ships.
void turn03(void) __banked
{
    if (g_gameFlags6 & 0x40)
        return;
    if (g_p1Options & 0x40) {
        g_p1Options &= ~0x40;
        SFX_Queue = 0x0c;
    }
    g_gameADelay = 0x50;
    if (!(g_topModeFlags & 0x01))
        g_turnCount++;
    if ((countNonZero5(g_shipHitsLeft) || g_revealFlag) && (g_p2Options & 0x08)
        && ((g_gameFlags6 & 0x01) || (isAircraftTurn() && g_p2ShipHitsLeft[0]))) {
        // Point $c810/$c812 at the player 2 fleet and shot grids.
        copyFromBank1(&g_activeFleetGridPtr, GameA_EnemyFleetGridPtrs, 4);
        g_currentPlayer = 1;
        processShipList();
        g_currentPlayer = 0;
        if (g_topModeFlags & 0x80) {
            g_serialSend = 0x06;
            g_gameALinkState = 0x05;
        }
        if (!(g_gameFlags6 & 0x01)) {
            g_gameFlags6 |= 0x08;
            if (g_linkCellRow != 0xff) {
                g_gameFlags6 |= 0x01;
                g_replyShotFlag = 1;
            }
        }
    }
    g_gameACpuState = 0x0b;
    if (!g_spriteCount)
        return;
    copyFromBank1(&g_vramCopyQueue, GameA_ShipTileCopyEntries, 7);
    g_vramCopyCount = 1;
    g_gameADelay = 0x88;
    g_gameACpuState = 0x09;
}

// Turn sub-state 4: clear the first $70 bytes of OAM. Put the enemy
// aircraft next to the player 1 ship with cell ids 5 and $17, facing
// the ship direction. Go to sub-state $12.
void turn04ClearOAM(void) __banked
{
    uint8_t *s = OAM;
    for (uint8_t n = 0x70; n; n--)
        *s++ = 0;
    uint16_t rc = findGridCellByValue(&g_playerFleetGrid, 0x05, 0x17);
    uint8_t dir = *GRID_CELL(&g_playerFleetGrid, rc >> 8, rc) & 0xc0;
    g_shipDirBits = dir;
    uint8_t m = getGridMargin();
    // 2 bytes per direction: row and column offset.
    uint16_t off = peek1w(GameA_AircraftDirOffsets + (dir >> 5));
    setMetaspritePos2x2(OAM, CELL_SCREEN((uint8_t)(m + (off >> 8))), CELL_SCREEN((uint8_t)(m + off)), 8);
    setMetaspriteTiles4(OAM, GameA_AircraftDirTiles + (dir >> 3));
    g_gameACpuState = 0x12;
    g_gameATurnTimer = 0x50;
    g_gameADelay = 0xff;
    NoiseSfx_Queue = 0x0a;
}

// Turn sub-state $12: wait $1e frames, then go to sub-state $10.
void cpu12SetDelay(void) __banked
{
    g_gameADelay = 0x1e;
    g_gameACpuState = 0x10;
}

// Turn step 6: clear OAM and list the ships that sank ($c966 bit 4) at
// $c92e. With sunk ships, start the burning anim (step $0f). Else, when
// $ffb0 bits 0-1 are clear, queue the 2 "miss" tiles at $98ca.
void turn06ClearOAM(void) __banked
{
    g_gameATurnState = 0x08;
    uint8_t *s = OAM;
    for (uint8_t n = 0xa0; n; n--)
        *s++ = 0;
    g_spriteCount = 0;
    const uint8_t *r = &g_salvoResults;
    uint8_t *ship = &g_shipNumToShow;
    uint8_t n = g_salvoResultCount;
    do {
        if (r[0] & 0x10) {
            *ship++ = r[1];
            g_spriteCount++;
        }
        r += 2;
    } while (--n);
    if (g_spriteCount) {
        SFX_Queue = 0x1d;
        g_gameATurnState = 0x0f;
        g_gameADelay = 0x38;
        ((uint16_t *)&g_animScriptPtrs)[2] = (uint16_t)GameA_ShipBurningLoopAnim;
        return;
    }
    if (g_gameFlags9 & 0x03)
        return;
    uint8_t level = g_gameALevel & 0xc0;
    uint8_t tile = level == 0x40 ? 0xa6 : level == 0x80 ? 0x6a : 0x92;
    (&g_vramQueue)[0] = tile;
    (&g_vramQueue)[1] = tile;
    queueVramCopy(&g_vramQueue, 0x98ca, 2, 1, 2);
}

// Turn step 8: reset the scroll, LYC and palettes after the shots.
// When the player 2 fleet is gone (and no reply shot), go to state $15
// step 2. Else go to step $0a. With the Aircraft option, the player 2
// aircraft can attack the CPU (player 1) fleet.
void turn08ResetScroll(void) __banked
{
    g_gameFlags2 &= 0xdb;
    rSCX = 0;
    rLYC = 0;
    rOBP0 = 0x27;
    rOBP1 = 0x36;
    if (!countNonZero5(g_p2ShipHitsLeft)) {
        g_gameFlags4 |= 0x20;
        if (!g_replyShotFlag) {
            g_gameAState = 0x15;
            g_gameAState15Step = 0x02;
            g_gameATurnTimer = 0x28;
            BGM_Queue = 0xff;
            g_gameALevel &= ~0x02;
            return;
        }
    }
    g_gameFlags5 |= 0x40;
    g_gameATurnTimer = 0;
    g_gameATurnState = 0x0a;
    if (g_topModeFlags & 0x01)
        g_turnCount++;
    if (g_topModeFlags & 0x80)
        return;
    if (!(g_p1Options & 0x08))
        return;
    if (!(g_gameFlags6 & 0x02) && (!isAircraftTurn() || !g_shipHitsLeft[0]))
        return;
    WORD(g_activeFleetGridPtr) = (uint16_t)&g_playerFleetGrid;
    WORD(g_activeShotGridPtr) = (uint16_t)&g_cpuShotGrid;
    g_currentPlayer = 0;
    processShipList();
    g_currentPlayer++;
    if (g_gameFlags6 & 0x04)
        return;
    if (g_linkCellRow == 0xff)
        return;
    *GRID_CELL(&g_playerFleetGrid, g_linkCellRow, g_linkCellCol) = 0;
    g_revealFlag = 1;
    g_gameFlags6 |= 0x02;
}

// Set the STAT effect for the window at line $47 (arg 2 = $49).
// Clear $ffaa bit 4, set bit 2.
void resetStatEffectA(void) __banked
{
    g_gameAStatEffect = 0;
    g_statEffect1Arg = 0x47;
    rLYC = 0x47;
    g_statEffect2Arg = 0x49;
    g_gameFlags2 &= ~0x10;
    g_gameFlags2 |= 0x04;
}

// Set the STAT effect at line 1 (arg 2 = $8f). Set $ffaa bits 2-4.
void resetStatEffectB(void) __banked
{
    g_gameAStatEffect = 0;
    g_statEffect1Arg = 1;
    rLYC = 1;
    g_statEffect2Arg = 0x8f;
    g_gameFlags2 |= 0x1c;
}

// Game A state $16 step 0: every 8 frames move the 8 sprites at OAM
// entry $0a up 1 line, $ffe4 times. Then go to step 1.
void state16Step0(void) __banked
{
    if (--g_gameAFxTimer)
        return;
    g_gameAFxTimer = 8;
    if (!g_gameAFxSteps) {
        g_gameAState16Step = 1;
        g_gameADelay = 0xd9;
        return;
    }
    g_gameAFxSteps--;
    uint8_t *s = OAM + 0x28;
    for (uint8_t n = 8; n; n--, s += 4)
        (*s)--;
}

// Game A state $16 step 1: fade out ($ffaa bit 1) and add the turns to
// the total. Go to the results (state $17), or to the next level (state
// $18) after level 1 or 2 out of link play.
void state16Step1(void) __banked
{
    g_gameFlags2 |= 0x02;
    g_gameAState = 0x17;
    g_gameATotalTurns += g_turnCount;
    if (g_topModeFlags & 0x80)
        return;
    if ((g_gameALevel & 0xc0) == 0x80)
        return;
    g_gameAState = 0x18;
    g_nextLevelStep = 0;
}

// Game A state $1a step 2: the aircraft fly up 8 lines a frame. At Y
// $b0 go to step 5.
void state1AStep2(void) __banked
{
    uint8_t *s = OAM;
    for (uint8_t n = 0x27; n; n--, s += 4)
        *s -= 8;
    if (*OAM != 0xb0)
        return;
    g_gameADelay = 0x2c;
    g_gameAState1AStep = 5;
}

// Level title step 1: after $ffb2 frames play music 3. Then A or Start
// fades out ($ffaa bit 1) to state 3. With $ffaf bit 7, sprites 0 and
// 1 change tile every $18 frames.
void levelTitleStep1(void) __banked
{
    uint8_t n = g_listCount;
    if (n) {
        g_listCount = --n;
        if (!n)
            BGM_Queue = 0x03;
    }
    if (!n && (g_joyPressedA & 0x09)) {
        g_gameFlags2 |= 0x02;
        g_gameFlags8 &= ~0x80;
        g_gameAState = 0x03;
        return;
    }
    if (!(g_gameFlags8 & 0x80))
        return;
    if (--g_gameATurnTimer)
        return;
    g_gameATurnTimer = 0x18;
    OAM[1] ^= 0x7e;
    OAM[5] ^= 0x7e;
}

// Game A state 4: every $18 frames show or hide "WAITING" (7 tiles at
// $9a0b) through the tilemap queue ($ffab bit 2).
void state04Delay(void) __banked
{
    if (--g_blinkTimer)
        return;
    g_blinkTimer = 0x18;
    uint8_t *q = &g_vramQueue;
    q[0x0a] = 0;
    q[0] = 0x9a;
    q[1] = 0x0b;
    q[2] = 0x07;
    for (uint8_t i = 3; i < 10; i++)
        q[i] = 0;
    if (g_blinkCount)
        copyFromBank1(q + 3, GameA_WaitingText, 7);
    g_gameFlags3 |= 0x04;
    g_blinkCount ^= 0x01;
}

// Turn sub-state $10: wait $16 frames, then go to sub-state 7.
void cpu10SetDelay(void) __banked
{
    g_gameADelay = 0x16;
    g_gameACpuState = 0x07;
}

// Game A state 2: fade out ($ffaa bit 1) and set the fade targets. Go
// to state $1b. In link play copy $ff8a bit 6 to bit 0.
void state02Init(void) __banked
{
    ClearShadowOAM();
    g_gameFlags2 |= 0x02;
    uint8_t *f = &g_fadeTargetBgp;
    f[0] = 0x36;
    f[1] = 0xb4;
    f[2] = 0x36;
    g_gameAState = 0x1b;
    if (!(g_topModeFlags & 0x80))
        return;
    if (g_topModeFlags & 0x40)
        g_topModeFlags |= 0x01;
}

// For each ship cell of the CPU fleet grid ($90 cells), store the cell
// direction (bits 6-7) in the record of its ship ($c9a4). The ship is
// found from the cell id and the player 2 ship lengths.
void storeCpuShipDirs(void) __banked
{
    const uint8_t *g = &g_cpuFleetGrid;
    uint8_t c = 0x90;
    do {
        uint8_t v = *g++;
        if (!v)
            continue;
        uint8_t dir = v >> 6;
        g_placeDirection = dir;
        uint8_t id = v & 0x3f;
        uint8_t n = (g_gameALevel & 0xc0) ? 4 : 5;
        const uint8_t *len = g_p2ShipLengths;
        uint8_t sum = *len++;
        uint8_t ship = 0;
        while (sum < id) {
            sum += *len++;
            ship++;
            if (!--n)
                break;
        }
        (&g_shipRecords)[ship] = dir;
    } while (--c);
}
