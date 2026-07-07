# Sudoku Generator & Solver

A complete, high-performance Sudoku puzzle generator and solver written in C. This project was developed to demonstrate **Backtracking** and **Branch & Bound** algorithms for a Design and Analysis of Algorithms (DAA) mini-project. 

It features a modular codebase with a custom-built HTTP server in C that serves an interactive, dark-mode web graphical user interface (GUI).

---

## Features

- **Algorithmic Solvers:**
  - **Backtracking:** An optimized depth-first search approach for exploring valid Sudoku configurations.
  - **Branch & Bound:** An advanced candidate-reduction approach that prioritizes cells with the fewest remaining valid candidates to minimize the search space.
- **Visual Solver Animations:**
  - Watch the algorithms solve the puzzle cell-by-cell in real-time.
  - Distinct color codings: **Green** for placing values, **Red** for backtracks, and **Orange** for pruned paths.
  - Adjustable animation speed controls and pause/resume capabilities.
- **Performance Comparison Panel:**
  - Real-time comparison metrics including **Recursive Call Count**, **Step Count**, and **Execution Time** (in milliseconds).
  - Displays a dedicated comparison ratio (e.g., `2.4x fewer calls!`) to highlight the efficiency gains of Branch & Bound over traditional Backtracking.
- **Puzzle Generator:** Generates playable Sudoku puzzles of varying difficulty levels (Easy, Intermediate, Hard) by solving a full board and then intelligently removing cells.
- **Custom HTTP Server:** Uses native socket programming (`winsock2`) to process HTTP API requests and serve static assets seamlessly.
- **Sleek Web GUI:** An interactive, dark-themed responsive frontend built with modern vanilla HTML/CSS/JS, featuring premium dark glassmorphism styling and smooth animations.

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
│   └── gui_html.c    # Embedded HTML/CSS/JS for the frontend GUI
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
   gcc -O3 -o sudoku.exe src/main.c src/sudoku.c src/solver.c src/server.c src/gui_html.c -lws2_32
   ```

3. **Run the application:**
   Start the backend server by running the generated executable:
   ```bash
   ./sudoku.exe
   ```

4. **Access the GUI:**
   Open your preferred web browser and navigate to:
   [http://localhost:8080](http://localhost:8080)

---

## Usage & Controls

### GUI Actions
- **Generate New Puzzle:** Click to generate a random valid Sudoku board based on your chosen difficulty.
- **Solve: Backtracking:** Run and visualize the backtracking algorithm.
- **Solve: Branch & Bound:** Run and visualize the candidate-minimization solver.
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
| **Spacebar** | Pause or resume the solver visualization animation. |
| **+** or **=** | Speed up the visualization speed. |
| **-** | Slow down the visualization speed. |