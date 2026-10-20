// FarCall shims for the Game A code in bank 8. Each one has the
// register contract of its target, so the arguments go through.
#include "bank8_gamea.h"

// Call an asm routine or C function with bank 1 mapped.
#define FAR1_JP(f) __asm__("call _FarCall\n.db 1\n.dw _" #f "\nret")

uint8_t drawTargetPattern(uint8_t row, uint8_t col);
uint8_t checkCoordOutsideGrid(uint8_t v);
uint16_t gridCellToOamXY(uint8_t row, uint8_t col);
void queueShipCellTiles(uint8_t id, uint16_t cell);
void drawStreamToBgMap(const uint8_t *grid);
void writeTileRuns(uint8_t *dst, const uint8_t *src);
uint16_t splitCellByte(uint8_t v);
void loadShipRecord(uint8_t n, const uint8_t *table);
void drawShipRecords(const uint8_t *table);
uint8_t scrollStepsTo(uint8_t cell, uint8_t margin);
uint16_t gridCellToScreen(uint8_t row, uint8_t col);

uint8_t gaDrawTargetPattern(uint8_t row, uint8_t col) __naked { (void)row; (void)col; FAR_JP(drawTargetPattern); }
uint8_t gaCheckCoordOutsideGrid(uint8_t v) __naked { (void)v; FAR_JP(checkCoordOutsideGrid); }
uint16_t gaGridCellToOamXY(uint8_t row, uint8_t col) __naked { (void)row; (void)col; FAR_JP(gridCellToOamXY); }
void gaQueueShipCellTiles(uint8_t id, uint16_t cell) __naked { (void)id; (void)cell; FAR_JP(queueShipCellTiles); }
void gaDrawStreamToBgMap(const uint8_t *grid) __naked { (void)grid; FAR_JP(drawStreamToBgMap); }
void gaWriteTileRuns(uint8_t *dst, const uint8_t *src) __naked { (void)dst; (void)src; FAR_JP(writeTileRuns); }
uint16_t gaSplitCellByte(uint8_t v) __naked { (void)v; FAR_JP(splitCellByte); }
void gaLoadShipRecord(uint8_t n, const uint8_t *table) __naked { (void)n; (void)table; FAR_JP(loadShipRecord); }
void gaDrawShipRecords(const uint8_t *table) __naked { (void)table; FAR_JP(drawShipRecords); }
uint8_t gaScrollStepsTo(uint8_t cell, uint8_t margin) __naked { (void)cell; (void)margin; FAR_JP(scrollStepsTo); }
uint16_t gaGridCellToScreen(uint8_t row, uint8_t col) __naked { (void)row; (void)col; FAR_JP(gridCellToScreen); }

// VBlank_RunCopyQueue maps bank 1 at the end.
void gaRunCopyQueue(void) __naked { FAR1_JP(VBlank_RunCopyQueue); }

// rst $28 with bank 1 mapped. DE = script.
void gaDrawTilemapScript(const uint8_t *script) __naked { (void)script; FAR1_JP(DrawTilemapScript); }

// BuildSpriteStrips with bank 1 mapped. DE = list, BC = attributes and
// count, oam on the stack (the callee removes it).
void gaSpriteStrips(const uint8_t *list, uint16_t attrCount, uint8_t *oam) __naked
{
    (void)list; (void)attrCount; (void)oam;
    __asm__("ldhl sp, #2\nld a, (hl+)\nld h, (hl)\nld l, a\n"
            "call _FarCall\n.db 1\n.dw _BuildSpriteStrips\n"
            "pop hl\nadd sp, #2\njp (hl)");
}

// Burn time (about 28 cycles per n). The LCD-off loads use it to take as
// long as the asm did: the load time sets the timer IRQ phase.
void gaPadCycles(uint16_t n)
{
    do {
        __asm__("nop");
    } while (--n);
}
