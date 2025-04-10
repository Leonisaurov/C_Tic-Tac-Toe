#include "config.h"
#include <unistd.h>

#include "game.h"

unsigned short player_1_color[3] = {200, 100, 150};
unsigned short player_2_color[3] = {100, 200, 150};

Option opts[] = {
    {
        .question = "Player 1 symbol: ",
        .type = CHAR_OPTION,
        .value = (void*) 'x',
    },
    {
        .question = "Player 1 color: ",
        .type = COLOR_OPTION,
        .value = (int*) player_1_color,
    },
    {
        .question = "Player 2 symbol: ",
        .type = CHAR_OPTION,
        .value = (void*) 'o',
    },
    {
        .question = "Player 2 color: ",
        .type = COLOR_OPTION,
        .value = (int*) player_2_color,
    },
    {
        .question = "  Start Game  ",
        .type = BUTTON_OPTION,
        .value = game,
    },
    {
        .question = "  Start Game VS AI  ",
        .type = BUTTON_OPTION,
        .value = game_with_ai,
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
        unsigned short *color = any;
        conf->player_1_color[0] = color[0];
        conf->player_1_color[1] = color[1];
        conf->player_1_color[2] = color[2];
        return;
    }
    if (strcmp(title, opts[2].question) == 0) {
        conf->player_2_symbol = (char)any;
        return;
    }
    if (strcmp(title, opts[3].question) == 0) {
        unsigned short *color = any;
        conf->player_2_color[0] = color[0];
        conf->player_2_color[1] = color[1];
        conf->player_2_color[2] = color[2];
        return;
    }
}

bool is_numeric(char c) {
    return (c >= '0' && c <= '9');
}

unsigned short read_ushort() {
    char c[2];
    unsigned short num = 0;
    for (unsigned short i = 0; i < 3; i++) {
        short n = read(STDIN_FILENO, c, 2);
        if (!is_numeric(c[0])) {
            i--;
            continue;
        }
        if (i == 0 && c[0] == '0') return 0;
        putchar(c[0]);
        fflush(stdout);

        num *= 10;
        num += c[0] - '0';
    }

    return num;
}

int set_option(unsigned int i, Config *conf) {
    Option opt = opts[i];
    const char* question = opt.question;

    char c;
    unsigned short color[3] = {0, 0, 0};
    switch(opt.type) {
        case CHAR_OPTION:
            printf("\n%s", question);
            fflush(stdout);
            read(STDIN_FILENO, &c, 1);
            opts[i].value = (void*)c;
            set_conf(conf, opts[i].question, (void*)c);
            break;
        case COLOR_OPTION:
            puts("");
            for (unsigned short i = 0; i < 3; i++) {
                char *light = "\x1b[48;2;175;175;175m";
                char *l1 = (i == 0?light:"");
                char *l2 = (i == 1?light:"");
                char *l3 = (i == 2?light:"");

                printf("\r\x1b[K%s[%s%u\x1b[0m, %s%u\x1b[0m, %s%u\x1b[0m]", question, l1, color[0], l2, color[1], l3, color[2]);
                fflush(stdout);
                color[i] = read_ushort();
            }

            ((unsigned short*)opts[i].value)[0] = color[0];
            ((unsigned short*)opts[i].value)[1] = color[1];
            ((unsigned short*)opts[i].value)[2] = color[2];

            set_conf(conf, opts[i].question, (void*) color);

            break;
        case BUTTON_OPTION:
            return ((int(*) (Config))opts[i].value)(*conf);
    }

    return RETURN_MENU_CODE;
}
