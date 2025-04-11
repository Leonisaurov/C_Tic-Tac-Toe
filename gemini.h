#ifndef TICTACTOE_GEMINI
#define TICTACTOE_GEMINI

#include <pthread.h>

#include "game.h"

typedef struct {
    unsigned int x;
    unsigned int y;
} MOVE;

typedef struct {
    Board board;
    Config conf;
} CONTEXT;

void* gemini_decide(void *args);

bool finished_thinking();
void start_think();

#endif
