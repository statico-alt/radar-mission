// Shared by all C files: RAM, hardware registers and small helpers.
#include <stdint.h>
#include "ram.h"
#include "hw.h"

// The 16-bit word at a RAM name (little endian, like the asm).
#define WORD(v) (*(uint16_t *)&(v))
// The SM83 swap instruction: exchange the two nibbles.
#define SWAP(x) ((uint8_t)(((x) << 4) | ((x) >> 4)))

// bank0_math.c
uint16_t multiply8(uint8_t a, uint8_t b);

// Wrappers for bank 0 routines (game.asm).
// copyBytes and fillBytes: count must not be 0.
void copyBytes(uint8_t *dst, const uint8_t *src, uint16_t count);
void fillBytes(uint8_t *dst, uint8_t value, uint16_t count);
void unpackRunList(uint8_t *dst);
void drawNumber3(uint8_t *dst, uint8_t value);
// attrCount = sprite attributes << 8 | sprite count.
void buildSpriteStrips(const uint8_t *list, uint16_t attrCount, uint8_t *oam);
