/*
 * solver.h - Sudoku Solver Algorithms (Backtracking & Branch and Bound)
 *
 * This header declares two solving approaches:
 *
 * 1. BACKTRACKING (Brute Force with Constraint Checking)
 *    - Tries each number 1-9 in the first empty cell
 *    - Recursively solves the rest
 *    - If stuck, backtracks and tries the next number
 *    - Time Complexity: O(9^(n)) where n = number of empty cells (worst case)
 *
 * 2. BRANCH AND BOUND (Optimized Backtracking)
 *    - Uses Minimum Remaining Values (MRV) heuristic to pick the most
 *      constrained cell first (fewest valid candidates)
 *    - Forward Checking: after placing a number, checks if any remaining
 *      empty cell has ZERO candidates. If so, prunes immediately.
 *    - Bounding: skips cells/branches that cannot lead to a solution
 *    - Typically reduces recursive calls by 50-90% compared to plain backtracking
 *
 * Both solvers record every step they take (place, remove, prune) for
 * step-by-step visualization in the GUI.
 */

#ifndef SOLVER_H
#define SOLVER_H

#include "sudoku.h"

/* ====================== Step Types for Visualization ====================== */

/*
 * StepType - Describes what happened at each step of the solve.
 *   STEP_PLACE   - A number was placed in a cell (trying a candidate)
 *   STEP_REMOVE  - A number was removed (backtracking)
 *   STEP_PRUNE   - A branch was pruned (B&B detected dead end early)
 */
typedef enum {
    STEP_PLACE,
    STEP_REMOVE,
    STEP_PRUNE
} StepType;

/* ====================== Data Structures ====================== */

/*
 * SolveStep - Records a single action during solving.
 *   row, col - The cell being operated on
 *   value    - The number placed (0 if removing/pruning)
 *   type     - What kind of step this is
 */
typedef struct {
    int      row;
    int      col;
    int      value;
    StepType type;
} SolveStep;

/*
 * SolveResult - Complete result of a solve operation.
 *   steps[]        - Array of recorded steps for animation
 *   stepCount      - How many steps were recorded
 *   recursiveCalls - Total recursive calls made (for performance comparison)
 *   executionTimeMs - How long the solve took in milliseconds
 *   solved         - Whether the puzzle was successfully solved
 *   grid           - Final state of the grid after solving
 */
typedef struct {
    SolveStep steps[MAX_STEPS];
    int       stepCount;
    int       recursiveCalls;
    double    executionTimeMs;
    bool      solved;
    int       grid[GRID_SIZE][GRID_SIZE];
} SolveResult;

/* ====================== Solver Functions ====================== */

/*
 * solveWithBacktracking - Solve using plain recursive backtracking.
 *
 * Algorithm (pseudocode):
 *   function solve(grid):
 *       find first empty cell (row, col)
 *       if no empty cell: return SUCCESS (puzzle solved!)
 *       for num = 1 to 9:
 *           if isSafe(grid, row, col, num):
 *               place num at (row, col)
 *               if solve(grid): return SUCCESS
 *               remove num from (row, col)  // backtrack
 *       return FAILURE (no valid number works)
 *
 * Parameters:
 *   grid - The puzzle to solve (will not be modified; result has final grid)
 *
 * Returns: SolveResult with steps, stats, and solved grid.
 */
SolveResult solveWithBacktracking(int grid[GRID_SIZE][GRID_SIZE]);

/*
 * solveWithBranchAndBound - Solve using optimized branch and bound.
 *
 * Optimizations over plain backtracking:
 *   1. MRV Heuristic: Always pick the empty cell with the fewest valid
 *      candidates. This fails faster on dead-end branches.
 *   2. Forward Checking: After placing a number, check ALL remaining empty
 *      cells. If ANY cell has zero valid candidates, prune immediately
 *      (don't recurse further - this branch is a dead end).
 *   3. Candidate Computation: Only try numbers that are actually valid for
 *      the chosen cell (already computed by MRV selection).
 *
 * Parameters:
 *   grid - The puzzle to solve (will not be modified; result has final grid)
 *
 * Returns: SolveResult with steps, stats, and solved grid.
 */
SolveResult solveWithBranchAndBound(int grid[GRID_SIZE][GRID_SIZE]);

#endif /* SOLVER_H */
