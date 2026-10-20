// Shared by the bank 1 and bank 9 C files (Game A and the Game B tally).
#include "game.h"

// Store to an HRAM byte. SDCC 4.5 makes a bad "ldh" (a garbage address)
// for some HRAM stores. The volatile store avoids it. Use this only
// where the build fails.
#define SET_HRAM(v, x) (*(volatile uint8_t *)&(v) = (x))

// The Game B tilemap script at $d878: VRAM address high, low, length,
// tiles. DRAW_TILE_SCRIPT marks it as ready ($d015 bit 6).
#define TS (&g_tileScript)
#define DRAW_TILE_SCRIPT() (g_modeFlags15 |= 0x40)

// The shadow OAM at $c000.
#define OAM (&g_oamBuffer)

// rst $20: the cell at row r, column c of a grid. A row has g_gridSize cells.
#define GRID_CELL(grid, r, c) ((grid) + multiply8((r), g_gridSize) + (uint8_t)(c))

// The ship lengths of the current player ($c0a5 for P1, $c76c for P2).
#define SHIP_LENGTHS (g_currentPlayer ? g_p2ShipLengths : g_p1ShipLengths)

// Cells in the shot pattern: 1, or 5 or 9 with the salvo option.
#define TARGET_CELL_COUNT (!(g_gameFlags4 & 0x01) ? 1 : (g_p1Options & 0x03) == 2 ? 9 : 5)

// OAM Y and X of a grid cell row or column.
#define CELL_OAM_Y(row) ((uint8_t)(((row) << 3) + ((12 - g_gridSize) << 2) + 0x20))
#define CELL_OAM_X(col) ((uint8_t)(((col) << 3) + ((12 - g_gridSize) << 2) + 0x18))

// Screen pixel Y or X of a grid cell coordinate (16 pixels per cell).
#define CELL_SCREEN(v) ((uint8_t)(((v) << 4) + 0x18 - ((g_gridSize - 8) << 3)))

// Bank 1 data (bank_001.asm).
extern const uint8_t OamDmaRoutine[];
extern const int8_t CursorMoveDeltas[];
extern const int8_t GameA_TargetPatternOffsets[];
extern const uint8_t GameA_ShipNameplateTable[];
extern const uint8_t GameA_ShipSpriteTable[];
extern const uint8_t GameA_CoordLabelLayout[];
extern const uint8_t GameA_FleetIconTiles[];
extern const uint8_t GameA_DpadToDirection[];
extern const uint8_t GameA_ShipCellIdRange[];
extern const uint8_t GameA_LevelSpritesLevel1[];
extern const uint8_t GameA_LevelSpritesLevel2[];
extern const uint8_t GameA_LevelSpritesLevel3[];
extern const uint8_t DirectionDeltas[];
extern const uint8_t GameA_ScrollYRowTriggers[];
extern const uint8_t GameA_ScrollXColTriggers[];
extern const uint8_t GameA_ScrollRowEdgeTiles[];
extern const uint8_t GameA_ScrollColEdgeTiles[];
extern const uint8_t GameA_ShipPartTiles[];
extern const uint8_t GameA_RedrawTimerPresets[];
extern const uint8_t GameA_EnemyAircraftSpriteLayout[];
extern const uint8_t GameA_StartText[];
extern const uint8_t GameA_ArrangeTextRuns[];
extern const uint8_t GameA_VsComTextRuns[];
extern const uint8_t GameA_VersusTextRuns[];
extern const uint8_t GameA_ArrangeFleetWindowTilemap[];
extern const uint8_t GameA_BoardRowRuns[];
extern const uint8_t GameA_BoardSideBorderRuns[];
extern const uint8_t GameA_ShotMarkerTiles[];
extern const uint8_t GameA_CutsceneEnemyAircraftLaunched[];
extern const uint8_t GameA_ShipBurningLoopAnim[];
extern const uint8_t GameA_ShipTileCopyEntries[];
extern const uint8_t GameA_AircraftDirOffsets[];
extern const uint8_t GameA_AircraftDirTiles[];
extern const uint8_t GameA_AircraftDescentDelays[];
extern const uint8_t GameA_EnemyFleetGridPtrs[];
extern const uint8_t GameA_TallyAnimSetup[];
extern const uint8_t GameA_WaitingText[];
extern const uint8_t GameA_CpuCursorTiles[];
extern const uint8_t GameA_AnimSlotFrameTiles[];
extern const uint8_t GameA_ResultsBlinkTiles[];
extern const uint8_t GameA_CongratsBannerSprite[];
extern const uint8_t GameA_CongratulationsText[];
extern const uint8_t GameA_EnemyCountText[];
extern const uint8_t GameA_NoEnemyText[];
extern const uint8_t GameA_BoardIconRuns[];
extern const uint8_t GameA_TurnCounterRuns[];
extern const uint8_t GameA_EnemyCountRuns[];
extern const uint8_t GameA_NoEnemyTextRuns[];
extern const uint8_t GameA_OptionsMenuTilemap[];
extern const uint8_t GameA_VictorySceneTilemap[];
extern const uint8_t GameA_VictorySceneAnim[];
extern const uint8_t GameA_VictorySceneAnimLevel3[];
extern const uint8_t GameA_VictoryBgpFadeSteps[];
extern const uint8_t GameA_VictorySpriteLayout[];
extern const uint8_t GameA_FleetEnemyHeaderRuns[];
extern const uint8_t GameA_SunkCrossTileMask[];
extern const uint8_t GameA_YouLostSpriteLayout[];
extern const uint8_t GameA_LevelMapRuns[];
extern const uint8_t GameA_LevelMapStream[];
extern const uint8_t GameB_TallyKillCountPtrTable[];
extern const uint8_t GameB_TallyRowSprites[];
extern const uint8_t GameB_TallyRowLabels[];
extern const uint8_t GameB_TallyRowPayloads[];
extern const uint8_t GameB_TallyRowNameTiles[];
extern const uint8_t GameB_TallyBonusTiles[];
extern const uint8_t GameB_TallyHeader[];
extern const uint8_t GameB_TimeBonusBanner[];
extern const uint8_t GameB_TimeBonusRankFloor[];
extern const uint8_t GameB_CutsceneSceneSteps[];
extern const uint8_t GameB_CutsceneAttrFlips[];
extern const uint8_t GameB_CutsceneSpriteScript[];
extern const uint8_t GameB_CutsceneFlashPtrs[];
extern const uint8_t GameB_BlinkSpriteXA[];
extern const uint8_t GameB_BlinkSpriteXB[];
// Bank 2 graphics (only the address is used).
extern const uint8_t GameA_GfxUiTiles[];
extern const uint8_t GameA_GfxTiles6050[];

// Bank 0: A = 1 + the sum of the 5 ship lengths of the current player.
uint8_t SumFleetBytes(void);

// Bank 0 routines that C calls directly.
void ClearShadowOAM(void);
// These map bank 1 again, so only bank 1 code calls them.
void VBlank_RunCopyQueue(void);
void DrawTilemapCmds(const uint8_t *cmds);
// Queue a copy of g_vramQueue to row $ffb1 of the map at base.
void QueueTilemapRowCopy(uint16_t base);

// Bank 0 routines with C wrappers in bank_001.asm (bank 1 code only).

// bank1_anim.c
uint8_t *animGetNextRecordPtr(void);
void clearAnimScripts(void);
uint16_t peek1w(const uint8_t *p) __banked;
void copyFromBank1(uint8_t *dst, const uint8_t *src, uint8_t count) __banked;
void setMetaspriteTiles4(uint8_t *oam, const uint8_t *tiles) __banked;
uint8_t *animOamPtr(void);

// bank1_place.c
void drawTargetCursor(void);
void showTargetCursor(void);
void buildLevelSprites(void);
void drawCoordLabels(void);

// bank9_frame.c
uint8_t stirRandom(void) __banked;

// bank9_grid.c (the functions without __banked are for bank 9 callers)
uint8_t *gridStep(uint8_t *p, uint8_t dir);
uint16_t gridCellToBgAddr(uint8_t row, uint8_t col);
void adjustShipOrigin(uint8_t dir, uint8_t len, uint8_t *row, uint8_t *col);
void drawShipSprites(void) __banked;
void setShipTileSource(void) __banked;
void placeGetShipSprite(void) __banked;
void pickUpShipCells(uint8_t len, uint8_t first) __banked;
void calcPlacementBounds(uint8_t dir) __banked;
uint8_t checkCursorInBounds(void) __banked;
uint8_t checkShipFits(uint8_t *grid, uint8_t dir) __banked;
uint8_t getShipFromCellId(uint8_t id) __banked;
void writeShipToGrid(uint8_t *grid) __banked;

// bank9_shots.c
uint8_t countNonZero5(const uint8_t *p) __banked;
void collectTouchingShips(uint8_t *grid, uint8_t row, uint8_t col) __banked;
uint8_t checkLuckySquare(uint8_t player, uint8_t cell) __banked;
void processShipList(void) __banked;
void initLuckySquares(void) __banked;

// bank1_board.c
void getShipPartTile(uint8_t id, uint8_t *out);
void drawStreamToBgMap(const uint8_t *grid);
void writeTileRuns(uint8_t *dst, const uint8_t *src);
void tickRedrawTimers(void);
void buildGridEdgeStrip(void);

// bank9_board.c
void setMetaspritePos2x2(uint8_t *oam, uint8_t x, uint8_t y, uint8_t size) __banked;
uint16_t findGridCellByValue(const uint8_t *grid, uint8_t a, uint8_t b) __banked;
uint8_t getGridMargin(void) __banked;
uint8_t beginGameOver(void) __banked;
void queueVramCopy(const uint8_t *src, uint16_t dst, uint8_t len, uint8_t rows, uint8_t bank) __banked;

// bank9_tally.c
void bcdAdd(uint8_t *dst, const uint8_t *src, uint8_t n) __banked;
