# Sudoku Generator & Solver

A complete, high-performance Sudoku puzzle generator and solver written in C. This project was developed to demonstrate **Backtracking** and **Branch & Bound** algorithms for a Design and Analysis of Algorithms (DAA) mini-project. 

It features a modular codebase with a custom-built HTTP server in C that serves an interactive, dark-mode web graphical user interface (GUI).

## Features

- **Algorithmic Solvers:**
  - **Backtracking:** An optimized standard approach for exploring valid Sudoku configurations.
  - **Branch & Bound:** An advanced approach that prioritizes cells with the fewest remaining valid candidates to minimize the search space.
- **Puzzle Generator:** Generates playable Sudoku puzzles of varying difficulty by fully solving a board and then intelligently removing cells.
- **Embedded Web GUI:** A sleek, dark-themed interactive frontend built with HTML/CSS/JS, served directly from the C backend.
- **Custom HTTP Server:** Uses native socket programming (`winsock2`) to process HTTP API requests and serve static assets seamlessly.

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
└── README.md         # This documentation file
```

## Prerequisites

- **C Compiler:** GCC (MinGW-w64 on Windows) or equivalent.
- **Operating System:** Windows (uses `winsock2` for the HTTP server).

## Compilation and Execution

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/sudoku-solver.git
   cd sudoku-solver
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
   ```
   http://localhost:8080
   ```

## Usage

- **Generate Puzzle:** Click the "Generate" button in the web interface to create a new, random Sudoku puzzle.
- **Solve with Backtracking:** Click the "Solve (Backtracking)" button to see the puzzle solved using the classic Depth-First Search method.
- **Solve with Branch & Bound:** Click the "Solve (Branch & Bound)" button to see the puzzle solved using the optimized candidate-reduction method.
- **Play:** You can also manually fill in the grid and test your Sudoku skills!