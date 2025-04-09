#ifndef TICTACTOE_CONF
#define TICTACTOE_CONF

#include <stdio.h>

typedef struct {
    char player_1_symbol;
    char player_2_symbol;
    unsigned short player_1_color[3];
    unsigned short player_2_color[3];
} Config;

static Config get_default_config() {
    return (Config) {
        .player_1_symbol = 'x',
        .player_2_symbol = 'o',
        .player_1_color = {
            200, 100, 150
        },
        .player_2_color = {
            100, 200, 150
        },
    };
}

typedef enum {
    CHAR_OPTION,
    COLOR_OPTION,
    BUTTON_OPTION,
} OptionType;

typedef struct {
    const char* question;
    OptionType type;
    void* value;
} Option;

Option* get_options();
int set_option(unsigned int i, Config *conf);

#endif
