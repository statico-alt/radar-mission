#include <stdint.h>

// h * l, 8 x 8 -> 16 bit.
uint16_t multiply8(uint8_t a, uint8_t b)
{
    uint16_t r = 0;
    for (uint8_t i = 0; i < 8; i++) {
        r <<= 1;
        if (a & 0x80)
            r += b;
        a <<= 1;
    }
    return r;
}
