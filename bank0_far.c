#include "bank0.h"

// Copy n bytes (n > 0) from ROM bank `bank`. Keeps the mapped bank.
void farCopy(uint8_t bank, uint8_t *dst, const uint8_t *src, uint16_t n)
{
    uint8_t old = g_currentBankShadow;
    SwitchROMBank(bank);
    copyBytes(dst, src, n);
    SwitchROMBank(old);
}

// Read one byte from ROM bank `bank`. Keeps the mapped bank.
uint8_t farByte(uint8_t bank, const uint8_t *p)
{
    uint8_t old = g_currentBankShadow;
    SwitchROMBank(bank);
    uint8_t v = *p;
    SwitchROMBank(old);
    return v;
}
