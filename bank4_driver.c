// Sound driver entry point, song start and stop, and the song 5 save state.
#include "game.h"
#include "sound.h"

extern const uint8_t *const Music_SongTable[];
extern const uint8_t Music_SongPanTable[];

// Wave RAM ($ff30-$ff3f).
static volatile __at(0xff30) uint8_t waveRam[16];

// Count frames in $decd. Every 12 frames, increment $dece.
// Other banks read $dece as a slow timer.
static void soundTickClock(void)
{
    if (++g_soundFrameCount != 0x0c)
        return;
    g_soundClock++;
    g_soundFrameCount = 0;
}

// Copy 16 bytes from p to wave RAM ($ff30-$ff3f).
void musicLoadWaveRam(const uint8_t *p)
{
    for (uint8_t i = 0; i != 16; i++)
        waveRam[i] = p[i];
}

// Silence square 1, square 2 and noise (envelope $08, restart) and turn
// off the wave channel.
void soundResetChannels(void) __banked
{
    rNR12 = 0x08;
    rNR22 = 0x08;
    rNR42 = 0x08;
    rNR14 = 0x80;
    rNR24 = 0x80;
    rNR44 = 0x80;
    rNR10 = 0;
    rNR30 = 0;
}

// Clear the SFX IDs, the channel mute flags and $de4c-$de4f, then reset
// the channels.
void musicSilence(void)
{
    g_sfxSq1Id = 0;
    g_soundCmdB1 = 0;
    g_noiseSfxId = 0;
    g_musicCh1Flags = 0;
    g_musicCh2Flags = 0;
    g_musicCh3Flags = 0;
    g_musicCh4Flags = 0;
    g_musicFlag4e = 0;
    g_musicFlag4f = 0;
    g_musicFlag4c = 0;
    g_musicKeepInstrument = 0;
    soundResetChannels();
}

// Stop the music: NR51 = $ff, clear the current song ($dea9) and the
// channel state.
void musicStop(void) __banked
{
    rNR51 = 0xff;
    g_musicPanMode = 0x03;
    g_musicSongId = 0;
    musicSilence();
}

// Copy the music state ($de50-$dea9) to $def0 so the song can resume later.
// The asm increments only the low byte of the destination. So $de50-$de5f
// goes to $def0-$deff, and $de60-$dea9 goes to $de00-$de49.
static void musicSaveState(void)
{
    const uint8_t *src = &g_musicCh1;
    uint8_t *page = &g_musicSaveState - 0xf0; // $de00
    uint8_t e = 0xf0;
    do
        page[e] = *src++;
    while (++e != 0x4a);
    g_musicSavedSongId = 0x05;
}

// Copy the saved music state back from $def0-$df49 to $de50-$dea9 and
// restore the song ID.
static void musicRestoreState(void)
{
    const uint8_t *src = &g_musicSaveState;
    uint8_t *dst = &g_musicCh1;
    for (uint8_t i = 0; i != 0x5a; i++)
        dst[i] = src[i];
    g_musicSongId = g_musicSavedSongId;
}

// Load the stereo settings for the song in $dea9 from Music_SongPanTable.
static void musicLoadSongPan(void)
{
    const uint8_t *p = Music_SongPanTable + ((uint16_t)(uint8_t)(g_musicSongId - 1) << 2);
    g_musicPanMode = p[0];
    g_musicPanPeriod = p[1];
    g_musicPanNR51A = p[2];
    rNR51 = p[2];
    g_musicPanNR51B = p[3];
    g_musicPanCounter = 0;
    g_musicPanToggle = 0;
}

// Set up a song from its header: transpose, note length table and the
// four channel list pointers. Then load each channel's first pattern.
static void musicInitSong(const uint8_t *h)
{
    musicSilence();
    g_musicTranspose = h[0];
    WORD(g_musicNoteLenPtr) = h[1] | (h[2] << 8);
    MusicChannel *ch = (MusicChannel *)&g_musicCh1;
    h += 3;
    for (uint8_t i = 0; i != 4; i++, h += 2)
        ch[i].list = (const uint8_t *)(h[0] | (h[1] << 8));
    // MusicLoadFirstPattern for each channel.
    for (uint8_t i = 0; i != 4; i++)
        ch[i].pattern = *(const uint8_t *const *)ch[i].list;
    for (uint8_t i = 0; i != 4; i++)
        ch[i].timer = 1;
    g_musicCh1VibratoStep = 0;
    g_musicCh2VibratoStep = 0;
    g_musicCh3VibratoStep = 0;
    g_musicKeepInstrument = 0;
    g_musicFlag4c = 0;
}

// Start the song requested in BGM_Queue ($dea8). $ff stops the music.
// IDs of $30 or more are ignored. Songs 7 and $0c save song 5 when they
// interrupt it. A request for song 5 during song 7 or $0c resumes the saved song.
static void musicStartQueued(void)
{
    uint8_t prev = g_musicSongId;
    uint8_t id = BGM_Queue;
    if (!id)
        return;
    if (id == 0xff) {
        musicStop();
        return;
    }
    if (id >= 0x30)
        return;
    g_musicSongId = id;
    if (id == 0x05) {
        // MusicStart_CheckResume: restore the saved song 5 and its wave samples.
        if (prev == 0x07 || prev == 0x0c) {
            musicRestoreState();
            rNR30 = 0;
            musicLoadWaveRam((const uint8_t *)WORD(g_musicCh3WavePtr));
            soundResetChannels();
            return;
        }
    } else if (id == 0x07 || id == 0x0c) {
        // MusicStart_CheckSave
        if (prev == 0x05)
            musicSaveState();
    }
    // MusicStart_Play: look up the song header (ID & $1f) and start the song.
    // The asm index is a byte offset ((ID & $1f) - 1) * 2, so it wraps at 256.
    musicInitSong(Music_SongTable[(uint8_t)((id & 0x1f) - 1) & 0x7f]);
    musicLoadSongPan();
}

// Sound driver entry point. Bank 0 calls it once per frame with bank 4 paged in.
// Runs the fade, sound effect, command, noise and music updates, then
// clears all request queues. Music is skipped while the driver is paused ($dec7).
void soundUpdate(void)
{
    soundTickClock();
    soundUpdateEffects();
    if (!g_soundPaused) {
        musicStartQueued();
        musicUpdate();
    }
    SFX_Queue = 0;
    BGM_Queue = 0;
    SoundCmd_Queue = 0;
    NoiseSfx_Queue = 0;
    g_soundQueue21 = 0;
    g_soundCueRequest = 0;
}
