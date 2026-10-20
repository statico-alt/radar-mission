// Game B cutscenes and the level select screen (end of bank_000.asm).
#include "bank0.h"
#include "bank10_gameb.h"

#define AT(v, i) (((uint8_t *)&(v))[i])
#define VRAM(a) ((uint8_t *)(a))

static void tallyLoad(void)
{
    LcdOffMaskIrqs();
    farCopy(7, VRAM(0x9110), GameB_MenuTiles + 0x110, 0x10);
    farCopy(7, VRAM(0x8800), GameB_DigitTiles, 0xe0);
    farCopy(7, VRAM(0x89d0), GameB_CutsceneTiles + 0x30, 0x5c0);
    farCopy(7, VRAM(0x8e00), GameB_TallyTiles, 0x150);
    gbB1TallyInit();
    rBGP = 0x36;
    rOBP0 = 0x36;
    LcdOnEnableIrqs();
    g_sfxBlockSfx01 = 0;
}

// GameB_CutsceneDispatch_d824: the tally cutscene.
void gbTallyCutscene(void)
{
    switch (g_tallyCutsceneSubState) {
    case 0: tallyLoad(); break;
    case 1: gbB1Tally00(); break;
    case 2: gbB1Tally03(); break;     // RankScreen_State04
    case 3: gbB1TallyState3(); break;
    case 4: gbB1TallyState4(); break;
    case 5: gbB1TallyState5(); break;
    case 6: gbB1Tally20(); break;
    }
    gbB1Tally26();
}

// GameB_CutsceneDispatch_d825: the rank screen.
void gbRankCutscene(void)
{
    if (g_rankCutsceneStep) {
        gbCutsceneTimerStep();
        return;
    }
    LcdOffMaskIrqs();
    farCopy(7, VRAM(0x9000), GameB_DigitTiles, 0x40);
    farCopy(7, VRAM(0x8820), GameB_DigitTilesFrom1, 0xc0);
    farCopy(7, VRAM(0x89a0), GameB_CutsceneTiles, 0x600);
    gbDrawRankScreen();
    g_rankCutsceneStep = 1;
    rOBP0 = 0x86;
    rOBP1 = 0x36;
    rBGP = 0x36;
    LcdOnEnableIrqs();
    rLCDC = 0xcb;
}

static void titleLoad(void)
{
    LcdOffMaskIrqs();
    farCopy(7, VRAM(0x9000), GameB_DigitTiles, 0x40);
    farCopy(7, VRAM(0x8800), GameB_ExtraTiles, 0x140);
    if (g_topModeFlags & 0x04)
        farCopy(7, VRAM(0x8860), GameB_ExtraTiles + 0x110, 0x10);
    farCopy(7, VRAM(0x8000), GameB_ExtraTiles + 0x140, 0x2a0);
    farCopy(8, VRAM(0x9040), GameB_CutsceneGfxTiles, 0x620);
    gbScrollSpritesSkip4();
    LcdOnEnableIrqs();
}

// GameB_TitleCutscene: the $d828 cutscene.
void gbTitleCutscene(void)
{
    if ((g_cutsceneFlags & 0x01) && !g_soundFadeState) {
        g_cutsceneFlags &= ~0x01;
        NoiseSfx_Queue = 0x0e;
    }
    switch (g_cutsceneD828Step) {
    case 0: titleLoad(); break;
    case 1: gbB1CutState1(); break;
    case 2: gbB1CutState2(); break;
    case 3: gbB1CutState3(); break;
    case 4: gbB1Tally23(); break;
    }
    if (!g_cutsceneD828Step)
        return;
    gbMoveStars();
    gbB1ScrollTiles();
    gbB1AnimateSprites();
}

// ---- State $04: level select ----

static void levelSelectLoad(void)
{
    LcdOffMaskIrqs();
    farCopy(7, VRAM(0x9000), GameB_DigitTiles, 0x40);
    farCopy(7, VRAM(0x8840), GameB_DigitTilesFrom1 + 0x20, 0xa0);
    farCopy(7, VRAM(0x8b60), GameB_CutsceneTiles + 0x1c0, 0x3c0);
    farCopy(7, VRAM(0x83a0), GameB_MenuTiles + 0x3a0, 0x40);
    farCopy(7, VRAM(0x9110), GameB_MenuTiles + 0x110, 0x10);
    farCopy(7, VRAM(0x8760), GameB_ExtraTiles + 0x3e0, 0x40);
    farCopy(7, VRAM(0x85d0), GameB_SceneAltTiles + 0x5d0, 0x80);
    fillBytes(VRAM(0x9800), 2, 0x240);
    DrawTilemapScript(GameB_LevelSelectTilemap);
    gbDrawRankScreen();
    // Five rank strings (length, bytes) from bank 3 to the window map.
    const uint8_t *s = GameB_RankStrings;
    uint8_t *d = VRAM(0x9c62);
    for (uint8_t i = 5; i; i--) {
        uint8_t n = farByte(3, s++);
        farCopy(3, d, s, n);
        s += n;
        d += 0x40;
    }
    fillBytes(VRAM(0x9dc0), 2, 0x80);
    ClearShadowOAM();
    buildSpriteStrips(GameB_LevelSelectSprites, 0x2004, &g_oamBuffer);
    if (g_topModeFlags & 0x80) {
        buildSpriteStrips(GameB_LevelSelectSpritesA, 2, &g_oamBuffer);
        buildSpriteStrips(GameB_LevelSelectSpritesB, 2, &g_oamBuffer + 0x28);
        buildSpriteStrips((g_topModeFlags & 0x40) ? GameB_LevelSelectSpritesC : GameB_LevelSelectSpritesD,
                          8, &g_oamBuffer + 0x08);
        g_tallyBlinkTimer = 0x14;
        AT(g_tallyBlinkTimer, 1) = 8;
        g_rankTimer = 0;
    } else {
        DrawTilemapScript(GameB_LevelSelectScoreTilemap);
        g_cutsceneFlags |= 0x40;
    }
    AT(g_linkSwapPair, 0) = 1;
    AT(g_linkSwapPair, 1) = 1;
    g_levelSelectStep = 1;
    g_subStateScratch4 &= ~0x02;
    g_tallyStepTimer = 0x12;
    rSCY = 0;
    rSCX = 0;
    rBGP = 0x39;
    rOBP0 = 0x39;
    rOBP1 = 0xc9;
    LcdOnEnableIrqs();
}

static void levelSelectWait(void)
{
    uint8_t t = g_tallyStepTimer;
    if (t) {
        g_tallyStepTimer = t - 1;
        if (t == 1)
            BGM_Queue = 1;
        return;
    }
    g_levelSelectPlayer = 0;
    if (!(g_cutsceneFlags & 0x20) && gbLevelSelectInput()) {
        SFX_Queue = 5;
        g_cutsceneFlags |= 0x20;
        if (!(g_topModeFlags & 0x80)) {
            g_levelSelectStep = 2;
            g_rankTimer = 0x14;
            g_tallyRowIndex = 0;
            return;
        }
    }
    g_levelSelectPlayer++;
    if (!(g_cutsceneFlags & 0x40) && gbLevelSelectInput())
        g_cutsceneFlags |= 0x40;
    if (!(g_topModeFlags & 0x80))
        return;
    gbB1Tally29();
    if (g_tallyBlinkRoundsLeft)
        return;
    SFX_Queue = 0;
    g_subState++;
}

// Every 20 frames blink the chosen level name through g_tileScript.
static void levelSelectBlink(void)
{
    if (--g_rankTimer)
        return;
    g_rankTimer = 0x14;
    uint8_t n = ++g_levelBlinkCount;
    uint16_t dst = 0x98a6 + ((uint16_t)g_gameBSkill << 7);
    uint8_t *t = &g_tileScript;
    t[0] = dst >> 8;
    t[1] = dst;
    t[2] = 8;
    fillBytes(t + 3, 2, 8);
    if (!(n & 0x01))
        copyBytes(t + 3, GameB_LevelNames + (uint8_t)(g_gameBSkill << 3), 8);
    if (n == 8)
        g_subState++;
    g_tallyTimeMinutes = 0;
    g_modeFlags15 |= 0x40;
}

void gbState04(void)
{
    switch (g_levelSelectStep) {
    case 0: levelSelectLoad(); break;
    case 1: levelSelectWait(); break;
    case 2: levelSelectBlink(); break;
    }
}

// State $26: the ending.
void gbState26(void)
{
    if (g_state26CutsceneSubState)
        gbRankScreenFrame();
    else
        gameBEndingLoad();
}
