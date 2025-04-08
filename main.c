#include <strings.h>
#include <unistd.h>
#include <termios.h>
#include "menu.h"
#include <stdio.h>

int main() {
    struct termios raw_term;
    struct termios original_term;

    tcgetattr(STDIN_FILENO, &raw_term);
    original_term = raw_term;

    raw_term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw_term);

    printf("\x1b[?25l");
    fflush(stdout);

    int res = main_menu();

    printf("\x1b[?25h");
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
    return res;
}
