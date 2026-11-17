#ifndef TICTACTOE_CONF
#define TICTACTOE_CONF

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    char player_1_symbol;
    char player_2_symbol;
    unsigned short player_1_color[3];
    unsigned short player_2_color[3];
    char pollinations_app_key[256];      // Developer's App Key (pk_...)
    char pollinations_user_key[256];     // User's authorized key (sk_...)
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
        .pollinations_app_key = "",
        .pollinations_user_key = "",
    };
}

typedef enum {
    CHAR_OPTION,
    COLOR_OPTION,
    STRING_OPTION,
    BUTTON_OPTION,
} OptionType;

typedef struct {
    const char* question;
    OptionType type;
    void* value;           // For CHAR_OPTION: char*, for COLOR_OPTION: unsigned short*, for STRING_OPTION: char*, for BUTTON_OPTION: function pointer
    size_t max_len;        // For STRING_OPTION: max length of string buffer
} Option;

Option* get_options();
int set_option(unsigned int i, Config *conf);

// Config persistence functions
bool config_save(const Config *conf, const char *path);
bool config_load(Config *conf, const char *path);
const char* config_get_path(void);

// External variables for menu.c to sync with opts array
extern char player_1_symbol;
extern char player_2_symbol;
extern unsigned short player_1_color[3];
extern unsigned short player_2_color[3];
extern char pollinations_app_key[256];
extern char pollinations_user_key[256];

#endif
