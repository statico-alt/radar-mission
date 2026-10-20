// Game B tally steps that read no bank 1 data. Bank 1
// glue calls them from Bank1_TallyVectors.
#include "bank1.h"

// BCD add: dst += src, n bytes, one digit per byte, low digit first.
void bcdAdd(uint8_t *dst, const uint8_t *src, uint8_t n) __banked
{
    uint8_t carry = 0;
    do {
        uint8_t a = *src++ + *dst + carry;
        carry = a >= 10;
        if (carry)
            a -= 10;
        *dst++ = a;
    } while (--n);
}

// Tally state 4: count the row product down by 100 a frame and add 100
// to the score. Every 2 frames play sound $18. At zero go to state 6.
void tallyState4AccumulateScore(void) __banked
{
    if (g_tallyStepTimer) {
        g_tallyStepTimer--;
        return;
    }
    const uint8_t *p = &g_rowProductBCD;
    uint8_t n = 6;
    while (!*p++) {
        if (!--n) {
            g_tallyCutsceneSubState = 0x06;
            g_tallyStepTimer = 0x18;
            return;
        }
    }
    // The addend is 100.
    uint8_t *d = &g_scoreAddendBCD;
    for (uint8_t i = 0; i != 6; i++)
        d[i] = i == 2;
    bcdAdd(&g_scoreTotalBCD, &g_scoreAddendBCD, 6);
    bcdAdd(&g_displayedScoreBCD, &g_scoreAddendBCD, 6);
    // BCD subtract 1 from the row product, from digit 2 up.
    d = &g_tallyDigitAnim;
    while (--*d == 0xff)
        *d++ = 9;
    if (--g_tallyRowIndex)
        return;
    g_tallyRowIndex = 0x02;
    SFX_Queue = 0x18;
}

// Tally state 5: wait for a button, then leave the tally.
void tallyState5WaitInput(void) __banked
{
    if (!(g_joyPressed & 0x0f))
        return;
    g_tallyCutsceneSubState = 0;
    g_subState++;
}

// Tally state 6: after the wait pick the fanfare from bits 0 and 6 of
// g_sceneFlagsC, then go to state 5.
void tallyState6SelectFanfare(void) __banked
{
    if (--g_tallyStepTimer)
        return;
    g_tallyCutsceneSubState = 0x05;
    uint8_t f = g_sceneFlagsC & 0x41;
    BGM_Queue = !f ? 0x0b : f == 0x41 ? 0xff : 0x19;
}

// Copy the score digits (g_scoreTotalBCD, then g_displayedScoreBCD, high
// digit first) to the tiles of the 12 score sprites.
void tallyCopyDigitsToTiles(void) __banked
{
    uint8_t *s = OAM + 2;
    const uint8_t *d = &g_scoreTotalBCD + 5;
    for (uint8_t n = 6; n; n--, s += 4)
        *s = *d-- + 0x84;
    d = &g_displayedScoreBCD + 5;
    for (uint8_t n = 6; n; n--, s += 4)
        *s = *d-- + 0x84;
}
