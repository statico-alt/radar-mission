// The main loop and the top state machine. SoftReset (bank_000.asm)
// sets up the stack, clears the RAM and jumps to softReset.
#include "bank0.h"

void titleUpdate(void) __banked;    // bank10_title.c

// Top state 0: wait for the fade ($ffaa bit 0), then set up serial and IE.
static void topStateBoot(void)
{
    UpdatePaletteFade();
    if (g_gameFlags2 & 0x01)
        return;
    g_topState = 1;
    rSB = 0xfe;
    rIF = 0;
    rSC = 0;
    rSC = 0x80;
    rIE = 0x0f;
}

// Top state 2: run the Game A or the Game B frame (g_topModeFlags bit 5).
static void topStateInGame(void)
{
    if (g_topModeFlags & 0x20) {
        GameB_Frame();
        SwitchROMBank(1);
    } else {
        GameA_Frame();
    }
}

// One main loop tick. compare.py counts the calls.
static void mainLoop(void)
{
    switch (g_topState) {
    case 0: topStateBoot(); break;
    case 1: titleUpdate(); break;
    case 2: topStateInGame(); break;
    // 3: link wait. The serial IRQ moves on.
    }
    // 2-player link master: wait a moment, then start the next transfer.
    if ((g_topModeFlags & 0x84) == 0x84) {
        __asm__("ld c, #0x60\n1$:\nnop\nnop\nnop\nnop\nnop\ndec c\njr nz, 1$");
        rSC = 0x81;
    }
}

// Wait for the VBlank IRQ to set g_topModeFlags bit 4. The first
// instruction must be the call: compare.py uses waitFrame+3 as the
// return address of the idle StirRandom call.
static void waitFrame(void)
{
    StirRandom();
    __asm__("halt");
    for (;;) {
        if (VOLATILE(g_gameFlags8) & 0x04)
            SoftReset();    // A+B+Select+Start
        if (VOLATILE(g_topModeFlags) & 0x10)
            break;
    }
    g_topModeFlags &= ~0x10;
}

// Load the title graphics, start the title screen and run the main loop.
void softReset(void)
{
    CopyOamDmaToHram();
    farCopy(2, (uint8_t *)0x9000, Title_GfxLogoTiles, 0x530);
    farCopy(2, (uint8_t *)0x8000, GameA_GfxSprites, 0x800);
    farCopy(2, (uint8_t *)0x8800, GameA_GfxUiTiles, 0x800);
    farCopy(4, (uint8_t *)0x9530, Title_ExtraTiles2Gfx, 0x60);
    farCopy(6, (uint8_t *)0x9590, Title_GfxExtraTiles, 0x170);
    farCopy(5, (uint8_t *)0x9700, Title_ExtraTilesGfx, 0xa0);
    Vec5_TitleScreenInit();
    for (;;) {
        mainLoop();
        waitFrame();
    }
}
