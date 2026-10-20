// Bank 1 helpers: sprite animation scripts and the OAM DMA setup.
#include "bank1.h"

// One 4-byte record per active script, at $c883:
// +0 script index, +1 wait timer, +2/+3 offset into the script.
#define ANIM_RECORDS (&g_animRecords - 1)

// The record of the next new anim script.
uint8_t *animGetNextRecordPtr(void)
{
    return ANIM_RECORDS + (uint8_t)(g_animScriptCount << 2);
}

// Clear all sprite animation script records at $c883.
// Clears $ffae bit 6, so UpdateAnimScripts stops.
void clearAnimScripts(void)
{
    g_gameFlags6 &= ~0x40;
    fillBytes(ANIM_RECORDS, 0, 0x84);
}

// Run the commands of the script in rec until a wait or the end.
// oam = the first sprite of the script. Returns 1 at the end command.
static uint8_t runAnimCommands(uint8_t *rec, uint8_t *oam)
{
    const uint8_t *base = ((const uint8_t **)&g_animScriptPtrs)[rec[0]];
    const uint8_t *p = base + WORD(rec[2]);
    for (;;) {
        uint8_t cmd = *p++;
        if (cmd == 0xff)
            return 1;
        uint8_t arg = *p++;
        uint8_t *sprite = oam + (uint8_t)(arg << 2);
        if (cmd == 0xf7) {
            rBGP = arg;
        } else if (cmd == 0xf8) {
            SFX_Queue = arg;
        } else if (cmd == 0xf9) {
            *sprite = 0xf0;
        } else if (cmd == 0xfa || cmd == 0xfe) {
            // Wait arg frames. $fa also goes back to the script start.
            rec[1] = arg;
            WORD(rec[2]) = cmd == 0xfa ? 0 : p - base;
            return 0;
        } else if (cmd == 0xfb) {
            NoiseSfx_Queue = arg;
        } else if (cmd == 0xfc) {
            copyBytes(sprite, p, 4);
            p += 4;
        } else {
            // $fd
            sprite[2] = *p++;
        }
    }
}

// Run one frame of the sprite animation scripts.
// Runs only when bit 6 of $ffae is set.
// $c880 = number of active scripts, $c881 = sprites per script,
// $c882 = first OAM slot, $c878 = table of script base pointers.
// Script bytes are commands $f7-$ff:
//   $f7 v: rBGP = v            $f8 v: play sound effect v
//   $f9 n: hide sprite n       $fa n: restart script, wait n frames
//   $fb v: $deb8 = v (sound)   $fc n y x t a: write OAM entry n
//   $fd n t: tile of sprite n  $fe n: wait n frames
//   $ff: end, remove the script and its sprites
void updateAnimScripts(void)
{
    if (!(g_gameFlags6 & 0x40))
        return;
    g_animSlotIndex = 0;
    uint8_t step = g_animSpritesPerScript << 2;
    uint8_t *oam = &g_oamBuffer + (uint8_t)(g_animFirstOamSlot << 2);
    uint8_t *rec = ANIM_RECORDS;
    do {
        if (rec[1]) {
            rec[1]--;
        } else if (runAnimCommands(rec, oam)) {
            // Remove this record and the sprites of this script. Both
            // copies move everything after it down. The sizes are 8-bit,
            // as in the asm.
            uint8_t left = g_animScriptCount - g_animSlotIndex - 1;
            if (left) {
                copyBytes(rec, rec + 4, (uint8_t)(0xa0 - (left << 2)));
                uint8_t used = multiply8(g_animSlotIndex + 1, step);
                copyBytes(oam, oam + step, (uint8_t)(0xa0 - used - (g_animFirstOamSlot << 2)));
            }
            g_animSlotIndex--;
            if (!--g_animScriptCount) {
                clearAnimScripts();
                return;
            }
            oam -= step;
            rec -= 4;
        }
        oam += step;
        rec += 4;
    } while (++g_animSlotIndex != g_animScriptCount);
}

// Copy OamDmaRoutine to HRAM at $ff80.
void copyOamDmaToHram(void)
{
    copyBytes((uint8_t *)0xff80, OamDmaRoutine, 10);
}

// Read 2 bytes of bank 1 data for code in another bank.
// The low byte is p[0].
uint16_t peek1w(const uint8_t *p) __banked
{
    return *(const uint16_t *)p;
}

// Copy count bytes of bank 1 data to dst, for code in another bank.
void copyFromBank1(uint8_t *dst, const uint8_t *src, uint8_t count) __banked
{
    while (count--)
        *dst++ = *src++;
}

// Copy 4 tile and attribute pairs from tiles (bank 1 data) into the 4
// OAM entries at oam.
void setMetaspriteTiles4(uint8_t *oam, const uint8_t *tiles) __banked
{
    for (uint8_t n = 4; n; n--, oam += 4) {
        oam[2] = *tiles++;
        oam[3] = *tiles++;
    }
}

// The first OAM entry of the next new anim script:
// $c000 + 4 * (first slot + script count * sprites per script).
uint8_t *animOamPtr(void)
{
    return &g_oamBuffer + multiply8(g_animScriptCount, g_animSpritesPerScript << 2)
        + (uint8_t)(g_animFirstOamSlot << 2);
}
