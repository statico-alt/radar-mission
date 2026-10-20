# Radar Mission in C

This repo is a C version of *Radar Mission*, a Game Boy game by Nintendo from 1990.

The game code is in C. The game data and some small asm routines stay in asm. The build makes a ROM that you can play in a Game Boy emulator.

The new ROM is not the same as the original byte for byte. It plays the same game. A test tool runs the two ROMs side by side and compares the screens frame by frame.

Radar Mission is © 1990 Nintendo. This repo does not contain the original ROM.

## What you need

- [GBDK-2020](https://github.com/gbdk-2020/gbdk-2020), for the SDCC C compiler (tested with SDCC 4.5.1)
- [RGBDS](https://github.com/gbdev/rgbds) (tested with 1.0.1)
- Python 3 with `pyboy` and `numpy` (for the tests only)

## Build the ROM

1. Install GBDK-2020 and RGBDS.
2. Run `make`, and give the tool paths. Each directory path must end with `/`:

   ```
   make GBDK=/opt/gbdk/ RGBDS=/usr/local/bin/ PY=python3
   ```

3. Open `game.gb` in a Game Boy emulator.

## Run the tests

The tests need the original ROM and a matching disassembly of the game, with its `game.sym` file.

1. Set `RM_ROM` to the path of the original ROM.
2. Set `RM_DISASM` to the disassembly directory. The path must end with `/`.
3. Run `make test`:

   ```
   RM_ROM="/path/to/Radar Mission (UE).gb" RM_DISASM=/path/to/disasm/ make test
   ```

4. Make sure that each output line starts with `SAME`.

## More information

[ARCHITECTURE.md](ARCHITECTURE.md) explains how the port works:

- how the build works
- the status of each ROM bank, and what stays in asm
- the extra ROM banks for the C code
- the calls between banks, and the glue between asm and C
- the rules for the C code
- the test tool (`compare.py`)
- how to port a function
