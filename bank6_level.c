// Game B level setup: layout, object slots, difficulty and weapon tables.
#include "game.h"

extern const uint8_t *const LevelLayoutPtrs[];
extern const uint8_t *const DifficultyTablePtrs[];
extern const uint8_t GameB_ScaledTableSource[36];

// Selects the layout for the next Game B scene and sets the world width.
// Normal: entry stage * 3 + variant. When g_sceneFlagsC bit 3 is set:
// entry 9 + min(deaths, 5) / 2.
// Returns the rest of the layout (a list that ends with $ff).
const uint8_t *loadLevelLayout(void)
{
    uint8_t entry;
    if (g_sceneFlagsC & 0x08) {
        uint8_t deaths = g_deathTallyA + g_deathTallyB;
        if (deaths > 5)
            deaths = 5;
        entry = 9 + deaths / 2;
    } else {
        entry = g_gameBStage * 3 + g_gameBLayoutVariant;
    }
    const uint8_t *layout = LevelLayoutPtrs[entry];
    uint8_t units = *layout++;
    g_layoutWidthUnits = units;
    uint16_t width = multiply8(units, 0xe0);
    ScrollScript_LastTileByte = width >> 12;
    WORD(g_worldWidth) = width;
    (&WORD(g_worldWidth))[1] = (uint16_t)units << 1;
    return layout;
}

// Byte (g_difficultyColumn & $0f) of difficulty table `table`.
static uint8_t getDifficultyTableByte(uint8_t table)
{
    return DifficultyTablePtrs[table][g_difficultyColumn & 0x0f];
}

// Fills bytes +2..+5 of an 8-byte object slot. Copies *src to g_slotInitByte.
void initObjectSlot(uint8_t *slot, uint8_t index, const uint8_t *src)
{
    slot[2] = getDifficultyTableByte(g_gameBSkill);
    slot[3] = 0;
    slot[4] = 0;
    slot[5] = index;
    g_slotInitByte = *src;
}

// Copies GameB_ScaledTableSource to g_scaledTable. Each byte is shifted
// left by (2 - skill), so a lower skill gives larger values.
void initScaledTable(void)
{
    uint8_t shift = 2 - g_gameBSkill;
    for (uint8_t i = 0; i < 36; i++)
        g_scaledTable[i] = GameB_ScaledTableSource[i] << shift;
}

// Number of bytes after list[0] in the same group (bits 4-6) as list[0]. Stops at $ff.
uint8_t countGroupRun(const uint8_t *list)
{
    uint8_t group = *list++ & 0x70;
    uint8_t n = 0;
    while ((*list & 0x70) == group && *list != 0xff) {
        n++;
        list++;
    }
    return n;
}

// Random value from 0 to range - 1 (range is a power of 2). Updates g_randomPos.
uint8_t randomMasked(const uint8_t *seed, uint8_t range)
{
    uint8_t ly = rLY;
    g_randomPos = ly + g_frameCounter + *seed;
    return g_randomPos & (range - 1);
}

// Three rows of 8 bytes, read by updateWeaponFlags.
static const uint8_t weaponBonusTable[3][8] = {
    {0x00, 0x20, 0x40, 0x20, 0x80, 0x20, 0x40, 0x20},
    {0x00, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0x60},
    {0x00, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0},
};

// Sets the top 3 bits of g_killFlagsPendingB from g_weaponFlagsBase.
// When skill and stage are not zero, it acts only when g_weaponBonusRow is
// not zero: it clears the row and ORs in a byte from weaponBonusTable.
void updateWeaponFlags(void)
{
    uint8_t flags = SWAP(g_weaponFlagsBase) << 1;
    if (g_gameBSkill && g_gameBStage) {
        uint8_t row = g_weaponBonusRow;
        if (!row)
            return;
        g_weaponBonusRow = 0;
        uint8_t kept = g_killFlagsPendingB & 0xe0;
        flags = kept | weaponBonusTable[row - 1][SWAP(flags ^ kept) >> 1];
    }
    g_killFlagsPendingB = flags;
}
