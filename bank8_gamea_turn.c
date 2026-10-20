// Game A state $0a: the player's turn (was GameA_State0A_Turn and the
// GameA_Turn* steps in bank_000.asm).
#include "bank8_gamea.h"

// Turn step 0: aim. A fires, B or Select swaps the view.
static void gaTurn00Aim(void)
{
    tickRedrawTimers();
    if (!(g_gameFlags8 & 0x10)) {
        if (moveGridCursor()) {
            drawCoordLabels();
            if (g_topModeFlags & 0x80) {
                g_serialSend = 0x0d;
                g_gameALinkState = 0x0e;
            }
        }
        if (g_joyPressedA & 0x01) {
            uint16_t rc = getCursorCell();
            if (gaDrawTargetPattern(rc >> 8, rc)) {
                NoiseSfx_Queue = 0x05;
                return;
            }
            gaSpriteStrips(GameA_PlacementSpriteLayout + 0x38, 0x1003, OAM + 0x6c);
            g_gameATurnTimer = 0x20;
            g_gameATurnState = 0x03;
            g_gameFlags6 &= ~0x10;
            SFX_Queue = 0x05;
            goto saveCursor;
        }
    }
    if (!(g_joyPressedA & 0x06))
        return;
    SFX_Queue = 0x08;
    rLCDC ^= 0x08;
    g_gameFlags8 ^= 0x10;
    if (!(g_gameFlags8 & 0x10)) {
        showTargetCursor();
        drawCoordLabels();
        return;
    }
    fillBytes(OAM + 0x60, 0, 0x0c);
    fillBytes(OAM, 0, 0x24);
saveCursor:
    g_savedCursorRow = g_cursorRow;
    g_savedCursorCol = g_cursorCol;
}

// Turn step 1: every 32 frames launch the next shot of the salvo. When
// all are out, mark the shot cells and build the sunk list.
static void gaTurn01LaunchShots(void)
{
    if (--g_gameATurnTimer)
        return;
    g_gameATurnTimer = 0x20;
    uint8_t a = g_salvoLaunched;
    if (a != g_salvoCount) {
        uint8_t ship = RAM(g_salvoShots)[1 + (uint8_t)(a << 1)];
        g_p2ShipHitsLeft[ship]--;
        uint8_t x = 0x20 + (uint8_t)multiply8((uint8_t)(ship << 1), 12);
        uint16_t off = multiply8(g_animScriptCount, (uint8_t)(g_animSpritesPerScript << 2))
                       + (uint8_t)(g_animFirstOamSlot << 2);
        setMetaspritePos2x2(OAM + off, x, 0x38, 8);
        RAM(g_animRecords)[(uint8_t)(g_animScriptCount << 2) - 1] = 0x02;
        g_gameFlags6 |= 0x40;
        g_animScriptCount++;
        g_salvoLaunched++;
        g_p1ShotsFired++;
        g_p1Hits++;
        return;
    }
    g_gameATurnState = 0x0e;
    uint8_t c = g_salvoCount;
    uint8_t *p = RAM(g_salvoShots);
    do {
        uint8_t v = *p;
        uint8_t *cell = GRID_CELL(RAM(g_playerShotGrid), v >> 4, v & 0x0f);
        *cell = 0x01;
        cell += 0x1b0;  // the CPU fleet grid
        uint8_t b = SumFleetBytes();
        if ((*cell & 0x3f) < b)
            *cell += b;
        p += 2;
    } while (--c);
    g_salvoResultCount = 0;
    fillBytes(RAM(g_salvoCount), 0, 0x24);
    uint8_t *r = RAM(g_salvoResults);
    const uint8_t *s = g_salvoShipsHit;
    for (;;) {
        uint8_t ship = *s++;
        if (ship == 0xff)
            return;
        if (g_p2ShipHitsLeft[ship])
            continue;
        *r++ |= 0x10;
        *r++ = ship << 1;
        g_salvoResultCount++;
    }
}

// Turn step 2: when the VRAM queue is empty, draw the next sea row.
static void gaTurn02DrawSea(void)
{
    if (g_vramCopyCount)
        return;
    tickRedrawTimers();
    if (g_gameFlags3 & 0x04)
        return;
    drawGridRow();
    if (g_gameATurnTimer < 0x12)
        return;
    ClearShadowOAM();
    resetStatEffectA();
    g_gameATurnState = 0x0d;
    rOBP0 = 0x27;
}

// Turn step 3: wait, set the sea map pointer and go to step 2.
static void gaTurn03Delay(void)
{
    tickRedrawTimers();
    if (--g_gameATurnTimer)
        return;
    setSeaMapPtr();
    g_gameATurnTimer = 0;
    g_gameATurnState = 0x02;
}

// Turn step 4: move the shot sprites up. Then set up the splash
// sprites and anim scripts and go to step 5.
static void gaTurn04FlyShots(void)
{
    if (g_shotLaunchDelay) {
        g_shotLaunchDelay--;
    } else {
        g_shotLaunchDelay = 0x0e;
        if (g_gameATurnTimer) {
            g_gameATurnTimer--;
            g_animScriptCount++;
            g_shotSpritesLaunched++;
            g_p1ShotsFired++;
            NoiseSfx_Queue = 0x03;
        }
    }
    uint8_t n = g_shotSpritesLaunched;
    uint8_t *p = OAM + 0x18;
    uint8_t b = 0;
    do {
        if (*p == 0xa1) {
            p += 12;
            b++;
        } else {
            for (uint8_t c = 3; c; c--, p += 4)
                *p -= 3;
        }
    } while (--n);
    if ((g_topModeFlags & 0x80) && p[-12] == 0xdd) {
        g_serialSend = 0x04;
        g_gameALinkState = 0x03;
    }
    if (g_shotSpritesLaunched != b)
        return;
    ClearShadowOAM();
    clearAnimScripts();
    g_gameATurnState = 0x05;
    g_salvoResultsShown = 0;
    g_salvoResultsAnimated = 0;
    n = g_salvoResultCount;
    const uint8_t *r = RAM(g_salvoResults);
    p = OAM;
    do {
        p[0] = (r[0] & 0x20) ? 0xe6 : 0x02;
        p[1] = 0x24 + (uint8_t)multiply8(r[1], 12);
        p[2] = 0x5c;
        p += 4;
        r += 2;
    } while (--n);
    if (!(g_gameFlags4 & 0x03))
        RAM(g_oamBuffer)[1] = 0x24;
    g_gameATurnTimer = 0;
    g_animScriptCount = 0;
    g_animSpritesPerScript = 0x04;
    g_animFirstOamSlot = 0x0b;
    uint8_t *s = RAM(g_animScriptPtrs);
    s[0] = 0xb0;
    s[1] = 0x64;
    if (GA_LEVEL3) {
        s[1] = 0x65;
        s[0] = 0x01;
    }
    s[2] = 0x5f;
    s[3] = 0x64;
    s[4] = 0xf6;
    s[5] = 0x63;
    s[6] = 0xdd;
    s[7] = 0x70;
    if (GA_LEVEL3) {
        s[7] = 0x71;
        s[6] = 0x10;
    }
    g_gameADelay = 0x50;
    // The splash delay: 32 frames per result before the first sunk one.
    uint8_t e = 0x1e, a;
    b = 0;
    r = RAM(g_salvoResults);
    n = g_salvoResultCount;
    for (;;) {
        if (*r++ & 0x20) {
            a = *r;
            break;
        }
        r++;
        b++;
        if (!--n) {
            b = 0;
            e = 2;
            a = 0;
            break;
        }
    }
    WORD(g_shotAnimDelay) = (uint16_t)(b << 5) + (uint8_t)(a * 3) + e;
}

// Turn step 5: animate the hit and miss splashes. Then go to step 1
// (more shots), $0e (done) or $0b (lucky square).
static void gaTurn05ShotAnim(void)
{
    if (g_gameFlags6 & 0x01) {
        uint16_t d = WORD(g_shotAnimDelay);
        if (d) {
            WORD(g_shotAnimDelay) = d - 1;
        } else if (!(g_gameFlags5 & 0x08)) {
            g_gameFlags5 |= 0x0c;
            gaSpriteStrips(GameA_ShotAnimSpriteStrip, 0x0002, OAM + 0x24);
            NoiseSfx_Queue = 0x0c;
        }
    }
    if (!g_gameATurnTimer) {
        g_gameATurnTimer = 0x20;
        if (g_salvoResultCount != g_salvoResultsShown)
            g_salvoResultsShown++;
    }
    g_gameATurnTimer--;
    uint8_t c = g_salvoResultsShown;
    uint8_t *p = OAM;
    uint8_t *r = RAM(g_salvoResults);
    do {
        if (*r & 0x40)
            goto next;
        (*p)++;
        if (*p == 0x04) {
            SFX_Queue = 0x03;
            g_sfx04Busy = 0;
        }
        if (*r & 0x20) {
            if (*p != 0x26)
                goto next;
            *p = 0;
            RAM(g_oamBuffer)[0x26] = 0x66;
            RAM(g_oamBuffer)[0x2a] = 0x67;
            g_gameFlags4 |= 0x10;
            g_gameFlags6 &= ~0x01;
            g_p1Hits++;
            g_replyShotFlag = 0;
            NoiseSfx_Queue = 0x01;
            g_shotAnimDelay = 0x04;
            RAM(g_shipRecords)[7] = 0x05;
            if (countNonZero5(g_p2ShipHitsLeft) == 1 && !(g_gameFlags9 & 0x20))
                g_gameFlags9 |= 0x60;
        } else {
            if (*p != 0x42)
                goto next;
            *p = 0;
            uint8_t v = *r;
            setMetaspritePos2x2(OAM + 0x2c + (uint8_t)(g_animScriptCount << 4), p[1] - 4, 0x38, 8);
            g_gameFlags6 |= 0x40;
            uint8_t *rec = RAM(g_animRecords) + (uint8_t)(g_animScriptCount << 2) - 1;
            *rec = 0x03;
            if (v & 0x80) {
                *rec = 0x01;
                if ((v & 0x10) || (g_gameFlags4 & 0x02))
                    *rec = 0x02;
                g_gameFlags2 |= 0x20;
                g_p1Hits++;
            } else if ((v & 0x02) && (g_p1Options & 0x04)) {
                *rec = 0;
                g_p1Options |= 0x40;
            }
            rec[1] = 0;
            rec[2] = 0;
            rec[3] = 0;
        }
        g_animScriptCount++;
        g_salvoResultsAnimated++;
        *r |= 0x40;
next:
        p += 4;
        r += 2;
    } while (--c);

    if (g_gameFlags5 & 0x04) {
        // The lucky shot banner flies right (and bobs on a lucky hit).
        uint8_t b = 0xfc, d = 0;
        if (g_gameFlags4 & 0x10) {
            b = 0xff;
            d = 0x0e;
        }
        p = OAM + 0x25;
        for (c = 2; c; c--) {
            *p++ += b;
            if ((g_gameFlags4 & 0x10) && !--RAM(g_shipRecords)[7]) {
                RAM(g_shipRecords)[7] = 0x05;
                NoiseSfx_Queue = 0x06;
                *p ^= d;
                p[-2]++;
            }
            p += 3;
        }
        if (RAM(g_oamBuffer)[0x25] >= 0xa8) {
            fillBytes(OAM + 0x24, 0, 8);
            g_gameFlags5 &= ~0x04;
        }
    }
    if (g_salvoResultCount != g_salvoResultsAnimated)
        return;
    if (g_gameFlags5 & 0x04)
        return;
    if ((g_gameFlags4 & 0x02) && (g_salvoResults & 0x80)) {
        uint16_t rc = getCursorCell();
        collectTouchingShips(RAM(g_cpuFleetGrid), rc >> 8, rc);
        if (g_salvoCount != 1) {
            g_gameATurnState = 0x01;
            g_gameATurnTimer = 0x20;
            g_salvoLaunched++;
            for (c = 0; c < 6; c++)
                g_salvoShipsHit[c] = 0xff;
            // The list of ships the salvo hits, each ship once.
            c = g_salvoCount;
            const uint8_t *s = RAM(g_salvoShots) + 1;
            do {
                uint8_t *h = g_salvoShipsHit;
                for (;;) {
                    if (*h == 0xff) {
                        *h = *s;
                        break;
                    }
                    if (*h == *s)
                        break;
                    h++;
                }
                s += 2;
            } while (--c);
            goto flags;
        }
    }
    g_gameATurnState = 0x0e;
flags:
    g_gameFlags5 &= ~0x08;
    uint8_t a = g_gameFlags4 & 0x03;
    if (a) {
        g_gameFlags9 |= a;
        g_gameFlags4 &= 0xfc;
    }
    c = g_salvoResultCount;
    r = RAM(g_salvoResults);
    do {
        a = *r++ & 0x0c;
        if (a)
            goto lucky;
        r++;
    } while (--c);
    return;
lucky:
    g_gameFlags4 |= a >> 2;
    farCopy(1, RAM(g_vramQueue), GameA_AircraftDirOffsets + 0x12, 0x0a);
    g_gameATurnTimer = 0x40;
    g_gameATurnState = 0x0b;
    if (g_gameFlags9 & 0x01)
        return;
    g_gameFlags6 &= ~0x40;
}

// Turn step 7: build the shot sprites, resolve the shots, go to step 4.
static void gaTurn07StartFlight(void)
{
    gaSpriteStrips(GA_LEVEL3 ? GameA_ShipCellIdRange + 10 : GameA_PlacementSpriteLayout + 0x3e,
                   0x1002, OAM);
    g_animScriptCount = 0x01;
    g_animSpritesPerScript = 0x03;
    g_animFirstOamSlot = 0x06;
    RAM(g_animScriptPtrs)[0] = 0xec;
    RAM(g_animScriptPtrs)[1] = 0x63;
    fillBytes(RAM(g_animRecords) - 1, 0, 0x24);
    g_gameFlags6 |= 0x40;
    g_gameATurnState = 0x04;
    resolveShots();
    g_shotSpritesLaunched = 0;
    g_shotLaunchDelay = 0;
    uint8_t c = g_salvoResultCount;
    g_gameATurnTimer = c;
    uint8_t *p = OAM + 0x18;
    do {
        uint8_t e = GA_LEVEL3 ? 0x60 : 0x70;
        uint8_t y = 0x88, t = 0x5d;
        for (uint8_t b = 3; b; b--) {
            p[0] = y;
            p[1] = e;
            p[2] = t;
            p += 4;
            t++;
            y += 8;
        }
    } while (--c);
}

// Turn step 9: flash the hit ships, then go to step $0f or 8.
static void gaTurn09HitFlash(void)
{
    if (g_vramCopyCount)
        return;
    if (!g_hitFlashStarted) {
        g_hitFlashStarted++;
        uint8_t *q = RAM(g_vramQueue);
        q[0x0c] = 0;
        q[0] = 0x99;
        q[1] = 0x06;
        q[2] = 0x09;
        farCopy(1, q + 3, GameA_ArrangeFleetWindowTilemap + 0x33, 9);
        g_gameFlags3 |= 0x04;
    }
    uint8_t d = 0, e = 0;
    if (!--g_blinkTimer) {
        d++;
        g_blinkTimer = 0x10;
        if (!--g_blinkCount) {
            ClearShadowOAM();
            clearAnimScripts();
            g_gameATurnState = 0x0f;
            uint8_t *s = RAM(g_shipNumToShow);
            for (uint8_t i = 0; i < 4; i++)
                s[i] = s[i + 1];
            if (--g_spriteCount)
                return;
            g_gameATurnState = 0x08;
            uint8_t n = countNonZero5(g_p2ShipHitsLeft);
            if (n == 1) {
                g_gameFlags9 |= 0x60;
                return;
            }
            if (n || !g_replyShotFlag)
                return;
            g_gameALevel |= 0x10;
            return;
        }
    }
    if (GA_LEVEL3)
        return;
    if (!(g_blinkTimer & 0x03))
        e++;
    uint8_t *p = OAM + 0x28;
    for (uint8_t c = 4; c; c--, p += 4) {
        p[0] += d;
        p[1] ^= e;
    }
}

// Turn step $0a: draw the sunk ship onto the grid one row at a time,
// then the counters, and go to step $0c.
static void gaTurn0ARevealSunk(void)
{
    if (g_vramCopyCount)
        return;
    gaDrawStreamToBgMap(RAM(g_playerShotGrid));
    uint8_t c = (uint8_t)(0x0c - g_gridSize) >> 1;
    uint8_t b = 0x0f - c;
    c += 3;
    uint8_t t = g_gameATurnTimer;
    if (t >= c && t < b) {
        const uint8_t *s = GRID_CELL(RAM(g_playerShotGrid), t - c, 0);
        uint8_t *q = RAM(g_vramQueue) + (uint8_t)(c - 1);
        for (uint8_t n = g_gridSize; n; n--, q++) {
            uint8_t v = *s++;
            if (v)
                *q = (uint8_t)((v - 1) << 1) + 0xb6;
        }
    }
    if (g_gameATurnTimer == 0x11) {
        gaWriteTileRuns(RAM(g_vramQueue), GameA_TurnCounterRuns);
        drawNumber3(RAM(g_vramQueue) + 6, g_turnCount);
        uint8_t n = countNonZero5(g_p2ShipHitsLeft);
        if (n) {
            gaWriteTileRuns(RAM(g_vramQueue), GameA_EnemyCountRuns);
            RAM(g_vramQueue)[0x12] = n + 0xd8;
        } else {
            gaWriteTileRuns(RAM(g_vramQueue), GameA_NoEnemyTextRuns);
        }
        resetStatEffectB();
        g_gameATurnState = 0x0c;
        g_gameATurnTimer = 0x2e;
        g_shipSpriteBlinkTimer = 0x10;
    }
    g_vramCopyCount++;
}

// Turn step $0b: the lucky shot aircraft fly over, then go to $0e.
static void gaTurn0BShowSunk(void)
{
    if (g_gameFlags6 & 0x40)
        return;
    uint8_t *p;
    uint8_t c;
    if (g_gameATurnTimer == 0x40) {
        p = OAM + 0x40;
        for (c = 5; c; c--, p += 4) {
            p[0] = 0x40;
            p[1] = 0x3c;
            p[2] = 0x6b;
            p[3] = 0;
        }
        uint8_t tiles[16];
        farCopy(1, tiles, GameA_AircraftDirOffsets + 0x1c, 16);
        p = OAM;
        uint8_t x = 0xd8;
        for (c = 0; c < 16; c++, p += 4) {
            p[0] = 0x48;
            p[1] = x;
            p[2] = tiles[c];
            p[3] = 0x10;
            x += 8;
        }
        SFX_Queue = 0x04;
        rOBP1 = 0xd8;
        rOBP0 = 0x36;
        if (!(g_gameFlags4 & 0x01))
            rOBP0 = 0xc6;
    }
    // 5 timers at g_vramQueue (step, counter): on each carry, move
    // one sprite by the direction offsets.
    uint8_t steps[10];
    farCopy(1, steps, GameA_AircraftDirOffsets + 8, 10);
    uint8_t *q = RAM(g_vramQueue);
    p = OAM + 0x40;
    for (c = 0; c < 5; c++, q += 2, p += 4) {
        uint16_t sum = q[0] + q[1];
        q[0] = sum;
        if (sum > 0xff) {
            p[0] += steps[c << 1];
            p[1] += steps[(c << 1) + 1];
        }
    }
    if (g_gameATurnTimer >= 0x24 && !(g_gameATurnTimer & 0x03)) {
        p = OAM + 0x28;
        for (c = 6; c; c--, p += 4)
            (*p)--;
    }
    if (--g_gameATurnTimer)
        return;
    ClearShadowOAM();
    g_gameATurnState = 0x0e;
}

// Turn step $0c: show the shot pattern, blink, then go to state $11.
static void gaTurn0CCheckWin(void)
{
    if (g_gameFlags2 & 0x04)
        return;
    if (g_gameATurnTimer == 0x2e) {
        g_gameALevel |= 0x02;
        uint16_t rc = getCursorCell();
        uint8_t n = 1;
        if ((g_gameFlags9 & 0x03) && !(g_gameFlags9 & 0x02)) {
            n = 5;
            if ((g_p1Options & 0x03) == 0x02)
                n = 9;
        }
        int8_t offs[18];
        farCopy(1, (uint8_t *)offs, GameA_TargetPatternOffsets, 18);
        uint8_t *p = OAM;
        const int8_t *o = offs;
        for (; n; n--, p += 4, o += 2) {
            *p = 0xf0;
            uint8_t row = (rc >> 8) + o[0];
            if (gaCheckCoordOutsideGrid(row))
                continue;
            uint8_t col = (uint8_t)rc + o[1];
            if (gaCheckCoordOutsideGrid(col))
                continue;
            uint16_t yx = gaGridCellToOamXY(row, col);
            p[0] = yx >> 8;
            p[1] = yx;
            p[2] = (g_gameFlags9 & 0x03) ? 0x6b : 0x6a;
        }
    }
    if (!--g_shipSpriteBlinkTimer) {
        g_shipSpriteBlinkTimer = 0x10;
        rOBP0 ^= 0xc0;
    }
    if (--g_gameATurnTimer)
        return;
    rOBP0 = 0x27;
    g_gameFlags3 &= ~0x20;
    rWY = 0;
    g_gameATurnTimer = 0;
    rLCDC |= 0x20;
    g_gameAState = 0x11;
}

// Turn step $0d: start the next shot, go to step 7.
static void gaTurn0DNextShot(void)
{
    if (g_gameFlags2 & 0x04)
        return;
    g_gameALevel &= ~0x02;
    g_gameADelay = 0x1e;
    g_gameATurnState = 0x07;
    if (g_gameFlags4 & 0x03)
        return;
    rLYC = 0x2f;
    g_statEffect1Arg = 0x09;
    RAM(g_statEffect1Arg)[1] = 0;
    g_gameAStatEffect = 0x02;
    g_gameFlags2 |= 0x04;
}

// Turn step $0e: end of the turn, go to step 6.
static void gaTurn0EEndTurn(void)
{
    if (g_gameFlags6 & 0x40)
        return;
    if (g_p1Options & 0x40) {
        g_p1Options &= ~0x40;
        SFX_Queue = 0x15;
    }
    g_gameADelay = 0x4e;
    g_gameATurnState = 0x06;
}

// Turn step $0f: queue the hit tiles, start the hit animation, go to 9.
static void gaTurn0FHitAnim(void)
{
    uint8_t lvl = g_gameALevel & 0xc0;
    uint8_t t = lvl == 0x40 ? 0xa6 : lvl == 0x80 ? 0x6a : 0x92;
    RAM(g_vramQueue)[0] = t;
    RAM(g_vramQueue)[1] = t;
    uint16_t dst = 0x98ca;
    if (g_gameFlags9 & 0x03) {
        uint8_t h = g_shipNumToShow >> 1;
        dst = 0x98c3 + (uint8_t)(h * 3);
    }
    uint8_t *e = gaQueueVramCopy(RAM(g_vramQueue), dst);
    e[0] = 0x02;
    e[1] = 0x01;
    e[2] = 0x02;
    g_vramCopyCount++;
    g_animScriptCount = 0;
    g_animSpritesPerScript = 0x04;
    g_animFirstOamSlot = 0x0a;
    uint8_t x = 0x20;
    if (g_gameFlags9 & 0x03)
        x = (uint8_t)multiply8(g_shipNumToShow, 12) + 0x20;
    uint8_t off = (uint8_t)(g_animFirstOamSlot + (uint8_t)multiply8(g_animScriptCount, g_animSpritesPerScript)) << 2;
    setMetaspritePos2x2(OAM + off, x, 0x38, 8);
    *animGetNextRecordPtr() = 0x02;
    g_animScriptCount++;
    uint8_t *p = OAM;
    for (uint8_t c = 10; c; c--, p += 4)
        *p = 0x48;
    g_gameFlags6 |= 0x40;
    g_blinkTimer = 0x10;
    g_blinkCount = 0x08;
    g_gameATurnState = 0x09;
    g_hitFlashStarted = 0;
}

void gameATurnState(void)
{
    switch (g_gameATurnState) {
    case 0x00: gaTurn00Aim(); break;
    case 0x01: gaTurn01LaunchShots(); break;
    case 0x02: gaTurn02DrawSea(); break;
    case 0x03: gaTurn03Delay(); break;
    case 0x04: gaTurn04FlyShots(); break;
    case 0x05: gaTurn05ShotAnim(); break;
    case 0x06: turn06ClearOAM(); break;
    case 0x07: gaTurn07StartFlight(); break;
    case 0x08: turn08ResetScroll(); break;
    case 0x09: gaTurn09HitFlash(); break;
    case 0x0a: gaTurn0ARevealSunk(); break;
    case 0x0b: gaTurn0BShowSunk(); break;
    case 0x0c: gaTurn0CCheckWin(); break;
    case 0x0d: gaTurn0DNextShot(); break;
    case 0x0e: gaTurn0EEndTurn(); break;
    case 0x0f: gaTurn0FHitAnim(); break;
    }
}
