#include "menu.h"
#include "screen.h"
#include "actions.h"
#include <stdbool.h>
#include <string.h>

#include "config.h"

char main_menu() {
    Config conf = get_default_config();
    config_load(&conf, NULL);  // Load saved config
    
    // Sync loaded config to opts array
    player_1_symbol = conf.player_1_symbol;
    player_2_symbol = conf.player_2_symbol;
    player_1_color[0] = conf.player_1_color[0];
    player_1_color[1] = conf.player_1_color[1];
    player_1_color[2] = conf.player_1_color[2];
    player_2_color[0] = conf.player_2_color[0];
    player_2_color[1] = conf.player_2_color[1];
    player_2_color[2] = conf.player_2_color[2];
    strncpy(pollinations_app_key, conf.pollinations_app_key, 255);
    pollinations_app_key[255] = '\0';
    strncpy(pollinations_user_key, conf.pollinations_user_key, 255);
    pollinations_user_key[255] = '\0';
    
    open_alternative_buffer();

    ACTION action = NONE;
    unsigned short cursor = 0;

    int game_returncode = RETURN_MENU_CODE;
    do {
        switch(action) {
            case KEY_ENTER: 
                game_returncode = set_option(cursor, &conf);
                break;
            case KEY_UP:
                if (cursor > 0) cursor--;
                break;
            case KEY_DOWN:
                if (cursor < 8) cursor++;
                break;
            case KEY_LEFT:
            case KEY_RIGHT:
            case NONE:
            case REQUEST:
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
                    printf("%s\x1b[4m%c\x1b[0m", opt.question, *(char*)opt.value);
                    break;
                case STRING_OPTION:
                    {
                        char *str = (char*)opt.value;
                        if (strlen(str) > 0) {
                            // Mask the key for display (show first 8 and last 4 chars)
                            char display[256];
                            if (strlen(str) > 16) {
                                snprintf(display, sizeof(display), "%.8s...%.4s", str, str + strlen(str) - 4);
                            } else {
                                strncpy(display, str, sizeof(display) - 1);
                                display[sizeof(display) - 1] = '\0';
                            }
                            printf("%s\x1b[4m%s\x1b[0m", opt.question, display);
                        } else {
                            printf("%s\x1b[4m%s\x1b[0m", opt.question, "(empty)");
                        }
                    }
                    break;
                case COLOR_OPTION:
                    printf("%s[%u, %u, %u]", opt.question, ((unsigned short*)opt.value)[0], ((unsigned short*)opt.value)[1], ((unsigned short*)opt.value)[2]);
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


