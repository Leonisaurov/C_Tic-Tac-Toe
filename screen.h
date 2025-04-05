#ifndef TICTACTOE_SCREEN
#define TICTACTOE_SCREEN
#include <stdio.h>

static void open_alternative_buffer() {
    printf("\x1b[?1049h");
    fflush(stdout);
}

static void close_alternative_buffer() {
    printf("\x1b[?1049l");
    fflush(stdout);
}

static void clear_buffer() {
    printf("\x1b[2J\x1b[3K\x1b[H");
    fflush(stdout);
}

#endif
