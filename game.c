#include "game.h"
#include "actions.h"
#include "screen.h"

#include "menu.h"
#define DRAW_CELL 27

#include "gemini.h"
#include <pthread.h>

#include <malloc.h>

void print_board(Board *board, Config conf) {
    char player = (board->x_turn?conf.player_1_symbol:conf.player_2_symbol);
    unsigned short color[3];
    if (player == conf.player_1_symbol) {
        color[0] = conf.player_1_color[0];
        color[1] = conf.player_1_color[1];
        color[2] = conf.player_1_color[2];
    } else if (player == conf.player_2_symbol) {
        color[0] = conf.player_2_color[0];
        color[1] = conf.player_2_color[1];
        color[2] = conf.player_2_color[2];
    }
    printf("Turn Of: '\x1b[38;2;%u;%u;%um%c\x1b[0m'\n", color[0], color[1], color[2], player);

    for (unsigned short y = 0; y < 3; y++) {
        if (y != 0)
            printf("\n─┼─┼─\n");
        for (unsigned short x = 0; x < 3; x++) {
            if (x != 0) printf("│");
            if (x == board->x_selected && y == board->y_selected)
                printf("\x1b[48;2;127;127;127m");
            char tile = board->board[y][x];
            if (tile == INIT_CELL) tile = ' ';
            unsigned short color[3] = {255, 255, 255};
            if (tile == conf.player_1_symbol) {
                color[0] = conf.player_1_color[0];
                color[1] = conf.player_1_color[1];
                color[2] = conf.player_1_color[2];
            } else if (tile == conf.player_2_symbol) {
                color[0] = conf.player_2_color[0];
                color[1] = conf.player_2_color[1];
                color[2] = conf.player_2_color[2];
            }
            printf("\x1b[38;2;%d;%d;%dm%c\x1b[0m", color[0], color[1], color[2], tile);
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
    if (n != 1) return -1;

    if (c == 'r') return 1;
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
                                bool turn = board.x_turn;
                                board = new_board();
                                board.x_turn = turn;
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

char game_with_ai(Config conf) {
    Board board = new_board();
    ACTION action = KEY_UP;

    _Bool end_flag                      = false;
    unsigned short ai_thinking_state    = 0;
    pthread_t ai_thinking_thread;
    MOVE move;
    do {
        if (finished_thinking() == 1) {
            start_think();
            ai_thinking_state = 0;
            void *res;
            pthread_join(ai_thinking_thread, &res);
            if (res == NULL) {
                puts("\r\x1b[KRetrying. . .");
                continue;
            }

            move = *(MOVE*)res;
            free(res);

            board.x_selected = move.x;
            board.y_selected = move.y;
            if(put_on_board(&board, conf))
                puts("\r\x1b[KRetrying. . .");
        }
        switch(action) {
            case RETURN:
                return RETURN_MENU_CODE;
            case QUIT:
                end_flag = true;
                break;
            case KEY_ENTER:
                if (board.x_turn && put_on_board(&board, conf)) {
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
            case REQUEST: {
                           // Another KEY Input
                           if (board.x_turn || ai_thinking_state != 0) break;
                           ai_thinking_state++;
                           CONTEXT *context = (CONTEXT*) malloc(sizeof(CONTEXT));
                           *context = (CONTEXT) {
                               .conf = conf,
                               .board = board
                           };
                           if (pthread_create(&ai_thinking_thread, NULL, gemini_decide, context)) {
                               fprintf(stderr, "Error al crear el hilo\n");
                               return 0;
                           }
                           break;
                       }
            case NONE:
                  break;
        }

        if (ai_thinking_state == 0) {
            clear_buffer();
            print_board(&board, conf);
            if (!board.x_turn)
                printf("\nPress 'r' to request the AI move");
            fflush(stdout);
            continue;
        }

        char *ai_msg[4] = {
            "AI thinking",
            "AI thinking.",
            "AI thinking. .",
            "AI thinking. . .",
        };

        unsigned short ai_msg_i = 0;
        if (ai_thinking_state > 500)
            ai_thinking_state = 1;

        if (ai_thinking_state > 400)
            ai_msg_i = 3;
        else if (ai_thinking_state > 300)
            ai_msg_i = 2;
        else if (ai_thinking_state > 200)
            ai_msg_i = 1;


        printf("\r\x1b[K%s", ai_msg[ai_msg_i]);
        fflush(stdout);
        usleep(5000);
        if (ai_thinking_state > 0)
            ai_thinking_state++;
    } while (!end_flag && (ai_thinking_state != 0 || (action = process_action()) != QUIT));

    return 0;
}
