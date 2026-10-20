// Fades, the SoundCue1 effect and the sound commands (pause, resume, fades).
#include "game.h"
#include "sound.h"

extern const uint8_t FadeInSteps[], FadeOutSteps[], FadeOutFastSteps[];
extern const uint8_t SoundCue1_Start_Sq1Regs[], SoundCue1_Update_Sq1Regs[];
extern const uint8_t SoundCue1_Start_NoiseRegs[];

// Write the next byte of the fade table to NR50 and advance the fade
// step pointer ($decf/$ded0, high byte first).
static void soundWriteFadeStep(void)
{
    const uint8_t *p = (const uint8_t *)((g_soundFadePtrHi << 8) | g_soundFadePtrLo);
    rNR50 = *p++;
    g_soundFadePtrHi = (uint16_t)p >> 8;
    g_soundFadePtrLo = (uint16_t)p;
}

// Start a fade: state 4, first wait, step count and table. Writes the first step.
static void soundFadeStart(uint8_t timer, uint8_t steps, const uint8_t *table)
{
    g_soundFadeState = 0x04;
    sfxClearTimers();
    g_soundFadeTimer = timer;
    g_soundFadeSteps = steps;
    g_soundFadePtrHi = (uint16_t)table >> 8;
    g_soundFadePtrLo = (uint16_t)table;
    soundWriteFadeStep();
}

// Start a fade out through FadeOutSteps, then stop the music.
// The last step reads the first byte of FadeOutFastSteps.
static void soundCmd_FadeOut(void)
{
    g_soundFadeStopMusic = 0x04;
    soundFadeStart(0x01, 0x07, FadeOutSteps);
}

// Same as SoundCmd_FadeOut with FadeOutFastSteps (4 steps).
// The last step reads the byte after the table. NR50 = $77 overwrites it at once.
static void soundCmd_FadeOutFast(void)
{
    g_soundFadeStopMusic = 0x04;
    soundFadeStart(0x01, 0x04, FadeOutFastSteps);
}

// Start a fade in through FadeInSteps. The music keeps playing.
static void soundFadeIn_Start(void)
{
    soundFadeStart(0x30, 0x08, FadeInSteps);
}

// Clear the fade state ($dec9) and $de2a.
static void soundFadeEnd(void)
{
    g_soundFadeState = 0;
    g_sfxBlockSfx0A = 0;
}

// Set NR50 to full volume ($77) and clear $deca. Also SFX request 7.
void soundRestoreVolume(void)
{
    rNR50 = 0x77;
    g_soundFadeStopMusic = 0;
    soundFadeEnd();
}

// Every $30 frames, write the next fade step to NR50. After the last
// step, stop the music if $deca is set and restore NR50 to $77.
static void soundFadeUpdate(void)
{
    if (--g_soundFadeTimer)
        return;
    soundWriteFadeStep();
    g_soundFadeTimer = 0x30;
    if (--g_soundFadeSteps)
        return;
    if (!g_soundFadeStopMusic) {
        soundFadeEnd();
        return;
    }
    musicStop();
    soundRestoreVolume();
}

// Start the effect requested by $dec8 = 1 (square 1 and noise).
static void soundCue1_Start(void)
{
    if (g_sfx04Busy)
        return;
    if (g_sfxNoiseBusy)
        return;
    if (g_sfxBlockCue1)
        return;
    if (g_sfxBlockAll) {
        sfxSq2Update();
        return;
    }
    g_soundFadeState = 0x09;
    g_sfxBlockSfx0A = 0x09;
    sfxSetFlagC3(0x09);
    g_sfxSq1Id = 0;
    g_noiseSfxId = 0;
    g_sfxBlockSfx17B = 0;
    musicMuteCh1();
    musicMuteCh4();
    // $dec3 was just set to 9, so the asm never writes square 1 here.
    if (!g_sfxSq1Busy)
        sfxWriteRegs5(SoundCue1_Start_Sq1Regs);
    sfxWriteRegs4(&rNR41, SoundCue1_Start_NoiseRegs);
    sfxClearTimers();
    g_sfxTimer1 = 0x04;
}

// SoundCue1 has two parts. Part 1 waits for $de1b, then writes the second
// square 1 entry. Part 2 waits again, then gives channels 1 and 4 back.
static void soundCue1_Update(void)
{
    if (!g_sfxPhase1) {
        if (sfxTickSq1Timer())
            return;
        sfxWriteRegs5(SoundCue1_Update_Sq1Regs);
        g_sfxTimer1 = 0x19;
        g_sfxPhase1 = 1;
        return;
    }
    if (sfxTickSq1Timer())
        return;
    musicUnmuteCh1();
    musicUnmuteCh4();
    rNR10 = 0;
    g_soundFadeState = 0;
    g_sfxBlockSfx0B = 0;
    g_sfxBlockSfx0D = 0;
    g_sfxBlockSfx0A = 0;
    sfxSetFlagC3(0);
}

// Per-frame fade handler. $dec8 = 1 starts SoundCue1, 4 starts a fade in.
// $dec9 holds the running effect: 4 = fade, 9 = SoundCue1.
void soundUpdateFade(void)
{
    if (g_sfxBlockAll) {
        noiseSfxStop();
        return;
    }
    uint8_t cue = g_soundCueRequest;
    if (cue == 0x01) {
        soundCue1_Start();
        return;
    }
    if (cue == 0x02)
        return;
    if (cue == 0x04) {
        soundFadeIn_Start();
        return;
    }
    if (g_soundFadeState == 0x09)
        soundCue1_Update();
    else if (g_soundFadeState == 0x04)
        soundFadeUpdate();
}

// Pause: set the SFX busy flags, silence the channels and set $dec7.
static void soundCmd_Pause(void)
{
    g_sfxPauseLock = 0x01;
    g_sfxBlockAll = 0x01;
    g_sfxBlockSfx03 = 0x01;
    g_sfxBlockSfx0B = 0x01;
    g_sfx04Busy = 0x01;
    g_sfxNoiseBusy = 0x01;
    g_sfxBlockCue1 = 0x01;
    g_sfxBlockNoise09 = 0x01;
    g_sfxBlockSfx0D = 0x01;
    g_sfxBlockSfx0A = 0x01;
    g_sfxSq1Id = 0;
    g_noiseSfxId = 0;
    soundResetChannels();
    g_soundPaused = 0x01;
}

// Resume: clear the SFX busy flags and $dec7.
static void soundCmd_Resume(void)
{
    g_sfxPauseLock = 0;
    g_sfxBlockAll = 0;
    g_sfxBlockSfx03 = 0;
    g_sfxBlockSfx0B = 0;
    g_sfx04Busy = 0;
    g_sfxNoiseBusy = 0;
    g_sfxBlockCue1 = 0;
    g_sfxBlockNoise09 = 0;
    g_sfxBlockSfx0D = 0;
    g_sfxBlockSfx0A = 0;
    g_soundPaused = 0;
}

// Handle the command in $deb0: 1 = pause, 2 = resume, 3 = fade out,
// 5 = fast fade out, $1e = give channel 4 back and play SFX $0A.
void soundCmdUpdate(void)
{
    switch (SoundCmd_Queue) {
    case 0x01: soundCmd_Pause(); break;
    case 0x02: soundCmd_Resume(); break;
    case 0x03: soundCmd_FadeOut(); break;
    case 0x05: soundCmd_FadeOutFast(); break;
    case 0x1e: // SoundCmd_1E
        musicUnmuteCh4();
        sfx0A_Start();
        break;
    }
}
