"""Convert SDCC asm (asxxxx syntax) to rgbasm syntax, so C links with the game asm.

Usage: sdcc2rgbds.py BANK IN.asm OUT.s   (BANK = 0 for ROM0, else the ROMX bank number)

SDCC has its own --asm=rgbds mode, but it drops instructions (SDCC 4.5.1),
so we convert the normal output instead.
- `_name` -> `name`, so C and asm share names (g_score, DrawMetasprite).
- `_name = 0xNNNN` (from __at) -> a local `DEF name EQU $NNNN`.
- Code and constants go to one section in ROM0 or in ROMX bank BANK.
- Anything in a RAM area (a C global) is an error: the game owns all RAM.
- `b_name` (the bank of a __banked function) -> `BANK(name)`.
"""
import re
import sys

bank, src, dst = int(sys.argv[1]), sys.argv[2], sys.argv[3]
name = re.sub(r'\W', '_', src.rsplit('/', 1)[-1].rsplit('.', 1)[0])
section = f'SECTION "C {name}", ' + ('ROM0' if bank == 0 else f'ROMX, BANK[{bank}]')


def operand(op):
    op = op.strip()
    if op.startswith('(') and op.endswith(')'):
        return '[' + operand(op[1:-1]) + ']'
    op = op.replace('#', '')
    op = re.sub(r'<\(', 'LOW(', op)
    op = re.sub(r'>\(', 'HIGH(', op)
    op = re.sub(r'\b0x([0-9a-fA-F]+)', r'$\1', op)
    op = re.sub(r'\b(\d+)\$', r'.l\1', op)
    op = re.sub(r'\bb_(\w+)', r'BANK(\1)', op)
    return re.sub(r'(?<![\w.$])_([A-Za-z]\w*)', r'\1', op)


out, area, opened = [], None, False
for line in open(src):
    line = line.split(';', 1)[0].rstrip()
    # SDCC 4.5.1 sometimes prints `ldh (x), a` as `ld h (x), a`.
    line = re.sub(r'^(\s*)ld\s+h\s+\(', r'\1ldh (', line)
    if not line.strip():
        continue
    m = re.match(r'\s*\.area\s+(\w+)', line)
    if m:
        area = m.group(1)
        continue
    if re.match(r'\s*\.(module|optsdcc|globl)\b', line):
        continue
    m = re.match(r'_(\w+)\s*=\s*0x([0-9a-fA-F]+)$', line)
    if m:
        out.append(f'DEF {m.group(1)} EQU ${m.group(2)}')
        continue
    if re.match(r'\s*b_\w+\s*=', line):  # SDCC's guess of the bank; rgblink knows the real one
        continue
    if area != '_CODE':
        sys.exit(f'{src}: {area} has content (C globals are not allowed): {line.strip()}')
    if not opened:
        out.append(section)
        opened = True
    m = re.match(r'(\d+)\$:$', line)
    if m:
        out.append(f'.l{m.group(1)}:')
        continue
    m = re.match(r'(_?\w+)(::?)$', line)
    if m:
        out.append(operand(m.group(1)) + m.group(2))
        continue
    m = re.match(r'\s+(\.?\w+)\s*(.*)$', line)
    op, args = m.group(1).lower(), m.group(2)
    if op == 'ldhl':
        out.append('\tld hl, sp+' + operand(args.split(',')[1]))
        continue
    op = {'.db': 'db', '.dw': 'dw', '.ds': 'ds'}.get(op, op)
    if op.startswith('.'):
        sys.exit(f'{src}: unknown directive: {line.strip()}')
    args = [operand(a) for a in re.split(r',(?![^(]*\))', args)] if args else []
    if op == 'jp' and args == ['[hl]']:
        args = ['hl']
    out.append(f'\t{op} ' + ', '.join(args))

open(dst, 'w').write('\n'.join(out) + '\n')
