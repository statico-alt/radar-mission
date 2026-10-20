; Disassembly of "rm.gb"
; This file was created with:
; mgbdis v3.0 - Game Boy ROM disassembler by Matt Currie and contributors.
; https://github.com/mattcurrie/mgbdis

INCLUDE "hardware.inc"
INCLUDE "ram.inc"
INCLUDE "glue.inc"
INCLUDE "bank_000.asm"
INCLUDE "bank_001.asm"
INCLUDE "bank_002.asm"
INCLUDE "bank_003.asm"
INCLUDE "bank_004.asm"
INCLUDE "bank_005.asm"
INCLUDE "bank_006.asm"
INCLUDE "bank_007.asm"
; C wrappers for bank 0 routines. They take the SDCC registers (see README.md).
; They are in ROM0, so C code in all banks can call them.
SECTION "C wrappers", ROM0

; void unpackRunList(uint8_t *dst): DE = dst.
unpackRunList::
    ld h, d
    ld l, e
    jp UnpackRunList

; void drawNumber3(uint8_t *dst, uint8_t value): DE = dst, A = value.
drawNumber3::
    ld h, d
    ld l, e
    jp DrawNumber3

; void buildSpriteStrips(const uint8_t *list, uint16_t attrCount, uint8_t *oam):
; DE = list, B = sprite attributes, C = sprite count, oam on the stack.
buildSpriteStrips::
    ld hl, sp + 2
    ld a, [hl+]
    ld h, [hl]
    ld l, a
    call BuildSpriteStrips
    ; Remove the stack argument and return (the callee removes it in SDCC).
PopArgReturn:
    pop hl
    add sp, 2
    jp hl

; void copyBytes(uint8_t *dst, const uint8_t *src, uint16_t count):
; DE = dst, BC = src, count on the stack. count must not be 0.
copyBytes::
    push bc
    ld hl, sp + 4
    ld a, [hl+]
    ld b, [hl]
    ld c, a
    pop hl
    call CopyMem
    jr PopArgReturn

; void fillBytes(uint8_t *dst, uint8_t value, uint16_t count):
; DE = dst, A = value, count on the stack. count must not be 0.
fillBytes::
    ld h, d
    ld l, e
    ld e, a
    push hl
    ld hl, sp + 4
    ld a, [hl+]
    ld b, [hl]
    ld c, a
    pop hl
    ld a, e
    call FillMem
    jr PopArgReturn
