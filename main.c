#include <strings.h>
#include <unistd.h>
#include <termios.h>
#include "menu.h"

int main() {
    struct termios raw_term;
    struct termios original_term;

    tcgetattr(STDIN_FILENO, &raw_term);
    original_term = raw_term;

    raw_term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw_term);

    int res = main_menu();

    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
    return res;
}
