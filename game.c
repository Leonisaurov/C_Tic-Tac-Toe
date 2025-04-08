#include "game.h"
#include "actions.h"
#include "screen.h"

#include "menu.h"
#define DRAW_CELL 27

void print_board(Board *board, Config conf) {
    printf("Turn Of: '%c'\n", (board->x_turn?conf.player_1_symbol:conf.player_2_symbol));

    for (unsigned short y = 0; y < 3; y++) {
        if (y != 0)
            printf("\n─┼─┼─\n");
        for (unsigned short x = 0; x < 3; x++) {
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
    bool blank = false;
    for (unsigned short y = 0; y < 3; y++) {
        char *fila = board.board[y];
        if (fila[0] == INIT_CELL) {
            blank = true;
        } else if (fila[0] == fila[1]) {
            if (fila[1] == fila[2])
                return fila[0];
            else if (fila[2] == INIT_CELL)
                blank = true;
        } else if (fila[1] == INIT_CELL)
            blank = true;
    }

    for (unsigned short x = 0; x < 3; x++) {
        if (board.board[0][x] == INIT_CELL)
            blank = true;
        else if (board.board[0][x] == board.board[1][x]) {
            if (board.board[1][x] == board.board[2][x])
                return board.board[0][x];
            else if (board.board[2][x] == INIT_CELL)
                blank = true;
        } else if (board.board[1][x] == INIT_CELL)
            blank = true;
    }

    if (board.board[0][0] == INIT_CELL)
        blank = true;
    else if (board.board[0][0] == board.board[1][1]) {
        if(board.board[1][1] == board.board[2][2])
            return board.board[0][0];
        else if (board.board[2][2] == INIT_CELL)
            blank = true;
    } else if (board.board[1][1] == INIT_CELL)
        blank = true;

    if (board.board[2][0] == INIT_CELL)
        blank = true;
    else if (board.board[0][2] == board.board[1][1]) {
        if (board.board[1][1] == board.board[2][0])
            return board.board[0][2];
        else if (board.board[2][0] == INIT_CELL)
            blank = true;
    } else if (board.board[1][1] == INIT_CELL)
        blank = true;

    if (blank == false)
        return DRAW_CELL;

    return INIT_CELL;
}

bool put_on_board(Board *board, Config conf) {
    char *cell = &board->board[board->y_selected][board->x_selected];
    if (*cell == INIT_CELL) {
        *cell = (board->x_turn?conf.player_1_symbol:conf.player_2_symbol);
        board->x_turn = !board->x_turn;
        return true;
    }
    return false;
}

short press_to_close() {
    puts("Press r to play again.");
    puts("      q to exit.");
    char c;
    short n = read(STDIN_FILENO, &c, 1);
    fprintf(stderr, "Input: %c\n", c);
    if (n != 1) return -1;

    fprintf(stderr, "Despues del error\n");
    if (c == 'r') return 1;
    fprintf(stderr, "C, no es igual a 'r'\n");
    if (c == 'q') return 0;

    return 2;
}

char game(Config conf) {
    Board board = new_board();
    ACTION action = NONE;

    _Bool end_flag = false;
    do {
        switch(action) {
            case RETURN:
                return RETURN_MENU_CODE;
            case QUIT:
                end_flag = true;
                break;
            case KEY_ENTER:
                if (put_on_board(&board, conf)) {
                    char c = check_win(board);
                    if (c != INIT_CELL) {
                        while (true) {
                            clear_buffer();
                            print_board(&board, conf);
                            if (c == DRAW_CELL)
                                printf("\nDraw! Anyone win. . .\n");
                            else
                                printf("\nThe %c player are the winner!\n", c);
                            short res = press_to_close();
                            if (res == 2) continue;

                            if (res < 1) end_flag = true;
                            else if (res == 1) {
                                board = new_board();
                                fprintf(stderr, "Hola\n");
                            }
                            break;
                        }
                        break;
                    }
                }
                break;
            case KEY_UP:
                if(board.y_selected > 0)
                    board.y_selected--;
                break;
            case KEY_DOWN:
                if(board.y_selected < 2)
                    board.y_selected++;
                break;
            case KEY_LEFT:
                if(board.x_selected > 0)
                    board.x_selected--;
                break;
            case KEY_RIGHT:
                if(board.x_selected < 2)
                    board.x_selected++;
                break;
            case NONE:
                // Another KEY Input
                break;
        }

        clear_buffer();
        print_board(&board, conf);
    } while (!end_flag && (action = process_action()) != QUIT);

    return 0;
}
