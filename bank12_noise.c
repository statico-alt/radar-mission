// Noise channel sound effects (NR41-NR44).
#include "game.h"
#include "sound.h"

extern const uint8_t NoiseSfx01_Start_NoiseRegs[], NoiseSfx01_Update_NoiseRegs[];
extern const uint8_t NoiseSfx02_Start_NoiseRegs[], NoiseSfx02_Update_NoiseRegs[];
extern const uint8_t NoiseSfx03_Start_NoiseRegs[], NoiseSfx03_Update_NoiseRegs[];
extern const uint8_t NoiseSfx03_Update_NoiseRegs2[], NoiseSfx03_Update_NoiseRegs3[];
extern const uint8_t NoiseSfx04_Start_NoiseRegs[], NoiseSfx05_Start_NoiseRegs[];
extern const uint8_t NoiseSfx07_Start_NoiseRegs[], NoiseSfx08_Start_NoiseRegs[];
extern const uint8_t NoiseSfx09_Start_NoiseRegs[];
extern const uint8_t NoiseSfx0A_Start_NoiseRegs[], NoiseSfx0A_Start_Sq1Regs[];
extern const uint8_t NoiseSfx0C_Start_NoiseRegs[], NoiseSfx0D_Start_NoiseRegs[];
extern const uint8_t NoiseSfx0E_Run_NoiseRegs[];
extern const uint8_t NoiseSfx0F_Start_NoiseRegs[], NoiseSfx0F_Update_NoiseRegs[];

// Write a 4-byte entry to NR41-NR44.
static void noiseWrite(const uint8_t *p)
{
    sfxWriteRegs4(&rNR41, p);
}

// Clear the noise SFX ID and give channel 4 back to the music.
static void noiseSfxEnd(void)
{
    g_noiseSfxId = 0;
    musicUnmuteCh4();
}

// Start a one-step noise effect that NoiseSfx07_Update runs (noise SFX ID 7).
// timer2 = first wait, then 1 step with reloads $10 / $0f.
static void noiseSfxStartStep(const uint8_t *p, uint8_t timer2)
{
    sfxClearTimers();
    g_noiseSfxId = 0x07;
    musicMuteCh4();
    sfxSetSq2Ptr(p);
    noiseWrite(p);
    g_sfxTimer2 = timer2;
    g_sfxReload1 = 0x10;
    g_sfxReload2 = 0x0f;
    g_sfxSteps2 = 0x01;
}

// Noise effect that stops all square effects first. Blocked by $de39.
static void noiseSfx01_Start(void)
{
    if (g_sfx04Busy)
        return;
    noiseSfxStop();
    sfxClearTimers();
    sfxStopAll();
    g_sfxBlockSfx0A = 0x01;
    g_sfxFlagC6 = 0x01;
    g_sfxBlockNoise0D = 0x01;
    noiseSfxSetId(0x01);
    sfxSetIdAndSweep(0);
    g_sfxFlag33 = 0;
    g_soundFadeState = 0;
    g_sfxBlockSfx17B = 0;
    musicMuteCh4();
    noiseWrite(NoiseSfx01_Start_NoiseRegs);
}

// Second part of NoiseSfx01: steps through noise data as noise SFX 7.
static void noiseSfx01_Update(void)
{
    g_noiseSfxId = 0x07;
    musicMuteCh4();
    sfxSetSq2Ptr(NoiseSfx01_Update_NoiseRegs);
    noiseWrite(NoiseSfx01_Update_NoiseRegs);
    sfxClearPhaseFlags();
    g_sfxTimer2 = 0x08;
    g_sfxReload1 = 0x07;
    g_sfxReload2 = 0x36;
    g_sfxSteps2 = 0x02;
}

// Noise one-shot. Blocked by $de39.
static void noiseSfx02_Start(void)
{
    if (g_sfx04Busy)
        return;
    g_noiseSfxId = 0x02;
    musicMuteCh4();
    noiseWrite(NoiseSfx02_Start_NoiseRegs);
    sfxClearPhaseFlags();
}

// Write the second entry, hold $20 frames, then give channel 4 back.
static void noiseSfx02_Update(void)
{
    if (!g_sfxPhase1) {
        noiseWrite(NoiseSfx02_Update_NoiseRegs);
        g_sfxPhase1 = 1;
        g_sfxTimer1 = 0x20;
        return;
    }
    if (sfxTickSq1Timer())
        return;
    noiseSfxSetId(0);
    musicUnmuteCh4();
}

// Noise effect with four entries. Stops the square 1 effect.
static void noiseSfx03_Start(void)
{
    g_sfxSq1Id = 0;
    rNR12 = 0;
    g_noiseSfxId = 0x03;
    sfxClearTimers();
    musicMuteCh4();
    noiseWrite(NoiseSfx03_Start_NoiseRegs);
}

// Phases 1-3 write the next noise entry every 8 frames (last one held $40).
// Then give channel 4 back.
static void noiseSfx03_Update(void)
{
    if (!g_sfxPhase1) {
        noiseWrite(NoiseSfx03_Update_NoiseRegs);
        g_sfxPhase1 = 1;
        g_sfxTimer1 = 0x08;
        return;
    }
    if (!g_sfxPhase2) {
        if (sfxTickSq1Timer())
            return;
        noiseWrite(NoiseSfx03_Update_NoiseRegs2);
        g_sfxPhase2 = 1;
        g_sfxTimer1 = 0x08;
        return;
    }
    if (!g_sfxPhase3) {
        if (sfxTickSq1Timer())
            return;
        noiseWrite(NoiseSfx03_Update_NoiseRegs3);
        g_sfxPhase3 = 1;
        g_sfxTimer1 = 0x40;
        return;
    }
    if (sfxTickSq1Timer())
        return;
    noiseSfxEnd();
}

// Long noise effect. Stops all square effects and sets NR51 to $ff.
static void noiseSfx04_Start(void)
{
    sfxClearTimers();
    g_noiseSfxId = 0x07;
    g_sfxBlockNoise0D = 0x07;
    sfxSetIdAndSweep(0);
    g_sfxFlag33 = 0;
    sfxStopAll();
    rNR51 = 0xff;
    musicMuteCh4();
    sfxSetSq2Ptr(NoiseSfx04_Start_NoiseRegs);
    noiseWrite(NoiseSfx04_Start_NoiseRegs);
    g_sfxTimer1 = 0x07;
    g_sfxTimer2 = 0x08;
    g_sfxReload1 = 0x07;
    g_sfxReload2 = 0x1e;
    g_sfxSteps2 = 0x01;
}

// Noise one-shot, 8 frames. Blocked by $dedf.
static void noiseSfx05_Start(void)
{
    if (g_sfxBlockNoise05)
        return;
    g_noiseSfxId = 0x05;
    musicMuteCh4();
    noiseWrite(NoiseSfx05_Start_NoiseRegs);
    g_sfxTimer2 = 0x08;
}

// After 8 frames rewrite the entry, then give channel 4 back.
static void noiseSfx05_Update(void)
{
    if (sfxTickSq2Timer())
        return;
    noiseWrite(NoiseSfx05_Start_NoiseRegs);
    noiseSfxEnd();
}

// Noise effect, $20 + $30 frames. Blocked by $de34.
static void noiseSfx06_Start(void)
{
    if (g_sfxBlockNoise06)
        return;
    g_noiseSfxId = 0x06;
    musicMuteCh4();
    noiseWrite(NoiseSfx03_Update_NoiseRegs3);
    g_sfxTimer1 = 0x20;
    g_sfxTimer2 = 0x30;
}

// Wait for timer 1, then timer 2, then give channel 4 back.
static void noiseSfx06_Update(void)
{
    if (!g_sfxPhase1) {
        if (sfxTickSq1Timer())
            return;
        g_sfxPhase1 = 1;
        return;
    }
    if (sfxTickSq2Timer())
        return;
    noiseSfxEnd();
}

// Multi-step noise effect ($13 steps of 7 frames).
static void noiseSfx07_Start(void)
{
    g_noiseSfxId = 0x07;
    musicMuteCh4();
    sfxSetSq2Ptr(NoiseSfx07_Start_NoiseRegs);
    noiseWrite(NoiseSfx07_Start_NoiseRegs);
    sfxClearPhaseFlags();
    g_sfxTimer2 = 0x07;
    g_sfxReload1 = 0x07;
    g_sfxReload2 = 0x07;
    g_sfxSteps2 = 0x13;
}

// Like NoiseSfx07, but silences square 1 and sets $dedd so channel 1
// stays muted when it ends.
static void noiseSfx0B_Start(void)
{
    rNR12 = 0;
    g_noiseSfx0BKeepSq1 = 0x01;
    noiseSfx07_Start();
}

// Shared update for multi-step noise effects (noise SFX ID 7).
// Steps the noise data by 4 bytes every $de1d frames, then releases.
static void noiseSfx07_Update(void)
{
    if (!g_sfxPhase2) {
        if (sfxTickSq2Timer())
            return;
        const uint8_t *p = sfxGetSq2Ptr() + 4; // 4 bytes per noise data entry
        sfxSetSq2Ptr(p);
        noiseWrite(p);
        g_sfxTimer2 = g_sfxReload1;
        if (--g_sfxSteps2)
            return;
        g_sfxPhase2 = 1;
        g_sfxTimer2 = g_sfxReload2;
        return;
    }
    if (sfxTickSq2Timer())
        return;
    sfxSetFlagD4AndC3(0);
    noiseSfxSetIdAnd29(0);
    musicUnmuteCh4();
    if (g_noiseSfx0BKeepSq1)
        return;
    g_noiseSfx0BKeepSq1 = 0;
    rNR10 = 0;
    musicUnmuteCh1();
}

// Noise one-shot, $0a frames.
static void noiseSfx08_Start(void)
{
    g_noiseSfxId = 0x08;
    musicMuteCh4();
    noiseWrite(NoiseSfx08_Start_NoiseRegs);
    g_sfxTimer2 = 0x0a;
}

// Wait for timer 2, then give channel 4 back.
static void noiseSfx08_Update(void)
{
    if (sfxTickSq2Timer())
        return;
    noiseSfxEnd();
}

// Noise effect, 1 step after $30 frames. Blocked by $de2f.
static void noiseSfx09_Start(void)
{
    if (g_sfxBlockNoise09)
        return;
    sfxClearTimers();
    g_noiseSfxId = 0x07;
    g_sfxSq1Id = 0;
    rNR12 = 0;
    g_sfxBlockSfx17B = 0;
    musicMuteCh4();
    sfxSetSq2Ptr(NoiseSfx09_Start_NoiseRegs);
    noiseWrite(NoiseSfx09_Start_NoiseRegs);
    g_sfxTimer2 = 0x30;
    g_sfxReload1 = 0x10;
    g_sfxReload2 = 0x0f;
    g_sfxSteps2 = 0x01;
}

// Noise + square 1 one-shot, $40 frames.
static void noiseSfx0A_Start(void)
{
    g_noiseSfxId = 0x0a;
    musicMuteCh4();
    musicMuteCh1();
    sfxSetSq2Ptr(NoiseSfx0A_Start_NoiseRegs);
    noiseWrite(NoiseSfx0A_Start_NoiseRegs);
    sfxWriteRegs5(NoiseSfx0A_Start_Sq1Regs);
    g_sfxTimer2 = 0x40;
}

// Wait for timer 2, then clear the noise SFX ID.
static void noiseSfx0A_Update(void)
{
    if (sfxTickSq2Timer())
        return;
    g_noiseSfxId = 0;
}

// Multi-step noise effect ($11 steps of 3 frames).
static void noiseSfx0C_Start(void)
{
    g_noiseSfxId = 0x07;
    musicMuteCh4();
    sfxSetSq2Ptr(NoiseSfx0C_Start_NoiseRegs);
    noiseWrite(NoiseSfx0C_Start_NoiseRegs);
    sfxClearPhaseFlags();
    g_sfxTimer2 = 0x01;
    g_sfxReload1 = 0x03;
    g_sfxReload2 = 0x03;
    g_sfxSteps2 = 0x11;
}

// Noise effect, 1 step after $33 frames. Blocked by $ded4 and $dede.
static void noiseSfx0D_Start(void)
{
    if (g_sfxBlockNoise0D)
        return;
    if (g_sfx1ABusy)
        return;
    noiseSfxStartStep(NoiseSfx0D_Start_NoiseRegs, 0x33);
}

// Noise effect, 1 step after $25 frames. Blocked by $ded4 and $dede.
static void noiseSfx0E_Run(void)
{
    if (g_sfxBlockNoise0D)
        return;
    if (g_sfx1ABusy)
        return;
    noiseSfxStartStep(NoiseSfx0E_Run_NoiseRegs, 0x25);
}

// Noise effect with two entries, 7 frames apart.
static void noiseSfx0F_Start(void)
{
    g_noiseSfxId = 0x0f;
    musicMuteCh4();
    noiseWrite(NoiseSfx0F_Start_NoiseRegs);
    sfxClearTimers();
    g_sfxTimer1 = 0x07;
}

// Write the second entry, hold 8 frames, then give channel 4 back.
static void noiseSfx0F_Update(void)
{
    if (!g_sfxPhase1) {
        if (sfxTickSq1Timer())
            return;
        noiseWrite(NoiseSfx0F_Update_NoiseRegs);
        g_sfxPhase1 = 1;
        g_sfxTimer1 = 0x08;
        return;
    }
    if (sfxTickSq1Timer())
        return;
    noiseSfxSetId(0);
    musicUnmuteCh4();
}

// Noise channel sound effects. A new request in NoiseSfx_Queue ($deb8) runs
// NoiseSfxNN_Start. Otherwise the active effect in $deb9 gets its update.
// While $de2e is set, the noise SFX is stopped.
void noiseSfxUpdate(void)
{
    if (g_sfxBlockAll) {
        noiseSfxStop();
        return;
    }
    switch (NoiseSfx_Queue) {
    case 0x01: noiseSfx01_Start(); return;
    case 0x02: noiseSfx02_Start(); return;
    case 0x03: noiseSfx03_Start(); return;
    case 0x04: noiseSfx04_Start(); return;
    case 0x05: noiseSfx05_Start(); return;
    case 0x06: noiseSfx06_Start(); return;
    case 0x07: noiseSfx07_Start(); return;
    case 0x08: noiseSfx08_Start(); return;
    case 0x09: noiseSfx09_Start(); return;
    case 0x0a: noiseSfx0A_Start(); return;
    case 0x0b: noiseSfx0B_Start(); return;
    case 0x0c: noiseSfx0C_Start(); return;
    case 0x0d: noiseSfx0D_Start(); return;
    case 0x0e: noiseSfx0E_Run(); return;
    case 0x0f: noiseSfx0F_Start(); return;
    }
    switch (g_noiseSfxId) {
    case 0x01: noiseSfx01_Update(); break;
    case 0x02: noiseSfx02_Update(); break;
    case 0x03: noiseSfx03_Update(); break;
    case 0x05: noiseSfx05_Update(); break;
    case 0x06: noiseSfx06_Update(); break;
    case 0x07: noiseSfx07_Update(); break;
    case 0x08: noiseSfx08_Update(); break;
    case 0x0a: noiseSfx0A_Update(); break;
    case 0x0e: noiseSfx0E_Run(); break;
    case 0x0f: noiseSfx0F_Update(); break;
    }
}
