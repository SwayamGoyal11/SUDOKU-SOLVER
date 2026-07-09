/*
 * server.c - Minimal HTTP Server Implementation
 *
 * Handles JSON API requests from the web frontend and serves the HTML page.
 * Uses Winsock2 on Windows, POSIX sockets on Linux/Mac.
 */

#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef int socklen_t;
  #define CLOSESOCK closesocket
#else
  #include <unistd.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #define CLOSESOCK close
  #define SOCKET int
  #define INVALID_SOCKET -1
#endif

/* Embedded HTML page (declared in gui_html.h, auto-generated or included) */
extern const char *GUI_HTML;
extern const char *JOURNEY_HTML;

/* ====================== JSON Helpers ====================== */

/* Format a grid as a JSON 2D array string */
static void gridToJSON(char *buf, int bufSize, int grid[GRID_SIZE][GRID_SIZE]) {
    int pos = 0;
    pos += snprintf(buf + pos, bufSize - pos, "[");
    for (int r = 0; r < GRID_SIZE; r++) {
        pos += snprintf(buf + pos, bufSize - pos, "[");
        for (int c = 0; c < GRID_SIZE; c++) {
            pos += snprintf(buf + pos, bufSize - pos, "%d", grid[r][c]);
            if (c < 8) pos += snprintf(buf + pos, bufSize - pos, ",");
        }
        pos += snprintf(buf + pos, bufSize - pos, "]");
        if (r < 8) pos += snprintf(buf + pos, bufSize - pos, ",");
    }
    snprintf(buf + pos, bufSize - pos, "]");
}

/* Parse a JSON grid from request body (simple parser for our format) */
static bool parseGridFromJSON(const char *json, int grid[GRID_SIZE][GRID_SIZE]) {
    const char *p = strchr(json, '[');
    if (!p) return false;
    p++; /* skip outer [ */

    for (int r = 0; r < GRID_SIZE; r++) {
        p = strchr(p, '[');
        if (!p) return false;
        p++;
        for (int c = 0; c < GRID_SIZE; c++) {
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
            grid[r][c] = (int)strtol(p, (char **)&p, 10);
            while (*p == ' ' || *p == ',' || *p == '\t') p++;
        }
        p = strchr(p, ']');
        if (p) p++;
    }
    return true;
}

/* Parse "difficulty" field from JSON */
static int parseDifficulty(const char *json) {
    const char *p = strstr(json, "\"difficulty\"");
    if (!p) return 0;
    p = strchr(p + 12, ':');
    if (!p) return 0;
    p++;
    while (*p == ' ') p++;
    return (int)strtol(p, NULL, 10);
}

/* ====================== Request Handling ====================== */

/* Send an HTTP response */
static void sendResponse(SOCKET client, int status, const char *contentType,
                          const char *body, int bodyLen) {
    char header[512];
    const char *statusText = (status == 200) ? "OK" : "Bad Request";
    int headerLen = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Cache-Control: no-cache, no-store, must-revalidate\r\n"
        "Pragma: no-cache\r\n"
        "Expires: 0\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, statusText, contentType, bodyLen);
    send(client, header, headerLen, 0);
    if (bodyLen > 0) send(client, body, bodyLen, 0);
}

/* Global puzzle state (single user, local server) */
static SudokuPuzzle g_puzzle;
static bool g_initialized = false;

static void ensurePuzzleInitialized(void) {
    if (!g_initialized) {
        initPuzzle(&g_puzzle);
        g_initialized = true;
    }
}

static void handleGenerateRequest(SOCKET client, const char *body) {
    ensurePuzzleInitialized();

    int diff = parseDifficulty(body);
    if (diff < 0 || diff > 2) diff = 0;
    generatePuzzle(&g_puzzle, (Difficulty)diff);

    static char response[4096];
    static char gridJSON[2048], solJSON[2048], fixedJSON[2048];
    gridToJSON(gridJSON, sizeof(gridJSON), g_puzzle.grid);
    gridToJSON(solJSON, sizeof(solJSON), g_puzzle.solution);

    int fp = 0;
    fp += snprintf(fixedJSON + fp, sizeof(fixedJSON) - fp, "[");
    for (int r = 0; r < GRID_SIZE; r++) {
        fp += snprintf(fixedJSON + fp, sizeof(fixedJSON) - fp, "[");
        for (int c = 0; c < GRID_SIZE; c++) {
            fp += snprintf(fixedJSON + fp, sizeof(fixedJSON) - fp, "%s",
                           g_puzzle.fixed[r][c] ? "true" : "false");
            if (c < 8) fp += snprintf(fixedJSON + fp, sizeof(fixedJSON) - fp, ",");
        }
        fp += snprintf(fixedJSON + fp, sizeof(fixedJSON) - fp, "]");
        if (r < 8) fp += snprintf(fixedJSON + fp, sizeof(fixedJSON) - fp, ",");
    }
    snprintf(fixedJSON + fp, sizeof(fixedJSON) - fp, "]");

    int len = snprintf(response, sizeof(response),
        "{\"grid\":%s,\"solution\":%s,\"fixed\":%s}",
        gridJSON, solJSON, fixedJSON);
    sendResponse(client, 200, "application/json", response, len);
}

static void handleSolveRequest(SOCKET client, const char *body, bool branchAndBound) {
    ensurePuzzleInitialized();

    int grid[GRID_SIZE][GRID_SIZE];
    if (!parseGridFromJSON(body, grid)) {
        sendResponse(client, 400, "application/json", "{\"error\":\"bad grid\"}", 19);
        return;
    }

    SolveResult res = branchAndBound ? solveWithBranchAndBound(grid) : solveWithBacktracking(grid);
    char *response = (char *)malloc(res.stepCount * 40 + 4096);
    static char solvedGrid[2048];
    gridToJSON(solvedGrid, sizeof(solvedGrid), res.grid);

    int pos = snprintf(response, res.stepCount * 40 + 4096,
        "{\"solved\":%s,\"recursiveCalls\":%d,\"executionTimeMs\":%.4f,"
        "\"stepCount\":%d,\"grid\":%s,\"steps\":[",
        res.solved ? "true" : "false", res.recursiveCalls,
        res.executionTimeMs, res.stepCount, solvedGrid);

    for (int i = 0; i < res.stepCount; i++) {
        SolveStep *s = &res.steps[i];
        pos += snprintf(response + pos, 60, "{\"r\":%d,\"c\":%d,\"v\":%d,\"t\":%d}",
                       s->row, s->col, s->value, (int)s->type);
        if (i < res.stepCount - 1) pos += snprintf(response + pos, 2, ",");
    }
    pos += snprintf(response + pos, 4, "]}");

    sendResponse(client, 200, "application/json", response, pos);
    free(response);
}

static void handleHintRequest(SOCKET client, const char *body) {
    ensurePuzzleInitialized();

    parseGridFromJSON(body, g_puzzle.grid);
    int hr, hc, hv;
    if (getHint(&g_puzzle, &hr, &hc, &hv)) {
        static char resp[128];
        int len = snprintf(resp, sizeof(resp),
            "{\"found\":true,\"row\":%d,\"col\":%d,\"value\":%d}", hr, hc, hv);
        sendResponse(client, 200, "application/json", resp, len);
    } else {
        sendResponse(client, 200, "application/json", "{\"found\":false}", 15);
    }
}

/* ====================== Server Main Loop ====================== */

void startServer(int port) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        fprintf(stderr, "Error: Cannot create socket\n");
        return;
    }

    int optval = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((unsigned short)port);

    if (bind(serverSock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        fprintf(stderr, "Error: Cannot bind to port %d\n", port);
        CLOSESOCK(serverSock);
        return;
    }

    listen(serverSock, 5);
    printf("===========================================\n");
    printf("  Sudoku Solver Server running!\n");
    printf("  Open your browser at:\n");
    printf("  >>> http://localhost:%d <<<\n", port);
    printf("===========================================\n");
    fflush(stdout);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET client = accept(serverSock, (struct sockaddr *)&clientAddr, &clientLen);
        if (client == INVALID_SOCKET) continue;

        /* Read request (up to 64KB) */
        char buf[65536];
        int total = 0;
        int n;
        while (total < (int)sizeof(buf) - 1) {
            n = recv(client, buf + total, sizeof(buf) - 1 - total, 0);
            if (n <= 0) break;
            total += n;
            buf[total] = '\0';
            /* Check if we have the full request (headers + body) */
            char *headerEnd = strstr(buf, "\r\n\r\n");
            if (headerEnd) {
                char *clHeader = strstr(buf, "Content-Length:");
                if (!clHeader) break; /* No body expected */
                int contentLen = atoi(clHeader + 15);
                int headerSize = (int)(headerEnd - buf) + 4;
                if (total >= headerSize + contentLen) break;
            }
        }

        /* Parse method and path */
        char method[16] = {0}, path[256] = {0};
        sscanf(buf, "%15s %255s", method, path);

        if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
            sendResponse(client, 200, "text/html; charset=utf-8", GUI_HTML, (int)strlen(GUI_HTML));
            CLOSESOCK(client);
            continue;
        }

        if (strcmp(method, "GET") == 0 && strcmp(path, "/journey") == 0) {
            sendResponse(client, 200, "text/html; charset=utf-8", JOURNEY_HTML, (int)strlen(JOURNEY_HTML));
            CLOSESOCK(client);
            continue;
        }


        /* Find body */
        char *body = strstr(buf, "\r\n\r\n");
        if (body) body += 4; else body = "";

        if (strcmp(method, "OPTIONS") == 0) {
            sendResponse(client, 200, "text/plain", "", 0);
        } else if (strcmp(method, "POST") == 0 && strcmp(path, "/generate") == 0) {
            handleGenerateRequest(client, body);
        } else if (strcmp(method, "POST") == 0 && strcmp(path, "/solve_bt") == 0) {
            handleSolveRequest(client, body, false);
        } else if (strcmp(method, "POST") == 0 && strcmp(path, "/solve_bb") == 0) {
            handleSolveRequest(client, body, true);
        } else if (strcmp(method, "POST") == 0 && strcmp(path, "/hint") == 0) {
            handleHintRequest(client, body);
        } else {
            sendResponse(client, 404, "text/plain", "Not Found", 9);
        }
        CLOSESOCK(client);
    }

    CLOSESOCK(serverSock);
#ifdef _WIN32
    WSACleanup();
#endif
}
