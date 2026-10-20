// Square channel sound effects and the SFX helpers. Each SfxNN_Start writes
// the first register entry, mutes the music channels it needs and sets the
// step timers. Each SfxNN_Update runs one frame of the effect.
#include "game.h"
#include "sound.h"

extern const uint8_t Sfx01_Start_Sq1Regs[], Sfx02_Start_Sq1Regs[], Sfx02_Start_Sq2Regs[];
extern const uint8_t Sfx04_Start_Sq1Regs[], Sfx05_Start_Sq1Regs[], Sfx06_Start_Sq1Regs[];
extern const uint8_t Sfx08_Start_Sq1Regs[], Sfx0A_Start_Sq1Regs[];
extern const uint8_t Sfx0B_Start_Sq1Regs[], Sfx0B_Start_NoiseRegs[];
extern const uint8_t Sfx0D_Start_Sq1Regs[], Sfx0D_Start_NoiseRegs[];
extern const uint8_t Sfx0D_Update_Sq1Regs[], Sfx0D_Update_NoiseRegs[];
extern const uint8_t Sfx0F_Start_Sq1Regs[], Sfx0F_Start_Sq2Regs[];
extern const uint8_t Sfx10_Start_Sq1Regs[], Sfx10_Start_NoiseRegs[];
extern const uint8_t Sfx11_Play_Sq1Regs[], Sfx11_Play_NoiseRegs[], Sfx11_Update_Sq1Regs[];
extern const uint8_t Sfx13_Start_NoiseRegs[], Sfx14_Start_Sq1Regs[], Sfx14_Start_Sq2Regs[];
extern const uint8_t Sfx15_Start_Sq1Regs[], Sfx17_Start_Sq1Regs[], Sfx18_Start_Sq1Regs[];
extern const uint8_t Sfx19_Start_Sq1Regs[], Sfx19_Start_Sq2Regs[];
extern const uint8_t Sfx1A_Start_Sq1Regs[], Sfx1A_Start_Sq2Regs[];
extern const uint8_t Sfx1B_Start_Sq1Regs[], Sfx1B_Start_Sq2Regs[];
extern const uint8_t Sfx1D_Start_Sq1Regs[], Sfx1D_Start_Sq2Regs[];
extern const uint8_t Sfx21_Start_Sq1Regs[], Sfx32_Update_Sq1Regs[];

// ---------------------------------------------------------------------------
// SFX helpers: step timers, data pointers, busy flags and channel mute bits.
// ---------------------------------------------------------------------------

// Set all four square SFX step timers ($de1b-$de1e) to a.
static void sfxSetAllTimers(uint8_t a)
{
    g_sfxTimer1 = a;
    g_sfxTimer2 = a;
    g_sfxReload1 = a;
    g_sfxReload2 = a;
}

// Set both SFX step counts ($de1f square 2, $de20 square 1) to a.
static void sfxSetStepCounts(uint8_t a)
{
    g_sfxSteps1 = a;
    g_sfxSteps2 = a;
}

// Clear the SFX phase flags $de36 and $de15-$de17.
void sfxClearPhaseFlags(void)
{
    g_sfxPhase4 = 0;
    g_sfxPhase1 = 0;
    g_sfxPhase2 = 0;
    g_sfxPhase3 = 0;
}

// Clear the step timers, the reload values, the step counts and the
// phase flags. (The asm falls through SfxSetSq2Timers with A = 0.)
void sfxClearTimers(void)
{
    g_sfxTimer1 = 0;
    g_sfxReload1 = 0;
    g_sfxTimer2 = 0;
    g_sfxReload2 = 0;
    sfxSetStepCounts(0);
    sfxClearPhaseFlags();
}

// Small store helpers. Each one writes a to the RAM bytes in its name.
// $dea1 is the active square 1 SFX ID.

// Writes a to $de2f and $dec6.
static void sfxSetFlag2F(uint8_t a)
{
    g_sfxBlockNoise09 = a;
    g_sfxFlagC6 = a;
}

// Writes a to $dec3, $de2f and $dec6.
void sfxSetFlagC3(uint8_t a)
{
    g_sfxSq1Busy = a;
    sfxSetFlag2F(a);
}

// Writes a to $dea1, $dec3, $de2f and $dec6.
static void sfxSetIdAndFlags(uint8_t a)
{
    g_sfxSq1Id = a;
    sfxSetFlagC3(a);
}

// Writes a to $de2a and $dea1.
static void sfxSetFlag2AAndId(uint8_t a)
{
    g_sfxBlockSfx0A = a;
    g_sfxSq1Id = a;
}

// Writes a to $ded4, $de2a and $dea1.
static void sfxSetFlagD4AndId(uint8_t a)
{
    g_sfxBlockNoise0D = a;
    sfxSetFlag2AAndId(a);
}

// Writes a to $dec3 and $dec6.
static void sfxSetFlagC3Only(uint8_t a)
{
    g_sfxSq1Busy = a;
    g_sfxFlagC6 = a;
}

// Writes a to $de2a, $dec3 and $dec6.
static void sfxSetFlag2AAndC3(uint8_t a)
{
    g_sfxBlockSfx0A = a;
    sfxSetFlagC3Only(a);
}

// Writes a to $ded4, $de2a, $dec3 and $dec6.
void sfxSetFlagD4AndC3(uint8_t a)
{
    g_sfxBlockNoise0D = a;
    sfxSetFlag2AAndC3(a);
}

// Writes a to $dea1 and rNR10 (sweep).
void sfxSetIdAndSweep(uint8_t a)
{
    g_sfxSq1Id = a;
    rNR10_PTR = a;
}

// Writes a to $dea1 and $ded4.
static void sfxSetIdAndFlagD4(uint8_t a)
{
    g_sfxSq1Id = a;
    g_sfxBlockNoise0D = a;
}

// Writes a to $de2f and $dea1.
static void sfxSetIdAndFlag2F(uint8_t a)
{
    g_sfxBlockNoise09 = a;
    g_sfxSq1Id = a;
}

// Writes a to rNR10, $de2f and $dea1.
static void sfxSetSweepAndId(uint8_t a)
{
    rNR10_PTR = a;
    sfxSetIdAndFlag2F(a);
}

// Writes a to $deb9 (noise SFX ID), $de30 and $de2f.
void noiseSfxSetId(uint8_t a)
{
    g_noiseSfxId = a;
    g_sfxNoiseBusy = a;
    g_sfxBlockNoise09 = a;
}

// Writes a to $de29, $deb9, $de30 and $de2f.
void noiseSfxSetIdAnd29(uint8_t a)
{
    g_sfxFlag29 = a;
    noiseSfxSetId(a);
}

// Decrement the square 1 step timer $de1b. Returns the new value.
uint8_t sfxTickSq1Timer(void)
{
    return --g_sfxTimer1;
}

// Decrement the square 2 step timer $de1c. Returns the new value.
uint8_t sfxTickSq2Timer(void)
{
    return --g_sfxTimer2;
}

// Stop the noise SFX: clear $deb9 and silence NR42.
void noiseSfxStop(void)
{
    g_noiseSfxId = 0;
    rNR42 = 0;
}

// Store p as the square 1 SFX data pointer ($de25/$de26, high byte first).
static void sfxSetSq1Ptr(const uint8_t *p)
{
    g_sfxSq1PtrHi = (uint16_t)p >> 8;
    g_sfxSq1PtrLo = (uint16_t)p;
}

// Store p as the square 2 SFX data pointer ($de27/$de28, high byte first).
void sfxSetSq2Ptr(const uint8_t *p)
{
    g_sfxSq2PtrHi = (uint16_t)p >> 8;
    g_sfxSq2PtrLo = (uint16_t)p;
}

// Load the square 1 SFX data pointer.
static const uint8_t *sfxGetSq1Ptr(void)
{
    return (const uint8_t *)((g_sfxSq1PtrHi << 8) | g_sfxSq1PtrLo);
}

// Load the square 2 SFX data pointer.
const uint8_t *sfxGetSq2Ptr(void)
{
    return (const uint8_t *)((g_sfxSq2PtrHi << 8) | g_sfxSq2PtrLo);
}

// Set or clear bit 7 of a music channel's flag byte ($de5f, $de6f, $de8f).
// While bit 7 is set the music engine does not write that channel's
// registers, so a sound effect can use it.
void musicMuteCh1(void)
{
    g_musicCh1Flags |= 0x80;
}

static void musicMuteCh2(void)
{
    g_musicCh2Flags |= 0x80;
}

void musicMuteCh4(void)
{
    g_musicCh4Flags |= 0x80;
}

void musicUnmuteCh1(void)
{
    g_musicCh1Flags &= ~0x80;
}

static void musicUnmuteCh2(void)
{
    g_musicCh2Flags &= ~0x80;
}

void musicUnmuteCh4(void)
{
    g_musicCh4Flags &= ~0x80;
}

// Copy 5 bytes from p to NR10-NR14.
void sfxWriteRegs5(const uint8_t *p)
{
    rNR10 = p[0];
    sfxWriteRegs4(&rNR11, p + 1);
}

// Copy 4 bytes from p to the registers at reg (NR11, NR21 or NR41).
void sfxWriteRegs4(volatile uint8_t *reg, const uint8_t *p)
{
    reg[0] = p[0];
    reg[1] = p[1];
    reg[2] = p[2];
    reg[3] = p[3];
}

// Advance the square 1 SFX data pointer by 5 and write the next entry to NR10-NR14.
static void sfxSq1NextStep(void)
{
    const uint8_t *p = sfxGetSq1Ptr() + 5; // 5 bytes per square 1 data entry
    sfxSetSq1Ptr(p);
    sfxWriteRegs5(p);
}

// Decrement the square 1 step count $de20. At 0, set phase flag $de15
// and load the release timer ($28 if $de31 is set, else $10).
static void sfxSq1CountStep(void)
{
    if (--g_sfxSteps1)
        return;
    g_sfxPhase1 = 1;
    g_sfxTimer1 = g_sfxLongRelease ? 0x28 : 0x10;
}

// Stop the square 1 effect, clear the SFX busy flags and give channel 1 back to the music.
void sfxStopAll(void)
{
    rNR12 = 0;
    g_sfxSq1Id = 0;
    g_sfx04Busy = 0;
    g_sfxSq1Busy = 0;
    g_sfxBlockAll = 0;
    g_sfxBlockCue1 = 0;
    g_sfxNoiseBusy = 0;
    g_sfxBlockSfx0B = 0;
    g_sfxBlockSfx0D = 0;
    musicUnmuteCh1();
}

// ---------------------------------------------------------------------------
// The effects.
// ---------------------------------------------------------------------------

// Square 1 one-shot. Does nothing while $de35 is set. Plays for 6 frames.
static void sfx01_Start(void)
{
    if (g_sfxBlockSfx01)
        return;
    sfxSetFlagD4AndId(0x01);
    g_sfxTimer1 = 0;
    g_sfxBlockNoise05 = 0;
    g_sfxBlockSfx02 = 0;
    musicMuteCh1();
    musicUnmuteCh2();
    sfxWriteRegs5(Sfx01_Start_Sq1Regs);
    g_sfxTimer1 = 0x06;
}

// Wait for timer 1, then clear the SFX ID and give channel 1 back.
static void sfx01_Update(void)
{
    if (sfxTickSq1Timer())
        return;
    sfxSetFlagD4AndId(0);
    musicUnmuteCh1();
}

// Two-channel effect (square 1 + square 2), 10 steps of 2 frames.
// Does nothing while $dee2 is set.
static void sfx02_Start(void)
{
    if (g_sfxBlockSfx02)
        return;
    sfxSetIdAndFlagD4(0x06);
    g_sfxBlockNoise05 = 0x06;
    g_noiseSfxId = 0;
    rNR42 = 0;
    musicMuteCh1();
    musicMuteCh2();
    sfxSetSq1Ptr(Sfx02_Start_Sq1Regs);
    sfxWriteRegs5(Sfx02_Start_Sq1Regs);
    sfxSetSq2Ptr(Sfx02_Start_Sq2Regs);
    sfxWriteRegs4(&rNR21, Sfx02_Start_Sq2Regs);
    // The asm calls SfxSetSq2Timers here. The lines below set the timers
    // and step counts again, so only the phase flags change.
    sfxClearPhaseFlags();
    sfxSetAllTimers(0x02);
    sfxSetStepCounts(0x0a);
    g_sfxSq2Active = 1;
}

// Falling pitch sweep on square 1 (frequency $87e0 down to $87c5).
// Does nothing while $ded8 is set.
static void sfx03_Start(void)
{
    if (g_sfxBlockSfx03)
        return;
    g_sfxSq1Id = 0x03;
    g_sfxSq1Busy = 0x03;
    g_sfxBlockSfx0B = 0x03;
    g_sfxBlockSfx0D = 0x03;
    g_sfxBlockSfx17B = 0;
    musicMuteCh1();
    g_sfxSweepFreqLo = 0xe0;
    g_sfxSweepFreqHi = 0x87;
    g_sfxSweepDelay = 0x01;
}

// Run one frame of the Sfx03 sweep: lower the frequency by one each step.
// At $c5 the effect ends through SfxStopAll.
static void sfx03_Update(void)
{
    g_sfxSweepSubsteps = 1;
    rNR10 = 0;
    rNR11 = 0xff;
    rNR12 = 0x8f;
    do {
        if (--g_sfxSweepDelay)
            return;
        uint8_t lo = g_sfxSweepFreqLo - 1;
        if (lo == 0xc5) {
            sfxStopAll();
            return;
        }
        g_sfxSweepFreqLo = lo;
    } while (--g_sfxSweepSubsteps);
    rNR13 = g_sfxSweepFreqLo;
    rNR14 = g_sfxSweepFreqHi;
    g_sfxSweepDelay = 0x02;
}

// Long square 1 effect with 9 steps. Does nothing while $dee1 (pause) is set.
// Silences all other effects and sets $de39 to block most other SFX.
static void sfx04_Start(void)
{
    if (g_sfxPauseLock)
        return;
    rNR12 = 0;
    rNR22 = 0;
    rNR42 = 0;
    g_sfxSq1Busy = 0;
    g_soundFadeState = 0;
    g_sfxBlockSfx03 = 0;
    g_noiseSfxId = 0;
    sfxClearTimers();
    g_sfx04Busy = 0x06;
    sfxSetFlag2F(0x06);
    g_sfxFlag29 = 0x06;
    g_sfxBlockSfx0A = 0x06;
    g_sfxSq1Id = 0x06;
    g_sfxBlockNoise0D = 0x06;
    musicMuteCh1();
    sfxSetSq1Ptr(Sfx04_Start_Sq1Regs);
    sfxWriteRegs5(Sfx04_Start_Sq1Regs);
    g_sfxTimer1 = 0x02;
    g_sfxReload1 = 0x04;
    g_sfxSteps1 = 0x09;
}

// Square 1 effect, 4 steps of 15 frames. Uses the Sfx07 update.
static void sfx05_Start(void)
{
    g_sfxFlag29 = 0x06;
    g_sfxBlockSfx02 = 0x06;
    sfxSetFlagD4AndId(0x06);
    noiseSfxStop();
    musicMuteCh1();
    musicUnmuteCh2();
    sfxSetSq1Ptr(Sfx05_Start_Sq1Regs);
    sfxWriteRegs5(Sfx05_Start_Sq1Regs);
    sfxClearTimers();
    g_sfxTimer1 = 0x0f;
    g_sfxReload1 = 0x0f;
    g_sfxSteps1 = 0x04;
}

// Step the square 2 part of an effect when $de32 is set. Each step advances
// the square 2 data pointer by 4 and writes NR21-NR24.
void sfxSq2Update(void)
{
    if (!g_sfxSq2Active)
        return;
    if (!g_sfxPhase2) {
        if (sfxTickSq2Timer())
            return;
        const uint8_t *p = sfxGetSq2Ptr() + 4; // 4 bytes per square 2 / noise data entry
        sfxSetSq2Ptr(p);
        sfxWriteRegs4(&rNR21, p);
        g_sfxTimer2 = g_sfxReload2;
        if (--g_sfxSteps2)
            return;
        g_sfxPhase2 = 1;
        if (!g_sfxLongRelease)
            g_sfxTimer2 = 0x10;
        return;
    }
    if (sfxTickSq2Timer())
        return;
    sfxSetFlagD4AndId(0);
    sfxSetIdAndSweep(0);
    g_sfxSq2Active = 0;
    musicUnmuteCh2();
    musicUnmuteCh1();
}

// Square 1 effect with 8 steps of 4 frames, and queues noise SFX 7.
// Blocked by $de39 and $de30. With $de2e set, it only steps square 2.
// Square 1 part skipped if $dec3 is set.
static void sfx06_Start(void)
{
    if (g_sfx04Busy)
        return;
    if (g_sfxNoiseBusy)
        return;
    if (g_sfxBlockAll) {
        sfxSq2Update();
        return;
    }
    g_soundFadeState = 0;
    g_sfxSq2Active = 0;
    g_sfxBlockSfx17B = 0;
    g_sfxFlag29 = 0x06;
    g_sfxBlockSfx0A = 0x06;
    sfxSetFlag2F(0x06);
    NoiseSfx_Queue = 0x07;
    if (g_sfxSq1Busy)
        return;
    g_sfxSq1Id = 0x06;
    musicMuteCh1();
    sfxSetSq1Ptr(Sfx06_Start_Sq1Regs);
    sfxWriteRegs5(Sfx06_Start_Sq1Regs);
    sfxClearPhaseFlags();
    g_sfxTimer1 = 0x04;
    g_sfxReload1 = 0x04;
    g_sfxSteps1 = 0x08;
}

// Step the Sfx06 square 1 part. When it ends, clear the busy flags and give
// channels 1 and 4 back. Then step the square 2 part.
static void sfx06_Update(void)
{
    if (!g_sfxPhase1) {
        if (!sfxTickSq1Timer()) {
            sfxSq1NextStep();
            g_sfxTimer1 = g_sfxReload1;
            sfxSq1CountStep();
        }
    } else if (!sfxTickSq1Timer()) {
        g_sfxFlagC6 = 0;
        g_sfxNoiseBusy = 0;
        g_sfxBlockAll = 0;
        g_sfxBlockNoise0D = 0;
        g_sfx04Busy = 0;
        g_sfxBlockSfx0A = 0;
        g_sfx1ABusy = 0;
        g_sfxBlockNoise05 = 0;
        g_sfxBlockSfx02 = 0;
        sfxSetSweepAndId(0);
        musicUnmuteCh1();
        musicUnmuteCh4();
    }
    sfxSq2Update();
}

// Shared update for multi-step square 1 effects (Sfx05, Sfx08, Sfx0B, Sfx32...).
// Phase 1: step the data every $de1d frames. Phase 2: release, then repeat
// through Sfx32 while $de2c is above 1, else clear all busy flags.
static void sfx07_Update(void)
{
    if (!g_sfxPhase1) {
        if (sfxTickSq1Timer())
            return;
        sfxSq1NextStep();
        g_sfxTimer1 = g_sfxReload1;
        sfxSq1CountStep();
        return;
    }
    if (sfxTickSq1Timer())
        return;
    musicUnmuteCh1();
    musicUnmuteCh4();
    rNR10 = 0;
    g_sfx0AActive = 0;
    g_sfxLongRelease = 0;
    g_sfxBlockNoise0D = 0;
    g_sfxBlockSfx17 = 0;
    g_sfxBlockSfx0A = 0;
    g_sfxBlockNoise09 = 0;
    uint8_t n = g_sfx07Repeats - 1;
    if (n) {
        g_sfx07Repeats = n;
        g_sfxSq1Id = 0x32;
        return;
    }
    g_sfxBlockAll = 0;
    g_sfx04Busy = 0;
    g_sfxNoiseBusy = 0;
    g_sfxBlockNoise06 = 0;
    g_sfxBlockSfx01 = 0;
    g_sfxBlockSfx03 = 0;
    g_sfxBlockSfx0D = 0;
    g_sfxBlockCue1 = 0;
    g_sfxBlockSfx0B = 0;
    sfxSetIdAndFlag2F(0);
}

// Square 1 effect, 2 steps. Blocked by $de2a and $ded8.
void sfx0A_Start(void)
{
    if (g_sfxBlockSfx0A)
        return;
    if (g_sfxBlockSfx03)
        return;
    g_sfxTimer1 = 0;
    g_sfx0AStepTime = 0;
    g_sfxPhase1 = 0;
    g_sfxSteps1 = 0;
    g_soundFadeState = 0;
    g_sfxBlockSfx17B = 0;
    g_sfx0AActive = 0x0a;
    g_sfxSq1Id = 0x0a;
    musicMuteCh1();
    sfxSetSq1Ptr(Sfx0A_Start_Sq1Regs);
    sfxWriteRegs5(Sfx0A_Start_Sq1Regs);
    g_sfxTimer1 = 0x03;
    g_sfx0AStepTime = 0x02;
    g_sfxLongRelease = 0x01;
    g_sfxSteps1 = 0x02;
    g_sfx07Repeats = 0x01;
}

// Step Sfx0A every $ded5 frames; after the release, give channel 1 back.
static void sfx0A_Update(void)
{
    if (!g_sfxPhase1) {
        if (sfxTickSq1Timer())
            return;
        sfxSq1NextStep();
        g_sfxTimer1 = g_sfx0AStepTime;
        sfxSq1CountStep();
        return;
    }
    if (sfxTickSq1Timer())
        return;
    musicUnmuteCh1();
    g_sfx0AActive = 0;
    sfxSetFlag2AAndId(0);
}

// Square 1 effect, 1 step of 8 frames. Uses the Sfx07 update.
static void sfx08_Start(void)
{
    sfxSetIdAndFlagD4(0x07);
    noiseSfxStop();
    musicMuteCh1();
    musicUnmuteCh2();
    sfxSetSq1Ptr(Sfx08_Start_Sq1Regs);
    sfxWriteRegs5(Sfx08_Start_Sq1Regs);
    sfxClearPhaseFlags();
    g_sfxTimer1 = 0x08;
    g_sfxReload1 = 0x08;
    g_sfxSteps1 = 0x01;
    g_sfx07Repeats = 0x01;
}

// Square 1 + noise effect, 5 steps. Blocked by $de2e and $ded6.
// Sets $de34 and $de35 to block NoiseSfx06, Sfx01 and Sfx18.
static void sfx0B_Start(void)
{
    if (g_sfxBlockAll)
        return;
    if (g_sfxBlockSfx0B)
        return;
    g_sfxSq1Id = 0x07;
    g_sfxBlockNoise06 = 0x07;
    g_sfxBlockSfx01 = 0x07;
    g_sfxBlockSfx0A = 0x07;
    g_sfxBlockNoise09 = 0x07;
    g_soundFadeState = 0;
    g_noiseSfxId = 0;
    g_sfxBlockSfx17B = 0;
    sfxClearTimers();
    musicMuteCh1();
    musicMuteCh4();
    sfxSetSq1Ptr(Sfx0B_Start_Sq1Regs);
    sfxWriteRegs5(Sfx0B_Start_Sq1Regs);
    sfxWriteRegs4(&rNR41, Sfx0B_Start_NoiseRegs);
    g_sfxTimer1 = 0x05;
    g_sfxSteps1 = 0x05;
    g_sfxReload1 = 0x01;
    g_sfx07Repeats = 0x01;
}

// Start (or restart) the 11-step square 1 effect at Sfx32_Update_Sq1Regs.
// $dea1 is set to 7 so Sfx07_Update steps it.
static void sfx32_Update(void)
{
    g_sfxSq1Id = 0x07;
    g_sfxBlockSfx0D = 0x07;
    g_sfxBlockSfx0A = 0;
    sfxSetFlagC3Only(0);
    g_sfxBlockSfx17B = 0;
    musicMuteCh1();
    sfxSetSq1Ptr(Sfx32_Update_Sq1Regs);
    sfxWriteRegs5(Sfx32_Update_Sq1Regs);
    sfxClearPhaseFlags();
    g_sfxTimer1 = 0x01;
    g_sfxReload1 = 0x01;
    g_sfxSteps1 = 0x0b;
}

// Starts the Sfx32 effect twice ($de2c = 2) and blocks other effects.
// Does nothing while $dee1 (pause) is set.
static void sfx0C_Start(void)
{
    if (g_sfxPauseLock)
        return;
    noiseSfxStop();
    g_sfxBlockSfx17 = 0x03;
    g_sfxBlockSfx03 = 0x03;
    g_sfxBlockSfx0D = 0x03;
    g_sfx07Repeats = 0x02;
    g_sfxBlockAll = 0x02;
    sfx32_Update();
}

// Write the Sfx11 square 1 and noise entries and wait 6 frames.
// It is also the second part of Sfx0D.
static void sfx11_Play(void)
{
    g_sfxSq1Id = 0x11;
    sfxWriteRegs5(Sfx11_Play_Sq1Regs);
    sfxWriteRegs4(&rNR41, Sfx11_Play_NoiseRegs);
    sfxClearTimers();
    g_sfxTimer1 = 0x06;
}

// Square 1 + noise effect. Blocked by $de39 and $ded9.
static void sfx0D_Start(void)
{
    if (g_sfx04Busy)
        return;
    if (g_sfxBlockSfx0D)
        return;
    sfxClearTimers();
    sfxSetFlag2AAndId(0x0d);
    g_sfxBlockSfx17B = 0x0d;
    g_sfxBlockCue1 = 0x0d;
    g_sfxFlagC6 = 0x0d;
    g_sfxBlockSfx0B = 0x0d;
    noiseSfxSetId(0x0d);
    g_soundFadeState = 0;
    rNR10 = 0;
    g_sfxFlag33 = 0;
    musicMuteCh1();
    musicMuteCh4();
    sfxWriteRegs5(Sfx0D_Start_Sq1Regs);
    sfxWriteRegs4(&rNR41, Sfx0D_Start_NoiseRegs);
    sfxClearTimers();
    g_sfxTimer1 = 0x06;
}

// After 6 frames write the second entry; after 6 more, play Sfx11.
static void sfx0D_Update(void)
{
    if (!g_sfxPhase1) {
        if (sfxTickSq1Timer())
            return;
        sfxWriteRegs5(Sfx0D_Update_Sq1Regs);
        sfxWriteRegs4(&rNR41, Sfx0D_Update_NoiseRegs);
        g_sfxTimer1 = 0x06;
        g_sfxPhase1 = 1;
        return;
    }
    if (sfxTickSq1Timer())
        return;
    sfx11_Play();
}

// Two-channel effect (square 1 + square 2), 2 steps of 7 frames.
static void sfx0F_Start(void)
{
    g_sfxFlag29 = 0x06;
    sfxSetFlagD4AndId(0x06);
    noiseSfxStop();
    musicMuteCh1();
    musicMuteCh2();
    sfxSetSq1Ptr(Sfx0F_Start_Sq1Regs);
    sfxWriteRegs5(Sfx0F_Start_Sq1Regs);
    sfxSetSq2Ptr(Sfx0F_Start_Sq2Regs);
    sfxWriteRegs4(&rNR21, Sfx0F_Start_Sq2Regs);
    // SfxSetSq2Timers: only the phase flags change (see Sfx02).
    sfxClearPhaseFlags();
    sfxSetAllTimers(0x07);
    sfxSetStepCounts(0x02);
    g_sfxSq2Active = 1;
}

// Square 1 + noise effect, 1 step of 8 frames.
static void sfx10_Start(void)
{
    g_sfxSq1Id = 0x06;
    musicMuteCh1();
    musicMuteCh4();
    sfxSetSq1Ptr(Sfx10_Start_Sq1Regs);
    sfxWriteRegs5(Sfx10_Start_Sq1Regs);
    sfxSetSq2Ptr(Sfx10_Start_NoiseRegs);
    sfxWriteRegs4(&rNR41, Sfx10_Start_NoiseRegs);
    sfxClearPhaseFlags();
    g_sfxTimer1 = 0x08;
    g_sfxReload1 = 0x0c;
    g_sfxSteps1 = 0x01;
}

// Square 1 + noise effect. Mutes the channels, then plays Sfx11_Play.
static void sfx11_Start(void)
{
    musicMuteCh1();
    musicMuteCh4();
    sfx11_Play();
}

// After 6 frames write the second square 1 entry; after 9 more frames give
// channels 1 and 4 back and clear the busy flags.
static void sfx11_Update(void)
{
    if (!g_sfxPhase1) {
        if (sfxTickSq1Timer())
            return;
        sfxWriteRegs5(Sfx11_Update_Sq1Regs);
        g_sfxTimer1 = 0x09;
        g_sfxPhase1 = 1;
        return;
    }
    if (sfxTickSq1Timer())
        return;
    musicUnmuteCh1();
    musicUnmuteCh4();
    g_sfxBlockCue1 = 0;
    g_sfxBlockSfx0B = 0;
    g_sfxBlockSfx17B = 0;
    sfxSetFlag2AAndC3(0);
    noiseSfxSetIdAnd29(0);
    sfxSetSweepAndId(0);
}

// Noise effect that uses the square 2 step timers. Plays as noise SFX 7.
static void sfx13_Start(void)
{
    g_noiseSfxId = 0x07;
    g_sfxBlockNoise0D = 0x07;
    sfxSetIdAndSweep(0);
    rNR12 = 0;
    sfxClearTimers();
    musicMuteCh4();
    sfxSetSq2Ptr(Sfx13_Start_NoiseRegs);
    sfxWriteRegs4(&rNR41, Sfx13_Start_NoiseRegs);
    g_sfxTimer2 = 0x04;
    g_sfxReload1 = 0x07;
    g_sfxReload2 = 0x07;
    g_sfxSteps2 = 0x01;
}

// Write one square 1 entry and one square 2 entry. No update.
// The asm stores the square 2 data in the square 1 pointer (a bug).
static void sfx14_Start(void)
{
    sfxSetSq1Ptr(Sfx14_Start_Sq1Regs);
    sfxWriteRegs5(Sfx14_Start_Sq1Regs);
    sfxSetSq1Ptr(Sfx14_Start_Sq2Regs);
    sfxWriteRegs4(&rNR21, Sfx14_Start_Sq2Regs);
    musicUnmuteCh1();
}

// Square 1 effect, 3 steps of 7 frames.
static void sfx15_Start(void)
{
    sfxSetIdAndFlagD4(0x06);
    sfxClearTimers();
    noiseSfxStop();
    musicMuteCh1();
    sfxSetSq1Ptr(Sfx15_Start_Sq1Regs);
    sfxWriteRegs5(Sfx15_Start_Sq1Regs);
    g_sfxTimer1 = 0x07;
    g_sfxReload1 = 0x07;
    g_sfxSteps1 = 0x03;
}

// Square 1 effect, 2 steps. Blocked by $dec3, $ded7 and $dee0.
static void sfx17_Start(void)
{
    if (g_sfxSq1Busy)
        return;
    if (g_sfxBlockSfx17)
        return;
    if (g_sfxBlockSfx17B)
        return;
    g_sfxSq1Id = 0x06;
    g_sfxBlockNoise09 = 0x06;
    noiseSfxStop();
    musicMuteCh1();
    sfxSetSq1Ptr(Sfx17_Start_Sq1Regs);
    sfxWriteRegs5(Sfx17_Start_Sq1Regs);
    sfxClearTimers();
    g_sfxTimer1 = 0x03;
    g_sfxReload1 = 0x08;
    g_sfxSteps1 = 0x02;
}

// Write one square 1 entry and one square 2 entry. No update.
static void sfx19_Start(void)
{
    sfxSetSq1Ptr(Sfx19_Start_Sq1Regs);
    sfxWriteRegs5(Sfx19_Start_Sq1Regs);
    sfxSetSq2Ptr(Sfx19_Start_Sq2Regs);
    sfxWriteRegs4(&rNR21, Sfx19_Start_Sq2Regs);
}

// Square 1 effect, 2 steps. Blocked by $de35.
static void sfx18_Start(void)
{
    if (g_sfxBlockSfx01)
        return;
    sfxSetFlag2AAndId(0x06);
    sfxClearTimers();
    musicMuteCh1();
    sfxSetSq1Ptr(Sfx18_Start_Sq1Regs);
    sfxWriteRegs5(Sfx18_Start_Sq1Regs);
    g_sfxTimer1 = 0x02;
    g_sfxReload1 = 0x06;
    g_sfxSteps1 = 0x02;
}

// Two-channel effect, 1 step of 7 frames. Sets $dede to block noise SFX $0D/$0E.
static void sfx1A_Start(void)
{
    g_sfxFlag29 = 0x06;
    g_sfx1ABusy = 0x06;
    sfxSetFlag2AAndId(0x06);
    g_noiseSfxId = 0;
    rNR42 = 0;
    sfxClearTimers();
    musicMuteCh1();
    musicMuteCh2();
    sfxSetSq1Ptr(Sfx1A_Start_Sq1Regs);
    sfxWriteRegs5(Sfx1A_Start_Sq1Regs);
    sfxSetSq2Ptr(Sfx1A_Start_Sq2Regs);
    sfxWriteRegs4(&rNR21, Sfx1A_Start_Sq2Regs);
    sfxSetAllTimers(0x07);
    sfxSetStepCounts(0x01);
    g_sfxSq2Active = 1;
}

// Two-channel effect, 3 steps of 5 frames.
static void sfx1B_Start(void)
{
    g_sfxSq1Id = 0x06;
    noiseSfxStop();
    g_soundFadeState = 0; // the asm stores A = 0 from NoiseSfxStop
    sfxClearTimers();
    musicMuteCh1();
    musicMuteCh2();
    sfxSetSq1Ptr(Sfx1B_Start_Sq1Regs);
    sfxWriteRegs5(Sfx1B_Start_Sq1Regs);
    sfxSetSq2Ptr(Sfx1B_Start_Sq2Regs);
    sfxWriteRegs4(&rNR21, Sfx1B_Start_Sq2Regs);
    sfxSetAllTimers(0x05);
    sfxSetStepCounts(0x03);
    g_sfxSq2Active = 1;
}

// Two falling pitch sweeps on square 1. Blocked by $de2e.
static void sfx1C_Start(void)
{
    if (g_sfxBlockAll)
        return;
    g_sfxBlockSfx0B = 0x1c;
    g_sfxBlockSfx0D = 0x1c;
    sfxSetIdAndFlags(0x1c);
    g_sfxNoiseBusy = 0;
    g_sfxBlockSfx17B = 0;
    musicMuteCh1();
    musicUnmuteCh4();
    g_sfx1CPhase = 0; // SoundClearDec4
    sfxClearTimers();
    g_sfxSweepFreqLo = 0xff;
    g_sfxSweepFreqHi = 0xc5;
    g_sfx1CSweepSpeed = 0x04;
    g_sfx1CEnvelope = 0xff;
}

// Run one frame of the Sfx1C sweep. First sweep: $c5ff down to $c590.
// Second sweep: $c590 down to $c500. Then stop and give channel 1 back.
static void sfx1C_Update(void)
{
    g_sfxSweepSubsteps = g_sfx1CSweepSpeed;
    rNR10 = 0;
    rNR11 = 0x7c;
    rNR12 = g_sfx1CEnvelope;
    do {
        uint8_t lo = g_sfxSweepFreqLo - 1;
        if (lo == (g_sfx1CPhase ? 0x00 : 0x90)) {
            // .sweepEnd
            if (!g_sfx1CPhase) {
                g_sfxSweepFreqLo = 0x90;
                g_sfxSweepFreqHi = 0xc5;
                g_sfx1CPhase = 0x01;
                g_sfx1CSweepSpeed = 0x01;
                g_sfx1CEnvelope = 0xff;
                return;
            }
            rNR12 = 0;
            g_sfxBlockCue1 = 0;
            g_sfxNoiseBusy = 0;
            g_sfxBlockSfx0B = 0;
            g_sfxBlockSfx0D = 0;
            sfxSetIdAndFlags(0);
            musicUnmuteCh1();
            return;
        }
        g_sfxSweepFreqLo = lo;
    } while (--g_sfxSweepSubsteps);
    rNR13 = g_sfxSweepFreqLo;
    rNR14 = g_sfxSweepFreqHi;
}

// Two-channel effect, 3 steps of 8 frames.
static void sfx1D_Start(void)
{
    g_sfxSq1Id = 0x06;
    musicMuteCh1();
    musicMuteCh2();
    sfxSetSq1Ptr(Sfx1D_Start_Sq1Regs);
    sfxWriteRegs5(Sfx1D_Start_Sq1Regs);
    sfxSetSq2Ptr(Sfx1D_Start_Sq2Regs);
    sfxWriteRegs4(&rNR21, Sfx1D_Start_Sq2Regs);
    // SfxSetSq2Timers: only the phase flags change (see Sfx02).
    sfxClearPhaseFlags();
    sfxSetAllTimers(0x08);
    sfxSetStepCounts(0x03);
    g_sfxSq2Active = 1;
}

// Square 1 one-shot, 15 frames.
static void sfx21_Start(void)
{
    sfxSetIdAndFlagD4(0x21);
    musicMuteCh1();
    sfxClearTimers();
    g_soundFadeState = 0;
    sfxWriteRegs5(Sfx21_Start_Sq1Regs);
    g_sfxTimer1 = 0x0f;
}

// Wait for timer 1, then clear the SFX and give channel 1 back.
static void sfx21_Update(void)
{
    if (sfxTickSq1Timer())
        return;
    sfxSetIdAndSweep(0);
    g_sfxBlockNoise0D = 0;
    musicUnmuteCh1();
}

// Square channel sound effects. A new request in SFX_Queue ($dea0) runs
// SfxNN_Start. Otherwise the active effect in $dea1 gets its SfxNN_Update.
// Request 7 restores the volume. Requests $09, $0e, $12 and $1e-$20 do nothing.
void sfxUpdate(void)
{
    switch (SFX_Queue) {
    case 0x01: sfx01_Start(); return;
    case 0x02: sfx02_Start(); return;
    case 0x03: sfx03_Start(); return;
    case 0x04: sfx04_Start(); return;
    case 0x05: sfx05_Start(); return;
    case 0x06: sfx06_Start(); return;
    case 0x07: soundRestoreVolume(); return;
    case 0x08: sfx08_Start(); return;
    case 0x0a: sfx0A_Start(); return;
    case 0x0b: sfx0B_Start(); return;
    case 0x0c: sfx0C_Start(); return;
    case 0x0d: sfx0D_Start(); return;
    case 0x0f: sfx0F_Start(); return;
    case 0x10: sfx10_Start(); return;
    case 0x11: sfx11_Start(); return;
    case 0x13: sfx13_Start(); return;
    case 0x14: sfx14_Start(); return;
    case 0x15: sfx15_Start(); return;
    case 0x16: return; // Sfx16_Start is empty
    case 0x17: sfx17_Start(); return;
    case 0x18: sfx18_Start(); return;
    case 0x19: sfx19_Start(); return;
    case 0x1a: sfx1A_Start(); return;
    case 0x1b: sfx1B_Start(); return;
    case 0x1c: sfx1C_Start(); return;
    case 0x1d: sfx1D_Start(); return;
    case 0x21: sfx21_Start(); return;
    }
    switch (g_sfxSq1Id) {
    case 0x01: sfx01_Update(); break;
    case 0x03: sfx03_Update(); break;
    case 0x06: sfx06_Update(); break;
    case 0x07: sfx07_Update(); break;
    case 0x0a: sfx0A_Update(); break;
    case 0x32: sfx32_Update(); break;
    case 0x0d: sfx0D_Update(); break;
    case 0x11: sfx11_Update(); break;
    case 0x1c: sfx1C_Update(); break;
    case 0x21: sfx21_Update(); break;
    }
}

// The bank 12 part of SoundUpdate: SFX, fades, commands and noise SFX.
// One banked call from bank 4 runs all four.
void soundUpdateEffects(void) __banked
{
    sfxUpdate();
    soundUpdateFade();
    soundCmdUpdate();
    noiseSfxUpdate();
}
