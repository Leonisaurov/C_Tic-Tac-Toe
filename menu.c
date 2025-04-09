#include "menu.h"
#include "screen.h"
#include "actions.h"
#include <stdbool.h>

#include "config.h"

#include "game.h"

char main_menu() {
    Config conf = get_default_config();
    open_alternative_buffer();

    ACTION action = NONE;
    unsigned short cursor = 0;

    int game_returncode = RETURN_MENU_CODE;
    do {
        switch(action) {
            case KEY_ENTER:
                set_option(cursor, &conf);
                break;
            case KEY_UP:
                if (cursor > 0) cursor--;
                break;
            case KEY_DOWN:
                if (cursor < 2) cursor++;
                break;
            case KEY_LEFT:
            case KEY_RIGHT:
            case NONE:
            case RETURN:
            case QUIT:
                break;
        }

        clear_buffer();
        unsigned int i = 0;
        while (get_options()[i].question != NULL) {
            if (i != 0)
                printf("\n");
            Option opt = get_options()[i];
            if (cursor == i) printf("\x1b[48;2;150;150;150m");

           switch(opt.type) {
                case CHAR_OPTION:
                    printf("%s\x1b[4m%c\x1b[0m", opt.question, (char)opt.value);
                    break;
                case BUTTON_OPTION:
                    printf("[%s]", opt.question);
                    break;
            }

            if (cursor == i) printf("\x1b[0m");
            i++;
        }

        fflush(stdout);
    } while ((game_returncode == RETURN_MENU_CODE) && (action = process_action()) != QUIT);

    close_alternative_buffer();
    return game_returncode;
}


