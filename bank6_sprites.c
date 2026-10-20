// Game B metasprites: the draw routine, the six animated sprites and the death tallies.
#include "game.h"

// Metasprite data: pairs of (tile, attr). A tile of $fd or more is a command.
#define MSPR_ROW  0xfd  // move down 8 pixels, back to the left edge
#define MSPR_SKIP 0xfe  // move right 8 pixels with no sprite
#define MSPR_END  0xff

extern const uint8_t *const MetaspritePtrs[];
#define ANIM_SPRITES (MetaspritePtrs + 29)
#define TALLY_ICONS  (MetaspritePtrs + 45)

// Draws metasprite table[index] into shadow OAM with its top-left corner at (x, y).
// Sprites go to $c000, or to $c100 after the first 40. g_metaspriteOamSlot counts them.
void drawMetasprite(const uint8_t *const *table, uint8_t index, uint8_t x, uint8_t y)
{
    const uint8_t *m = table[index];
    uint8_t slot = g_metaspriteOamSlot;
    uint8_t *oam = (uint8_t *)0xc000;
    if (slot >= 40) {
        oam = (uint8_t *)0xc100;
        slot -= 40;
    }
    oam += slot * 4;
    g_metaspriteStartX = x;

    for (;;) {
        uint8_t tile = *m++;
        if (tile == MSPR_ROW) {
            y += 8;
            x = g_metaspriteStartX;
            continue;
        }
        if (tile > MSPR_SKIP)
            return;
        if (tile != MSPR_SKIP) {
            *oam++ = y;
            *oam++ = x;
            *oam++ = tile;
            *oam++ = *m++;
            g_metaspriteOamSlot++;
        }
        x += 8;
    }
}

// (Y, X) of each animated sprite.
static const uint8_t sixSpritePositions[6][2] = {
    {0x58, 0x30}, {0x57, 0x3c}, {0x64, 0x34},
    {0x58, 0x70}, {0x57, 0x7c}, {0x64, 0x74},
};

// Draws 6 animated metasprites when g_sceneFlagsC bits 7, 3, 1 and 0 are all set.
// Sprite i has a counter at g_gameBAnimSpriteTable + i. Its high nibble is the frame (0-4).
void drawSixAnimSprites(void)
{
    if ((g_sceneFlagsC & 0x8b) != 0x8b)
        return;
    for (uint8_t i = 6; i; i--) {
        uint8_t *counter = &g_gameBAnimSpriteTable + i;
        uint8_t frame = *counter >> 4;
        (*counter)++;
        if (frame >= 5) {
            frame = 0;
            *counter = 0;
        }
        drawMetasprite(ANIM_SPRITES, frame + 3, sixSpritePositions[i - 1][1], sixSpritePositions[i - 1][0]);
    }
}

// Draws one icon per death for a side, 32 pixels apart. The icon grows
// (counter g_tallyIconGrow) when the scene is won or when bit 0 of
// g_sceneFlagsC equals the side. Otherwise it blinks with bit 0 of g_soundClock.
static void drawSideTally(uint8_t side, uint8_t y)
{
    ScrollScript_LastTileByte = side;  // the asm used this byte as scratch; kept for the same RAM state
    for (uint8_t n = (&g_deathTallyA)[side]; n; n--) {
        uint8_t x = 0x20 + (n << 5);
        uint8_t frame;
        uint8_t iconY = y;
        if ((g_sceneFlagsB & 0x80) || (g_sceneFlagsC & 1) == side) {
            frame = 9;
            if (g_tallyIconGrow < 0x30) {
                g_tallyIconGrow++;
                frame = 8;
            }
        } else {
            uint8_t blink = g_soundClock & 1;
            iconY -= blink;
            frame = 6 + blink;
        }
        drawMetasprite(TALLY_ICONS, frame, x, iconY);
    }
}

// Draws the death tallies: side 1 at Y = $18, then side 0 at Y = $88.
void drawDeathTallies(void)
{
    drawSideTally(1, 0x18);
    drawSideTally(0, 0x88);
}
