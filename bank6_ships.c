// Game B ship arrays: mirror the player ships to the enemy side, and give them random positions.
// A slot is 8 bytes. These functions point at byte +1 (type), then +2/+3 is the X position.
#include "game.h"

#define PLAYER_SLOTS (&g_shipArray_Player)
#define ENEMY_SLOTS  (&g_shipArray_Enemy)
#define TYPE_FIXED   0x79  // a slot of this type keeps its position

extern const uint8_t RandomLimitTable[];

// For n slots: sets bit 7 of the type (unless g_mirrorKeepFlags bit 0 is set),
// and sets X = world width - X.
static void mirrorSlots(uint8_t *slot, uint8_t n)
{
    uint16_t width = WORD(g_worldWidth);
    for (; n; n--, slot += 8) {
        if (!(g_mirrorKeepFlags & 1))
            slot[0] |= 0x80;
        WORD(slot[1]) = width - WORD(slot[1]);
    }
}

// Copies the player ships to the enemy array and mirrors them. When
// g_sceneFlagsC bits 2 and 3 are both set, it also mirrors 3 more enemy
// slots from slot 9, then (with flags kept) 3 player slots from slot 9.
void initShipArrays(void)
{
    g_mirrorKeepFlags = 0;
    uint8_t n = g_gameBShipCountM1 + 1;
    for (uint8_t i = 0; i < n * 8; i++)
        ENEMY_SLOTS[i] = PLAYER_SLOTS[i];
    mirrorSlots(ENEMY_SLOTS + 1, n);
    if ((g_sceneFlagsC & 0x0c) != 0x0c)
        return;
    mirrorSlots(ENEMY_SLOTS + 9 * 8 + 1, 3);
    g_mirrorKeepFlags++;
    mirrorSlots(PLAYER_SLOTS + 9 * 8 + 1, 3);
}

// Number of next slots whose type has the same group (bits 4-6).
static uint8_t countGroupSlots(const uint8_t *slot)
{
    uint8_t group = *slot & 0x70;
    uint8_t n = 0;
    while ((slot[8] & 0x70) == group) {
        n++;
        slot += 8;
    }
    return n;
}

// Mixes g_gameBRandomSeed with rDIV, rLY and g_randomPos. Stores the new seed.
static uint8_t randomByte(void)
{
    uint8_t mix = rDIV + g_gameBRandomSeed;
    mix += SWAP(mix);
    uint8_t r = (rLY << 1) + mix;
    g_gameBRandomSeed = SWAP(g_randomPos) + r;
    return r;
}

// g_randomPos = the seed, minus $3e if it is $3e or more.
static uint8_t setPosFromSeed(void)
{
    uint8_t pos = g_gameBRandomSeed;
    if (pos >= 0x3e)
        pos -= 0x3e;
    return g_randomPos = pos;
}

// Adds a random step ($24 to limit) to g_randomPos.
static uint8_t advancePosRandom(uint8_t limit)
{
    uint8_t step = randomByte();
    if (step < 0x24)
        step = 0x24;
    else if (step >= limit)
        step = limit;
    return g_randomPos += step;
}

// X = (pos * ScrollScript_LastTileByte) << 4, unless the slot type is TYPE_FIXED.
static void setSlotX(uint8_t *slot, uint8_t pos)
{
    uint16_t x = multiply8(pos, ScrollScript_LastTileByte) << 4;
    if ((slot[0] & 0x7f) != TYPE_FIXED)
        WORD(slot[1]) = x;
}

// Gives each ship group in the player array a random X position.
// A single ship gets a random position. A group is spread with random steps.
// A group of type TYPE_FIXED stops the placement.
void placeShips(void)
{
    g_randomPos += rLYC;
    uint8_t left = g_gameBShipCountM1 + 1;
    uint8_t *slot = PLAYER_SLOTS + 1;
    while (left) {
        uint8_t more = countGroupSlots(slot);
        if (!more) {
            randomByte();
            setSlotX(slot, setPosFromSeed());
            slot += 8;
            left--;
            continue;
        }
        if ((*slot & 0x7f) == TYPE_FIXED)
            break;
        if ((*slot & 0x70) == 0x70 && g_gameBSkill == 2)
            g_randomPos = 0;
        else
            setPosFromSeed();
        uint8_t limit = RandomLimitTable[more + 1];
        for (uint8_t n = more + 1; n && left; n--, left--) {
            setSlotX(slot, advancePosRandom(limit));
            slot += 8;
        }
    }
    PLAYER_SLOTS[3] = 0x10;
}
