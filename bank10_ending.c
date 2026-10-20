// Game B ending (state $26, step 0).
#include "bank0.h"

void LcdOffMaskIrqs(void);
void LcdOnEnableIrqs(void);
void GameB_SelectRank(void) __banked;   // bank 3
void RankScreen_Build(void) __banked;   // bank 3
extern const uint8_t GameB_DigitTiles[], GameB_EndingBgTiles[], GameB_EndingTilesB[];
extern const uint8_t GameB_EndingSpriteTiles[], GameB_AdmiralTiles[];
extern const uint8_t GameB_EndingGfxTilesA[], GameB_EndingGfxTilesB[], GameB_CutsceneGfxTiles[];

// Pick the rank. Only show the ending when the rank is below 5 and
// g_sceneFlagsC has bits 6 and 0 set. Else go to the next state.
// Rank 0 (admiral) gets extra art.
void gameBEndingLoad(void) __banked
{
    GameB_SelectRank();
    if (g_rankIndex >= 5 || (g_sceneFlagsC & 0x41) != 0x41) {
        g_subState++;
        return;
    }
    LcdOffMaskIrqs();
    farCopy(7, (uint8_t *)0x9000, GameB_DigitTiles, 0x40);
    farCopy(7, (uint8_t *)0x9040, GameB_EndingBgTiles, 0x450);
    farCopy(7, (uint8_t *)0x8dd0, GameB_EndingTilesB, 0x220);
    farCopy(7, (uint8_t *)0x8010, GameB_EndingSpriteTiles, 0x3c0);
    farCopy(6, (uint8_t *)0x83d0, GameB_EndingGfxTilesA, 0xc0);
    if (!g_rankIndex) {
        farCopy(8, (uint8_t *)0x9000, GameB_CutsceneGfxTiles + 0x1000, 0x800);
        farCopy(8, (uint8_t *)0x8800, GameB_CutsceneGfxTiles + 0x800, 0x330);
        farCopy(6, (uint8_t *)0x8490, GameB_EndingGfxTilesB, 0x1e0);
        farCopy(6, (uint8_t *)0x8b30, GameB_EndingGfxTilesA + 0xc0, 0x30);
        farCopy(7, (uint8_t *)0x8c00, GameB_AdmiralTiles, 0x1b0);
    }
    RankScreen_Build();
    LcdOnEnableIrqs();
}
