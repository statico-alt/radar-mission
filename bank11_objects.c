// Game B world math and object update. Each object slot is 8 bytes.
// Byte 0: bit 7 = active, bit 5 = moves right, bits 0-4 = speed.
// Byte 1: bit 7 = special, bits 0-3 = type. Bytes 2-3: world X.
#include "game.h"
#include "bank3.h"

#define LINK_MODE (g_sceneFlagsC & 0x08)
#define PLAYER_DEAD (g_modeFlags10 & 0x10)
#define SLOT_X(s) WORD((s)[2])

// Decrements *t unless it is 0. Returns the old value.
uint8_t decAndCheckTimer(uint8_t *t)
{
    uint8_t v = *t;
    if (v)
        *t = v - 1;
    return v;
}

// Entry from bank 0. Set SCY to $88, start the damage gauge raster
// (count 1, LYC table 0, 7) and clear $d010 bit 4.
void gameBStartDamageGauge(void) __banked
{
    rSCY = 0x88;
    g_rasterCount = 1;
    (&g_rasterLycTable)[0] = 0;
    (&g_rasterLycTable)[1] = 7;
    g_modeFlags10 &= ~0x10;
}

// Entry from bank 0. Tick the game clock at $d2f9 while the round runs
// ($d002 bit 1 and $d001 bit 1 clear). Digits carry at 15 frames,
// 60 and 99 ($63). At the end the last digit is $99 and $d001 bit 1 is set.
void gameBTickGameClock(void) __banked
{
    uint8_t *clock = &g_gameClock;
    if ((g_sceneFlagsC & 0x02) || (g_sceneFlagsB & 0x02))
        return;
    if (++clock[0] < 0x0f)
        return;
    clock[0] = 0;
    if (++clock[1] < 0x3c)
        return;
    clock[1] = 0;
    if (++clock[2] < 0x63)
        return;
    clock[2] = 0x99;
    g_sceneFlagsB |= 0x02;
}

// Once per scene ($d015 bit 7), store the player X in screen units at $d2f2.
static void initPlayerScrollOnce(void)
{
    if (g_modeFlags15 & 0x80)
        return;
    g_modeFlags15 |= 0x80;
    uint16_t v = (WORD(g_worldWidth) - SLOT_X(&g_shipArray_Player)) >> 2;
    (&g_playerScreenX)[0] = v;
    (&g_playerScreenX)[1] = ((v >> 8) & 0x1f) + 1;
}

// In link mode, set up the player scroll once.
static void checkLinkMode(void)
{
    if (LINK_MODE)
        initPlayerScrollOnce();
}

// x - speed. Below 0 the high byte wraps to the world width - 1.
uint16_t moveLeft(uint16_t x, uint8_t speed)
{
    if (x < speed)
        return (x - speed) & 0x00ff | ((uint16_t)(g_worldWidthHi - 1) << 8);
    return x - speed;
}

// x + speed. At the world width the high byte wraps to 0.
uint16_t moveRight(uint16_t x, uint8_t speed)
{
    x += speed;
    if ((uint8_t)(x >> 8) >= g_worldWidthHi)
        x &= 0x00ff;
    return x;
}

// Input: pad bits (bit 4 right, bit 5 left) for one side.
// Update the move flags in $d010 + side.
static void setMoveFlags(uint8_t pad, uint8_t side)
{
    uint8_t *p = &g_modeFlags10 + side;
    if (!(pad & 0x30)) {
        *p &= ~0x08;
        return;
    }
    uint8_t f = *p;
    if (pad & 0x20) {
        if ((f & 0x03) == 0x03)
            f &= ~0x04;
        else
            f &= 0xf0;
    } else {
        if ((f & 0x03) == 0x02)
            f |= 0x04;
        else
            f |= 0x05;
    }
    *p = f | 0x0a;
}

// Counts the speed timer at $d2ee + side down. When it ends, slow the
// speed at $d2ec + side by 1. At speed 0 stop moving (clear $d010 bit 1).
static void slowDown(uint8_t side, uint8_t *speed)
{
    speed[2] = 0;
    if (*speed && --*speed)
        return;
    (&g_modeFlags10)[side] &= ~0x02;
    if (!(side & 1))
        g_modeFlags16 |= 0x04;
}

// Speed up or slow down $d2ec + side from the move flags in $d010 + side.
// The speed timer at $d2ee + side delays each step.
// Returns the old speed. *newSpeed = the new speed.
static uint8_t updateSpeed(uint8_t side, uint8_t *newSpeed)
{
    uint8_t flags = (&g_modeFlags10)[side];
    uint8_t *speed = &g_sideSpeed + side;
    uint8_t *timer = speed + 2;
    uint8_t old = *speed;

    if (!(flags & 0x08)) {
        // No pad input: slow down.
        if (!old) {
            *newSpeed = 0;
            return old;
        }
        if (*timer) {
            --*timer;
        } else {
            slowDown(side, speed);
        }
    } else if ((flags & 0x01) != ((flags >> 2) & 0x01)) {
        // Turning: slow down twice as fast.
        if (*timer && --*timer)
            --*timer;
        else
            slowDown(side, speed);
    } else if (*timer) {
        --*timer;
    } else {
        // Speed up to the maximum from the weapon flags.
        const uint8_t *max = &g_weaponFlagsBase + 1;
        if (flags & 0x10)
            max += 2;
        if ((&g_killFlagsPending)[side] & 0x20)
            max++;
        uint8_t limit = *max + 1;
        uint8_t v = *speed + 1;
        if (v >= limit) {
            v--;
            if (!(flags & 0x10)) {
                v--;
                if (v >= limit)
                    v = limit;
            }
        }
        *speed = v;
    }
    *newSpeed = *speed;
    return old;
}

// Moves the object by its speed (byte 0 low 5 bits). b = byte 1.
// A normal object stops (byte 0 = 0) when byte 4 bit 7 is set and the player lives.
static void moveSlot(uint8_t *slot, uint8_t b)
{
    uint8_t e = (slot[0] & 0x3f) | (b & 0x80);
    uint8_t speed = e & 0x1f;
    uint16_t x = SLOT_X(slot);
    uint8_t right = e & 0x20;
    if (e & 0x80) {
        // Fixed objects move the other way.
        right = !right;
    } else if ((slot[4] & 0x80) && !PLAYER_DEAD) {
        slot[0] = 0;
    }
    SLOT_X(slot) = right ? moveRight(x, speed) : moveLeft(x, speed);
}

// Types 7 and 8. When byte 1 bit 7 is set, wait for the $d2f5 timer.
// Then start the next spawn slot at the object X, then move.
static void objUpdateType07(uint8_t *slot, uint8_t b)
{
    if (b & 0x80) {
        if (g_objType7Timer) {
            g_objType7Timer--;
        } else {
            // $d2f6 = next spawn index, below the count at $d444.
            uint8_t *next = &g_objType7Timer + 1;
            uint8_t i = *next;
            if (++*next >= (&g_weaponFlagsBase)[0x0d])
                *next = i = 0;
            uint8_t *spawn = &g_spawnObjSlots + (i << 3);
            if (!(spawn[0] & 0x80)) {
                g_objType7Timer = (&g_weaponFlagsBase)[0x0e];
                spawn[0] = 0x8a;
                spawn[1] = 0xfa;
                spawn[2] = slot[2];
                spawn[3] = slot[3];
                spawn[4] = 0;
                spawn[5] = 0;
                spawn[6] = (g_frameCounter & 0x06) | ((g_modeFlags1b & 0x04) << 2);
                spawn[7] = 0;
            }
        }
    }
    moveSlot(slot, b);
}

// Type 9: count down the low 6 bits of byte 4. At 0 reload 2 and
// grow byte 0 bits 0-4 up to $11. Then move.
static void objUpdateType09(uint8_t *slot, uint8_t b)
{
    uint8_t v = slot[4];
    if (v & 0x3f) {
        slot[4] = v - 1;
    } else {
        slot[4] = (v & 0xc0) | 0x02;
        if ((slot[0] & 0x1f) < 0x11)
            slot[0]++;
    }
    moveSlot(slot, b);
}

// Frees a type 10-12 slot. The X gets the last value of A in the asm.
static void freeSlot(uint8_t *slot, uint8_t x)
{
    slot[6] = 0;
    slot[0] = 0;
    slot[2] = x;
    slot[3] = 0;
}

// Types 10-12. When byte 6 bit 5 is set, step byte 5 up to $6c (or to $0a
// when byte 7 bit 5 is set). At the end free the slot.
static void objUpdateType0a(uint8_t *slot, uint8_t b)
{
    if (slot[6] & 0x20) {
        uint8_t v = slot[5];
        if (slot[7] & 0x20) {
            if (v >= 0x0a) {
                freeSlot(slot, v);
                return;
            }
        } else if (v >= 0x6c) {
            freeSlot(slot, decAndCheckTimer(&g_kind22ActiveCount));
            return;
        }
        slot[5]++;
    }
    moveSlot(slot, b);
}

// Type 4: a ship. The pad moves it (bit 7 = the computer side).
// Link mode and the round state ($d002 bits 3 and 1) can hold it still.
static void objUpdateType04(uint8_t *slot, uint8_t b)
{
    uint8_t side, pad;
    if (b & 0x80) {
        uint8_t f = g_sceneFlagsC;
        if (LINK_MODE) {
            if (!(f & 0x02))
                return;
        } else if ((f & 0x03) == 0x03 && !(g_sceneFlagsB & 0x20)) {
            return;
        }
        side = 1;
        pad = (g_modeFlags1d & 0x40) ? 0 : g_aiPadState;
    } else {
        side = 0;
        pad = g_padState;
    }
    setMoveFlags(pad, side);
    uint8_t speed;
    if (updateSpeed(side, &speed) != speed && PLAYER_DEAD && !side)
        hudQueuePush(4, speed);
    uint8_t flags = (&g_modeFlags10)[side];
    if (!(flags & 0x02))
        return;
    uint16_t x = SLOT_X(slot);
    SLOT_X(slot) = (flags & 0x01) ? moveRight(x, speed) : moveLeft(x, speed);
}

// Runs the update handler for each active (byte 0 bit 7) slot.
// The handler comes from the type (byte 1 & $0f).
static void updateObjects(uint8_t *slot, uint8_t count)
{
    do {
        if (slot[0] & 0x80) {
            uint8_t b = slot[1];
            switch (b & 0x0f) {
            case 4:
                objUpdateType04(slot, b);
                break;
            case 7:
            case 8:
                objUpdateType07(slot, b);
                break;
            case 9:
                objUpdateType09(slot, b);
                break;
            case 10:
            case 11:
            case 12:
                objUpdateType0a(slot, b);
                break;
            default:
                moveSlot(slot, b);
            }
        }
        slot += 8;
    } while (--count);
}

// Entry from bank 0. Pull the partner position from the link ring,
// then update the 12 enemy ship slots (11 from slot 1 in link mode).
void gameBUpdateEnemyShips(void) __banked
{
    checkLinkMode();
    linkPopEnemyPos();
    if (LINK_MODE)
        updateObjects(&g_shipArray_Enemy + 8, 11);
    else
        updateObjects(&g_shipArray_Enemy, 12);
}

// Entry from bank 0. Update the 12 enemy objects at $d278, then age the
// 3 objects at $d200. Type 9 counts byte 4 up to $1a. The others count
// byte 4 bits 0-3 down, then add $10 to bits 4-7. Both end the object.
void gameBUpdateEnemyObjects(void) __banked
{
    checkLinkMode();
    linkPopEnemyPos();
    updateObjects(&g_enemyObjSlots, 12);
    uint8_t *slot = &g_gameBObjSlots;
    for (uint8_t n = 3; n; n--, slot += 8) {
        if (!(slot[0] & 0x80))
            continue;
        if ((slot[1] & 0x0f) == 9) {
            if (++slot[4] < 0x1a)
                continue;
        } else {
            uint8_t v = slot[4];
            v = (v & 0x0f) ? v - 1 : (v + 0x10) | 0x02;
            slot[4] = v;
            if ((v & 0xf0) < 0x90)
                continue;
        }
        slot[0] &= ~0x80;
    }
}

// Runs the object updates for the 8 player-side slots at $d180.
static void updatePlayerObjects(void)
{
    updateObjects(&g_shipArray_Player, 8);
}

// Entry from bank 0. Update the player slots. In link mode, send a
// message when the direction ($d010 bit 0) changes, then set up the scroll.
void gameBUpdatePlayerOrLink(void) __banked
{
    if (!LINK_MODE) {
        updatePlayerObjects();
        return;
    }
    uint8_t before = g_modeFlags10 & 0x01;
    updatePlayerObjects();
    if ((g_modeFlags10 & 0x01) != before)
        linkTxQueuePush(0, 1);
    initPlayerScrollOnce();
}

// Entry from bank 0. Update the 8 slots at $d1c0.
void gameBRunObjectHandlers(void) __banked
{
    checkLinkMode();
    updateObjects(&g_shipArray_Player + 0x40, 8);
}
