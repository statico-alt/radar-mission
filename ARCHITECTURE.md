# Architecture

This file explains how the port works and how to change it. For the setup, read [README.md](README.md).

## How the build works

The base is a matching disassembly of the game. The `bank_00N.asm` files keep the game data, the interrupt entry code and the routines that must stay in asm.

1. SDCC compiles each `.c` file to SDCC asm (`.sdasm`).
2. `sdcc2rgbds.py` converts the SDCC asm to RGBDS asm (`.s`) and puts it in the correct ROM bank.
3. `rgbasm` assembles the `.s` files and `game.asm`.
4. `rgblink` links all the objects into `game.gb`. Then `rgbfix` sets the header checksums.

By default, the Makefile looks for the tools in `../../tools`. `compare.py` looks for the original ROM and the disassembly in `..`. The README shows how to give other paths.

## Status

| Bank | State |
|---|---|
| 0 | C: `bank0_main.c` (SoftReset tail, `mainLoop`, `waitFrame`), `bank0_irq.c` (VBlank, timer, Game A STAT and serial IRQ bodies), `bank0_far.c` (`farCopy`). In bank 8, Game A: `bank8_gamea_frame.c`, `bank8_gamea_states.c`, `bank8_gamea_turn.c`, `bank8_gamea_cpu.c`, `bank8_gamea_far.c` (`bank8_gamea.h`). In bank 10: `bank10_title.c` (title menu), `bank10_ending.c` (Game B ending load), and Game B in `bank10_gameb.c`, `bank10_gbcut.c`, `bank10_gbfar.c` (`bank10_gameb.h`). `bank0.h` has the shared declarations, `VOLATILE()` and `FAR_JP()`. `bank_000.asm` keeps the vectors, the SP setup and RAM clear, the IRQ entry and exit code, `FarCall` (asm that calls asm in another bank and keeps all registers), the helpers that return values in registers or flags, the Game B STAT raster code, the Game B per-frame engine and object code, and Game B state $07 (in C, it moves the timer IRQ phase). The serial IRQ, the link code and the ending are not tested. |
| 2, 7 | asm |
| 1 | C: `bank1_*.c`, and in bank 9 `bank9_*.c`. `bank1.h` has the externs for the bank 1 data. `bank_001.asm` keeps the data, the vectors, the glue, `OamDmaRoutine` (it runs from HRAM), `moveScrollSprites` (it runs in VBlank, and C is too slow there) and `Bank1_TallyVectors` (bank 0 jumps into the middle of its entries). |
| 3 | Part C: `bank3_ai.c`, `bank3_range.c`, `bank3_hud.c`, `bank3_sound.c`, `bank3_far.c`, and in bank 11 `bank11_damage.c`, `bank11_link.c`, `bank11_objects.c`, `bank11_queues.c`, `bank11_weapons.c`. `bank3.h` has the externs for the bank 3 data. `bank_003.asm` keeps the data, the glue, the routines that return values in registers (`MemFill`, `HudQueue_ReadSlot`, `DispatchJumpTable` and others), the RLE tile routines (kept for speed: the LCD-off load time sets the timer IRQ phase) and the groups that are not ported yet. The link code is not tested. |
| 4 | C: `bank4_driver.c`, `bank4_music.c`, and in bank 12 `bank12_sfx.c`, `bank12_noise.c`, `bank12_fade.c`. `sound.h` has the shared types. `bank_004.asm` keeps the data and the `SoundUpdate` stub at $4000. |
| 5 | C: `bank5_screens.c`, `bank5_options.c`, `bank5_grid.c`, `bank5_ai.c`, and in bank 13 `bank13_ai.c`, `bank13_link.c`. `bank_005.asm` keeps the data, the jump table and the glue. The link code is not tested. |
| 6 | C: `bank6_sprites.c`, `bank6_level.c`, `bank6_ships.c`, `bank6_effects.c`. `bank_006.asm` keeps the data, the jump table and the glue. |

`bank0_math.c` has `multiply8`. The end of `game.asm` has ROM0 wrappers that C in all banks can call: `copyBytes`, `fillBytes`, `unpackRunList`, `drawNumber3` and `buildSpriteStrips`. ROM0 has about 10 KB free.

## Build and test

1. Run `make`.
2. Run `make test`. It runs `compare.py` once for each line of each `*.test` file.
3. Make sure that each output line starts with `SAME`.

## Banks

The ROM is 256 KB (MBC1, banks 0-15). The original uses banks 0-7. Banks 1-5 and 7 are full.

The C code for bank N goes in bank N, in the space that the asm code used. C code that does not fit goes in an extra bank:

| Code of bank | Extra bank |
|---|---|
| 0 | 8 (graphics data from bank 6 uses $4000-$57ff), 10 |
| 1 | 9 |
| 3 | 11 |
| 4 | 12 |
| 5 | 13 |

The data stays in its bank. Other banks find it with a fixed bank number. A C function that reads the data of bank N must be in bank N.

### Calls between banks

- Declare a C function in another bank with `__banked` (`void f(void) __banked;`). SDCC calls it through `___sdcc_bcall_ehl` in `glue.asm`. This trampoline maps the bank and maps the caller's bank again after the call.
- A `__banked` function gets all its arguments on the stack. The caller removes them. The result is in A or BC.
- A normal call works only to bank 0 or to the same bank.
- Asm calls a `__banked` function with no arguments through `FAR_CALL_C` (`glue.inc`).
- Many bank 0 routines switch banks and then map one fixed bank again (for example `SwitchToBank3_Tail`). Call such a routine only from the bank that it maps again.

## Files

| File | Use |
|---|---|
| `bankN_name.c` | C code for ROM bank N. The Makefile puts it in `SECTION "C name"` in bank N (bank 0 = ROM0). |
| `game.h` | Include this in each `.c` file. It includes `ram.h` and `hw.h`, and gives `WORD()`, `SWAP()` and `multiply8()`. |
| `ram.h`, `hw.h` | RAM and hardware registers as `__at` variables. `gen_headers.py` makes them from `ram.inc` and `hardware.inc`. |
| `sdcc2rgbds.py` | Converts SDCC asm output to RGBDS asm. Do not use `sdcc --asm=rgbds`, because it drops instructions. |
| `compare.py` | Runs the original ROM and `game.gb` side by side (see below). |

## Rules for C code

- Do not declare C globals. Use the RAM names in `ram.h`. A `static const` table is correct, because it goes in ROM.
- Do not use operations that need SDCC library helpers: `%`, `/` by a variable, `*` by a variable. Use `multiply8()`, shifts and masks. The link fails when a helper is necessary.
- A `static` function is a local label in `game.sym`. `compare.py` can count it with `@name`.
- Keep the names and comments of the asm when they are correct.

## Glue between asm and C

SDCC `__sdcccall(1)` uses these registers:

- First argument: A (8-bit) or DE (16-bit).
- Second argument: E (8-bit after an 8-bit), or A (8-bit after DE).
- More arguments go on the stack. The called function removes them.
- Result: A (8-bit) or BC (16-bit).
- The C code can change all registers.

The asm callers expect other registers, and they expect that registers stay the same. Each C function that asm calls has a glue routine in the asm bank:

- A void function with no arguments uses `CALL_C name` (`glue.inc`). It keeps AF, BC, DE and HL.
- Other glue moves the asm input registers to the SDCC registers, then sets each register that the asm returned. Keep every other register.
- Read all callers of the asm function before you write the glue. Write down which registers and flags they use after the call.

## compare.py

`compare.py FRAMES [frame:button ...] [frame:addr=val ...] [@Label ...]`

- `frame:button` presses a button at that frame of the original ROM. The C ROM gets the press when its main loop (`mainLoop` in the C ROM, `SoftReset.mainLoop` in the original) reaches the same count.
- `frame:addr=val` writes a RAM byte (hex) the same way. Use it to force a game state that is hard to reach.
- `@Label` counts the calls of a label in the C ROM. Use it to show that the test runs the new code.

The C code is slower than the asm, so the C ROM can fall behind (by up to 20 frames in the Game B tests). A C screen passes when it is the same as one of the last 30 original screens. The timing also moves mid-frame effects. More than 8 frames in a row with no match is a failure.

The sound driver is checked per call. Each call of the C driver runs again in a third emulator with the original driver, from the same sound RAM ($de00-$df49). The two results must be equal. A per-frame check does not work: the timing changes when sound requests arrive, so correct drivers can have different sound RAM for a long time. The check does not see the writes to the sound registers.

The game makes random numbers from `rDIV` and `rLY`. These values depend on the timing. `compare.py` sets A after each such read to the next value of one fixed sequence. It finds the reads from the `ldh a, [rDIV]` lines in the asm, and from the C functions in `C_RNG`.

The wait-for-frame loop calls `StirRandom` once per main loop tick. The C ROM is slower, so it makes fewer such calls. `compare.py` sets A to 0 for these calls, so they do not change the seed.

## Port a function

1. Write the function in a `bankN_*.c` file.
2. Remove the asm version from its bank.
3. If asm still calls the function, add glue (see above).
4. Run `make`. Then add a line to `bankN.test` that reaches the function, with an `@name` count.
5. Run `make test`.
