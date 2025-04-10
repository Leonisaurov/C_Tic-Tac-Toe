#ifndef TICTACTOE_ACTION
#define TICTACTOE_ACTION

typedef enum {
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    REQUEST,
    RETURN,
    QUIT,
    NONE,
} ACTION;

ACTION process_action();

#endif
