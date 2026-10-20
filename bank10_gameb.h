// Game B states and cutscenes in bank 10 (bank10_gameb*.c, bank10_gbfar.c).

// bank10_gbfar.c: shims to asm in banks 1 and 3 (named after the asm label).
void gbClearDamageGauge(void);
void gbClearDamageGaugeB(void);         // GameB_ClearDamageGauge
void gbMarkLinkEnd(void);
void gbDrawSideHeader(void);
void gbPickLevelValue(void);
void gbReadPadAndClock(void);
void gbDrawStatusPattern(void);
void gbDrawStatusScreen(void);
void gbHudDrawWeaponIcons(void);
void gbHudDrawCounters(void);
void gbClearShadowSprites(void);
void gbResetSceneFlags(void);
void gbDrawPlayerSlots(void);
void gbDrawPlayerSlotsD1c0(void);
void gbDrawEnemySlots(void);
void gbDrawEnemySlotsD258(void);
void gbResetPlayerShip(void);
void gbEngine(void);                    // GameB_State14_PerFrameEngine
void gbSubDispatchD009(void);
void gbSendLinkStatus(void);
void gbKillConfirm(void);
void gbSub5b11(void);
void gbResetEnemySub(void);
void gbSetLevelClearTimer(void);
void gbMarkEnemyShip0(void);
void gbPlayNextTableSound(void);
void gbEndScene(void);                  // ScrollScript_EndScene
void gbBlockMovement(void);
void gbResetAnimPeriod(void);
void gbTallyKillFlags(void);
void gbUpdateDistanceCounters(void);
void gbStartDamageGauge(void);
void gbHudToggleWindow(void);
void gbClearMgTrails(void);
void gbClearFireFlags(void);
void gbLoadLevel(void);
void gbMaybeAddItem(void);
void gbDeathTallies(void);              // Bank6_DrawDeathTallies
void gbSixAnimSprites(void);            // Bank6_DrawSixAnimSprites
void gbDrawRankScreen(void);
void gbMoveStars(void);                 // RankScreen_MoveStars
void gbScrollSpritesSkip4(void);        // RankScreen_ScrollSprites.skip4
void gbCutsceneTimerStep(void);
void gbRankScreenFrame(void);
void gbTogglePause(void);
void gbSoftReset(void);
void gbPlaySFX(uint8_t id);
void gbPlayDeb8(uint8_t id);
void gbPlayMusic(uint8_t id);
void gbPlayDeb0(uint8_t id);
void gbSetPalettes(uint8_t i);
void gbSetPalettesD31f(uint8_t i);      // SetPalettes.d31fEntry
void gbClearSpritesFrom(uint8_t i);
void gbTilemap3(const uint8_t *script); // DrawTilemapScript with bank 3 mapped
uint8_t gbSub52ab(void);
uint8_t gbSideAdjustFlipped(uint8_t c);
uint8_t gbAdvanceScrollScript(void);    // 1 = the script is done
uint8_t gbAnimTick(void);               // 1 = carry
uint8_t gbLevelSelectInput(void);       // 1 = confirmed
void gbHudPush(uint8_t cmd, uint8_t param);
void gbLinkTxPush(uint8_t cmd, uint8_t param);
void gbAnimateWaves(uint8_t c);
void gbHudCmd14(uint8_t b);
void gbSub4561(uint8_t e);
void gbDrawTableString(const uint8_t *table, uint8_t i);
void gbMemClear(uint8_t *p, uint16_t n);
void gbFillBgFa(void);                  // $9800-$9a3f = tile $fa
void gbClearSpriteEntries(uint8_t *p, uint8_t n);
uint8_t *gbMapSelectedShip(void);
uint8_t gbLookup6e78(uint8_t a);
uint8_t *gbSaveToWreckSlot(uint8_t *ship);
void gbDrawObjMeta2(uint16_t pos, uint16_t frameAttr);
void gbDepthCharges(uint8_t d);
void gbMeta00(uint16_t pos, uint8_t i); // MetaspritePtrs
void gbMeta1a(uint16_t pos, uint8_t i); // MetaspritePtrs + $1a
void gbMeta3a(uint16_t pos, uint8_t i); // MetaspritePtrs + $3a
void gbMetaObj6(uint16_t pos, uint8_t i); // GameB_ObjMetaspritePtrs + 6
void gbRle(uint16_t *p);                // p = {map address, source}
void gbRleRows(uint16_t *p);
void gbRleRowsThenRle(uint16_t *p);
void gbRleTwoParts(uint16_t *p);
void gbB1Tally00(void);                 // Bank1_TallyVectors + n
void gbB1Tally03(void);
void gbB1Tally20(void);
void gbB1Tally23(void);
void gbB1Tally26(void);
void gbB1Tally29(void);
void gbB1TallyState3(void);
void gbB1TallyState4(void);
void gbB1TallyState5(void);
void gbB1TallyInit(void);
void gbB1CutState1(void);
void gbB1CutState2(void);
void gbB1CutState3(void);
void gbB1ScrollTiles(void);
void gbB1AnimateSprites(void);

// bank10_gbcut.c
void gbTitleCutscene(void);
void gbTallyCutscene(void);
void gbRankCutscene(void);
void gbState04(void);
void gbState26(void);

// Bank 0 asm.
void GameB_ResetScroll(void);
void HideWindow(void);
void GameB_SetRasterFlags(uint8_t flags);
void GameB_LcdOff(void);
void GameB_EnableIrqs(void);
void LcdOffMaskIrqs(void);
void LcdOnEnableIrqs(void);
void ClearShadowOAM(void);
void gameBEndingLoad(void) __banked;    // bank10_ending.c
void Vec6_GameB_InitShipArrays(void) __banked;
void Vec6_GameB_UpdateWeaponFlags(void) __banked;
void Vec6_GameB_InitScaledTable(void) __banked;

// Data.
extern const uint8_t GameB_GfxSprites[], InitD091[];                    // bank 6
extern const uint8_t GameB_MenuTiles[], GameB_SceneAltTiles[], GameB_OutfitTiles[];
extern const uint8_t GameB_SceneTiles[], GameB_DigitTiles[], GameB_DigitTilesFrom1[];
extern const uint8_t GameB_CutsceneTiles[], GameB_TallyTiles[], GameB_ExtraTiles[]; // bank 7
extern const uint8_t GameB_CutsceneGfxTiles[];                          // bank 8
extern const uint8_t GameB_SceneTilemap[], GameB_MessagePtrs[], GameB_SixAnimCounterDefaults[];
extern const uint8_t KillMarkerColumnOffsets[], HudPanelColumnOffsets[], GameB_DamageStepTable[];
extern const uint8_t GameB_SpriteHitWidths[], GameB_RasterTables[], GameB_StatusPatternTiles[];
extern const uint8_t GameB_WaveStepSizes[], GameB_LevelEndAnimPtrs[], GameB_RankStrings[]; // bank 3
extern const uint8_t GameB_LevelSelectTilemap[], GameB_LevelSelectScoreTilemap[];
extern const uint8_t GameB_LevelSelectSprites[], GameB_LevelSelectSpritesA[], GameB_LevelSelectSpritesB[];
extern const uint8_t GameB_LevelSelectSpritesC[], GameB_LevelSelectSpritesD[], GameB_LevelNames[]; // bank 0
