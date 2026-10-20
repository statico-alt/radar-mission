// Sound driver: functions that one sound file calls in another.
// Bank 4 has the driver entry and the music engine. Bank 12 has the SFX,
// the fades and the commands. A call to the other bank is __banked.

// One music channel struct ($de50, $de60, $de70, $de80; 16 bytes).
typedef struct {
    const uint8_t *list;    // +0 channel list pointer
    uint8_t timer;          // +2 frames left in the note
    uint8_t length;         // +3 note length
    const uint8_t *pattern; // +4 pattern read pointer
    uint8_t env;            // +6 instrument byte 1: envelope (NRx2). Ch3: wave pointer low.
    uint8_t inst7;          // +7 instrument byte 2. Ch3: wave pointer high.
    uint8_t duty;           // +8 instrument byte 3: duty (NRx1), low nibble = vibrato. Ch3: volume.
    uint16_t freq;          // +9 note frequency (NRx3/NRx4)
    uint8_t rest;           // +11 1 = rest
    const uint8_t *repeat;  // +12 repeat address
    uint8_t vibStep;        // +14 vibrato step
    uint8_t flags;          // +15 bit 7 = muted for an SFX, bits 0-6 = repeat count
} MusicChannel;

// bank4_driver.c
void musicStop(void) __banked;
void musicSilence(void);
void soundResetChannels(void) __banked;
void musicLoadWaveRam(const uint8_t *p);

// bank4_music.c
void musicUpdate(void);

// bank12_sfx.c
void sfxUpdate(void);
void soundUpdateEffects(void) __banked;
void sfxClearTimers(void);
void sfxClearPhaseFlags(void);
void sfxSetFlagC3(uint8_t a);
void sfxSetFlagD4AndC3(uint8_t a);
void sfxSetIdAndSweep(uint8_t a);
void noiseSfxSetIdAnd29(uint8_t a);
void noiseSfxSetId(uint8_t a);
uint8_t sfxTickSq1Timer(void);
uint8_t sfxTickSq2Timer(void);
void noiseSfxStop(void);
void sfxSetSq2Ptr(const uint8_t *p);
const uint8_t *sfxGetSq2Ptr(void);
void musicMuteCh1(void);
void musicMuteCh4(void);
void musicUnmuteCh1(void);
void musicUnmuteCh4(void);
void sfxWriteRegs5(const uint8_t *p);
void sfxWriteRegs4(volatile uint8_t *reg, const uint8_t *p);
void sfxStopAll(void);
void sfxSq2Update(void);
void sfx0A_Start(void);

// bank12_noise.c
void noiseSfxUpdate(void);

// bank12_fade.c
void soundUpdateFade(void);
void soundCmdUpdate(void);
void soundRestoreVolume(void);

// SDCC 4.5.1 writes a bad address for some `ldh` stores to an __at register
// (for example `x = a; rNR10 = a;`). A store through a constant pointer
// does not have this bug.
#define rNR10_PTR (*(volatile uint8_t *)0xff10)
#define rNR51_PTR (*(volatile uint8_t *)0xff25)
