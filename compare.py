"""Run the original ROM and the C ROM with the same buttons. Fail when the screens differ for too long.

Usage: compare.py FRAMES [frame:button ...] [frame:addr=val ...] [@Label ...]
  frame:button  press a button at that frame of the original ROM, e.g. 600:down 630:start.
                The C ROM gets the press when its main loop reaches the same count.
  frame:addr=val  write a RAM byte at that frame (hex), the same way, e.g. 5000:d005=2d
  @Label        count how often the C ROM runs Label (from game.sym), to show that the test reaches it

C code is slower than the asm, so the C ROM can fall behind. A C screen
passes when it equals one of the last MAX_BEHIND original screens. Timing
also moves mid-frame effects, so a screen can match no original screen
(a glitch). More than MAX_GLITCH glitch frames in a row is a failure.

The sound driver is checked per call, not per frame. Game timing changes
when the sound requests arrive, and some requests cut an effect short. So
the sound RAM of the two ROMs can differ for a long time with correct
drivers. Instead, each call of the C driver is run again in a third
emulator with the original driver, from the same sound RAM ($de00-$df49).
The two results must be equal.

The game makes random numbers from rDIV and rLY. These values depend on
the timing, so the two ROMs get different random numbers. To prevent this,
a hook after each such read sets A to the next value of one fixed sequence.
"""
import collections
import glob
import os
import random
import re
import sys
import numpy as np
from pyboy import PyBoy

# The original ROM and the matching disassembly (with game.sym) are not in this repo.
ORIG, NEW = os.environ.get('RM_ROM', '../Radar Mission (UE).gb'), 'game.gb'
DISASM = os.environ.get('RM_DISASM', '../disasm/')
MAX_BEHIND = 30
MAX_GLITCH = 8
frames = int(sys.argv[1])
presses = collections.defaultdict(list)
for a in sys.argv[2:]:
    if ':' in a:
        f, b = a.split(':')
        presses[int(f)].append(b)


def act(pb, b):
    if '=' in b:
        addr, val = b.split('=')
        pb.memory[int(addr, 16)] = int(val, 16)
    else:
        pb.button(b, 2)


watch = [a[1:] for a in sys.argv[2:] if a.startswith('@')]

boys = [PyBoy(r, window='null', sound_emulated=False, symbols=s) for r, s in ((ORIG, DISASM + 'game.sym'), (NEW, 'game.sym'))]
calls = {w: [0] for w in watch}
for w in watch:
    bank, addr = boys[1].symbol_lookup(w)
    boys[1].hook_register(bank, addr, lambda c: c.__setitem__(0, c[0] + 1), calls[w])


# C functions that read rDIV or rLY for a random number. Add each new one here.
C_RNG = ['randomMasked', 'randomByte', 'random', 'stirRandom', 'gameBReadDiv']


def rng_labels(folder):
    """The label before each rDIV read in the asm files of folder."""
    labels = []
    for path in sorted(glob.glob(folder + 'bank_00*.asm')):
        parent = last = None
        for line in open(path):
            m = re.match(r'(\.?\w+):', line)
            if m:
                name = m.group(1)
                if name.startswith('.'):
                    last = parent + name
                else:
                    parent = last = name
            if re.match(r'\s*ldh a, \[rDIV\]', line):
                labels.append(last)
    return labels


RNG_READS = re.compile(rb'\xf0[\x04\x44]|\xfa[\x04\x44]\xff')  # ldh a, [rDIV/rLY] and ld a, [rDIV/rLY]


def pin_rng(pb, labels, wait):
    seq = random.Random(1)

    # The wait-for-frame loop calls StirRandom on each interrupt. Lag changes how
    # often, so those calls add 0 to the seed. idle is their return address.
    idle = pb.symbol_lookup(wait)[1] + 3

    def pin(_):
        sp = pb.register_file.SP
        # Stop at the stack top ($dfff). Above it is echo RAM, which can hold any bytes.
        if any(pb.memory[sp + i] | pb.memory[sp + i + 1] << 8 == idle for i in range(0, min(24, 0xdffe - sp), 2)):
            pb.register_file.A = 0
        else:
            pb.register_file.A = seq.randrange(256)

    hooked = set()
    for label in labels:
        bank, addr = pb.symbol_lookup(label)
        code = bytes(pb.memory[bank, addr + i] for i in range(48))
        code = code[:code.find(b'\xc9') + 1 or None]  # up to the first ret
        reads = list(RNG_READS.finditer(code))
        if not reads:
            sys.exit(f'no RNG read after {label}')
        for m in reads:
            if (bank, addr + m.end()) not in hooked:
                hooked.add((bank, addr + m.end()))
                pb.hook_register(bank, addr + m.end(), pin, None)


ticks = [[0], [0]]  # main-loop count of each ROM
for pb, t, main in zip(boys, ticks, ['SoftReset.mainLoop', 'mainLoop']):
    pb.hook_register(*pb.symbol_lookup(main), lambda c: c.__setitem__(0, c[0] + 1), t)

pin_rng(boys[0], rng_labels(DISASM) + ['RandomMasked', 'RandomByte'], 'SoftReset.waitFrame')
syms = open('game.sym').read()  # C_RNG names not in this ROM yet are skipped
pin_rng(boys[1], rng_labels('') + [l for l in C_RNG if re.search(rf' {l}$', syms, re.M)], 'waitFrame')


SOUND = slice(0xde00, 0xdf4a)  # sound RAM. The driver reads and writes only this RAM.
# The reference emulator runs this code at $c000:
# ld a, 4 / ld [$2000], a / call SoundUpdate / jr @ (stops at $c008).
REF_CODE = [0x3e, 0x04, 0xea, 0x00, 0x20, 0xcd, 0x00, 0x40, 0x18, 0xfe]
ref = PyBoy(ORIG, window='null', sound_emulated=False)
for i, v in enumerate(REF_CODE):
    ref.memory[0xc000 + i] = v
ref.memory[0xffff] = 0  # no interrupts
ref.memory[0xff50] = 1  # unmap the boot ROM. A song can read $0000-$00ff as data.


def replay(ram):
    """The sound RAM after one call of the original driver, from ram."""
    for i, v in enumerate(ram):
        ref.memory[SOUND.start + i] = v
    ref.register_file.PC, ref.register_file.SP = 0xc000, 0xd000
    ref.tick()
    assert ref.register_file.PC == 0xc008, 'the original driver did not return in one frame'
    return np.array(ref.memory[SOUND], np.uint8)


def counts():
    return ' '.join(f'{w}={c[0]}' for w, c in calls.items())


recent = collections.deque(maxlen=MAX_BEHIND + 1)  # the last original screens
bad, glitches, behind = 0, [], 0
sound_calls = []  # (sound RAM at entry, at exit) of each C driver call
sound_entry = []  # [sound RAM at entry, nested] of each running call
nested_calls = 0
new_pb = boys[1]
bank, addr = new_pb.symbol_lookup('SoundUpdate')


def sound_enter(_):
    # The timer IRQ can call the driver while the Game B main loop runs it.
    # Then the outer call does not start from its entry RAM, so it is not checked.
    for e in sound_entry:
        e[1] = True
    sound_entry.append([np.array(new_pb.memory[SOUND], np.uint8), False])


def sound_exit(_):
    global nested_calls
    entry, nested = sound_entry.pop()
    if nested:
        nested_calls += 1
    else:
        sound_calls.append((entry, np.array(new_pb.memory[SOUND], np.uint8)))


new_pb.hook_register(bank, addr, sound_enter, None)
# SoundUpdate + 7 is the instruction after `call soundUpdate` (CALL_C pushes 4 registers first).
new_pb.hook_register(bank, addr + 7, sound_exit, None)
pending = []  # (main-loop count, button) for the C ROM
for f in range(frames):
    for b in presses.get(f, []):
        act(boys[0], b)
        pending.append((ticks[0][0], b))
    while pending and ticks[1][0] >= pending[0][0]:
        act(boys[1], pending.pop(0)[1])
    for pb in boys:
        pb.tick()
    for entry, new in sound_calls:
        diff = np.flatnonzero(replay(entry) != new)
        if diff.size:
            sys.exit(f'SOUND DIFF at frame {f}, bytes ' + ' '.join(f'${0xde00 + i:04x}' for i in diff) + ' ' + counts())
    sound_calls.clear()
    a, b = (pb.screen.ndarray.copy() for pb in boys)
    recent.appendleft(a)
    lag = next((i for i, old in enumerate(recent) if (old == b).all()), None)
    if lag is not None:
        bad = 0
        behind = max(behind, lag)
        continue
    if bad == 0:
        glitches.append(f)
        boys[0].screen.image.save('diff_orig.png')
        boys[1].screen.image.save('diff_new.png')
    bad += 1
    if bad > MAX_GLITCH:
        sys.exit(f'DIFF at frame {glitches[-1]} (diff_orig.png, diff_new.png) ' + counts())
boys[1].screen.image.save('last.png')
print(f'SAME for {frames} frames, at most {behind} frames behind, glitches at {glitches}', f'nested sound calls={nested_calls}' if nested_calls else '', counts())
