#include "actions.h"
#include <unistd.h>

ACTION process_action() {
    char input[3];
    int n = read(STDIN_FILENO, input, 3);
    if (n == 0) return QUIT;

    if (n == 1) {
        switch(input[0]) {
            case 'r':   return REQUEST;
            case 'q':   return QUIT;
            case 27:    return RETURN;
            case 10:
            case 13:
                        return KEY_ENTER;
        }
    }

    if (n == 3 && input[0] == 27 && input[1] == '[') {
        switch (input[2]) {
            case 'A':
                return KEY_UP;
            case 'B':
                return KEY_DOWN;
            case 'D':
                return KEY_LEFT;
            case 'C':
                return KEY_RIGHT;
        }
    }

    return NONE;
}
