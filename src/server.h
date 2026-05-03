/*
 * server.h - Minimal HTTP Server for Sudoku GUI
 *
 * A tiny embedded HTTP server that:
 *   - Serves the HTML/CSS/JS frontend on GET /
 *   - Handles API endpoints (POST /generate, /solve_bt, /solve_bb, /hint, /validate)
 *   - Uses Winsock2 on Windows, POSIX sockets on Linux
 *   - Single-threaded, handles one request at a time (sufficient for local use)
 */

#ifndef SERVER_H
#define SERVER_H

#include "sudoku.h"
#include "solver.h"

/* Start the HTTP server on the given port. Blocks forever. */
void startServer(int port);

#endif /* SERVER_H */
