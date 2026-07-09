# Sudoku Generator, Solver & Journey Visualizer

A complete, high-performance Sudoku puzzle generator and solver written in C. This project was developed to demonstrate **Backtracking** and **Branch & Bound** algorithms for a Design and Analysis of Algorithms (DAA) mini-project. 

It features a modular codebase with a custom-built HTTP server in C that serves an interactive, dark-mode web graphical user interface (GUI) and a detailed **Journey Visualizer** tab for analyzing the solver's execution path.

---

## Features

- **Algorithmic Solvers:**
  - **Backtracking:** An optimized depth-first search approach for exploring valid Sudoku configurations.
  - **Branch & Bound:** An advanced candidate-reduction approach that prioritizes cells with the fewest remaining valid candidates to minimize the search space.
- **Visual Solver Animations:**
  - Watch the algorithms solve the puzzle cell-by-cell in real-time.
  - Distinct color codings: **Green** for placing values, **Red** for backtracks, and **Orange** for pruned paths.
  - Adjustable animation speed controls and pause/resume capabilities.
- **Branch & Bound Journey Visualizer:**
  - Opens in a new tab to show the step-by-step decision-making process of the solver.
  - Real-time display of **Minimum Remaining Values (MRV)** cell selection heuristic.
  - Interactive constraint analysis (Row, Column, and Block) for every single step.
  - Visual tracking of forward checking and branch pruning.
- **Performance Comparison Panel:**
  - Real-time comparison metrics including **Recursive Call Count**, **Step Count**, and **Execution Time** (in milliseconds).
  - Displays a dedicated comparison ratio (e.g., `2.4x fewer calls!`) to highlight the efficiency gains of Branch & Bound over traditional Backtracking.
- **Puzzle Generator:** Generates playable Sudoku puzzles of varying difficulty levels (Easy, Intermediate, Hard) by solving a full board and then intelligently removing cells.
- **Custom HTTP Server:** Uses native socket programming (`winsock2`) to process HTTP API requests and serve static assets seamlessly.
- **Sleek Web GUI:** An interactive, dark-themed responsive frontend built with modern vanilla HTML/CSS/JS, featuring premium dark glassmorphism styling and smooth animations.

---

## Core Algorithms

### Standard Backtracking
- **Approach:** Iterates through empty cells in a fixed order (top-to-bottom, left-to-right).
- **Search:** Tries digits 1–9 sequentially. If a placement violates a row, column, or block constraint, it immediately backtracks.
- **Drawback:** Plunges deep into search branches that are doomed to fail, wasting substantial CPU cycles.

### Branch & Bound (B&B)
- **Optimization 1 (MRV Heuristic):** Instead of scanning cells in order, the solver selects the empty cell with the **fewest remaining valid candidates** (Minimum Remaining Values). This minimizes the branching factor at each level.
- **Optimization 2 (Forward Checking / Feasibility Bound):** After placing a digit, the solver instantly checks all empty cells in the same row, column, and 3×3 block. If any cell has **zero valid candidates** left, the solver immediately prunes the branch (determines it is infeasible) and backtracks without recursing further.

---

## Journey Visualizer Card Anatomy

The info panel in the Journey Visualizer breaks down the solver's logic at each step:

### I. Action Header
- **PLACE:** Marks a step where a candidate number is successfully assigned to a cell because it satisfies all constraints.
- **BACKTRACK:** Marks a step where the solver has reached a downstream dead end and must undo the current cell's digit, reverting it to empty and stepping back to try a new choice.
- **BRANCH PRUNED:** Marks a step where the solver rejects a branch without searching it because it has been proven mathematically impossible to complete.

### II. Cell Selection (MRV Heuristic)
Before choosing a digit, the solver must choose *which* cell to operate on. 
- The MRV heuristic scans all remaining empty cells on the board.
- It calculates the number of valid digits that can be placed in each empty cell.
- It selects the cell with the **lowest candidate count** (greater than 0).
- **Benefit:** Selecting the cell with the fewest options shrinks the branching factor of the search tree and exposes dead ends as early as possible.

### III. Row & Column Constraints
For the selected cell `(Row r, Col c)`:
- **Row Constraint:** Identifies all digits currently present in Row `r`. These digits are eliminated as candidates.
- **Column Constraint:** Identifies all digits currently present in Column `c`. These digits are eliminated as candidates.

### IV. Block Constraint (3×3 Box)
To enforce the local $3 \times 3$ block rule, the solver maps the cell's global coordinates `(r, c)` to its corresponding block group:
- **Block Row Start ($br$):** 
  $$br = 3 \times \lfloor r / 3 \rfloor$$
- **Block Column Start ($bc$):** 
  $$bc = 3 \times \lfloor c / 3 \rfloor$$
- **Check:** The solver scans all cells from Row $br$ to $br+2$ and Column $bc$ to $bc+2$. Any digit found within this box is eliminated as a candidate for the target cell.

### V. Placement / Pruning Decision
After applying all three constraints, the solver computes the logical union of all blocked digits:
$$\text{Blocked Digits} = \text{Row} \cup \text{Column} \cup \text{Block}$$

- **Valid Placement:** If there is at least one digit in $\{1..9\}$ not in the Blocked set, it is a valid candidate. The solver selects the first one and places it (**PLACE**).
- **Forward Checking Prune:** If placing a number causes any other empty cell on the board to have its union block **all 9 digits**, that cell drops to $0$ candidates. The bounding condition is triggered, and the solver discards the branch (**PRUNED**).

---

## Project Structure

```text
sudoku-solver/
│
├── src/
│   ├── main.c        # Entry point, initializes the HTTP server
│   ├── server.h/c    # Custom HTTP server implementation
│   ├── solver.h/c    # Backtracking and Branch & Bound algorithms
│   ├── sudoku.h/c    # Core Sudoku grid operations and generation logic
│   ├── gui_html.c    # Embedded HTML/CSS/JS for the main GUI
│   └── gui_journey.c # Embedded HTML/CSS/JS for the Journey Visualizer tab
│
├── .gitignore        # Standard Git ignore file for C/C++ projects
└── README.md         # Project documentation and guide
```

---

## Prerequisites

- **C Compiler:** GCC (MinGW-w64 on Windows) or equivalent.
- **Operating System:** Windows (uses `winsock2.h` for native networking).

---

## Compilation and Execution

1. **Clone the repository:**
   ```bash
   git clone https://github.com/SwayamGoyal11/SUDOKU-SOLVER.git
   cd SUDOKU-SOLVER
   ```

2. **Compile the code:**
   Run the following command from the root directory to compile the source code into an executable (using the `-O3` flag is highly recommended to enable compiler optimizations that maximize solver performance):
   ```bash
   gcc -O3 -o sudoku.exe src/main.c src/sudoku.c src/solver.c src/server.c src/gui_html.c src/gui_journey.c -lws2_32
   ```

3. **Run the application:**
   Start the backend server by running the generated executable:
   ```bash
   ./sudoku.exe
   ```

4. **Access the GUI:**
   Open your preferred web browser and navigate to:
   [http://localhost:8080](http://localhost:8080)
   To open the visualizer, generate a puzzle and click **Solve: Branch & Bound**.

---

## Usage & Controls

### GUI Actions
- **Generate New Puzzle:** Click to generate a random valid Sudoku board based on your chosen difficulty.
- **Solve: Backtracking:** Run and visualize the backtracking algorithm on the main page.
- **Solve: Branch & Bound:** Run the branch-and-bound solver, opening the detailed Journey Visualizer in a new tab.
- **Reset:** Wipe all user inputs and reset the board back to the original puzzle state.
- **Hint:** Fills a single empty cell with the correct value.

### Keyboard Shortcuts
When playing manually or controlling the solver visualization, the following keys are active:

| Key / Action | Command Description |
| :--- | :--- |
| **Mouse Click** | Select any cell in the 9x9 grid. |
| **Arrow Keys** | Navigate the selected cursor around the grid cells. |
| **1 - 9** | Input a digit into the selected empty cell. |
| **Backspace / Delete / 0** | Clear the value of the selected user cell. |
| **Spacebar** | Pause or resume the solver visualization animation / journey. |
| **+** or **=** | Speed up the visualization speed. |
| **-** | Slow down the visualization speed. |