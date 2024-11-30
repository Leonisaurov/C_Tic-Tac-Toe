#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdbool.h>

void open_alternative_buffer() {
    printf("\x1b[?1049h");
    fflush(stdout);
}

void close_alternative_buffer() {
    printf("\x1b[?1049l");
    fflush(stdout);
}

void clear_buffer() {
    printf("\x1b[2K\x1b[3J\x1b[H");
    fflush(stdout);
}

struct Board {
    char board[3][3];
    ushort x_selected;
    ushort y_selected;
    bool x_turn;
};

typedef struct Board Board;

#define INIT_CELL ' '

Board new_board() {
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

void print_board(Board *board) {
    printf("Turn Of: '%c'\n", (board->x_turn?'X':'O'));

    for (uint y = 0; y < 3; y++) {
        if (y != 0)
            printf("\n─┼─┼─\n");
        for (uint x = 0; x < 3; x++) {
            if (x != 0) printf("│");
            if (x == board->x_selected && y == board->y_selected)
                printf("\x1b[48;2;55;55;55m");
            printf("%c", board->board[y][x]);
            if (x == board->x_selected && y == board->y_selected)
                printf("\x1b[0m");
        }
    }
    fflush(stdout);
}

char check_win(Board board) {
    for (uint y = 0; y < 3; y++) {
        char *fila = board.board[y];
        if (fila[0] == fila[1] && fila[1] == fila[2] && fila[0] != INIT_CELL)
            return fila[0];
    }

    for (uint x = 0; x < 3; x++) {
        if (board.board[0][x] == board.board[1][x] && board.board[1][x] == board.board[2][x] && board.board[0][x] != INIT_CELL)
            return board.board[0][x];
    }

    if (board.board[0][0] == board.board[1][1] && board.board[1][1] == board.board[2][2] && board.board[2][2] != INIT_CELL)
        return board.board[0][0];

    if (board.board[0][2] == board.board[1][1] && board.board[1][1] == board.board[2][0] && board.board[2][0] != INIT_CELL)
        return board.board[0][2];

    return INIT_CELL;
}

bool put_on_board(Board *board) {
    char *cell = &board->board[board->y_selected][board->x_selected];
    if (*cell == INIT_CELL) {
        *cell = (board->x_turn?'x':'o');
        board->x_turn = !board->x_turn;
        return true;
    }
    return false;
}

void press_to_close() {
    puts("Press a key to continue. . .");
    char c;
    read(STDIN_FILENO, &c, 1);
}

char game() {
    char keys[3];
    int n;
    open_alternative_buffer();
    Board board = new_board();

    do {
        if (n == 1 && keys[0] == 'q') break;
        if (n == 1 && (keys[0] == 10 || keys[0] == 13)) {
            if (put_on_board(&board)) {
                char c = check_win(board);
                if (c != ' ') {
                    printf("The %c player are the winner!\n", c);
                    press_to_close();
                    break;
                }
            }
        }

        if (n == 3 && keys[0] == 27) {
            if (keys[1] == '[') {
                switch (keys[2]) {
                    case 'A':
                        if(board.y_selected > 0)
                            board.y_selected--;
                        break;
                    case 'B':
                        if(board.y_selected < 2)
                            board.y_selected++;
                        break;
                    case 'D':
                            if(board.x_selected > 0)
                                board.x_selected--;
                            break;
                    case 'C':
                        if(board.x_selected < 2)
                            board.x_selected++;
                        break;
                }
            }
        }
        clear_buffer();
        print_board(&board);
    } while ((n = read(STDIN_FILENO, keys, 3)) > 0);

    close_alternative_buffer();
    return 0;
}

int main() {
    struct termios raw_term;
    struct termios original_term;

    tcgetattr(STDIN_FILENO, &raw_term);
    original_term = raw_term;

    raw_term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw_term);

    int res = game();

    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
    return res;
}
