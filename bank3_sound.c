// Sound queue. A ring of 16 requests of (type, id) at g_soundQueue.
// soundQueuePush is in bank11_queues.c.
#include "game.h"
#include "bank3.h"

// Sound queue bytes, indexed by (sound type - 1).
static uint8_t *const soundQueuePtrs[] = {
    &SFX_Queue, &NoiseSfx_Queue, &BGM_Queue, &SoundCmd_Queue, &g_soundCueRequest,
};

// Takes one entry from the ring and writes its id to the queue byte of its type.
void soundQueueRunNext(void)
{
    uint8_t *slot = &g_soundQueue + (g_soundQueueRead << 1);
    uint8_t type = slot[0];
    if (!type)
        return;
    uint8_t *queue = soundQueuePtrs[type - 1];
    uint8_t id = slot[1];
    *queue = id;
    if (queue == &BGM_Queue) {
        // Music 6 and $18 clear some sound driver blocks.
        if (id == 0x06) {
            g_sfxBlockAll = 0;
            g_sfxNoiseBusy = 0;
            g_sfx04Busy = 0;
        } else if (id == 0x18) {
            g_sfxBlockSfx0D = 0;
            g_sfx04Busy = 0;
        }
    }
    slot[0] = 0;
    slot[1] = 0;
    g_soundQueueRead = (g_soundQueueRead + 1) & 0x0f;
}
