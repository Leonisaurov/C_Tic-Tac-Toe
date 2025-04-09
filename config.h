#ifndef TICTACTOE_CONF
#define TICTACTOE_CONF

#include <stdio.h>

typedef struct {
    char player_1_symbol;
    char player_2_symbol;
} Config;

static Config get_default_config() {
    return (Config) {
        .player_1_symbol = 'x',
        .player_2_symbol = 'o',
    };
}

typedef enum {
    CHAR_OPTION,
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
