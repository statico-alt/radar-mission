DEF g_sceneFlagsA EQU $d000
DEF g_sceneFlagsB EQU $d001 ; bit 7 = MissionObjectiveMet
DEF g_sceneFlagsC EQU $d002 ; bit 5 = ClearGate, bit 3 = SubKillBranch
DEF g_subStateScratch4 EQU $d004
DEF g_subState EQU $d005 ; indexes GameB_SubStateTable
DEF g_subStateScratch8 EQU $d008 ; drives $300a dispatch (phase selector in per-frame engine)
DEF g_subStateScratch9 EQU $d009
DEF g_modeFlags10 EQU $d010
DEF g_modeFlags15 EQU $d015
DEF g_modeFlags16 EQU $d016
DEF g_modeFlags17 EQU $d017
DEF g_modeFlags18 EQU $d018
DEF g_modeFlags1a EQU $d01a ; bit 0 toggled by bank3:$500a pause handler
DEF g_weaponEdges EQU $d01c ; bits 0,1=A/B edges; 4,5=twin-shot AI edges; 6,7=target-lock state
DEF g_modeFlags1d EQU $d01d
DEF g_modeFlags1e EQU $d01e
DEF ScrollScript_SlotA_Cursor EQU $d020 ; slot A queue cursor, reset to $14 per call
DEF ScrollScript_SlotB_Cursor EQU $d021 ; slot B queue cursor, reset to $14 per call
DEF g_spriteCursorA EQU $d022 ; sprite cursor for the $c000 buffer
DEF g_spriteCursorB EQU $d023 ; sprite cursor for the $c100 buffer
DEF g_weaponAiState EQU $d030 ; drives bank3:$6112 dispatch in $60fc periodic tick
DEF g_d0dbCounter EQU $d0db ; advanced by bank3:$467e during window animation
DEF g_torpedoSlots EQU $d0e0 ; 2 slots x 8 bytes: flags,type,xlo,xhi,speed,life,dirFlip,_
DEF g_mgSlots EQU $d0f0 ; 2 slots x 8 bytes; same layout as torpedoSlots
DEF g_enemyBulletSlots EQU $d100 ; 4 slots x 8 bytes; scanned by alloc at rom3:54a6
DEF g_shipArray_Player EQU $d180 ; 10 slots x 8 bytes (80B); +0 flags, +1 x-stagger, +2/+3 pos, +4 type ($80), +5 HP, +6 state, +7 index
DEF g_shipArray_Enemy EQU $d218 ; 10 slots x 8 bytes; mirrors g_shipArray_Player; picked by C.0 in rom3:GameB_ShipScan_SideAware
DEF g_linkSendRequest EQU $d2db ; bit 0 = start a new link message
DEF g_mgTrailA EQU $d140 ; 8 slots x 4 bytes (primary MG trail)
DEF g_mgTrailB EQU $d160 ; 8 slots x 4 bytes (twin-shot MG trail)
DEF g_mgBurstCooldownA EQU $d2e5 ; MG inter-frame burst gate (player A)
DEF g_mgBurstCooldownB EQU $d2e6 ; MG inter-frame burst gate (player B)
DEF g_d2f7Counter EQU $d2f7 ; bank0:$30b4 scene-transition counter
DEF g_enemySubFirePhase EQU $d2f8 ; updated by bank3:$550b; gates enemy-bullet emission cadence
DEF g_lockOnTargetIdx EQU $d343 ; index into $d180 used by bank3:$61a4 target-lock update
DEF g_torpedoSlotIdx EQU $d344 ; torpedo slot index for GameB_ClearTorpedo
DEF g_enemySubApproachShadow EQU $d34a ; shadow of $d44c written alongside by bank3:$624c
DEF g_hitTestPos EQU $d34d ; 2 bytes: projectile position tested against ship slots
DEF g_enemyFireGate EQU $d443 ; nonzero = enemy fire permitted (checked by bank3:$54bd)
DEF g_enemySubApproachCounter EQU $d44c ; advanced by bank3:$624c; hits $3c threshold -> sets $d011.0
DEF g_winCutsceneTimer EQU $d464 ; bumped by bank3:$5aec when final enemy sub destroyed
DEF g_windowYTarget EQU $d316 ; HUD window-split target; fed to rWY in bank3:$467e
DEF g_metaspriteOamSlot EQU $d318
DEF g_firstFreeSprite EQU $d319 ; first free sprite entry
DEF g_bannerVramPage EQU $d326
DEF g_bannerTileIndex EQU $d327
DEF g_bannerSrcBank EQU $d328
DEF g_gameBStage EQU $d333 ; caps at 3, then sets $d002.6
DEF g_deathTallyA EQU $d334
DEF g_deathTallyB EQU $d335
DEF g_playerShipsRemaining EQU $d356 ; DEC'd in GameB_PlayerHit at bank3:$489b; reaching 0 -> last-ship destroyed -> state $1a -> verdict $21
DEF g_playerShipDamageCount EQU $d357 ; INC'd alongside DEC of $d356
DEF g_killFlagsPending EQU $d33c ; upper 3 bits gated in $40f6; bit 6 = torpedo twin-shot enable
DEF g_killFlagsPendingB EQU $d33d ; mirror of $d33c for slot B; bit 6 = MG twin-shot enable
DEF g_savedWeaponFlags EQU $d33e ; weapon flags saved by the outfit menu
DEF g_killCount EQU $d33f
DEF g_dispatchTableD346 EQU $d346 ; used by RST/jr table at bank0:7228
DEF g_windowAnimTimer EQU $d358 ; bank3:$467e countdown; clears $d016.5 on expiry
DEF g_deathMarker EQU $d372 ; set to $ff on damage
DEF ScrollScript_PrimaryPtr_Lo EQU $d3e1 ; primary tile-stream ptr (low)
DEF ScrollScript_PrimaryPtr_Hi EQU $d3e2 ; primary tile-stream ptr (high) - $ff terminator -> scene end
DEF ScrollScript_VramDst_Lo EQU $d3e3 ; running VRAM dest cursor (low)
DEF ScrollScript_VramDst_Hi EQU $d3e4 ; running VRAM dest cursor (high)
DEF ScrollScript_Index EQU $d3e5 ; reveal index, bumped once per even frame
DEF ScrollScript_Pad EQU $d3e6 ; padding / unused
DEF ScrollScript_TemplateFlag EQU $d3e9 ; scene template flag word (copied to $d326)
DEF ScrollScript_TemplatePrimaryLo EQU $d3eb ; template primary ptr lo
DEF ScrollScript_TemplatePrimaryHi EQU $d3ec ; template primary ptr hi
DEF ScrollScript_TemplateVramLo EQU $d3ed ; template VRAM dst lo
DEF ScrollScript_TemplateVramHi EQU $d3ee ; template VRAM dst hi
DEF ScrollScript_LastTileByte EQU $d3f7 ; last processed tile byte, consumed by QueueVramWrite
DEF g_sceneScriptIndex EQU $d426
DEF g_torpedoFireCount EQU $d427 ; torpedoes fired (scene-lifetime counter)
DEF g_mgFireCount EQU $d428 ; MG shots fired
DEF g_levelClearTimer EQU $d42b ; gates $25bf
DEF g_periscopeFlags EQU $d42c ; bit 7 = draw the periscope marker
DEF g_torpedoAmmoMax EQU $d440 ; ammo-reload max; torpedo
DEF g_mgAmmoMax EQU $d441 ; ammo-reload max; MG
DEF g_sceneEndPauseTimer EQU $d44b
DEF g_hpA EQU $d453 ; TBD: possibly torpedo ammo (warn SFX at rom3:$562d when low)
DEF g_hpB EQU $d454 ; TBD: possibly MG ammo; see weapons NOTES
DEF g_torpedoCooldown EQU $d457 ; per-slot refire cooldown, set to 3 on fire
DEF g_mgCooldown EQU $d458 ; per-slot MG refire cooldown
DEF g_joyEdge EQU $d459 ; newly-pressed buttons
DEF g_joyPrev EQU $d45a
DEF g_livesGameB EQU $d45f
DEF g_displayedScoreBCD EQU $d3f1 ; 6-byte little-endian BCD shown as SCORE/TOTAL on tally; rank compare reads top 4 bytes ($d3f3..$d3f6)
DEF g_scoreTotalBCD EQU $d80c ; 6-byte little-endian BCD running score; accumulated during tally at rom1:$7a3c
DEF g_scoreAddendBCD EQU $d812 ; 6-byte BCD addend; added into $d80c and $d3f1 by TallyAddRow; seeded per-row
DEF g_rowMultiplicandBCD EQU $d818 ; 4-byte BCD row multiplicand (point-value per kill / per-bonus unit)
DEF g_rowProductBCD EQU $d81e ; 6-byte BCD scratch for row kill-count × multiplicand multiplication
DEF g_tallyDigitAnim EQU $d820 ; per-digit countdown animator driving the rolling-digits tally effect
DEF g_rankIndex EQU $d82c ; 0=ADMIRAL, 1=CAPTAIN, 2=COMMANDER, 3=OFFICER, 4=CADET, 5=NO RANK; set by rom3:$7576
DEF g_tallyStepTimer EQU $d873 ; seeded to $18 in TallyReset; per-step delay in tally animation
DEF g_tallyRowIndex EQU $d874 ; current tally row (index into payload tables at rom1:$7d63/$7dab)
DEF SFX_Queue EQU $dea0 ; sound-effect queue byte
DEF BGM_Queue EQU $dea8 ; background-music queue byte; $0b=loss fanfare, $19=victory fanfare, $ff=silence
DEF g_tallyCutsceneSubState EQU $d824 ; indexes GameB_TallyDispatchTable at $3adb (0..6)
DEF g_state26CutsceneSubState EQU $d826 ; indexes RST00 dispatch at $3e07
DEF g_topModeFlags EQU $ff8a ; bit 5 = GameB selected
DEF g_currentBankShadow EQU $ff8b
DEF g_topState EQU $ff8f ; indexes DispatchTopState table at $044d
DEF g_joyRaw EQU $ff98
DEF g_p1ShotsFired EQU $c0aa ; P1 shots fired this battle
DEF g_p1Hits EQU $c0ab ; P1 hits this battle
DEF g_p2ShotsFired EQU $c0ac ; P2 shots fired this battle
DEF g_p2Hits EQU $c0ad ; P2 hits this battle
DEF g_p1HitRatio EQU $c0ae ; P1 hit ratio in percent (results screen)
DEF g_p2HitRatio EQU $c0af ; P2 hit ratio in percent (results screen)
DEF g_statEffect1Arg EQU $c0b2 ; value read by Stat_Effect1
DEF g_statEffect2Arg EQU $c0b3 ; value read by Stat_Effect2
DEF g_p1Commander EQU $c0b4 ; P1 commander/fleet type (0..4); picks tileset
DEF g_p2Commander EQU $c0b5 ; P2/CPU commander type; also picks CPU AI (0..2)
DEF g_shipsAfloat EQU $c0b6 ; number of ships afloat
DEF g_vramQueue EQU $c0bf ; tilemap script buffer (RST $28 format) drawn later
DEF g_mapStreamPtr EQU $c522 ; 2-byte pointer to the map stream for DrawMapRowFromPtr
DEF g_cpuShotGrid EQU $c5b4 ; grid of cells the CPU has shot (nonzero = shot, 1 = hit)
DEF g_playerFleetGrid EQU $c644 ; grid of the fleet the CPU shoots at; low 6 bits = ship cell id
DEF g_currentPlayer EQU $c76b ; 0 = P1, 1 = P2
DEF g_aiHitList EQU $c7d3 ; AI list of (state, packed cell) pairs
DEF g_aiHitCount EQU $c7f5 ; number of entries in g_aiHitList
DEF g_aiState EQU $c7f6 ; CPU AI state (0 hunt .. 6)
DEF g_aiDirection EQU $c7f7 ; 0 up, 1 down, 2 left, 3 right
DEF g_aiTryCount EQU $c7f8 ; directions tried from the current hit
DEF g_aiKnownCount EQU $c7f9 ; count of cells in $c7fa known-cell list
DEF g_aiKnownCells EQU $c7fa ; AI known-cell list (count in g_aiKnownCount)
DEF g_aiHitCell EQU $c80a ; packed cell of the AI hit being followed up
DEF g_animScriptPtrs EQU $c878 ; sprite anim script pointer table (UpdateAnimScripts, bank 1 $400a)
DEF g_animScriptCount EQU $c880 ; number of active anim scripts
DEF g_animSpritesPerScript EQU $c881 ; sprites per anim script
DEF g_animFirstOamSlot EQU $c882 ; first OAM slot used by anim scripts
DEF g_animRecords EQU $c884 ; anim script 4-byte sprite records
DEF g_shipCellPos EQU $c935 ; 18 bytes: packed cell per ship cell id; $ff = gone
DEF g_optionsCursorRow EQU $c9cb ; 2 bytes: options menu row for P1/P2
DEF g_vramQueueIndex EQU $c9cd ; write offset into g_vramQueue
DEF g_gameBLinkState EQU $d00b ; link cable state, indexes LinkStateTable in bank 3
DEF g_linkTxWrite EQU $d02c ; link send ring write index (0..7)
DEF g_hudQueueWrite EQU $d031 ; HUD queue write index (0..19)
DEF g_hudQueueRead EQU $d033 ; HUD queue read index (0..19)
DEF g_hudQueue EQU $d035 ; HUD command queue, 20 entries of (cmd, param)
DEF g_linkTxQueue EQU $d05d ; link send ring, 8 entries of (param, cmd)
DEF g_soundQueueWrite EQU $d06f ; sound request ring write index (0..15)
DEF g_soundQueueRead EQU $d070 ; sound request ring read index (0..15)
DEF g_soundQueue EQU $d071 ; sound request ring, 16 entries of (type, id); see SoundQueuePtrs in bank 3
DEF g_metaspriteStartX EQU $d317 ; X of the left edge while DrawMetasprite runs
DEF g_worldWidth EQU $d329 ; word; scene width = first layout byte * $e0; used to mirror X and wrap projectiles
DEF g_scaledTable EQU $d3a8 ; 36 bytes, Table_50fd shifted by (2 - [$d337])
DEF g_tallyIconGrow EQU $d3f8 ; grow counter for the death tally icon (0..$30)
DEF g_layoutWidthUnits EQU $d3fc ; first byte of the level layout (world width / $e0)
DEF g_scanSlotType EQU $d3ff ; type byte of the active slot being scanned
DEF g_gameBRandomSeed EQU $d40a ; seed updated by bank6 RandomByte
DEF g_launchIndex EQU $d40b ; object index used by GameB_LaunchObject
DEF g_randomPos EQU $d465 ; random/placement accumulator used by bank6 RandomMasked and GameB_PlaceShips
DEF g_rankScreenState EQU $d827 ; rank screen state, indexes RankScreenStateTable in bank 3
DEF g_musicCh1 EQU $de50 ; music channel 1 struct (16 bytes; $de60, $de70, $de80 for channels 2-4)
DEF g_musicChannelNum EQU $de90 ; channel being updated (1-4)
DEF g_musicTranspose EQU $de91 ; song transpose
DEF g_musicNoteLenPtr EQU $de92 ; note length table pointer (word)
DEF g_musicPanMode EQU $de94 ; stereo pan mode from Music_SongPanTable
DEF g_sfxSq1Id EQU $dea1 ; active square-channel SFX ID (set by SfxNN_Start, read by SfxUpdate)
DEF g_musicSongId EQU $dea9 ; song ID now playing; 0 = none
DEF SoundCmd_Queue EQU $deb0 ; sound command request: 1 pause, 2 resume, 3 fade out, 5 fast fade out, $1e
DEF NoiseSfx_Queue EQU $deb8 ; noise-channel SFX request queue (cleared every frame)
DEF g_noiseSfxId EQU $deb9 ; active noise SFX ID
DEF g_soundPaused EQU $dec7 ; nonzero = music update skipped (set by SoundCmd_Pause)
DEF g_soundCueRequest EQU $dec8 ; 1 = start SoundCue1, 4 = fade in
DEF g_soundFadeState EQU $dec9 ; running fade effect: 4 = fade, 9 = SoundCue1; other banks poll it
DEF g_soundFrameCount EQU $decd ; frame counter 0-11 for SoundTickClock
DEF g_soundClock EQU $dece ; increments every 12 frames; read by bank 0 as a timer
DEF g_musicSaveState EQU $def0 ; saved copy of $de50-$dea9 (MusicSaveState)
DEF g_gameALevel EQU $ff8c ; bits 6-7 = Game A level (0..2); NextLevel adds $40
DEF g_p1Options EQU $ff8d ; bits 0-1 grid size (8/10/12), bits 2-4 option toggles, bit 5 cursor column
DEF g_p2Options EQU $ff8e ; same as $ff8d for player 2; bit 5/6 = P1/P2 ready
DEF g_gameALinkState EQU $ff91 ; serial link protocol state; Link_SerialHandler dispatches on it
DEF g_shipHitsLeft EQU $ff92 ; 5 bytes: hits left per ship (carrier..destroyer)
DEF g_revealFlag EQU $ff97 ; set to 1 when the AI reveal list is used
DEF g_serialRecv EQU $ffb3 ; last byte received over serial
DEF g_serialSend EQU $ffb5 ; next byte to send over serial
DEF g_gridSize EQU $ffcb ; grid width/height in cells (8, 10 or 12)
DEF g_targetRow EQU $ffdd ; row of the cell the CPU chose to shoot
DEF g_targetCol EQU $ffde ; column of the cell the CPU chose to shoot
DEF g_gameARandomSeed EQU $ffe0 ; updated by bank 5 Random (rDIV mix)
DEF g_oamBuffer EQU $c000 ; 160-byte shadow OAM copied by OAM DMA
DEF g_p1ShipLengths EQU $c0a5 ; 5 bytes: ship lengths of player 1
DEF g_linkSendPair EQU $c0b8 ; 2 bytes sent by link state $0f
DEF g_linkRecvPair EQU $c0ba ; 2 bytes received by link state $12
DEF g_linkPairCount EQU $c0be ; number of pairs exchanged (link states $0f/$12)
DEF g_tileWorkBuf EQU $c0df ; 16-byte tile buffer for the turned ship tile
DEF g_tileSrcBuf EQU $c0ef ; 16-byte copy of the source tile used by TurnTile90
DEF g_cellList EQU $c1bf ; temp list of packed cells (touching ships, strips)
DEF g_vramCopyCount EQU $c4bf ; number of queued VRAM copies (VBlank_RunCopyQueue)
DEF g_vramCopyQueue EQU $c4c0 ; VRAM copy queue, 7 bytes per entry: src, dst, length, repeat count, bank
DEF g_playerShotGrid EQU $c524 ; Game A grid of cells the player has shot (nonzero = shot)
DEF g_linkFleetGrid EQU $c6d2 ; other player's fleet grid received in link state $01 (offsets 2..$90)
DEF g_cpuFleetGrid EQU $c6d4 ; Game A grid of the other fleet; low 6 bits = ship cell id, top 2 bits = direction
DEF g_turnCount EQU $c764 ; Game A turn count (shown as a 3-digit number)
DEF g_p2ShipLengths EQU $c76c ; 5 bytes: ship lengths of player 2
DEF g_linkCellRow EQU $c814 ; cell row sent/received over the link ($ff = none)
DEF g_linkCellCol EQU $c815 ; cell column sent/received over the link
DEF g_levelMapStep EQU $c868 ; level map draw step; nonzero = continue drawing
DEF g_levelMapRows EQU $c869 ; row count copied to $ffb1 by GameA_DrawLevelMap
DEF g_scriptArgA EQU $c874 ; parameter set by GameA_RunCutsceneScript command $01
DEF g_scriptArgB EQU $c875 ; parameter set by GameA_RunCutsceneScript command $01
DEF g_animSlotIndex EQU $c877 ; sprite index while UpdateAnimScripts runs
DEF g_scrollDirSteps EQU $c927 ; bit 7 = scroll direction, bits 0-6 = steps left (GameA_UpdateScroll)
DEF g_shipDirBits EQU $c92b ; top 2 bits (direction) of the found fleet grid cell
DEF g_targetInCenter EQU $c92c ; set by GameA_IsTargetInCenter
DEF g_scrollSubStep EQU $c92d ; fraction counter; GameA_UpdateScroll adds $c0 per frame
DEF g_shipNumToShow EQU $c92e ; ship number drawn as sprites over its cells
DEF g_spriteCount EQU $c933 ; count of sprites written in the current build
DEF g_luckyCounters EQU $c94c ; Lucky Shot counters, 2 bytes per player
DEF g_luckySquaresP1 EQU $c950 ; 8 bytes: player 1 Lucky Shot squares (also a 16-byte link buffer)
DEF g_luckySquaresP2 EQU $c958 ; 8 bytes: player 2 Lucky Shot squares
DEF g_salvoResultCount EQU $c963 ; entries in the result list at $c966
DEF g_salvoResults EQU $c966 ; per-shot result flags (bit 5 hit, bit 6 done)
DEF g_shotCells EQU $c967 ; cells hit by the current shot pattern (GameA_ResolveShots)
DEF g_salvoCount EQU $c97a ; shots in this salvo
DEF g_salvoLaunched EQU $c97b ; shots launched so far
DEF g_shipRecords EQU $c9a4 ; 5-byte ship records (GameA_LoadShipRecord)
DEF g_titleStartSfx EQU $c9ae ; sound queued by the timer IRQ when $ffab bit 3 is set
DEF g_nextLevelShipTiming EQU $c9b8 ; 2 bytes per ship: start delay and step count (NextLevel_State1_SailIn)
DEF g_idleTimer EQU $c9c8 ; idle countdown (GameA_TickIdleTimer)
DEF g_linkRecvC9d1 EQU $c9d1 ; byte received by link state $13
DEF g_linkSyncBytes EQU $c9d3 ; 2 bytes: last received link bytes, checked for the $f0 marker
DEF g_stripY EQU $c9d7 ; sprite strip: Y of the next sprite (BuildSpriteStrips)
DEF g_stripX EQU $c9d8 ; sprite strip: X of the next sprite
DEF g_stripTile EQU $c9d9 ; sprite strip: tile of the next sprite
DEF g_stripFlags EQU $c9da ; sprite strip flags: bit 7 down, bit 6 count tiles, bits 0-5 sprites left
DEF g_gameAEndTimer EQU $c9db ; Game A battle end countdown (state $13)
DEF g_gameAWindowY EQU $c9dc ; WY value set in VBlank when $ffaf bit 3 is set
DEF g_linkRecvC9dd EQU $c9dd ; byte received by link state $0d
DEF g_aiScanIndex EQU $d00e ; weapon AI scan index (0..15)
DEF g_aiCandidateCount EQU $d00f ; weapon AI candidate count
DEF g_enemySubFlags EQU $d011 ; enemy sub flags: bit 0 side/direction, bit 4 burst pending, bit 6 firing
DEF g_modeFlags13 EQU $d013 ; Game B effect flags: bit 6 = palette flash, bit 4 = window wobble
DEF g_oamBufToggle EQU $d014 ; bit 0 picks the OAM buffer
DEF g_subFireMode EQU $d019 ; sub fire mode: bit 6 burst, bit 5 single
DEF g_linkTxRead EQU $d02d ; link send ring read index
DEF g_padState EQU $d02e ; local pad byte used for Game B movement and link send
DEF g_gameBLayoutVariant EQU $d091 ; layout column (0..2) for LevelLayoutPtrs; seeded from InitD091 by bank 0
DEF g_scratch92 EQU $d092 ; scratch flags: bit 0 mirror, bit 1 borrow
DEF g_gameBAnimSpriteTable EQU $d097 ; sprite table read by GameB_DrawSixAnimSprites (word per sprite)
DEF g_shipsSpawned EQU $d09f ; number of ships spawned (compared with g_gameBShipCountM1)
DEF g_rasterCount EQU $d0a1 ; raster table length
DEF g_rasterLine EQU $d0a2 ; current raster table index
DEF g_cutsceneScrollX EQU $d0b6 ; cutscene scroll position; a new tile column every 8 pixels
DEF g_damageGaugeLevel EQU $d0dd ; damage gauge level, bumped when a player ship is lost
DEF g_gameBObjSlots EQU $d200 ; Game B object slots, 8 bytes each, bit 7 = active
DEF g_linkMsg EQU $d2dd ; current link message (6 bytes): type in low 4 bits
DEF g_linkMsgData EQU $d2df ; link message data byte
DEF g_linkRecvType EQU $d2e0 ; received message type; indexes LinkRecvTable
DEF g_sideSpeed EQU $d2ec ; ship speed per side (2 bytes); also scales the wave animation
DEF g_linkRecvPos EQU $d2f0 ; received partner position
DEF g_playerScreenX EQU $d2f2 ; player X in screen units, sent to the link partner
DEF g_objType7Timer EQU $d2f5 ; wait timer for object types 7 and 8
DEF g_gameClock EQU $d2f9 ; game clock frame counter; digits follow
DEF g_posRingWrite EQU $d304 ; partner position ring write index
DEF g_posRingRead EQU $d305 ; partner position ring read index
DEF g_posRing EQU $d306 ; partner position ring, 2-byte entries
DEF g_animFrameTimer EQU $d31d ; animation frame timer (period 8 or 10)
DEF g_animFlags EQU $d31e ; animation flags: bit 7 slow period
DEF g_linkPad EQU $d324 ; partner pad byte (2 bytes with $d325)
DEF g_worldWidthHi EQU $d32a ; world width high byte; X wraps here
DEF g_spriteDrawMode EQU $d32d ; object sprite draw mode, cleared before each draw pass
DEF g_linkSwapPair EQU $d336 ; 2 bytes swapped when the link game starts
DEF g_gameBSkill EQU $d337 ; Game B skill level (0..2); scales Table_50fd and picks difficulty tables
DEF g_gameBShipCountM1 EQU $d339 ; number of ships in the ship arrays minus 1
DEF g_hudCounterA EQU $d33a ; counter drawn by the HUD counters command
DEF g_hudCounterB EQU $d33b ; counter drawn by the HUD; added to $d379 on even scenes
DEF g_weaponBonusRow EQU $d340 ; pending weapon bonus row (1..3) into Table_4474; cleared when applied
DEF g_mgLockShots EQU $d349 ; MG lock-on shots used by the enemy sub AI
DEF g_burstDelay EQU $d355 ; enemy sub burst delay
DEF g_sonarSweepStep EQU $d35d ; sweep step 0-8
DEF g_stepDelay EQU $d363 ; delay before the next step (bank 0 entry $47e8)
DEF g_wreckSlotCycle EQU $d365 ; next wreck slot index
DEF g_gaugeBlinkCount EQU $d367 ; HUD damage gauge blink steps left (set to 5 by GameB_RegisterKill)
DEF g_barBlinkCount EQU $d368 ; HUD bar blink steps left
DEF g_paletteFlashTimer EQU $d369 ; palette flash timer ($d013 bit 6 effect)
DEF g_windowWobbleTimer EQU $d36d ; window wobble timer ($d013 bit 4 effect)
DEF g_gameBScenePhase EQU $d376 ; Game B scene phase (2 = setup, 3 = intro)
DEF g_gameBLevel EQU $d377 ; Game B level number (1-based)
DEF g_gameBStateTimer EQU $d378 ; frame counter used by Game B states
DEF g_aiCandidates EQU $d380 ; weapon AI candidate list (distance, index pairs)
DEF g_slotInitByte EQU $d3f9 ; last byte copied from DE by GameB_InitObjectSlot
DEF g_difficultyColumn EQU $d3fa ; low nibble = column read by GetDifficultyTableByte
DEF g_mirrorKeepFlags EQU $d400 ; 1 = GameB_MirrorSlots does not set bit 7 of byte +1
DEF g_firedTorpedoSlot EQU $d401 ; slot of the last fired torpedo
DEF g_cycle11 EQU $d413 ; counter cycling 0..10
DEF g_weaponFlagsBase EQU $d437 ; weapon flag bits; bits 0-2 become the top 3 bits of g_killFlagsPendingB
DEF g_subAiLevel EQU $d43c ; sub AI level; sets the burst delay
DEF g_subAiCloseRange EQU $d43d ; sub AI close range (depth / 2)
DEF g_mgAmmoPenalty EQU $d442 ; MG reload penalty (7 - value = refill)
DEF g_stepCountA EQU $d447 ; step counter (with $d448)
DEF g_selectedEnemyShip EQU $d449 ; index of the selected enemy ship (8-byte entries at $d219)
DEF g_waveSpawnCount EQU $d450 ; enemy ships left to spawn in this wave
DEF g_soundStep EQU $d451 ; sound step count, stops at 3
DEF g_soundStepIdx EQU $d452 ; index into Data_529f (wraps at 12)
DEF g_mgAmmoA EQU $d455 ; MG ammo A (twin mode copies to $d456)
DEF g_mgAmmoB EQU $d456 ; MG ammo B (copy of g_mgAmmoA in twin mode)
DEF g_selectedShipX EQU $d45b ; selected enemy ship X (2 bytes)
DEF g_linkSendHold EQU $d45d ; side 0 sends only when this is clear
DEF g_screenEffectTimers EQU $d460 ; 2 countdown timers ($d460, $d461) for screen effects
DEF g_gameBInitTimer EQU $d477 ; delay before Game B state $02 runs
DEF g_gameBKillTimer EQU $d478 ; delay before Game B state $1a runs
DEF g_rankCutsceneStep EQU $d825 ; 0 = load, 1 = run
DEF g_cutsceneD828Step EQU $d828 ; cutscene step, see GameB_TitleCutscene
DEF g_levelSelectStep EQU $d82a ; 0 = load, 1 = wait, 2 = blink
DEF g_cutsceneFlags EQU $d82b ; bit 0 = noise pending, bit 5/6 = input seen, bit 3 = intro done
DEF g_rankTimer EQU $d842 ; rank screen timer (with $d843)
DEF g_starCount EQU $d845 ; number of rank screen stars
DEF g_levelSelectPlayer EQU $d86e ; player being edited on the level select screen
DEF g_levelBlinkCount EQU $d875 ; level name blink steps (8)
DEF g_sfxSweepFreqLo EQU $de0d ; frequency low byte for the Sfx03/Sfx1C pitch sweep
DEF g_sfxSweepFreqHi EQU $de0e ; frequency high byte (NR14 value) for the Sfx03/Sfx1C pitch sweep
DEF g_sfxSweepSubsteps EQU $de0f ; sweep steps left in this frame (Sfx03/Sfx1C)
DEF g_sfxSweepDelay EQU $de13 ; frames until the next Sfx03 sweep step
DEF g_sfxPhase1 EQU $de15 ; SFX phase flag 1 (0 = attack part, 1 = release part)
DEF g_sfxPhase2 EQU $de16 ; SFX phase flag 2 (square 2 / noise second phase)
DEF g_sfxPhase3 EQU $de17 ; SFX phase flag 3 (NoiseSfx03 third phase)
DEF g_sfxTimer1 EQU $de1b ; SFX step timer 1 (square 1); SfxTickSq1Timer decrements it
DEF g_sfxTimer2 EQU $de1c ; SFX step timer 2 (square 2 / noise); SfxTickSq2Timer decrements it
DEF g_sfxReload1 EQU $de1d ; reload value for g_sfxTimer1 after each step
DEF g_sfxReload2 EQU $de1e ; reload value for g_sfxTimer2 after each step
DEF g_sfxSteps2 EQU $de1f ; steps left in the square 2 / noise part
DEF g_sfxSteps1 EQU $de20 ; steps left in the square 1 part
DEF g_soundQueue21 EQU $de21 ; request byte cleared every frame by SoundUpdate; never read in bank 4
DEF g_sfxSq1PtrHi EQU $de25 ; square 1 SFX data pointer, high byte
DEF g_sfxSq1PtrLo EQU $de26 ; square 1 SFX data pointer, low byte
DEF g_sfxSq2PtrHi EQU $de27 ; square 2 / noise SFX data pointer, high byte
DEF g_sfxSq2PtrLo EQU $de28 ; square 2 / noise SFX data pointer, low byte
DEF g_sfxFlag29 EQU $de29 ; SFX flag; written by several SFX starts, never read
DEF g_sfxBlockSfx0A EQU $de2a ; nonzero = Sfx0A_Start does nothing
DEF g_sfx0AActive EQU $de2b ; set while Sfx0A plays; never read
DEF g_sfx07Repeats EQU $de2c ; Sfx07 repeat count; at 0 the effect ends
DEF g_sfxBlockAll EQU $de2e ; nonzero = noise SFX, fades and several square SFX blocked (pause, Sfx0C)
DEF g_sfxBlockNoise09 EQU $de2f ; nonzero = NoiseSfx09_Start does nothing
DEF g_sfxNoiseBusy EQU $de30 ; set by NoiseSfxSetId; blocks Sfx06 and SoundCue1
DEF g_sfxLongRelease EQU $de31 ; nonzero = release time $28 instead of $10
DEF g_sfxSq2Active EQU $de32 ; nonzero = SfxSq2Update steps the square 2 part
DEF g_sfxFlag33 EQU $de33 ; SFX flag; cleared by Sfx0D and NoiseSfx01/04, never read
DEF g_sfxBlockNoise06 EQU $de34 ; nonzero = NoiseSfx06_Start does nothing
DEF g_sfxBlockSfx01 EQU $de35 ; nonzero = Sfx01 and Sfx18 do not start
DEF g_sfxPhase4 EQU $de36 ; cleared with the phase flags, never read
DEF g_sfx04Busy EQU $de39 ; set by Sfx04; blocks SoundCue1, Sfx06, Sfx0D, NoiseSfx01/02
DEF g_musicVibratoMode EQU $de40 ; vibrato mode of the channel being updated (low nibble of channel +8)
DEF g_musicFlag4c EQU $de4c ; music flag cleared by MusicInitSong/MusicSilence, never read
DEF g_musicKeepInstrument EQU $de4d ; nonzero = $9d instrument command does not store the square instrument
DEF g_musicFlag4e EQU $de4e ; cleared by MusicSilence, never read
DEF g_musicFlag4f EQU $de4f ; cleared by MusicSilence, never read
DEF g_musicCh1NoteTimer EQU $de52 ; channel 1 frames left in the note (struct +2)
DEF g_musicCh1PatternPtr EQU $de54 ; channel 1 pattern read pointer (struct +4)
DEF g_musicCh1VibratoStep EQU $de5e ; channel 1 vibrato step, +1 per frame (struct +14)
DEF g_musicCh1Flags EQU $de5f ; channel 1 flags (struct +15): bit 7 = muted for SFX, bits 0-6 repeat count
DEF g_musicCh2 EQU $de60 ; music channel 2 struct (16 bytes)
DEF g_musicCh2PatternPtr EQU $de64 ; channel 2 pattern read pointer
DEF g_musicCh2VibratoStep EQU $de6e ; channel 2 vibrato step
DEF g_musicCh2Flags EQU $de6f ; channel 2 flags: bit 7 = muted for SFX
DEF g_musicCh3 EQU $de70 ; music channel 3 (wave) struct (16 bytes)
DEF g_musicCh3PatternPtr EQU $de74 ; channel 3 pattern read pointer
DEF g_musicCh3WavePtr EQU $de76 ; channel 3 wave sample pointer (struct +6)
DEF g_musicCh3Volume EQU $de78 ; channel 3 instrument byte 3 (struct +8); bit 7 checked by MusicHoldNote
DEF g_musicCh3VibratoStep EQU $de7e ; channel 3 vibrato step
DEF g_musicCh3Flags EQU $de7f ; channel 3 flags: bit 7 = muted for SFX
DEF g_musicCh4 EQU $de80 ; music channel 4 (noise) struct (16 bytes)
DEF g_musicCh4PatternPtr EQU $de84 ; channel 4 pattern read pointer
DEF g_musicCh4NoiseData EQU $de86 ; channel 4 noise note data, 5 bytes from Music_NoiseTable
DEF g_musicCh4Flags EQU $de8f ; channel 4 flags: bit 7 = muted for SFX
DEF g_musicPanCounter EQU $de95 ; frames since the last stereo pan toggle
DEF g_musicPanPeriod EQU $de96 ; frames between stereo pan toggles
DEF g_musicPanToggle EQU $de97 ; bit 0 selects g_musicPanNR51A or g_musicPanNR51B
DEF g_musicPanNR51A EQU $de98 ; first NR51 value of the song
DEF g_musicPanNR51B EQU $de99 ; second NR51 value of the song
DEF g_soundCmdB1 EQU $deb1 ; cleared by MusicSilence, never read
DEF g_sfx1CSweepSpeed EQU $dec1 ; Sfx1C sweep steps per frame
DEF g_sfx1CEnvelope EQU $dec2 ; Sfx1C NR12 envelope value
DEF g_sfxSq1Busy EQU $dec3 ; nonzero = square 1 SFX playing; blocks Sfx06, Sfx17, SoundCue1 square part
DEF g_sfx1CPhase EQU $dec4 ; Sfx1C phase: 0 = first sweep, 1 = second sweep
DEF g_sfxFlagC6 EQU $dec6 ; SFX flag; written, never read
DEF g_soundFadeStopMusic EQU $deca ; nonzero = stop the music when the fade ends
DEF g_soundFadeTimer EQU $decb ; frames until the next fade step
DEF g_soundFadeSteps EQU $decc ; fade steps left
DEF g_soundFadePtrHi EQU $decf ; fade step table pointer, high byte
DEF g_soundFadePtrLo EQU $ded0 ; fade step table pointer, low byte
DEF g_sfxBlockCue1 EQU $ded3 ; nonzero = SoundCue1 does not start
DEF g_sfxBlockNoise0D EQU $ded4 ; nonzero = NoiseSfx0D/0E do not start
DEF g_sfx0AStepTime EQU $ded5 ; Sfx0A frames per step
DEF g_sfxBlockSfx0B EQU $ded6 ; nonzero = Sfx0B does not start
DEF g_sfxBlockSfx17 EQU $ded7 ; nonzero = Sfx17 does not start (set by Sfx0C)
DEF g_sfxBlockSfx03 EQU $ded8 ; nonzero = Sfx03 and Sfx0A do not start
DEF g_sfxBlockSfx0D EQU $ded9 ; nonzero = Sfx0D does not start
DEF g_noiseSfx0BKeepSq1 EQU $dedd ; set by NoiseSfx0B; NoiseSfx07 then does not give channel 1 back
DEF g_sfx1ABusy EQU $dede ; set by Sfx1A; blocks NoiseSfx0D/0E
DEF g_sfxBlockNoise05 EQU $dedf ; nonzero = NoiseSfx05 does not start
DEF g_sfxBlockSfx17B EQU $dee0 ; nonzero = Sfx17 does not start
DEF g_sfxPauseLock EQU $dee1 ; set while paused; blocks Sfx04 and Sfx0C
DEF g_sfxBlockSfx02 EQU $dee2 ; nonzero = Sfx02 does not start
DEF g_musicSavedSongId EQU $deff ; song ID saved by MusicSaveState
DEF g_frameParity EQU $ff81 ; bit 0 flips each VBlank in Game B while $d000 bit 2 is set
DEF g_gameAState EQU $ff90 ; Game A main state; indexes the table in GameA_RunState
DEF g_joyPressed EQU $ff99 ; buttons newly pressed this frame (title screen)
DEF g_pixelY EQU $ff9a ; pixel Y input for PixelToBgMapAddr
DEF g_pixelX EQU $ff9b ; pixel X input for PixelToBgMapAddr
DEF g_bgMapAddrHi EQU $ff9c ; BG map address high byte (PixelToBgMapAddr output)
DEF g_bgMapAddrLo EQU $ff9d ; BG map address low byte (PixelToBgMapAddr output)
DEF g_savedIE EQU $ff9e ; IE saved by WaitVBlankLcdOff
DEF g_gameAPlaceState EQU $ff9f ; Game A ship placement sub-state (0..6)
DEF g_gameATurnState EQU $ffa0 ; Game A battle turn sub-state; indexes the table in GameA_State0A_Turn
DEF g_gameACpuState EQU $ffa1 ; CPU turn step, table in GameA_State0D_CpuTurn
DEF g_gameATitleStep EQU $ffa2 ; level title step (state $0e)
DEF g_gameABoardStep EQU $ffa3 ; board step (state $12)
DEF g_gameAStatEffect EQU $ffa4 ; Game A STAT effect index (0-2)
DEF g_gameAState15Step EQU $ffa5 ; step of Game A state $15
DEF g_gameAState16Step EQU $ffa6 ; step of Game A state $16
DEF g_gameAResultsStep EQU $ffa7 ; results step (state $17)
DEF g_nextLevelStep EQU $ffa8 ; sub-state of GameA_NextLevelScreen
DEF g_gameAState1AStep EQU $ffa9 ; step of Game A state $1a
DEF g_gameFlags2 EQU $ffaa ; HRAM flags, rst $30 index 2: bits 0-1 fade running (pause logic), bit 1 link start, bit 6 link sync seen
DEF g_gameFlags3 EQU $ffab ; HRAM flags, rst $30 index 3: bit 0 link sync, bit 1 link ended, bit 2 VRAM queue pending, bit 3 link up, bit 4 new CPU turn, bit 7 run GameA_RunCutsceneScript
DEF g_gameFlags4 EQU $ffac ; HRAM flags, rst $30 index 4: bits 0-1 salvo cursor, bits 2-3 Lucky Shot markers shown
DEF g_gameFlags5 EQU $ffad ; HRAM flags, rst $30 index 5: bit 6 draw to BG map $9c00 instead of $9800
DEF g_gameFlags6 EQU $ffae ; HRAM flags, rst $30 index 6: bit 1 link cell received, bit 2 link moves a ship, bit 6 anim scripts enabled
DEF g_gameFlags8 EQU $ffaf ; HRAM flags, rst $30 index 8: bit 1 link byte $c9d1 received, bit 6 game over started
DEF g_gameFlags9 EQU $ffb0 ; HRAM flags, rst $30 index 9: bit 2 CPU turn started, bit 3 AI line mode, bit 4 AI path built
DEF g_gameATurnTimer EQU $ffb1 ; frame timer / row counter used by the turn steps
DEF g_listCount EQU $ffb2 ; general count; number of cells in the $c1bf list or shot list
DEF g_linkByteIndex EQU $ffb6 ; byte index for multi-byte link transfers
DEF g_shipPlaced EQU $ffb8 ; 5 bytes, one per ship; $ff = ship placed
DEF g_selectedShip EQU $ffbe ; ship being placed (0-4)
DEF g_placeBounds EQU $ffbf ; 4 bytes: cursor bounds so the placed ship fits the grid
DEF g_shipTileCount EQU $ffc3 ; ship tiles left to turn and copy
DEF g_shipTileSrc EQU $ffc4 ; 2 bytes: source of the next ship tile (bank 2), or the cutscene script pointer (bank 1)
DEF g_shipTileDst EQU $ffc6 ; 2 bytes: VRAM target of the next ship tile
DEF g_placeDirection EQU $ffc8 ; ship placement direction to try (0..3)
DEF g_placeNextState EQU $ffc9 ; placement sub-state to enter after the animation
DEF g_cursorRow EQU $ffcc ; grid cursor row
DEF g_cursorCol EQU $ffcd ; grid cursor column
DEF g_savedCursorRow EQU $ffce ; copy of g_cursorRow
DEF g_savedCursorCol EQU $ffcf ; copy of g_cursorCol
DEF g_redrawTimers EQU $ffd1 ; 3 timer pairs for GameA_TickRedrawTimers
DEF g_blinkTimer EQU $ffe1 ; frame timer for GameA_BlinkLuckyMarkers (24 frames) and GameA_State04_Delay
DEF g_gameADelay EQU $ffe5 ; Game A frame delay; GameA_RunState waits while nonzero
DEF g_joyPressedA EQU $ffe6 ; Game A buttons newly pressed this frame
DEF g_frameCounter EQU $ffe7 ; Game B frame counter (bank0 Game B frame incs it); Game A frame reuses it as last raw joypad, $ffe6 = new presses
DEF g_tmpE8 EQU $ffe8 ; scratch byte (pointer low / offset)
DEF g_tmpE9 EQU $ffe9 ; scratch byte (pointer high / flag)
DEF g_tmpEA EQU $ffea ; scratch byte (write offset / saved coordinate)
DEF g_fadeTargetObp0 EQU $c0a1 ; OBP0 target for UpdatePaletteFade (follows the BGP target at $c0a0).
DEF g_fadeTargetObp1 EQU $c0a2 ; OBP1 target for UpdatePaletteFade.
DEF g_gameATotalTurns EQU $c0b0 ; Game A turns summed over all levels; shown on level 3 and the link results.
DEF g_statEffectTimer EQU $c0b1 ; Countdown of steps left in the STAT/LYC raster effect.
DEF g_aiPatternPtr EQU $c771 ; Low byte of the pointer to the AI search pattern run table.
DEF g_aiPatternPtrHi EQU $c772 ; High byte of the pointer to the AI search pattern run table.
DEF g_aiRunCellsLeft EQU $c773 ; Cells left in the current AI search run; zero picks a new run.
DEF g_aiRunCellIndex EQU $c774 ; Index of the next cell in the expanded run list at $c7b7.
DEF g_aiRunsLeft EQU $c776 ; Number of unused runs left in the AI search pattern.
DEF g_aiLinesDone EQU $c80b ; Count of hit lines the AI finished (both ends tried) in state 3.
DEF g_aiHitsScored EQU $c80c ; CPU hits added to the hit list; the easy AI hunts while this is below ships afloat.
DEF g_aiSavedState EQU $c80d ; AI state saved while the AI runs special state 6; restored after it.
DEF g_activeFleetGridPtr EQU $c810 ; Low byte of the pointer to the fleet grid being checked (also $c812 = shot grid pointer).
DEF g_activeFleetGridPtrHi EQU $c811 ; High byte of the pointer to the fleet grid being checked.
DEF g_scriptSpriteSlot EQU $c876 ; Next OAM slot for sprites placed by the cutscene script.
DEF g_cpuCursorRowSteps EQU $c923 ; CPU aim cursor: rows left to move; bit 7 = move up.
DEF g_cpuCursorColSteps EQU $c924 ; CPU aim cursor: columns left to move; bit 7 = move left.
DEF g_cpuCursorRow EQU $c925 ; CPU aim cursor current row.
DEF g_cpuCursorCol EQU $c926 ; CPU aim cursor current column.
DEF g_scrollXDirSteps EQU $c928 ; SCX scroll: bit 7 = direction, bits 0-6 = steps left (pairs with g_scrollDirSteps for SCY).
DEF g_savedScrollYSteps EQU $c929 ; Copy of the start SCY scroll steps; reversed to scroll back at game over.
DEF g_savedScrollXSteps EQU $c92a ; Copy of the start SCX scroll steps; reversed to scroll back at game over.
DEF g_shownShipLength EQU $c934 ; Length of the ship shown as sprites; counts down as its sprites move.
DEF g_shotTallyTimer EQU $c949 ; Frame countdown before the CPU shot and hit tallies ($c0ac/$c0ad) go up.
DEF g_salvoLaunchSfxCount EQU $c94b ; Salvo launch sounds played so far; stops at the salvo count.
DEF g_cpuShotCell EQU $c960 ; Packed row/column of the cell the CPU just fired at.
DEF g_shotSpritesLaunched EQU $c961 ; Shot sprites launched so far in the fly animation (turn step 4).
DEF g_shotLaunchDelay EQU $c962 ; Frames until the next shot sprite launches (reload $0e).
DEF g_salvoResultsShown EQU $c964 ; Splash results shown so far; goes up to g_salvoResultCount.
DEF g_shotAnimDelay EQU $c978 ; Low byte of a 16-bit frame countdown that holds the shot animation.
DEF g_shotAnimDelayHi EQU $c979 ; High byte of the shot animation frame countdown.
DEF g_ambientSfxTimer EQU $c9af ; Frames until the next random ambient sound (GameA_RandomAmbientSfx).
DEF g_p1FaceAnimTimer EQU $c9b2 ; P1 commander face animation frame timer on the results screen.
DEF g_p1FaceAnimPeriod EQU $c9b3 ; P1 commander face animation reload period.
DEF g_p2FaceAnimTimer EQU $c9b5 ; P2 commander face animation frame timer on the results screen.
DEF g_p2FaceAnimPeriod EQU $c9b6 ; P2 commander face animation reload period.
DEF g_p1HitRank EQU $c9c7 ; P1 hit ratio rank (1..4) from HitRatioRankThresholds; picks the medal.
DEF g_commanderChangeCount EQU $c9ce ; Commander changes this frame in the options screen; cleared each frame.
DEF g_gameABlinkTimer EQU $c9cf ; Shared $18-frame blink/step timer for menus and screen flips.
DEF g_linkNewMsgBase EQU $d00d ; Link message type minus 1 that LinkSend_00 sends on a new request. No direct writes found.
DEF g_modeFlags1b EQU $d01b ; Game B flag byte: bit 1 set on outfit confirm, bits 2 and 6 cleared by the enemy object scan, bit 7 cleared with the fire flags.
DEF g_aiPadState EQU $d02f ; Pad byte for the computer side: $10 or $20 by enemy ship bit 5. Object moves read it in place of g_padState.
DEF g_hitTargetFlags EQU $d093 ; Shot flags with bit 7 flipped, so bit 7 picks the target side for the projectile hit test.
DEF g_introFadeHi EQU $d094 ; High byte of the intro fade step counter at $d376. GameB_StepScoreCount runs only when it is 1.
DEF g_sixAnimCounter0 EQU $d098 ; Counter of sprite 0 for GameB_DrawSixAnimSprites. Also holds the score count and a saved slot byte.
DEF g_sixAnimCounter1 EQU $d099 ; Counter of sprite 1 for GameB_DrawSixAnimSprites. Also the wreck slot pointer low byte and the depth charge sprite count.
DEF g_sixAnimCounter2 EQU $d09a ; Counter of sprite 2 for GameB_DrawSixAnimSprites. Also the wreck slot pointer high byte.
DEF g_sixAnimCounter3 EQU $d09b ; Counter of sprite 3 for GameB_DrawSixAnimSprites. Also an X position the scene end draw counts up.
DEF g_rasterLyc2 EQU $d0a4 ; LYC line of raster band 2 in the STAT raster table at $d0a3. The intro sets it to $77.
DEF g_rasterScx1 EQU $d0b0 ; SCX of raster band 1. Also the start of the link handshake buffer.
DEF g_rasterScx2 EQU $d0b1 ; SCX of raster band 2. The intro slide and the rank screen move it.
DEF g_rasterScx3 EQU $d0b2 ; SCX of raster band 3. Set to $60 at game over and in the menus.
DEF g_rasterScy1 EQU $d0be ; SCY of raster band 1. The damage gauge writes it.
DEF g_rasterScy2 EQU $d0bf ; SCY of raster band 2. The damage gauge writes it after a hit.
DEF g_rasterScy3 EQU $d0c0 ; SCY of raster band 3. Cleared when the damage gauge reaches $71.
DEF g_linkSyncIndex EQU $d2e3 ; Byte index of the link slot sync in link state 5. Ends at $1e or $20.
DEF g_mgHitMarker EQU $d2ea ; Cleared when an MG shot hits a ship and on scene reset. No reads found.
DEF g_torpedoHitMarker EQU $d2eb ; Cleared when a torpedo hits a ship and on scene reset. No reads found.
DEF g_linkStartMarker EQU $d323 ; Set to $ff at Game B start and $0f when the link game starts. No reads found.
DEF g_scanSlotClass EQU $d345 ; First byte of the object slot being handled. Bits 4-6 give the enemy class.
DEF g_depthBand EQU $d347 ; Depth band (0..7) found in Data_6f71 for a projectile. GameB_DepthBand reads it. The pad/clock screen also uses it as the next sub-state.
DEF g_shotFlags EQU $d348 ; Flag byte of the shot being handled: bit 7 side, bit 3 lock-on. The pad/clock screen also uses it as a timeout ($ff = none).
DEF g_setupObjType EQU $d3fb ; Object type byte given to GameB_SetupObjectSlot. Cleared after a projectile hit check. No reads found.
DEF g_objShapeAttr EQU $d3fe ; Data_6e78 byte for the current object class and subtype. Bits 6-7 are draw flags.
DEF g_enemyBulletSpeed EQU $d405 ; Bullet speed from GameB_UnknownCurve6e10 for the firing ship class. Bank 0 uses bits 4-6 for the wreck type.
DEF g_enemySubResetByte EQU $d414 ; Cleared when the enemy sub resets for a new approach. No reads found.
DEF g_depthChargeOrigin EQU $d44a ; Base coordinate that the OffsetRowPtrs rows subtract from when depth charges draw. Set to $22 or $58.
DEF g_kind22ActiveCount EQU $d476 ; Number of kind $22 objects started (max 2). Types 10-12 count it down when one ends.
DEF g_timeBonusDigit100 EQU $d809 ; Hundreds digit of the Game B time bonus. TallyState3 writes it from the time left under the rank floor.
DEF g_timeBonusDigit1000 EQU $d80a ; Thousands digit of the Game B time bonus.
DEF g_timeBonusDigit10000 EQU $d80b ; Ten-thousands digit of the Game B time bonus. The tally copies digits from here down into the banner.
DEF g_cutsceneFlipTimer EQU $d831 ; Cutscene frame timer (16). At zero it toggles $d832 and flips the sprites at $c020.
DEF g_cutsceneScriptDelay EQU $d833 ; Frames until the next step of GameB_CutsceneSpriteScript. Each step reloads it from the script.
DEF g_cutsceneScriptPos EQU $d834 ; Byte offset of the next step in GameB_CutsceneSpriteScript.
DEF g_cutsceneFlashIndex EQU $d837 ; Index into GameB_CutsceneFlashPtrs. Starts at $ff and goes up by one for each script step.
DEF g_scrollNoiseCounter EQU $d838 ; Counts new tile columns in the cutscene scroll. Every 8 columns it plays noise SFX $0e.
DEF g_rankStripSpriteCount EQU $d844 ; Number of rank screen strip sprites that RankScreen_State02 moves. Loaded from Data_7e37.
DEF g_tallyBlinkRoundsLeft EQU $d870 ; Blink rounds left while both score rows blink. The level select wait moves on when it reaches 0.
DEF g_rankFlybyPassesLeft EQU $d876 ; Number of sprite fly-by passes left in RankScreen_State04 (1 or 2).
DEF g_rankTextScriptEnd EQU $d882 ; Terminator byte after the 7 rank-name tiles in the $d878 tilemap script. Also the minutes tens digit in the time bonus banner.
DEF g_tallyTimeMinutes EQU $d883 ; Minutes ones digit in the time bonus banner at $d878. The level select blink writes 0 here as the script terminator.
DEF g_tallyKillCountTile EQU $d88c ; Kill-count digit tile in the tally row banner at $d878.
DEF g_tileRowScriptEnd EQU $d89b ; Terminator byte after a 32-tile row in the $d878 tilemap script.
DEF g_shipSpriteBlinkTimer EQU $ffca ; Game A frame timer for blinking the ship sprites. At zero it flips the sprite palette and reloads.
DEF g_p2ShipHitsLeft EQU $ffd7 ; 5 bytes: hits left per player 2 ship. Battle setup copies them from ShipHitPointsByLevel.
DEF g_replyShotFlag EQU $ffdc ; Set to 1 when player 2 takes a reply shot after the player 1 fleet is gone. With it set, a sunk player 2 fleet does not end in a plain win.
DEF g_shipCellId EQU $ffdf ; Ship cell value (bits 0-5 cell id, bits 6-7 direction). Ship tile drawing and the CPU hit check use it.
DEF g_blinkCount EQU $ffe2 ; Blink step counter for g_blinkTimer. Bit 0 also selects show or hide of a 7-tile text.
DEF g_gameAFxTimer EQU $ffe3 ; Game A effect frame timer (8) for the board flash and state 16 sprite moves.
DEF g_gameAFxSteps EQU $ffe4 ; Game A effect steps left for the board flash and state 16 sprite moves.
DEF g_fadeTargetBgp EQU $c0a0 ; BGP target for UpdatePaletteFade; OBP0 and OBP1 targets follow at $c0a1/$c0a2
DEF g_fadeStepTimer EQU $c0a3 ; Frames until the next palette fade step (reload 8)
DEF g_fadeStepCount EQU $c0b7 ; Palette fade steps done; the fade stops after 3
DEF g_p1ClockFrames EQU $c0bc ; P1 play clock frame counter; at 60 it adds a second to $c0b9
DEF g_p2ClockFrames EQU $c0bd ; P2 play clock frame counter; at 60 it adds a second to $c0bb
DEF g_oamBufferB EQU $c100 ; Second 160-byte shadow OAM buffer (cursor in g_spriteCursorB)
DEF g_savedShipHitsLeft EQU $c765 ; 6 bytes: copy of g_shipHitsLeft saved at battle end and restored at the next setup
DEF g_aiRunOffsets EQU $c777 ; AI search pattern run offsets (0, 3, 6, ...); one is picked at random per run
DEF g_aiRunCells EQU $c7b7 ; expanded cell list (row, col pairs) of the current AI search run
DEF g_aiSunkLineCell EQU $c80e ; 2 bytes: row, col of the sunk-ship cell that GameA_AiMarkSunkLine walks from
DEF g_activeShotGridPtr EQU $c812 ; pointer to the shot grid being checked (pairs with g_activeFleetGridPtr)
DEF g_openNeighborCells EQU $c816 ; list of (row, col) neighbour cells not yet shot; one is picked at random
DEF g_foundShipCell EQU $c866 ; 2 bytes: row, col of the fleet grid cell found by value $15; compared with g_linkCellRow/Col
DEF g_shipCellList EQU $c86a ; (row, col) pairs of the ship cells collected by GameA_ProcessShipList
DEF g_obp1CycleTimer EQU $c947 ; frame countdown (16) for the link-play OBP1 colour cycle; next byte is the cycle step
DEF g_salvoLaunchSfxTimer EQU $c94a ; frame countdown between salvo launch sounds; next byte counts the sounds
DEF g_salvoResultsAnimated EQU $c965 ; count of salvo results with an anim script started; done when it equals g_salvoResultCount
DEF g_salvoShots EQU $c97c ; salvo shot list, 2 bytes per shot: packed cell, ship index
DEF g_salvoShipsHit EQU $c99e ; 6 bytes: distinct ship indexes hit by the salvo; $ff = empty
DEF g_hitFlashStarted EQU $c9ac ; set once turn step 9 starts the hit-ship flash; cleared before the step
DEF g_boardScroll EQU $c9b0 ; 2 bytes: board SCY, SCX restored after the STAT map-swap effect
DEF g_gameABlinkStep EQU $c9d0 ; step value paired with g_gameABlinkTimer
DEF g_padEdgeHistory EQU $c9d5 ; joypad bytes and press edges; shifted down into g_linkSyncBytes each frame
DEF g_mgTrailPhase EQU $d012 ; MG trail state per side (indexed by slot): bit 3 picks the trail half, bits 0-2 count down
DEF g_enemySubDirLock EQU $d01f ; bit 0 set = enemy sub move direction is fixed
DEF g_gameBItemCount EQU $d096 ; items added by GameB_MaybeAddItem (max 4)
DEF g_rasterLycTable EQU $d0a3 ; STAT raster LYC table, band 1 first; filled from Data_7272
DEF g_rasterScxTable EQU $d0af ; STAT raster SCX table indexed by g_rasterLine (entry 0 = top band)
DEF g_rasterScyTable EQU $d0bd ; STAT raster SCY table indexed by g_rasterLine (entry 0 = top band)
DEF g_damageGaugeRows EQU $d0cd ; 13 bytes of damage gauge row offsets; cleared by GameB_ClearDamageGauge
DEF g_damageGaugeStep EQU $d0de ; damage gauge step countdown; GameB_DamageGaugeEntry reads it with g_damageGaugeLevel
DEF g_enemyObjSlots EQU $d278 ; 12 enemy-side object slots, 8 bytes each; GameB_UpdateEnemyObjects runs them
DEF g_spawnObjSlots EQU $d298 ; 8 object slots, 8 bytes each; GameB_FindFreeSlotD298 copies a new object into a free one
DEF g_bonusHitCountA EQU $d2e7 ; hits on the player ship while it has bonus weapons; at 6 it loses a weapon bit
DEF g_bonusHitCountB EQU $d2e8 ; hits on the enemy ship while it has bonus weapons; at 6 it loses a weapon bit
DEF g_paletteShadow EQU $d31f ; 3 bytes: BGP, OBP0, OBP1 copied from PaletteTable by SetPalettes.d31fEntry
DEF g_worldXScale EQU $d32b ; word; layout width * 2; divisor that turns world X into screen X
DEF g_mgTrailEmitTimer EQU $d353 ; 2 bytes, one per side: frames until the next MG trail particle (reload 2)
DEF g_playStats EQU $d379 ; scene stats: +0 HUD counter sum, +2 depth sum, +3 seconds, +4 minutes (max 99)
DEF g_wavePhases EQU $d3d6 ; wave phase per Data_6b5a band; GameB_AnimateWaves steps it by the side speed
DEF g_enemyFireDelays EQU $d415 ; per-ship fire delay, indexed by ship byte +6 bits 0-4; the ship fires when it reaches 0
DEF g_enemyHitCount EQU $d429 ; hits scored on enemy ships; follows the torpedo and MG fire counts
DEF g_randomLimits EQU $d42f ; 2 bytes copied from RandomLimitTable at level load; no direct reads found
DEF g_startClearPair EQU $d44d ; 2 bytes cleared at Game B start next to the game clock; no reads found
DEF g_launchSideToggle EQU $d462 ; 2 bytes, one per side: launch direction bit; flips while the side's screen effect timer runs
DEF g_levelGroupList EQU $d466 ; enemy group list copied from the level layout; each group ends in $ff
DEF g_killCountsByClass EQU $d800 ; 7 bytes: kills per enemy class; the tally reads them per row
DEF g_timeBonusDigit1 EQU $d807 ; ones digit of the Game B time bonus; TallyState3 clears the 5 digits from here
DEF g_cutsceneScrollPrevX EQU $d82d ; last g_cutsceneScrollX seen by GameB_CutsceneScrollTiles
DEF g_cutsceneSpriteFrac EQU $d82e ; fraction that g_cutsceneSpriteSpeed adds into; overflow moves the sprites
DEF g_cutsceneSpriteSpeed EQU $d82f ; speed added to g_cutsceneSpriteFrac each frame
DEF g_cutsceneSpriteStep EQU $d830 ; signed pixels the 8 cutscene sprites at $c001 move on each overflow
DEF g_cutsceneFlashTimer EQU $d835 ; frames until the next cutscene flash step; reloads from g_cutsceneFlashPeriod
DEF g_cutsceneFlashPeriod EQU $d836 ; reload value for g_cutsceneFlashTimer; goes down by one per script step
DEF g_rankSparkleSlots EQU $d83a ; 4 x 2-byte entries (timer, frame) for rank screen sprite bursts; $ff = free
DEF g_starRecords EQU $d846 ; star records after g_starCount (fraction, speed, ...); copied from Data_7d85 or Data_7f3c
DEF g_cutsceneObjMotion EQU $d85e ; 7 x 2-byte (fraction, speed) entries seeded from Data_7f4f
DEF g_tallyBlinkTimer EQU $d86f ; frame timer (20) for GameB_BlinkTallySprites
DEF g_levelSelectLock EQU $d871 ; 2 bytes, one per player: nonzero blocks level changes; Start confirms only at 1
DEF g_tileScript EQU $d878 ; tilemap script buffer: VRAM address hi, lo, length, then tiles; run via rst $28
DEF g_cursorRepeatDelay EQU $ffd0 ; Game A key-repeat delay for GameA_MoveGridCursor (7 frames)
