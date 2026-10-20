// The bodies of the VBlank, timer, Game A STAT and serial interrupts.
// The asm entry code in bank_000.asm saves the registers and does reti.
#include "bank0.h"

// VBlank: OAM DMA, copy queue, tilemap script queue, Game B VBlank work,
// then joypad read and the soft reset check.
void irqVBlank(void)
{
    uint8_t bank = g_currentBankShadow;
    if ((rSTAT & 0x03) < 2) {
        __asm__("call 0xff80");     // OAM DMA (OamDmaRoutine in HRAM)
        VBlank_RunCopyQueue();
        if (g_gameFlags3 & 0x04) {
            DrawTilemapScript(&g_vramQueue);
            g_gameFlags3 &= ~0x04;
        }
    }
    if (g_topState == 2 && (g_topModeFlags & 0x20)) {
        if (g_sceneFlagsA & 0x04)
            g_frameParity ^= 0x01;
        else
            g_frameParity &= ~0x01;
        if ((rSTAT & 0x03) == 1) {
            SwitchROMBank(3);
            Vec_GameB_VBlank();
        }
    }
    // Let only the serial IRQ in while the joypad is read.
    rIE &= 0x08;
    __asm__("ei");
    if (g_gameFlags8 & 0x08) {
        g_gameFlags8 &= ~0x08;
        rWY = g_gameAWindowY;
    }
    SwitchROMBank(1);
    readJoypad();
    SwitchROMBank(bank);
    if (!(g_topModeFlags & 0x40))
        g_topModeFlags |= 0x10;     // frame done (waitFrame)
    if (!g_joyRaw) {
        g_topModeFlags &= ~0x08;
    } else if (g_topModeFlags & 0x08) {
        // Buttons stay ignored until all are released after a reset.
        g_joyRaw = 0;
        g_joyPressed = 0;
    } else if ((g_gameALevel & 0x08) && g_joyRaw == 0x0f) {
        g_topModeFlags |= 0x08;
        g_gameFlags8 |= 0x04;       // soft reset (waitFrame)
    }
    rIE |= 0x07;
}

// Timer: run the sound driver (bank 4) when g_topModeFlags bit 1 is set.
// With $ffab bit 3 set, also queue the sound in g_titleStartSfx.
void irqTimer(void)
{
    if (!(g_topModeFlags & 0x02))
        return;
    SoundUpdate();
    if (g_gameFlags3 & 0x08) {
        SFX_Queue = g_titleStartSfx;
        g_gameFlags3 &= ~0x08;
    }
}

// Game A STAT effect 0: move LYC by 4 lines each step and swap the BG
// tile map ($ffaa bit 3 picks the layout, bit 4 the direction).
static void statEffect0(void)
{
    uint8_t *p = &g_statEffect2Arg;
    int8_t step = 4;
    uint8_t f = g_gameFlags2;
    if (f & 0x08) {
        p--;
        step = -4;
    }
    if (f & 0x10)
        step = -step;
    VOLATILE(rLYC) = *p += step;
    if (--g_statEffectTimer) {
        g_gameFlags2 ^= 0x08;
        rLCDC ^= 0x08;
        return;
    }
    g_statEffectTimer = 0x24;
    g_gameFlags2 &= 0xf3;
}

// Game A STAT effect 1: swap the BG tile map and the scroll at each LYC.
static void statEffect1(void)
{
    rLCDC ^= 0x08;
    uint8_t b = g_statEffect1Arg;
    uint8_t scy = g_boardScroll[0], scx = g_boardScroll[1];
    if (g_gameFlags8 & 0x20) {
        b = 0x10 - b;
        scy = scx = 0;
    }
    g_gameFlags8 ^= 0x20;
    rSCY = scy;
    VOLATILE(rSCX) = scx;
    if ((VOLATILE(rLYC) += b) != 0x90)
        return;
    rLYC = 0;
    if (--g_statEffect2Arg)
        return;
    g_statEffect2Arg = 3;
    g_statEffect1Arg++;
    if (--g_statEffectTimer)
        return;
    rLCDC ^= 0x08;
    rSCY = g_boardScroll[0];
    rSCX = g_boardScroll[1];
    g_gameFlags2 &= ~0x04;
}

// Game A STAT effect 2: scroll split. $ffaa bit 3 zeroes SCX.
static void statEffect2(void)
{
    VOLATILE(rSCX) = (g_gameFlags2 & 0x08) ? 0 : g_statEffect2Arg;
    rLYC ^= 0x18;
    if ((g_gameFlags2 ^= 0x08) & 0x20)
        return;
    if (--g_statEffect1Arg)
        return;
    g_statEffect1Arg = 9;
    g_statEffect2Arg++;
}

// LCD STAT in Game A: run the effect in g_gameAStatEffect while $ffaa bit 2 is set.
void irqStatGameA(void)
{
    if (!(g_gameFlags2 & 0x04))
        return;
    switch (g_gameAStatEffect) {
    case 0: statEffect0(); break;
    case 1: statEffect1(); break;
    case 2: statEffect2(); break;
    }
}

// Serial: swap the link bytes and run the link handler, in bank 3
// (Game B) or bank 5 (Game A).
void irqSerial(void)
{
    rIE &= 0xf3;
    __asm__("ei");
    g_serialRecv = rSB;
    rSB = g_serialSend;
    g_serialSend = 0;
    if (g_topState == 2 && (g_topModeFlags & 0x20))
        Vec3_GameB_LinkUpdate();
    else
        Bank5_EntryVector();
    if (!(g_topModeFlags & 0x04))
        rSC = 0x80;     // link slave: wait for the next byte
    if (g_topModeFlags & 0x40)
        g_topModeFlags |= 0x10;
    rIE |= 0x0c;
}
