// Game A results screen, board drawing state, cutscene scripts and ship
// record drawing. They read bank 1 data.
#include "bank1.h"

// Bank 0 asm. SoftReset does not return.
void SoftReset(void);
// Bank 0 asm wrappers in bank_001.asm.
uint8_t rotateShipTileStep(void);

// Results screen: one commander face anim. t[0] counts down. At 0 it
// reloads from t[1] and draws frame t[2] (0 or 1) of the face. The 16
// tiles come from GameA_AnimSlotFrameTiles + commander * $40, + $20 for
// half xor $ffaf bit 0, + frame * $10. tileAdd is added to each tile.
static void tickAnimSlot(uint8_t *t, uint8_t *oam, uint8_t tileAdd, uint8_t half, uint8_t commander)
{
    if (--t[0])
        return;
    t[0] = t[1];
    const uint8_t *src = GameA_AnimSlotFrameTiles + ((uint16_t)commander << 6)
        + (uint8_t)((((g_gameFlags8 & 0x01) ^ half) << 5) + (t[2] << 4));
    for (uint8_t n = 16; n; n--, oam += 4)
        *oam = *src++ + tileAdd;
    t[2] ^= 0x01;
}

// Results screen idle timer ($c9c8-$c9ca). Not in link play or with
// $ffaf bit 0. When it runs out, show the "CONGRATULATIONS" banner
// ($ffae bit 7). The banner then moves right 1 pixel a frame and its
// text goes to the tilemap 1 letter at a time.
static void tickIdleTimer(void)
{
    if (g_topModeFlags & 0x80)
        return;
    if (g_gameFlags8 & 0x01)
        return;
    uint8_t *t = &g_idleTimer;
    uint8_t *s = OAM + 0x81;
    if (!(g_gameFlags6 & 0x80)) {
        if (t[0]) {
            t[0]--;
            return;
        }
        if (!--t[1]) {
            g_gameFlags6 |= 0x80;
            t[0] = 0x40;
            t[1] = 0xd8;
            t[2] = 0x04;
            buildSpriteStrips(GameA_CongratsBannerSprite, 5, OAM + 0x80);
            rOBP0 = 0xe4;
            if (g_p1HitRank != 1)
                g_gameALevel |= 0x04;
            return;
        }
        if (!*s)
            NoiseSfx_Queue = 0x0c;
        for (uint8_t n = 6; n; n--, s += 4)
            *s += 2;
        return;
    }
    if (g_gameALevel & 0x04)
        return;
    if (t[0]) {
        t[0]--;
        return;
    }
    if (!--t[1])
        g_gameALevel |= 0x04;
    if (!--t[2]) {
        t[2] = 0x04;
        OAM[0x8e] ^= 0x06;
    }
    uint8_t x = *s;
    if (x == 0xe8)
        NoiseSfx_Queue = 0x0b;
    for (uint8_t n = 5; n; n--, s += 4)
        (*s)++;
    if (x < 0x18 || x >= 0x98 || (x & 0x07))
        return;
    // One letter every 8 pixels, from $9d02.
    uint8_t i = (uint8_t)(x - 0x18) >> 3;
    uint16_t addr = 0x9d02 + i;
    uint8_t *q = &g_vramQueue;
    q[0] = addr >> 8;
    q[1] = addr;
    q[2] = 1;
    q[3] = GameA_CongratulationsText[i];
    q[4] = 0;
    g_gameFlags3 |= 0x04;
}

// Results step 1. Start fades out ($ffab bit 1). When the sound fade is
// done, fade the screen ($ffaa bit 1) and go to step 2. Until then, run
// the face anims and blink the 5 ship rows, and run the idle timer.
void resultsStep1(void)
{
    if (g_gameFlags3 & 0x02) {
        if (!g_soundFadeState) {
            g_gameFlags2 |= 0x02;
            g_gameAResultsStep = 2;
            return;
        }
    } else if (g_joyPressedA & 0x08) {
        g_gameFlags3 |= 0x02;
        SoundCmd_Queue = 0x05;
        if (!(g_topModeFlags & 0x80))
            return;
        g_serialSend = 0x14;
        g_gameALinkState = 0x08;
        g_gameADelay = 0x08;
        if (g_topModeFlags & 0x04)
            return;
        g_topModeFlags &= 0xbb;
        return;
    }
    tickAnimSlot(&g_p1FaceAnimTimer, OAM + 0x02, 0x00, 1, g_p1Commander);
    tickAnimSlot(&g_p2FaceAnimTimer, OAM + 0x42, 0x40, 0, g_p2Commander);
    // 2 bytes per ship row: timer and blink frame.
    uint8_t *t = g_nextLevelShipTiming;
    for (uint8_t c = 0; c < 5; c++, t += 2) {
        if (--t[0])
            continue;
        t[0] = (g_gameFlags8 & 0x01) ? 0x18 : 0x0c;
        t[1] ^= 0x01;
        // 6 tiles: 2 per row of 3 map rows.
        const uint8_t *src = GameA_ResultsBlinkTiles + (t[1] ? 6 : 0) + ((g_gameFlags8 & 0x01) ? 12 : 0);
        uint8_t *q = &g_vramQueue;
        uint16_t addr = 0x9d43 + (uint8_t)((c << 1) + c);
        for (uint8_t r = 0; r < 3; r++, q += 5, addr += 0x20) {
            q[0] = addr >> 8;
            q[1] = addr;
            q[2] = 2;
            q[3] = *src++;
            q[4] = *src++;
        }
        *q = 0;
        g_gameFlags3 |= 0x04;
    }
    tickIdleTimer();
}

// Results step 2: restart the game.
void resultsStep2Reset(void)
{
    SoftReset();
}

// Game A state $0f: every 7 frames show or hide (by $ffb1 bit 0) the
// "ENEMY n" or "NO ENEMY" text at $9e0b, 15 times. Then go to state $0a.
void blinkEnemyCount(void)
{
    if (g_gameFlags3 & 0x04)
        return;
    if (--g_listCount)
        return;
    g_listCount = 0x07;
    uint8_t *q = &g_vramQueue;
    q[0] = 0x9e;
    q[1] = 0x0b;
    q[2] = 0x08;
    fillBytes(q + 3, 0, 9);
    uint8_t t = g_gameATurnTimer;
    if (!(t & 0x01)) {
        if (t != 0x06 && t != 0x0e)
            SFX_Queue = 0x1a;
        if (countNonZero5(g_p2ShipHitsLeft)) {
            copyBytes(q + 3, GameA_EnemyCountText, 7);
            q[0x0a] = 0xd9;
        } else {
            copyBytes(q + 4, GameA_NoEnemyText, 6);
        }
    }
    g_gameFlags3 |= 0x04;
    if (++g_gameATurnTimer != 0x0f)
        return;
    g_gameAState = 0x0a;
    g_gameFlags6 |= 0x10;
    if (countNonZero5(g_p2ShipHitsLeft) && ((g_gameFlags4 & 0x40) || !(g_gameFlags3 & 0x40)))
        BGM_Queue = 0x04;
    g_gameFlags9 &= ~0x40;
    g_gameALevel &= ~0x10;
}

// Game A state 8: when the VRAM copy queue is empty, draw the next board
// row ($ffb1). Row 3 adds the board icons. Row $11 adds the turn count
// and the enemy count. After row $12 go to state $13 (or the game over).
void state08(void)
{
    if (g_vramCopyCount)
        return;
    if (!g_gameATurnTimer)
        initLuckySquares();
    if (g_gameATurnTimer == 0x12) {
        ClearShadowOAM();
        g_boardScroll[0] = 0;
        g_boardScroll[1] = 0;
        g_currentPlayer = 1;
        g_gameADelay = 0x20;
        if (beginGameOver())
            return;
        g_gameAState = 0x13;
        rWY = 0x90;
        g_gameAEndTimer = 0x18;
        g_gameFlags5 &= ~0x40;
        rLCDC |= 0x20;
        return;
    }
    // $ffad bit 6 is set here, so the rows show no ships and the grid
    // is not read.
    drawStreamToBgMap(&g_playerFleetGrid);
    uint8_t *q = &g_vramQueue;
    uint8_t row = g_gameATurnTimer;
    if (row == 0x03) {
        writeTileRuns(q, GameA_BoardIconRuns);
    } else if (row == 0x11) {
        g_gameADelay = 0x0e;
        writeTileRuns(q, GameA_TurnCounterRuns);
        drawNumber3(q + 6, g_turnCount);
        if (!(g_gameFlags9 & 0x40) && !(g_gameALevel & 0x10)) {
            uint8_t n = countNonZero5(g_p2ShipHitsLeft);
            if (n) {
                writeTileRuns(q, GameA_EnemyCountRuns);
                q[0x12] = 0xd8 + n;
            } else {
                writeTileRuns(q, GameA_NoEnemyTextRuns);
            }
        }
    }
    g_vramCopyCount++;
}

// Run the cutscene script at the $ffc4 pointer up to the next sprite.
// $00 ends it (clears $ffab bit 7). $01 y x sets the position. $02 moves
// x 8 pixels. Any other byte is a sprite tile; a $03 n after it sets
// the delay to n (else 8). Each sprite goes 8 pixels right of the last.
void runCutsceneScript(void)
{
    const uint8_t *p = *(const uint8_t **)g_shipTileSrc;
    for (;;) {
        uint8_t op = *p++;
        if (!op) {
            g_gameFlags3 &= ~0x80;
            return;
        }
        if (op == 0x01) {
            g_scriptArgA = *p++;
            g_scriptArgB = *p++;
            continue;
        }
        if (op == 0x02) {
            g_scriptArgB += 8;
            continue;
        }
        g_gameADelay = 0x08;
        if (*p == 0x03) {
            p++;
            g_gameADelay = *p++;
        }
        *(const uint8_t **)g_shipTileSrc = p;
        uint8_t *s = OAM + (uint8_t)(g_scriptSpriteSlot << 2);
        s[0] = g_scriptArgA;
        s[1] = g_scriptArgB;
        s[2] = op;
        s[3] = 0x10;
        g_scriptSpriteSlot++;
        g_scriptArgB += 8;
        NoiseSfx_Queue = 0x08;
        return;
    }
}

// Turn sub-state 8: run the cutscene script ($ffab bit 7). When it ends,
// go to sub-state 4.
void turn08AltHandler(void)
{
    g_gameFlags3 |= 0x80;
    runCutsceneScript();
    if (g_gameFlags3 & 0x80) {
        g_gameFlags3 &= ~0x80;
        return;
    }
    g_gameACpuState = 0x04;
    g_gameADelay = 0x60;
}

// Load the 5-byte record of ship n from table into $ffc3-$ffc7, and its
// direction ($c9a4 + n) into $ffc8.
void loadShipRecord(uint8_t n, const uint8_t *table)
{
    g_placeDirection = (&g_shipRecords)[n];
    copyBytes(&g_shipTileCount, table + (uint8_t)((n << 2) + n), 5);
}

// Load ship n's record, then turn and draw its tiles through the
// tilemap queue until the last one.
static void drawShipTiles(uint8_t n, const uint8_t *table)
{
    loadShipRecord(n, table);
    uint8_t last;
    do {
        last = rotateShipTileStep();
        DrawTilemapCmds(&g_vramQueue);
    } while (!last);
}

// Draw the tiles of ships 0-5 from the records in table.
void drawShipRecords(const uint8_t *table)
{
    for (uint8_t n = 0; n < 6; n++)
        drawShipTiles(n, table);
}

// Scroll steps from margin to cell: the distance << 4, with bit 7 set
// when cell < margin.
uint8_t scrollStepsTo(uint8_t cell, uint8_t margin)
{
    if (cell < margin)
        return ((uint8_t)(margin - cell) << 4) | 0x80;
    return (uint8_t)(cell - margin) << 4;
}
