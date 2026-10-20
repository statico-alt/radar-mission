// The LCD-off builds of the rank screen and the title cutscene, and the
// level select input. They read the bank 3 rank data, so they stay in bank 3.
// bank3_rank.c has the rank screen frame code. Two files fit the two free
// gaps of bank 3.
#include "game.h"
#include "bank3.h"

#define OAM (&g_oamBuffer)
#define RANK_TIMER_HI (&g_rankTimer)[1]
#define LINK_GAME (g_topModeFlags & 0x80)

// Waits n loop passes. The LCD-off load time sets the timer IRQ phase.
// The rank build waits for the time that the asm build took longer. The
// title build count comes from a sweep against the title cutscene (frame
// 1931) and the Game B race (frame 3241). 205 to 265 pass both. 0, 80, 120,
// 165 and 305 fail one.
static void padCycles(uint16_t n)
{
    do {
        __asm__("nop");
    } while (--n);
}

// Draws BG map row r ($9800 + r * 32) from the run list at g_mapStreamPtr.
static void drawStreamRow(uint8_t r)
{
    uint8_t *p = &g_tileScript;
    uint16_t a = 0x9800 + ((uint16_t)r << 5);
    p[0] = a >> 8;
    p[1] = a;
    p[2] = 0x20;
    fillBytes(p + 3, 2, 0x20);
    unpackRunList(p + 3);
    g_tileRowScriptEnd = 0;
    DrawTilemapScript(p);
}

// Entry from bank 0 (LCD off). Builds the rank screen.
void rankScreenBuild(void)
{
    uint8_t r;
    ClearShadowOAM();
    fillBytes((uint8_t *)0x9800, 2, 0x400);
    g_gameBLayoutVariant = 3;
    g_scratch92 = 0x0b;
    drawRepeatedPattern((uint8_t *)0x99c0, Rank_FramePattern, 4);
    // The run lists are in the rank data after the star records.
    WORD(g_mapStreamPtr) = (uint16_t)(Rank_StarRecordsInit + 0x0f);
    for (r = 0; r != 5; r++)
        drawStreamRow(r);
    if (!g_rankIndex) {
        GameB_IntroInit();
        rOBP0 = 0x78;
        rOBP1 = 0;
    } else {
        rOBP0 = 0x39;
        rOBP1 = 0xb3;
        if (g_rankIndex < 3) {
            WORD(g_mapStreamPtr) = (uint16_t)(Rank_StarRecordsInit + 0x29);
            for (r = 5; r != 0x0e; r++)
                drawStreamRow(r);
            padCycles(485);
        }
    }
    DrawTilemapScript(Rank_LabelStrings);
    buildSpriteStrips(Rank_SpriteStrips, 0x0017, OAM);
    OAM[0x53] = 0x20;
    copyBytes(&g_rasterLycTable, Rank_LycTable, 5);
    copyBytes(&g_rasterScy2, Rank_ScyTable, 5);
    copyBytes(&g_rasterScx2, Rank_ScxTable, 5);
    g_rasterCount = 5;
    g_d0dbCounter = 0x92;
    g_subStateScratch4 |= 0x02;
    copyBytes(&g_starCount, Rank_StarCountInit, 0x10);
    rBGP = 0x39;
    rSTAT |= 0x40;
    g_state26CutsceneSubState = 1;
    BGM_Queue = 0x10;
    g_tallyRowIndex = 0x30;
    g_levelBlinkCount = 0x60;
    if (g_rankIndex >= 3)
        g_rasterScx2 = 0x40;
}

// Entry from bank 0 (LCD off). Builds the Game B title cutscene screen.
void titleCutsceneBuild(void)
{
    uint8_t c;
    ClearShadowOAM();
    fillBytes((uint8_t *)0x9800, 2, 0x240);
    WORD(g_mapStreamPtr) = (uint16_t)(Rank_StarRecordsInit + 0xee);
    for (c = 0; c != 0x12; c++)
        drawStreamRow(c >= 8 && c < 0x0c ? c + 0x12 : c);
    g_gameBLayoutVariant = 4;
    g_scratch92 = 8;
    drawRepeatedPattern((uint8_t *)0x9840, TitleCutscene_PatternA, 1);
    g_scratch92 = 6;
    drawRepeatedPattern((uint8_t *)0x9867, TitleCutscene_PatternB, 1);
    g_gameBLayoutVariant = 3;
    g_scratch92 = 8;
    drawRepeatedPattern((uint8_t *)0x9980, TitleCutscene_PatternC, 1);
    copyBytes(&g_rasterLycTable, TitleCutscene_LycTable, 6);
    copyBytes(&g_rasterScy2, TitleCutscene_ScyTable, 6);
    fillBytes(&g_rasterScx2, 0, 6);
    g_d0dbCounter = 0x92;
    g_rasterCount = 6;
    copyBytes(&g_starCount, TitleCutscene_StarInit, 0x13);
    uint8_t *m = &g_cutsceneObjMotion;
    for (c = 0; c != 7; c++) {
        *m++ = 0;
        *m++ = TitleCutscene_ObjMotion[c];
    }
    buildSpriteStrips(TitleCutscene_SpritesA, 0x8008, OAM);
    buildSpriteStrips(TitleCutscene_SpritesB, 0x000c, OAM + 0x20);
    buildSpriteStrips(TitleCutscene_SpritesC, 0x1002, OAM + 0x50);
    g_subStateScratch4 |= 0x02;
    SoundCmd_Queue = 3;
    g_cutsceneFlags |= 0x01;
    g_tallyStepTimer = 0x20;
    g_rankTimer = 0;
    g_tallyRowIndex = 0;
    g_rankFlybyPassesLeft = 0;
    (&g_rankFlybyPassesLeft)[1] = 0;
    g_scrollNoiseCounter = 7;
    g_cutsceneSpriteFrac = 0;
    g_cutsceneSpriteSpeed = 0x60;
    g_cutsceneSpriteStep = 1;
    g_cutsceneFlipTimer = 0x10;
    g_cutsceneScriptDelay = 0x50;
    g_cutsceneFlashTimer = 0x50;
    g_cutsceneFlashPeriod = 0x0a;
    g_cutsceneFlashIndex = 0xff;
    rBGP = 0x39;
    rOBP1 = 0x36;
    g_cutsceneD828Step = 1;
    padCycles(225);
}

// Moves the 6 cursor sprites of the player by dy.
static void moveCursor(uint8_t dy)
{
    uint8_t *o = g_levelSelectPlayer ? OAM + 0x18 : OAM;
    for (uint8_t c = 6; c; c--) {
        *o += dy;
        o += 4;
    }
    SFX_Queue = 1;
}

// Joypad handler of the level select (bank 0 calls it). Player
// g_levelSelectPlayer uses its lock byte and pad. Down/Up change the level,
// A/Start confirm, B cancels. Returns 1 when the level is confirmed.
uint8_t gameBLevelSelectInput(void)
{
    uint8_t p2 = g_levelSelectPlayer;
    uint8_t *lock = g_levelSelectLock + (p2 ? 1 : 0);
    uint8_t pad = p2 ? (&g_linkPad)[1] : g_padState;
    uint8_t *skill = &g_gameBSkill;
    if (LINK_GAME && !p2)
        skill--;
    if (pad & 0xc0) {
        if (*lock)
            return 0;
        uint8_t a = *skill;
        if (pad & 0x80) {
            if (a == (LINK_GAME ? 2 : 3))
                return 0;
            *skill = a + 1;
            moveCursor(a == 2 ? 0x18 : 0x20);
        } else {
            if (!a)
                return 0;
            *skill = a - 1;
            moveCursor(a == 3 ? 0xe8 : 0xe0);
        }
        return 0;
    }
    if (pad & 0x08) {
        if (*lock != 1)
            return 0;
        if (!p2)
            fillBytes(OAM + 0x30, 0, 0x28);
        return 1;
    }
    if (pad & 0x02) {
        if (*lock == 1) {
            if (!p2) {
                fillBytes(OAM + 0x30, 0, 0x28);
                SFX_Queue = 8;
            }
        } else if (*lock == 2) {
            rLCDC = (rLCDC & ~0x08) | 0x02;
            rBGP = 0x39;
            SFX_Queue = 8;
        } else {
            return 0;
        }
        *lock = 0;
        return 0;
    }
    if (pad & 0x01) {
        if (*lock)
            return 0;
        if (!LINK_GAME) {
            if (g_gameBSkill < 3)
                return 1;
            *lock = 2;
            rLCDC = (rLCDC | 0x08) & ~0x02;
            rBGP = 0x36;
        } else {
            *lock = 1;
            if (p2)
                return 0;
            buildSpriteStrips(LevelSelect_PushStartSprites, 0x1009, OAM + 0x30);
        }
        SFX_Queue = 2;
    }
    return 0;
}
