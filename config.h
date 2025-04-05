#ifndef TICTACTOE_CONF
#define TICTACTOE_CONF

#include <stdio.h>

typedef struct {
    char player_1_symbol;
    char player_2_symbol;
} Config;

static Config get_default_config() {
    return (Config) {
        .player_1_symbol = '1',
        .player_2_symbol = '2',
    };
}

typedef enum {
    CHAR_OPTION,
} OptionType;

typedef struct {
    const char* question;
    OptionType type;
    char value;
} Option;

Option* get_options();
void set_option(unsigned int i);

#endif
