#ifndef TICTACTOE_GAME
#define TICTACTOE_GAME

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "menu.h"
#include "config.h"

struct Board {
    char board[3][3];
    unsigned short x_selected;
    unsigned short y_selected;
    bool x_turn;
};

typedef struct Board Board;

#define INIT_CELL ' '

static Board new_board() {
    Board board;

    board.board[0][0] = INIT_CELL;
    board.board[0][1] = INIT_CELL;
    board.board[0][2] = INIT_CELL;

    board.board[1][0] = INIT_CELL;
    board.board[1][1] = INIT_CELL;
    board.board[1][2] = INIT_CELL;

    board.board[2][0] = INIT_CELL;
    board.board[2][1] = INIT_CELL;
    board.board[2][2] = INIT_CELL;

    board.x_selected = 0;
    board.y_selected = 0;

    board.x_turn = true;

    return board;
}

char game(Config conf);

#endif
