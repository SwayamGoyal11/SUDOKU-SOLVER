/*
 * sudoku.c - Sudoku Puzzle Core Logic
 *
 * Implements puzzle generation, validation, and grid manipulation.
 *
 * Puzzle Generation Algorithm:
 *   1. Start with an empty 9x9 grid.
 *   2. Fill the three diagonal 3x3 boxes with random valid numbers.
 *      (These boxes don't constrain each other, so they can be filled independently.)
 *   3. Use backtracking to fill the remaining cells, producing a complete valid grid.
 *   4. Store this as the "solution".
 *   5. Copy the solution to the puzzle grid.
 *   6. Remove cells based on difficulty level to create the puzzle.
 */

#include "sudoku.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ====================== Helper Functions ====================== */

/*
 * shuffleArray - Fisher-Yates shuffle for randomizing number arrays.
 * Used during puzzle generation to create varied puzzles.
 */
static void shuffleArray(int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

/* ====================== Core Functions ====================== */

/*
 * initPuzzle - Set all cells to empty and reset all flags.
 */
void initPuzzle(SudokuPuzzle *p) {
    memset(p->grid, 0, sizeof(p->grid));
    memset(p->puzzle, 0, sizeof(p->puzzle));
    memset(p->solution, 0, sizeof(p->solution));
    memset(p->fixed, 0, sizeof(p->fixed));
    p->generated = false;
}

/*
 * copyGrid - Copy a 9x9 grid from src to dest.
 */
void copyGrid(int dest[GRID_SIZE][GRID_SIZE], int src[GRID_SIZE][GRID_SIZE]) {
    memcpy(dest, src, sizeof(int) * GRID_SIZE * GRID_SIZE);
}

/*
 * isSafe - The fundamental constraint check for Sudoku.
 *
 * Checks three constraints for placing 'num' at position (row, col):
 *
 *   1. ROW CHECK: No other cell in the same row contains 'num'.
 *      We scan all 9 columns in the given row.
 *
 *   2. COLUMN CHECK: No other cell in the same column contains 'num'.
 *      We scan all 9 rows in the given column.
 *
 *   3. BOX CHECK: No other cell in the same 3x3 sub-box contains 'num'.
 *      We calculate the top-left corner of the box using integer division:
 *        boxStartRow = (row / 3) * 3
 *        boxStartCol = (col / 3) * 3
 *      Then scan all 9 cells in that box.
 *
 * Time Complexity: O(1) - always checks at most 9+9+9 = 27 cells.
 *
 * Returns: true if it's safe to place 'num' at (row, col), false otherwise.
 */
bool isSafe(int grid[GRID_SIZE][GRID_SIZE], int row, int col, int num) {
    /* 1. ROW CHECK: Scan entire row for 'num' */
    for (int c = 0; c < GRID_SIZE; c++) {
        if (grid[row][c] == num) {
            return false;  /* 'num' already exists in this row */
        }
    }

    /* 2. COLUMN CHECK: Scan entire column for 'num' */
    for (int r = 0; r < GRID_SIZE; r++) {
        if (grid[r][col] == num) {
            return false;  /* 'num' already exists in this column */
        }
    }

    /* 3. BOX CHECK: Scan the 3x3 sub-box containing (row, col) */
    int boxStartRow = (row / BOX_SIZE) * BOX_SIZE;
    int boxStartCol = (col / BOX_SIZE) * BOX_SIZE;
    for (int r = boxStartRow; r < boxStartRow + BOX_SIZE; r++) {
        for (int c = boxStartCol; c < boxStartCol + BOX_SIZE; c++) {
            if (grid[r][c] == num) {
                return false;  /* 'num' already exists in this box */
            }
        }
    }

    return true;  /* All three checks passed - placement is valid */
}

/*
 * isValidEntry - Check if a user-entered number is valid.
 * Similar to isSafe but skips checking the cell itself (for cells that
 * already have a value being validated).
 */
bool isValidEntry(int grid[GRID_SIZE][GRID_SIZE], int row, int col, int num) {
    /* Row check (skip self) */
    for (int c = 0; c < GRID_SIZE; c++) {
        if (c != col && grid[row][c] == num) return false;
    }
    /* Column check (skip self) */
    for (int r = 0; r < GRID_SIZE; r++) {
        if (r != row && grid[r][col] == num) return false;
    }
    /* Box check (skip self) */
    int br = (row / BOX_SIZE) * BOX_SIZE;
    int bc = (col / BOX_SIZE) * BOX_SIZE;
    for (int r = br; r < br + BOX_SIZE; r++) {
        for (int c = bc; c < bc + BOX_SIZE; c++) {
            if ((r != row || c != col) && grid[r][c] == num) return false;
        }
    }
    return true;
}

/*
 * countBlanks - Return number of cells to remove based on difficulty.
 */
int countBlanks(Difficulty diff) {
    switch (diff) {
        case DIFFICULTY_EASY:   return 30;
        case DIFFICULTY_MEDIUM: return 40;
        case DIFFICULTY_HARD:   return 50;
        default:                return 30;
    }
}

/* ====================== Puzzle Generation ====================== */

/*
 * fillBox - Fill a single 3x3 box starting at (startRow, startCol)
 * with random numbers 1-9 without repeats.
 *
 * Since each box is independent of the others when they're on the
 * diagonal, we can simply shuffle 1-9 and fill sequentially.
 */
static void fillBox(int grid[GRID_SIZE][GRID_SIZE], int startRow, int startCol) {
    int nums[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    shuffleArray(nums, 9);
    int idx = 0;
    for (int r = startRow; r < startRow + BOX_SIZE; r++) {
        for (int c = startCol; c < startCol + BOX_SIZE; c++) {
            grid[r][c] = nums[idx++];
        }
    }
}

/*
 * solveFull - Solve a partially filled grid using backtracking.
 * Used internally for puzzle generation (not the user-facing solver).
 *
 * This is a standard backtracking approach:
 *   1. Find the first empty cell scanning left-to-right, top-to-bottom.
 *   2. Try numbers 1-9 in random order (for variety in generated puzzles).
 *   3. If a number is safe, place it and recurse.
 *   4. If recursion fails, remove the number (backtrack) and try the next.
 *   5. If no number works, return false (triggering backtrack in caller).
 */
static bool solveFull(int grid[GRID_SIZE][GRID_SIZE]) {
    /* Find the next empty cell */
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (grid[r][c] == EMPTY_CELL) {
                /* Try numbers in random order for puzzle variety */
                int nums[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
                shuffleArray(nums, 9);

                for (int i = 0; i < 9; i++) {
                    if (isSafe(grid, r, c, nums[i])) {
                        grid[r][c] = nums[i];
                        if (solveFull(grid)) {
                            return true;  /* Solution found! */
                        }
                        grid[r][c] = EMPTY_CELL;  /* Backtrack */
                    }
                }
                return false;  /* No valid number for this cell */
            }
        }
    }
    return true;  /* No empty cells remain - grid is complete */
}

/*
 * countSolutions - Count how many solutions exist for the grid, up to the given limit.
 * Stop searching once the limit is reached.
 */
static int countSolutions(int grid[GRID_SIZE][GRID_SIZE], int limit) {
    if (limit <= 0) return 0;

    int row = -1, col = -1;
    for (int r = 0; r < GRID_SIZE && row == -1; r++) {
        for (int c = 0; c < GRID_SIZE && row == -1; c++) {
            if (grid[r][c] == EMPTY_CELL) {
                row = r;
                col = c;
            }
        }
    }

    if (row == -1) {
        return 1;
    }

    int count = 0;
    for (int num = 1; num <= 9; num++) {
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;
            count += countSolutions(grid, limit - count);
            grid[row][col] = EMPTY_CELL;

            if (count >= limit) {
                break;
            }
        }
    }
    return count;
}

/*
 * generatePuzzle - Create a new random Sudoku puzzle.
 *
 * Steps:
 *   1. Clear the grid.
 *   2. Fill the three diagonal 3x3 boxes (positions 0,0 / 3,3 / 6,6).
 *      These are independent so can be filled without constraint checking.
 *   3. Solve the remaining cells using backtracking.
 *   4. Save the complete grid as the "solution".
 *   5. Remove cells randomly (using shuffled indices to avoid repeats)
 *      while ensuring the puzzle still has exactly one unique solution.
 *   6. Mark remaining cells as "fixed" (given clues).
 */
void generatePuzzle(SudokuPuzzle *p, Difficulty diff) {
    /* Step 1: Clear the grid */
    memset(p->grid, 0, sizeof(p->grid));

    /* Step 2: Fill the three diagonal 3x3 boxes.
     * The diagonal boxes (top-left, center, bottom-right) don't share
     * any row or column, so they can be filled independently. */
    fillBox(p->grid, 0, 0);  /* Top-left box */
    fillBox(p->grid, 3, 3);  /* Center box */
    fillBox(p->grid, 6, 6);  /* Bottom-right box */

    /* Step 3: Solve the remaining cells */
    solveFull(p->grid);

    /* Step 4: Save the complete solution */
    copyGrid(p->solution, p->grid);

    /* Step 5: Remove cells based on difficulty, ensuring unique solution */
    int blanks = countBlanks(diff);
    int removed = 0;

    /* Create a list of all cell indices (0 to 80) */
    int cells[81];
    for (int i = 0; i < 81; i++) {
        cells[i] = i;
    }
    /* Shuffle the cell indices to try removing them in random order */
    shuffleArray(cells, 81);

    for (int i = 0; i < 81 && removed < blanks; i++) {
        int r = cells[i] / GRID_SIZE;
        int c = cells[i] % GRID_SIZE;

        if (p->grid[r][c] != EMPTY_CELL) {
            int saved = p->grid[r][c];
            p->grid[r][c] = EMPTY_CELL;

            /* Only keep the cell empty if the puzzle has exactly 1 solution */
            if (countSolutions(p->grid, 2) == 1) {
                removed++;
            } else {
                p->grid[r][c] = saved;
            }
        }
    }

    /* Step 6: Save puzzle state and mark fixed cells */
    copyGrid(p->puzzle, p->grid);
    memset(p->fixed, 0, sizeof(p->fixed));
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            p->fixed[r][c] = (p->grid[r][c] != EMPTY_CELL);
        }
    }

    p->generated = true;
}

/*
 * resetPuzzle - Restore the grid to the original puzzle state.
 * Clears all user-entered numbers while keeping the given clues.
 */
void resetPuzzle(SudokuPuzzle *p) {
    if (p->generated) {
        copyGrid(p->grid, p->puzzle);
    }
}

/*
 * getHint - Find one empty cell and provide its solution value.
 * Returns true if a hint was found, false if no empty cells exist.
 */
bool getHint(SudokuPuzzle *p, int *hintRow, int *hintCol, int *hintVal) {
    if (!p->generated) return false;

    /* Collect all empty cells */
    int empties[81][2];
    int count = 0;
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (p->grid[r][c] == EMPTY_CELL) {
                empties[count][0] = r;
                empties[count][1] = c;
                count++;
            }
        }
    }

    if (count == 0) return false;

    /* Pick a random empty cell */
    int idx = rand() % count;
    *hintRow = empties[idx][0];
    *hintCol = empties[idx][1];
    *hintVal = p->solution[*hintRow][*hintCol];
    return true;
}
