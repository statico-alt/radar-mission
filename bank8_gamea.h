// Shared by the Game A C files in bank 8 (bank8_gamea*.c) and the
// overflow in bank 10 (bank10_gamea*.c). This code was GameA_Frame and
// the Game A states in bank_000.asm. That asm ran in ROM0 with bank 1
// mapped. Here bank 8 is mapped, so bank 1 code and data are reached
// through __banked calls or the FarCall shims in bank8_gamea_far.c.
#include "bank0.h"

// The shadow OAM at $c000.
#define OAM (&g_oamBuffer)
// A byte array at a RAM name.
#define RAM(v) (&(v))
// rst $20: the cell at row r, column c of a grid.
#define GRID_CELL(grid, r, c) ((grid) + multiply8((r), g_gridSize) + (uint8_t)(c))
// Game A level 3 (g_gameALevel bits 6-7 = 2) uses other sprites.
#define GA_LEVEL3 ((g_gameALevel & 0xc0) == 0x80)

// Bank 0 asm (bank_000.asm). These keep the mapped bank.
void ClearShadowOAM(void);
void LcdOffMaskIrqs(void);
void LcdOnEnableIrqs(void);
void QueueTilemapRowCopy(uint16_t base);
uint8_t SumFleetBytes(void);
// HL = src in ROM bank A, DE = dst, BC = count. Maps bank 8 again.
void gaFarCopyRegs(void);
// The ship tile loop of the state $1e load (bank_000.asm).
void gaLoadShipTiles(void);
void gaPadCycles(uint16_t n);
// farCopy with constant arguments and less overhead (the loads run with
// the LCD off, and their time sets the timer IRQ phase). Nothing may be
// live in registers across it.
#define GA_COPY(bank, dst, src, n) __asm__("ld a, #" #bank "\nld hl, #_" #src \
    "\nld de, #" #dst "\nld bc, #" #n "\ncall _gaFarCopyRegs")

// Bank 1 data (only the address is used, or it is read through farByte).
extern const uint8_t GameA_PlacementSpriteLayout[], GameA_ShipCellIdRange[];
extern const uint8_t GameA_VictorySpriteLayout[], GameA_ShotAnimSpriteStrip[];
extern const uint8_t GameA_AircraftDirOffsets[], GameA_ArrangeFleetWindowTilemap[];
extern const uint8_t GameA_TurnCounterRuns[], GameA_EnemyCountRuns[], GameA_NoEnemyTextRuns[];
extern const uint8_t GameA_TargetPatternOffsets[], GameA_CutsceneAircraftLaunching[];
extern const uint8_t GameA_FleetEnemyHeaderRuns[], GameA_SunkCrossTileMask[];
extern const uint8_t GameA_NextLevelShipTimingInit[], GameA_ShipTileCopyEntries[];
extern const uint8_t GameA_TallyAnimC[];
// Bank 2, 4 and 5 graphics.
extern const uint8_t GameA_GfxAirStrikeBgTiles[], GameA_GfxAirStrikeBgTiles2[];
extern const uint8_t GameA_GfxReadyBanner[], GameA_GfxAirStrikeSprites[];
extern const uint8_t GameA_GfxBgTiles[], GameA_GfxAltModeTiles[], GameA_GfxPlayerEnemyTiles[];
extern const uint8_t GameA_GfxShipSinkSprites[], GameA_GfxStatusTiles[], GameA_GfxTiles6050[];
extern const uint8_t GameA_GfxTiles9F[], GameA_VictorySpritesLevel3Gfx[];
extern const uint8_t NextLevelBgTiles[];
extern const uint8_t CommanderTiles0[], CommanderTiles1[], CommanderTiles2[];
extern const uint8_t CommanderTiles3[], CommanderTiles4[];

// Bank 1 and bank 5 C with no arguments. The functions are not
// __banked, but ___sdcc_bcall_ehl calls them the same way and keeps
// the A or BC result.
void updatePlayClock(void) __banked;
void updateAnimScripts(void) __banked;
void runCutsceneScript(void) __banked;
void selectNextShip(void) __banked;
uint8_t moveGridCursor(void) __banked;
uint8_t checkAllShipsPlaced(void) __banked;
void placeState0WaitButton(void) __banked;
void placeState2TurnShip(void) __banked;
void placeOnBPickUpShip(void) __banked;
void showTargetCursor(void) __banked;
void drawCoordLabels(void) __banked;
uint16_t getCursorCell(void) __banked;   // B = row, C = column
void updateScroll(void) __banked;
void nudgeCellSprites(void) __banked;
void state1AStep4(void) __banked;
void state1FWaitFade(void) __banked;
void drawArrangeScreen(void) __banked;
void tickRedrawTimers(void) __banked;
uint8_t *getSalvoShotPtr(void) __banked;
void state1AStep0(void) __banked;
void clearAnimScripts(void) __banked;
uint8_t *animGetNextRecordPtr(void) __banked;
uint8_t *animOamPtr(void) __banked;
void state1AStep3(void) __banked;
void resultsStep1(void) __banked;
void resultsStep2Reset(void) __banked;
void blinkEnemyCount(void) __banked;
void state08(void) __banked;
void turn08AltHandler(void) __banked;
void drawMenuTilemap(void) __banked;
void state14(void) __banked;
void state15Step1(void) __banked;
void drawVictoryScene(void) __banked;
void state1AStep1(void) __banked;
void state19(void) __banked;
void boardStep1(void) __banked;
void state11(void) __banked;
uint8_t rotateShipTileStep(void) __banked;  // 1 = the last tile was copied
void blinkShipSprites(void) __banked;
void optionsMenuUpdate(void) __banked;
void resultsScreenInit(void) __banked;
void showShipSprite(void) __banked;
void setSeaMapPtr(void) __banked;
void nextLevelScreen(void) __banked;
void textRevealScreenInit(void) __banked;
void textRevealScreenUpdate(void) __banked;
void setupBattle(void) __banked;
void drawShipCellMarkers(void) __banked;
void cyclePaletteOBP1(void) __banked;
void levelTitleScreenInit(void) __banked;
void randomAmbientSfx(void) __banked;
void drawGridRow(void) __banked;
// Bank 13 C (bank 5 calls it with ___sdcc_bcall_ehl too).
void cpuTurnType0(void) __banked;
void cpuTurnType1(void) __banked;
void cpuTurnType2(void) __banked;
void aiPickNextState(void) __banked;
void aiMarkSunkLine(void) __banked;

// Bank 9 C (__banked).
void updatePaletteFade(void) __banked;
void calcPlacementBounds(uint8_t dir) __banked;
uint8_t checkCursorInBounds(void) __banked;
void drawShipSprites(void) __banked;
void setShipTileSource(void) __banked;
void placeGetShipSprite(void) __banked;
void writeShipToGrid(uint8_t *grid) __banked;
uint8_t checkShipFits(uint8_t *grid, uint8_t dir) __banked;
void resolveShots(void) __banked;
void turn06FireAtTarget(void) __banked;
void collectTouchingShips(uint8_t *grid, uint8_t row, uint8_t col) __banked;
uint8_t checkLuckySquare(uint8_t player, uint8_t cell) __banked;
void loadPlayerFleet(uint8_t player) __banked;
uint8_t countNonZero5(const uint8_t *p) __banked;
uint16_t stepCellInDirectionAsm(uint8_t dir, uint16_t cell) __banked;
void setMetaspritePos2x2(uint8_t *oam, uint8_t x, uint8_t y, uint8_t size) __banked;
uint16_t findGridCellByValue(const uint8_t *grid, uint8_t a, uint8_t b) __banked;
uint8_t getGridMargin(void) __banked;
void initBoardScreen(void) __banked;
void state1AStep5(void) __banked;
void state15Step2(void) __banked;
void boardStep2(void) __banked;
void state20FadeOut(void) __banked;
uint8_t getShipFromCellId(uint8_t id) __banked;
void turn01PlayerAim(void) __banked;
void state16Step0(void) __banked;
void state16Step1(void) __banked;
void turn08ResetScroll(void) __banked;
void resetStatEffectA(void) __banked;
void resetStatEffectB(void) __banked;
void turn06ClearOAM(void) __banked;
void state1AStep2(void) __banked;
void levelTitleStep1(void) __banked;
void state02Init(void) __banked;
void cpu10SetDelay(void) __banked;
void storeCpuShipDirs(void) __banked;
void state10(void) __banked;
void cpu0B(void) __banked;
void blinkLuckyMarkers(void) __banked;
void state1DSlideWindow(void) __banked;
void state04Delay(void) __banked;
void turn02(void) __banked;
void turn03(void) __banked;
void turn04ClearOAM(void) __banked;
void cpu12SetDelay(void) __banked;

// bank8_gamea_far.c: FarCall shims for bank 1 and bank 5 C with
// arguments, and for bank 0 asm that reads bank 1 data or maps bank 1.
uint8_t gaDrawTargetPattern(uint8_t row, uint8_t col);  // 0 = a cell is not shot yet
uint8_t gaCheckCoordOutsideGrid(uint8_t v);
uint16_t gaGridCellToOamXY(uint8_t row, uint8_t col);   // B = Y, C = X
void gaQueueShipCellTiles(uint8_t id, uint16_t cell);
void gaDrawStreamToBgMap(const uint8_t *grid);
void gaWriteTileRuns(uint8_t *dst, const uint8_t *src);
uint16_t gaSplitCellByte(uint8_t v);                    // B = row, C = column
void gaLoadShipRecord(uint8_t n, const uint8_t *table);
void gaDrawShipRecords(const uint8_t *table);
uint8_t gaScrollStepsTo(uint8_t cell, uint8_t margin);
uint16_t gaGridCellToScreen(uint8_t row, uint8_t col);  // B = Y, C = X (bank 5)
void gaRunCopyQueue(void);                               // VBlank_RunCopyQueue
void gaSpriteStrips(const uint8_t *list, uint16_t attrCount, uint8_t *oam);  // list in bank 1
void gaDrawTilemapScript(const uint8_t *script);        // rst $28, script in bank 1

// bank8_gamea_frame.c
uint8_t *gaQueueVramCopy(const uint8_t *src, uint16_t dst);
void gameAFrame(void);
// bank8_gamea_turn.c, bank8_gamea_cpu.c
void gameATurnState(void);
void gameACpuState(void);
// bank8_gamea_states.c
void gameAState13BattleEnd(void);
void gameAStates0E(void);
void gaBoardLoad1E(void);
void gameALoadCursorTiles(void);
void gameADrawFleetPanel(void);
