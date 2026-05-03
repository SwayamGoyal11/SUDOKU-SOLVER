/*
 * main.c - Entry Point for Sudoku Solver Application
 *
 * Starts a local HTTP server that serves the web-based GUI and
 * handles API requests for puzzle generation and solving.
 *
 * Usage:
 *   1. Compile: gcc -o sudoku src/main.c src/sudoku.c src/solver.c src/server.c src/gui_html.c -lws2_32
 *   2. Run: ./sudoku
 *   3. Open browser: http://localhost:8080
 *
 * File Dependencies:
 *   sudoku.h/c   - Core puzzle logic (generation, validation)
 *   solver.h/c   - Backtracking and Branch & Bound solvers
 *   server.h/c   - Embedded HTTP server
 *   gui_html.c   - Embedded HTML/CSS/JS frontend
 */

#include "server.h"
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_PORT 8080

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) port = DEFAULT_PORT;
    }

    printf("Sudoku Solver - DAA Project\n");
    printf("Backtracking & Branch and Bound\n\n");

    /* Start the server (blocks forever) */
    startServer(port);

    return 0;
}
