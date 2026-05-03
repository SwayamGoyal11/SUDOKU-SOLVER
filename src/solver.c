/*
 * solver.c - Sudoku Solver Implementations
 *
 * Implements two solving strategies:
 *
 * ============================================================================
 * 1. BACKTRACKING (Plain Recursive)
 * ============================================================================
 * 
 * Backtracking is a systematic brute-force approach that explores all possible
 * number placements. It works by:
 *   - Finding the first empty cell (scanning left-to-right, top-to-bottom)
 *   - Trying each number 1-9 in that cell
 *   - For each valid placement, recursing to solve the rest
 *   - If the recursion fails, removing the number (backtracking) and trying next
 *   - If no number works, returning failure (backtrack to previous cell)
 *
 * Recursion Flow:
 *   solve(grid)
 *     └─ find empty cell (r, c)
 *     └─ try num = 1: isSafe? yes → place → solve(grid)
 *     │                                      └─ ... (deeper recursion)
 *     │                                      └─ FAIL → backtrack
 *     └─ try num = 2: isSafe? no → skip
 *     └─ try num = 3: isSafe? yes → place → solve(grid)
 *     │                                      └─ SUCCESS!
 *     └─ ...
 *
 * Time Complexity: O(9^n) where n = number of empty cells (worst case)
 * Space Complexity: O(n) for recursion stack
 *
 * ============================================================================
 * 2. BRANCH AND BOUND (Optimized with Pruning)
 * ============================================================================
 *
 * Branch and Bound improves backtracking using intelligent pruning:
 *
 * Optimization 1 - MRV (Minimum Remaining Values) Heuristic:
 *   Instead of picking the FIRST empty cell, pick the cell with the FEWEST
 *   valid candidates. This causes failures to occur sooner, pruning large
 *   subtrees of the search space.
 *   
 *   Example: If cell A has candidates {3,7,9} and cell B has {5}, we pick B
 *   first. If 5 doesn't work, we know immediately instead of exploring A first.
 *
 * Optimization 2 - Forward Checking:
 *   After placing a number, we check ALL remaining empty cells to see if any
 *   has ZERO valid candidates. If so, the current placement leads to a dead
 *   end, so we prune the entire branch WITHOUT recursing deeper.
 *
 *   This catches "obvious" failures early:
 *     - Place 5 in cell (2,3)
 *     - Cell (2,7) now has no valid numbers → PRUNE immediately
 *     - Without forward checking, we'd recurse many more levels before failing
 *
 * Optimization 3 - Candidate Pre-computation:
 *   Only iterate over VALID candidates (not all 1-9). Combined with MRV,
 *   this significantly reduces the branching factor.
 *
 * The "bound" in Branch and Bound is the constraint that every empty cell
 * must have at least one valid candidate. If this bound is violated at any
 * point, we prune the branch.
 *
 * Typical Performance: 50-90% fewer recursive calls than plain backtracking.
 */

#include "solver.h"
#include <string.h>
#include <time.h>

/* Use high-resolution timer if available */
#ifdef _WIN32
#include <windows.h>
static double getTimeMs(void) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart * 1000.0;
}
#else
#include <sys/time.h>
static double getTimeMs(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}
#endif

/* ====================== Helper: Record a Step ====================== */

/*
 * recordStep - Add a solve step to the result's step array.
 * Steps are capped at MAX_STEPS to prevent memory issues.
 */
static void recordStep(SolveResult *result, int row, int col, int value, StepType type) {
    if (result->stepCount < MAX_STEPS) {
        result->steps[result->stepCount].row   = row;
        result->steps[result->stepCount].col   = col;
        result->steps[result->stepCount].value = value;
        result->steps[result->stepCount].type  = type;
        result->stepCount++;
    }
}

/* =======================================================================
 *  BACKTRACKING SOLVER
 * ======================================================================= */

/*
 * backtrackSolve - Recursive backtracking solver (internal).
 *
 * This function implements the classic backtracking algorithm:
 *
 *   1. FIND EMPTY CELL: Scan the grid left-to-right, top-to-bottom for the
 *      first cell that contains EMPTY_CELL (0). If none found, the puzzle
 *      is solved (base case).
 *
 *   2. TRY EACH NUMBER: For numbers 1 through 9:
 *      a. Check if the number is SAFE (doesn't violate row/col/box constraints)
 *      b. If safe, PLACE the number and record a STEP_PLACE step
 *      c. RECURSE to solve the rest of the puzzle
 *      d. If recursion succeeds, propagate success upward
 *      e. If recursion fails, REMOVE the number (backtrack) and record STEP_REMOVE
 *
 *   3. RETURN FAILURE: If no number 1-9 works in this cell, return false.
 *      This triggers backtracking in the calling function.
 *
 * Parameters:
 *   grid   - The current state of the puzzle (modified in place)
 *   result - Accumulates steps and statistics
 *
 * Returns: true if solved, false if stuck (need to backtrack)
 */
static bool backtrackSolve(int grid[GRID_SIZE][GRID_SIZE], SolveResult *result) {
    /* Count this recursive call */
    result->recursiveCalls++;

    /* Step 1: Find the first empty cell */
    int row = -1, col = -1;
    for (int r = 0; r < GRID_SIZE && row == -1; r++) {
        for (int c = 0; c < GRID_SIZE && row == -1; c++) {
            if (grid[r][c] == EMPTY_CELL) {
                row = r;
                col = c;
            }
        }
    }

    /* Base case: No empty cell found → puzzle is solved! */
    if (row == -1) {
        return true;
    }

    /* Step 2: Try each number 1-9 */
    for (int num = 1; num <= 9; num++) {
        /*
         * CONSTRAINT CHECK: Is it safe to place 'num' here?
         * isSafe() checks row, column, and 3x3 box constraints.
         */
        if (isSafe(grid, row, col, num)) {
            /* Place the number */
            grid[row][col] = num;
            recordStep(result, row, col, num, STEP_PLACE);

            /* Recurse: try to solve the rest of the puzzle */
            if (backtrackSolve(grid, result)) {
                return true;  /* Success! Solution found */
            }

            /*
             * BACKTRACK: The placement of 'num' at (row, col) didn't lead
             * to a solution. Remove it and try the next number.
             * This is the "backtracking" step - we undo our choice and
             * explore alternative branches of the search tree.
             */
            grid[row][col] = EMPTY_CELL;
            recordStep(result, row, col, 0, STEP_REMOVE);
        }
    }

    /*
     * FAILURE: No number 1-9 works in this cell.
     * Return false to trigger backtracking in the calling function.
     * The caller will try a different number in its cell.
     */
    return false;
}

/*
 * solveWithBacktracking - Public interface for backtracking solver.
 * Copies the grid, runs the solver, and returns comprehensive results.
 */
SolveResult solveWithBacktracking(int grid[GRID_SIZE][GRID_SIZE]) {
    SolveResult result;
    memset(&result, 0, sizeof(SolveResult));

    /* Work on a copy so we don't modify the original */
    copyGrid(result.grid, grid);

    /* Measure execution time */
    double startTime = getTimeMs();
    result.solved = backtrackSolve(result.grid, &result);
    double endTime = getTimeMs();

    result.executionTimeMs = endTime - startTime;
    return result;
}

/* =======================================================================
 *  BRANCH AND BOUND SOLVER
 * ======================================================================= */

/*
 * getCandidates - Compute valid candidates for a cell.
 *
 * Checks which numbers 1-9 can be legally placed at (row, col)
 * by testing the row, column, and box constraints.
 *
 * Parameters:
 *   grid       - Current grid state
 *   row, col   - Cell to check
 *   candidates - Output array of valid numbers
 *
 * Returns: Number of valid candidates found.
 */
static int getCandidates(int grid[GRID_SIZE][GRID_SIZE], int row, int col,
                         int candidates[9]) {
    int count = 0;
    for (int num = 1; num <= 9; num++) {
        if (isSafe(grid, row, col, num)) {
            candidates[count++] = num;
        }
    }
    return count;
}

/*
 * findMRVCell - Find the empty cell with Minimum Remaining Values.
 *
 * MRV Heuristic (also called "Most Constrained Variable"):
 *   Pick the empty cell that has the FEWEST valid candidates.
 *   This is a key optimization because:
 *     - Cells with fewer options are more likely to fail quickly
 *     - Failing quickly means we prune more of the search tree
 *     - In the best case, a cell with 1 candidate is a forced move
 *
 * Example:
 *   Cell (2,3) has candidates: {5}        → 1 candidate
 *   Cell (4,7) has candidates: {1,3,7,9}  → 4 candidates
 *   Cell (0,1) has candidates: {2,8}      → 2 candidates
 *   MRV picks cell (2,3) because it's most constrained.
 *
 * Parameters:
 *   grid       - Current grid state
 *   outRow/Col - Output: position of the MRV cell
 *
 * Returns: true if an empty cell was found, false if grid is complete.
 */
static bool findMRVCell(int grid[GRID_SIZE][GRID_SIZE], int *outRow, int *outCol) {
    int minCandidates = 10;  /* Start with impossibly high value */
    *outRow = -1;
    *outCol = -1;

    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (grid[r][c] == EMPTY_CELL) {
                int candidates[9];
                int count = getCandidates(grid, r, c, candidates);

                if (count < minCandidates) {
                    minCandidates = count;
                    *outRow = r;
                    *outCol = c;

                    /* Optimization: if only 1 candidate, can't do better */
                    if (count == 1) return true;
                }
            }
        }
    }

    return (*outRow != -1);
}

/*
 * forwardCheck - Verify that no empty cell has zero valid candidates.
 *
 * Forward Checking is a lookahead technique:
 *   After placing a number, we check ALL remaining empty cells.
 *   If ANY cell has zero valid candidates, the current state is a dead end.
 *   We can prune this entire branch WITHOUT recursing further.
 *
 * Why this works:
 *   Without forward checking, we'd continue filling cells until we reach
 *   the problematic cell, potentially making many more recursive calls.
 *   Forward checking detects the dead end immediately.
 *
 * Example:
 *   We place 7 at (3,5). This removes 7 from the candidates of:
 *     - All cells in row 3
 *     - All cells in column 5
 *     - All cells in box (3,3)-(5,5)
 *   If cell (3,8) previously had candidates {7}, it now has NONE.
 *   Forward checking catches this → prune immediately.
 *
 * Returns: true if all empty cells have at least 1 candidate, false otherwise.
 */
static bool forwardCheck(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (grid[r][c] == EMPTY_CELL) {
                /* Check if this cell has any valid candidates */
                bool hasCandidate = false;
                for (int num = 1; num <= 9 && !hasCandidate; num++) {
                    if (isSafe(grid, r, c, num)) {
                        hasCandidate = true;
                    }
                }
                if (!hasCandidate) {
                    return false;  /* Dead end: this cell has NO valid options */
                }
            }
        }
    }
    return true;  /* All cells have at least one option */
}

/*
 * branchAndBoundSolve - Recursive B&B solver (internal).
 *
 * This function combines three optimizations over plain backtracking:
 *
 *   1. MRV CELL SELECTION: Pick the most constrained empty cell (fewest
 *      valid candidates). This makes failures happen sooner → more pruning.
 *
 *   2. BOUNDING CONDITION: If the selected cell has ZERO candidates,
 *      prune immediately. No need to try anything.
 *
 *   3. FORWARD CHECKING: After placing a number, check all remaining empty
 *      cells. If any has zero candidates, prune this branch immediately
 *      instead of recursing deeper into a guaranteed dead end.
 *
 * The search tree visualization:
 *
 *   branchAndBoundSolve(grid)
 *     └─ MRV selects cell (2,3) with candidates {5, 8}
 *        ├─ try 5: place → forwardCheck → PASS → recurse
 *        │  └─ MRV selects cell (4,1) with candidates {3}
 *        │     └─ try 3: place → forwardCheck → FAIL → PRUNE ✂️
 *        │        (cell (4,5) has 0 candidates → dead end detected early!)
 *        │     └─ backtrack from (4,1)
 *        │  └─ backtrack from (2,3)
 *        └─ try 8: place → forwardCheck → PASS → recurse
 *           └─ ... (continues search)
 *
 * Parameters:
 *   grid   - Current grid state (modified in place)
 *   result - Accumulates steps and statistics
 *
 * Returns: true if solved, false if dead end (need to backtrack)
 */
static bool branchAndBoundSolve(int grid[GRID_SIZE][GRID_SIZE], SolveResult *result) {
    /* Count this recursive call */
    result->recursiveCalls++;

    /* Step 1: Find the empty cell with minimum remaining values (MRV) */
    int row, col;
    if (!findMRVCell(grid, &row, &col)) {
        return true;  /* No empty cells → puzzle is solved! */
    }

    /* Step 2: Get valid candidates for the chosen cell */
    int candidates[9];
    int candidateCount = getCandidates(grid, row, col, candidates);

    /*
     * BOUNDING CONDITION: If this cell has ZERO candidates, prune immediately.
     * This means the current state of the grid is invalid - no number can be
     * placed here without violating Sudoku rules. There's no point exploring
     * further down this branch.
     */
    if (candidateCount == 0) {
        recordStep(result, row, col, 0, STEP_PRUNE);
        return false;  /* Prune: dead end */
    }

    /* Step 3: Try each candidate (only valid ones, not all 1-9) */
    for (int i = 0; i < candidateCount; i++) {
        int num = candidates[i];

        /* Place the candidate */
        grid[row][col] = num;
        recordStep(result, row, col, num, STEP_PLACE);

        /*
         * FORWARD CHECKING: After placing 'num', check if any other empty
         * cell now has zero valid candidates. If so, this placement leads
         * to a dead end → prune without recursing deeper.
         *
         * This is the key difference from plain backtracking: we detect
         * failures BEFORE they happen instead of waiting until we reach
         * the problematic cell.
         */
        if (forwardCheck(grid)) {
            /* Forward check passed → recurse to solve remaining cells */
            if (branchAndBoundSolve(grid, result)) {
                return true;  /* Solution found! */
            }
        } else {
            /*
             * PRUNED! Forward checking found a dead end.
             * Record this as a prune step for visualization.
             * Without this optimization, we would have recursed many
             * more levels before discovering the dead end.
             */
            recordStep(result, row, col, 0, STEP_PRUNE);
        }

        /* BACKTRACK: Remove the number and try the next candidate */
        grid[row][col] = EMPTY_CELL;
        recordStep(result, row, col, 0, STEP_REMOVE);
    }

    /* No candidate worked → backtrack to previous cell */
    return false;
}

/*
 * solveWithBranchAndBound - Public interface for B&B solver.
 * Copies the grid, runs the optimized solver, and returns results.
 */
SolveResult solveWithBranchAndBound(int grid[GRID_SIZE][GRID_SIZE]) {
    SolveResult result;
    memset(&result, 0, sizeof(SolveResult));

    /* Work on a copy so we don't modify the original */
    copyGrid(result.grid, grid);

    /* Measure execution time */
    double startTime = getTimeMs();
    result.solved = branchAndBoundSolve(result.grid, &result);
    double endTime = getTimeMs();

    result.executionTimeMs = endTime - startTime;
    return result;
}
