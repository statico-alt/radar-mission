// Shared by the bank 0 C files (bank0_*.c, bank8_*.c, bank10_*.c).
#include "game.h"

// Read an HRAM or IO byte again on each access (for wait loops).
#define VOLATILE(v) (*(volatile uint8_t *)&(v))

// Call an asm routine in another bank through FarCall (bank_000.asm).
// Use it as the body of a __naked function. FarCall keeps A, BC, DE
// and HL into the routine and keeps A, F, BC, DE and HL out of it, so
// the function has the same register contract as the routine.
#define FAR_JP(f) __asm__("call _FarCall\n.db b_" #f "\n.dw _" #f "\nret")

// Bank 0 asm (bank_000.asm).
void SwitchROMBank(uint8_t bank);       // A = bank
void SoftReset(void);                   // does not return
void GameA_Frame(void);
void GameB_Frame(void);
void Vec_GameB_VBlank(void);
void VBlank_RunCopyQueue(void);
void DrawTilemapScript(const uint8_t *script);  // rst $28, DE = script

// bank0_far.c: copy n bytes (n > 0) or read one byte from a ROM bank.
void farCopy(uint8_t bank, uint8_t *dst, const uint8_t *src, uint16_t n);
uint8_t farByte(uint8_t bank, const uint8_t *p);

// Bank 1 asm and C. The main loop runs with bank 1 mapped.
void UpdatePaletteFade(void);
void StirRandom(void);
void CopyOamDmaToHram(void);
void readJoypad(void);

// Asm in other banks. A __banked prototype maps the bank for a call
// with no arguments.
void Vec5_TitleScreenInit(void) __banked;
void Bank5_EntryVector(void) __banked;
void Vec3_GameB_LinkUpdate(void) __banked;
void SoundUpdate(void) __banked;

// Graphics for the title screen.
extern const uint8_t Title_GfxLogoTiles[], GameA_GfxSprites[], GameA_GfxUiTiles[];
extern const uint8_t Title_ExtraTiles2Gfx[], Title_GfxExtraTiles[], Title_ExtraTilesGfx[];
