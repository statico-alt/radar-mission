// Game A options menu: grid size, AIRCRAFT, NEAR MISS, LUCKY SHOT, commander and READY.
// In link play each Game Boy moves its own cursor. The other player's buttons
// come in through the link (g_linkRecvC9d1).
#include "game.h"

extern const uint8_t GridSizeDigitTiles[];
extern const uint8_t GridSizeTileQueueTemplate[];
extern const uint8_t OptionsCursorPositions[];

// bank5_screens.c

#define OAM (&g_oamBuffer)
#define LINK_PLAY (g_topModeFlags & 0x80)

// The VRAM queue at the current write offset.
static uint8_t *queueEnd(void)
{
    return &g_vramQueue + g_vramQueueIndex;
}

// End a queue record of n bytes: draw the queue in VBlank, play SFX 2.
static void queueDone(uint8_t n)
{
    g_vramQueueIndex += n;
    g_gameFlags3 |= 0x04;
    SFX_Queue = 2;
}

// Place the current player's cursor sprite for the selected row.
// Positions come from OptionsCursorPositions (3 bytes: Y, X, attr; 6 rows per player).
static void placeCursor(void)
{
    uint8_t player = g_currentPlayer;
    uint8_t row = g_optionsCursorRow[player];
    const uint8_t *pos = OptionsCursorPositions + (uint8_t)((player << 4) + (player << 1))
                         + (uint8_t)((row << 1) + row);
    uint8_t *oam = OAM;
    if (player && LINK_PLAY)
        oam += 4;
    oam[0] = pos[0];
    oam[1] = pos[1];
    oam[2] = 0x6d;
    oam[3] = pos[2];
    SFX_Queue = 1;
}

// Down: move the cursor one row down (rows 0-5; 5 = START).
static void optionsDown(void)
{
    uint8_t *row = &g_optionsCursorRow[g_currentPlayer];
    if (*row == 5)
        return;
    ++*row;
    placeCursor();
}

// Up: move the cursor one row up.
static void optionsUp(void)
{
    uint8_t *row = &g_optionsCursorRow[g_currentPlayer];
    if (!*row)
        return;
    --*row;
    placeCursor();
}

// Left: switch the cursor to the player 1 column (clear g_p1Options bit 5).
// Ignored in link play. Copies player 2's row to player 1's row.
static void optionsLeft(void)
{
    if (LINK_PLAY || !(g_p1Options & 0x20))
        return;
    g_p1Options &= ~0x20;
    g_optionsCursorRow[0] = g_optionsCursorRow[1];
    g_currentPlayer = 0;
    placeCursor();
}

// Right: switch the cursor to the player 2 column (set g_p1Options bit 5).
// Ignored in link play. Copies player 1's row to player 2's row.
static void optionsRight(void)
{
    if (LINK_PLAY || g_p1Options & 0x20)
        return;
    g_p1Options |= 0x20;
    g_optionsCursorRow[1] = g_optionsCursorRow[0];
    g_currentPlayer = 1;
    placeCursor();
}

// Row 0: cycle the grid size setting (g_p1Options bits 0-1: 8, 10 or 12 cells)
// and queue the new digit tiles.
static void optionsGridSize(void)
{
    uint8_t *q = queueEnd();
    copyBytes(q, GridSizeTileQueueTemplate, 0x0d);
    uint8_t size = (g_p1Options & 0x03) + 1;
    if (size == 3)
        size = 0;
    g_p1Options = (g_p1Options & 0xfc) | size;
    g_p2Options = (g_p2Options & 0xfc) | size;
    uint8_t tile = GridSizeDigitTiles[size];
    q[3] = tile;
    q[5] = tile;
    q[9] = tile;
    q[11] = tile;
    queueDone(0x0c);
}

// Toggle setting bit in g_p1Options (P1) or g_p2Options (P2) and queue the
// ON/OFF tiles at VRAM address vram (the P2 column is 15 tiles right).
static void optionsToggle(uint16_t vram, uint8_t bit)
{
    uint8_t player = g_currentPlayer;
    if (player)
        vram += 0x0f;
    uint8_t *q = queueEnd();
    q[0] = vram >> 8;
    q[1] = vram;
    q[2] = 3;
    q[3] = 0;
    q[4] = 0;
    q[5] = 0;
    uint8_t *options = player ? &g_p2Options : &g_p1Options;
    *options ^= bit;
    if (*options & bit) {
        uint8_t *on = player ? q + 3 : q + 4;
        on[0] = 0xf5;
        on[1] = 0xf4;
    } else {
        q[3] = 0xf5;
        q[4] = 0xec;
        q[5] = 0xec;
    }
    q[6] = 0;
    queueDone(6);
}

// Put a block of 4 sprites per row at Y $68, X x. Each sprite gets
// tile (counting up when step is 1) and attr.
static uint8_t *putSpriteBlock(uint8_t *oam, uint8_t rows, uint8_t x, uint8_t tile,
                               uint8_t step, uint8_t attr)
{
    for (uint8_t y = 0x68; rows; rows--, y += 8) {
        for (uint8_t n = 4, xx = x; n; n--, xx += 8) {
            *oam++ = y;
            *oam++ = xx;
            *oam++ = tile;
            *oam++ = attr;
            tile += step;
        }
    }
    return oam;
}

// Row 4: cycle the commander/fleet type (g_p1Commander or g_p2Commander) and
// redraw its portrait sprites and name. A second change in one frame sets
// g_gameFlags5 bit 7 instead.
static void optionsCommander(void)
{
    if (++g_commanderChangeCount == 2) {
        g_gameFlags5 |= 0x80;
        return;
    }
    rOBP0 = 0x27;
    uint8_t player = g_currentPlayer;
    uint8_t *commander = &g_p1Commander + player;
    uint8_t x = player ? 0x80 : 0x10;
    // The portrait of the old commander: 4x4 sprites.
    putSpriteBlock(OAM + 0x30, 4, x, (uint8_t)(*commander << 4) + 0x80, 1, 0x10);
    // The next commander: 0-4 in link play. Else P1 has 3-4 and P2 has 0-2.
    uint8_t first = 0, end = 5;
    if (!LINK_PLAY) {
        if (player)
            end = 3;
        else
            first = 3;
    }
    uint8_t next = *commander + 1;
    if (next == end)
        next = first;
    *commander = next;
    // Its name: 4 rows of 4 tiles.
    uint8_t *q = queueEnd();
    uint16_t vram = player ? 0x996f : 0x9961;
    uint8_t tile = (uint8_t)(next << 4) + 0x80;
    for (uint8_t rows = 4; rows; rows--, vram += 0x20) {
        *q++ = vram >> 8;
        *q++ = vram;
        *q++ = 4;
        for (uint8_t n = 4; n; n--)
            *q++ = tile++;
    }
    *q = 0;
    g_vramQueueIndex += 0x1c;
    g_gameFlags3 |= 0x04;
    // Cover sprites (tile $fe), 3 rows of 4.
    putSpriteBlock(OAM + 0x70, 3, x, 0xfe, 0, 0);
    g_gameATurnTimer = 0x22;
    g_gameAState = 0x0c;
    SFX_Queue = 2;
}

// Row 5: mark this player ready (g_p2Options bit 5 = P1, bit 6 = P2 in link play).
static void optionsReady(void)
{
    g_p2Options |= LINK_PLAY && g_currentPlayer ? 0x40 : 0x20;
    SFX_Queue = 5;
}

// A button: act on the selected menu row.
static void optionsA(void)
{
    switch (g_optionsCursorRow[g_currentPlayer]) {
    case 0: optionsGridSize(); break;
    case 1: optionsToggle(0x9861, 0x08); break; // AIRCRAFT
    case 2: optionsToggle(0x98a1, 0x04); break; // NEAR MISS
    case 3: optionsToggle(0x9901, 0x10); break; // LUCKY SHOT
    case 4: optionsCommander(); break;
    default: optionsReady(); break;
    }
}

// Act on the menu buttons of player. Only the first pressed button counts,
// in this order: Down, Up, Left, Right, Start, A.
static void handleButtons(uint8_t player, uint8_t pressed)
{
    g_currentPlayer = player;
    if (pressed & 0x80)
        optionsDown();
    else if (pressed & 0x40)
        optionsUp();
    else if (pressed & 0x20)
        optionsLeft();
    else if (pressed & 0x10)
        optionsRight();
    else if (pressed & 0x08) {
        // Start: the same as A on the START row (row 5).
        if (g_optionsCursorRow[player] == 5)
            optionsReady();
    } else if (pressed & 0x01)
        optionsA();
}

// Every $18 frames, flash the cursor sprite of a player who is ready.
static void blinkReadyCursor(void)
{
    uint8_t ready = g_p2Options & 0x60;
    if (!ready || --g_gameABlinkTimer)
        return;
    g_gameABlinkTimer = 0x18;
    OAM[ready & 0x20 ? 0 : 4] ^= 0x90;
}

// Per-frame update of the options menu. When both players are ready, start the
// timer and go to Game A state $20.
void optionsMenuUpdate(void)
{
    if (LINK_PLAY) {
        // Wait for the other side's buttons.
        if (!(g_gameFlags8 & 0x02))
            return;
        g_gameFlags8 &= ~0x02;
    }
    if (LINK_PLAY && g_gameFlags5 & 0x80) {
        g_gameFlags5 &= ~0x80;
        g_linkSyncBytes[0] = 1;
    } else {
        // Joypad history (c9d3-c9d6): the presses are used 2 frames late, so
        // both Game Boys act on them in the same frame. The new presses go out
        // on the link.
        uint8_t *pad = g_linkSyncBytes;
        pad[0] = pad[2];
        pad[2] = pad[3];
        uint8_t joy = g_joyRaw;
        pad[3] = joy & ~pad[1];
        pad[1] = joy;
        g_serialSend = pad[3];
        g_vramQueueIndex = 0;
        g_commanderChangeCount = 0;
        if (!LINK_PLAY) {
            handleButtons(g_p1Options & 0x20 ? 1 : 0, pad[0]);
            goto done;
        }
        blinkReadyCursor();
        if (!(g_p2Options & 0x40))
            handleButtons(1, g_linkRecvC9d1);
    }
    if (!(g_p2Options & 0x20))
        handleButtons(0, g_linkSyncBytes[0]);
done:
    g_linkRecvC9d1 = 0;
    if ((g_p2Options & 0x60) != 0x60)
        return;
    rTMA = 0xb8;
    rTAC = 0xfc;
    OAM[0] = 0x90;
    OAM[4] = 0x90;
    g_gameAState = 0x20;
    g_gameADelay = 0x48;
}
