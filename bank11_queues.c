// Ring queues that the game logic fills: sound requests, HUD commands,
// link messages and partner positions.
#include "game.h"
#include "bank3.h"

#define HUD_QUEUE_SIZE 20

// Adds (type, id) to the 16-entry sound ring at g_soundQueue.
void soundQueuePush(uint8_t type, uint8_t id)
{
    uint8_t *slot = &g_soundQueue + (g_soundQueueWrite << 1);
    slot[0] = type;
    slot[1] = id;
    g_soundQueueWrite = (g_soundQueueWrite + 1) & 0x0f;
}

// Adds HUD command cmd (1..16) with a parameter to the HUD queue.
// Bank 0 runs one command per call.
void hudQueuePush(uint8_t cmd, uint8_t param)
{
    uint8_t *slot = &g_hudQueue + (g_hudQueueWrite << 1);
    slot[0] = cmd;
    slot[1] = param;
    if (++g_hudQueueWrite >= HUD_QUEUE_SIZE)
        g_hudQueueWrite = 0;
}

// Adds a 2-byte message to the 8-entry link send ring.
void linkTxQueuePush(uint8_t cmd, uint8_t param)
{
    uint8_t *slot = &g_linkTxQueue + (g_linkTxWrite << 1);
    if (++g_linkTxWrite >= 8)
        g_linkTxWrite = 0;
    slot[0] = param;
    slot[1] = cmd;
}

// In link mode (and the round runs), takes one partner X from the
// position ring and stores it as the enemy ship X.
void linkPopEnemyPos(void)
{
    if ((g_sceneFlagsC & 0x0a) != 0x08)
        return;
    uint8_t *slot = &g_posRing + (g_posRingRead << 1);
    uint8_t hi = slot[0];
    if (!hi)
        return;
    uint8_t lo = slot[1];
    slot[0] = 0;
    slot[1] = 0;
    (&g_shipArray_Enemy)[2] = lo;
    (&g_shipArray_Enemy)[3] = hi & 0x7f;
    if (++g_posRingRead >= 8)
        g_posRingRead = 0;
}

// Adds the partner X (hi | $80, lo) to the position ring, unless the round is over.
void linkPushEnemyPos(uint8_t hi, uint8_t lo)
{
    if (g_sceneFlagsC & 0x02)
        return;
    uint8_t *slot = &g_posRing + (g_posRingWrite << 1);
    slot[0] = hi | 0x80;
    slot[1] = lo;
    if (++g_posRingWrite >= 8)
        g_posRingWrite = 0;
}

// Entry points for the asm glue in bank 3. The arguments come on the stack.
void soundQueuePushFar(uint8_t type, uint8_t id) __banked { soundQueuePush(type, id); }
void hudQueuePushFar(uint8_t cmd, uint8_t param) __banked { hudQueuePush(cmd, param); }
void linkTxQueuePushFar(uint8_t cmd, uint8_t param) __banked { linkTxQueuePush(cmd, param); }
