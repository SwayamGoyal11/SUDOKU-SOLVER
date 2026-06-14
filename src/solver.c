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
 * isGridConsistent - Check if the initial grid does not violate Sudoku constraints.
 * This checks that no row, column, or 3x3 box contains duplicate values.
 */
static bool isGridConsistent(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            int v = grid[r][c];

            if (v == EMPTY_CELL) continue;
            if (v < 1 || v > 9) return false;

            /* Temporarily clear the cell to check safety of its value */
            grid[r][c] = EMPTY_CELL;
            bool ok = isSafe(grid, r, c, v);
            grid[r][c] = v;

            if (!ok) return false;
        }
    }
    return true;
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

    if (!isGridConsistent(result.grid)) {
        result.solved = false;
        return result;
    }

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
 * getCandidateCountFast - Count the remaining valid candidates for a cell
 * by scanning the row, column, and sub-box once. This is much faster
 * than calling isSafe 9 times.
 */
static int getCandidateCountFast(int grid[GRID_SIZE][GRID_SIZE], int row, int col) {
    int mask = 0;

    // Row check
    for (int c = 0; c < GRID_SIZE; c++) {
        mask |= (1 << grid[row][c]);
    }

    // Column check
    for (int r = 0; r < GRID_SIZE; r++) {
        mask |= (1 << grid[r][col]);
    }

    // Box check
    int boxStartRow = (row / BOX_SIZE) * BOX_SIZE;
    int boxStartCol = (col / BOX_SIZE) * BOX_SIZE;
    for (int r = boxStartRow; r < boxStartRow + BOX_SIZE; r++) {
        for (int c = boxStartCol; c < boxStartCol + BOX_SIZE; c++) {
            mask |= (1 << grid[r][c]);
        }
    }

    return 9 - __builtin_popcount(mask & 0x3FE);
}

/*
 * findMRVCell - Find the empty cell with Minimum Remaining Values.
 * Also populates the candidates array and count for that cell to avoid double scan.
 */
static bool findMRVCell(int grid[GRID_SIZE][GRID_SIZE], int *outRow, int *outCol,
                        int outCandidates[9], int *outCount) {
    int minCandidates = 10;  /* Start with impossibly high value */
    *outRow = -1;
    *outCol = -1;
    *outCount = 0;
    int bestMask = 0;

    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (grid[r][c] == EMPTY_CELL) {
                int mask = 0;

                // Row check
                for (int col = 0; col < GRID_SIZE; col++) {
                    mask |= (1 << grid[r][col]);
                }

                // Column check
                for (int row = 0; row < GRID_SIZE; row++) {
                    mask |= (1 << grid[row][c]);
                }

                // Box check
                int boxStartRow = (r / BOX_SIZE) * BOX_SIZE;
                int boxStartCol = (c / BOX_SIZE) * BOX_SIZE;
                for (int br = boxStartRow; br < boxStartRow + BOX_SIZE; br++) {
                    for (int bc = boxStartCol; bc < boxStartCol + BOX_SIZE; bc++) {
                        mask |= (1 << grid[br][bc]);
                    }
                }

                int count = 9 - __builtin_popcount(mask & 0x3FE);

                if (count < minCandidates) {
                    minCandidates = count;
                    *outRow = r;
                    *outCol = c;
                    bestMask = mask;

                    /* Optimization: if 0 or 1 candidate, can't do better */
                    if (count <= 1) goto found;
                }
            }
        }
    }

found:
    if (*outRow != -1) {
        int count = 0;
        for (int num = 1; num <= 9; num++) {
            if (!(bestMask & (1 << num))) {
                outCandidates[count++] = num;
            }
        }
        *outCount = count;
        return true;
    }
    return false;
}

/*
 * forwardCheck - Verify that no empty cell in the same row, column, or sub-box
 * has zero valid candidates after placing a number at (row, col).
 * This is much faster than checking the entire grid.
 */
static bool forwardCheck(int grid[GRID_SIZE][GRID_SIZE], int row, int col) {
    // 1. Check same row
    for (int c = 0; c < GRID_SIZE; c++) {
        if (c != col && grid[row][c] == EMPTY_CELL) {
            if (getCandidateCountFast(grid, row, c) == 0) {
                return false;
            }
        }
    }

    // 2. Check same column
    for (int r = 0; r < GRID_SIZE; r++) {
        if (r != row && grid[r][col] == EMPTY_CELL) {
            if (getCandidateCountFast(grid, r, col) == 0) {
                return false;
            }
        }
    }

    // 3. Check same box
    int boxStartRow = (row / BOX_SIZE) * BOX_SIZE;
    int boxStartCol = (col / BOX_SIZE) * BOX_SIZE;
    for (int r = boxStartRow; r < boxStartRow + BOX_SIZE; r++) {
        for (int c = boxStartCol; c < boxStartCol + BOX_SIZE; c++) {
            if ((r != row || c != col) && grid[r][c] == EMPTY_CELL) {
                if (getCandidateCountFast(grid, r, c) == 0) {
                    return false;
                }
            }
        }
    }

    return true;  /* All affected empty cells have at least one option */
}

/*
 * branchAndBoundSolve - Recursive B&B solver (internal).
 * Optimized with MRV, fast candidate checks, and localized forward checking.
 */
static bool branchAndBoundSolve(int grid[GRID_SIZE][GRID_SIZE], SolveResult *result) {
    /* Count this recursive call */
    result->recursiveCalls++;

    /* Step 1: Find the empty cell with MRV and get its candidates */
    int row, col, candidateCount;
    int candidates[9];
    if (!findMRVCell(grid, &row, &col, candidates, &candidateCount)) {
        return true;  /* No empty cells → puzzle is solved! */
    }

    /*
     * BOUNDING CONDITION: If this cell has ZERO candidates, prune immediately.
     */
    if (candidateCount == 0) {
        recordStep(result, row, col, 0, STEP_PRUNE);
        return false;  /* Prune: dead end */
    }

    /* Step 3: Try each candidate (only valid ones) */
    for (int i = 0; i < candidateCount; i++) {
        int num = candidates[i];

        /* Place the candidate */
        grid[row][col] = num;
        recordStep(result, row, col, num, STEP_PLACE);

        /*
         * FORWARD CHECKING: Check if placing 'num' makes any other empty cell
         * in the same row/col/box have zero candidates.
         */
        if (forwardCheck(grid, row, col)) {
            /* Forward check passed → recurse to solve remaining cells */
            if (branchAndBoundSolve(grid, result)) {
                return true;  /* Solution found! */
            }
        } else {
            /* PRUNED! Forward checking found a dead end. */
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

    if (!isGridConsistent(result.grid)) {
        result.solved = false;
        return result;
    }

    /* Measure execution time */
    double startTime = getTimeMs();
    result.solved = branchAndBoundSolve(result.grid, &result);
    double endTime = getTimeMs();

    result.executionTimeMs = endTime - startTime;
    return result;
}
