// Link cable: the state machine, message send and receive, and the
// pad/clock entry. A 1-player game does not run most of this code.
#include "game.h"
#include "bank3.h"

void GameB_TogglePause(void) __banked;

// Queue a for the serial port. Returns the last received byte and clears it.
static uint8_t sendByte(uint8_t a)
{
    g_serialSend = a;
    uint8_t r = g_serialRecv;
    g_serialRecv = 0;
    return r;
}

// Handshake: send d, wait for e. Side 1 ($d002 bit 2) uses the other order.
// Returns 1 until the partner answers.
static uint8_t cableProtocol(uint8_t d, uint8_t e)
{
    if (!(g_sceneFlagsC & 0x04)) {
        if (sendByte(d) != e)
            return 1;
    } else if (!(g_sceneFlagsA & 0x20)) {
        if (sendByte(0) == d)
            g_sceneFlagsA |= 0x20;
        return 1;
    } else if (!(g_sceneFlagsA & 0x10)) {
        g_sceneFlagsA |= 0x10;
        sendByte(0);
        return 1;
    } else if (sendByte(e) != d) {
        return 1;
    }
    g_sceneFlagsA &= ~0x30;
    return 0;
}

// Write 0 to p[0], p[-1] and p[-2].
static void clear3BytesDown(uint8_t *p)
{
    p[0] = 0;
    p[-1] = 0;
    p[-2] = 0;
}

// Increment the link send read index. Wrap at 8.
static void advanceRead(void)
{
    if (++g_linkTxRead >= 8)
        g_linkTxRead = 0;
}

// Header byte: bits 6-7 = (bits 4-5 + bits 0-3) & 3. The receiver checks it.
static uint8_t headerCheck(uint8_t e)
{
    return ((e & 0x0f) + ((e & 0x30) >> 4)) & 0x03;
}

// Receive 0: a header byte. Values 1..$20 start a position message (type 1),
// $21..$29 map to types 2..10.
static void recvHeader(uint8_t c)
{
    if (headerCheck(c) != c >> 6)
        return;
    uint8_t a = c & 0x3f;
    if (!a || a >= 0x2a)
        return;
    if (a >= 0x21) {
        a -= 0x1f;
    } else {
        g_linkRecvPos = a - 1;
        a = 1;
    }
    uint8_t *p = &g_linkRecvType;
    p[0] = a;
    p[1] = 0;
    p[2] = 0;
}

// Receive 1: the partner position. Push it to the position ring.
static void recvPos(uint8_t c)
{
    linkPushEnemyPos((g_linkRecvPos << 2) | (c >> 6), c << 2);
}

// Receive 2: partner fire buttons. Set the matching weapon edges.
static void recvFire(uint8_t c)
{
    if (c & 0x01) {
        g_weaponEdges |= 0x10; // partner MG edge
        if (c & 0x10)
            g_killFlagsPendingB |= 0x40;
    }
    if (c & 0x02)
        g_weaponEdges |= 0x20; // partner fire edge
    g_enemySubFlags = (g_enemySubFlags & ~0x01) | ((c & 0x04) ? 0 : 0x01);
}

// Receive 3: partner hit. Apply it unless the round is over.
static void recvHit(uint8_t c)
{
    if (g_sceneFlagsC & 0x02) // round over
        return;
    if (c & 0x10) {
        setHitFlash(&g_shipArray_Enemy + 4);
        playDeb8(1);
        return;
    }
    g_shotFlags = 0x80;
    if (c & 0x20) {
        playerSubHit(1);
        return;
    }
    uint8_t *ship = &g_shipArray_Player + (selectEnemyShip(c) << 3);
    g_layoutWidthUnits = ship[1] & 0x0f;
    if (!(ship[4] & 0x80))
        hitShipCheckSub(ship + 4, 1);
}

// Receive 4: partner sub fire mode.
static void recvFireMode(uint8_t c)
{
    if ((c & 0x0c) == 0x0c) {
        // Single mode.
        g_subFireMode = (g_subFireMode & ~0x40) | 0x20;
        g_d2f7Counter = 0;
        return;
    }
    g_enemySubFlags |= 0x10;
    if (c & 0x10)
        return;
    g_enemySubFlags &= ~0x10;
    g_mgBurstCooldownB = 0;
    // Burst mode.
    g_subFireMode = (g_subFireMode & ~0x20) | 0x40;
    g_d2f7Counter = 0x11;
}

// Receive 5: partner scene flags, merged into $d001/$d002 once.
static void recvSceneFlags(uint8_t c)
{
    if (g_sceneFlagsC & 0x20)
        return;
    g_sceneFlagsC |= ((c & 0x01) << 4) | 0x20;
    g_sceneFlagsB |= c & 0x20;
    g_modeFlags1d &= ~0x80;
    if (c & 0x04) {
        g_modeFlags1d |= 0x80;
        selectEnemyShip(0);
    }
    shipDestroyed(c & 0x01);
}

// Receive 7: partner kill flags.
static void recvKillFlags(uint8_t c)
{
    uint8_t *flags = &g_killFlagsPending;
    uint8_t bits = (c & 0x38) << 2;
    if (c & 0x04) {
        flags[1] = bits;
        return;
    }
    if (c & 0x02)
        flags++;
    uint8_t old = *flags;
    uint8_t x = (old & 0xe0) ^ bits;
    // HUD parameter: 2 when bit 7 changes, else 1 when bit 6 changes, else 0.
    uint8_t e = (x & 0x80) ? 2 : (x & 0x40) ? 1 : 0;
    if (!(c & 0x02))
        hudQueuePush(0x0d, e);
    *flags = (old & 0x1f) | bits;
    g_shotFlags = 0x80;
    g_linkSendHold = 0x80;
    launchObject(c & 0x01, (c & 0x02) ? 0 : 1);
}

// Run the handler for the received message type ($d2e0) with byte c.
static void dispatchRecv(uint8_t c)
{
    uint8_t type = g_linkRecvType & 0x0f;
    if (type == 0) {
        recvHeader(c);
        return;
    }
    // Types 1..7 clear the received message bytes $d2e0..$d2e2 first.
    // ponytail: types 8..15 jumped past the end of the asm table; they do nothing here.
    if (type >= 8)
        return;
    clear3BytesDown(&g_linkRecvType + 2);
    switch (type) {
    case 1: recvPos(c); break;
    case 2: recvFire(c); break;
    case 3: recvHit(c); break;
    case 4: recvFireMode(c); break;
    case 5: recvSceneFlags(c); break;
    case 6:
        // Partner pause (bit 2). Toggle pause.
        if (c & 0x04) {
            g_modeFlags1a |= 0x02;
            GameB_TogglePause();
        }
        break;
    case 7: recvKillFlags(c); break;
    }
}

// Send 0: start the next message. Its header byte holds the type.
static void sendHeader(void)
{
    uint8_t *msg = &g_linkMsg;
    uint8_t e;
    if (g_linkSendRequest & 0x01) {
        e = g_linkNewMsgBase;
        msg[0] = e + 1;
        msg[1] = 0;
        msg[2] = 0;
        e += 0x20;
    } else {
        uint8_t *slot;
        for (;;) {
            slot = &g_linkTxQueue + (g_linkTxRead << 1);
            msg[1] = 0;
            if (slot[0] != 0xff)
                break;
            slot[1] = 0;
            slot[0] = 0;
            advanceRead();
        }
        e = slot[0];
        msg[0] = e + 1;
        g_linkMsgData = slot[1];
        slot[1] = 0;
        slot[0] = 0;
        if (e) {
            advanceRead();
            e += 0x20;
        } else {
            e = (&g_playerScreenX)[1];
        }
    }
    dispatchRecv(sendByte((headerCheck(e) << 6) | e));
}

// Send message byte a, then clear the message and handle the reply.
static void sendData(uint8_t a)
{
    uint8_t c = sendByte(a);
    clear3BytesDown(&g_linkMsgData);
    dispatchRecv(c);
}

// Link state 3: send the next message byte by type ($d2dd low 4 bits).
static void linkState3(void)
{
    uint8_t type = g_linkMsg & 0x0f;
    uint8_t a = g_linkMsgData;
    switch (type) {
    case 0:
        sendHeader();
        return;
    case 1:
        // The player X.
        a = g_playerScreenX;
        g_modeFlags15 &= ~0x80;
        break;
    case 2:
        // The data with the side bit.
        a |= (g_modeFlags10 & 0x01) << 2;
        break;
    case 4:
        // The data, with bit 4 set when the player is dead.
        if (g_modeFlags10 & 0x10)
            a |= 0x10;
        break;
    default:
        // ponytail: types 8..15 jumped into the receive table in asm; they send the data here.
        break;
    }
    sendData(a);
}

// Byte offset of sync byte i in the ship list: (i & $fe) * 4 + (i & 1).
static uint8_t syncOffset(uint8_t i)
{
    return ((i & 0xfe) << 2) + (i & 0x01);
}

// Link state 5: copy the ship lists between the two sides, one byte per frame.
static void linkState5(void)
{
    uint8_t i = g_linkSyncIndex;
    if (!(g_sceneFlagsC & 0x04)) {
        // Receiver. The first byte must be the sync byte $55.
        uint8_t e = sendByte(g_sceneFlagsC);
        if (i >= 0x1e)
            goto done;
        if (!i && e != 0x55)
            return;
        g_linkSyncIndex++;
        uint8_t *p;
        if (i < 0x12) {
            p = &g_shipArray_Player + 2;
        } else if (i < 0x18) {
            i -= 0x12;
            p = &g_shipArray_Enemy + 0x4a;
        } else {
            i -= 0x18;
            p = &g_shipArray_Player + 0x4a;
        }
        p[syncOffset(i)] = e;
        return;
    }
    // Sender.
    if (i >= 0x20)
        goto done;
    g_linkSyncIndex++;
    uint8_t *p = &g_shipArray_Player - 6;
    if (i >= 0x1a) {
        i -= 0x1a;
        p = &g_shipArray_Enemy + 0x4a;
    }
    sendByte(p[syncOffset(i)]);
    return;
done:
    g_modeFlags16 |= 0x40;
    g_gameBLinkState = 7;
}

// Link cable update (bank entry point). Runs only in link mode
// (g_sceneFlagsC bit 3). Runs the handler for link state $d00b.
void gameBLinkUpdate(void) __banked
{
    if (!(g_sceneFlagsC & 0x08)) // link mode
        return;
    switch (g_gameBLinkState) {
    case 0: {
        // Exchange pad bytes with the partner.
        uint8_t a = g_padState;
        uint8_t c = sendByte(a);
        if (g_sceneFlagsC & 0x80) {
            a &= 0x09;
            if (a)
                g_killFlagsPending |= 0x10;
        }
        (&g_linkPad)[0] = a | c;
        (&g_linkPad)[1] = c;
        break;
    }
    case 1:
        // Wait for the handshake $d0/$b0, then go to state 7.
        if (cableProtocol(0xd0, 0xb0))
            return;
        g_modeFlags16 |= 0x80;
        g_gameBLinkState = 7;
        break;
    case 2: {
        // Handshake $f1/$f4. Then clear the message buffer and go to state 3.
        if (cableProtocol(0xf1, 0xf4))
            return;
        g_sceneFlagsA |= 0x08;
        g_gameBLinkState = 3;
        uint8_t *p = &g_linkMsg;
        for (uint8_t n = 6; n; n--)
            *p++ = 0;
        linkTxQueuePush(((g_killFlagsPending & 0xe0) >> 2) | 0x04, 6);
        linkTxQueuePush(0x80, 1);
        break;
    }
    case 3:
        linkState3();
        break;
    case 4: {
        // Handshake for the end screen. The pair depends on the sub state.
        uint8_t d = 0xb0, e = 0x90;
        if (g_subState == 0x1b) {
            d = 0xfd;
            e = 0xfc;
        } else if (g_subState == 0x22) {
            d = 0xe0;
            e = 0x70;
        }
        if (!cableProtocol(d, e))
            g_modeFlags16 |= 0x80;
        break;
    }
    case 5:
        linkState5();
        break;
    case 6:
        // Handshake $f1/$f4, then sub state $2b.
        if (!cableProtocol(0xf1, 0xf4))
            g_subState = 0x2b;
        break;
    case 7:
        // Idle: send 0.
        cableProtocol(0, 0);
        break;
    }
}

// Entry from bank 0. Read the pad: the local pad, or the link pad in link
// mode. Then handle the game clock at $d2f9.
void gameBReadPadAndClock(void) __banked
{
    uint8_t pad = g_padState;
    if (g_sceneFlagsC & 0x08) { // link mode
        if (g_levelClearTimer && --g_levelClearTimer)
            return;
        pad = g_linkPad;
    }
    g_gameClock++;
    uint8_t t = g_shotFlags; // timeout, $ff = none
    if (t == 0xff || t >= g_gameClock) {
        if (!(pad & 0x09))
            return;
    }
    g_linkPad = 0;
    g_subState = (&g_dispatchTableD346)[1];
    g_levelClearTimer = 6;
    g_gameClock = 0;
}

// Entry from bank 0. Mark a link end: death marker 5, link state 0,
// clear the link pad bytes and the received byte.
void gameBMarkLinkEnd(void) __banked
{
    g_deathMarker = 5;
    g_gameBLinkState = 0;
    (&g_linkPad)[0] = 0;
    (&g_linkPad)[1] = 0;
    g_serialRecv = 0;
}

// Entry from bank 0. Set the level-clear timer (9, or 8 on side 1),
// move the link state to 4 and clear $d016 bit 7.
void gameBSetLevelClearTimer(void) __banked
{
    g_levelClearTimer = (g_sceneFlagsC & 0x04) ? 8 : 9;
    g_gameBLinkState = 4;
    g_modeFlags16 &= ~0x80;
}

// Entry from bank 0. Send a link status message (type 4):
// $40 | side bit | $d001 bit 5, plus bit 2 when no enemy ship is selected.
void gameBSendLinkStatus(void) __banked
{
    uint8_t c = (g_sceneFlagsC & 0x01) | 0x40;
    if (!(c & 0x01) && !g_selectedEnemyShip)
        c |= 0x04;
    linkTxQueuePush((g_sceneFlagsB & 0x20) | c, 4);
}
