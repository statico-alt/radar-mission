// Per-frame helpers that run every frame: the play clock and the joypad.
// They are in bank 1 (mapped then), so the calls need no bank switch.
#include "bank1.h"

// Count the play clock of the player in $ffae bit 4 (P1) or bit 5 (P2).
// The time is minutes:seconds in g_linkSendPair (P1) or g_linkRecvPair (P2).
// It stops at 14:59.
void updatePlayClock(void)
{
    uint8_t *frames, *clock;
    if (g_gameFlags6 & 0x10) {
        frames = &g_p1ClockFrames;
        clock = g_linkSendPair;
    } else if (g_gameFlags6 & 0x20) {
        frames = &g_p2ClockFrames;
        clock = g_linkRecvPair;
    } else {
        return;
    }
    if (++*frames != 60)
        return;
    *frames = 0;
    if (++clock[1] != 60)
        return;
    clock[1] = 0;
    if (++clock[0] != 15)
        return;
    clock[0] = 14;
    clock[1] = 59;
}

// Read the joypad. $ff98 = buttons held, $ff99 = buttons newly pressed.
// Bit 7 = Down, 6 = Up, 5 = Left, 4 = Right, 3 = Start, 2 = Select,
// 1 = B, 0 = A.
void readJoypad(void)
{
    rP1 = 0x20;
    (void)rP1;
    uint8_t keys = (~rP1 & 0x0f) << 4;
    rP1 = 0x10;
    // Read 6 times, so the lines settle. The last read is used.
    // A loop is too slow here: the frame timing must stay close to the asm.
    (void)rP1;
    (void)rP1;
    (void)rP1;
    (void)rP1;
    (void)rP1;
    keys |= ~rP1 & 0x0f;
    g_joyPressed = (g_joyRaw ^ keys) & keys;
    g_joyRaw = keys;
    rP1 = 0x30;
}
