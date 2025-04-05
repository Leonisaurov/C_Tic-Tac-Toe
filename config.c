#include "config.h"
#include <unistd.h>

Option opts[] = {
    {
        .question = "Player 1 symbol: ",
        .type = CHAR_OPTION,
        .value = 'x',
    },
    {
        .question = "Player 2 symbol: ",
        .type = CHAR_OPTION,
        .value = 'o',
    },
    {
        .question = NULL,
        .type = CHAR_OPTION,
        .value = 0,
    },
};

Option* get_options() {
    return opts;
}

void set_option(unsigned int i) {
    Option opt = opts[i];
    const char* question = opt.question;

    printf("%s", question);
    fflush(stdout);

    char c;
    switch(opt.type) {
        case CHAR_OPTION:
            read(STDIN_FILENO, &c, 1);
            opts[i].value = c;
            break;
    }
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
