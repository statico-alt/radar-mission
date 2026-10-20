; Glue macros: asm calls C. See README.md for the SDCC register rules.

; Calls a C function with no arguments and no result. Keeps all registers.
MACRO CALL_C
    push af
    push bc
    push de
    push hl
    call \1
    pop hl
    pop de
    pop bc
    pop af
    ret
ENDM

; The same for a __banked C function in another bank.
MACRO FAR_CALL_C
    push af
    push bc
    push de
    push hl
    ld e, BANK(\1)
    ld hl, \1
    call ___sdcc_bcall_ehl
    pop hl
    pop de
    pop bc
    pop af
    ret
ENDM
