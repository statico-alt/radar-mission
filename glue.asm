; Glue between the asm register rules and the SDCC rules (see README.md).
INCLUDE "hardware.inc"
INCLUDE "ram.inc"

SECTION "Glue", ROM0

; HL = H * L. Keeps A, BC and the flags, like the old asm.
Multiply8::
    push af
    push bc
    ld a, h
    ld e, l
    call multiply8
    ld h, b
    ld l, c
    pop bc
    pop af
    ret

; SDCC calls this for a __banked function: E = bank, HL = function.
; The arguments are on the stack. The function finds them at sp+6.
; Maps the bank, calls the function, then maps the caller's bank again.
; Keeps the result in A or BC. Changes E.
___sdcc_bcall_ehl::
    ldh a, [g_currentBankShadow]
    push af
    ld a, e
    ldh [g_currentBankShadow], a
    ld [rROMB0], a
    call .jumpHL
    ld e, a
    pop af
    ldh [g_currentBankShadow], a
    ld [rROMB0], a
    ld a, e
    ret
.jumpHL:
    jp hl
