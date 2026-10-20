.SUFFIXES:
RGBDS ?= ../../tools/vendor/rgbds/
GBDK ?= ../../tools/gbdk/
PY ?= ../../tools/.venv/bin/python
C_OBJ := $(patsubst %.c,%.o,$(wildcard bank*_*.c))

all: game.gb

# The sound driver runs in the timer interrupt. Game timing depends on its
# speed, so SDCC searches harder for a good register allocation.
SOUND_C := $(wildcard bank4_*.c bank12_*.c)
$(SOUND_C:.c=.s): SDCCFLAGS := --max-allocs-per-node 100000
$(SOUND_C:.c=.s): sound.h
# Bank 1 and 9 code is large. A longer register allocation search makes it about 10% smaller.
BANK1_C := $(wildcard bank1_*.c bank9_*.c)
$(BANK1_C:.c=.s): SDCCFLAGS := --max-allocs-per-node 50000
$(BANK1_C:.c=.s): bank1.h
BANK3_C := $(wildcard bank3_*.c bank11_*.c)
$(BANK3_C:.c=.s): bank3.h
# Game A main code (bank 8, overflow in bank 10).
GAMEA_C := $(wildcard bank8_gamea*.c bank10_gamea*.c)
$(GAMEA_C:.c=.s): SDCCFLAGS := --max-allocs-per-node 50000
$(GAMEA_C:.c=.s): bank8_gamea.h
$(patsubst %.c,%.s,$(wildcard bank10_gameb.c bank10_gb*.c)): bank10_gameb.h

# bankN_name.c compiles into ROM bank N (bank0 = ROM0).
%.s: %.c game.h bank0.h ram.h hw.h sdcc2rgbds.py
	$(GBDK)bin/sdcc -msm83 -D__PORT_sm83 -I$(GBDK)include $(SDCCFLAGS) -S -o $*.sdasm $<
	$(PY) sdcc2rgbds.py $(patsubst bank%,%,$(firstword $(subst _, ,$<))) $*.sdasm $@

%.o: %.s
	$(RGBDS)rgbasm -o $@ $<

glue.o: glue.asm ram.inc
	$(RGBDS)rgbasm -o $@ $<

game.o: game.asm ram.inc glue.inc bank_*.asm
	$(RGBDS)rgbasm -o game.o game.asm

game.gb: game.o glue.o $(C_OBJ)
	$(RGBDS)rgblink -n game.sym -m game.map -o $@ $^
	$(RGBDS)rgbfix -v -p 255 $@

# Each line of each *.test file is one compare.py run.
test: game.gb
	grep -hv '^#' *.test | while read -r args; do $(PY) compare.py $$args || exit 1; done

clean:
	rm -f *.o *.s *.sdasm *.png game.gb game.sym game.map
