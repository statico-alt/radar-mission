// Game B HUD: the HUD commands, RLE tile drawing, the scroll script,
// the blink timers and the window slide. They read bank 3 tables, so
// they stay in bank 3.
#include "game.h"
#include "bank3.h"

// Asm routines.
void DrawTilemapScript(const uint8_t *script);  // bank 0 rst $28, DE = script
// The RLE routines in bank_003.asm. $ff, n, tile = write tile n times. $ff, $ff = end.
void drawTilesRLE(uint8_t *dst, const uint8_t *src);
void drawTilesRLETwoParts(uint8_t *dst, const uint8_t *src);
void SetPalettes(uint8_t set);
void PlaySFX(uint8_t id);
void PlayMusic(uint8_t id);
void hudQueuePushFar(uint8_t cmd, uint8_t param) __banked;

extern const uint8_t *const HudStringPtrs[];
extern const uint8_t *const HudIconPtrs[];
extern const uint8_t KillMarkerColumnOffsets[], Hud2x2IconTiles[];
extern const uint8_t GameB_StatusPatternTiles[];
extern const uint8_t GameB_StatusScreenTilemap[], GameB_StatusScreenStrings[];
extern const uint8_t WeaponIconLowerHudCells[], WeaponIconUpperHudCells[], WeaponIconTiles[];
extern const uint8_t HudPanelColumnOffsets[], HudPanelTiles[], HudPanelBlankTiles[];
extern const uint8_t HudBarCellsA[], HudBarCellsA_LinkLast[], HudBarCellsA_Last[];
extern const uint8_t HudBarCellsB[], HudBarCellsB_LinkLast[], HudBarCellsB_Last[];
extern const uint8_t HudColumnFallbackData[];

#define MAP_ROW 0x20
#define HUD_ROW(cell) ((uint8_t *)(0x9c00 | (cell)))
#define LINK_MODE (g_sceneFlagsC & 0x08)
#define ROUND_OVER (g_sceneFlagsC & 0x02)
#define PLAYER_HP (&g_shipArray_Player)[4]

// ------------------------------------------------------------------
// The queue: 20 two-byte entries (command, parameter) at g_hudQueue.
// Bank 0 (HudQueue_RunNext) runs one command per call through HudCmdTable.
// hudQueuePush is in bank11_queues.c.
// ------------------------------------------------------------------

// The parameter of the command that runs now.
static uint8_t hudParam(void)
{
    return (&g_hudQueue)[(uint8_t)(g_hudQueueRead << 1) + 1];
}

// ------------------------------------------------------------------
// Tile drawing helpers.
// ------------------------------------------------------------------

// Fills n bytes at d with value. n = 0 fills 256.
// The fill loops have no return value, so SDCC keeps them in registers.
static void fillRow(uint8_t *d, uint8_t value, uint8_t n)
{
    do {
        *d++ = value;
    } while (--n);
}

// Copies one row of g_gameBLayoutVariant tiles from s to d. Returns d after the row.
static uint8_t *copyPatternRow(uint8_t *d, const uint8_t *s)
{
    uint8_t n = g_gameBLayoutVariant;
    do {
        *d++ = *s++;
    } while (--n);
    return d;
}

// Draws one VRAM string from a pointer table.
void drawTableString(const uint8_t *const *table, uint8_t index)
{
    DrawTilemapScript(table[index]);
}

// Draws groups rows of tiles at dst. A row of width g_gameBLayoutVariant tiles is
// drawn g_scratch92 times, one under the other. Each group uses the next row.
// When the width is odd, the next group starts one tile to the left.
uint8_t *drawRepeatedPattern(uint8_t *dst, const uint8_t *row, uint8_t groups)
{
    do {
        uint8_t times = g_scratch92;
        do {
            dst = copyPatternRow(dst, row);
        } while (--times);
        row += g_gameBLayoutVariant;
        if (g_gameBLayoutVariant & 1)
            dst--;
    } while (--groups);
    return dst;
}

// Entry from bank 0. Clears the top 3 tile rows of BG map $9800,
// draws the 11x11 pattern, then draws the RLE tiles after it.
void gameBDrawStatusPattern(void)
{
    fillRow((uint8_t *)0x9800, 1, 0x60);
    g_gameBLayoutVariant = 3;
    g_scratch92 = 11;
    drawTilesRLE(drawRepeatedPattern((uint8_t *)0x9860, GameB_StatusPatternTiles, 11),
                 GameB_StatusPatternTiles + 3 * 11);
}

// Draws 2x2 icon number icon (4 tiles from Hud2x2IconTiles) at dst.
static void draw2x2Icon(uint8_t icon, uint8_t *dst)
{
    const uint8_t *t = Hud2x2IconTiles + (uint8_t)(icon << 2);
    *dst++ = *t++;
    *dst = *t++;
    dst += MAP_ROW - 1;
    *dst++ = *t++;
    *dst = *t;
}

// Draws a two-digit number. Digit tiles start at $04. The tens go left of ones.
static void drawNumber2(uint8_t value, uint8_t *ones)
{
    uint8_t tens = 0;
    while (value >= 10) {
        value -= 10;
        tens++;
    }
    ones[0] = value + 4;
    ones[-1] = tens + 4;
}

// ------------------------------------------------------------------
// HUD commands.
// ------------------------------------------------------------------

// Draws a 2x2 kill marker on the $99df map row.
// Bit 7 of marker set = use the lit tile.
static void drawKillMarker(uint8_t marker)
{
    uint8_t *dst = (uint8_t *)0x99df + KillMarkerColumnOffsets[marker & 0x7f];
    draw2x2Icon(marker >> 7, dst);
}

// HUD command 11: draw string number <param> from HudStringPtrs.
void hudCmdDrawString(void)
{
    drawTableString(HudStringPtrs, hudParam());
}

// HUD command 9: draw kill marker <param>.
void hudCmdDrawKillMarker(void)
{
    drawKillMarker(hudParam());
}

// Draws one lit kill marker per kill in g_killCount.
static void drawKillCount(void)
{
    for (uint8_t n = g_killCount; n; n--)
        drawKillMarker((n - 1) | 0x80);
}

// HUD command 3: draw the numbers in g_hudCounterB and g_hudCounterA.
void hudCmdDrawCounters(void)
{
    drawNumber2(g_hudCounterB, (uint8_t *)0x9e13);
    drawNumber2(g_hudCounterA, (uint8_t *)0x9d73);
}

// HUD command 4: draw the number in g_sideSpeed at $9cb1.
void hudCmdDrawD2ec(void)
{
    drawNumber2(g_sideSpeed, (uint8_t *)0x9cb1);
}

// HUD command 2: draw the 8-tile hit point gauge at $9c46.
// Parameter 0 draws it empty. The player HP is capped at 16 (2 per tile).
void hudCmdDrawGauge(void)
{
    uint8_t lit = hudParam();
    uint8_t hp = 0;
    uint8_t *dst = (uint8_t *)0x9c46;
    if (lit) {
        hp = PLAYER_HP & 0x3f;
        if (hp > 0x10)
            hp = 0x10;
    }
    uint8_t n = 8;
    do {
        uint8_t tile = 0x33;  // empty
        if (hp >= 2) {
            tile = 0x37;  // full
            hp -= 2;
        } else if (hp) {
            tile = 0x34;  // half
            hp = 0;
        }
        *dst++ = tile;
    } while (--n);
    if (lit)
        g_modeFlags18 &= ~0x04;
    else
        g_modeFlags18 |= 0x04;
}

// Copies 2 rows of 6 tiles from src to dst. Zero tiles are skipped.
static void copyNonZeroRows(const uint8_t *src, uint8_t *dst)
{
    uint8_t row = 2;
    do {
        uint8_t n = 6;
        do {
            uint8_t t = *src++;
            if (t)
                *dst = t;
            dst++;
        } while (--n);
        dst += MAP_ROW - 6;
    } while (--row);
}

// HUD command 10: draw a panel (2 rows of 6 tiles). Bit 3 = blank panel.
// The first call (bit 2 clear) draws the top half and queues the bottom half.
// The bottom half starts at the second row of the data, 2 map rows lower.
// The commands run in VBlank, so the push comes after the VRAM writes.
void hudCmdDrawPanel(void)
{
    uint8_t p = hudParam();
    const uint8_t *src = (p & 0x08) ? HudPanelBlankTiles : HudPanelTiles;
    uint8_t col = HudPanelColumnOffsets[p & 3];
    if (p & 0x04) {
        src += 6;
        col += 0x40;
    }
    copyNonZeroRows(src, (uint8_t *)0x9900 + col);
    if (!(p & 0x04))
        hudQueuePushFar(10, p | 0x04);
}

// HUD command 6: draw icon <param> from HudIconPtrs.
void hudCmdDrawIcon(void)
{
    const uint8_t *src = HudIconPtrs[hudParam()];
    uint8_t *dst = (uint8_t *)0x9840;
    *(uint8_t *)0x9821 = *src++;
    uint8_t n = 5;
    do {
        *dst++ = *src++;
    } while (--n);
}

// In link mode, adjust a by c: g_sceneFlagsC bit 2 clear = a + c, set = a + 1 - c.
static uint8_t gameBSideAdjust(uint8_t a, uint8_t c)
{
    if (!LINK_MODE)
        return a;
    if (g_sceneFlagsC & 0x04)
        return a + 1 - c;
    return a + c;
}

// Flips bit 0 of a, but not in link mode.
static uint8_t sideFlip(uint8_t a)
{
    return LINK_MODE ? a : a ^ 1;
}

// Entry from bank 0.
uint8_t gameBGetSideAdjustFlipped(uint8_t c)
{
    return sideFlip(gameBSideAdjust(0, c));
}

// Entry from bank 0. Draws the 2x2 icon of each side: at $9d40 and $9de0,
// or at $9822 and $99e2 when the sub state is 1. The asm used c as the
// side adjust input. c is 0 at both bank 0 callers.
void gameBDrawSideHeader(void)
{
    uint8_t top = 2, bottom = 3;
    if (gameBGetSideAdjustFlipped(0)) {
        top = 3;
        bottom = 2;
    }
    if (g_subState == 1) {
        draw2x2Icon(top + 2, (uint8_t *)0x9822);
        draw2x2Icon(bottom + 2, (uint8_t *)0x99e2);
    } else {
        draw2x2Icon(top, (uint8_t *)0x9d40);
        draw2x2Icon(bottom, (uint8_t *)0x9de0);
    }
}

// Draws tiles t to t+3 as a 2x2 block at dst.
static void draw2x2Seq(uint8_t *dst, uint8_t t)
{
    *dst++ = t++;
    *dst = t++;
    dst += MAP_ROW - 1;
    *dst++ = t++;
    *dst = t;
}

// Draws weapon icon (2x2 tiles). Bit 0 = lit, the upper bits = icon slot.
// From sub state $0c the icon goes to the lower HUD, and only when lit.
static void drawWeaponIcon(uint8_t icon)
{
    uint8_t tile = WeaponIconTiles[icon];
    uint8_t slot = icon >> 1;
    if (g_subState < 0x0c)
        draw2x2Seq((uint8_t *)(0x9800 | WeaponIconUpperHudCells[slot]), tile - 0x80);
    else if (icon & 1)
        draw2x2Seq(HUD_ROW(WeaponIconLowerHudCells[slot]), tile - 0x1c);
}

// HUD command 8: draw three weapon icons. They are lit by the top bits of g_killFlagsPending.
void hudCmdDrawWeaponIcons(void)
{
    uint8_t flags = g_killFlagsPending;
    for (uint8_t icon = 0; icon != 6; icon += 2) {
        drawWeaponIcon(icon | (flags >> 7));
        flags <<= 1;
    }
}

// HUD command 13: clear weapon icon 2 - <param> in the lower HUD.
void hudCmdClearIcon(void)
{
    draw2x2Icon(0x0e, HUD_ROW(WeaponIconLowerHudCells[2 - hudParam()]));
}

#define HEADER_CLEAR 0x2a
// Draws tiles t, t+1, t+2 at dst. HEADER_CLEAR fills 3 tiles with HEADER_CLEAR.
static void draw3Tiles(uint8_t t, uint8_t *dst)
{
    if (t == HEADER_CLEAR) {
        dst[0] = dst[1] = dst[2] = t;
        return;
    }
    dst[0] = t;
    dst[1] = t + 1;
    dst[2] = t + 2;
}
#define drawHeaderLeft(t) draw3Tiles(t, (uint8_t *)0x9c06)
#define drawHeaderRight(t) draw3Tiles(t, (uint8_t *)0x9c0b)

// HUD command 5: draw or clear the two header parts.
// 0 = both, 1 = left only, 2 = right only, 3 = none.
void hudCmdDrawHeader(void)
{
    uint8_t p;
    if (g_subState >= 0x22)
        return;
    p = hudParam();
    if (p > 3)
        return;
    if (p == 1)
        drawHeaderRight(HEADER_CLEAR);
    drawHeaderLeft((p & 2) ? HEADER_CLEAR : 0x16);
    if (p != 1)
        drawHeaderRight((p & 1) ? HEADER_CLEAR : 0x19);
}

// Writes tile to count map cells in row $9cxx. The low address bytes come
// from the table that ends at last (read backward). Flag bit 1 = tile 3.
static void drawBarTiles(uint8_t tile, uint8_t count, const uint8_t *last, uint8_t flags)
{
    if (flags & 2)
        tile = 3;
    if (!count)
        return;
    do {
        *HUD_ROW(*last--) = tile;
    } while (--count);
}

// Plays music 6 once (g_modeFlags1e bit 2).
static void playMusic6Once(void)
{
    if (!(g_modeFlags1e & 0x04))
        PlayMusic(6);
    g_modeFlags1e |= 0x04;
}

static uint8_t clamp12(uint8_t a)
{
    return a > 12 ? 12 : a;
}

// True when the player side uses the second tile set (g_sceneFlagsC bit 2,
// flipped in link mode).
static uint8_t otherSide(void)
{
    uint8_t f = g_sceneFlagsC;
    if (f & 0x08)
        f ^= 0x04;
    return f & 0x04;
}

// Draws one bar. Flag bit 0 = bar A (else bar B), bit 1 = draw the empty tile.
// After bar B, counts down the bar blink.
void hudCmd14(uint8_t flags)
{
    if (flags & 1) {
        drawBarTiles(otherSide() ? 0x24 : 0x2f, clamp12(g_hudCounterA),
                     LINK_MODE ? HudBarCellsA_LinkLast : HudBarCellsA_Last, flags);
        return;
    }
    drawBarTiles(otherSide() ? 0x2f : 0x24, clamp12(g_hudCounterB),
                 LINK_MODE ? HudBarCellsB_LinkLast : HudBarCellsB_Last, flags);
    if (!(g_modeFlags18 & 0x80))
        return;
    if (flags & 2)
        g_modeFlags18 |= 0x40;
    else
        g_modeFlags18 &= ~0x40;
    if (g_barBlinkCount && --g_barBlinkCount)
        return;
    g_modeFlags18 &= ~0x80;
    if (ROUND_OVER)
        return;
    if ((PLAYER_HP & 0x1f) >= 7)
        playMusic6Once();
}

// HUD command 14.
void hudCmdDrawBars(void)
{
    hudCmd14(hudParam());
}

// HUD command 7: draw a mark on bar B, or on bar A when bit 7 is set.
void hudCmdDrawMark(void)
{
    uint8_t p = hudParam();
    const uint8_t *cells = HudBarCellsB;
    if (!p)
        return;
    if (p & 0x80) {
        p &= 0x7f;
        cells = HudBarCellsA;
    }
    if (LINK_MODE) {
        cells++;
        p += 4;
    }
    if (p > 12)
        return;
    *HUD_ROW(cells[12 - p]) = g_subState >= 0x1d ? 0x21 : 0x03;
}

// HUD command 16: draw a column of 10 tiles at $98a0 + <param>.
// The tiles come from the pointer at g_scratch92 (or the fallback data when
// g_introFadeHi >= 2). The first byte is the count of blank tiles on top.
void hudCmdDrawColumn(void)
{
    uint8_t *dst = (uint8_t *)(0x9800 | ((hudParam() & 0x1f) + 0xa0));
    const uint8_t *src = (const uint8_t *)WORD(g_scratch92);
    uint8_t left = 10;
    uint8_t blank;
    if (g_introFadeHi >= 2)
        src = HudColumnFallbackData;
    blank = *src++;
    if (blank) {
        do {
            *dst = 0;
            dst += MAP_ROW;
            left--;
        } while (--blank);
        if (!left)
            return;
    }
    do {
        *dst = *src++;
        dst += MAP_ROW;
    } while (--left);
    WORD(g_scratch92) = (uint16_t)src;
}

// HUD command 1 (and an entry from bank 0 with the parameter in e).
// Draws two 2x2 icons at $9925 and $992d.
void gameBDrawPair(uint8_t p)
{
    uint8_t left, right;
    if (g_gameBStage) {
        left = right = p + 6;
    } else {
        left = 5;
        right = 4;
        if (g_sceneFlagsC & 0x04) {
            left = 4;
            right = 5;
        }
        if (p)
            left = 0x0f;
    }
    draw2x2Icon(left, (uint8_t *)0x9925);
    draw2x2Icon(right, (uint8_t *)0x992d);
}

void hudCmdDrawPair(void)
{
    gameBDrawPair(hudParam());
}

// ------------------------------------------------------------------
// Scroll script. Streams tile columns and banners for the scene intro.
// ------------------------------------------------------------------

#define SCRIPT_PTR WORD(ScrollScript_PrimaryPtr_Lo)
#define SCRIPT_DST WORD(ScrollScript_VramDst_Lo)
#define SCRIPT_NEXT_ROW 0xfe
#define SCRIPT_END 0xff

// End the scroll scene: sub state $1e, death marker $90, palette set 4.
void scrollScriptEndScene(void)
{
    g_subState = 0x1e;
    g_deathMarker = 0x90;
    SetPalettes(4);
}

// HUD command 12: write the next script tile to VRAM.
// $fe starts the next row. The $fe tile is also written (at the old index).
void scrollScriptStreamToVram(void)
{
    uint8_t index = ScrollScript_Index;
    const uint8_t *p = (const uint8_t *)SCRIPT_PTR + index;
    uint8_t *dst = (uint8_t *)SCRIPT_DST;
    uint8_t tile = *p;
    if (tile == SCRIPT_END) {
        scrollScriptEndScene();
        return;
    }
    if (tile == SCRIPT_NEXT_ROW) {
        dst += MAP_ROW;
        SCRIPT_PTR = (uint16_t)(p + 1);
        SCRIPT_DST = (uint16_t)dst;
        ScrollScript_Index = 0;
    }
    dst[index] = tile;
    ScrollScript_Index++;
}

// Writes a shadow OAM entry: Y, X, the last script byte, attribute $10.
static void queueSprite(uint8_t *oam, uint8_t y, uint8_t x)
{
    oam[0] = y;
    oam[1] = x;
    oam[2] = ScrollScript_LastTileByte;
    oam[3] = 0x10;
}

// Draws the first count script bytes as sprites, two per byte (one in each
// shadow OAM half). $fe moves to the next row. $ff stops.
static void renderSweep(uint8_t count)
{
    uint8_t y = ScrollScript_VramDst_Lo;
    uint8_t x = ScrollScript_VramDst_Hi;
    const uint8_t *p = (const uint8_t *)SCRIPT_PTR;
    do {
        uint8_t tile = *p++;
        ScrollScript_LastTileByte = tile;
        if (tile == SCRIPT_NEXT_ROW) {
            y += 0x0c;
            x = ScrollScript_VramDst_Hi;
        } else if (tile == SCRIPT_END) {
            return;
        } else {
            uint8_t slot = ScrollScript_SlotA_Cursor++;
            g_metaspriteOamSlot = slot + 1;
            queueSprite((uint8_t *)0xc000 + (uint8_t)(slot << 2), y, x);
            slot = ScrollScript_SlotB_Cursor++;
            g_metaspriteOamSlot = slot + 1;
            queueSprite((uint8_t *)0xc100 + (uint8_t)(slot << 2), y, x);
            x += 8;
        }
    } while (--count);
}

// Steps the scroll script: on even frames show one more byte. Draws the shown part.
// Returns 1 when the script is done.
uint8_t gameBAdvanceScrollScript(void)
{
    uint8_t index = ScrollScript_Index;
    ScrollScript_SlotA_Cursor = 0x14;
    ScrollScript_SlotB_Cursor = 0x14;
    if (!(g_frameCounter & 1)) {
        if (((const uint8_t *)SCRIPT_PTR)[index] == SCRIPT_END) {
            renderSweep(index + 1);
            return 1;
        }
        ScrollScript_Index++;
    }
    renderSweep(index + 1);
    return 0;
}

// Entry from bank 0. Draws the Game B status screen: tilemap, weapon icons and kill count.
void gameBDrawStatusScreen(void)
{
    drawTilesRLETwoParts((uint8_t *)0x9800, GameB_StatusScreenTilemap);
    hudCmdDrawWeaponIcons();
    drawKillCount();
    DrawTilemapScript(GameB_StatusScreenStrings);
}

// Entry from bank 0. Clears g_sceneFlagsB bit 7. Keeps only bits 2-3 of
// g_sceneFlagsC and copies the old value to the byte after it.
void gameBResetSceneFlags(void)
{
    uint8_t old = g_sceneFlagsC;
    g_sceneFlagsB &= ~0x80;
    g_sceneFlagsC = old & 0x0c;
    (&g_sceneFlagsC)[1] = old;
}

void clearDamageGauge(void)
{
    fillRow(&g_rasterCount, 0, 0x3a);
}

void clearMgTrails(void)
{
    fillRow(&g_mgTrailA, 0, 0x40);
}

// ------------------------------------------------------------------
// Blink timers and the window slide.
// ------------------------------------------------------------------

// Every 8 frames, counts down g_gaugeBlinkCount and redraws the gauge lit or
// unlit. At 0, stops the blink. Returns the byte that the asm left in hl:
// the count when it queued a redraw, else g_modeFlags18.
static uint8_t *gaugeBlinkTick(void)
{
    if (g_frameCounter & 0x04)
        return &g_modeFlags18;
    if (!g_gaugeBlinkCount) {
        g_modeFlags18 &= ~0x08;
        return &g_modeFlags18;
    }
    hudQueuePushFar(2, g_gaugeBlinkCount-- & 1);
    return &g_gaugeBlinkCount;
}

// Gauge blink (g_modeFlags18 bit 3) and bar blink (bit 7).
// The bar blink bits are tested on the byte that gaugeBlinkTick returns (an asm quirk).
static void blinkTick(void)
{
    uint8_t *flags = &g_modeFlags18;
    uint8_t n;
    if (g_modeFlags18 & 0x08)
        flags = gaugeBlinkTick();
    if (!(*flags & 0x80))
        return;
    if (g_frameCounter & 0x0c)
        return;
    n = g_barBlinkCount;
    if (!n) {
        *flags &= ~0x80;
        return;
    }
    hudQueuePushFar(0x0e, ((n & 1) ^ 1) << 1);
    if (n & 1)
        PlaySFX(0x0b);
}

// Entry from bank 0. Runs the blink timers. With low HP (under 7), blinks string 4/5.
void gameBHudBlinkUpdate(void)
{
    if (ROUND_OVER)
        return;
    blinkTick();
    if ((PLAYER_HP & 0x1f) >= 7)
        return;
    if (g_frameCounter & 0x1c)
        return;
    if (!g_gaugeBlinkCount) {
        g_modeFlags18 &= ~0x08;
        hudQueuePushFar(2, 1);
    }
    hudQueuePushFar(0x0b, (g_gaugeBlinkCount-- & 1) + 4);
}

// Starts the window slide. Toggles g_modeFlags16 bit 4.
// Sets the window Y target to $70 (show) or $90 (hide).
void hudToggleWindow(void)
{
    uint8_t f = (g_modeFlags16 ^ 0x10) | 0x20;
    g_windowAnimTimer = 0x20;
    g_modeFlags16 = f;
    g_windowYTarget = (f & 0x10) ? 0x90 : 0x70;
    rWY = g_windowYTarget;
}

// Moves rWY one line per two frames while g_modeFlags16 bit 5 is set.
void hudWindowSlideTick(void)
{
    if (!(g_modeFlags16 & 0x20))
        return;
    if (g_frameCounter & 1)
        return;
    if (g_modeFlags16 & 0x10) {
        g_windowYTarget--;
        rWY = g_windowYTarget;
        g_d0dbCounter++;
    } else {
        if (g_playerShipsRemaining < 8)
            return;
        g_windowYTarget++;
        rWY = g_windowYTarget;
        g_d0dbCounter--;
    }
    if (!--g_windowAnimTimer)
        g_modeFlags16 &= ~0x20;
}

// Entry from bank 0. Clears the animation period flag and the frame timer.
void gameBResetAnimPeriod(void)
{
    g_animFlags = 0;
    g_animFrameTimer = 0;
}
