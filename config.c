#include "config.h"
#include <unistd.h>

#include "game.h"

Option opts[] = {
    {
        .question = "Player 1 symbol: ",
        .type = CHAR_OPTION,
        .value = (void*) 'x',
    },
    {
        .question = "Player 2 symbol: ",
        .type = CHAR_OPTION,
        .value = (void*) 'o',
    },
    {
        .question = "  Start Game  ",
        .type = BUTTON_OPTION,
        .value = game,
    },
    {
        .question = NULL,
        .type = CHAR_OPTION,
        .value = NULL,
    },
};

Option* get_options() {
    return opts;
}

#include "string.h"

void set_conf(Config *conf, char *title, void* any) {
    if (strcmp(title, opts[0].question) == 0) {
        conf->player_1_symbol = (char)any;
        return;
    }
    if (strcmp(title, opts[1].question) == 0) {
        conf->player_2_symbol = (char)any;
        return;
    }
}

int set_option(unsigned int i, Config *conf) {
    Option opt = opts[i];
    const char* question = opt.question;

    char c;
    switch(opt.type) {
        case CHAR_OPTION:
            printf("\n%s", question);
            fflush(stdout);
            read(STDIN_FILENO, &c, 1);
            opts[i].value = (void*)c;
            set_conf(conf, opts[i].question, (void*)c);
            break;
        case BUTTON_OPTION:
            return ((int(*) (Config))opts[i].value)(*conf);
            break;
    }

    return RETURN_MENU_CODE;
}
