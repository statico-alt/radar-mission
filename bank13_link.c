// Link cable protocol for Game A two-player play.
// The serial interrupt runs linkSerialHandler. g_serialRecv holds the byte
// just received. The byte in g_serialSend goes out next.
// g_gameALinkState picks the handler. Bank 5 calls this bank through FAR_CALL_C.
#include "game.h"

#define FLEET_GRID (&g_playerFleetGrid)
#define LINK_FLEET_GRID (&g_linkFleetGrid)
#define LUCKY_BUFFER g_luckySquaresP1 // 16 bytes: the P1 and P2 squares
#define CELL_LIST (&g_cellList)

// The value that marks the cells of the current player's ship being moved:
// 1 + the sum of that player's 5 ship lengths (bank 0 SumFleetBytes).
static uint8_t movedShipId(void)
{
    const uint8_t *len = g_currentPlayer ? g_p2ShipLengths : g_p1ShipLengths;
    uint8_t id = 1;
    for (uint8_t i = 0; i < 5; i++)
        id += len[i];
    return id;
}

// Find the first cell of the grid whose ship id is movedShipId(), and clear it.
static void clearShipCellById(uint8_t *grid)
{
    uint8_t id = movedShipId();
    while ((*grid & 0x3f) != id)
        grid++;
    *grid = 0;
}

// Store movedShipId() | g_shipDirBits in the grid cell at row, col.
static void setGridCellShip(uint8_t *grid, uint8_t row, uint8_t col)
{
    grid[multiply8(row, g_gridSize) + col] = g_shipDirBits | movedShipId();
}

// The cursor cell packed as row << 4 | column.
static uint8_t packedCursor(void)
{
    return SWAP(g_cursorRow) | g_cursorCol;
}

// Things to do when the link comes up: the timer IRQ plays the start sound.
static void linkUp(void)
{
    g_gameFlags3 |= 0x08;
    g_titleStartSfx = 2;
    SFX_Queue = 7;
    BGM_Queue = 0xff;
}

// State $00: handshake. Each side sends $fe.
// Received $fd: the other Game Boy is master. Set link mode bit 6, wait, go to state $15.
// Received $fe: we are master. Send our mode bits and go to state $0b.
// Anything else: keep sending $fe. If the player cancels (level bit 3), return to the title.
static void linkState00(void)
{
    uint8_t recv = g_serialRecv;
    if (recv == 0xfd) {
        g_topModeFlags |= 0x40;
        // A short busy wait (the same loop as the asm).
        for (uint8_t i = 0x69; i; i--)
            __asm__("nop\n nop\n nop\n nop\n nop");
        g_gameALinkState = 0x15;
        rTIMA = 0;
        linkUp();
        return;
    }
    if (recv != 0xfe) {
        rSB = 0xfe;
        if (!(g_gameALevel & 0x08))
            return;
        g_gameALevel &= ~0x08;
        (&g_oamBuffer)[5] = 0x2e;
        g_topModeFlags &= 0x3b;
        NoiseSfx_Queue = 5;
        g_topState = 1;
        return;
    }
    g_topModeFlags |= 0x04;
    g_serialSend = g_topModeFlags & 0xa0;
    linkUp();
    g_gameALinkState = 0x0b;
    rTIMA = 0;
}

// State $01: exchange the fleet grids.
// Sends one byte of g_playerFleetGrid per transfer (g_linkByteIndex = byte index) and stores the
// other player's bytes at g_linkFleetGrid. After $91 bytes, go to state $02 with g_gameATurnTimer = $60.
// Before that it syncs on the $f0 marker byte with g_gameFlags2 bit 6 and g_gameFlags3 bit 0.
static void linkState01(void)
{
    if (!(g_gameALevel & 0x01)) {
        uint8_t old = g_linkSyncBytes[0];
        g_linkSyncBytes[0] = g_linkSyncBytes[1];
        g_linkSyncBytes[1] = 0;
        if (old == 0xf0)
            g_gameFlags2 |= 0x40;
        if (g_serialRecv == 0xf0) {
            g_gameFlags3 |= 0x01;
            if (!(g_gameFlags2 & 0x40)) {
                rTMA = 0x50;
                rTAC = 0xff;
                g_topModeFlags |= 0x01;
                return;
            }
        } else {
            if (!(g_gameFlags2 & 0x40))
                return;
            if (!(g_gameFlags3 & 0x01)) {
                g_topModeFlags &= ~0x01;
                return;
            }
        }
        g_gameALevel |= 0x01;
    } else if (g_linkByteIndex >= 2) {
        LINK_FLEET_GRID[g_linkByteIndex] = g_serialRecv;
        if (g_linkByteIndex == 0x91) {
            g_gameALinkState = 0x02;
            g_gameATurnTimer = 0x60;
            g_linkByteIndex = 0;
            g_gameAState = 0;
            g_gameFlags2 |= 0x02;
            return;
        }
    }
    if (g_linkByteIndex < 0x90)
        g_serialSend = FLEET_GRID[g_linkByteIndex];
    g_linkByteIndex++;
}

// State $02: idle. A received byte $01-$15 is the next link state (a command from the other side).
static void linkState02(void)
{
    uint8_t recv = g_serialRecv;
    if (recv && recv < 0x16)
        g_gameALinkState = recv;
}

// States $03 and $0e: send our cursor cell. Then state $07.
static void linkState03(void)
{
    g_serialSend = packedCursor();
    g_gameALinkState = 0x07;
}

// State $04: receive the other player's shot cell into g_targetRow/Col.
// Sets turn sub-state g_gameACpuState = 6 (fire) and returns to state $02.
static void linkState04(void)
{
    g_targetRow = SWAP(g_serialRecv) & 0x0f;
    g_targetCol = g_serialRecv & 0x0f;
    g_gameACpuState = 0x06;
    g_gameALinkState = 0x02;
}

// State $05: send the cell in g_linkCellRow/Col. Then state $07.
static void linkState05(void)
{
    g_serialSend = SWAP(g_linkCellRow) | g_linkCellCol;
    g_gameALinkState = 0x07;
}

// State $06: receive a cell. If g_gameFlags6 bit 2 is set, move a ship there on our grid.
// Otherwise store it in g_linkCellRow/Col ($ff = none) and set g_revealFlag = 1. Then state $02.
static void linkState06(void)
{
    uint8_t recv = g_serialRecv;
    if (g_gameFlags6 & 0x04) {
        clearShipCellById(FLEET_GRID);
        setGridCellShip(FLEET_GRID, SWAP(recv) & 0x0f, recv & 0x0f);
    } else {
        g_gameFlags6 |= 0x02;
        if (recv == 0xff) {
            g_linkCellRow = 0xff;
            g_linkCellCol = 0xff;
        } else {
            g_linkCellRow = SWAP(recv) & 0x0f;
            g_linkCellCol = recv & 0x0f;
            g_revealFlag = 1;
        }
    }
    g_gameALinkState = 0x02;
}

// State $07: the byte was sent. Return to idle state $02.
static void linkState07(void)
{
    g_gameALinkState = 0x02;
}

// States $08 (link stopped) and $0c (unused): ignore all bytes.
static void linkStateNone(void)
{
}

// State $09: go to state $0a.
static void linkState09(void)
{
    g_gameALinkState = 0x0a;
}

// State $0a: start BGM $05 and return to idle.
static void linkState0A(void)
{
    BGM_Queue = 0x05;
    g_gameALinkState = 0x02;
}

// State $0b: the slave copies the master's mode bits (bits 5 and 7 of g_topModeFlags).
// Starts Game A (top state 2; Game A state 2 unless bit 5 is set). Then state $08.
static void linkState0B(void)
{
    if (g_topModeFlags & 0x40)
        g_topModeFlags = (g_topModeFlags & 0x5f) | g_serialRecv;
    g_topState = 0x02;
    if (!(g_topModeFlags & 0x20))
        g_gameAState = 0x02;
    g_gameALinkState = 0x08;
}

// State $0d: store the received byte in g_linkRecvC9dd and set g_p1Options bit 7. Return to idle.
static void linkState0D(void)
{
    g_linkRecvC9dd = g_serialRecv;
    g_p1Options |= 0x80;
    g_gameALinkState = 0x02;
}

// State $0f: send the 2 bytes of g_linkSendPair. Then count g_linkPairCount up and return to idle.
static void linkState0F(void)
{
    g_serialSend = g_linkSendPair[g_linkByteIndex];
    if (++g_linkByteIndex != 2)
        return;
    g_linkByteIndex = 0;
    g_linkPairCount++;
    g_gameALinkState = 0x02;
}

// State $10: receive 16 bytes into the Lucky Shot squares. Then swap the P1 and P2 halves
// (through g_cellList).
static void linkState10(void)
{
    LUCKY_BUFFER[g_linkByteIndex] = g_serialRecv;
    if (++g_linkByteIndex != 0x10)
        return;
    g_linkByteIndex = 0;
    copyBytes(CELL_LIST, LUCKY_BUFFER, 0x10);
    copyBytes(g_luckySquaresP1, CELL_LIST + 8, 8);
    copyBytes(g_luckySquaresP2, CELL_LIST, 8);
    g_gameALinkState = 0x02;
}

// State $11: send the 16 bytes of the Lucky Shot squares. Then state $07.
static void linkState11(void)
{
    g_serialSend = LUCKY_BUFFER[g_linkByteIndex];
    if (++g_linkByteIndex != 0x10)
        return;
    g_linkByteIndex = 0;
    g_gameALinkState = 0x07;
}

// State $12: receive 2 bytes into g_linkRecvPair. After the first pair, send our pair
// (state $0f, sending $12). After the second pair (g_linkPairCount = 2) return to idle.
static void linkState12(void)
{
    g_linkRecvPair[g_linkByteIndex] = g_serialRecv;
    if (++g_linkByteIndex != 2)
        return;
    g_linkByteIndex = 0;
    g_gameALinkState = 0x02;
    if (++g_linkPairCount == 2)
        return;
    g_gameALinkState = 0x0f;
    g_serialSend = 0x12;
}

// State $13: store the received byte in g_linkRecvC9d1 and set g_gameFlags8 bit 1.
static void linkState13(void)
{
    g_linkRecvC9d1 = g_serialRecv;
    g_gameFlags8 |= 0x02;
}

// State $14: the link ended. Clear the link mode bits, send sound command $05, stop the
// link (state $08), set the delay g_gameADelay = 8 and set g_gameFlags3 bit 1.
static void linkState14(void)
{
    g_topModeFlags &= 0xbb;
    SoundCmd_Queue = 0x05;
    g_gameALinkState = 0x08;
    g_gameADelay = 0x08;
    g_gameFlags3 |= 0x02;
}

// State $15: go to state $0b.
static void linkState15(void)
{
    g_gameALinkState = 0x0b;
}

static void (*const linkStates[])(void) = {
    linkState00, linkState01, linkState02, linkState03,
    linkState04, linkState05, linkState06, linkState07,
    linkStateNone, linkState09, linkState0A, linkState0B,
    linkStateNone, linkState0D, linkState03, linkState0F,
    linkState10, linkState11, linkState12, linkState13,
    linkState14, linkState15,
};

// Serial interrupt handler for link play (called from the bank 0 serial IRQ).
void linkSerialHandler(void)
{
    linkStates[g_gameALinkState]();
}
