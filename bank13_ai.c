// CPU opponent AI for Game A: three AI types, each a state machine on g_aiState.
// The AI keeps a hit list at g_aiHitList: g_aiHitCount entries of 2 bytes
// (state, packed cell). A packed cell is row << 4 | column. Entry states:
//   1 a single hit, 2 two hits, 3 a line of hits to follow, 4 a hit to extend,
//   7 a line with both ends tried, 8 done (its ship is sunk).
// Bank 5 calls the entry points through FAR_CALL_C.
#include "game.h"

// bank5_ai.c
uint8_t random(void) __banked;
void aiNextPatternCell(void) __banked;
uint8_t isLastHitOnShip(uint8_t cell) __banked;
uint8_t countOpenNeighbors(uint8_t row, uint8_t col, uint8_t n) __banked;

#define HIT_LIST (&g_aiHitList)
#define SHOT_GRID (&g_cpuShotGrid)
#define FLEET_GRID (&g_playerFleetGrid)
#define KNOWN_CELLS (&g_aiKnownCells)
#define CELL_LIST (&g_vramQueue) // scratch list of packed cells
#define CPU_SHOOT 0x06 // g_gameACpuState: the target cell is chosen
#define EMPTY_SHIP_ID 0x12

typedef struct {
    uint8_t row, col;
} Cell;

static void unpackCell(Cell *c, uint8_t packed)
{
    c->row = packed >> 4;
    c->col = packed & 0x0f;
}

static uint8_t packCell(const Cell *c)
{
    return SWAP(c->row) | c->col;
}

// The byte of grid at cell c.
static uint8_t gridAt(const uint8_t *grid, const Cell *c)
{
    return grid[multiply8(c->row, g_gridSize) + c->col];
}

static void setTarget(const Cell *c)
{
    g_targetRow = c->row;
    g_targetCol = c->col;
}

// Move c one cell in direction dir (0 up, 1 down, 2 left, 3 right).
// Returns 1 when the new cell is outside the grid.
static uint8_t stepInDirection(Cell *c, uint8_t dir)
{
    if (dir == 0)
        return c->row-- == 0;
    if (dir == 1)
        return ++c->row >= g_gridSize;
    if (dir == 2)
        return c->col-- == 0;
    return ++c->col >= g_gridSize;
}

// Search the hit list from p (2-byte entries, g_aiHitCount of them) for v.
// Returns the match, or 0 when there is none.
static uint8_t *findEntry(uint8_t *p, uint8_t v)
{
    uint8_t n = g_aiHitCount;
    do {
        if (*p == v)
            return p;
        p += 2;
    } while (--n);
    return 0;
}

// The same, but with no match it returns the byte after the searched entries.
// Some callers do not check for a match and write there, like the asm.
static uint8_t *findEntryOrEnd(uint8_t *p, uint8_t v)
{
    uint8_t *e = findEntry(p, v);
    if (e)
        return e;
    return p + (g_aiHitCount ? (uint16_t)g_aiHitCount << 1 : 0x200);
}

// Add the target cell to the end of the hit list. Returns the new entry.
// Its state byte is not set.
static uint8_t *addTargetToHitList(void)
{
    uint8_t *e = HIT_LIST + (uint8_t)(g_aiHitCount << 1);
    e[1] = SWAP(g_targetRow) | g_targetCol;
    g_aiHitCount++;
    return e;
}

// Set the AI direction to 0 or 2 at random and clear the try count.
static void aiRandomDirection(void)
{
    g_aiDirection = (random() & 0x01) << 1;
    g_aiTryCount = 0;
}

// Turn to the next of the 4 directions. Returns 1 after the 4th try.
static uint8_t turnToNextDirection(void)
{
    g_aiDirection = (g_aiDirection + 1) & 0x03;
    return ++g_aiTryCount == 4;
}

// Choose the next AI state after a shot: 3 or 4 (or 1 or 2) if the hit list has
// an open entry, 5 if known cells remain, else 0 (hunt).
void aiPickNextState(void)
{
    uint8_t state = 0;
    // The easy AI (commander 0) hunts while it has fewer hits than ships afloat.
    if ((g_p2Commander || g_aiHitsScored >= g_shipsAfloat) && g_aiHitCount) {
        state = g_gameFlags9 & 0x04 ? 4 : 3;
        while (state && !findEntry(HIT_LIST, state))
            state--;
    }
    if (!state && g_aiKnownCount)
        state = 5;
    g_aiState = state;
    aiRandomDirection();
}

// If only one hit point of the player fleet is left, mark every AI list entry as
// state 4. With no hit points left and g_revealFlag set, mark only the entries on
// cells with ship ids $13-$17. Either way, go to state 4.
static void aiCheckShipsLeft(void)
{
    uint8_t left = 0;
    for (uint8_t i = 0; i < 5; i++)
        left += g_shipHitsLeft[i];
    uint8_t onlyShipIds = 0;
    if (left != 1) {
        if (left || !g_revealFlag)
            return;
        onlyShipIds = 1;
    }
    uint8_t *e = HIT_LIST;
    uint8_t n = g_aiHitCount;
    do {
        if (onlyShipIds) {
            Cell c;
            unpackCell(&c, e[1]);
            uint8_t id = gridAt(FLEET_GRID, &c) & 0x3f;
            if (id >= 0x13 && id < 0x18)
                e[0] = 4;
        } else {
            e[0] = 4;
        }
        e += 2;
    } while (--n);
    g_aiState = 4;
}

// Pick a random cell that holds a ship, then build a diagonal path in
// g_aiRunCells that ends on it. The AI walks this path in hunt mode.
static void aiBuildPathToShip(void)
{
    const uint8_t *fleet = FLEET_GRID;
    uint8_t n = 0;
    for (uint8_t row = 0; row != g_gridSize; row++) {
        for (uint8_t col = 0; col != g_gridSize; col++) {
            uint8_t id = *fleet++ & 0x3f;
            if (id && id < EMPTY_SHIP_ID)
                CELL_LIST[n++] = SWAP(row) | col;
        }
    }
    uint8_t pick = random() & 0x1f;
    while (pick >= n)
        pick -= n;
    Cell c;
    unpackCell(&c, CELL_LIST[pick]);
    // Start len - 1 cells away on a diagonal that stays in the grid.
    uint8_t len = ((g_gridSize - 8) >> 1) + 2;
    int8_t rowStep = 1, colStep = 1;
    uint8_t rowOffset = len, colOffset = len;
    if (len < c.row) {
        rowOffset = -len;
    } else {
        rowStep = -1;
    }
    if (len < c.col) {
        colOffset = -len;
    } else {
        colStep = -1;
    }
    c.row += rowOffset;
    c.col += colOffset;
    g_aiRunCellsLeft = len + 2;
    uint8_t *cell = &g_aiRunCells;
    n = len + 1;
    do {
        *cell++ = c.row;
        *cell++ = c.col;
        c.row += rowStep;
        c.col += colStep;
    } while (--n);
    g_aiRunCellIndex = 0;
}

// Shoot the target cell if the CPU has not shot it yet.
static void shootTargetIfNew(void)
{
    Cell c = { g_targetRow, g_targetCol };
    if (!gridAt(SHOT_GRID, &c))
        g_gameACpuState = CPU_SHOOT;
}

// AI state 0 (hunt), types 0 and 1: take the next cell from the search pattern.
// When the pattern is used up, mark every hit-list entry as state 4 and go to state 4.
static void aiState0_Hunt(void)
{
    aiCheckShipsLeft();
    if (g_aiState == 4) {
        aiRandomDirection();
        return;
    }
    aiNextPatternCell();
    if (g_gameFlags9 & 0x08) {
        uint8_t *e = HIT_LIST;
        uint8_t n = g_aiHitCount;
        do {
            *e = 4;
            e += 2;
        } while (--n);
        g_aiState = 4;
    }
    shootTargetIfNew();
}

// AI state 0 (hunt), type 2: the first time (g_gameFlags9 bit 4 clear) build a
// path that leads to a ship cell. Then take the next pattern cell.
static void aiState0_HuntToShip(void)
{
    aiCheckShipsLeft();
    if (g_aiState == 4) {
        aiRandomDirection();
        return;
    }
    if (!(g_gameFlags9 & 0x10)) {
        g_gameFlags9 |= 0x10;
        aiBuildPathToShip();
    }
    aiNextPatternCell();
    shootTargetIfNew();
}

// State 1 tried all 4 directions of the hit in g_aiHitCell: mark it state 4.
static void aiState1_HitDone(void)
{
    findEntryOrEnd(HIT_LIST + 1, g_aiHitCell)[-1] = 4;
    if (!(g_gameFlags9 & 0x04)) {
        if (findEntry(HIT_LIST, 1)) {
            aiRandomDirection();
            return;
        }
        if (!(g_gameFlags9 & 0x08)) {
            g_aiState = 0;
            return;
        }
    }
    g_aiState = 4;
    aiRandomDirection();
}

// AI state 1: probe around a single hit, one direction at a time.
// The probe is 2 cells away from the hit (it skips the cell next to it).
static void aiState1_ProbeAroundHit(void)
{
    Cell c;
    uint8_t *e = findEntry(HIT_LIST, 1);
    if (!e) {
        g_aiState = 2;
        return;
    }
    g_aiHitCell = e[1];
    unpackCell(&c, g_aiHitCell);
    if (stepInDirection(&c, g_aiDirection) || gridAt(SHOT_GRID, &c)
        || stepInDirection(&c, g_aiDirection))
        goto nextDirection;
    if (gridAt(SHOT_GRID, &c)) {
        // Already shot. The easy AI joins it with this hit when it is a
        // single hit (state 1) on a ship.
        if (g_p2Commander)
            goto nextDirection;
        uint8_t id = gridAt(FLEET_GRID, &c) & 0x3f;
        if (!id || id == EMPTY_SHIP_ID)
            goto nextDirection;
        e = findEntryOrEnd(HIT_LIST + 1, packCell(&c)) - 1;
        if (*e != 1)
            goto nextDirection;
    } else {
        g_gameACpuState = CPU_SHOOT;
        setTarget(&c);
        uint8_t v = gridAt(FLEET_GRID, &c);
        if (!v || v == EMPTY_SHIP_ID)
            goto nextDirection;
        if (g_gameFlags4 & 0x08) {
            findEntryOrEnd(HIT_LIST + 1, g_aiHitCell)[-1] = 8;
            aiRandomDirection();
            aiPickNextState();
            return;
        }
        uint8_t sunk = isLastHitOnShip(v);
        e = addTargetToHitList();
        if (sunk)
            goto nextDirection;
    }
    // Two hits: both become state 2.
    *e = 2;
    findEntryOrEnd(HIT_LIST + 1, g_aiHitCell)[-1] = 2;
    g_aiState = 2;
    return;

nextDirection:
    if (turnToNextDirection())
        aiState1_HitDone();
}

// Set the states of the next two entries with state from to state to.
static void setTwoEntries(uint8_t from, uint8_t to)
{
    uint8_t *e = findEntryOrEnd(HIT_LIST, from);
    *e = to;
    *findEntryOrEnd(e, from) = to;
}

// Half of the 9-bit difference a - b (the borrow is the sign), plus b.
static uint8_t midpoint(uint8_t a, uint8_t b)
{
    return b + (uint8_t)(((uint16_t)a - b) >> 1);
}

// AI state 2: two hits found; shoot the cell between them.
static void aiState2_ProbeBetweenHits(void)
{
    Cell a, b;
    uint8_t *e = findEntryOrEnd(HIT_LIST, 2);
    unpackCell(&a, e[1]);
    e = findEntryOrEnd(e + 2, 2);
    unpackCell(&b, e[1]);
    if (a.row != b.row)
        b.row = midpoint(a.row, b.row);
    else
        b.col = midpoint(a.col, b.col);
    if (gridAt(SHOT_GRID, &b)) {
        setTwoEntries(2, 3);
        g_aiState = 3;
        aiRandomDirection();
        return;
    }
    setTarget(&b);
    g_gameACpuState = CPU_SHOOT;
    uint8_t v = gridAt(FLEET_GRID, &b);
    if (!v || v == EMPTY_SHIP_ID) {
        setTwoEntries(2, 1);
        g_aiState = 1;
        aiRandomDirection();
        return;
    }
    uint8_t sunk = isLastHitOnShip(v);
    e = addTargetToHitList();
    if (sunk)
        return;
    *e = 3;
    setTwoEntries(2, 3);
    g_aiDirection &= 0xfe;
    g_aiTryCount = 0;
    g_aiState = 3;
    g_aiLinesDone = 0;
}

// AI state 3: a line of hits found; keep shooting along the line.
static void aiState3_FollowLine(void)
{
    Cell c;
    uint8_t *e = findEntry(HIT_LIST, 3);
    if (!e) {
        // No line left: the lines with both ends tried (7) become state 4.
        e = HIT_LIST;
        while ((e = findEntry(e, 7)))
            *e = 4;
        g_aiState = 4;
        aiRandomDirection();
        return;
    }
    g_aiHitCell = e[1];
    unpackCell(&c, g_aiHitCell);
    if (!stepInDirection(&c, g_aiDirection)) {
        uint8_t isShip = 0;
        uint8_t id = gridAt(FLEET_GRID, &c) & 0x3f;
        if (id) {
            isShip = 1;
            g_shipCellId = id;
        }
        if (gridAt(SHOT_GRID, &c)) {
            // A hit already in the list joins the line, unless its line is done.
            if (isShip) {
                e = findEntry(HIT_LIST + 1, packCell(&c));
                if (e && e[-1] != 7)
                    e[-1] = 3;
            }
        } else {
            g_gameACpuState = CPU_SHOOT;
            setTarget(&c);
            if (isShip && g_shipCellId != EMPTY_SHIP_ID) {
                uint8_t sunk = isLastHitOnShip(g_shipCellId);
                e = addTargetToHitList();
                if (sunk)
                    return;
                *e = 3;
            }
        }
    }
    // Turn around. After both ends, this line is done.
    g_aiDirection ^= 0x01;
    if (++g_aiTryCount != 2)
        return;
    g_aiTryCount = 0;
    findEntryOrEnd(HIT_LIST + 1, g_aiHitCell)[-1] = 7;
    g_aiLinesDone++;
}

// AI state 4: try to extend from a hit marked state 4.
static void aiState4_ExtendLine(void)
{
    Cell c;
    uint8_t *e = findEntry(HIT_LIST, 4);
    if (!e) {
        aiPickNextState();
        return;
    }
    unpackCell(&c, e[1]);
    if (!stepInDirection(&c, g_aiDirection) && !gridAt(SHOT_GRID, &c)) {
        g_gameACpuState = CPU_SHOOT;
        setTarget(&c);
        uint8_t id = gridAt(FLEET_GRID, &c) & 0x3f;
        if (id && id != EMPTY_SHIP_ID) {
            uint8_t sunk = isLastHitOnShip(id);
            e = addTargetToHitList();
            if (sunk)
                return;
            // A new line: follow it.
            *e = 3;
            g_aiDirection &= 0x02;
            g_aiTryCount = 0;
            *findEntryOrEnd(HIT_LIST, 4) = 3;
            g_aiState = 3;
            return;
        }
    }
    if (!turnToNextDirection())
        return;
    aiRandomDirection();
    *findEntryOrEnd(HIT_LIST, 4) = 8;
}

// AI state 5: shoot the cells around the first cell of the known-cell list.
static void aiState5_KnownCells(void)
{
    Cell c;
    uint8_t *known = KNOWN_CELLS;
    if (known[0] != 0xff) {
        unpackCell(&c, known[0]);
        uint8_t hit = 0;
        if (!stepInDirection(&c, g_aiDirection) && !gridAt(SHOT_GRID, &c)) {
            g_gameACpuState = CPU_SHOOT;
            setTarget(&c);
            uint8_t id = gridAt(FLEET_GRID, &c) & 0x3f;
            hit = id && id != EMPTY_SHIP_ID;
        }
        if (!hit && !turnToNextDirection())
            return;
    }
    // Drop the first known cell and hunt again.
    for (uint8_t i = 0; i < 15; i++)
        known[i] = known[i + 1];
    g_aiKnownCount--;
    g_aiState = 0;
}

// Shoot an unshot cell next to any unfinished hit (state below 7) in the AI list.
// g_gameATurnTimer counts the directions.
static void aiShootNextToHit(void)
{
    g_aiState = g_aiSavedState;
    uint8_t n = g_aiHitCount;
    if (!n)
        return;
    uint8_t *e = HIT_LIST;
    do {
        if (e[0] < 7) {
            for (g_gameATurnTimer = 0; g_gameATurnTimer != 4; g_gameATurnTimer++) {
                Cell c;
                unpackCell(&c, e[1]);
                if (!stepInDirection(&c, g_gameATurnTimer) && !gridAt(SHOT_GRID, &c)) {
                    setTarget(&c);
                    g_gameACpuState = CPU_SHOOT;
                    return;
                }
            }
        }
        e += 2;
    } while (--n);
}

// AI state 6: shoot a random cell from those with the most unshot cells
// around them (9 or 5 open cells, by grid size).
static void aiState6_BestOpenCell(void)
{
    if (g_gameFlags4 & 0x08) {
        aiShootNextToHit();
        return;
    }
    g_aiState = g_aiSavedState;
    uint8_t want = (g_p1Options & 0x03) == 2 ? 9 : 5;
    uint8_t n = 0;
    for (uint8_t row = 0; row != g_gridSize; row++) {
        for (uint8_t col = 0; col != g_gridSize; col++) {
            if (countOpenNeighbors(row, col, want) == want)
                CELL_LIST[n++] = SWAP(row) | col;
        }
    }
    if (!n)
        return;
    uint8_t pick = random();
    while (pick >= n)
        pick -= n;
    Cell c;
    unpackCell(&c, CELL_LIST[pick]);
    setTarget(&c);
    g_gameACpuState = CPU_SHOOT;
}

// Run the AI state in g_aiState. hunt is the state 0 of the AI type.
static void runAiState(void (*hunt)(void))
{
    switch (g_aiState) {
    case 0: hunt(); break;
    case 1: aiState1_ProbeAroundHit(); break;
    case 2: aiState2_ProbeBetweenHits(); break;
    case 3: aiState3_FollowLine(); break;
    case 4: aiState4_ExtendLine(); break;
    case 5: aiState5_KnownCells(); break;
    case 6: aiState6_BestOpenCell(); break;
    }
}

// g_gameFlags3 bit 4 = a new turn. Clear it and set g_gameFlags9 bit 2.
// Returns 1 on a new turn.
static uint8_t startNewTurn(void)
{
    if (!(g_gameFlags3 & 0x10))
        return 0;
    g_gameFlags3 &= ~0x10;
    g_gameFlags9 |= 0x04;
    return 1;
}

// CPU turn for commander type 0/1/2 (g_p2Commander picks one of these three).
// Runs the AI state to choose a target cell (g_targetRow, g_targetCol).
// g_gameACpuState = 6 means "shot chosen".
void cpuTurnType0(void)
{
    startNewTurn();
    runAiState(aiState0_Hunt);
}

// Types 1 and 2 also pick the next AI state on a new turn when the AI is idle.
void cpuTurnType1(void)
{
    if (startNewTurn() && !g_aiState)
        aiPickNextState();
    runAiState(aiState0_Hunt);
}

void cpuTurnType2(void)
{
    if (startNewTurn() && !g_aiState)
        aiPickNextState();
    runAiState(aiState0_HuntToShip);
}

// Remove the 4 cells next to c from the known-cell list.
static void aiDropKnownCellsAround(const Cell *c)
{
    if (!g_aiKnownCount)
        return;
    for (uint8_t dir = 0; dir != 4; dir++) {
        Cell next;
        next.row = c->row;
        next.col = c->col;
        if (stepInDirection(&next, dir))
            continue;
        uint8_t packed = packCell(&next);
        uint8_t *known = KNOWN_CELLS;
        uint8_t n = g_aiKnownCount;
        do {
            if (*known == packed) {
                *known = 0xff;
                break;
            }
            known++;
        } while (--n);
    }
}

// g_aiSunkLineCell = a cell of a sunk ship. Walk both ways along the line of
// hits and mark those AI list entries as state 8 (done). g_gameATurnTimer counts
// them. When fewer than 2 are marked, try the other axis.
void aiMarkSunkLine(void)
{
    Cell c;
    for (;;) {
        c.row = g_aiSunkLineCell[0];
        c.col = g_aiSunkLineCell[1];
        // Walk up or left to the end of the hits.
        g_aiDirection &= 0x02;
        while (!stepInDirection(&c, g_aiDirection) && gridAt(SHOT_GRID, &c) == 1)
            ;
        // Walk back down or right over the hits.
        g_aiDirection |= 0x01;
        g_gameATurnTimer = 0;
        stepInDirection(&c, g_aiDirection);
        while (gridAt(SHOT_GRID, &c) == 1) {
            aiDropKnownCellsAround(&c);
            uint8_t *e = findEntry(HIT_LIST + 1, packCell(&c));
            if (e) {
                e[-1] = 8;
                g_gameATurnTimer++;
            }
            if (stepInDirection(&c, g_aiDirection))
                break;
        }
        if (g_gameATurnTimer >= 2)
            return;
        g_aiDirection ^= 0x02;
    }
}
