// Music engine. Each channel has a 16-byte struct at $de50/$de60/$de70/$de80
// (MusicChannel in sound.h). $de90 = channel number 1-4.
#include "game.h"
#include "sound.h"

extern const uint8_t Music_FreqTable[];
extern const uint8_t Music_NoiseTable[];
extern const uint8_t Music_VibratoTable[];

// If the song uses a moving stereo pan ($de94 != 1), count frames in
// $de95. Every $de96 frames, toggle between the two NR51 values.
static void musicUpdatePan(void)
{
    if (g_musicPanMode == 0x01)
        return;
    if (++g_musicPanCounter != g_musicPanPeriod)
        return;
    g_musicPanCounter = 0;
    g_musicPanToggle++;
    rNR51_PTR = (g_musicPanToggle & 1) ? g_musicPanNR51B : g_musicPanNR51A;
}

// Add a vibrato offset from Music_VibratoTable to the note frequency
// and write it to the channel's frequency registers.
// The step can be up to 255, so the read can go past the table.
static void musicApplyVibrato(MusicChannel *ch)
{
    uint8_t mode = ch->duty & 0x0f;
    if (!mode)
        return;
    g_musicVibratoMode = mode;
    uint8_t n = g_musicChannelNum;
    if (n == 4)
        return;
    // Each table byte holds two signed 4-bit offsets. An even step uses the
    // high nibble. An odd step uses the low nibble.
    uint8_t step = ch->vibStep;
    uint8_t v = Music_VibratoTable[step >> 1];
    if (!(step & 1))
        v = SWAP(v);
    v &= 0x0f;
    if (v & 0x08)
        v |= 0xf0;
    uint16_t freq = ch->freq + (int8_t)v;
    volatile uint8_t *reg = &rNR33;
    if (n == 1)
        reg = &rNR13;
    else if (n == 2)
        reg = &rNR23;
    reg[0] = freq;
    reg[1] = freq >> 8;
}

// The current note is still playing. Apply vibrato if the channel has it.
static void musicHoldNote(MusicChannel *ch)
{
    if (g_musicChannelNum == 3 && (g_musicCh3Volume & 0x80) && ch->timer == 0x06)
        rNR32 = 0x40; // wave volume 50%
    if (ch->rest)
        return;
    if (ch->flags & 0x80)
        return;
    musicApplyVibrato(ch);
}

// End of a pattern: move to the next entry in the channel list.
// High byte $00 stops the song. High byte $ff jumps: the list pointer
// takes the word after the entry. Returns 0 when the song stops.
static uint8_t musicNextPattern(MusicChannel *ch)
{
    ch->list += 2;
    for (;;) {
        // MusicLoadFirstPattern
        ch->pattern = *(const uint8_t *const *)ch->list;
        uint8_t hi = (uint16_t)ch->pattern >> 8;
        if (hi == 0x00) {
            g_musicSongId = 0;
            musicStop();
            return 0;
        }
        if (hi != 0xff)
            return 1;
        ch->list = *(const uint8_t *const *)(ch->list + 2);
    }
}

// Command $9d at p: read 3 instrument bytes. Store them at +6/+7/+8 unless
// $de4d is set. On the wave channel, bytes 1-2 are the wave sample pointer.
// Returns the pointer after the command.
static const uint8_t *musicCmd_SetInstrument(MusicChannel *ch, const uint8_t *p)
{
    uint8_t e = p[1];
    uint8_t d = p[2];
    if (!g_musicKeepInstrument) {
        ch->env = e;
        ch->inst7 = d;
        ch->duty = p[3];
    }
    if (g_musicChannelNum == 3) {
        // MusicCmd_SetWave
        rNR30 = 0;
        musicLoadWaveRam((const uint8_t *)(e | (d << 8)));
    }
    return p + 4;
}

// Write the note to the channel's 4 registers from reg and trigger it,
// unless the channel is muted for an SFX (flags bit 7).
// d = NRx1 value, e = NRx2 value. A rest writes NRx2 = $08.
static void musicWriteNoteRegs(MusicChannel *ch, volatile uint8_t *reg, uint8_t d, uint8_t e)
{
    if (ch->rest)
        e = 0x08;
    ch->vibStep = 0;
    if (ch->flags & 0x80)
        return;
    reg[0] = d;
    reg[1] = e;
    reg[2] = ch->freq;
    reg[3] = (ch->freq >> 8) | 0x80;
}

// Write the note frequency and envelope to the channel's registers and trigger it.
// The asm skips the envelope (+6) when +7 is not zero. All instruments
// in the song data have +7 = 0, so this code does not check +7.
static void musicPlayNote(MusicChannel *ch)
{
    uint8_t n = g_musicChannelNum;
    uint8_t d = ch->duty;
    uint8_t e = ch->env;
    volatile uint8_t *reg = &rNR41;
    if (n == 1) {
        reg = &rNR11;
    } else if (n == 2) {
        reg = &rNR21;
    } else if (n == 3) {
        if (!(g_musicCh3Flags & 0x80)) {
            rNR30 = 0;
            rNR30 = 0x80;
        }
        // The wave channel writes NR31 = 0 and NR32 = the +8 byte.
        reg = &rNR31;
        e = d;
        d = 0;
    }
    musicWriteNoteRegs(ch, reg, d, e);
}

// Store note b in the channel. Noise copies the note's 5 bytes from
// Music_NoiseTable to $de86-$de8a. $01 is a rest. Other notes are a byte
// offset into Music_FreqTable, plus the transpose.
static void musicSetNote(MusicChannel *ch, uint8_t b)
{
    if (g_musicChannelNum == 4) {
        // MusicPlayNoise
        const uint8_t *src = Music_NoiseTable + b;
        uint8_t *dst = &g_musicCh4NoiseData;
        do
            *dst++ = *src++;
        while (dst != &g_musicCh4NoiseData + 5);
    } else if (b == 0x01) {
        ch->rest = 1;
    } else {
        ch->rest = 0;
        ch->freq = *(const uint16_t *)(Music_FreqTable + b + (int8_t)g_musicTranspose);
    }
}

// Run the pattern command b at p. Returns the pointer to the next byte,
// or 0 when the song stops.
static const uint8_t *musicRunCommand(MusicChannel *ch, const uint8_t *p, uint8_t b)
{
    if (b == 0x00) { // end of pattern
        if (!musicNextPattern(ch))
            return 0;
        return ch->pattern;
    }
    if (b == 0x9d)
        return musicCmd_SetInstrument(ch, p);
    if (b == 0x9e) {
        // MusicCmd_SetNoteLengths: set the note length table pointer ($de92/$de93).
        (&g_musicNoteLenPtr)[0] = p[1];
        (&g_musicNoteLenPtr)[1] = p[2];
        return p + 3;
    }
    if (b == 0x9f) {
        // MusicCmd_SetTranspose: set the transpose value ($de91).
        g_musicTranspose = p[1];
        return p + 2;
    }
    if (b == 0x9b) {
        // MusicCmd_RepeatStart: add the repeat count n to the flags and
        // remember the position after n.
        ch->flags |= p[1];
        ch->repeat = p + 2;
        return p + 2;
    }
    // $9c MusicCmd_RepeatEnd: jump back to the repeat start until the count runs out.
    if (--ch->flags & 0x7f)
        return ch->repeat;
    return p + 1;
}

// Read and run pattern bytes until a note or rest is found. Then play it
// and load the note timer. Returns 0 when the song stops.
static uint8_t musicReadCommand(MusicChannel *ch)
{
    // p is a local copy of the pattern pointer (+4). It is stored back below.
    const uint8_t *p = ch->pattern;
    uint8_t b;
    // $00 and $9b-$9f are commands.
    while ((b = *p) == 0x00 || (uint8_t)(b - 0x9b) < 5) {
        p = musicRunCommand(ch, p, b);
        if (!p)
            return 0;
    }
    if ((b & 0xf0) == 0xa0) {
        // $a0-$af: set the note length, then read the note.
        ch->length = ((const uint8_t *)WORD(g_musicNoteLenPtr))[b & 0x0f];
        b = *++p;
    }
    ch->pattern = p + 1;
    musicSetNote(ch, b);
    musicPlayNote(ch);
    ch->timer = ch->length;
    return 1;
}

// Per-frame music update. Walks the four channel structs at $de50, $de60,
// $de70 and $de80. A channel with no list pointer is skipped.
// Count down the note timer (+2). When it ends, read the next pattern bytes.
void musicUpdate(void)
{
    if (!g_musicSongId)
        return;
    musicUpdatePan();
    g_musicChannelNum = 1;
    MusicChannel *ch = (MusicChannel *)&g_musicCh1;
    for (;;) {
        if ((uint16_t)ch->list >> 8) {
            if (--ch->timer)
                musicHoldNote(ch);
            else if (!musicReadCommand(ch))
                return;
        }
        if (g_musicChannelNum == 4)
            break;
        g_musicChannelNum++;
        ch++;
    }
    g_musicCh1VibratoStep++;
    g_musicCh2VibratoStep++;
    g_musicCh3VibratoStep++;
}
