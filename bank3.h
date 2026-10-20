// Functions shared by the bank 3 and bank 11 C files.

// bank11_queues.c (bank 11 callers only)
void soundQueuePush(uint8_t type, uint8_t id);
void hudQueuePush(uint8_t cmd, uint8_t param);
void linkTxQueuePush(uint8_t cmd, uint8_t param);
void linkPopEnemyPos(void);
void linkPushEnemyPos(uint8_t hi, uint8_t lo);
#define playSfx(id)     soundQueuePush(1, id)
#define playDeb8(id)    soundQueuePush(2, id)
#define playMusic(id)   soundQueuePush(3, id)
#define playDeb0(id)    soundQueuePush(4, id)
#define playDec8(id)    soundQueuePush(5, id)

// bank11_objects.c (bank 11 callers only)
uint8_t decAndCheckTimer(uint8_t *t);
uint16_t moveLeft(uint16_t x, uint8_t speed);
uint16_t moveRight(uint16_t x, uint8_t speed);

// bank3_far.c: bank 11 reads bank 3 tables through this.
uint8_t bank3Byte(const uint8_t *p) __banked;
extern const uint8_t GameB_UnknownCurve6e10[], GameB_TorpedoRangeDelays[];

// bank11_damage.c (bank 11 callers only)
uint8_t selectEnemyShip(uint8_t a);
void playerSubHit(uint8_t side);
void hitShipCheckSub(uint8_t *hp, uint8_t side);
void setHitFlash(uint8_t *hp);
void shipDestroyed(uint8_t side);
void launchObject(uint8_t dir, uint8_t side);

// bank3_range.c (bank 3 callers only)
uint8_t divide16(uint16_t n, uint8_t d);
uint8_t rangeCheck(uint16_t x, uint16_t w, uint16_t *dist, uint16_t ref, uint8_t m);
uint8_t relativeX(uint16_t x, const uint8_t *ref);

// The rank screen (bank3_rank.c, bank3_rankbuild.c): asm routines and bank 3 data.
void ClearShadowOAM(void);
void DrawTilemapScript(const uint8_t *script);  // bank 0 rst $28, DE = script
void GameB_IntroInit(void);
void GameB_IntroUpdate(void);
uint8_t *drawRepeatedPattern(uint8_t *dst, const uint8_t *row, uint8_t groups);  // bank3_hud.c

extern const uint8_t Rank_FrameStrings[], Rank_RowDotTiles[], GameB_RankStrings[];
extern const uint8_t GameB_RankText_NoRank[], GameB_RankStringOffsets[], GameB_RankMedalLayout[];
extern const uint8_t Rank_FramePattern[], Rank_LycTable[], Rank_ScyTable[], Rank_ScxTable[];
extern const uint8_t Rank_StarCountInit[], Rank_StarRecordsInit[], Rank_LabelStrings[];
extern const uint8_t Rank_SpriteStrips[], Rank_RevealOrder[], Rank_CongratulationsTiles[];
extern const uint8_t Rank_StarStripTable[], Rank_StarStripTimes[];
extern const uint8_t Rank_State4SpritesA[], Rank_State4SpritesB[], Rank_EndSprites[];
extern const uint8_t TitleCutscene_LycTable[], TitleCutscene_ScyTable[], TitleCutscene_StarInit[];
extern const uint8_t TitleCutscene_ObjMotion[], TitleCutscene_PatternA[], TitleCutscene_PatternB[];
extern const uint8_t TitleCutscene_PatternC[], TitleCutscene_SpritesA[], TitleCutscene_SpritesB[];
extern const uint8_t TitleCutscene_SpritesC[], GameB_RankThresholds[], LevelSelect_PushStartSprites[];
