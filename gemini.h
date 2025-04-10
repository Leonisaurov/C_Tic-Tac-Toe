#ifndef TICTACTOE_GEMINI
#define TICTACTOE_GEMINI

#include "game.h"

typedef struct {
    unsigned int x;
    unsigned int y;
} MOVE;

MOVE gemini_decide(Board board, Config conf);

#endif
