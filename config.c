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

int set_option(unsigned int i, Config *conf) {
    Option opt = opts[i];
    const char* question = opt.question;

    char c;
    switch(opt.type) {
        case CHAR_OPTION:
            printf("%s", question);
            fflush(stdout);
            read(STDIN_FILENO, &c, 1);
            opts[i].value = (void*)c;
            break;
        case BUTTON_OPTION:
            return ((int(*) (Config))opts[i].value)(*conf);
            break;
    }
    return RETURN_MENU_CODE;
}
// #include <stdlib.h>
//
// typedef Option* Options;
// unsigned int opt_n = 0;
//
// Options opts = NULL;
//
// Options add_option(Option opt) {
//     Options new_opts = (Options) realloc(opts, sizeof(Option) * (opt_n + 1));
//     if (new_opts == NULL) return new_opts;
//     opts = new_opts;
//     opts[opt_n] = opt;
//     opt_n++;
//     return opts;
// }
