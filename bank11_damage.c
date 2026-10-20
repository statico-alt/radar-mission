// Damage, kills, wreck slots, projectile hits, weapon steals and the
// object launch. Side 0 is the player, side 1 is the enemy.
#include "game.h"
#include "bank3.h"

#define ROUND_OVER (g_sceneFlagsC & 0x02)
#define LINK_MODE (g_sceneFlagsC & 0x08)
#define PLAYER_DEAD (g_modeFlags10 & 0x10)

extern const uint8_t ScrollScript_Scene0_SideA[], ScrollScript_Scene1[];
extern const uint8_t ScrollScript_Scene2[], ScrollScript_Scene3[];

// Count a kill unless one is pending (kill flags bits 5-7). Set $d018
// bit 3 and start the gauge blink ($d367 = 5).
static void registerKill(void)
{
    if (g_killFlagsPending & 0xe0)
        return;
    g_modeFlags18 |= 0x08;
    g_gaugeBlinkCount = 5;
}

// Divide the speed at $d2ec by 4.
static void quarterSpeed(void)
{
    g_sideSpeed >>= 2;
}

static void setD01dBit7(void)
{
    g_modeFlags1d |= 0x80;
    (&g_sideSpeed)[1] = 0;
}

// Play music 6 unless g_modeFlags1e bit 2 says it already started.
static void playMusic6Once(void)
{
    if (!(g_modeFlags1e & 0x04))
        playMusic(6);
    g_modeFlags1e |= 0x04;
}

// Store a & $0f in $d449 and copy that ship's X ($d21a + a * 8) to $d45b.
// Returns the index.
uint8_t selectEnemyShip(uint8_t a)
{
    a &= 0x0f;
    g_selectedEnemyShip = a;
    WORD(g_selectedShipX) = WORD((&g_shipArray_Enemy)[(a << 3) + 2]);
    return a;
}

// Clear the torpedo slot with index $d344 and set the flags
// that say a torpedo ended.
static void clearTorpedo(void)
{
    g_modeFlags1e |= 0x80;
    g_oamBufToggle |= 0x40;
    uint8_t *p = &g_torpedoSlots + (g_torpedoSlotIdx << 3);
    for (uint8_t n = 8; n; n--)
        *p++ = 0;
}

// Cycle $d365 through 0..2. Returns the wreck slot at $d200 + index * 8.
static uint8_t *nextWreckSlot(void)
{
    if (++g_wreckSlotCycle >= 3)
        g_wreckSlotCycle = 0;
    return &g_gameBObjSlots + (g_wreckSlotCycle << 3);
}

// Copy the 8 bytes that end at src into the slot.
static void copy8(uint8_t *slot, const uint8_t *src)
{
    src -= 7;
    for (uint8_t n = 8; n; n--)
        *slot++ = *src++;
}

// Pick the next wreck slot and copy into it the 8 bytes that end at src.
// The address of its last byte is kept at $d099.
uint8_t *saveToWreckSlot(const uint8_t *src)
{
    uint8_t *slot = nextWreckSlot();
    WORD(g_sixAnimCounter1) = (uint16_t)(slot + 7);
    copy8(slot, src);
    return slot;
}

// Entry from bank 0. Save the ship that ends at src to a wreck slot and
// turn the slot into a wreck: type $0a | c, timer 1, active, bit 5 set.
void makeWreck(const uint8_t *src, uint8_t c)
{
    uint8_t *slot = saveToWreckSlot(src);
    slot[1] = (slot[1] & 0x80) | 0x0a | c;
    slot[4] = 1;
    slot[5] |= 0x80;
    slot[7] |= 0x20;
}

// Input: side. Output: the launch direction bit. It flips each time the
// launch timer $d460 + side runs. Else it comes from *flags.
static uint8_t pickLaunchSide(const uint8_t *flags, uint8_t side)
{
    uint8_t *toggle = g_launchSideToggle + side;
    uint8_t a = (&g_screenEffectTimers)[side] ? *toggle ^ 0x01 : *flags;
    *toggle = a;
    return a & 0x01;
}

// Fill the 8-byte slot: flags, type $79 (or $f9), X of the launching
// ship, speed 2, index $d40b.
static void initLaunchedObject(uint8_t *slot, uint8_t dir, uint8_t side)
{
    slot[0] = ((dir & 0x01) << 5) | 0x80;
    slot[1] = (side & 1) ? 0xf9 : 0x79;
    WORD(slot[2]) = (side & 1) ? WORD((&g_shipArray_Player)[2]) : WORD((&g_shipArray_Enemy)[2]);
    slot[4] = 2;
    slot[5] = 0;
    slot[6] = 0;
    slot[7] = g_launchIndex;
}

// Put a new object in the first free ship slot after index $d339, then
// tell the link partner (message 6) and set the launch timer for side.
void launchObject(uint8_t dir, uint8_t side)
{
    g_mirrorKeepFlags = dir;
    uint8_t n = 0x0f - g_gameBShipCountM1;
    g_launchIndex = g_gameBShipCountM1 + 1;
    uint8_t *slot = ((side & 1) ? &g_shipArray_Enemy : &g_shipArray_Player) + (uint8_t)(g_launchIndex << 3);
    while (slot[0] & 0x80) {
        g_launchIndex++;
        slot += 8;
        if (!--n)
            return;
    }
    initLaunchedObject(slot, g_mirrorKeepFlags, side);
    uint8_t flags;
    if (side & 1) {
        if ((g_shotFlags & 0xc0) != 0xc0)
            goto setTimer;
        flags = (g_killFlagsPending & 0xe0) | 0x08;
    } else {
        // Side 0 sends only when $d45d is clear, else it clears it.
        if (g_linkSendHold) {
            g_linkSendHold = 0;
            return;
        }
        flags = g_killFlagsPendingB & 0xe0;
    }
    linkTxQueuePush(g_mirrorKeepFlags | (flags >> 2), 6);
setTimer:
    (&g_screenEffectTimers)[side] = 0x2d;
}

// Pick a launch side from the side flags, then launch.
static void shipScanSideAware(uint8_t side)
{
    launchObject(pickLaunchSide(&g_modeFlags10 + ((side & 1) ? 0 : 1), side), side);
}

// Rows of 3 masks, picked by the kill flag bits 5-7.
static const uint8_t weaponStealMasks[] = {
    0xdf, 0x00, 0x00, 0x00, 0xbf, 0x00, 0xdf, 0xbf,
    0x00, 0x00, 0x00, 0x7f, 0xdf, 0x00, 0x7f, 0x00,
    0xbf, 0x7f, 0xdf, 0xbf, 0x7f,
};

// Remove one weapon flag from the other side (side xor 1). Returns 1
// when a flag was removed.
static uint8_t stealWeaponBit(uint8_t side)
{
    uint8_t *flags = &g_killFlagsPending + (side ^ 1);
    uint8_t v = *flags;
    if (!(v & 0xe0))
        return 0;
    uint8_t row = v >> 5;
    const uint8_t *masks = weaponStealMasks + (uint8_t)((row << 1) + row) - 3;
    uint8_t col = (g_frameCounter & 0x01) + ((&g_shipArray_Enemy)[2] & 0x01);
    while (!masks[col])
        if (++col >= 3)
            col = 0;
    v &= masks[col];
    if (side & 1)
        hudQueuePush(0x0d, col);
    *flags = v;
    g_linkSendHold = 0;
    shipScanSideAware(side);
    return 1;
}

// Input: side that was hit. Take one HP from $d184 or $d21c.
// Returns 1 when the ship survives.
static uint8_t damageShip(uint8_t side)
{
    uint8_t *hp, *count;
    if (side & 1) {
        hp = &g_shipArray_Player + 4;
        count = &g_bonusHitCountA;
        if (g_killFlagsPending & 0xe0) {
            if (LINK_MODE)
                return 1;
            goto bonus;
        }
    } else {
        hp = &g_shipArray_Enemy + 4;
        count = &g_bonusHitCountB;
        if (g_killFlagsPendingB & 0xe0)
            goto bonus;
    }
    uint8_t v = *hp;
    if ((v & 0x3f) > 1) {
        *hp = v - 1;
        return 1;
    }
    *hp = v & 0xc0;
    return 0;
bonus:
    // A ship with weapon flags loses a flag every 6 hits.
    if (++*count < 6)
        return 1;
    *count = 0;
    return stealWeaponBit(side);
}

// Set sub state $18, $17 or $1a from the player state flags.
// scratch8 = 1 picks $18 when g_subStateScratch8 is 2.
static void setEndSubState(uint8_t scratch8)
{
    uint8_t s = 0x1a;
    if (PLAYER_DEAD)
        s = 0x18;
    else if (g_modeFlags10 & 0x40)
        s = 0x17;
    else if (scratch8 && g_subStateScratch8 == 2)
        s = 0x18;
    g_subState = s;
}

// Entry from bank 0 (GameB_KillConfirm.skip). Set the death marker and
// pick the end sub state. When the enemy sub is gone, start the win timer.
void killConfirmEnd(void)
{
    if (g_sceneFlagsC & 0x01) {
        g_deathMarker = 0x52;
        g_gameBKillTimer = 4;
        setEndSubState(0);
        return;
    }
    g_deathMarker = (g_sceneFlagsB & 0x20) ? 0xa4 : 0x52;
    g_winCutsceneTimer = 0x1e;
    setEndSubState(1);
}

// Start the level-clear timer ($10) and set the round-over flag ($d002 bit 1).
// In link mode, sub state = $16.
static void killConfirm(void)
{
    g_levelClearTimer = 0x10;
    g_enemySubApproachCounter = 0;
    g_sceneFlagsC |= 0x02;
    if (LINK_MODE) {
        g_subState = 0x16;
        return;
    }
    killConfirmEnd();
}

// Queue the HUD updates for a destroyed ship on side, mark which side
// won in $d002 bit 0 (set for side 0) and run the kill path.
void shipDestroyed(uint8_t side)
{
    if (ROUND_OVER)
        return;
    hudQueuePush(2, 1);
    hudQueuePush(0x0b, 5);
    hudQueuePush(6, 0);
    if (!(side & 1)) {
        g_sceneFlagsC |= 0x01;
    } else {
        g_sceneFlagsC &= ~0x01;
        registerKill();
    }
    if (PLAYER_DEAD && !(&g_hudCounterA)[side & 1])
        g_modeFlags1a |= 0x08;
    killConfirm();
}

// The player-side ship was hit: flash, damage and maybe destroy it.
void playerSubHit(uint8_t side)
{
    g_modeFlags13 |= 0x10;
    g_windowWobbleTimer = 5;
    registerKill();
    if (damageShip(side))
        return;
    g_shipArray_Player |= 0x40;
    shipDestroyed(1);
}

// Entry from the MG trails. Input: trail Y, side. Check a trail
// particle hit on a ship.
void mgTrailHit(uint8_t y, uint8_t side)
{
    if (ROUND_OVER)
        return;
    if (side & 1) {
        if ((g_modeFlags10 & 0x50) || LINK_MODE)
            return;
        playerSubHit(side);
        return;
    }
    // Side 0: hit when the point is within $38 of the sub approach
    // line ($d34a - $1c).
    if (g_enemySubFlags & 0x10)
        return;
    uint8_t d = g_enemySubApproachShadow - 0x1c;
    if (d >= y || (uint8_t)(d + 0x38) < y)
        return;
    g_periscopeFlags = (y - d) | 0x80;
    linkTxQueuePush(((g_killFlagsPending & 0xe0) >> 4) | 0xa0, 2);
    if (damageShip(0))
        return;
    quarterSpeed();
    g_selectedEnemyShip = 0;
    selectEnemyShip(0);
    setD01dBit7();
    g_killCountsByClass[4]++;
    shipDestroyed(0);
}

// Set bit 6 of hp (slot byte 4) and set the hit flash timer
// (low 4 bits of byte 5).
void setHitFlash(uint8_t *hp)
{
    hp[0] |= 0x40;
    hp[1] = (hp[1] & 0x50) | 0x0f;
}

// Clear the shot that hit. On a 4-unit layout a side 1 hit also flashes
// the palette and counts a kill.
static void hitShipEnd(uint8_t side)
{
    if ((side & 1) && g_layoutWidthUnits == 4) {
        g_paletteFlashTimer = 0x1e;
        g_modeFlags13 |= 0x40;
        registerKill();
    }
    clearTorpedo();
}

// The ship lived. A flag with bit 7 set deflects the shot.
// Else play a hit sound for side and flash the ship.
static void hitShipSurvived(uint8_t *hp, uint8_t side)
{
    if (*hp & 0x80) {
        // The shot bounced off: free the slot unless the player is dead.
        if (!PLAYER_DEAD)
            hp[-4] = 0;
        playSfx(0x0b);
        clearTorpedo();
        return;
    }
    uint8_t layout4 = g_layoutWidthUnits == 4;
    if (!(side & 1) || layout4)
        playDeb8(1);
    else
        playSfx(0x0b);
    setHitFlash(hp);
    if ((side & 1) && layout4 && !(g_modeFlags18 & 0x80) && ((&g_shipArray_Player)[4] & 0x1f) < 7)
        playMusic6Once();
    hitShipEnd(side);
}

// The ship has no hit points left. Turn it into a wreck, count the
// kill by class, update the HUD and maybe end the round.
static void hitShipKilled(uint8_t *hp, uint8_t side)
{
    if (ROUND_OVER)
        return;
    uint8_t layout = g_layoutWidthUnits;
    if (!(side & 1) && layout != 9) {
        // Copy the wreck saved by the hit to the next wreck slot. The
        // first copy gets the ship X back.
        uint8_t *src = (uint8_t *)WORD(g_sixAnimCounter1);
        copy8(nextWreckSlot(), src);
        src[0] |= 0x80;
        src[-4] = g_sixAnimCounter0;
        src[-5] = g_gameBAnimSpriteTable;
    }
    hp[0] = (hp[0] & 0xc0) | 0x80;
    hp[1] = (hp[1] & 0xd0) | 0x0f;
    if (!(side & 1) && layout < 9) {
        uint8_t i = layout;
        if (layout >= 6)
            i--;
        if (layout == 8)
            i--;
        g_killCountsByClass[i]++;
        if ((g_hitTargetFlags & 0x70) == 0x70)
            hudQueuePush(6, 0);
    }
    if (layout == 4) {
        quarterSpeed();
        if (!(side & 1)) {
            hp[0] &= ~0x80;
            setD01dBit7();
        }
    } else if (layout == 9) {
        (&g_killCount)[side]++;
        playSfx(4);
        hitShipEnd(side);
        return;
    } else {
        uint8_t *count = &g_hudCounterA + side;
        uint8_t old = (*count)--;
        uint8_t param = old;
        if (side & 1) {
            if (old == 4) {
                g_modeFlags18 |= 0x80;
                g_barBlinkCount = 7;
            }
        } else {
            param |= 0x80;
        }
        hudQueuePush(7, param);
        hudQueuePush(3, param);
        if (old != 1)
            goto sound;
        g_sceneFlagsB |= 0x20;
    }
    shipDestroyed(side);
sound:
    if (side & 1)
        playSfx(0x0b);
    else
        playDeb8(1);
    hitShipEnd(side);
}

// Reduce the ship hit points (low 6 bits of byte 4) by 6 and react.
static void hitShipApplyDamage(uint8_t *hp, uint8_t side)
{
    uint8_t v = *hp & 0x3f;
    if (v > 6) {
        *hp = (*hp & 0xc0) | (v - 6);
        hitShipSurvived(hp, side);
    } else {
        hitShipKilled(hp, side);
    }
}

// Entry from the link code. On a 4-unit layout with a kill pending,
// skip the damage.
void hitShipCheckSub(uint8_t *hp, uint8_t side)
{
    if (g_layoutWidthUnits == 4 && (g_killFlagsPending & 0xe0))
        hitShipSurvived(hp, side);
    else
        hitShipApplyDamage(hp, side);
}

// A projectile hit the ship slot. mg bit 7 = MG shot (side 0), else a
// torpedo (side 1). Applies damage, kill tally, HUD and sound, then
// clears the projectile.
void projectileHitShip(uint8_t *slot, uint8_t mg)
{
    if (ROUND_OVER)
        return;
    uint8_t *hp = slot + 4;
    uint8_t side;
    if (!(mg & 0x80)) {
        g_torpedoHitMarker = 0;
        side = 1;
        if (*hp & 0x80) {
            clearTorpedo();
            return;
        }
        if (!selectEnemyShip(slot[7]))
            linkTxQueuePush(0x10, 2);
    } else {
        g_mgHitMarker = 0;
        side = 0;
        if (*hp & 0x80) {
            clearTorpedo();
            return;
        }
        // Save the ship as a wreck at the hit point. Keep its X.
        selectEnemyShip(slot[7]);
        uint8_t *w = saveToWreckSlot(slot + 7);
        g_gameBAnimSpriteTable = w[2];
        g_sixAnimCounter0 = w[3];
        WORD(w[2]) = WORD(g_hitTestPos[0]);
        w[4] = 1;
        w[5] |= 0x80;
        g_enemyHitCount++;
        linkTxQueuePush(g_selectedEnemyShip | 0x80, 2);
    }
    if (g_layoutWidthUnits == 4 && stealWeaponBit(side))
        hitShipSurvived(hp, side);
    else
        hitShipApplyDamage(hp, side);
}

// Entry from bank 0. Pick the scroll script for the end scene from the
// scene flags and set up the script template at $d3e9.
void sub5b11(void)
{
    const uint8_t *script = ScrollScript_Scene0_SideA;
    uint16_t param = 0x982e;
    uint8_t index = 0;
    if (!(g_sceneFlagsC & 0x01)) {
        index = 1;
        script = ScrollScript_Scene1;
        param = 0x9830;
        if (!(g_sceneFlagsB & 0x20)) {
            g_sceneFlagsB |= 0x40;
            index = 2;
            script = ScrollScript_Scene2;
            param = 0x345c;
            if (g_sceneFlagsC & 0x08) {
                index = 3;
                script = ScrollScript_Scene3;
                param = 0x3c5c;
            }
        }
    }
    g_sceneScriptIndex = index;
    uint8_t *t = &ScrollScript_TemplateFlag;
    t[0] = 1;
    t[1] = 0;
    WORD(t[2]) = (uint16_t)script;
    WORD(t[4]) = param;
    t[6] = 0;
    t[7] = 0;
}

// Entry points for the asm glue in bank 3. The arguments come on the stack.
void mgTrailHitFar(uint8_t y, uint8_t side) __banked { mgTrailHit(y, side); }
void playerSubHitFar(uint8_t side) __banked { playerSubHit(side); }
void shipDestroyedFar(uint8_t side) __banked { shipDestroyed(side); }
void hitShipCheckSubFar(uint8_t *hp, uint8_t side) __banked { hitShipCheckSub(hp, side); }
void setHitFlashFar(uint8_t *hp) __banked { setHitFlash(hp); }
void launchObjectFar(uint8_t dir, uint8_t side) __banked { launchObject(dir, side); }
void killConfirmEndFar(void) __banked { killConfirmEnd(); }
void sub5b11Far(void) __banked { sub5b11(); }
void makeWreckFar(const uint8_t *src, uint8_t c) __banked { makeWreck(src, c); }
uint8_t *saveToWreckSlotFar(const uint8_t *src) __banked { return saveToWreckSlot(src); }
void projectileHitShipFar(uint8_t *slot, uint8_t mg) __banked { projectileHitShip(slot, mg); }
