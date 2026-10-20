// Range and hit tests, the mirror offsets and the 16-bit divide.
// Bank 0 calls these in its object loops, so they stay in bank 3.
#include "game.h"
#include "bank3.h"

extern const uint8_t GameB_HitRangeBounds[], GameB_SpriteHitWidths[];
extern const uint8_t GameB_DepthScaleDivisors[];
extern const uint8_t *const GameB_ObjSpriteIdPtrs[];
void projectileHitShipFar(uint8_t *slot, uint8_t mg) __banked;

// Returns n / d (low 8 bits). d is below $100.
// The quotient bits go through $d377, like the asm.
uint8_t divide16(uint16_t n, uint8_t d)
{
    // The high byte gives only quotient bits above bit 7. Keep its remainder.
    uint8_t r = n >> 8;
    while (r >= d)
        r -= d;
    uint8_t lo = n, q = 0;
    for (uint8_t i = 8; i; i--) {
        uint8_t carry = r & 0x80;
        r <<= 1;
        if (lo & 0x80)
            r |= 1;
        lo <<= 1;
        q <<= 1;
        if (carry || r >= d) {
            r -= d;
            q |= 1;
        }
    }
    g_gameBLevel = q;
    return q;
}

// If $d092 bit 0 is set, negate d. Then return d - b.
// $d092 bit 1 = borrow.
uint8_t mirrorOffset(uint8_t d, uint8_t b)
{
    if (g_scratch92 & 0x01)
        d = -d;
    g_scratch92 &= ~0x02;
    if (d < b)
        g_scratch92 |= 0x02;
    return d - b;
}

// Divide n by GameB_DepthScaleDivisors[idx], add 8, then mirror the offset by 4.
uint8_t scaledMirrorOffset(uint16_t n, uint8_t idx)
{
    return mirrorOffset(divide16(n, GameB_DepthScaleDivisors[idx]) + 8, 4);
}

// Distance from the reference X ref to x, in a window 2w + 2m wide.
// The world wraps at its width. Returns 1 when x is out of range
// (then *dist = the raw distance). Else *dist = |distance - m| and
// $d092 bit 0 = 1 when the distance is below m.
uint8_t rangeCheck(uint16_t x, uint16_t w, uint16_t *dist, uint16_t ref, uint8_t m)
{
    uint16_t span = w + m;
    uint16_t left = ref - span;
    if (ref < span)
        left += WORD(g_worldWidth);
    uint16_t d = x - left;
    if (x < left)
        d += WORD(g_worldWidth);
    if ((uint16_t)(span << 1) < d) {
        *dist = d;
        return 1;
    }
    if (d >= m) {
        d -= m;
        g_scratch92 &= ~0x01;
    } else {
        d = m - d;
        g_scratch92 |= 0x01;
    }
    *dist = d;
    return 0;
}

// X relative to the reference X at ref, scaled by $d32b. The window
// is half the world wide.
uint8_t relativeX(uint16_t x, const uint8_t *ref)
{
    uint16_t dist;
    rangeCheck(x, WORD(g_worldWidth) >> 1, &dist, *(const uint16_t *)ref, g_spriteDrawMode);
    return divide16(dist, g_worldXScale);
}

// The sprite byte of object class a: GameB_ObjSpriteIdPtrs[(a >> 4) & 7][a & 15].
uint8_t objSpriteByte(uint8_t a)
{
    return GameB_ObjSpriteIdPtrs[(a >> 4) & 0x07][a & 0x0f];
}

// The sprite id: the sprite byte with bits 6 and 7 cleared.
static uint8_t objSpriteId(uint8_t a)
{
    return objSpriteByte(a) & 0x3f;
}

// Test one ship slot against the projectile at pos. Calls
// projectileHitShip on a hit.
static void checkProjectileHit(uint8_t *slot, uint16_t pos)
{
    uint8_t side = g_hitTargetFlags & 0x80;
    uint8_t cls = slot[1];
    uint8_t units = cls & 0x0f;
    g_layoutWidthUnits = units;
    if (units >= 0x0a)
        goto done;
    if (units == 4) {
        // The sub: test it only while it can be hit.
        if (side) {
            if (g_enemySubFlags & 0x10)
                goto done;
        } else if ((g_modeFlags10 & 0x10) || g_playerShipsRemaining >= 0x40) {
            goto done;
        }
    }
    // The ship X goes to $d091/$d092. The range test measures from there.
    g_gameBLayoutVariant = slot[2];
    g_scratch92 = slot[3];
    uint16_t w = GameB_SpriteHitWidths[objSpriteId(cls)];
    if (!side && g_lockOnTargetIdx)
        w <<= 1;
    uint16_t dist;
    if (rangeCheck(pos, w, &dist, WORD(g_gameBLayoutVariant), 0)) {
        // Out of range. A lock-on shot still hits its locked target.
        if (!(g_shotFlags & 0x08) || g_lockOnTargetIdx == 0xff)
            goto done;
        uint8_t idx = slot[7] & 0x0f;
        if (!idx || idx != (g_lockOnTargetIdx & 0x0f) || !(g_weaponEdges & 0x80))
            goto done;
    }
    projectileHitShipFar(slot, side);
done:
    g_setupObjType = 0;
}

// Input: slot = projectile, pos = its position. Find the depth band of
// slot byte 1 in GameB_HitRangeBounds. On an exact match, test the
// ships of the target side.
void projectileCollide(uint8_t *slot, uint16_t pos)
{
    uint8_t v = slot[1];
    g_dispatchTableD346 = v;
    uint8_t i = 0;
    while (i < 8 && GameB_HitRangeBounds[i] < v)
        i++;
    g_depthBand = i;
    uint8_t a = slot[6] & 0xf8;
    if (i == 8 || GameB_HitRangeBounds[i] != v) {
        slot[6] = a;
        return;
    }
    a |= i;
    slot[6] = a;
    g_shotFlags = a;
    g_modeFlags1e &= ~0x80;
    uint8_t *ship = (a & 0x80) ? &g_shipArray_Player : &g_shipArray_Enemy;
    a ^= 0x80;
    g_hitTargetFlags = a;
    uint8_t cls = (a & 0x07) << 4;
    // In link mode, no ship test when flag bits 6 and 7 are both clear.
    if ((g_sceneFlagsC & 0x08) && !(a & 0xc0))
        return;
    for (uint8_t n = 16; n; n--, ship += 8) {
        if (!(ship[0] & 0x80) || (ship[1] & 0x70) != cls)
            continue;
        WORD(g_hitTestPos[0]) = pos;
        checkProjectileHit(ship, pos);
        if (g_modeFlags1e & 0x80)
            return;
    }
}
