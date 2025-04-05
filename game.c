#include "game.h"
#include "actions.h"
#include "screen.h"

#include "menu.h"

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
    for (unsigned short y = 0; y < 3; y++) {
        char *fila = board.board[y];
        if (fila[0] == fila[1] && fila[1] == fila[2] && fila[0] != INIT_CELL)
            return fila[0];
    }

    for (unsigned short x = 0; x < 3; x++) {
        if (board.board[0][x] == board.board[1][x] && board.board[1][x] == board.board[2][x] && board.board[0][x] != INIT_CELL)
            return board.board[0][x];
    }

    if (board.board[0][0] == board.board[1][1] && board.board[1][1] == board.board[2][2] && board.board[2][2] != INIT_CELL)
        return board.board[0][0];

    if (board.board[0][2] == board.board[1][1] && board.board[1][1] == board.board[2][0] && board.board[2][0] != INIT_CELL)
        return board.board[0][2];

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

void press_to_close() {
    puts("Press a key to continue. . .");
    char c;
    read(STDIN_FILENO, &c, 1);
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
                    if (c != ' ') {
                        clear_buffer();
                        print_board(&board, conf);
                        printf("\nThe %c player are the winner!\n", c);
                        press_to_close();
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
