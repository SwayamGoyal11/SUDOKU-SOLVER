/*
 * sudoku.h - Sudoku Puzzle Data Structures and Core Functions
 * 
 * This header defines the core data structures for representing a 9x9 Sudoku
 * puzzle and declares functions for puzzle generation, validation, and manipulation.
 * 
 * Key concepts:
 *   - A Sudoku grid is a 9x9 array of integers (0 = empty, 1-9 = filled)
 *   - "Fixed" cells are the ones given in the original puzzle (cannot be changed)
 *   - Difficulty controls how many cells are removed from a complete solution
 */

#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdbool.h>

/* ====================== Constants ====================== */
#define GRID_SIZE   9       /* Sudoku grid is 9x9 */
#define BOX_SIZE    3       /* Each sub-box is 3x3 */
#define EMPTY_CELL  0       /* 0 represents an empty cell */
#define MAX_STEPS   10000   /* Maximum solve steps to record for visualization */

/* ====================== Enumerations ====================== */

/*
 * Difficulty levels control how many cells are removed from a complete puzzle.
 *   Easy:   30 cells removed (51 given)
 *   Medium: 40 cells removed (41 given)
 *   Hard:   50 cells removed (31 given)
 */
typedef enum {
    DIFFICULTY_EASY   = 0,
    DIFFICULTY_MEDIUM = 1,
    DIFFICULTY_HARD   = 2
} Difficulty;

/* ====================== Data Structures ====================== */

/*
 * SudokuPuzzle - Represents a complete Sudoku puzzle state.
 *
 * Fields:
 *   grid     - Current state of the puzzle (what the user sees/modifies)
 *   puzzle   - Original generated puzzle (used for reset)
 *   solution - The complete valid solution
 *   fixed    - Marks which cells were given in the puzzle (true = given, read-only)
 *   generated - Whether a puzzle has been generated yet
 */
typedef struct {
    int  grid[GRID_SIZE][GRID_SIZE];
    int  puzzle[GRID_SIZE][GRID_SIZE];
    int  solution[GRID_SIZE][GRID_SIZE];
    bool fixed[GRID_SIZE][GRID_SIZE];
    bool generated;
} SudokuPuzzle;

/* ====================== Function Declarations ====================== */

/* Initialize a puzzle to empty state */
void initPuzzle(SudokuPuzzle *p);

/*
 * isSafe - Check if placing 'num' at grid[row][col] is valid.
 *
 * This is the core constraint check for Sudoku. It verifies:
 *   1. 'num' is not already in the same row
 *   2. 'num' is not already in the same column
 *   3. 'num' is not already in the same 3x3 sub-box
 *
 * Returns: true if placement is valid, false otherwise.
 */
bool isSafe(int grid[GRID_SIZE][GRID_SIZE], int row, int col, int num);

/*
 * generatePuzzle - Generate a new random Sudoku puzzle.
 *
 * Algorithm:
 *   1. Fill the three diagonal 3x3 boxes (they don't constrain each other)
 *   2. Solve the remaining cells using backtracking
 *   3. Remove cells based on difficulty level
 *   4. Store both the puzzle and solution
 */
void generatePuzzle(SudokuPuzzle *p, Difficulty diff);

/* Reset the puzzle grid back to the original generated puzzle */
void resetPuzzle(SudokuPuzzle *p);

/* Copy one grid into another */
void copyGrid(int dest[GRID_SIZE][GRID_SIZE], int src[GRID_SIZE][GRID_SIZE]);

/* Check if placing 'num' at (row, col) conflicts with existing numbers */
bool isValidEntry(int grid[GRID_SIZE][GRID_SIZE], int row, int col, int num);

/* Get the number of blanks for a given difficulty */
int countBlanks(Difficulty diff);

/* Find a hint: returns true and sets *hintRow, *hintCol, *hintVal if found */
bool getHint(SudokuPuzzle *p, int *hintRow, int *hintCol, int *hintVal);

#endif /* SUDOKU_H */
