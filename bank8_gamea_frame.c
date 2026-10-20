// Game A frame, state dispatch, the battle setup and ship placement
// (was GameA_Frame to GameA_State09_TurnInit in bank_000.asm).
#include "bank8_gamea.h"

// Add a VRAM copy to the queue at $c4c0 (QueueVramCopy). Returns the
// entry + 4; the caller stores the length, repeat count and bank.
uint8_t *gaQueueVramCopy(const uint8_t *src, uint16_t dst)
{
    uint8_t *e = &g_vramCopyQueue + (uint8_t)(g_vramCopyCount * 7);
    e[0] = (uint16_t)src;
    e[1] = (uint16_t)src >> 8;
    e[2] = dst;
    e[3] = dst >> 8;
    return e + 4;
}

static void gaPlaceInput(void);
static void gaPlaceWaitAnim(void);
static void gaPlaceCommitShip(void);
static void gaPlaceConfirm(void);

// Game A state $00: load the intro text screen.
static void gaState00IntroInit(void)
{
    LcdOffMaskIrqs();
    GA_COPY(2, 0x94e0, GameA_GfxAirStrikeBgTiles, 0x320);
    GA_COPY(2, 0x8800, GameA_GfxAirStrikeBgTiles2, 0x2a0);
    GA_COPY(2, 0x8d00, GameA_GfxReadyBanner, 0x130);
    GA_COPY(2, 0x8010, GameA_GfxAirStrikeSprites, 0x240);
    textRevealScreenInit();
    RAM(g_animScriptPtrs)[0] = 0x19;    // anim script 0 = $6e19 (bank 1)
    RAM(g_animScriptPtrs)[1] = 0x6e;
    gaPadCycles(269);
    LcdOnEnableIrqs();
}

// Game A state $03: load the battle screen and read the player fleet.
static void gaState03BattleInit(void)
{
    LcdOffMaskIrqs();
    ClearShadowOAM();
    GA_COPY(2, 0x9490, GameA_GfxBgTiles + 0x490, 0xc0);
    GA_COPY(2, 0x8aa0, GameA_GfxUiTiles + 0x2a0, 0x560);
    GA_COPY(2, 0x8800, GameA_GfxUiTiles + 0x3b0, 0x110);
    GA_COPY(2, 0x8000, GameA_GfxSprites, 0x800);
    setupBattle();
    loadPlayerFleet(0);
    // The direction of each ship: the top 2 bits of its first cell.
    uint8_t *rec = RAM(g_shipRecords);
    const uint8_t *len = g_p1ShipLengths;
    uint8_t b = 1, c = 1;
    for (uint8_t n = 5; n; n--) {
        uint16_t rc = findGridCellByValue(RAM(g_playerFleetGrid), c, b);
        *rec++ = *GRID_CELL(RAM(g_playerFleetGrid), rc >> 8, rc) >> 6;
        b += *len++;
        c = b;
    }
    gaDrawShipRecords(GameA_VictorySpriteLayout + 0xf);
    b = 0;
    for (uint8_t i = 0; i < 5; i++)
        if (g_shipHitsLeft[i])
            b++;
    g_shipsAfloat = b;
    if (g_p2Commander != 1 || !g_shipHitsLeft[0])
        g_gameFlags9 |= 0x04;
    for (uint8_t i = 0; i < 6; i++) {
        g_savedShipHitsLeft[i] = g_shipHitsLeft[i];
        g_shipHitsLeft[i] = 0;
    }
    g_selectedShip = 0;
    g_gameATurnTimer = 0;
    for (uint8_t i = 0; i < 6; i++)
        g_shipPlaced[i] = 0xff;
    drawArrangeScreen();
    g_fadeTargetObp0 = 0xaa;
    if (g_topModeFlags & 0x80)
        g_gameALinkState = 1;
    g_gameFlags5 &= ~0x01;
    g_gameAState = 0x05;
    // This load is 52168 cycles slower than the asm (the bank 1 and 9 C).
    // Pad it to one full timer period (73728) so the timer IRQ phase stays.
    gaPadCycles(674);
    LcdOnEnableIrqs();
}

// Game A state $05: ship placement.
static void gaState05Placement(void)
{
    blinkShipSprites();
    switch (g_gameAPlaceState) {
    case 0: placeState0WaitButton(); break;
    case 1: gaPlaceInput(); break;
    case 2: placeState2TurnShip(); break;
    case 3: gaPlaceWaitAnim(); break;
    case 4: drawShipSprites(); break;
    case 5: gaPlaceCommitShip(); break;
    case 6: gaPlaceConfirm(); break;
    }
}

// Start: when all ships are placed, show "ready?" (sub-state 6).
static void gaPlaceOnStart(void)
{
    if (checkAllShipsPlaced())
        return;
    gaSpriteStrips(GameA_PlacementSpriteLayout, 0x1009, OAM);
    g_gameFlags5 |= 0x01;
    g_gameAPlaceState = 6;
}

// A: try the 4 directions for the current ship.
static void gaPlaceOnA(void)
{
    if (!checkAllShipsPlaced())
        return;
    g_placeDirection = 0;
    for (;;) {
        calcPlacementBounds(g_placeDirection);
        if (!checkCursorInBounds() && !checkShipFits(RAM(g_playerFleetGrid), g_placeDirection))
            break;
        if (++g_placeDirection == 4) {
            NoiseSfx_Queue = 0x05;
            return;
        }
    }
    setShipTileSource();
    SFX_Queue = 0x08;
    g_gameAPlaceState = 3;
    g_placeNextState = 4;
}

// Placement sub-state 1: move the cursor, read the buttons.
static void gaPlaceInput(void)
{
    moveGridCursor();
    uint8_t p = g_joyPressedA;
    if (p & 0x08)
        gaPlaceOnStart();
    else if (p & 0x04)
        selectNextShip();
    else if (p & 0x02)
        placeOnBPickUpShip();
    else if (p & 0x01)
        gaPlaceOnA();
}

// Placement sub-state 3: wait for the ship animation.
static void gaPlaceWaitAnim(void)
{
    if (g_gameFlags3 & 0x04)
        return;
    uint8_t done = rotateShipTileStep();
    g_gameFlags3 |= 0x04;
    if (done)
        g_gameAPlaceState = g_placeNextState;
}

// Placement sub-state 5: write the ship to the fleet grid.
static void gaPlaceCommitShip(void)
{
    placeGetShipSprite();
    writeShipToGrid(RAM(g_playerFleetGrid));
    uint8_t i = g_selectedShip;
    RAM(g_shipRecords)[i] = g_placeDirection;
    g_shipPlaced[i] = 0xff;
    g_shipHitsLeft[i] = 0;
    g_gameFlags3 |= 0x04;
    selectNextShip();
    g_gameAPlaceState = 1;
}

// Placement sub-state 6: the "ready?" yes/no prompt.
static void gaPlaceConfirm(void)
{
    uint8_t p = g_joyPressedA;
    if (p & 0x80) {
        g_gameFlags2 |= 0x80;
        g_oamBuffer = 0x6f;
        return;
    }
    if (p & 0x40) {
        g_gameFlags2 &= ~0x80;
        g_oamBuffer = 0x5f;
        return;
    }
    if (!(p & 0x09))
        return;
    ClearShadowOAM();
    if (g_gameFlags2 & 0x80) {
        g_gameFlags2 &= ~0x80;
        showTargetCursor();
        g_gameFlags5 &= ~0x01;
        g_gameAPlaceState = 1;
        return;
    }
    BGM_Queue = 0xff;
    g_titleStartSfx = 0x05;
    g_gameFlags3 |= 0x08;
    g_gameAState = 0;
    g_gameADelay = 0x26;
    if (g_topModeFlags & 0x80) {
        // Link game: start the serial timer and send "placement done".
        rTMA = 0xb8;
        rTAC = 0xfc;
        g_gameAState = 0x04;
        g_serialSend = 0xf0;
        g_linkSyncBytes[1] = 0xf0;
        g_blinkTimer = 0x18;
        g_blinkCount = 0;
        return;
    }
    g_gameFlags2 |= 0x02;
}

// Game A state $09: start a turn.
static void gaState09TurnInit(void)
{
    g_targetInCenter |= 0x04;
    rOBP1 = 0x3e;
    g_gameAState = 0x0d;
    g_gameATurnTimer = 0;
    g_gameACpuState = 0;
    if ((g_gameFlags6 & 0x02) && !(g_gameFlags6 & 0x04)) {
        // The aircraft launch cutscene.
        g_gameFlags6 |= 0x04;
        g_gameFlags3 |= 0x10;
        g_gameACpuState = 0x08;
        g_shipTileSrc[0] = (uint16_t)GameA_CutsceneAircraftLaunching;
        g_shipTileSrc[1] = (uint16_t)GameA_CutsceneAircraftLaunching >> 8;
        g_scriptSpriteSlot = 0;
        uint16_t rc = findGridCellByValue(RAM(g_playerFleetGrid), 0x15, 0x03);
        g_foundShipCell[0] = rc >> 8;
        g_foundShipCell[1] = rc;
        uint8_t m = getGridMargin();
        uint8_t a = gaScrollStepsTo(rc >> 8, m);
        g_scrollDirSteps = a;
        g_savedScrollYSteps = a;
        a = gaScrollStepsTo(rc, m);
        g_scrollXDirSteps = a;
        g_savedScrollXSteps = a;
        g_gameALevel &= ~0x02;
        BGM_Queue = 0x09;
        return;
    }
    drawShipCellMarkers();
    g_obp1CycleTimer = 0x10;
    RAM(g_obp1CycleTimer)[1] = 0;
    g_p2Options |= 0x80;
    g_gameFlags6 |= 0x20;
    if (g_gameFlags4 & 0x0c) {
        gaSpriteStrips(GameA_ShipCellIdRange + 0xf, 0x0006, OAM + 0x10);
        g_blinkTimer = 0x18;
        g_gameACpuState = 0x11;
        g_gameATurnTimer = 0xa0;
    }
    if (g_topModeFlags & 0x80) {
        g_cpuCursorRowSteps = 0;
        RAM(g_cpuCursorRowSteps)[1] = 0;
        g_gameACpuState = 1;
        return;
    }
    g_gameADelay = 0x40;
}

// Game A state $0b: after the turn, go to state $14 (or $09).
static void gaState0BAfterTurn(void)
{
    if (g_gameFlags2 & 0x04)
        return;
    rOBP0 = 0x27;
    g_gameATurnTimer = 0;
    g_statEffectTimer = 0x24;
    g_gameAState = 0x14;
    if (g_topModeFlags & 0x01)
        g_gameAState = 0x09;
    if (!(g_topModeFlags & 0x80)) {
        BGM_Queue = 0x05;
        return;
    }
    if (!(g_topModeFlags & 0x40))
        return;
    g_serialSend = 0x0a;
    g_gameALinkState = 0x09;
}

// Game A state $0c: move 28 sprites down to Y $87, then hide them.
static void gaState0CSinkSprites(void)
{
    uint8_t *p = OAM + 0x30;
    for (uint8_t c = 0x1c; c; c--, p += 4) {
        if (*p == 0xf0)
            continue;
        if (*p == 0x87)
            *p = 0xf0;
        else
            (*p)++;
    }
    if (--g_gameATurnTimer)
        return;
    g_gameAState = 0x1c;
    g_linkSyncBytes[0] = 0;
    RAM(g_linkSyncBytes[0])[2] = 0;
    RAM(g_linkSyncBytes[0])[3] = 0;
}

// Run one frame of Game A logic (GameA_RunState). GameA_Frame has run
// updateScroll and cyclePaletteOBP1 first.
static void gaRunState(void)
{
    if (g_gameFlags2 & 0x03)
        return;
    if (g_gameADelay) {
        g_gameADelay--;
        return;
    }
    if (g_gameFlags3 & 0x80) {
        runCutsceneScript();
        return;
    }
    switch (g_gameAState) {
    case 0x00: gaState00IntroInit(); break;
    case 0x01: textRevealScreenUpdate(); break;
    case 0x02: state02Init(); break;
    case 0x03: gaState03BattleInit(); break;
    case 0x04: state04Delay(); break;
    case 0x05: gaState05Placement(); break;
    case 0x06: break;
    case 0x07:
        loadPlayerFleet(1);
        g_gameAState = 0x10;
        g_gameATurnTimer = 0;
        break;
    case 0x08: state08(); break;
    case 0x09: gaState09TurnInit(); break;
    case 0x0a: gameATurnState(); break;
    case 0x0b: gaState0BAfterTurn(); break;
    case 0x0c: gaState0CSinkSprites(); break;
    case 0x0d: gameACpuState(); break;
    case 0x10: state10(); break;
    case 0x11: state11(); break;
    case 0x13: gameAState13BattleEnd(); break;
    case 0x14: state14(); break;
    case 0x19: state19(); break;
    case 0x1d: state1DSlideWindow(); break;
    case 0x1f: state1FWaitFade(); break;
    case 0x20: state20FadeOut(); break;
    case 0x1e: gaBoardLoad1E(); break;     // direct: the load must start in this VBlank
    default: gameAStates0E(); break;
    }
}

// Game A per-frame entry (GameA_Frame calls it after updatePlayClock):
// read the joypad edges, then run the Game A state.
void gameAFrame(void)
{
    uint8_t raw = g_joyRaw;
    uint8_t last = g_frameCounter;  // $ffe7 = last raw joypad
    g_frameCounter = raw;
    g_joyPressedA = raw & ~last;
    gaRunState();
    updateAnimScripts();
    updatePaletteFade();
    randomAmbientSfx();
}
