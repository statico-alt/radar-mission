// Lock-on AI, enemy sub AI and the Game B joypad.
// They call the range code in bank 3, so they stay in bank 3.
#include "game.h"
#include "bank3.h"

void PlayMusic(uint8_t id);

#define SHIP(i) (&g_shipArray_Player + ((i) << 3))
#define ENEMY_X (&g_shipArray_Enemy + 2)

// Set $d019 for fire state a: bit 6 = burst (counter $11), bit 5 = single (counter 0).
static void subFireModeSet(uint8_t a)
{
    if (a & 0x40) {
        g_subFireMode = (g_subFireMode | 0x40) & ~0x20;
        g_d2f7Counter = 0x11;
    } else if (a & 0x20) {
        g_subFireMode = (g_subFireMode | 0x20) & ~0x40;
        g_d2f7Counter = 0;
    }
}

// Store flags | $40 in $d011 and set fire state a.
static void setFireState(uint8_t a, uint8_t flags)
{
    g_enemySubFlags = flags | 0x40;
    subFireModeSet(a);
}

// Start a burst: counter $11, fire state $40.
void enemySubStartBurst(uint8_t flags)
{
    g_d2f7Counter = 0x11;
    setFireState(0x40, flags);
}

// Set the burst delay ($d355) from the AI level ($d43c), fire state $20.
static void startVolley(uint8_t flags)
{
    uint8_t v = 7 - g_subAiLevel;
    g_burstDelay = SWAP(v) << 1;
    setFireState(0x20, flags);
}

// Set bit 5 of *flags when the enemy sub depth is within $d446 of $58.
static void checkSubInRange(uint8_t *flags)
{
    uint8_t a = g_enemySubApproachShadow;
    if (a == 0xd0 || a >= 0xa8 || a < 0x08)
        return;
    a -= 0x08;
    a = (a >= 0x50) ? a - 0x50 : 0x50 - a;
    if ((&g_weaponFlagsBase)[0x0f] >= a)
        *flags |= 0x20;
}

// Start a volley when the player is close (depth < 2 * $d43d) and
// lock shots remain.
static void checkClose(uint8_t flags)
{
    if (g_enemySubApproachShadow == 0xd0)
        return;
    if (((&g_shipArray_Enemy)[4] & 0x1f) >= (uint8_t)(g_subAiCloseRange << 1))
        return;
    if (g_mgLockShots)
        startVolley(flags);
}

// Decide when the enemy sub starts a burst ($d011 bit 4, $d019).
static void enemySubFireAI(void)
{
    uint8_t flags = g_enemySubFlags;
    if (!(flags & 0x10)) {
        // Idle: while the round runs, start a volley once the sub has used
        // enough MG lock shots ($d349) and the counter is 0.
        if (g_sceneFlagsC & 0x02)
            return;
        if (g_mgLockShots < (uint8_t)(7 - g_mgAmmoPenalty) || g_d2f7Counter)
            checkClose(flags);
        else
            startVolley(flags);
        return;
    }
    if (!(g_sceneFlagsC & 0x02)) {
        if (g_burstDelay) {
            g_burstDelay--;
            return;
        }
        if (!(g_weaponEdges & 0x80)) {
            if (g_enemySubApproachShadow == 0xd0)
                return;
            if (!(g_frameCounter & g_subAiCloseRange))
                return;
            g_weaponEdges |= 0x10;
            checkSubInRange(&g_weaponEdges);
        }
    }
    enemySubStartBurst(flags);
}

// Store the AI state and the lock-on target, then check the sub range.
// The asm sets bit 5 of the target index here, not of $d01c.
static void setLockOnTarget(uint8_t state, uint8_t target)
{
    g_weaponAiState = state;
    g_lockOnTargetIdx = target;
    checkSubInRange(&g_lockOnTargetIdx);
}

// Input: x = target X (centre $38), margin, ship = the target slot.
// Set or clear the enemy ship direction bit ($d218 bit 5).
// Returns the distance from the centre, or the sub state when it is $1a or more.
static uint8_t faceTarget(uint8_t x, uint8_t margin, uint8_t *ship)
{
    if (g_subState >= 0x1a)
        return g_subState;
    uint8_t a;
    if (x >= 0x38) {
        a = x - 0x38;
        if (a < margin || !(ship[0] & 0x20)) {
            g_shipArray_Enemy |= 0x20;
            return a;
        }
    } else {
        a = 0x38 - x;
        if (a >= margin && !(ship[0] & 0x20)) {
            g_shipArray_Enemy |= 0x20;
            return a;
        }
    }
    g_shipArray_Enemy &= ~0x20;
    return a;
}

// Run the enemy fire AI, then keep the lock-on target valid. Drops it
// when the ship slot is no longer active.
static void updateLockOnTarget(void)
{
    enemySubFireAI();
    uint8_t target = g_lockOnTargetIdx;
    if (target == 0xff)
        return;
    // Bit 4 of the index is never set: it comes from ship byte 7 & $0f.
    uint8_t *ship = SHIP(target & 0x0f);
    if (!(ship[0] & 0x80)) {
        g_weaponEdges &= ~0xc0;
        setLockOnTarget(0, 0xff);
        return;
    }
    uint8_t x = relativeX(WORD(ship[2]), ENEMY_X);
    if (g_sceneFlagsC & 0x02)
        x += 2;
    uint8_t dist = faceTarget(x, 6, ship);
    uint8_t limit = g_subAiLevel;
    if (!limit)
        limit = (g_frameCounter & 0x07) ? 0 : 1;
    if (dist < limit)
        g_weaponEdges |= 0x90;
    else
        g_weaponEdges &= ~0x80;
    checkSubInRange(&g_weaponEdges);
}

// State 1: test one ship per frame for 16 frames. Add each candidate
// to the list at $d380 as (distance from $38, index | $20 when right).
// Then go to state 2.
static void weaponAiScan(void)
{
    uint8_t *ship;
    do {
        uint8_t i = g_aiScanIndex++;
        if (g_aiScanIndex >= 0x10) {
            g_aiScanIndex = 0;
            g_weaponAiState = 2;
            return;
        }
        ship = SHIP(i);
    } while (!(ship[0] & 0x80));
    uint8_t x = relativeX(WORD(ship[2]), ENEMY_X);
    uint8_t idx = ship[7] & 0x0f;
    uint8_t *c = &g_aiCandidates + (g_aiCandidateCount++ << 1);
    if (x >= 0x38) {
        x -= 0x38;
        idx |= 0x20;
    } else {
        x = 0x38 - x;
    }
    c[0] = x;
    c[1] = idx;
}

// State 2: pick the closest candidate (distance < $38) as the lock-on target.
static void weaponAiPick(void)
{
    uint8_t target = 0xff;
    uint8_t n = g_aiCandidateCount;
    if (n) {
        uint8_t best = 0x38;
        for (uint8_t *c = &g_aiCandidates; n; n--, c += 2) {
            if (c[0] < best) {
                best = c[0];
                target = c[1];
            }
        }
        g_weaponEdges |= 0x40;
        if (target != g_lockOnTargetIdx)
            g_mgLockShots = 0;
    }
    setLockOnTarget(0, target);
}

// Entry from bank 0 ($60fc). Weapon AI tick: every 4th frame update the
// lock-on target. While the round runs, run the weapon AI state handler.
void weaponAiTick(void)
{
    if (!(g_frameCounter & 0x03))
        updateLockOnTarget();
    if (g_sceneFlagsC & 0x02)
        return;
    switch (g_weaponAiState) {
    case 0:
        // Reset the scan ($d00e = 1, $d00f = 0).
        g_aiScanIndex = 1;
        g_aiCandidateCount = 0;
        g_weaponAiState = 1;
        // fall through
    case 1:
        weaponAiScan();
        break;
    case 2:
        weaponAiPick();
        break;
    }
}

// Move the enemy sub toward or away from the player. The approach counter
// stops at $3c. Input: d = the caller's d. Returns the new d.
uint8_t enemySubApproachAI(uint8_t d)
{
    if (g_modeFlags1d & 0x80)
        return d;
    uint8_t a = g_enemySubApproachCounter;
    if (a != 0x3c) {
        if (g_modeFlags1e & 0x80) {
            a--;
            g_enemySubFlags &= ~0x01;
        } else {
            a++;
            g_enemySubFlags |= 0x01;
        }
    }
    g_enemySubApproachCounter = a;
    g_enemySubApproachShadow = a;
    return a;
}

// Input: x = relative X. Pick the enemy sub move direction (bits 4, 5 of $d02e)
// and its speed limits ($d438).
static void pickDirection(uint8_t x)
{
    uint8_t e = 0;
    if (!(g_enemySubDirLock & 0x01)) {
        uint8_t a = 0x08;
        uint8_t dir; // 0 = lock, 1 = right, 2 = left
        if (g_selectedEnemyShip) {
            dir = (x < 0x38) ? 2 : (x != 0x38) ? 1 : 0;
        } else {
            uint8_t s = g_enemySubApproachShadow;
            dir = (s < 0x54) ? 2 : (s >= 0x5c) ? 1 : 0;
        }
        if (dir == 0) {
            g_enemySubDirLock |= 0x01;
            a = 0;
        } else if (dir == 1) {
            g_shipArray_Player |= 0x20;
            e = 0x10;
        } else {
            g_shipArray_Player &= ~0x20;
            e = 0x20;
        }
        (&g_weaponFlagsBase)[1] = a;
        (&g_weaponFlagsBase)[2] = a;
    }
    g_padState = (g_padState & 0xcf) | e;
}

// Map the selected enemy ship X ($d45b) to the screen and pick the sub direction.
void mapSelectedShip(void)
{
    g_spriteDrawMode = 0;
    pickDirection(relativeX(WORD(g_selectedShipX), &g_shipArray_Player + 2));
}

// Read the joypad: build the new-press edge (g_joyEdge). Before sub
// state $13 only menu input is used.
void gameBProcessInput(void)
{
    uint8_t raw = g_joyRaw;
    uint8_t edge = (g_joyPrev ^ raw) & raw;
    g_joyEdge = edge;
    g_joyPrev = raw;
    uint8_t s = g_subState;
    uint8_t a;
    if (s < 0x13)
        a = edge;
    else if (s >= 0x16 && s < 0x23)
        a = 0;
    else
        a = raw;
    uint8_t pad = (a & 0xf0) | (edge & 0x0f);
    g_padState = pad;
    g_weaponEdges |= pad & 0x03;
    g_aiPadState = (g_shipArray_Enemy & 0x20) ? 0x10 : 0x20;
}

// Entry from bank 0 ($6394). Copy the 8 bytes that end at src + 1 into a
// free slot (bit 7 clear) among the 8 slots at $d298, then clear bytes 4
// and 5 and set byte 7 bit 5. Returns 1 when all slots are in use.
uint8_t findFreeSlotD298(const uint8_t *src)
{
    uint8_t *slot = &g_spawnObjSlots;
    for (uint8_t n = 8; n; n--, slot += 8) {
        if (slot[0] & 0x80)
            continue;
        src -= 6;
        for (uint8_t i = 0; i < 8; i++)
            slot[i] = src[i];
        slot[4] = 0;
        slot[5] = 0;
        slot[7] |= 0x20;
        return 0;
    }
    return 1;
}

// Entry from bank 0 ($63be). Set enemy ship 0 bit 6 once and reset the
// enemy sub fire phase.
void markEnemyShip0(void)
{
    if (g_shipArray_Enemy & 0x40)
        return;
    g_shipArray_Enemy |= 0x40;
    g_enemySubFirePhase = 0;
}
