// Projectiles and weapons. Torpedo slots ($d0e0), MG slots ($d0f0),
// enemy bullet slots ($d100). Ammo per tube at $d453.
// Ammo index: 0, 1 = torpedo tubes A and B. 2, 3 = MG A and B.
// A projectile slot is 8 bytes. Byte 0: bit 7 = active, bit 5 = moves
// right, bits 0-4 = speed. Byte 1 = type. Bytes 2-3 = world X.
// Byte 4 = range delay. Byte 5 = slow-down timer. Byte 6 = flags.
#include "game.h"
#include "bank3.h"

#define TWIN_TORPEDO (g_killFlagsPending & 0x40)
#define TWIN_MG (g_killFlagsPendingB & 0x40)
#define SLOT_X(s) WORD((s)[2])

// Moves a projectile by half its speed. Every 4 calls the speed drops by 1.
void projectileTick(uint8_t *slot)
{
    uint8_t t = slot[5];
    if (t & 0x0f) {
        slot[5] = t - 1;
    } else {
        slot[5] = t | 0x03;
        if (slot[0] & 0x1f)
            slot[0]--;
    }
    uint8_t speed = (slot[0] & 0x1f) >> 1;
    uint16_t x = SLOT_X(slot);
    SLOT_X(slot) = (slot[0] & 0x20) ? moveRight(x, speed) : moveLeft(x, speed);
}

// Finds the n-th free slot of count slots. When none is free it returns
// the address after the last slot. The asm fire code writes there anyway.
static uint8_t *allocSlot(uint8_t *slot, uint8_t count, uint8_t n)
{
    do {
        if (!(slot[0] & 0x80) && !--n)
            return slot;
        slot += 8;
    } while (--count);
    return slot;
}

#define allocTorpedoSlot(n) allocSlot(&g_torpedoSlots, 2, n)
#define allocMGSlot(n) allocSlot(&g_mgSlots, 2, n)
#define TORPEDO_NONE (&g_torpedoSlots + 16)
#define MG_NONE (&g_mgSlots + 16)
#define BULLET_NONE (&g_enemyBulletSlots + 32)

// Fills an 8-byte projectile slot. Returns the address of byte 7.
static uint8_t *initProjectile(uint8_t *slot, uint8_t flags, uint8_t type, uint16_t x, uint8_t dir)
{
    slot[0] = flags | 0x80;
    slot[1] = type;
    SLOT_X(slot) = x;
    slot[4] = bank3Byte(&GameB_TorpedoRangeDelays[type]);
    slot[5] = 3;
    slot[6] = (dir & 1) ? 0x80 : 0;
    slot[7] = 0;
    return slot + 7;
}

// Starts an enemy bullet in a free enemy bullet slot. The ship data is
// at p: byte 0 flags, byte 1 bits 4-6 = speed class, bytes 2-3 = X.
// Returns byte 7 of the slot, or 0 when no slot is free.
static uint8_t *spawnEnemyBullet(const uint8_t *p, uint8_t flags)
{
    uint8_t *slot = allocSlot(&g_enemyBulletSlots, 4, 1);
    if (slot == BULLET_NONE)
        return 0;
    uint8_t speed = bank3Byte(&GameB_UnknownCurve6e10[(p[1] & 0x70) >> 4]);
    return initProjectile(slot, flags, speed, WORD(p[2]), 1);
}

// Entry from bank 0. p = enemy ship, e = fire delay, range = distance.
// Fire only when the ship is active, g_enemyFireGate is set, the sub state
// is $14 and the range is $10..$9f. The skill makes the next delay longer.
void enemyFireGate(uint8_t *p, uint8_t e, uint8_t range)
{
    if (!(p[0] & 0x80) || !g_enemyFireGate || g_subState != 0x14)
        return;
    if (range < 0x10 || range >= 0xa0)
        return;
    uint8_t *delay = &g_enemyFireDelays + (p[6] & 0x1f);
    if (*delay && --*delay)
        return;
    if (g_gameBSkill == 1)
        e += e >> 1;
    else if (g_gameBSkill)
        e <<= 1;
    *delay = e;
    uint8_t *s7 = spawnEnemyBullet(p - 1, p[-1] & 0x20);
    if (!s7)
        return;
    *s7 = 0x14;
    s7[-1] |= 0x60;
}

// Entry from bank 0. The enemy ship with byte 6 at p fires a bullet.
void enemyShipFire(uint8_t *p)
{
    uint8_t *q = p - 6;
    uint8_t *s7 = spawnEnemyBullet(q, (q[0] & 0x60) | 0x40);
    if (!s7)
        return;
    // Bank 0 reads the speed (the slot type) for the wreck.
    g_enemyBulletSpeed = s7[-6];
    s7[-1] = (s7[-1] | 0x40) & ~0x20;
    playSfx(6);
}

// Uses one step of ammo i. Returns 1 when it is 0 now.
static uint8_t ammoDecIfNZ(uint8_t i)
{
    uint8_t *a = &g_hpA + i;
    if (*a)
        --*a;
    return !*a;
}

// Refills ammo i. Kill flag bit 6 of the side picks the upgraded value.
static void ammoReload(uint8_t i)
{
    uint8_t up = (&g_killFlagsPending)[(i >> 1) & 1] & 0x40;
    (&g_hpA)[i] = (&g_torpedoAmmoMax)[up ? 1 : 0];
}

// Works out the weapon icon state from the torpedo ammo. Queues HUD
// command 5 when it changed ($d45f holds the last state).
static void updateWeaponHud(void)
{
    uint8_t e;
    if (TWIN_TORPEDO)
        e = g_hpA ? 3 : 0;
    else if (g_hpA)
        e = g_hpB ? 3 : 2;
    else
        e = g_hpB ? 1 : 0;
    if (e == g_livesGameB)
        return;
    g_livesGameB = e;
    hudQueuePush(5, e);
}

// After a shot (sub state $14+), count down the cooldown of tube i,
// clear $d01e bit 1 and update the weapon HUD.
static void postFireAccounting(uint8_t i)
{
    if (g_subState < 0x14)
        return;
    uint8_t *c = &g_torpedoCooldown + i;
    if (*c)
        --*c;
    g_modeFlags1e &= ~0x02;
    updateWeaponHud();
}

// Plays the low-ammo SFX $0d unless the round is over ($d002 bit 1).
static void ammoLowWarn(uint8_t ammo)
{
    if (ammo == 7 && !(g_sceneFlagsC & 0x02))
        playSfx(0x0d);
}

// x - $18. Below 0 it wraps to the world width.
static uint16_t twinShotLeft(uint16_t x)
{
    if (x < 0x18)
        x += WORD(g_worldWidth);
    return x - 0x18;
}

// Counts the shot, then fills a torpedo slot for the player side ($d010 bit 0).
static void torpedoSpawn(uint8_t *slot, uint16_t x)
{
    g_torpedoFireCount++;
    initProjectile(slot, (g_modeFlags10 & 0x01) << 5, 0x38, x, 0);
}

// Clears the A button edge (g_weaponEdges bit 0).
static void clearTorpedoEdge(void)
{
    g_weaponEdges &= ~0x01;
}

// Fires from tube i into the n-th free torpedo slot. It launches only on
// a fresh A press, when the player lives, ships remaining < $40 and the
// sub state is $13 or $14.
static void fireTorpedoAlloc(uint8_t n, uint8_t i)
{
    uint8_t *slot = allocTorpedoSlot(n);
    if (slot == TORPEDO_NONE)
        return;
    postFireAccounting(i);
    if ((&g_torpedoCooldown)[i]) {
        clearTorpedoEdge();
        return;
    }
    if (!(g_weaponEdges & 0x01))
        return;
    g_weaponEdges &= ~0x01;
    if (g_modeFlags10 & 0x10)
        return;
    if (g_playerShipsRemaining >= 0x40)
        return;
    if (g_subState < 0x13 || g_subState >= 0x15)
        return;
    g_firedTorpedoSlot = i;
    playSfx(6);
    ammoReload(i);
    linkTxQueuePush(0x81, 1);
    uint16_t x = SLOT_X(&g_shipArray_Player);
    if (TWIN_TORPEDO) {
        torpedoSpawn(slot, twinShotLeft(x));
        // Twin tubes: a second shot on the other side. With no free slot
        // the asm writes past the torpedo slots (into MG slot 0).
        torpedoSpawn(allocTorpedoSlot(1), moveRight(x, 0x18));
    } else {
        torpedoSpawn(slot, x);
    }
    (&g_torpedoCooldown)[g_firedTorpedoSlot] = 3;
    g_modeFlags1e |= 0x02;
    updateWeaponHud();
}

// Entry from bank 0. A button: fire a torpedo from each tube whose ammo
// timer ends. Kill flag bit 6 means twin tubes share one shot.
void fireTorpedo(void) __banked
{
    if (TWIN_TORPEDO) {
        if (ammoDecIfNZ(0)) {
            fireTorpedoAlloc(2, 0);
            return;
        }
        ammoLowWarn(g_hpA);
        g_hpB = g_hpA;
        clearTorpedoEdge();
        return;
    }
    if (ammoDecIfNZ(0))
        fireTorpedoAlloc(1, 0);
    ammoLowWarn(g_hpA);
    if (ammoDecIfNZ(1)) {
        fireTorpedoAlloc(1, 1);
        return;
    }
    ammoLowWarn(g_hpB);
    clearTorpedoEdge();
}

// In link mode, clears the partner MG edge (g_weaponEdges bit 4).
static void fireMGClearEdge(void)
{
    if (g_sceneFlagsC & 0x08)
        g_weaponEdges &= ~0x10;
}

// Counts the shot, then fills an MG slot for the enemy side ($d011 bit 0).
// With the lock-on state (g_weaponEdges bit 7) the round follows the
// target in g_lockOnTargetIdx. Else byte 7 = $ff.
static void mgSpawn(uint8_t *slot, uint16_t x)
{
    g_mgFireCount++;
    uint8_t *s7 = initProjectile(slot, (g_enemySubFlags & 0x01) << 5, 0x38, x, 1);
    if (!(g_weaponEdges & 0x80)) {
        *s7 = 0xff;
        return;
    }
    *s7 = g_lockOnTargetIdx;
    s7[-1] |= 0x08;
    g_mgLockShots++;
}

// Fires MG i into the n-th free MG slot, on the partner edge
// (g_weaponEdges bit 4), while the sub state is below $15.
static void fireMGAlloc(uint8_t n, uint8_t i)
{
    uint8_t *slot = allocMGSlot(n);
    if (slot == MG_NONE)
        return;
    if (!(g_weaponEdges & 0x10)) {
        fireMGClearEdge();
        return;
    }
    g_weaponEdges &= ~0x10;
    if (g_subState >= 0x15 || g_d2f7Counter >= 0x08 || (g_enemySubFlags & 0x10))
        return;
    if (g_enemySubApproachShadow != 0xd0 && !(g_modeFlags10 & 0x10))
        playSfx(6);
    ammoReload(i);
    uint16_t x = SLOT_X(&g_shipArray_Enemy);
    if (TWIN_MG) {
        mgSpawn(slot, twinShotLeft(x));
        // With no free slot the asm writes past the MG slots.
        mgSpawn(allocMGSlot(1), moveRight(x, 0x18));
    } else {
        mgSpawn(slot, x);
    }
    g_weaponEdges &= ~0x80;
}

// Entry from bank 0. B button: fire the MG. Same plan as the torpedo,
// with ammo 2 and 3.
void fireMachineGun(void) __banked
{
    if (TWIN_MG) {
        if (ammoDecIfNZ(2)) {
            fireMGAlloc(2, 2);
            return;
        }
        g_mgAmmoB = g_mgAmmoA;
        fireMGClearEdge();
        return;
    }
    if (ammoDecIfNZ(2))
        fireMGAlloc(1, 2);
    if (ammoDecIfNZ(3))
        fireMGAlloc(1, 3);
    else
        fireMGClearEdge();
}

// Entry points for the asm glue in bank 3. The arguments come on the stack.
void projectileTickFar(uint8_t *slot) __banked { projectileTick(slot); }
void enemyFireGateFar(uint8_t *p, uint8_t e, uint8_t range) __banked { enemyFireGate(p, e, range); }
void enemyShipFireFar(uint8_t *p) __banked { enemyShipFire(p); }
