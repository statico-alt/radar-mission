// Game A state $0d: the CPU's turn (was GameA_State0D_CpuTurn and the
// GameA_Cpu* steps in bank_000.asm).
#include "bank8_gamea.h"

// Step 0: run the AI of the CPU commander.
static void gaCpu00Think(void)
{
    switch (g_p2Commander) {
    case 0: cpuTurnType0(); break;
    case 1: cpuTurnType1(); break;
    case 2: cpuTurnType2(); break;
    }
}

// Step 5: fire the next shot of the CPU salvo at the player fleet.
static void gaCpu05Fire(void)
{
    if ((g_gameFlags4 & 0x04) && g_salvoLaunchSfxCount != g_salvoCount
        && !--g_salvoLaunchSfxTimer) {
        g_salvoLaunchSfxTimer = 0x20;
        g_salvoLaunchSfxCount++;
        SFX_Queue = 0x03;
        g_sfx04Busy = 0;
    }
    if (--g_shotTallyTimer)
        return;
    g_shotTallyTimer = 0x20;
    g_p2ShotsFired++;
    uint8_t *rec = animGetNextRecordPtr();
    uint8_t packed = *getSalvoShotPtr();
    uint8_t d = packed >> 4, e = packed & 0x0f;
    g_tmpE8 = 0;
    uint8_t *cell = GRID_CELL(RAM(g_playerFleetGrid), d, e);
    uint8_t v = *cell;
    if (!v) {
        // A miss. On the hard setting, note a miss next to a ship.
        g_listCount = 0;
        if (!(g_p2Options & 0x04))
            goto mark;
        for (uint8_t dir = 0; dir < 4; dir++) {
            uint8_t r = d, c = e;
            if (dir == 0) r--;
            else if (dir == 1) r++;
            else if (dir == 2) c--;
            else c++;
            if (r >= g_gridSize || c >= g_gridSize)
                continue;
            uint8_t w = *GRID_CELL(RAM(g_playerFleetGrid), r, c) & 0x3f;
            if (w && w < 0x12)
                g_listCount = 0x02;
        }
        if (!g_listCount)
            goto mark;
        g_p1Options |= 0x40;
        RAM(g_aiKnownCells)[g_aiKnownCount++] = packed;
        goto mark;
    }
    // A hit.
    uint8_t id = v & 0x3f;
    g_shipCellPos[id - 1] = packed;
    g_listCount = 0x01;
    drawShipCellMarkers();
    uint8_t ship = getShipFromCellId(id);
    if (!--g_shipHitsLeft[ship]) {
        if (id == 0x12)
            goto sunk;
        RAM(g_shipNumToShow)[g_spriteCount++] = ship;
        g_tmpE8++;
    }
    {
        uint8_t n = g_aiHitCount;
        uint8_t *h = RAM(g_aiHitList) + 1;
        for (; n; n--, h += 2)
            if (*h == packed)
                goto sunk;
        h[0] = packed;
        h[-1] = 0x08;
        if (!(g_gameFlags4 & 0x08))
            h[-1] = (g_gameFlags9 & 0x04) ? 0x04 : 0x01;
        g_aiHitCount++;
        g_gameFlags9 &= ~0x10;
        g_aiHitsScored++;
    }
sunk:
    v += 0x12;
    *cell = v;
    g_shipCellId = v;
    g_placeDirection = v >> 6;
    g_gameAState = 0x19;
    g_shipTileCount = 0x05;
    g_shotTallyTimer = 0x1b;
    g_p2Hits++;
    v -= 0x12;
mark:
    rec[0] = g_listCount;
    rec[1] = 0;
    rec[2] = 0;
    rec[3] = 0;
    *GRID_CELL(RAM(g_cpuShotGrid), d, e) = g_listCount == 1 ? 1 : 2;
    if (g_tmpE8) {
        g_aiSunkLineCell[0] = d;
        g_aiSunkLineCell[1] = e;
        aiMarkSunkLine();
        if (g_gameATurnTimer >= 0x05)
            g_gameFlags9 |= 0x04;
        g_aiState = 0;
    }
    uint8_t *oam = animOamPtr();
    uint16_t yx = gaGridCellToScreen(d, e);
    setMetaspritePos2x2(oam, yx, yx >> 8, 8);
    g_animScriptCount++;
    g_gameFlags6 |= 0x40;
    if ((v & 0x3f) == 0x12) {
        g_gameFlags6 &= ~0x02;
        if (countNonZero5(g_shipHitsLeft) < 2 && !(g_gameFlags3 & 0x40)) {
            g_gameFlags3 |= 0x40;
            g_gameFlags4 |= 0x40;
        }
    }
    if (++g_salvoLaunched != g_salvoCount)
        return;
    if (!g_aiState)
        aiPickNextState();
    g_gameACpuState = 0x03;
    uint8_t a = g_gameFlags4 & 0x0c;
    if (a) {
        g_gameFlags9 |= a >> 2;
        g_gameFlags4 &= 0xf3;
    }
    uint8_t n = g_salvoCount;
    const uint8_t *s = RAM(g_salvoShots);
    do {
        packed = *s;
        a = checkLuckySquare(1, packed);
        if (a)
            goto lucky;
        s += 2;
    } while (--n);
    return;
lucky:
    g_gameFlags4 |= a << 2;
    g_cpuShotCell = packed;
    g_gameATurnTimer = 0x40;
    g_gameACpuState = 0x0a;
    if (g_gameFlags9 & 0x01)
        return;
    g_gameFlags6 &= ~0x40;
}

// The player fleet is gone, or the CPU turn is over: scroll back and go
// to Game A state $11.
static void gaCpuGameOver(void)
{
    g_scrollDirSteps = g_savedScrollYSteps ^ 0x80;
    g_scrollXDirSteps = g_savedScrollXSteps ^ 0x80;
    fillBytes(OAM, 0, 0x10);
    g_gameAState = 0x11;
    g_gameADelay = 0x50;
    g_gameATurnTimer = 0;
    g_gameFlags3 &= ~0x20;
    g_gameFlags9 |= 0x80;
    g_gameALevel |= 0x02;
}

// Step 7: move the CPU shot sprites. Then aim the ship sprite at the
// link cell and go to step $0c.
static void gaCpu07FlyShots(void)
{
    if (g_gameATurnTimer == 0x50)
        NoiseSfx_Queue = 0x0b;
    nudgeCellSprites();
    if (--g_gameATurnTimer)
        return;
    if (g_linkCellRow == 0xff) {
        gaCpuGameOver();
        return;
    }
    uint8_t dir = g_shipDirBits;
    const uint8_t *hl = g_foundShipCell + 1, *de = RAM(g_linkCellCol);
    uint8_t c = RAM(g_oamBuffer)[1], b;
    if (!(dir & 0x80)) {
        b = (dir & 0x40) ? 0 : 0xa0;
    } else {
        c = RAM(g_oamBuffer)[0];
        b = (dir & 0x40) ? 0xf8 : 0xa8;
        hl--;
        de--;
    }
    uint8_t y = b, x = (uint8_t)((uint8_t)(*de - *hl) << 4) + c;
    if (dir & 0x80) {
        y = x;
        x = b;
    }
    setMetaspritePos2x2(OAM, x, y, 8);
    hl = g_foundShipCell;
    de = RAM(g_linkCellRow);
    b = 0x2c;
    if (dir & 0x80) {
        hl++;
        de++;
    } else if (!(dir & 0x40)) {
        b = 0x24;
    }
    if (!(dir & 0x40)) {
        const uint8_t *t = hl;
        hl = de;
        de = t;
    }
    uint8_t t = (uint8_t)((uint8_t)(*de - *hl) << 3) + b;
    g_gameATurnTimer = t;
    g_gameADelay = 0x48 - t;
    g_gameACpuState = 0x0c;
}

// Step $0a: the lucky square splash.
static void gaCpu0ASplash(void)
{
    if (g_gameFlags6 & 0x40)
        return;
    uint8_t *p = OAM;
    if (g_gameATurnTimer == 0x40) {
        uint8_t v = g_cpuShotCell;
        uint16_t yx = gaGridCellToScreen(v >> 4, v & 0x0f);
        setMetaspritePos2x2(OAM, yx, yx >> 8, 8);
        for (uint8_t c = 0; c < 4; c++)
            p[2 + (c << 2)] = 0x6b;
        SFX_Queue = 0x04;
        rOBP0 = 0xc6;
        if (g_gameFlags4 & 0x04)
            rOBP0 = 0x36;
    }
    if (!(g_gameATurnTimer & 0x03)) {
        p[0]--;
        p[1]--;
        p[4]--;
        p[5]++;
        p[8]++;
        p[9]--;
        p[12]++;
        p[13]++;
    }
    if (--g_gameATurnTimer)
        return;
    fillBytes(OAM, 0, 0x10);
    g_gameACpuState = 0x03;
    if (g_topModeFlags & 0x80)
        return;
    g_aiSavedState = g_aiState;
    g_aiState = 0x06;
}

// Step $0c: mark the hit cell on the player grid, then end the turn.
static void gaCpu0CCheckFleet(void)
{
    nudgeCellSprites();
    if (--g_gameATurnTimer)
        return;
    uint8_t v = g_shipDirBits | 0x12;
    *GRID_CELL(RAM(g_playerFleetGrid), g_linkCellRow, g_linkCellCol) = v;
    gaQueueShipCellTiles(v, (uint16_t)(g_linkCellRow << 8) | g_linkCellCol);
    gaCpuGameOver();
}

// Step $0d: move the ship sprites, then go to step $0e.
static void gaCpu0DStep(void)
{
    if (--g_listCount)
        return;
    g_listCount = 0x20;
    NoiseSfx_Queue = 0x06;
    uint8_t c = g_shownShipLength << 2;
    uint8_t *p = OAM + 2;
    do {
        *p += 3;
        p += 4;
    } while (--c);
    if (--g_gameATurnTimer)
        return;
    g_shipCellId = farByte(1, GameA_ShipCellIdRange + 1 + (uint8_t)(g_shipNumToShow << 1));
    g_shipTileCount = 0x04;
    g_gameATurnTimer = 0x20;
    g_gameACpuState = 0x0e;
}

// Step $0e: turn the ship tiles one step a frame, then go to $0f.
static void gaCpu0ERotateTiles(void)
{
    if (g_gameFlags3 & 0x04)
        return;
    g_gameATurnTimer--;
    if (g_shipTileCount == 0x04) {
        uint8_t i = g_shipCellId - 1;
        uint8_t a = farByte(1, GameA_ShipTileCopyEntries + 14 + i) << 6;
        uint16_t *t = (uint16_t *)g_shipTileSrc;
        t[0] = (uint16_t)(GameA_GfxAirStrikeSprites + 0x230) + a;
        t[1] = ((uint16_t)i << 6) + 0x9020;
    }
    uint8_t done = rotateShipTileStep();
    g_gameFlags3 |= 0x04;
    if (!done)
        return;
    g_shipCellId++;
    g_shipTileCount = 0x04;
    if (--g_shownShipLength)
        return;
    g_gameADelay = g_gameATurnTimer;
    g_gameACpuState = 0x0f;
}

// Step $0f: next ship to show, or step $0b when all are shown.
static void gaCpu0FReset(void)
{
    fillBytes(OAM, 0, 0x70);
    uint8_t *s = RAM(g_shipNumToShow);
    for (uint8_t i = 0; i < 4; i++)
        s[i] = s[i + 1];
    g_gameACpuState = 0x09;
    g_gameADelay = 0x20;
    if (--g_spriteCount)
        return;
    farCopy(1, RAM(g_vramCopyQueue), GameA_ShipTileCopyEntries + 7, 7);
    g_vramCopyCount = 0x01;
    g_gameACpuState = 0x0b;
    if (countNonZero5(g_shipHitsLeft) != 1)
        return;
    g_gameFlags3 |= 0x40;
    g_gameFlags4 |= 0x40;
}

void gameACpuState(void)
{
    switch (g_gameACpuState) {
    case 0x00: gaCpu00Think(); break;
    case 0x01: turn01PlayerAim(); break;
    case 0x02: turn02(); break;
    case 0x03: turn03(); break;
    case 0x04: turn04ClearOAM(); break;
    case 0x05: gaCpu05Fire(); break;
    case 0x06: turn06FireAtTarget(); break;
    case 0x07: gaCpu07FlyShots(); break;
    case 0x08: turn08AltHandler(); break;
    case 0x09:
        if (!g_vramCopyCount)
            showShipSprite();
        break;
    case 0x0a: gaCpu0ASplash(); break;
    case 0x0b: cpu0B(); break;
    case 0x0c: gaCpu0CCheckFleet(); break;
    case 0x0d: gaCpu0DStep(); break;
    case 0x0e: gaCpu0ERotateTiles(); break;
    case 0x0f: gaCpu0FReset(); break;
    case 0x10: cpu10SetDelay(); break;
    case 0x11:
        if (--g_gameATurnTimer)
            blinkLuckyMarkers();
        else
            g_gameACpuState = 0;
        break;
    case 0x12: cpu12SetDelay(); break;
    }
}
