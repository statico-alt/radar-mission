; Disassembly of "rm.gb"
; This file was created with:
; mgbdis v3.0 - Game Boy ROM disassembler by Matt Currie and contributors.
; https://github.com/mattcurrie/mgbdis

; ROM bank 4. Sound driver data: songs, music tables and the SFX register
; data. The code is now C: bank4_driver.c and bank4_music.c (bank 4),
; bank12_sfx.c, bank12_noise.c and bank12_fade.c (bank 12).
; The data keeps its original addresses, so the pointers that the driver
; keeps in RAM have the same values as in the original game.
; The SFX data and the fade tables moved to bank 12 (same addresses), with
; the C code that reads them.

; The C code reads these tables.
EXPORT Music_SongTable, Music_SongPanTable, Music_FreqTable, Music_NoiseTable
EXPORT Music_VibratoTable, FadeInSteps, FadeOutSteps, FadeOutFastSteps
EXPORT Sfx21_Start_Sq1Regs, Sfx01_Start_Sq1Regs, Sfx18_Start_Sq1Regs
EXPORT Sfx08_Start_Sq1Regs, Sfx17_Start_Sq1Regs, NoiseSfx02_Start_NoiseRegs
EXPORT NoiseSfx02_Update_NoiseRegs, NoiseSfx0F_Start_NoiseRegs, NoiseSfx0F_Update_NoiseRegs
EXPORT Sfx0A_Start_Sq1Regs, NoiseSfx01_Start_NoiseRegs, NoiseSfx01_Update_NoiseRegs
EXPORT SoundCue1_Start_Sq1Regs, SoundCue1_Update_Sq1Regs, SoundCue1_Start_NoiseRegs
EXPORT Sfx11_Play_Sq1Regs, Sfx11_Update_Sq1Regs, Sfx11_Play_NoiseRegs
EXPORT Sfx0D_Start_Sq1Regs, Sfx0D_Update_Sq1Regs, Sfx0D_Start_NoiseRegs
EXPORT Sfx0D_Update_NoiseRegs, NoiseSfx07_Start_NoiseRegs, NoiseSfx0C_Start_NoiseRegs
EXPORT Sfx06_Start_Sq1Regs, NoiseSfx0D_Start_NoiseRegs, NoiseSfx0E_Run_NoiseRegs
EXPORT NoiseSfx09_Start_NoiseRegs, Sfx0B_Start_Sq1Regs, Sfx14_Start_Sq1Regs
EXPORT Sfx14_Start_Sq2Regs, Sfx19_Start_Sq1Regs, Sfx19_Start_Sq2Regs
EXPORT Sfx32_Update_Sq1Regs, NoiseSfx03_Start_NoiseRegs, NoiseSfx03_Update_NoiseRegs
EXPORT NoiseSfx03_Update_NoiseRegs2, NoiseSfx03_Update_NoiseRegs3, NoiseSfx08_Start_NoiseRegs
EXPORT NoiseSfx04_Start_NoiseRegs, Sfx13_Start_NoiseRegs, Sfx10_Start_Sq1Regs
EXPORT Sfx10_Start_NoiseRegs, Sfx04_Start_Sq1Regs, Sfx15_Start_Sq1Regs
EXPORT Sfx0F_Start_Sq1Regs, Sfx0F_Start_Sq2Regs, Sfx02_Start_Sq1Regs
EXPORT Sfx02_Start_Sq2Regs, Sfx1D_Start_Sq1Regs, Sfx1D_Start_Sq2Regs
EXPORT Sfx1B_Start_Sq1Regs, Sfx1B_Start_Sq2Regs, Sfx1A_Start_Sq1Regs
EXPORT Sfx1A_Start_Sq2Regs, Sfx05_Start_Sq1Regs, NoiseSfx05_Start_NoiseRegs
EXPORT Sfx0B_Start_NoiseRegs, NoiseSfx0A_Start_NoiseRegs, NoiseSfx0A_Start_Sq1Regs

SECTION "ROM Bank $004", ROMX[$4000], BANK[$4]

; Sound driver entry point. Bank 0 calls it once per frame with bank 4
; paged in (IRQ_Timer and GameB_TickSoundEngine).
SoundUpdate:
    CALL_C soundUpdate


SECTION "Bank 4 song table", ROMX, BANK[$4]

; Song pointers, indexed by song ID - 1 (song IDs $01-$1C go in BGM_Queue).
Music_SongTable:
    dw Music_Song01
    dw Music_Song02
    dw Music_Song03
    dw Music_Song04
    dw Music_Song05
    dw Music_Song06
    dw Music_Song07
    dw Music_Song08
    dw Music_Song09
    dw Music_Song0A
    dw Music_Song0B
    dw Music_Song0C
    dw Music_Song0D
    dw Music_Song0E
    dw Music_Song0F
    dw Music_Song10
    dw Music_Song11
    dw Music_Song12
    dw Music_Song13
    dw Music_Song14
    dw Music_Song15
    dw Music_Song16
    dw Music_Song17
    dw Music_Song18
    dw Music_Song19
    dw Music_Song1A
    dw Music_Song1B
    dw Music_Song1C


SECTION "Bank 4 music tables", ROMX[$54b4], BANK[$4]

; Per-song stereo settings, 4 bytes per song (song $01 first):
;   pan mode (1 = fixed), pan toggle period, NR51 value, alternate NR51 value
; MusicLoadSongPan copies the entry to $de94/$de96/$de98/$de99.
Music_SongPanTable:
    db $01, $ff, $ff, $ff ; song $01
    db $01, $ff, $ff, $eb ; song $02
    db $01, $ff, $ff, $b7 ; song $03
    db $01, $ff, $ff, $ed ; song $04
    db $01, $ff, $ff, $ff ; song $05
    db $01, $ff, $ff, $be ; song $06
    db $01, $ff, $ff, $f7 ; song $07
    db $01, $ff, $ff, $ff ; song $08
    db $02, $20, $fd, $df ; song $09
    db $01, $ff, $ff, $e7 ; song $0A
    db $01, $ff, $ff, $e7 ; song $0B
    db $01, $ff, $ff, $ff ; song $0C
    db $01, $ff, $ff, $e7 ; song $0D
    db $02, $30, $ed, $de ; song $0E
    db $01, $ff, $ff, $e7 ; song $0F
    db $01, $c0, $df, $fd ; song $10
    db $01, $ff, $ff, $e7 ; song $11
    db $02, $30, $fd, $df ; song $12
    db $01, $ff, $ff, $e7 ; song $13
    db $01, $ff, $ff, $e7 ; song $14
    db $01, $ff, $ff, $e7 ; song $15
    db $01, $ff, $ff, $e7 ; song $16
    db $01, $ff, $ff, $e7 ; song $17
    db $01, $ff, $ff, $ff ; song $18
    db $01, $ff, $ff, $e7 ; song $19
    db $01, $ff, $ff, $e7 ; song $1A
    db $01, $ff, $ff, $e7 ; song $1B
    db $01, $ff, $ff, $be ; song $1C

; Song headers. Music_SongTable points here (indexed by song ID - 1).
; Header: transpose, note length table, then channel lists 1-4.
Music_Song01:
    db $fe ; transpose (-2)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song01_Ch1, Music_Song01_Ch2, Music_Song01_Ch3, Music_Song01_Ch4
; Songs $02 and $0b share one header.
Music_Song02:
Music_Song0B:
    db $00 ; transpose (0)
    dw Music_NoteLengths4 ; note length table
    dw Music_Song02_Ch1, Music_Song02_Ch2, Music_Song02_Ch3, Music_Song02_Ch4
Music_Song03:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song03_Ch1, Music_Song03_Ch2, Music_Song03_Ch3, Music_Song03_Ch4
Music_Song04:
    db $02 ; transpose (2)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song04_Ch1, Music_Song04_Ch2, Music_Song04_Ch3, Music_Song04_Ch4
Music_Song05:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song05_Ch1, Music_Song05_Ch2, Music_Song05_Ch3, Music_Song05_Ch4
Music_Song06:
    db $fc ; transpose (-4)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song06_Ch1, Music_Song06_Ch2, Music_Song06_Ch3, Music_Song06_Ch4
Music_Song07:
    db $08 ; transpose (8)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song07_Ch1, Music_Song07_Ch2, Music_Song07_Ch3, Music_Song07_Ch4
Music_Song08:
    db $00 ; transpose (0)
    dw Music_NoteLengths2 ; note length table
    dw Music_Song08_Ch1, Music_Song08_Ch2, Music_Song08_Ch3, Music_Song08_Ch4
Music_Song09:
    db $00 ; transpose (0)
    dw Music_NoteLengths1 ; note length table
    dw Music_Song09_Ch1, Music_Song09_Ch2, Music_Song09_Ch3, Music_Song09_Ch4
Music_Song0A:
    db $00 ; transpose (0)
    dw Music_NoteLengths2 ; note length table
    dw Music_Song0A_Ch1, Music_Song0A_Ch2, Music_Song0A_Ch3, Music_Song0A_Ch4
Music_Song0C:
    db $00 ; transpose (0)
    dw Music_NoteLengths2 ; note length table
    dw Music_Song0C_Ch1, Music_Song0C_Ch2, Music_Song0C_Ch3, Music_Song0C_Ch4
; Songs $0d and $0e share one header.
Music_Song0D:
Music_Song0E:
    db $08 ; transpose (8)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song0D_Ch1, Music_Song0D_Ch2, Music_Song0D_Ch3, Music_Song0D_Ch4
Music_Song0F:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song0F_Ch1, Music_Song0F_Ch2, Music_Song0F_Ch3, Music_Song0F_Ch4
Music_Song10:
    db $f4 ; transpose (-12)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song10_Ch1, Music_Song10_Ch2, Music_Song10_Ch3, Music_Song10_Ch4
Music_Song11:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song11_Ch1, Music_Song11_Ch2, Music_Song11_Ch3, Music_Song11_Ch4
Music_Song12:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song12_Ch1, Music_Song12_Ch2, Music_Song12_Ch3, Music_Song12_Ch4
Music_Song13:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song13_Ch1, Music_Song13_Ch2, Music_Song13_Ch3, Music_Song13_Ch4
Music_Song14:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song14_Ch1, Music_Song14_Ch2, Music_Song14_Ch3, Music_Song14_Ch4
Music_Song15:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song15_Ch1, Music_Song15_Ch2, Music_Song15_Ch3, Music_Song15_Ch4
Music_Song16:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song16_Ch1, Music_Song16_Ch2, Music_Song16_Ch3, Music_Song16_Ch4
Music_Song17:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song17_Ch1, Music_Song17_Ch2, Music_Song17_Ch3, Music_Song17_Ch4
Music_Song18:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song18_Ch1, Music_Song18_Ch2, Music_Song18_Ch3, Music_Song18_Ch4
Music_Song19:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song19_Ch1, Music_Song19_Ch2, Music_Song19_Ch3, Music_Song19_Ch4
Music_Song1A:
    db $00 ; transpose (0)
    dw Music_NoteLengths2 ; note length table
    dw Music_Song1A_Ch1, Music_Song1A_Ch2, Music_Song1A_Ch3, Music_Song1A_Ch4
Music_Song1B:
    db $00 ; transpose (0)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song1B_Ch1, Music_Song1B_Ch2, Music_Song1B_Ch3, Music_Song1B_Ch4
Music_Song1C:
    db $fc ; transpose (-4)
    dw Music_NoteLengths3 ; note length table
    dw Music_Song1C_Ch1, Music_Song1C_Ch2, Music_Song1C_Ch3, Music_Song1C_Ch4

; Note frequencies for NRx3/NRx4, one word per note. Note bytes are
; byte offsets into this table ($02 = first entry).
Music_FreqTable:
    dw $0f00, $002c, $009c, $0106, $016b, $01c9, $0223, $0277
    dw $02c6, $0312, $0356, $039b, $03da, $0416, $044e, $0483
    dw $04b5, $04e5, $0511, $053b, $0563, $0589, $05ac, $05ce
    dw $05ed, $060a, $0627, $0642, $065b, $0672, $0689, $069e
    dw $06b2, $06c4, $06d6, $06e7, $06f7, $0706, $0714, $0721
    dw $072d, $0739, $0744, $074f, $0759, $0762, $076b, $0773
    dw $077b, $0783, $078a, $0790, $0797, $079d, $07a2, $07a7
    dw $07ac, $07b1, $07b6, $07ba, $07be, $07c1, $07c4, $07c8
    dw $07cb, $07ce, $07d1, $07d4, $07d6, $07d9, $07db, $07dd
    dw $07df

; Noise channel notes. The note byte is an offset into this table.
; The engine copies 5 bytes from that offset to $de86-$de8a.
Music_NoiseTable:
    db $00, $00, $00, $00, $00, $c0, $a1, $00, $3a, $00, $c0, $b1, $00, $29, $01, $c0
    db $61, $00, $3a, $00, $c0, $d1, $00, $3a, $00, $c0, $f1, $00, $3a, $00, $c0, $f1
    db $00, $29, $01, $c0, $97, $00, $97, $a0, $80, $f3, $00, $2f, $00, $80

; Wave channel samples, 16 bytes (32 4-bit samples) each.
Music_Wave1:
    db $aa, $aa, $aa, $aa, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
Music_WaveUnused:
    db $01, $23, $44, $55, $67, $88, $9a, $bb, $a9, $88, $76, $55, $44, $33, $22, $11
Music_Wave2:
    db $01, $23, $45, $67, $89, $ab, $cd, $ef, $fe, $dc, $ba, $98, $76, $54, $32, $10
Music_Wave3:
    db $11, $23, $56, $78, $99, $98, $76, $67, $9a, $df, $fe, $c9, $85, $42, $11, $31

; Note length tables. A note length command $aN selects entry N.
; Unused table:
    db $01, $02, $04, $08, $10, $20, $06, $0c, $18, $02, $03, $06, $0c, $18, $30, $09
    db $12, $24, $04, $04, $10, $10
Music_NoteLengths1:
    db $02, $04, $08, $10, $20, $40, $0c, $18, $30, $05, $06, $15, $16, $01, $70
Music_NoteLengths2:
    db $03, $05, $0a, $14, $28, $50, $0f, $1e, $3c, $07, $06, $02, $04, $01
Music_NoteLengths3:
    db $03, $06, $0c, $18, $30, $60, $12, $24, $48, $08, $01, $02, $10, $04, $c0
Music_NoteLengths4:
    db $05, $07, $0e, $1c, $38, $70, $15, $2a, $54, $09, $0a, $04, $02, $03, $04
; Unused table:
    db $04, $08, $10, $20, $40, $80, $18, $30, $60, $04, $09, $12, $24, $48, $90, $1b
    db $36, $6c, $0c, $18


SECTION "Bank 4 music data", ROMX[$5b40], BANK[$4]

; Vibrato offsets. Each byte holds two signed 4-bit pitch offsets.
; MusicApplyVibrato reads the byte at (step / 2) and picks one nibble.
Music_VibratoTable:
    db $00, $00, $00, $00, $00, $00, $10, $00, $0f, $00, $00, $11, $00, $0f, $f0, $01
    db $12, $10, $ff, $ef, $01, $12, $10, $ff, $ef, $01, $12, $10, $ff, $ef, $01, $12
    db $10, $ff, $ef, $01, $12, $10, $ff, $ef, $01, $12, $10, $ff, $ef, $01, $12, $10
    db $ff, $ef, $01, $12, $10, $ff, $ef

; ---------------------------------------------------------------------------
; Music data. Each song has four channel lists (square 1, square 2, wave,
; noise). A channel list is a series of pattern pointers. $0000 ends the
; song. $ffff, addr jumps back to addr. Pattern bytes:
;   $01         rest
;   $02-$9a     note (offset into Music_FreqTable; noise: Music_NoiseTable)
;   $9b n       repeat start, n times     $9c   repeat end
;   $9d a,b,c   set instrument (wave channel: wave pointer, volume)
;   $9e ptr     set note length table      $9f n set transpose
;   $a0-$af     set note length (index into note length table)
;   $00         end of pattern
; ---------------------------------------------------------------------------

Music_Song01_Ch1:
    dw Music_Song01_Ch1_Pat0
.loop:
    dw Music_Song01_Ch1_Pat1
    dw Music_Song01_Ch1_Pat2
    dw Music_Song01_Ch1_Pat2
    dw $ffff, .loop ; loop

Music_Song01_Ch2:
    dw Music_Song01_Ch2_Pat0
.loop:
    dw Music_Song01_Ch2_Pat1
    dw $ffff, .loop ; loop

Music_Song01_Ch3:
    dw Music_Song01_Ch3_Pat0
.loop:
    dw Music_Song01_Ch3_Pat1
    dw $ffff, .loop ; loop

Music_Song01_Ch4:
    dw Music_Song01_Ch4_Pat0
.loop:
    dw Music_Song01_Ch4_Pat1
    dw $ffff, .loop ; loop

Music_Song01_Ch1_Pat0:
    db $9d, $91, $00, $c1 ; set instrument: envelope, duty
    db $a3, $01, $a1, $01, $a0, $01, $48, $4a, $4e, $52, $54, $58, $5c, $60, $62, $66
    db $6a, $6c, $70
    db $9d, $f4, $00, $c1 ; set instrument: envelope, duty
    db $9b, $04 ; repeat start
    db $a9, $28, $01, $28
    db $9c ; repeat end
    db $a3, $28, $2a, $2c, $2e
    db $00 ; end of pattern

Music_Song01_Ch2_Pat0:
    db $9d, $73, $00, $c1 ; set instrument: envelope, duty
    db $a3, $01, $a1, $01, $a0, $01, $01, $60, $62, $66, $6a, $6c, $70, $74, $78, $7a
    db $7e, $82
    db $9d, $a2, $00, $c1 ; set instrument: envelope, duty
    db $9b, $06 ; repeat start
    db $a9, $30
    db $9c ; repeat end
    db $9b, $06 ; repeat start
    db $48
    db $9c ; repeat end
    db $9b, $06 ; repeat start
    db $60
    db $9c ; repeat end
    db $9d, $a5, $00, $c1 ; set instrument: envelope, duty
    db $a4, $60
    db $00 ; end of pattern

Music_Song01_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $a3, $01, $a1, $01, $a0, $30, $32, $36, $3a, $3c, $40, $44, $48, $4a, $4e, $52
    db $54, $58, $5c
    db $9b, $06 ; repeat start
    db $a1, $36, $ab, $01
    db $9c ; repeat end
    db $9b, $06 ; repeat start
    db $a1, $4e, $ab, $01
    db $9c ; repeat end
    db $9b, $06 ; repeat start
    db $a1, $66, $ab, $01
    db $9c ; repeat end
    db $a3, $66, $a3, $01
    db $00 ; end of pattern

Music_Song01_Ch4_Pat0:
    db $a3, $01, $a1, $01
    db $9b, $0e ; repeat start
    db $a0, $15
    db $9c ; repeat end
    db $9b, $18 ; repeat start
    db $ab, $15
    db $9c ; repeat end
    db $9b, $18 ; repeat start
    db $15
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $a3, $0b
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song01_Ch1_Pat1:
    db $9d, $a4, $00, $c1 ; set instrument: envelope, duty
    db $a9, $1a, $01, $40
    db $9b, $02 ; repeat start
    db $1a, $01, $1a, $40, $01, $1a
    db $9c ; repeat end
    db $01, $01, $1a, $20, $01, $20, $22, $01, $22
    db $9b, $05 ; repeat start
    db $1a, $01, $1a
    db $9c ; repeat end
    db $01, $01, $1a, $20, $01, $20, $22, $01, $22
    db $00 ; end of pattern

Music_Song01_Ch1_Pat2:
    db $a9, $24, $01, $4a
    db $9b, $02 ; repeat start
    db $24, $01, $24, $4a, $01, $24
    db $9c ; repeat end
    db $01, $01, $24, $2a, $01, $2a, $2c, $01, $2c
    db $00 ; end of pattern

Music_Song01_Ch3_Pat1:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $9b, $02 ; repeat start
    db $a9, $40, $01, $40, $44, $01, $01, $46, $01, $44, $01, $01, $40, $01, $01, $40
    db $44, $01, $01, $40, $01, $01, $44, $01, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $4a, $01, $4a, $4e, $01, $01, $50, $01, $4e, $01, $01, $4a, $01, $01, $4a, $4e
    db $01, $01, $4a, $01, $01, $4e, $01, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song01_Ch2_Pat1:
    db $9d, $71, $00, $c0 ; set instrument: envelope, duty
    db $9b, $18 ; repeat start
    db $ad, $4a
    db $9c ; repeat end
    db $9b, $18 ; repeat start
    db $62
    db $9c ; repeat end
    db $9d, $74, $00, $00 ; set instrument: envelope, duty
    db $a9, $01, $01, $40
    db $9b, $02 ; repeat start
    db $a3, $01, $a9, $40, $01, $01
    db $9c ; repeat end
    db $a8, $01
    db $9d, $71, $00, $c0 ; set instrument: envelope, duty
    db $9b, $18 ; repeat start
    db $ad, $54
    db $9c ; repeat end
    db $9b, $18 ; repeat start
    db $6c
    db $9c ; repeat end
    db $9d, $92, $00, $00 ; set instrument: envelope, duty
    db $a9, $72, $70, $6c, $68, $6c, $68, $66, $62, $5e, $62, $5e, $5a, $5e, $5a, $54
    db $5a, $54, $50, $54, $50, $4a, $50, $4a, $46
    db $00 ; end of pattern

Music_Song01_Ch4_Pat1:
    db $a9, $0b, $01, $0b, $a4, $29, $a9, $0b, $0b, $0b, $0b, $01, $0b
    db $9b, $0c ; repeat start
    db $ab, $15
    db $9c ; repeat end
    db $a9, $0b, $0b, $0b, $0b, $01, $0b
    db $00 ; end of pattern

Music_Song08_Ch1:
    dw Music_Song08_Ch1_Pat0
    dw $0000 ; end of song

; Channel lists 2-4 have no end marker. Channel 1 ends the song first.
Music_Song08_Ch2:
    dw Music_Song08_Ch2_Pat0

Music_Song08_Ch3:
    dw Music_Song08_Ch3_Pat0

Music_Song08_Ch4:
    dw Music_Song08_Ch4_Pat0

Music_Song08_Ch1_Pat0:
    db $9d, $f1, $00, $c0 ; set instrument: envelope, duty
    db $9b, $06 ; repeat start
    db $a3, $02, $04
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $1a, $1c
    db $9c ; repeat end
    db $1a, $1c, $1e, $20
    db $00 ; end of pattern

Music_Song08_Ch2_Pat0:
    db $9d, $f2, $00, $c0 ; set instrument: envelope, duty
    db $9b, $06 ; repeat start
    db $a3, $10, $12
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $10, $12
    db $9c ; repeat end
    db $10, $12, $14, $16
    db $00 ; end of pattern

Music_Song08_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave3
    db $20
    db $9b, $06 ; repeat start
    db $a3, $1a, $1c
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $1a, $1c
    db $9c ; repeat end
    db $1a, $1c, $1e, $20
    db $00 ; end of pattern

Music_Song08_Ch4_Pat0:
    db $9b, $06 ; repeat start
    db $a4, $29
    db $9c ; repeat end
    db $9b, $08 ; repeat start
    db $a2, $29
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song1C_Ch1:
    dw Music_Song1C_Ch1_Pat0
    dw $ffff, Music_Song1C_Ch1 ; loop

Music_Song1C_Ch2:
    dw Music_Song1C_Ch2_Pat0
    dw $ffff, Music_Song1C_Ch2 ; loop

Music_Song1C_Ch3:
    dw Music_Song1C_Ch3_Pat0
    dw $ffff, Music_Song1C_Ch3 ; loop

Music_Song1C_Ch4:
    dw Music_Song1C_Ch4_Pat0
    dw $ffff, Music_Song1C_Ch4 ; loop

Music_Song06_Ch1:
    dw Music_Song06_Ch1_Pat0
.loop:
    dw Music_Song1C_Ch1_Pat0
    dw $ffff, .loop ; loop

Music_Song06_Ch2:
    dw Music_Song06_Ch2_Pat0
.loop:
    dw Music_Song1C_Ch2_Pat0
    dw $ffff, .loop ; loop

Music_Song06_Ch3:
    dw Music_Song06_Ch3_Pat0
.loop:
    dw Music_Song1C_Ch3_Pat0
    dw $ffff, .loop ; loop

Music_Song06_Ch4:
    dw Music_Song06_Ch4_Pat0
.loop:
    dw Music_Song1C_Ch4_Pat0
    dw $ffff, .loop ; loop

Music_Song06_Ch1_Pat0:
    db $9d, $92, $00, $80 ; set instrument: envelope, duty
    db $a3, $2c, $2a, $28, $26
    db $00 ; end of pattern

Music_Song06_Ch2_Pat0:
    db $9d, $a7, $00, $c0 ; set instrument: envelope, duty
    db $a3, $22, $20, $1e, $1c
    db $00 ; end of pattern

Music_Song06_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $a3, $2c, $2a, $28, $3e
    db $00 ; end of pattern

Music_Song06_Ch4_Pat0:
    db $a3, $29, $29, $a1, $1a, $1a, $1a, $1a, $a3, $29
    db $00 ; end of pattern

Music_Song1C_Ch1_Pat0:
    db $9d, $b2, $00, $80 ; set instrument: envelope, duty
    db $9b, $04 ; repeat start
    db $a2, $24, $a1, $26, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $a2, $1a, $a1, $1c, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $a2, $28, $a1, $2a, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $a2, $1e, $a1, $20, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song1C_Ch2_Pat0:
    db $9d, $c7, $00, $c0 ; set instrument: envelope, duty
    db $9b, $04 ; repeat start
    db $a2, $1a, $a1, $1c, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $24, $a1, $26, $01, $a0, $2c, $a6, $2e, $a0, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $a2, $1e, $a1, $20, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $28, $a1, $2a, $01, $a0, $2e, $a6, $30, $a0, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song1C_Ch3_Pat0:
    db $9b, $08 ; repeat start
    db $a2, $3c, $a1, $3e, $01
    db $9c ; repeat end
    db $9b, $08 ; repeat start
    db $a2, $40, $a1, $42, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song1C_Ch4_Pat0:
    db $9b, $08 ; repeat start
    db $a1, $06, $06, $06, $06, $0b, $06, $06, $06
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song09_Ch1:
    dw Music_Song09_Ch1_Pat0
    dw Music_Song09_Ch1_Pat1
    dw Music_Song09_Ch1_Pat2
    dw Music_Song09_Ch1_Pat3
    dw $0000 ; end of song

Music_Song09_Ch2:
    dw Music_Song09_Ch2_Pat0
    dw Music_Song09_Ch1_Pat1
    dw Music_Song09_Ch1_Pat2
    dw Music_Song09_Ch2_Pat3
    dw $0000 ; end of song

Music_Song09_Ch3:
    dw Music_Song09_Ch3_Pat0
    dw Music_Song09_Ch1_Pat1
    dw Music_Song09_Ch1_Pat2
    dw Music_Song09_Ch3_Pat3
    dw $0000 ; end of song

Music_Song09_Ch4:
    dw Music_Song09_Ch4_Pat0
    dw $0000 ; end of song

Music_Song09_Ch1_Pat0:
    db $9d, $77, $00, $80 ; set instrument: envelope, duty
    db $00 ; end of pattern

Music_Song09_Ch1_Pat1:
    db $ad, $04, $06, $08, $0a
    db $00 ; end of pattern

Music_Song09_Ch2_Pat0:
    db $9d, $d8, $00, $c1 ; set instrument: envelope, duty
    db $00 ; end of pattern

Music_Song09_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $00 ; end of pattern

Music_Song09_Ch1_Pat2:
    db $ad, $0c, $0e, $10, $12, $14, $16, $18, $1a, $1c, $1e, $20, $22, $24, $26, $28
    db $2a, $2c, $2e, $30, $32, $34, $36, $38, $3a, $3c, $3e, $40, $42, $44, $46, $48
    db $4a, $4c, $4e, $50, $52, $54, $56, $58, $5a, $5c, $5e, $60, $62, $64, $66, $68
    db $6a, $6c, $6e, $70, $72, $74, $76, $78, $7a, $7c, $7e, $80, $82
    db $9b, $04 ; repeat start
    db $ad, $6a, $38, $36, $01
    db $9c ; repeat end
    db $6a, $38, $36, $34, $32, $30, $2e, $2c, $2a, $28, $26, $24, $22, $20, $1e, $1c
    db $00 ; end of pattern

Music_Song09_Ch1_Pat3:
    db $9d, $a2, $00, $80 ; set instrument: envelope, duty
    db $a2, $28, $28, $a3, $01, $a2, $2a, $2a, $a3, $01, $2c, $2e, $30, $32
    db $9d, $93, $00, $c0 ; set instrument: envelope, duty
    db $9b, $02 ; repeat start
    db $a1, $32, $01, $32, $01, $4a, $01, $32, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $2c, $01, $2c, $01, $44, $01, $2c, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $30, $01, $30, $01, $48, $01, $30, $01
    db $9c ; repeat end
    db $3a, $01, $3a, $01, $52, $01, $3a, $01, $a3, $3a, $36
    db $9b, $02 ; repeat start
    db $a1, $32, $01, $32, $01, $4a, $01, $32, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $2c, $01, $2c, $01, $44, $01, $2c, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $48, $01, $30, $01, $30, $01, $48, $01, $30, $01, $30, $01, $48, $01, $30, $01
    db $9c ; repeat end
    db $9d, $b3, $00, $c0 ; set instrument: envelope, duty
    db $ad, $48, $46, $44, $42, $40, $3e, $3c, $3a, $38, $36, $34, $32, $30, $2e, $2c
    db $2a, $28, $26, $24, $22, $20, $1e, $1c, $1a, $18, $16, $14, $12, $10, $0e, $0c
    db $0a, $a5, $01, $01, $01, $01
    db $00 ; end of pattern

Music_Song09_Ch2_Pat3:
    db $9d, $d4, $00, $c1 ; set instrument: envelope, duty
    db $a3, $22, $01, $24, $01, $26, $28, $2a, $2c
    db $9d, $a4, $00, $c0 ; set instrument: envelope, duty
    db $a2, $44, $4a, $52, $60, $52, $4a, $44, $01, $44, $4a, $54, $60, $54, $4a, $44
    db $01, $48, $4e, $58, $60, $58, $4e, $48, $01, $40, $4a, $52, $60, $52, $4a, $40
    db $01, $44, $4a, $52, $60, $52, $4a, $44, $01, $44, $4a, $54, $60, $54, $4a, $44
    db $01
    db $9b, $02 ; repeat start
    db $a1, $66, $01, $4e, $01, $4e, $01, $66, $01, $4e, $01, $4e, $01, $66, $01, $4e
    db $01
    db $9c ; repeat end
    db $9d, $d4, $00, $c0 ; set instrument: envelope, duty
    db $ad, $4e, $4c, $4a, $48, $46, $44, $42, $40, $3e, $3c, $3a, $38, $36, $34, $32
    db $30, $2e, $2c, $2a, $28, $26, $24, $22, $20, $1e, $1c, $1a, $18, $16, $14, $12
    db $10, $a5, $01, $01, $01, $01
    db $00 ; end of pattern

Music_Song09_Ch3_Pat3:
    db $a1, $22, $01, $22, $01, $a3, $01, $a1, $24, $01, $24, $01, $a3, $01, $a2, $26
    db $01, $28, $01, $2a, $01, $2c, $01
    db $9b, $02 ; repeat start
    db $a1, $2c, $01, $2c, $01, $44, $01, $2c, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $24, $01, $24, $01, $3c, $01, $24, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $28, $01, $28, $01, $40, $01, $28, $01
    db $9c ; repeat end
    db $32, $01, $32, $01, $4a, $01, $32, $01, $a3, $32, $48
    db $9b, $02 ; repeat start
    db $a1, $2c, $01, $2c, $01, $44, $01, $2c, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $24, $01, $24, $01, $3c, $01, $24, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $40, $01, $28, $01, $28, $01, $40, $01, $28, $01, $28, $01, $40, $01, $28, $01
    db $9c ; repeat end
    db $ad, $40, $3e, $3c, $3a, $38, $36, $34, $32, $30, $2e, $2c, $2a, $28, $26, $24
    db $22, $20, $1e, $1c, $1a, $18, $16, $14, $12, $10, $0e, $0c, $0a, $08, $06, $04
    db $02
    db $9b, $04 ; repeat start
    db $a5, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song09_Ch4_Pat0:
    db $9b, $18 ; repeat start
    db $a1, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $1f, $1a, $1a, $1f, $1a, $1a, $1f, $1a
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a3, $1a, $1f, $a2, $1a, $1a, $a3, $1f, $a2, $1a, $1a, $1f, $a3, $1a, $a2, $1a
    db $a3, $1f
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $1f, $1a, $1a, $1f, $1a, $1a, $1f, $1a
    db $9c ; repeat end
    db $9b, $20 ; repeat start
    db $a1, $01
    db $9c ; repeat end
    db $a5, $01, $01, $01, $01
    db $00 ; end of pattern

Music_Song07_Ch1:
    dw Music_Song07_Ch1_Pat0

Music_Song07_Ch2:
    dw Music_Song07_Ch2_Pat0

Music_Song07_Ch3:
    dw Music_Song07_Ch3_Pat0

Music_Song07_Ch4:
    dw Music_Song07_Ch4_Pat0
    dw $0000 ; end of song

Music_Song07_Ch1_Pat0:
    db $9d, $67, $00, $81 ; set instrument: envelope, duty
    db $a2, $01
    db $9b, $0f ; repeat start
    db $a1, $02, $01
    db $9c ; repeat end
    db $9d, $99, $00, $81 ; set instrument: envelope, duty
    db $a2, $32, $ad, $2a, $32, $38, $a8, $42, $a2, $36, $ad, $2e, $36, $3c, $a8, $46
    db $9d, $98, $00, $81 ; set instrument: envelope, duty
    db $a4, $4a, $a3, $40, $3c, $a5, $3a
    db $00 ; end of pattern

Music_Song07_Ch2_Pat0:
    db $9d, $79, $00, $81 ; set instrument: envelope, duty
    db $a2, $01
    db $9b, $0f ; repeat start
    db $a1, $1a, $01
    db $9c ; repeat end
    db $9d, $97, $00, $81 ; set instrument: envelope, duty
    db $a2, $32, $01, $a3, $32, $a2, $01, $32, $32, $32, $36, $01, $a3, $36, $a2, $01
    db $36, $36, $36
    db $9d, $68, $00, $81 ; set instrument: envelope, duty
    db $a4, $3a, $a3, $30, $30, $a5, $32
    db $00 ; end of pattern

Music_Song07_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $a2, $01
    db $9b, $0f ; repeat start
    db $a1, $1a, $01
    db $9c ; repeat end
    db $a2, $2a, $01, $a3, $2a, $a2, $01, $2a, $2a, $2a, $2e, $01, $a3, $2e, $a2, $01
    db $2e, $2e, $2e, $a4, $32, $a3, $28, $28, $a5, $1a
    db $00 ; end of pattern

Music_Song07_Ch4_Pat0:
    db $9b, $02 ; repeat start
    db $a2, $1a, $1a, $01, $1a, $01, $1a, $01, $1a
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a3, $1a, $a2, $01, $1f, $01, $1a, $a1, $1a, $1a, $1a, $1a
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $1a, $1a, $01, $1a
    db $9c ; repeat end
    db $9b, $18 ; repeat start
    db $ab, $06
    db $9c ; repeat end
    db $9b, $18 ; repeat start
    db $06
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0A_Ch1:
    dw Music_Song0A_Ch1_Pat0
    dw Music_Song0A_Ch1_Pat1
    dw Music_Song0A_Ch1_Pat2
    dw Music_Song0A_Ch1_Pat3
    dw Music_Song0A_Ch1_Pat2
    dw Music_Song0A_Ch1_Pat5
    dw Music_Song0A_Ch1_Pat2
    dw Music_Song0A_Ch1_Pat7
    dw Music_Song0A_Ch1_Pat2

Music_Song0A_Ch2:
    dw Music_Song0A_Ch2_Pat0
    dw Music_Song0A_Ch2_Pat1
    dw Music_Song0A_Ch2_Pat2
    dw Music_Song0A_Ch2_Pat3
    dw Music_Song0A_Ch2_Pat2
    dw Music_Song0A_Ch2_Pat5
    dw Music_Song0A_Ch2_Pat2
    dw Music_Song0A_Ch2_Pat7
    dw Music_Song0A_Ch2_Pat2

Music_Song0A_Ch3:
    dw Music_Song0A_Ch3_Pat0
    dw Music_Song0A_Ch3_Pat1
    dw Music_Song0A_Ch3_Pat2
    dw Music_Song0A_Ch3_Pat3
    dw Music_Song0A_Ch3_Pat2
    dw Music_Song0A_Ch3_Pat5
    dw Music_Song0A_Ch3_Pat2
    dw Music_Song0A_Ch3_Pat2

Music_Song0A_Ch4:
    dw Music_Song0A_Ch4_Pat0
    dw Music_Song0A_Ch4_Pat1
    dw Music_Song0A_Ch4_Pat2
    dw $0000 ; end of song

Music_Song0A_Ch1_Pat0:
    db $9d, $71, $00, $80 ; set instrument: envelope, duty
    db $a2, $01, $66, $64, $62, $a1, $60, $60, $5e, $5e, $5c, $5c, $5a, $5a
    db $00 ; end of pattern

Music_Song0A_Ch2_Pat0:
    db $9d, $88, $00, $81 ; set instrument: envelope, duty
    db $a2, $01, $44, $42, $40, $3e, $3c, $3a, $38
    db $00 ; end of pattern

Music_Song0A_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $a2, $01, $1e, $1c, $1a, $18, $16, $14, $12
    db $00 ; end of pattern

Music_Song0A_Ch4_Pat0:
    db $a3, $1a, $01, $01, $01
    db $00 ; end of pattern

Music_Song0A_Ch1_Pat1:
    db $9b, $04 ; repeat start
    db $a6, $58, $a1, $66, $a3, $64, $a2, $01, $64, $a1, $64, $64, $64, $64
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0A_Ch2_Pat1:
    db $9b, $04 ; repeat start
    db $a6, $40, $a1, $4e, $a8, $4c
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0A_Ch3_Pat1:
    db $9b, $08 ; repeat start
    db $a2, $28, $40, $34, $40
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0A_Ch4_Pat1:
    db $9b, $08 ; repeat start
    db $a2, $1f, $1a, $a1, $1a, $1a, $1a, $1a
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0A_Ch1_Pat2:
    db $a6, $58, $a1, $66, $a3, $64, $a2, $01, $64, $a1, $64, $64, $64, $64
    db $00 ; end of pattern

Music_Song0A_Ch1_Pat3:
    db $9d, $51, $00, $80 ; set instrument: envelope, duty
    db $00 ; end of pattern

Music_Song0A_Ch1_Pat5:
    db $9d, $31, $00, $80 ; set instrument: envelope, duty
    db $00 ; end of pattern

Music_Song0A_Ch1_Pat7:
    db $9d, $11, $00, $80 ; set instrument: envelope, duty
    db $00 ; end of pattern

Music_Song0A_Ch2_Pat2:
    db $a6, $40, $a1, $4e, $a8, $4c
    db $00 ; end of pattern

Music_Song0A_Ch2_Pat3:
    db $9d, $58, $00, $81 ; set instrument: envelope, duty
    db $00 ; end of pattern

Music_Song0A_Ch2_Pat5:
    db $9d, $38, $00, $81 ; set instrument: envelope, duty
    db $00 ; end of pattern

Music_Song0A_Ch2_Pat7:
    db $9d, $18, $00, $81 ; set instrument: envelope, duty
    db $00 ; end of pattern

Music_Song0A_Ch3_Pat2:
    db $9b, $02 ; repeat start
    db $a2, $28, $40, $34, $40
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0A_Ch3_Pat3:
    db $9d ; set instrument: wave, volume
    dw Music_Wave2
    db $21
    db $00 ; end of pattern

Music_Song0A_Ch3_Pat5:
    db $9d ; set instrument: wave, volume
    dw Music_Wave2
    db $20
    db $00 ; end of pattern

Music_Song0A_Ch4_Pat2:
    db $9b, $04 ; repeat start
    db $a2, $1f, $1a, $a1, $1a, $1a, $1a, $1a
    db $9b, $06 ; repeat start
    db $a2, $0b, $06, $a1, $06, $06, $06, $06
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song1A_Ch4_Pat1:
    db $9b, $04 ; repeat start
    db $a2, $1f, $1a, $a1, $1a, $1a, $1a, $1a
    db $9b, $05 ; repeat start
    db $a2, $0b, $06, $a1, $06, $06, $06, $06
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song02_Ch1:
    dw Music_Song02_Ch1_Pat0
.loop:
    dw Music_Song02_Ch1_Pat1
    dw $ffff, .loop ; loop

Music_Song02_Ch2:
    dw Music_Song02_Ch2_Pat0
.loop:
    dw Music_Song02_Ch2_Pat1
    dw Music_Song02_Ch2_Pat1
    dw Music_Song02_Ch2_Pat3
    dw Music_Song02_Ch2_Pat3
    dw $ffff, .loop ; loop

Music_Song02_Ch3:
    dw Music_Song02_Ch3_Pat0
.loop:
    dw Music_Song02_Ch3_Pat1
    dw $ffff, .loop ; loop

Music_Song02_Ch4:
    dw Music_Song02_Ch4_Pat0
    dw $ffff, Music_Song02_Ch4 ; loop

Music_Song02_Ch1_Pat0:
    db $9e ; set note length table
    dw Music_NoteLengths3
    db $9d, $83, $00, $c0 ; set instrument: envelope, duty
    db $a4, $01
    db $00 ; end of pattern

Music_Song02_Ch1_Pat1:
    db $9b, $02 ; repeat start
    db $a2, $4a, $01, $50, $01, $4e, $01, $46, $01, $4a, $40, $46, $4e, $4a, $01, $a3
    db $38
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $42, $01, $50, $01, $4e, $01, $46, $01, $4a, $42, $46, $4e, $4a, $01, $a3
    db $38
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song02_Ch2_Pat0:
    db $9e ; set note length table
    dw Music_NoteLengths3
    db $9d, $d3, $00, $c1 ; set instrument: envelope, duty
    db $a4, $01
    db $00 ; end of pattern

Music_Song02_Ch2_Pat1:
    db $a1, $1a, $1a, $1a, $1a
    db $9b, $06 ; repeat start
    db $a2, $1a, $32
    db $9c ; repeat end
    db $a3, $02
    db $00 ; end of pattern

Music_Song02_Ch2_Pat3:
    db $a1, $12, $12, $12, $12
    db $9b, $06 ; repeat start
    db $a2, $12, $2a
    db $9c ; repeat end
    db $a3, $02
    db $00 ; end of pattern

Music_Song02_Ch3_Pat0:
    db $9e ; set note length table
    dw Music_NoteLengths3
    db $9d ; set instrument: wave, volume
    dw Music_Wave3
    db $21
    db $a4, $01
    db $00 ; end of pattern

Music_Song02_Ch3_Pat1:
    db $9b, $08 ; repeat start
    db $a2, $1a, $01, $20, $01
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a2, $12, $01, $1a, $01
    db $9c ; repeat end
    db $12, $01, $20, $01
    db $9b, $03 ; repeat start
    db $a2, $12, $01, $1a, $01
    db $9c ; repeat end
    db $12, $01, $20, $01
    db $00 ; end of pattern

Music_Song02_Ch4_Pat0:
    db $9e ; set note length table
    dw Music_NoteLengths3
    db $a1, $1a, $1a, $1a, $1a, $a3, $29
    db $9b, $04 ; repeat start
    db $a1, $0b, $0b, $1a, $1a, $06, $06, $06, $06, $10, $10, $10, $10, $a3, $29
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0C_Ch1:
    dw Music_Song0C_Ch1_Pat0
.loop:
    dw Music_Song0C_Ch1_Pat1
    dw $ffff, .loop ; loop

Music_Song0C_Ch2:
    dw Music_Song0C_Ch2_Pat0
.loop:
    dw Music_Song0C_Ch2_Pat1
    dw $ffff, .loop ; loop

Music_Song0C_Ch3:
    dw Music_Song0C_Ch3_Pat0
.loop:
    dw Music_Song0C_Ch3_Pat1
    dw $ffff, .loop ; loop

Music_Song0C_Ch4:
    dw Music_Song0C_Ch4_Pat0
.loop:
    dw Music_Song0C_Ch4_Pat1
    dw Music_Song0C_Ch4_Pat1
    dw $ffff, .loop ; loop
    dw $0000 ; (never reached)

Music_Song0C_Ch1_Pat0:
    db $9d, $94, $00, $c1 ; set instrument: envelope, duty
    db $a3, $01, $a3, $1a, $a9, $1a, $1c, $aa, $1e
    db $9d, $97, $00, $c1 ; set instrument: envelope, duty
    db $a5, $20, $a4, $01
    db $9e ; set note length table
    dw Music_NoteLengths3
    db $9d, $92, $00, $c1 ; set instrument: envelope, duty
    db $ab, $36, $34, $32, $30, $2e, $2c, $2a, $28, $26, $24, $22, $20, $1e, $1c, $1a
    db $18, $16, $14, $12, $10, $0e, $0c
    db $00 ; end of pattern

Music_Song0C_Ch2_Pat0:
    db $9d, $a4, $00, $c0 ; set instrument: envelope, duty
    db $a3, $01, $a3, $3a, $a9, $3a, $3c, $aa, $3e
    db $9d, $b8, $00, $c1 ; set instrument: envelope, duty
    db $a5, $40, $a4, $01
    db $9e ; set note length table
    dw Music_NoteLengths3
    db $ab, $56, $54, $52, $50, $4e, $4c, $4a, $48, $46, $44, $42, $40, $3e, $3c, $3a
    db $38, $36, $34, $32, $30, $2e, $2c
    db $00 ; end of pattern

Music_Song0C_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $a3, $01, $a3, $4a, $a9, $4a, $4c, $aa, $4e, $a5, $50, $a4, $01
    db $9e ; set note length table
    dw Music_NoteLengths3
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $ab, $4e, $4c, $4a, $48, $46, $44, $42, $40, $3e, $3c, $3a, $38, $36, $34, $32
    db $30, $2e, $2c, $2a, $28, $26, $24
    db $00 ; end of pattern

Music_Song0C_Ch4_Pat0:
    db $a3, $01, $29, $a9, $0b, $0b, $aa, $29
    db $9b, $28 ; repeat start
    db $ab, $1a
    db $9c ; repeat end
    db $a4, $01
    db $9e ; set note length table
    dw Music_NoteLengths3
    db $9b, $16 ; repeat start
    db $ab, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0C_Ch1_Pat1:
    db $9d, $b8, $00, $81 ; set instrument: envelope, duty
    db $9b, $08 ; repeat start
    db $a5, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0C_Ch2_Pat1:
    db $9d, $b4, $00, $81 ; set instrument: envelope, duty
    db $a2, $22, $a1, $22, $22, $a3, $24, $a1, $22, $01, $22, $01, $a3, $28, $a2, $22
    db $a1, $22, $22, $a3, $24, $a1, $22, $01, $22, $01, $28, $26, $24, $22
    db $9b, $04 ; repeat start
    db $a2, $22, $22, $a3, $01
    db $9c ; repeat end
    db $a2, $2c, $a1, $2c, $2c, $a3, $2e, $a1, $2c, $01, $2c, $01, $a3, $32, $a2, $2c
    db $a1, $2c, $2c, $a3, $2e, $a1, $2c, $01, $2c, $01, $32, $30, $2e, $2c
    db $9b, $04 ; repeat start
    db $a2, $2c, $2c, $a3, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0C_Ch3_Pat1:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $a2, $2c, $a1, $2c, $2c, $a3, $2e, $a1, $2c, $01, $2c, $01, $a3, $32, $a2, $2c
    db $a1, $2c, $2c, $a3, $2e, $a1, $2c, $01, $2c, $01, $32, $30, $2e, $2c
    db $9b, $04 ; repeat start
    db $a2, $2c, $2c, $a3, $01
    db $9c ; repeat end
    db $a2, $36, $a1, $36, $36, $a3, $38, $a1, $36, $01, $36, $01, $a3, $3c, $a2, $36
    db $a1, $36, $36, $a3, $38, $a1, $36, $01, $36, $01, $3c, $3a, $38, $36
    db $9b, $04 ; repeat start
    db $a2, $36, $36, $a3, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0C_Ch4_Pat1:
    db $9b, $02 ; repeat start
    db $a2, $1f, $a1, $1a, $1a, $a3, $29, $a2, $1f, $1f, $a1, $1a, $1a, $1a, $1a
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $a2, $1f, $1f, $a1, $1a, $1a, $1a, $1a
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0D_Ch1:
    dw Music_Song0D_Ch1_Pat0

Music_Song0D_Ch2:
    dw Music_Song0D_Ch2_Pat0

Music_Song0D_Ch3:
    dw Music_Song0D_Ch3_Pat0

Music_Song0D_Ch4:
    dw Music_Song0D_Ch4_Pat0
    dw $0000 ; end of song

Music_Song0D_Ch1_Pat0:
    db $9d, $f1, $00, $80 ; set instrument: envelope, duty
    db $a2, $70, $6e, $6c, $6a
    db $9d, $f1, $00, $c0 ; set instrument: envelope, duty
    db $9b, $08 ; repeat start
    db $a1, $38
    db $9c ; repeat end
    db $9b, $08 ; repeat start
    db $50
    db $9c ; repeat end
    db $9b, $08 ; repeat start
    db $68
    db $9c ; repeat end
    db $9d, $f1, $00, $80 ; set instrument: envelope, duty
    db $a1, $68, $01, $6a, $01, $6c, $01, $6e, $01
    db $00 ; end of pattern

Music_Song0D_Ch2_Pat0:
    db $9d, $ff, $00, $80 ; set instrument: envelope, duty
    db $a2, $1c, $1a, $18, $16
    db $9d, $f9, $00, $c0 ; set instrument: envelope, duty
    db $9b, $06 ; repeat start
    db $14, $01
    db $9c ; repeat end
    db $9d, $ef, $00, $80 ; set instrument: envelope, duty
    db $20, $22, $24, $26
    db $00 ; end of pattern

Music_Song0D_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $a2, $22, $20, $1e, $1c
    db $9b, $0c ; repeat start
    db $a1, $32, $01
    db $9c ; repeat end
    db $a2, $1a, $1c, $1e, $20
    db $00 ; end of pattern

Music_Song0D_Ch4_Pat0:
    db $a4, $01
    db $9b, $03 ; repeat start
    db $a2, $1f, $1f, $a1, $1a, $1a, $1a, $1a
    db $9c ; repeat end
    db $1a, $1a, $1a, $1a, $a2, $1f, $1f
    db $00 ; end of pattern

Music_Song0F_Ch1:
    dw Music_Song0F_Ch1_Pat0
    dw Music_Song0F_Ch1_Pat1
    dw Music_Song0F_Ch1_Pat0
    dw Music_Song0F_Ch1_Pat3
.loop:
    dw Music_Song0F_Ch1_Pat4
    dw Music_Song0F_Ch1_Pat4
    dw Music_Song0F_Ch1_Pat6
    dw $ffff, .loop ; loop
    dw $0000 ; (never reached)

Music_Song0F_Ch2:
    dw Music_Song0F_Ch2_Pat0
.loop:
    dw Music_Song0F_Ch2_Pat1
    dw $ffff, .loop ; loop
    dw $0000 ; (never reached)

Music_Song0F_Ch3:
    dw Music_Song0F_Ch3_Pat0
.loop:
    dw Music_Song0F_Ch3_Pat1
    dw $ffff, .loop ; loop
    dw $0000 ; (never reached)

Music_Song0F_Ch4:
    dw Music_Song0F_Ch4_Pat0
.loop:
    dw Music_Song0F_Ch4_Pat0
    dw Music_Song0F_Ch4_Pat0
    dw Music_Song0F_Ch4_Pat3
    dw $ffff, .loop ; loop
    dw $0000 ; (never reached)

Music_Song0F_Ch1_Pat0:
    db $9d, $f4, $00, $c0 ; set instrument: envelope, duty
    db $a3, $0c, $a9, $0c, $0c, $0c, $a3, $0c, $0c, $0c, $a9, $0c, $0c, $0c
    db $00 ; end of pattern

Music_Song0F_Ch1_Pat1:
    db $9b, $03 ; repeat start
    db $0c, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0F_Ch1_Pat3:
    db $9b, $06 ; repeat start
    db $0c
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0F_Ch1_Pat4:
    db $a3, $0c, $a9, $0c, $24, $0c, $a3, $0c, $0c, $0c, $a9, $0c, $02, $0c, $0c, $01
    db $02, $01, $0c, $01, $a3, $0c, $a9, $0c, $24, $0c, $a3, $0c, $0c, $0c, $a9, $0c
    db $24, $0c, $0c, $24, $0c, $0c, $24, $0c
    db $00 ; end of pattern

Music_Song0F_Ch2_Pat0:
    db $9d, $68, $00, $01 ; set instrument: envelope, duty
    db $9b, $02 ; repeat start
    db $a2, $54, $01, $a4, $4a, $a9, $54, $4a, $54, $a5, $4a
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0F_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $ad, $01, $a2, $54, $01, $a4, $4a, $a9, $54, $4a, $54, $a5, $4a, $a2, $54, $01
    db $a4, $4a, $a9, $54, $4a, $54, $a8, $4a, $a6, $01, $ab, $01
    db $00 ; end of pattern

Music_Song0F_Ch4_Pat0:
    db $a3, $0b, $a9, $0b, $0b, $0b, $a3, $0b
    db $9b, $0c ; repeat start
    db $ab, $15
    db $9c ; repeat end
    db $a3, $0b, $a9, $0b, $0b, $0b, $ac, $0b, $0b, $0b, $a3, $0b, $a9, $0b, $0b, $0b
    db $a3, $0b
    db $9b, $0c ; repeat start
    db $ab, $15
    db $9c ; repeat end
    db $a3, $0b
    db $9b, $09 ; repeat start
    db $a9, $0b
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song0F_Ch1_Pat6:
    db $9d, $f3, $00, $c0 ; set instrument: envelope, duty
    db $9b, $03 ; repeat start
    db $a3, $16, $a9, $16, $16, $16, $a3, $16, $16, $0c, $a9, $0c, $02, $0c, $a3, $0c
    db $02
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a9, $02, $01, $02, $02, $1a, $02
    db $9c ; repeat end
    db $ac, $02, $02, $04, $06, $08, $0a
    db $00 ; end of pattern

Music_Song0F_Ch2_Pat1:
    db $9b, $02 ; repeat start
    db $9d, $e6, $00, $81 ; set instrument: envelope, duty
    db $a6, $3c, $a1, $01, $a4, $32, $a9, $3c, $32, $3c, $a4, $44, $ac, $44, $3c, $44
    db $9d, $9f, $00, $81 ; set instrument: envelope, duty
    db $a4, $48, $4e, $58, $66
    db $9c ; repeat end
    db $9d, $f9, $00, $41 ; set instrument: envelope, duty
    db $36
    db $9d, $f9, $00, $41 ; set instrument: envelope, duty
    db $a2, $01, $36, $a6, $3a, $a1, $3c, $a3, $44, $40, $3c, $32
    db $9d, $f9, $00, $41 ; set instrument: envelope, duty
    db $a4, $36
    db $9d, $f9, $00, $41 ; set instrument: envelope, duty
    db $a2, $01, $36, $a6, $3a, $a1, $3c
    db $9d, $f9, $00, $41 ; set instrument: envelope, duty
    db $a5, $32
    db $9d, $c8, $00, $c1 ; set instrument: envelope, duty
    db $a4, $4e
    db $9d, $c6, $00, $c0 ; set instrument: envelope, duty
    db $a2, $01, $4e, $a6, $52, $a1, $54, $a3, $5c, $58, $54, $4a
    db $9d, $ad, $00, $81 ; set instrument: envelope, duty
    db $a5, $62
    db $9d, $a7, $00, $81 ; set instrument: envelope, duty
    db $ac, $62, $62, $64, $66, $68, $6a
    db $00 ; end of pattern

Music_Song0F_Ch3_Pat1:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $9b, $02 ; repeat start
    db $a1, $01, $a6, $3c, $a1, $01, $a4, $32, $a9, $3c, $32, $3c, $a4, $44, $ac, $44
    db $3c, $a9, $44, $ab, $01, $a4, $40, $48, $4e, $58
    db $9c ; repeat end
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $a3, $5e, $a9, $5e, $5e, $5e, $a3, $5e, $5e, $6c, $a9, $6c, $6c, $6c, $a3, $6c
    db $6c, $5e, $a9, $5e, $5e, $5e, $a3, $5e, $5e, $54, $a9, $54, $58, $5c, $ac, $5e
    db $60, $62, $a3, $5e, $a9, $5e, $5e, $5e, $a3, $5e, $5e, $6c, $a9, $6c, $6c, $6c
    db $a3, $6c, $6c, $a9, $54, $58, $5c, $60, $62, $66, $6a, $6c, $70, $74, $78, $7a
    db $ac, $3a, $3a, $3c, $3e, $40, $42
    db $00 ; end of pattern

Music_Song0F_Ch4_Pat3:
    db $9b, $03 ; repeat start
    db $a3, $0b, $ab, $15, $15, $15, $15, $15, $15, $15, $15, $15, $15, $15, $15, $a9
    db $0b, $01, $0b, $a3, $0b, $a9, $0b, $01, $0b, $ab, $15, $15, $15, $15, $15, $15
    db $15, $15, $15, $15, $15, $15, $a9, $0b, $01, $0b, $0b, $0b, $0b
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a9, $0b, $01, $0b, $0b, $0b, $0b
    db $9c ; repeat end
    db $9b, $06 ; repeat start
    db $ac, $0b
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song10_Ch1:
    dw Music_Song10_Ch1_Pat0
.loop:
    dw Music_Song10_Ch1_Pat1
    dw Music_Song10_Ch1_Pat2
    dw $ffff, .loop ; loop

Music_Song10_Ch2:
    dw Music_Song10_Ch2_Pat0
.loop:
    dw Music_Song10_Ch2_Pat1
    dw Music_Song10_Ch2_Pat2
    dw $ffff, .loop ; loop

Music_Song10_Ch3:
    dw Music_Song10_Ch3_Pat0
.loop:
    dw Music_Song10_Ch3_Pat1
    dw Music_Song10_Ch3_Pat2
    dw $ffff, .loop ; loop

Music_Song10_Ch4:
    dw Music_Song10_Ch4_Pat0
.loop:
    dw Music_Song10_Ch4_Pat1
    dw Music_Song10_Ch4_Pat1
    dw $ffff, .loop ; loop

Music_Song10_Ch1_Pat0:
    db $9d, $82, $00, $81 ; set instrument: envelope, duty
    db $a7, $62, $a2, $66, $a7, $6a, $a2, $6c, $a6, $70, $a1, $74, $a6, $76, $a1, $78
    db $a3, $7a
    db $00 ; end of pattern

Music_Song10_Ch2_Pat0:
    db $9d, $d7, $00, $80 ; set instrument: envelope, duty
    db $a7, $32, $a2, $36, $a7, $3a, $a2, $3c, $a6, $40, $a1, $44, $a6, $46, $a1, $48
    db $a3, $4a
    db $00 ; end of pattern

Music_Song10_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $a3, $4a, $a2, $01, $48, $a3, $46, $a2, $01, $44, $40, $a1, $01, $3c, $a2, $3a
    db $a1, $01, $36, $a2, $32, $01
    db $00 ; end of pattern

Music_Song10_Ch4_Pat0:
    db $a5, $01, $a2, $1a, $1a, $a1, $1a, $1a, $01, $1a, $a2, $1a, $1a
    db $00 ; end of pattern

Music_Song10_Ch1_Pat1:
    db $9d, $46, $00, $80 ; set instrument: envelope, duty
    db $a3, $01
    db $9b, $02 ; repeat start
    db $a2, $6c, $6a, $66, $62
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $5e, $5c, $58, $54
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $62, $5e, $5c, $58
    db $9c ; repeat end
    db $a2, $54, $a1, $54, $54, $a2, $62, $01, $5e, $01, $6c, $01
    db $9b, $02 ; repeat start
    db $a2, $6c, $6a, $66, $62
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $70, $6c, $6a, $66
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $62, $58, $62, $6a
    db $9c ; repeat end
    db $a2, $74, $74, $74, $74, $a3, $5c
    db $00 ; end of pattern

Music_Song10_Ch2_Pat1:
    db $9d, $d8, $00, $c1 ; set instrument: envelope, duty
    db $a2, $4a, $a1, $01, $4a, $a4, $54, $a2, $01, $54, $a6, $52, $a1, $54, $a2, $58
    db $01, $a7, $4e, $a2, $01, $a6, $4e, $a1, $4e
    db $9d, $e7, $00, $c1 ; set instrument: envelope, duty
    db $a7, $52, $a2, $4e, $a3, $4a, $a2, $5e, $01
    db $9d, $e8, $00, $c1 ; set instrument: envelope, duty
    db $a8, $5c
    db $9d, $e8, $00, $c1 ; set instrument: envelope, duty
    db $a2, $4a, $a1, $01, $4a, $a4, $54, $a2, $01, $54, $a6, $52, $a1, $54, $a2, $58
    db $01, $a7, $4e, $a2, $01, $a6, $4e, $a1, $4e
    db $9d, $e7, $00, $c1 ; set instrument: envelope, duty
    db $a3, $52, $4a, $58, $a6, $4e, $a1, $52
    db $9d, $e8, $00, $c1 ; set instrument: envelope, duty
    db $a8, $54
    db $00 ; end of pattern

Music_Song10_Ch3_Pat1:
    db $a3, $01
    db $9b, $04 ; repeat start
    db $a2, $3c, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $46, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $4a, $01
    db $9c ; repeat end
    db $9b, $08 ; repeat start
    db $3c, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $40, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $4a, $01
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $3c, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song10_Ch4_Pat1:
    db $9b, $0c ; repeat start
    db $ab, $1a
    db $9c ; repeat end
    db $a2, $1a, $1a, $a1, $1a, $1a, $01, $1a, $a2, $1a, $1a
    db $9b, $0c ; repeat start
    db $ab, $1a
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a2, $1f, $ab, $1a, $1a, $1a, $1a, $1a, $1a, $a1, $1a, $1a, $01, $1a, $a2, $1a
    db $1a, $a1, $1a, $1a, $1a, $1a, $a2, $1f, $ab, $1a, $1a, $1a, $1a, $1a, $1a, $a1
    db $1a, $1a, $01, $1a, $a2, $1a, $1a, $ab, $1a, $1a, $1a, $1a, $1a, $1a, $1a, $1a
    db $1a, $1a, $1a, $1a
    db $9c ; repeat end
    db $a2, $1a, $1a, $a1, $1a, $1a, $01, $1a, $a2, $1a, $1a
    db $00 ; end of pattern

Music_Song10_Ch1_Pat2:
    db $9d, $8d, $00, $81 ; set instrument: envelope, duty
    db $a3, $01, $a2, $01, $54, $52, $4e, $4a, $46, $44, $40, $a6, $46, $a1, $40, $a8
    db $4e, $a7, $4a, $a2, $4a, $4a, $4e, $52, $4a, $a3, $5c, $54, $01, $01, $a2, $01
    db $54, $52, $4e, $4a, $46, $44, $40, $a1, $40, $44, $46, $4a, $a8, $4e, $a2, $4a
    db $a1, $40, $4a, $a3, $52, $a2, $01, $52, $52, $52, $a8, $54
    db $00 ; end of pattern

Music_Song10_Ch2_Pat2:
    db $9d, $bf, $00, $80 ; set instrument: envelope, duty
    db $a2, $7a, $a1, $01, $7a, $a4, $84, $a2, $01, $84, $a6, $82, $a1, $84, $a2, $88
    db $01, $a7, $7e, $a2, $01, $a6, $7e, $a1, $7e, $a7, $82, $a2, $7e, $a3, $7a, $a2
    db $8e, $01
    db $9d, $c8, $00, $81 ; set instrument: envelope, duty
    db $a8, $8c
    db $9d, $bf, $00, $80 ; set instrument: envelope, duty
    db $a2, $7a, $a1, $01, $7a, $a4, $84, $a2, $01, $84, $a6, $82, $a1, $84, $a2, $88
    db $01, $a7, $7e, $a2, $01, $a6, $7e, $a1, $7e, $a3, $82, $7a, $88, $a6, $7e, $a1
    db $82
    db $9d, $c8, $00, $81 ; set instrument: envelope, duty
    db $a8, $84
    db $00 ; end of pattern

Music_Song10_Ch3_Pat2:
    db $a3, $01
    db $9b, $04 ; repeat start
    db $a2, $3c, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $46, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $4a, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $3c, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $3c, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $40, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $4a, $01
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $3c, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song12_Ch1:
    dw Music_Song12_Ch1_Pat0
    dw $ffff, Music_Song12_Ch1 ; loop

Music_Song12_Ch2:
    dw Music_Song12_Ch2_Pat0
    dw $ffff, Music_Song12_Ch2 ; loop

Music_Song12_Ch3:
    dw Music_Song12_Ch3_Pat0
    dw $ffff, Music_Song12_Ch3 ; loop

Music_Song12_Ch4:
    dw Music_Song12_Ch4_Pat0
    dw $ffff, Music_Song12_Ch4 ; loop
    dw $0000 ; (never reached)

Music_Song12_Ch1_Pat0:
    db $9d, $95, $00, $81 ; set instrument: envelope, duty
    db $9b, $08 ; repeat start
    db $a1, $20, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song12_Ch2_Pat0:
    db $9d, $f9, $00, $c1 ; set instrument: envelope, duty
    db $9b, $08 ; repeat start
    db $a1, $1a, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song12_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $9b, $08 ; repeat start
    db $a1, $1a, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song12_Ch4_Pat0:
    db $a2, $1a, $1a, $01, $1a, $01, $1a, $01, $1a
    db $00 ; end of pattern

Music_Song13_Ch1:
    dw Music_Song13_Ch1_Pat0
    dw Music_Song13_Ch1_Pat0
    dw Music_Song13_Ch1_Pat2
    dw Music_Song13_Ch1_Pat3
    dw Music_Song13_Ch1_Pat4
    dw Music_Song13_Ch1_Pat4
    dw $ffff, Music_Song13_Ch1 ; loop

Music_Song13_Ch2:
    dw Music_Song13_Ch2_Pat0
    dw Music_Song13_Ch2_Pat1
    dw Music_Song13_Ch2_Pat2
    dw Music_Song13_Ch2_Pat2
    dw $ffff, Music_Song13_Ch2 ; loop

Music_Song13_Ch3:
    dw Music_Song13_Ch3_Pat0
    dw Music_Song13_Ch3_Pat0
    dw Music_Song13_Ch3_Pat2
    dw Music_Song13_Ch3_Pat3
    dw Music_Song13_Ch3_Pat4
    dw Music_Song13_Ch3_Pat4
    dw Music_Song13_Ch3_Pat4
    dw Music_Song13_Ch3_Pat4
    dw $ffff, Music_Song13_Ch3 ; loop

Music_Song13_Ch4:
    dw Music_Song13_Ch4_Pat0
    dw Music_Song13_Ch4_Pat1
    dw Music_Song13_Ch4_Pat2
    dw Music_Song13_Ch4_Pat2
    dw Music_Song13_Ch4_Pat2
    dw Music_Song13_Ch4_Pat2
    dw $ffff, Music_Song13_Ch4 ; loop

Music_Song14_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave2
    db $21
    db $a5, $01
    db $00 ; end of pattern

Music_Song14_Ch1:
    dw Music_Song14_Ch1_Pat0
    dw $ffff, Music_Song14_Ch1 ; loop

Music_Song14_Ch2:
    dw Music_Song14_Ch1_Pat0
    dw $ffff, Music_Song14_Ch2 ; loop

Music_Song14_Ch3:
    dw Music_Song14_Ch3_Pat0
    dw $ffff, Music_Song14_Ch3 ; loop

Music_Song14_Ch4:
    dw Music_Song14_Ch4_Pat0
    dw $ffff, Music_Song14_Ch4 ; loop
    dw $0000 ; (never reached)

Music_Song14_Ch1_Pat0:
    db $9d, $b8, $00, $81 ; set instrument: envelope, duty
    db $a5, $01
    db $00 ; end of pattern

Music_Song14_Ch4_Pat0:
    db $9b, $04 ; repeat start
    db $a1, $24, $24, $24, $24
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song15_Ch1:
    dw Music_Song0C_Ch1_Pat1
    dw $ffff, Music_Song15_Ch1 ; loop

Music_Song15_Ch2:
    dw Music_Song15_Ch2_Pat0
    dw $ffff, Music_Song15_Ch2 ; loop

Music_Song15_Ch3:
    dw Music_Song15_Ch3_Pat0
    dw $ffff, Music_Song15_Ch3 ; loop

Music_Song15_Ch4:
    dw Music_Song15_Ch4_Pat0
    dw $ffff, Music_Song15_Ch4 ; loop
    dw $0000 ; (never reached)

Music_Song15_Ch2_Pat0:
    db $9d, $a2, $00, $c0 ; set instrument: envelope, duty
    db $a4, $4e, $a9, $01, $01, $48, $48, $01, $52, $a4, $4e, $a9, $01, $01, $4e, $4e
    db $01, $48, $a3, $48, $a9, $3a, $40, $48, $48, $44, $40, $a3, $44, $a3, $48, $a9
    db $3a, $40, $48, $a3, $40, $a9, $01, $44, $4a, $a3, $52, $4e, $a9, $4a, $4e, $48
    db $a3, $4a, $a3, $52, $4e, $4a, $a9, $01, $01, $4a
    db $9b, $04 ; repeat start
    db $a9, $4e, $01, $4e, $4e, $4e, $4e
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song15_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $a2, $28, $01, $36, $01, $40, $01, $36, $01, $28, $01, $36, $01, $40, $01, $a9
    db $36, $01, $40, $a2, $3a, $01, $30, $01, $3a, $01, $30, $01, $3a, $01, $30, $01
    db $3a, $01, $a9, $30, $01, $3a, $a2, $2c, $01, $3a, $01, $2c, $01, $3a, $01, $2c
    db $01, $3a, $01, $2c, $01, $a9, $3a, $01, $2c
    db $9b, $02 ; repeat start
    db $a2, $36, $01, $36, $01, $36, $01, $36, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song15_Ch4_Pat0:
    db $9b, $04 ; repeat start
    db $a9, $1f, $01, $1a, $1f, $1a, $1a, $1f, $01, $1a, $1f, $01, $1a, $a9, $1f, $01
    db $1a, $1f, $01, $1a, $1f, $01, $1a, $1f, $1a, $1a
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song16_Ch1:
    dw Music_Song16_Ch1_Pat0
    dw $0000 ; end of song

Music_Song16_Ch2:
    dw Music_Song16_Ch2_Pat0
    dw $0000 ; end of song

Music_Song16_Ch3:
    dw Music_Song16_Ch3_Pat0
    dw $0000 ; end of song

Music_Song16_Ch4:
    dw Music_Song16_Ch4_Pat0
    dw $0000 ; end of song

Music_Song16_Ch1_Pat0:
    db $9d, $e1, $00, $80 ; set instrument: envelope, duty
    db $a9, $52, $54, $56
    db $9d, $98, $00, $81 ; set instrument: envelope, duty
    db $a8, $58
    db $00 ; end of pattern

Music_Song16_Ch2_Pat0:
    db $9d, $e1, $00, $80 ; set instrument: envelope, duty
    db $ad, $32, $01, $36, $01, $38, $01
    db $9d, $68, $00, $81 ; set instrument: envelope, duty
    db $a8, $36
    db $00 ; end of pattern

Music_Song16_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave2
    db $20
    db $ad, $3a, $01, $3c, $01, $3e, $01, $a8, $40
    db $00 ; end of pattern

Music_Song16_Ch4_Pat0:
    db $a9, $1f, $1f, $1f
    db $9b, $0c ; repeat start
    db $ab, $1a
    db $9c ; repeat end
    db $9b, $0c ; repeat start
    db $1a
    db $9c ; repeat end
    db $9b, $0c ; repeat start
    db $1a
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song17_Ch1:
    dw Music_Song17_Ch1_Pat0
.loop:
    dw Music_Song17_Ch1_Pat1
    dw $ffff, .loop ; loop

Music_Song17_Ch2:
    dw Music_Song17_Ch1_Pat0
.loop:
    dw Music_Song17_Ch2_Pat1
    dw $ffff, .loop ; loop

Music_Song17_Ch3:
    dw Music_Song17_Ch3_Pat0
.loop:
    dw Music_Song17_Ch2_Pat1
    dw $ffff, .loop ; loop

Music_Song17_Ch4:
    dw Music_Song17_Ch4_Pat0
    dw $ffff, Music_Song17_Ch4 ; loop

Music_Song17_Ch1_Pat0:
    db $9d, $d7, $00, $c1 ; set instrument: envelope, duty
    db $a4, $01
    db $00 ; end of pattern

Music_Song17_Ch2_Pat1:
    db $9b, $03 ; repeat start
    db $a2, $1a, $1a, $01, $01
    db $9c ; repeat end
    db $a2, $1a, $1a, $a3, $1c
    db $9b, $03 ; repeat start
    db $a2, $1a, $1a, $01, $01
    db $9c ; repeat end
    db $a2, $1a, $1a, $a1, $1a, $18, $16, $14
    db $9b, $03 ; repeat start
    db $a2, $12, $12, $01, $01
    db $9c ; repeat end
    db $a2, $12, $12, $a3, $14
    db $9b, $03 ; repeat start
    db $a2, $12, $12, $01, $01
    db $9c ; repeat end
    db $a2, $12, $12, $a1, $14, $16, $18, $1a
    db $00 ; end of pattern

Music_Song17_Ch1_Pat1:
    db $9b, $08 ; repeat start
    db $a5, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song17_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $a4, $01
    db $00 ; end of pattern

Music_Song17_Ch4_Pat0:
    db $a1, $1a, $1a, $1a, $1a, $a3, $29
    db $00 ; end of pattern

Music_Song18_Ch1:
    dw Music_Song14_Ch1_Pat0
.loop:
    dw Music_Song18_Ch1_Pat1
    dw Music_Song18_Ch1_Pat2
    dw Music_Song18_Ch1_Pat3
    dw Music_Song18_Ch1_Pat4
    dw $ffff, .loop ; loop

Music_Song18_Ch2:
    dw Music_Song14_Ch1_Pat0
.loop:
    dw Music_Song18_Ch2_Pat1
    dw Music_Song18_Ch2_Pat2
    dw Music_Song18_Ch2_Pat2
    dw Music_Song18_Ch2_Pat4
    dw Music_Song18_Ch2_Pat4
    dw Music_Song18_Ch2_Pat4
    dw Music_Song18_Ch2_Pat4
    dw Music_Song18_Ch2_Pat8
    dw $ffff, .loop ; loop

Music_Song18_Ch3:
    dw Music_Song14_Ch3_Pat0
.loop:
    dw Music_Song18_Ch3_Pat1
    dw Music_Song18_Ch3_Pat2
    dw Music_Song18_Ch3_Pat3
    dw Music_Song18_Ch3_Pat4
    dw $ffff, .loop ; loop

Music_Song18_Ch4:
    dw Music_Song18_Ch4_Pat0
.loop:
    dw Music_Song18_Ch4_Pat1
    dw Music_Song18_Ch4_Pat1
    dw Music_Song18_Ch4_Pat1
    dw Music_Song18_Ch4_Pat1
    dw Music_Song18_Ch4_Pat1
    dw $ffff, .loop ; loop

Music_Song18_Ch4_Pat0:
    db $9b, $02 ; repeat start
    db $a1, $15, $15, $15, $15, $1f, $15, $15, $15
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song18_Ch1_Pat1:
    db $9d, $95, $00, $c1 ; set instrument: envelope, duty
    db $9b, $03 ; repeat start
    db $a2, $1a, $1a, $1a, $1a, $1a, $1a, $a3, $20
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $1a, $1a, $a3, $20
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a2, $16, $16, $16, $16, $16, $16, $a3, $1c
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $16, $16, $a3, $1c
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song18_Ch2_Pat1:
    db $9d, $95, $00, $c1 ; set instrument: envelope, duty
    db $9b, $03 ; repeat start
    db $a2, $32, $32, $32, $32, $32, $32, $a3, $26
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $32, $32, $a3, $26
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a2, $2e, $2e, $2e, $2e, $2e, $2e, $a3, $24
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $2e, $2e, $a3, $24
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song18_Ch3_Pat1:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $9b, $03 ; repeat start
    db $a1, $1a, $01, $1a, $01, $a0, $1a, $01, $1a, $01, $1a, $01, $1a, $01, $a1, $1a
    db $01, $1a, $01, $a3, $1c
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $1a, $01, $1a, $01, $a3, $1c
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a1, $16, $01, $16, $01, $a0, $16, $01, $16, $01, $16, $01, $16, $01, $a1, $16
    db $01, $16, $01, $a3, $18
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $16, $01, $16, $01, $a3, $18
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song18_Ch4_Pat1:
    db $9b, $06 ; repeat start
    db $a1, $15, $15, $15, $15, $1f, $15, $15, $15
    db $9c ; repeat end
    db $15, $15, $15, $15, $1f, $15, $15, $15, $15, $15, $15, $15, $1f, $1f, $1f, $1f
    db $00 ; end of pattern

Music_Song18_Ch1_Pat2:
    db $9d, $86, $00, $81 ; set instrument: envelope, duty
    db $9b, $02 ; repeat start
    db $a2, $01, $a2, $32, $01, $32, $01, $a2, $38, $a1, $32, $38, $a2, $40, $a1, $38
    db $40, $a1, $46, $01, $46, $40, $a2, $46
    db $9d, $89, $00, $80 ; set instrument: envelope, duty
    db $a1, $46, $01, $46, $40
    db $9d, $86, $00, $80 ; set instrument: envelope, duty
    db $a3, $46
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song18_Ch3_Pat2:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $9b, $02 ; repeat start
    db $a2, $4a, $01, $4a, $01, $a2, $50, $a1, $4a, $50, $a2, $58, $a1, $50, $58, $a1
    db $5e, $01, $5e, $58, $a8, $5e
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song18_Ch2_Pat2:
    db $9d, $f2, $00, $c0 ; set instrument: envelope, duty
    db $9b, $02 ; repeat start
    db $a1, $1a, $01, $1a, $01, $a0, $1a, $01, $1a, $01, $1a, $01, $1a, $01
    db $9c ; repeat end
    db $a1, $20, $01, $20, $01
    db $9b, $04 ; repeat start
    db $a0, $20, $01
    db $9c ; repeat end
    db $a1, $20, $01, $20, $01
    db $9b, $04 ; repeat start
    db $a0, $1e, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song18_Ch1_Pat3:
    db $9d, $a7, $00, $81 ; set instrument: envelope, duty
    db $a2, $01, $a7, $2a, $32, $a3, $38, $36, $32, $2e, $24, $a7, $32, $38, $a3, $40
    db $3c, $38, $a2, $36, $a3, $32, $a2, $2e, $a7, $2a, $32, $a3, $38, $36, $32, $2e
    db $24, $a7, $32, $38, $a3, $40, $3c, $38, $36, $a2, $2e
    db $00 ; end of pattern

Music_Song18_Ch3_Pat3:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $a7, $42, $4a, $a3, $50, $a3, $4e, $4a, $46, $3c, $a7, $4a, $50, $a3, $58, $a3
    db $54, $50, $a2, $4e, $a3, $4a, $a2, $46, $a7, $42, $4a, $a3, $50, $a3, $4e, $4a
    db $46, $3c, $a7, $4a, $50, $a3, $58, $a3, $54, $50, $4e, $46
    db $00 ; end of pattern

Music_Song18_Ch2_Pat4:
    db $9b, $02 ; repeat start
    db $a1, $12, $01, $12, $01, $a0, $12, $01, $12, $01, $12, $01, $12, $01
    db $9c ; repeat end
    db $a1, $16, $01, $16, $01
    db $9b, $04 ; repeat start
    db $a0, $16, $01
    db $9c ; repeat end
    db $a1, $16, $01, $16, $01, $a3, $01
    db $00 ; end of pattern

Music_Song18_Ch1_Pat4:
    db $9d, $93, $00, $c1 ; set instrument: envelope, duty
    db $9b, $02 ; repeat start
    db $a1, $40, $01, $40, $01, $a0, $40, $01, $40, $01, $40, $01, $40, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $3c, $01, $3c, $01, $a0, $3c, $01, $3c, $01, $3c, $01, $3c, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $32, $01, $32, $01, $a0, $32, $01, $32, $01, $32, $01, $32, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $30, $01, $30, $01, $a0, $30, $01, $30, $01, $30, $01, $30, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $40, $01, $40, $01, $a0, $40, $01, $40, $01, $40, $01, $40, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $42, $01, $42, $01, $a0, $42, $01, $42, $01, $42, $01, $42, $01
    db $9c ; repeat end
    db $9d, $92, $00, $00 ; set instrument: envelope, duty
    db $9b, $04 ; repeat start
    db $a1, $48, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $4e, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $58, $01
    db $9c ; repeat end
    db $9b, $04 ; repeat start
    db $60, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song18_Ch2_Pat8:
    db $9d, $b6, $00, $41 ; set instrument: envelope, duty
    db $a3, $38, $38, $a2, $38, $a3, $3c, $a2, $38, $a3, $36, $36, $a2, $36, $a3, $32
    db $a2, $2e, $a3, $2a, $2a, $a2, $2a, $a3, $32, $a2, $38, $a3, $36, $36, $a2, $36
    db $a3, $32, $a2, $30, $a3, $38, $a2, $01, $a3, $38, $a2, $01, $a3, $38, $a3, $3c
    db $3c, $a2, $3c, $a3, $38, $a2, $3c
    db $9d, $98, $00, $41 ; set instrument: envelope, duty
    db $a5, $40, $40
    db $00 ; end of pattern

Music_Song18_Ch3_Pat4:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $21
    db $9b, $02 ; repeat start
    db $a1, $1a, $01, $1a, $01, $a0, $1a, $01, $1a, $01, $1a, $01, $1a, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $16, $01, $16, $01, $a0, $16, $01, $16, $01, $16, $01, $16, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $12, $01, $12, $01, $a0, $12, $01, $12, $01, $12, $01, $12, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $10, $01, $10, $01, $a0, $10, $01, $10, $01, $10, $01, $10, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $1a, $01, $1a, $01, $a0, $1a, $01, $1a, $01, $1a, $01, $1a, $01
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a1, $24, $01, $24, $01, $a0, $24, $01, $24, $01, $24, $01, $24, $01
    db $9c ; repeat end
    db $9b, $08 ; repeat start
    db $a1, $40, $01, $40, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song19_Ch1:
    dw Music_Song19_Ch1_Pat0
    dw Music_Song19_Ch1_Pat1
    dw Music_Song19_Ch1_Pat1
    dw Music_Song19_Ch1_Pat3
    dw Music_Song19_Ch1_Pat3
    dw Music_Song19_Ch1_Pat5
    dw $ffff, Music_Song19_Ch1 ; loop

Music_Song19_Ch2:
    dw Music_Song19_Ch2_Pat0
    dw $ffff, Music_Song19_Ch2 ; loop

Music_Song19_Ch3:
    dw Music_Song19_Ch3_Pat0
    dw Music_Song19_Ch3_Pat1
    dw Music_Song19_Ch3_Pat1
    dw Music_Song19_Ch3_Pat3
    dw Music_Song19_Ch3_Pat3
    dw Music_Song19_Ch3_Pat5
    dw $ffff, Music_Song19_Ch3 ; loop

Music_Song19_Ch4:
    dw Music_Song19_Ch4_Pat0
    dw Music_Song19_Ch4_Pat1
    dw Music_Song19_Ch4_Pat1
    dw Music_Song19_Ch4_Pat1
    dw Music_Song19_Ch4_Pat1
    dw Music_Song19_Ch4_Pat5
    dw $ffff, Music_Song19_Ch4 ; loop

Music_Song19_Ch1_Pat0:
    db $9d, $72, $00, $81 ; set instrument: envelope, duty
    db $a2, $40, $44
    db $00 ; end of pattern

Music_Song19_Ch1_Pat1:
    db $9b, $02 ; repeat start
    db $a2, $62, $58, $5e, $58
    db $9c ; repeat end
    db $62, $58, $5e, $62, $a3, $01, $62
    db $00 ; end of pattern

Music_Song19_Ch1_Pat3:
    db $9b, $02 ; repeat start
    db $a2, $6c, $62, $68, $62
    db $9c ; repeat end
    db $6c, $62, $68, $6c, $a3, $01, $6c
    db $00 ; end of pattern

Music_Song19_Ch1_Pat5:
    db $9d, $f2, $00, $c1 ; set instrument: envelope, duty
    db $a2, $1e, $24, $01, $1e, $a4, $24, $a2, $1a, $20, $01, $1a, $a4, $20
    db $9b, $03 ; repeat start
    db $a2, $28, $2e
    db $9c ; repeat end
    db $01, $a8, $32, $a2, $01
    db $00 ; end of pattern

Music_Song19_Ch2_Pat0:
    db $9d, $f2, $00, $c1 ; set instrument: envelope, duty
    db $a3, $01
    db $9b, $06 ; repeat start
    db $a2, $02, $1a
    db $9c ; repeat end
    db $a3, $01, $02
    db $9b, $06 ; repeat start
    db $a2, $02, $1a
    db $9c ; repeat end
    db $a3, $01, $02
    db $9b, $06 ; repeat start
    db $a2, $0c, $24
    db $9c ; repeat end
    db $a3, $01, $0c
    db $9b, $06 ; repeat start
    db $a2, $0c, $24
    db $9c ; repeat end
    db $a3, $01, $0c
    db $9b, $02 ; repeat start
    db $a2, $10, $10, $28, $10
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $0c, $0c, $24, $0c
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a2, $02, $02, $1a, $02
    db $9c ; repeat end
    db $a3, $02
    db $00 ; end of pattern

Music_Song19_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $a2, $28, $2c
    db $00 ; end of pattern

Music_Song19_Ch3_Pat1:
    db $9b, $02 ; repeat start
    db $a2, $32, $28, $2e, $28
    db $9c ; repeat end
    db $32, $28, $2e, $32, $a3, $01, $a1, $58, $a6, $01
    db $00 ; end of pattern

Music_Song19_Ch3_Pat3:
    db $9b, $02 ; repeat start
    db $a2, $3c, $32, $38, $32
    db $9c ; repeat end
    db $3c, $32, $38, $3c, $a3, $01, $a1, $62, $a6, $01
    db $00 ; end of pattern

Music_Song19_Ch3_Pat5:
    db $9b, $08 ; repeat start
    db $a0, $60, $01, $01, $01
    db $9c ; repeat end
    db $9b, $08 ; repeat start
    db $a0, $5c, $01, $01, $01
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a1, $28, $01, $2e, $01
    db $9c ; repeat end
    db $a2, $01, $a8, $32, $a2, $01
    db $00 ; end of pattern

Music_Song19_Ch4_Pat0:
    db $a3, $01
    db $00 ; end of pattern

Music_Song19_Ch4_Pat1:
    db $9b, $03 ; repeat start
    db $a2, $0b, $0b, $a1, $15, $15, $15, $15
    db $9c ; repeat end
    db $a3, $01, $29
    db $00 ; end of pattern

Music_Song19_Ch4_Pat5:
    db $9b, $03 ; repeat start
    db $a2, $0b, $0b, $15, $a3, $0b, $a2, $0b, $15, $0b
    db $9c ; repeat end
    db $9b, $08 ; repeat start
    db $a1, $15
    db $9c ; repeat end
    db $a3, $29
    db $00 ; end of pattern

Music_Song1A_Ch1:
    dw Music_Song0A_Ch1_Pat0
    dw Music_Song0A_Ch1_Pat2
    dw Music_Song0A_Ch1_Pat3
    dw Music_Song0A_Ch1_Pat2
    dw Music_Song0A_Ch1_Pat7
    dw Music_Song0A_Ch1_Pat2

Music_Song1A_Ch2:
    dw Music_Song0A_Ch2_Pat0
    dw Music_Song0A_Ch2_Pat2
    dw Music_Song0A_Ch2_Pat3
    dw Music_Song0A_Ch2_Pat2
    dw Music_Song0A_Ch2_Pat7
    dw Music_Song0A_Ch2_Pat2

Music_Song1A_Ch3:
    dw Music_Song0A_Ch3_Pat0
    dw Music_Song0A_Ch3_Pat2
    dw Music_Song0A_Ch3_Pat3
    dw Music_Song0A_Ch3_Pat2
    dw Music_Song0A_Ch3_Pat5
    dw Music_Song0A_Ch3_Pat2

Music_Song1A_Ch4:
    dw Music_Song0A_Ch4_Pat0
    dw Music_Song1A_Ch4_Pat1
    dw $0000 ; end of song

Music_Song1B_Ch1:
    dw Music_Song1B_Ch1_Pat0
    dw $0000 ; end of song

Music_Song1B_Ch2:
    dw Music_Song1B_Ch2_Pat0
    dw $0000 ; end of song

Music_Song1B_Ch3:
    dw Music_Song1B_Ch3_Pat0
    dw $0000 ; end of song

Music_Song1B_Ch4:
    dw Music_Song1B_Ch4_Pat0
    dw $0000 ; end of song

Music_Song1B_Ch1_Pat0:
    db $9d, $e1, $00, $40 ; set instrument: envelope, duty
    db $a0, $4e, $4c, $4a, $46, $40, $3e, $3c, $36, $34, $32, $2e, $28
    db $00 ; end of pattern

Music_Song1B_Ch2_Pat0:
    db $9d, $d3, $00, $01 ; set instrument: envelope, duty
    db $a1, $01, $a0, $4e, $4c, $4a, $46, $40, $3e, $3c, $36, $34, $32, $2e, $28
    db $00 ; end of pattern

Music_Song1B_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $aa, $32, $34, $36, $38, $3a, $3c, $3e, $40, $42, $44, $aa, $46, $48, $4a, $4c
    db $4e, $50, $52, $54, $56, $aa, $58, $5a, $5c, $5e, $60, $62, $64, $66, $68, $6a
    db $6c, $4c, $4a, $48, $46, $44, $aa, $42, $40, $3e, $3c, $3a, $38, $36, $34, $32
    db $30
    db $00 ; end of pattern

Music_Song1B_Ch4_Pat0:
    db $a5, $01, $01
    db $00 ; end of pattern

Music_Song03_Ch1:
    dw Music_Song14_Ch1_Pat0
.loop:
    dw Music_Song03_Ch1_Pat1
    dw $ffff, .loop ; loop

Music_Song03_Ch2:
    dw Music_Song03_Ch2_Pat0
.loop:
    dw Music_Song03_Ch2_Pat1
    dw $ffff, .loop ; loop

Music_Song03_Ch3:
    dw Music_Song14_Ch3_Pat0
.loop:
    dw Music_Song03_Ch3_Pat1
    dw Music_Song03_Ch3_Pat2
    dw Music_Song03_Ch3_Pat3
    dw Music_Song03_Ch3_Pat2
    dw Music_Song03_Ch3_Pat5
    dw $ffff, .loop ; loop

Music_Song03_Ch4:
    dw Music_Song03_Ch4_Pat0
.loop:
    dw Music_Song03_Ch4_Pat1
    dw $ffff, .loop ; loop

Music_Song03_Ch2_Pat0:
    db $9d, $47, $00, $81 ; set instrument: envelope, duty
    db $a2, $32, $01, $a9, $32, $4a, $32, $a2, $32, $01, $32, $01
    db $00 ; end of pattern

Music_Song03_Ch4_Pat0:
    db $a3, $15, $a9, $15, $15, $15, $a6, $15, $a1, $15
    db $9b, $04 ; repeat start
    db $ab, $06, $06, $06
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song03_Ch1_Pat1:
    db $9d, $58, $00, $81 ; set instrument: envelope, duty
    db $9b, $02 ; repeat start
    db $a8, $4a, $a6, $48, $a1, $4a, $a2, $4e, $a1, $01, $4e, $a4, $4e, $a3, $52
    db $9c ; repeat end
    db $9d, $94, $00, $c1 ; set instrument: envelope, duty
    db $a3, $44, $a9, $44, $44, $44, $a6, $44, $a1, $44, $a3, $5c, $58, $54, $52, $4e
    db $40, $a9, $40, $40, $40, $a6, $40, $a1, $40, $a3, $58, $54, $52, $4e, $4a, $44
    db $a9, $44, $44, $44, $a6, $44, $a1, $44, $a3, $5c, $48, $a9, $48, $48, $48, $a6
    db $48, $a1, $48, $a3, $60, $40, $a9, $40, $40, $40, $a6, $40, $a1, $40, $a3, $58
    db $9d, $b4, $00, $c1 ; set instrument: envelope, duty
    db $9b, $06 ; repeat start
    db $a9, $40, $58
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song03_Ch2_Pat1:
    db $9d, $68, $00, $81 ; set instrument: envelope, duty
    db $9b, $02 ; repeat start
    db $a8, $52, $a6, $4e, $a1, $52, $a2, $54, $a1, $01, $54, $a4, $54, $a3, $58
    db $9c ; repeat end
    db $9d, $88, $00, $81 ; set instrument: envelope, duty
    db $a7, $3c, $44, $a3, $52, $a7, $4e, $4a, $a3, $48, $a7, $40, $48, $a3, $40, $a8
    db $4a, $a3, $01, $a7, $3c, $44, $a3, $52, $a7, $4e, $4a, $a3, $48, $a7, $40, $48
    db $a3, $4a, $a4, $58, $01
    db $00 ; end of pattern

Music_Song03_Ch3_Pat1:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $9b, $04 ; repeat start
    db $a2, $32, $01, $a9, $32, $4a, $32, $a2, $32, $01, $4a, $01
    db $9c ; repeat end
    db $a2, $3c, $01, $a9, $3c, $3c, $3c, $a2, $3c, $01, $3c, $01
    db $00 ; end of pattern

Music_Song03_Ch3_Pat2:
    db $40, $01, $a9, $40, $40, $40, $a2, $40, $01, $40, $01, $3a, $01, $a9, $3a, $3a
    db $3a, $a2, $3a, $01, $3a, $01
    db $00 ; end of pattern

Music_Song03_Ch3_Pat3:
    db $9b, $02 ; repeat start
    db $3c, $01, $a9, $3c, $3c, $3c, $a2, $3c, $01, $3c, $01
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song03_Ch3_Pat5:
    db $40, $01, $a9, $40, $40, $40, $a2, $40, $01, $40, $01
    db $00 ; end of pattern

Music_Song03_Ch4_Pat1:
    db $9b, $0c ; repeat start
    db $a3, $15, $a9, $15, $15, $15, $a6, $15, $a1, $15, $ab, $06, $06, $06, $06, $06
    db $06, $06, $06, $06, $06, $06, $06
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song05_Ch1:
    dw Music_Song05_Ch1_Pat0
.loop:
    dw Music_Song05_Ch1_Pat1
    dw Music_Song05_Ch1_Pat1
    dw Music_Song13_Ch1_Pat2
    dw Music_Song05_Ch1_Pat4
    dw Music_Song05_Ch1_Pat5
    dw Music_Song05_Ch1_Pat5
    dw $ffff, .loop ; loop

Music_Song05_Ch2:
    dw Music_Song05_Ch2_Pat0
.loop:
    dw Music_Song05_Ch2_Pat1
    dw Music_Song05_Ch2_Pat2
    dw Music_Song05_Ch2_Pat3
    dw Music_Song05_Ch2_Pat3
    dw $ffff, .loop ; loop

Music_Song05_Ch3:
    dw Music_Song05_Ch3_Pat0
.loop:
    dw Music_Song05_Ch3_Pat1
    dw Music_Song13_Ch3_Pat2
    dw Music_Song05_Ch3_Pat3
    dw Music_Song05_Ch3_Pat4
    dw Music_Song05_Ch3_Pat4
    dw $ffff, .loop ; loop

Music_Song05_Ch4:
    dw Music_Song05_Ch4_Pat0
.loop:
    dw Music_Song05_Ch4_Pat1
    dw Music_Song05_Ch4_Pat2
    dw Music_Song05_Ch4_Pat3
    dw Music_Song05_Ch4_Pat3
    dw $ffff, .loop ; loop

; Unused pattern. No channel list points here.
    db $a2, $0b, $ab, $15, $15, $15, $15, $15, $15, $9b, $03, $a9, $0b, $15, $15, $9c
    db $00

Music_Song05_Ch1_Pat0:
    db $9d, $57, $00, $80 ; set instrument: envelope, duty
    db $a5, $02, $02
    db $00 ; end of pattern

Music_Song05_Ch2_Pat0:
    db $9d, $85, $00, $c0 ; set instrument: envelope, duty
    db $a2, $02, $01, $a9, $02, $02, $02
    db $9b, $03 ; repeat start
    db $a2, $02, $01
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a9, $02, $02, $02
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song05_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave2
    db $20
    db $a2, $02, $01, $a9, $02, $02, $02
    db $9b, $03 ; repeat start
    db $a2, $02, $01
    db $9c ; repeat end
    db $9b, $03 ; repeat start
    db $a9, $02, $02, $02
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song05_Ch4_Pat0:
    db $a2, $0b, $ab, $15, $15, $15, $15, $15, $15, $a9, $0b, $15, $15, $a6, $15, $a1
    db $15, $a3, $0b, $a2, $0b, $ab, $15, $15, $15, $15, $15, $15, $a9, $0b, $15, $15
    db $a9, $0b, $15, $15, $0b, $15, $15
    db $00 ; end of pattern

Music_Song05_Ch1_Pat5:
    db $9d, $57, $00, $80 ; set instrument: envelope, duty
    db $a8, $02
    db $9d, $5b, $00, $80 ; set instrument: envelope, duty
    db $a3, $08
    db $9d, $57, $00, $80 ; set instrument: envelope, duty
    db $a5, $02
    db $00 ; end of pattern

Music_Song05_Ch2_Pat3:
    db $9d, $56, $00, $80 ; set instrument: envelope, duty
    db $a2, $1a, $01, $a9, $1a, $1a, $1a, $a2, $1a, $01
    db $9d, $5b, $00, $80 ; set instrument: envelope, duty
    db $a3, $1c
    db $9d, $56, $00, $80 ; set instrument: envelope, duty
    db $a2, $1a, $01
    db $9b, $03 ; repeat start
    db $a9, $1a, $1a, $1a
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song05_Ch3_Pat4:
    db $a2, $1a, $01, $a9, $1a, $1a, $1a, $a2, $1a, $01, $a3, $1c, $a2, $1a, $01
    db $9b, $03 ; repeat start
    db $a9, $1a, $1a, $1a
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song05_Ch4_Pat3:
    db $a2, $0b, $ab, $15, $15, $15, $15, $15, $15, $a9, $0b, $15, $15, $a6, $15, $a1
    db $15, $a3, $0b, $a2, $0b, $ab, $15, $15, $15, $15, $15, $15, $a9, $0b, $15, $15
    db $a9, $0b, $15, $15, $0b, $15, $15
    db $00 ; end of pattern

Music_Song05_Ch1_Pat1:
    db $9d, $73, $00, $c0 ; set instrument: envelope, duty
    db $9b, $02 ; repeat start
    db $a2, $38, $a1, $01, $38, $a9, $32, $38, $40, $40, $38, $40, $a3, $4a
    db $9c ; repeat end
    db $9b, $02 ; repeat start
    db $a2, $40, $a1, $01, $40, $a9, $38, $40, $46, $46, $40, $46, $a3, $50
    db $9c ; repeat end
    db $a2, $2e, $a1, $01, $2e, $a9, $2e, $36, $3c, $a2, $46, $a1, $01, $46, $a9, $46
    db $3c, $36, $a2, $36, $a1, $01, $36, $a9, $2e, $36, $3c, $3c, $36, $3c, $a3, $46
    db $9b, $02 ; repeat start
    db $a2, $38, $a1, $01, $38, $a9, $32, $38, $40, $40, $38, $40, $a3, $4a
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song05_Ch2_Pat1:
    db $9d, $97, $00, $81 ; set instrument: envelope, duty
    db $a8, $32, $a9, $32, $38, $40, $a7, $46, $40, $a3, $38, $a8, $40, $a6, $40, $a1
    db $46, $a7, $4e, $50, $a3, $54, $a8, $46, $a6, $46, $a1, $4a, $a4, $4e, $ac, $54
    db $50, $4e, $a8, $4e, $a2, $4e, $a1, $50, $4e, $a5, $4a, $a8, $32, $a9, $32, $38
    db $40, $a7, $46, $40, $a3, $38, $a8, $40, $a9, $40, $46, $4e, $a7, $54, $a3, $50
    db $54, $a2, $58
    db $9d, $68, $00, $81 ; set instrument: envelope, duty
    db $a4, $5a, $a7, $58, $a1, $5a, $58, $a4, $54
    db $9d, $84, $00, $81 ; set instrument: envelope, duty
    db $a3, $5e, $5a
    db $9d, $68, $00, $81 ; set instrument: envelope, duty
    db $a4, $58, $a7, $4e, $a2, $50, $a5, $4a
    db $00 ; end of pattern

Music_Song05_Ch3_Pat1:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $9b, $02 ; repeat start
    db $a2, $32, $a1, $01, $32, $a2, $32, $a1, $01, $32, $a2, $32, $a1, $01, $32, $a9
    db $32, $38, $46, $a2, $32, $a1, $01, $32, $a2, $32, $a1, $01, $32, $a2, $32, $a1
    db $01, $32, $a9, $32, $38, $46, $a2, $38, $a1, $01, $38, $a2, $38, $a1, $01, $38
    db $a2, $38, $a1, $01, $38, $a9, $50, $46, $40, $a2, $38, $a1, $01, $38, $a2, $38
    db $a1, $01, $38, $a2, $38, $a1, $01, $38, $a9, $50, $46, $40, $a2, $2e, $a1, $01
    db $2e, $a2, $2e, $a1, $01, $2e, $a2, $2e, $a1, $01, $2e, $a9, $2e, $38, $3c, $a2
    db $2e, $a1, $01, $2e, $a2, $2e, $a1, $01, $2e, $a2, $2e, $a1, $01, $2e, $a9, $46
    db $3c, $38, $a2, $32, $a1, $01, $32, $a2, $32, $a1, $01, $32, $a2, $32, $a1, $01
    db $32, $a9, $32, $38, $40, $a2, $32, $a1, $01, $32, $a2, $32, $a1, $01, $32, $a2
    db $32, $a1, $01, $32, $a9, $32, $38, $40
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song05_Ch4_Pat1:
    db $9b, $07 ; repeat start
    db $a2, $0b, $ab, $15, $15, $15, $15, $15, $15, $a9, $0b, $15, $15, $a6, $15, $a1
    db $15, $a3, $0b, $a2, $0b, $ab, $15, $15, $15, $15, $15, $15, $a9, $0b, $15, $15
    db $a9, $0b, $15, $15, $0b, $15, $15
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song13_Ch1_Pat2:
    db $9d, $7d, $00, $81 ; set instrument: envelope, duty
    db $a2, $40, $01, $a9, $38, $40, $46, $50, $46, $28, $46, $50, $40, $a2, $46, $01
    db $a9, $40, $46, $4e, $58, $4e, $46, $4e, $58, $5e, $a2, $4a, $01, $a9, $42, $4a
    db $50, $5a, $50, $4a, $50, $5a, $62, $a2, $4e, $01, $a9, $46, $4e, $54, $5a, $54
    db $4e, $a6, $54, $a1, $5a, $a2, $40, $01, $a9, $38, $40, $46, $50, $46, $28, $46
    db $50, $40, $a2, $46, $01, $a9, $40, $46, $4e, $58, $4e, $46, $4e, $58, $5e
    db $00 ; end of pattern

Music_Song05_Ch1_Pat4:
    db $9d, $7b, $00, $81 ; set instrument: envelope, duty
    db $a2, $4a, $01, $a9, $42, $4a, $50, $5a, $50, $4a, $a3, $42, $a2, $48, $01, $a9
    db $42, $48, $50, $5a, $50, $48, $a3, $42
    db $00 ; end of pattern

Music_Song05_Ch2_Pat2:
    db $9d, $97, $00, $81 ; set instrument: envelope, duty
    db $a3, $01, $a9, $38, $40, $46, $a4, $50, $a3, $01, $a9, $40, $46, $4e, $a4, $54
    db $a3, $01, $a9, $42, $4a, $50, $a4, $58
    db $9d, $97, $00, $81 ; set instrument: envelope, duty
    db $a7, $58, $a1, $5a, $58, $a3, $54, $58
    db $9d, $97, $00, $81 ; set instrument: envelope, duty
    db $a4, $50, $a2, $4e, $a3, $50, $a2, $54, $a8, $46, $a9, $46, $4a, $4e, $a3, $54
    db $a4, $50
    db $9d, $8b, $00, $81 ; set instrument: envelope, duty
    db $a9, $50, $54, $58, $a8, $5a, $a3, $01
    db $00 ; end of pattern

Music_Song13_Ch3_Pat2:
    db $a2, $38, $01, $a7, $38, $a2, $01, $a9, $38, $40, $46, $a2, $28, $01, $a7, $28
    db $a2, $01, $a9, $28, $2e, $36, $a2, $2a, $01, $a7, $2a, $a2, $01, $a9, $2a, $32
    db $38, $a2, $2e, $01, $a7, $2e, $a2, $01, $a9, $2e, $36, $3c, $a2, $38, $01, $a7
    db $38, $a2, $01, $a9, $38, $40, $46, $a2, $28, $01, $a7, $28, $a2, $01, $a9, $36
    db $2e, $28, $a2, $2a, $01, $a7, $2a, $a2, $01, $a9, $38, $32, $2a
    db $00 ; end of pattern

Music_Song05_Ch3_Pat3:
    db $a2, $2a, $01, $a7, $2a, $a2, $01, $a9, $38, $30, $2a
    db $00 ; end of pattern

Music_Song05_Ch4_Pat2:
    db $9b, $08 ; repeat start
    db $a2, $0b, $ab, $15, $15, $15, $15, $15, $15, $a9, $0b, $15, $15, $a6, $15, $a1
    db $15, $a3, $0b
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song11_Ch1:
    dw Music_Song05_Ch1_Pat1
    dw Music_Song05_Ch1_Pat1
    dw Music_Song13_Ch1_Pat2
    dw Music_Song05_Ch1_Pat4
    dw Music_Song05_Ch1_Pat5
    dw Music_Song05_Ch1_Pat5
    dw $ffff, Music_Song05_Ch1 ; loop

Music_Song11_Ch2:
    dw Music_Song05_Ch2_Pat1
    dw Music_Song05_Ch2_Pat2
    dw Music_Song05_Ch2_Pat3
    dw Music_Song05_Ch2_Pat3
    dw $ffff, Music_Song05_Ch2 ; loop

Music_Song11_Ch3:
    dw Music_Song05_Ch3_Pat1
    dw Music_Song13_Ch3_Pat2
    dw Music_Song05_Ch3_Pat3
    dw Music_Song05_Ch3_Pat4
    dw Music_Song05_Ch3_Pat4
    dw $ffff, Music_Song05_Ch3 ; loop

Music_Song11_Ch4:
    dw Music_Song05_Ch4_Pat1
    dw Music_Song05_Ch4_Pat2
    dw Music_Song05_Ch4_Pat3
    dw Music_Song05_Ch4_Pat3
    dw $ffff, Music_Song05_Ch4 ; loop

Music_Song04_Ch1:
    dw Music_Song04_Ch1_Pat0
.loop:
    dw Music_Song13_Ch1_Pat0
    dw Music_Song13_Ch1_Pat0
    dw Music_Song13_Ch1_Pat2
    dw Music_Song13_Ch1_Pat3
    dw Music_Song13_Ch1_Pat4
    dw Music_Song13_Ch1_Pat4
    dw $ffff, .loop ; loop

Music_Song04_Ch2:
    dw Music_Song04_Ch2_Pat0
.loop:
    dw Music_Song13_Ch2_Pat0
    dw Music_Song13_Ch2_Pat1
    dw Music_Song13_Ch2_Pat2
    dw Music_Song13_Ch2_Pat2
    dw $ffff, .loop ; loop

Music_Song04_Ch3:
    dw Music_Song04_Ch3_Pat0
.loop:
    dw Music_Song13_Ch3_Pat0
    dw Music_Song13_Ch3_Pat0
    dw Music_Song13_Ch3_Pat2
    dw Music_Song13_Ch3_Pat3
    dw Music_Song13_Ch3_Pat4
    dw Music_Song13_Ch3_Pat4
    dw Music_Song13_Ch3_Pat4
    dw Music_Song13_Ch3_Pat4
    dw $ffff, .loop ; loop

Music_Song04_Ch4:
    dw Music_Song04_Ch4_Pat0
.loop:
    dw Music_Song13_Ch4_Pat0
    dw Music_Song13_Ch4_Pat1
    dw Music_Song13_Ch4_Pat2
    dw Music_Song13_Ch4_Pat2
    dw Music_Song13_Ch4_Pat2
    dw Music_Song13_Ch4_Pat2
    dw $ffff, .loop ; loop

Music_Song04_Ch1_Pat0:
    db $9d, $62, $00, $81 ; set instrument: envelope, duty
    db $a3, $60, $62, $64, $66
    db $00 ; end of pattern

Music_Song04_Ch2_Pat0:
    db $9d, $f2, $00, $c0 ; set instrument: envelope, duty
    db $a3, $18, $1a, $1c, $1e
    db $00 ; end of pattern

Music_Song04_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $a3, $30, $32, $34, $36
    db $00 ; end of pattern

Music_Song04_Ch4_Pat0:
    db $a3, $29, $29, $29, $29
    db $00 ; end of pattern

Music_Song13_Ch1_Pat4:
    db $9d, $3d, $00, $81 ; set instrument: envelope, duty
    db $a2, $40, $01, $a9, $38, $40, $46, $50, $46, $28, $46, $50, $40
    db $9d, $5d, $00, $81 ; set instrument: envelope, duty
    db $a2, $46, $01, $a9, $40, $46, $4e, $58, $4e, $46, $4e, $58, $5e
    db $9d, $7d, $00, $81 ; set instrument: envelope, duty
    db $a2, $4a, $01, $a9, $42, $4a, $50, $5a, $50, $4a, $50, $5a, $62
    db $9d, $9d, $00, $81 ; set instrument: envelope, duty
    db $a2, $4e, $01, $a9, $46, $4e, $54, $5a, $54, $4e, $a6, $54, $a1, $5a
    db $00 ; end of pattern

Music_Song13_Ch2_Pat2:
    db $9d, $26, $00, $80 ; set instrument: envelope, duty
    db $a2, $20, $01, $a9, $20, $20, $20, $a2, $20, $01
    db $9d, $2b, $00, $80 ; set instrument: envelope, duty
    db $a3, $20
    db $9d, $26, $00, $80 ; set instrument: envelope, duty
    db $a2, $20, $01
    db $9b, $03 ; repeat start
    db $a9, $20, $20, $20
    db $9c ; repeat end
    db $9d, $86, $00, $80 ; set instrument: envelope, duty
    db $a2, $20, $01, $a9, $20, $20, $20, $a2, $20, $01
    db $9d, $9b, $00, $80 ; set instrument: envelope, duty
    db $a3, $20
    db $9d, $a6, $00, $80 ; set instrument: envelope, duty
    db $a2, $20, $01
    db $9b, $03 ; repeat start
    db $a9, $20, $20, $20
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song13_Ch3_Pat4:
    db $a2, $20, $01, $a9, $20, $20, $20, $a2, $20, $01, $a3, $20, $a2, $20, $01
    db $9b, $03 ; repeat start
    db $a9, $20, $20, $20
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song13_Ch4_Pat2:
    db $a2, $1f, $ab, $1a, $1a, $1a, $1a, $1a, $1a, $a9, $1f, $1a, $1a, $a6, $1a, $a1
    db $1a, $a3, $1f, $a2, $1f, $ab, $1a, $1a, $1a, $1a, $1a, $1a, $a9, $1f, $1a, $1a
    db $a9, $1f, $1a, $1a, $1f, $1a, $1a
    db $00 ; end of pattern

Music_Song13_Ch1_Pat0:
    db $9d, $62, $00, $81 ; set instrument: envelope, duty
    db $9b, $0c ; repeat start
    db $a9, $58
    db $9c ; repeat end
    db $9b, $0c ; repeat start
    db $5e
    db $9c ; repeat end
    db $9b, $0c ; repeat start
    db $62
    db $9c ; repeat end
    db $9b, $0c ; repeat start
    db $66
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song13_Ch2_Pat0:
    db $9d, $a7, $00, $c1 ; set instrument: envelope, duty
    db $a4, $38, $a3, $38, $a6, $40, $a1, $3c, $a3, $38, $a4, $36, $a3, $2e, $a4, $32
    db $a3, $32, $a6, $3c, $a1, $38, $a7, $36, $a2, $32, $a4, $2e, $a4, $38, $a6, $38
    db $a1, $40, $a6, $46, $a1, $40, $a7, $4e, $a2, $4a, $a3, $46, $40, $a3, $3c, $42
    db $4a, $50, $a8, $4e
    db $9d, $71, $00, $81 ; set instrument: envelope, duty
    db $a0, $4e, $52, $54, $58, $5c, $60, $62, $66
    db $00 ; end of pattern

Music_Song13_Ch3_Pat0:
    db $9d ; set instrument: wave, volume
    dw Music_Wave1
    db $20
    db $9b, $03 ; repeat start
    db $a9, $38, $01, $38
    db $9c ; repeat end
    db $38, $40, $46
    db $9b, $03 ; repeat start
    db $40, $01, $40
    db $9c ; repeat end
    db $40, $46, $4e
    db $9b, $03 ; repeat start
    db $42, $01, $42
    db $9c ; repeat end
    db $42, $4a, $50
    db $9b, $03 ; repeat start
    db $46, $01, $46
    db $9c ; repeat end
    db $46, $4e, $54
    db $00 ; end of pattern

Music_Song13_Ch4_Pat0:
    db $9b, $08 ; repeat start
    db $a2, $1f, $ab, $1a, $1a, $1a, $1a, $1a, $1a, $a9, $1f, $1a, $1a, $a6, $1a, $a1
    db $1a, $a3, $29
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song13_Ch1_Pat3:
    db $9d, $7b, $00, $81 ; set instrument: envelope, duty
    db $9b, $02 ; repeat start
    db $a2, $4a, $01, $a9, $42, $4a, $50, $5a, $50, $4a, $a3, $42
    db $9c ; repeat end
    db $00 ; end of pattern

Music_Song13_Ch2_Pat1:
    db $9d, $87, $00, $81 ; set instrument: envelope, duty
    db $a4, $68, $a2, $68, $a3, $66, $a2, $68, $a8, $5e, $a3, $58, $a7, $5a, $a2, $58
    db $a3, $54, $a6, $66, $a1, $62, $a8, $5e, $a9, $5e, $62, $66, $a4, $68, $a2, $68
    db $a3, $66, $a2, $68, $a7, $66, $a2, $62, $a3, $5e, $58, $a4, $5a, $a2, $01, $5a
    db $a6, $58, $a1, $54, $a4, $50, $4e
    db $00 ; end of pattern

Music_Song13_Ch3_Pat3:
    db $a2, $2e, $01, $a7, $2e, $a2, $01, $a9, $2e, $36, $3c
    db $00 ; end of pattern

Music_Song13_Ch4_Pat1:
    db $9b, $08 ; repeat start
    db $a2, $1f, $ab, $1a, $1a, $1a, $1a, $1a, $1a, $a9, $1f, $1a, $1a, $a6, $1a, $a1
    db $1a, $a3, $29
    db $9c ; repeat end
    db $00 ; end of pattern

; Unused bytes to the end of the bank. Nothing reads them.
    db $aa, $7b, $a2, $3f, $a8, $b9, $a2, $af, $8a, $d9, $a8, $be, $aa, $de, $20, $ff
    db $0a, $bd, $82, $27, $a3, $d5, $2a, $b2, $aa, $7d, $aa, $fe, $8a, $af, $2a, $5f
    db $aa, $76, $a2, $ed, $aa, $0b, $2a, $ed, $aa, $bd, $2a, $4f, $e8, $79, $a8, $b7
    db $88, $74, $aa, $d6, $a8, $57, $aa, $ff, $aa, $93, $a8, $ff, $82, $9a, $a8, $17
    db $0a, $3f, $8a, $e6, $aa, $ad, $2a, $b6, $a6, $fd, $8a, $ef, $28, $2c, $a2, $49
    db $0a, $64, $a2, $fd, $a8, $9d, $a2, $fb, $2e, $ff, $98, $f7, $22, $ea, $a8, $ff
    db $a8, $7d, $8a, $ce, $aa, $f7, $28, $ff, $aa, $bf

; $630 bytes of sprite tiles. Bank 0 copies them to VRAM $8010.
GameA_VictorySpritesLevel3Gfx:
    db $00, $00, $00, $00, $01, $00
    db $00, $01, $00, $01, $00, $01, $00, $01, $00, $01, $00, $00, $00, $00, $00, $00
    db $00, $c0, $00, $f8, $00, $f8, $00, $f8, $00, $38, $00, $01, $00, $01, $00, $01
    db $02, $01, $00, $03, $00, $03, $00, $02, $00, $00, $00, $00, $00, $00, $00, $00
    db $80, $00, $00, $80, $00, $80, $00, $80, $00, $00, $00, $07, $0f, $16, $37, $5a
    db $51, $e8, $50, $ec, $54, $ea, $56, $eb, $57, $ea, $00, $c0, $c0, $b0, $d0, $ac
    db $10, $2a, $10, $6a, $50, $aa, $d0, $aa, $d0, $aa, $57, $ea, $57, $ea, $57, $ea
    db $57, $ea, $57, $ea, $57, $ea, $57, $ea, $57, $ea, $d0, $aa, $d0, $aa, $d0, $aa
    db $d0, $aa, $d0, $aa, $d0, $aa, $d0, $aa, $d0, $aa, $00, $10, $00, $38, $00, $10
    db $00, $7c, $00, $10, $00, $10, $00, $38, $00, $38, $00, $00, $00, $7f, $7f, $ff
    db $7f, $c0, $00, $ff, $00, $80, $7f, $c0, $7f, $c0, $00, $ff, $ff, $ff, $ff, $00
    db $00, $ff, $00, $00, $ff, $00, $ff, $00, $00, $ff, $55, $6a, $55, $68, $55, $68
    db $57, $6a, $57, $6a, $55, $6a, $55, $68, $55, $28, $50, $ab, $50, $2b, $50, $2b
    db $d0, $ab, $d0, $aa, $50, $ab, $50, $2b, $50, $2b, $00, $fe, $fe, $1f, $ff, $01
    db $01, $fe, $00, $01, $fe, $00, $ff, $00, $01, $fe, $00, $00, $00, $fc, $fc, $fe
    db $fc, $06, $00, $fe, $00, $02, $fc, $06, $fc, $06, $00, $00, $00, $07, $03, $00
    db $00, $05, $01, $02, $00, $00, $00, $00, $00, $00, $00, $ff, $ff, $0c, $54, $01
    db $ae, $51, $54, $ab, $00, $15, $00, $00, $00, $00, $00, $ff, $00, $80, $7e, $c1
    db $7e, $c1, $00, $ff, $00, $55, $00, $2a, $00, $00, $07, $07, $ff, $f8, $fc, $03
    db $84, $7b, $84, $7a, $84, $4a, $04, $8a, $00, $04, $ff, $ff, $ff, $00, $21, $de
    db $21, $de, $21, $52, $21, $52, $21, $52, $00, $21, $ff, $ff, $ff, $00, $08, $f7
    db $08, $f7, $08, $94, $08, $94, $08, $94, $00, $08, $c0, $c1, $fe, $3e, $7e, $81
    db $40, $bf, $42, $bd, $42, $a5, $40, $a6, $00, $44, $00, $ff, $00, $03, $fc, $07
    db $fc, $07, $00, $ff, $00, $56, $00, $a8, $00, $00, $00, $fe, $3e, $d9, $55, $00
    db $2b, $d4, $14, $ea, $00, $50, $00, $00, $00, $00, $00, $00, $00, $c0, $80, $60
    db $00, $40, $00, $a0, $00, $00, $00, $00, $00, $00, $00, $07, $0c, $15, $12, $70
    db $4f, $e2, $5f, $c7, $4f, $eb, $5f, $d7, $4f, $e3, $00, $c0, $20, $30, $d0, $cc
    db $90, $2a, $d0, $9a, $e0, $8a, $e0, $42, $c0, $9a, $5f, $c9, $4f, $e6, $57, $f0
    db $52, $e1, $55, $ec, $57, $ea, $57, $ea, $57, $ea, $e0, $8a, $e0, $4a, $d0, $8a
    db $10, $2a, $d0, $aa, $d0, $aa, $d0, $aa, $d0, $aa, $00, $00, $00, $00, $00, $00
    db $00, $01, $01, $02, $03, $04, $03, $05, $07, $08, $00, $00, $00, $0f, $0f, $70
    db $7f, $8d, $ff, $16, $ff, $bf, $ff, $69, $ff, $d0, $00, $00, $00, $c0, $c0, $38
    db $f8, $86, $fe, $1d, $ff, $c2, $ff, $b4, $ff, $73, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $80, $80, $40, $c0, $20, $c0, $20, $05, $0a, $03, $0d, $03, $1c
    db $0b, $15, $0b, $15, $1f, $22, $1f, $29, $1f, $29, $ff, $e3, $ff, $cf, $ff, $df
    db $ff, $df, $ff, $7f, $ff, $ff, $ff, $7f, $ff, $7f, $ff, $bd, $ff, $fa, $ff, $fe
    db $ff, $fd, $ff, $fe, $ff, $fe, $ff, $fb, $ff, $fa, $e0, $90, $e0, $90, $f0, $08
    db $f0, $28, $f8, $44, $f8, $b4, $f8, $94, $f8, $24, $1f, $26, $0f, $14, $0e, $11
    db $06, $09, $01, $0e, $01, $06, $00, $07, $00, $01, $ff, $bf, $ff, $7f, $ff, $1d
    db $7f, $ae, $3f, $c6, $3f, $c8, $1f, $e0, $44, $bb, $ff, $f7, $ff, $d5, $fd, $32
    db $fb, $c4, $f1, $0e, $c3, $3d, $77, $8a, $de, $21, $f0, $48, $f0, $08, $c0, $30
    db $e0, $50, $c0, $a0, $c0, $a0, $80, $40, $00, $80, $23, $5c, $0e, $31, $40, $4f
    db $50, $60, $57, $6a, $55, $6a, $55, $68, $55, $28, $c0, $3e, $00, $f9, $00, $e3
    db $10, $2b, $d0, $aa, $50, $ab, $50, $2b, $50, $2b, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $0f, $0f, $70, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $c0, $c0, $38, $00, $01, $01, $02, $03, $00
    db $03, $05, $05, $0a, $03, $0c, $03, $0d, $0b, $14, $7f, $84, $ff, $2b, $ff, $c7
    db $ff, $1b, $ff, $9f, $ff, $3f, $ff, $5f, $ff, $3f, $f8, $c6, $fe, $01, $ff, $cc
    db $ff, $e5, $ff, $fa, $ff, $fd, $ff, $fc, $ff, $fd, $00, $00, $00, $80, $80, $40
    db $c0, $20, $c0, $20, $e0, $10, $e0, $50, $f0, $a8, $0b, $14, $0f, $12, $1f, $2d
    db $1f, $29, $1d, $22, $1d, $2a, $0c, $13, $0e, $11, $ff, $bf, $ff, $7f, $ff, $bf
    db $ff, $5f, $ff, $bf, $ff, $57, $7f, $99, $7f, $84, $ff, $fe, $ff, $ff, $ff, $f5
    db $ff, $f8, $fe, $e9, $fc, $e3, $fc, $a3, $f8, $47, $f0, $c8, $f8, $a4, $f8, $c4
    db $f8, $a4, $f8, $04, $b0, $48, $70, $a8, $e0, $58, $06, $09, $01, $0e, $01, $06
    db $00, $01, $00, $00, $00, $00, $00, $00, $07, $07, $bf, $40, $0d, $f2, $00, $ff
    db $91, $6e, $6c, $33, $25, $1a, $7e, $39, $f5, $c4, $c1, $3e, $1b, $e4, $ea, $15
    db $c0, $3f, $2c, $d6, $88, $7c, $24, $db, $8b, $76, $c0, $b0, $40, $a0, $00, $c0
    db $00, $80, $00, $00, $00, $00, $00, $80, $e0, $e0, $40, $6e, $50, $71, $5d, $65
    db $57, $6a, $57, $6a, $55, $6a, $55, $68, $55, $28, $0a, $c1, $20, $0b, $50, $2b
    db $d0, $ab, $d0, $aa, $50, $ab, $50, $2b, $50, $2b, $00, $00, $00, $01, $03, $1c
    db $1c, $23, $13, $2c, $27, $5b, $0f, $75, $4f, $b3, $00, $39, $39, $c6, $ff, $1c
    db $ff, $6b, $ff, $bf, $ff, $ff, $ff, $ff, $ff, $ff, $00, $f0, $f0, $0f, $ff, $d0
    db $ff, $76, $ff, $fe, $ff, $e4, $ff, $fe, $ff, $fd, $00, $00, $00, $80, $80, $70
    db $f0, $08, $f8, $a4, $fc, $12, $3c, $da, $9e, $69, $1f, $e5, $1f, $e2, $47, $b9
    db $2e, $d1, $2a, $d5, $30, $4f, $0a, $75, $04, $3b, $ff, $ff, $ff, $ff, $ff, $b7
    db $ff, $4d, $ed, $12, $0b, $f4, $02, $fd, $81, $7e, $ff, $fe, $ff, $ff, $ff, $fa
    db $ff, $e2, $ff, $e0, $f5, $0a, $84, $7b, $8c, $73, $9e, $6d, $8e, $75, $ae, $51
    db $1e, $e1, $1c, $ea, $1c, $e2, $78, $84, $f0, $08, $00, $2f, $00, $0b, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $30, $cf, $4b, $b4, $00, $ff
    db $00, $bf, $21, $1e, $00, $1f, $08, $17, $2c, $13, $61, $9e, $20, $db, $00, $f2
    db $08, $f0, $00, $e0, $00, $e0, $80, $70, $30, $c0, $c0, $30, $00, $c0, $00, $00
    db $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $01, $00, $07, $01, $00, $00, $05, $1a, $03, $3c, $1b, $2c
    db $6b, $35, $c7, $7b, $c7, $fb, $1d, $62, $02, $1d, $50, $a0, $40, $b0, $e0, $70
    db $f0, $58, $f0, $1e, $fc, $0b, $c7, $3c, $ef, $1e, $00, $00, $00, $00, $00, $00
    db $00, $00, $00, $00, $00, $00, $80, $00, $e0, $e0, $00, $03, $03, $3c, $1f, $23
    db $3f, $4f, $7f, $92, $67, $98, $28, $57, $58, $a7, $00, $b9, $b9, $46, $ff, $1c
    db $ff, $eb, $ff, $3f, $7f, $a5, $65, $9a, $af, $50, $00, $f0, $f0, $0f, $ff, $d0
    db $ff, $77, $ff, $ff, $ff, $ee, $f7, $48, $8d, $72, $00, $00, $00, $80, $80, $70
    db $f0, $88, $f8, $f4, $f8, $66, $ec, $12, $0c, $f3, $0f, $f0, $00, $ff, $40, $b2
    db $00, $65, $00, $2e, $00, $03, $00, $00, $00, $00, $04, $fb, $07, $78, $20, $df
    db $00, $3f, $04, $0b, $03, $44, $02, $4d, $01, $86, $e0, $1f, $40, $bf, $00, $f3
    db $00, $f0, $00, $c6, $a0, $04, $80, $70, $80, $68, $18, $e7, $e0, $1e, $30, $c8
    db $00, $f8, $00, $10, $00, $60, $00, $80, $00, $00, $00, $43, $03, $04, $00, $07
    db $00, $05, $01, $00, $00, $01, $00, $01, $00, $03, $40, $80, $00, $c0, $00, $80
    db $00, $40, $00, $40, $00, $50, $88, $10, $00, $10, $01, $02, $03, $00, $03, $04
    db $03, $15, $07, $13, $07, $2b, $0d, $02, $02, $1d, $00, $88, $40, $a0, $e0, $60
    db $e0, $40, $f0, $14, $f0, $00, $c2, $38, $ec, $1c, $40, $60, $50, $70, $5d, $65
    db $57, $6a, $57, $6a, $55, $6a, $55, $68, $55, $28, $0a, $01, $20, $0b, $50, $2b
    db $d0, $ab, $d0, $aa, $50, $ab, $50, $2b, $50, $2b, $02, $00, $07, $02, $03, $00
    db $03, $02, $03, $02, $07, $02, $02, $00, $02, $02, $00, $00, $c4, $c4, $fc, $dc
    db $fc, $cc, $fc, $cc, $fc, $cc, $38, $28, $00, $00, $02, $02, $02, $02, $02, $02
    db $02, $02, $02, $02, $02, $02, $02, $02, $02, $02, $78, $78, $fe, $7c, $fe, $7c
    db $fe, $7c, $fe, $7c, $c6, $44, $00, $00, $00, $00, $00, $00, $01, $00, $01, $00
    db $01, $00, $03, $01, $00, $00, $00, $00, $00, $00, $70, $30, $fc, $7c, $ff, $fa
    db $fe, $fa, $3c, $34, $1c, $14, $0c, $08, $08, $08, $10, $10, $10, $10, $20, $20
    db $20, $20, $40, $40, $40, $40, $80, $80, $80, $80, $00, $00, $03, $02, $f7, $57
    db $7f, $5b, $3f, $29, $3f, $2d, $3e, $16, $10, $10, $00, $00, $00, $00, $80, $00
    db $80, $00, $c0, $80, $80, $80, $00, $00, $00, $00, $08, $08, $08, $08, $04, $04
    db $04, $04, $02, $02, $02, $02, $01, $01, $01, $01

; 6 tiles ($60 bytes). Bank 0 SoftReset copies them to VRAM $9530.
Title_ExtraTiles2Gfx:
    db $05, $06, $05, $06, $01, $02
    db $83, $44, $c7, $38, $ff, $c3, $ff, $ff, $3e, $3e, $e0, $38, $e8, $30, $e8, $74
    db $ce, $51, $cf, $d4, $8f, $97, $1b, $1b, $18, $18, $17, $18, $17, $18, $07, $09
    db $0f, $11, $1f, $e3, $fe, $0e, $fc, $fc, $f8, $f8, $9e, $a2, $9e, $a2, $9e, $a2
    db $1e, $22, $1c, $62, $3f, $41, $7f, $7f, $7f, $7f, $78, $88, $f8, $88, $b0, $c9
    db $5c, $65, $69, $76, $3f, $39, $1f, $1f, $07, $07, $79, $8a, $79, $9a, $f1, $12
    db $f1, $32, $e1, $66, $c3, $c4, $87, $87, $07, $07


SECTION "Bank 12 fade steps", ROMX[$4256], BANK[12]

; NR50 master volume steps for fades. One step per $30 frames.
FadeInSteps:
    db $11, $11, $11, $22, $33, $44, $55, $77, $77
FadeOutSteps:
    db $77, $66, $55, $33, $22, $11, $00
FadeOutFastSteps:
    db $66, $44, $22, $00


SECTION "Bank 12 SFX data", ROMX[$4b82], BANK[12]

; Sound effect register data. Each entry is copied to the channel registers
; by SfxWriteRegs5 (square 1: NR10-NR14) or SfxWriteRegs4 (NR21-NR24 or
; NR41-NR44). Multi-step effects walk through several entries.
Sfx21_Start_Sq1Regs:
    db $55, $8f, $f2, $5d, $c6
    db $00, $d0, $88, $c7, $c2
    db $d0, $88, $9b, $c3, $10
    db $28, $40, $c0, $00, $c0
    db $88, $c7, $82, $c0, $88
    db $9b, $83, $00, $28, $40
    db $80, $80, $c0, $60, $00
    db $80
Sfx01_Start_Sq1Regs:
    db $00, $bb, $f1, $59, $c7
Sfx18_Start_Sq1Regs:
    db $00, $bc, $f1, $83, $c7
    db $00, $bc, $f1, $83, $c7
    db $00, $bd, $b1, $6b, $c7
Sfx08_Start_Sq1Regs:
    db $37, $81, $f1, $50, $c6
    db $37, $81, $91, $50, $c6
Sfx17_Start_Sq1Regs:
    db $1b, $08, $f0, $ff, $c6
    db $1b, $08, $f0, $ff, $c6
    db $1e, $b1, $f1, $10, $c7
NoiseSfx02_Start_NoiseRegs:
    db $00, $dc, $f5, $80
NoiseSfx02_Update_NoiseRegs:
    db $00, $f4, $55, $80
NoiseSfx0F_Start_NoiseRegs:
    db $00, $f1, $17, $80
NoiseSfx0F_Update_NoiseRegs:
    db $00, $f7, $03, $80
Sfx0A_Start_Sq1Regs:
    db $00, $91, $92, $06, $c7
    db $00, $81, $b2, $06, $c7
    db $00, $91, $97, $06, $87
NoiseSfx01_Start_NoiseRegs:
    db $00, $ec, $41, $80
NoiseSfx01_Update_NoiseRegs:
    db $00, $f7, $43, $80
    db $00, $e7, $54, $80
    db $00, $f7, $67, $80
SoundCue1_Start_Sq1Regs:
    db $2a, $08, $d0, $e0, $c5
SoundCue1_Update_Sq1Regs:
    db $4c, $18, $d0, $e0, $c7
SoundCue1_Start_NoiseRegs:
    db $00, $f4, $45, $80
Sfx11_Play_Sq1Regs:
    db $00, $18, $d0, $e0, $c7
Sfx11_Update_Sq1Regs:
    db $00, $1f, $d0, $ef, $c7
Sfx11_Play_NoiseRegs:
    db $00, $e1, $05, $80
Sfx0D_Start_Sq1Regs:
    db $00, $1f, $50, $ef, $c7
Sfx0D_Update_Sq1Regs:
    db $00, $1f, $d0, $ef, $c7
Sfx0D_Start_NoiseRegs:
    db $00, $f2, $73, $80
Sfx0D_Update_NoiseRegs:
    db $00, $f1, $05, $80
    db $00, $1f, $06, $80
NoiseSfx07_Start_NoiseRegs:
    db $00, $f7, $21, $80
NoiseSfx0C_Start_NoiseRegs:
    db $00, $f7, $21, $80
    db $00, $e7, $31, $80
    db $00, $e7, $31, $80
    db $00, $d7, $41, $80
    db $00, $d7, $51, $80
    db $00, $c7, $61, $80
    db $00, $c7, $71, $80
    db $00, $b7, $81, $80
    db $00, $b7, $91, $80
    db $00, $b7, $a1, $80
    db $00, $a7, $a1, $80
    db $00, $a7, $81, $80
    db $00, $a7, $91, $80
    db $00, $a7, $a1, $80
    db $00, $a7, $a1, $80
    db $00, $a7, $81, $80
    db $00, $97, $91, $80
    db $00, $87, $a1, $80
    db $00, $77, $a1, $80
Sfx06_Start_Sq1Regs:
    db $33, $a7, $f2, $5d, $c6
    db $33, $a7, $f2, $65, $c6
    db $33, $a7, $e2, $6d, $c6
    db $33, $a7, $e2, $73, $c6
    db $33, $a7, $c2, $78, $c6
    db $33, $a7, $c2, $7d, $c6
    db $33, $a7, $a2, $83, $c6
    db $33, $a7, $a2, $8c, $c6
NoiseSfx0D_Start_NoiseRegs:
    db $01, $2f, $61, $80
    db $01, $76, $61, $80
NoiseSfx0E_Run_NoiseRegs:
    db $01, $2f, $61, $80
    db $01, $76, $61, $80
NoiseSfx09_Start_NoiseRegs:
    db $01, $3f, $61, $80
    db $01, $86, $61, $80
Sfx0B_Start_Sq1Regs:
    db $00, $88, $f7, $4e, $c5
    db $00, $88, $f7, $83, $c5
    db $00, $88, $e7, $b5, $c5
    db $00, $88, $e7, $e5, $c5
    db $00, $88, $e7, $11, $c6
    db $00, $88, $e7, $3b, $c6
Sfx14_Start_Sq1Regs:
    db $76, $c0, $b7, $4e, $83
Sfx14_Start_Sq2Regs:
    db $c0, $b7, $4e, $82
Sfx19_Start_Sq1Regs:
    db $00, $60, $f1, $63, $85
Sfx19_Start_Sq2Regs:
    db $60, $f1, $ed, $85
    db $00, $a0, $f1, $83
    db $87, $00, $79, $f0
    db $0b, $c6, $00, $79
    db $f0, $72, $c6
Sfx32_Update_Sq1Regs:
    db $76, $c0, $b7, $4e, $84
    db $00, $48, $97, $39, $87
    db $00, $48, $97, $39, $87
    db $00, $48, $97, $39, $87
    db $00, $48, $f7, $59, $87
    db $00, $48, $f7, $6b, $87
    db $00, $48, $f7, $7b, $c7
    db $00, $43, $f7, $8a, $c7
    db $00, $43, $f7, $a1, $c7
    db $00, $43, $f7, $a7, $c7
    db $00, $43, $f7, $ac, $c7
    db $00, $43, $f7, $b1, $c7
NoiseSfx03_Start_NoiseRegs:
    db $00, $ec, $a0, $80
NoiseSfx03_Update_NoiseRegs:
    db $00, $f7, $70, $80
NoiseSfx03_Update_NoiseRegs2:
    db $00, $f7, $70, $80
NoiseSfx03_Update_NoiseRegs3:
    db $00, $e7, $a0, $80
NoiseSfx08_Start_NoiseRegs:
    db $00, $62, $25, $80
NoiseSfx04_Start_NoiseRegs:
    db $00, $a2, $25, $80
    db $00, $96, $14, $80
Sfx13_Start_NoiseRegs:
    db $00, $62, $25, $80
    db $00, $72, $14, $80
Sfx10_Start_Sq1Regs:
    db $1b, $1f, $f0, $70, $c5
    db $1b, $33, $f0, $a0, $c5
Sfx10_Start_NoiseRegs:
    db $10, $a2, $45, $c0
Sfx04_Start_Sq1Regs:
    db $00, $81, $f2, $59, $c7
    db $00, $81, $f2, $7b, $c7
    db $00, $81, $f2, $90, $c7
    db $00, $81, $f2, $ac, $c7
    db $00, $81, $f2, $59, $c7
    db $00, $81, $f2, $7b, $c7
    db $00, $81, $f2, $90, $c7
    db $00, $81, $f2, $ac, $c7
    db $00, $81, $f2, $c1, $c7
    db $00, $81, $f2, $ce, $c7
Sfx15_Start_Sq1Regs:
    db $00, $6c, $f8, $ce, $c7
    db $00, $6c, $f8, $ce, $c7
    db $00, $6c, $f8, $ce, $c7
    db $00, $6c, $f8, $ce, $c7
Sfx0F_Start_Sq1Regs:
    db $00, $c1, $b2, $b2, $c6
    db $00, $c1, $b2, $f7, $c6
    db $00, $c1, $b2, $21, $c7
Sfx0F_Start_Sq2Regs:
    db $c1, $b2, $21, $c7
    db $c1, $b2, $59, $c7
    db $c1, $b2, $7b, $c7
Sfx02_Start_Sq1Regs:
    db $00, $a7, $c5, $b2, $86
    db $00, $a7, $c5, $f7, $86
    db $00, $a7, $c5, $21, $87
    db $00, $a7, $c5, $59, $87
    db $00, $a7, $c5, $7b, $87
    db $00, $a7, $55, $b2, $86
    db $00, $a7, $55, $f7, $86
    db $00, $a7, $55, $21, $87
    db $00, $a7, $55, $59, $87
    db $00, $a7, $55, $7b, $87
    db $00, $00, $00, $00
Sfx02_Start_Sq2Regs:
    db $00, $00, $00, $00
    db $a7, $95, $b2, $86
    db $a7, $95, $f7, $86
    db $a7, $95, $21, $87
    db $a7, $95, $59, $87
    db $a7, $95, $7b, $87
    db $a7, $25, $b2, $86
    db $a7, $25, $f7, $86
    db $a7, $25, $21, $87
    db $a7, $25, $59, $87
    db $a7, $25, $7b, $87
Sfx1D_Start_Sq1Regs:
    db $00, $e7, $ff, $16, $c4
    db $00, $e7, $ff, $4e, $c4
    db $00, $e7, $ff, $83, $c4
    db $00, $e7, $ff, $b5, $c4
Sfx1D_Start_Sq2Regs:
    db $e7, $ff, $c7, $c2
    db $e7, $ff, $12, $c3
    db $e7, $ff, $58, $c3
    db $e7, $ff, $9b, $c3
Sfx1B_Start_Sq1Regs:
    db $00, $b2, $b1, $39, $c7
    db $00, $b2, $b1, $59, $c7
    db $00, $b2, $b1, $7b, $c7
    db $00, $b2, $b1, $83, $c7
Sfx1B_Start_Sq2Regs:
    db $b2, $b1, $0b, $c6
    db $b2, $b1, $06, $c7
    db $b2, $b1, $0b, $c6
    db $b2, $b1, $06, $c7
Sfx1A_Start_Sq1Regs:
    db $00, $c1, $b2, $f7, $c6
    db $00, $c1, $b2, $b2, $c6
Sfx1A_Start_Sq2Regs:
    db $c1, $b2, $59, $c7
    db $c1, $b2, $21, $c7
Sfx05_Start_Sq1Regs:
    db $73, $81, $f2, $83, $c4
    db $7b, $81, $e2, $c8, $c7
    db $7b, $81, $c2, $c8, $c7
    db $7b, $81, $b2, $c8, $c7
    db $7b, $81, $a2, $c8, $c7
NoiseSfx05_Start_NoiseRegs:
    db $2e, $80, $4b, $c0
Sfx0B_Start_NoiseRegs:
    db $20, $80, $4b, $c0
NoiseSfx0A_Start_NoiseRegs:
    db $00, $1f, $06, $80
NoiseSfx0A_Start_Sq1Regs:
    db $00, $81, $1f, $e4, $87
    db $00, $81, $1f, $e4, $87
    db $00, $81, $1f, $e3, $87
    db $00, $81, $1f, $e2, $87
    db $00, $81, $1f, $e1, $87
    db $00, $ec, $a0, $80, $00
    db $f7, $02, $80, $00, $b7
    db $02, $80, $00, $b7, $17
    db $80, $00, $b7, $a1, $80
    db $00, $81, $a9, $e0, $87
    db $00, $8a, $a7, $e0, $87
