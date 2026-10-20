// Bank 11 code cannot see bank 3 data. It reads bank 3 tables here.
#include "game.h"
#include "bank3.h"

uint8_t bank3Byte(const uint8_t *p) __banked
{
    return *p;
}
