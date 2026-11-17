#include "config.h"
#include <unistd.h>
#include <time.h>

#include "game.h"

// Forward declaration
int authorize_pollinations(Config conf);

unsigned short player_1_color[3] = {200, 100, 150};
unsigned short player_2_color[3] = {100, 200, 150};
char pollinations_app_key[256] = "";
char pollinations_user_key[256] = "";

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
        .question = "Pollinations App Key (pk_): ",
        .type = CHAR_OPTION,
        .value = (void*) pollinations_app_key,
    },
    {
        .question = "  Authorize with Pollinations  ",
        .type = BUTTON_OPTION,
        .value = authorize_pollinations,
    },
    {
        .question = "Pollinations User Key (sk_): ",
        .type = CHAR_OPTION,
        .value = (void*) pollinations_user_key,
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
#include <curl/curl.h>
#include <jansson.h>

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
    if (strcmp(title, opts[4].question) == 0) {
        strncpy(conf->pollinations_app_key, (char*)any, 255);
        conf->pollinations_app_key[255] = '\0';
        return;
    }
}

// Estructura para acumular la respuesta en memoria
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        fprintf(stderr, "No hay suficiente memoria (realloc retornó NULL)\n");
        return 0;
    }
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// Device Flow: Request device code
bool pollinations_request_device_code(const char *app_key, char *device_code, char *user_code, char *verification_uri, char *verification_uri_complete, int *expires_in, int *interval) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl) {
        free(chunk.memory);
        curl_global_cleanup();
        return false;
    }

    char url[512];
    snprintf(url, sizeof(url), "https://enter.pollinations.ai/api/device/code");

    char post_data[512];
    snprintf(post_data, sizeof(post_data), "{\"client_id\": \"%s\"}", app_key);

    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() fallo: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory);
        curl_global_cleanup();
        return false;
    }

    json_error_t error;
    json_t *root = json_loads(chunk.memory, 0, &error);
    if(!root) {
        fprintf(stderr, "Error al parsear JSON: %s\n", error.text);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory);
        curl_global_cleanup();
        return false;
    }

    json_t *j_device_code = json_object_get(root, "device_code");
    json_t *j_user_code = json_object_get(root, "user_code");
    json_t *j_verification_uri = json_object_get(root, "verification_uri");
    json_t *j_verification_uri_complete = json_object_get(root, "verification_uri_complete");
    json_t *j_expires_in = json_object_get(root, "expires_in");
    json_t *j_interval = json_object_get(root, "interval");

    if (j_device_code && json_is_string(j_device_code))
        strncpy(device_code, json_string_value(j_device_code), 255);
    if (j_user_code && json_is_string(j_user_code))
        strncpy(user_code, json_string_value(j_user_code), 63);
    if (j_verification_uri && json_is_string(j_verification_uri))
        strncpy(verification_uri, json_string_value(j_verification_uri), 255);
    if (j_verification_uri_complete && json_is_string(j_verification_uri_complete))
        strncpy(verification_uri_complete, json_string_value(j_verification_uri_complete), 255);
    if (j_expires_in && json_is_integer(j_expires_in))
        *expires_in = json_integer_value(j_expires_in);
    if (j_interval && json_is_integer(j_interval))
        *interval = json_integer_value(j_interval);

    json_decref(root);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    curl_global_cleanup();
    return true;
}

// Device Flow: Poll for token
bool pollinations_poll_token(const char *device_code, char *access_token) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(!curl) {
        free(chunk.memory);
        curl_global_cleanup();
        return false;
    }

    char url[512];
    snprintf(url, sizeof(url), "https://enter.pollinations.ai/api/device/token");

    char post_data[512];
    snprintf(post_data, sizeof(post_data), "{\"device_code\": \"%s\"}", device_code);

    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() fallo: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory);
        curl_global_cleanup();
        return false;
    }

    json_error_t error;
    json_t *root = json_loads(chunk.memory, 0, &error);
    if(!root) {
        fprintf(stderr, "Error al parsear JSON: %s\n", error.text);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory);
        curl_global_cleanup();
        return false;
    }

    json_t *j_error = json_object_get(root, "error");
    if (j_error && json_is_string(j_error)) {
        const char *err = json_string_value(j_error);
        if (strcmp(err, "authorization_pending") == 0) {
            json_decref(root);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            free(chunk.memory);
            curl_global_cleanup();
            return false; // Still pending
        }
        fprintf(stderr, "Error en autorización: %s\n", err);
        json_decref(root);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory);
        curl_global_cleanup();
        return false;
    }

    json_t *j_access_token = json_object_get(root, "access_token");
    if (j_access_token && json_is_string(j_access_token)) {
        strncpy(access_token, json_string_value(j_access_token), 255);
        json_decref(root);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.memory);
        curl_global_cleanup();
        return true; // Success
    }

    json_decref(root);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    curl_global_cleanup();
    return false;
}

int authorize_pollinations(Config conf) {
    if (strlen(conf.pollinations_app_key) == 0) {
        printf("\n\x1b[31mError: Primero configure la 'Pollinations App Key (pk_)'\x1b[0m\n");
        printf("Presione cualquier tecla para continuar...");
        fflush(stdout);
        char dummy;
        read(STDIN_FILENO, &dummy, 1);
        return RETURN_MENU_CODE;
    }

    printf("\n\x1b[36m=== Autorización con Pollinations (Device Flow) ===\x1b[0m\n");
    fflush(stdout);

    char device_code[256] = "";
    char user_code[64] = "";
    char verification_uri[256] = "";
    char verification_uri_complete[256] = "";
    int expires_in = 1800;
    int interval = 5;

    printf("\nSolicitando código de dispositivo...\n");
    fflush(stdout);

    if (!pollinations_request_device_code(conf.pollinations_app_key, device_code, user_code, verification_uri, verification_uri_complete, &expires_in, &interval)) {
        printf("\n\x1b[31mError al solicitar código de dispositivo\x1b[0m\n");
        printf("Presione cualquier tecla para continuar...");
        fflush(stdout);
        char dummy;
        read(STDIN_FILENO, &dummy, 1);
        return RETURN_MENU_CODE;
    }

    printf("\n\x1b[32m¡Código obtenido!\x1b[0m\n");
    printf("\n\x1b[33m1. Abra su navegador en:\x1b[0m\n");
    printf("   \x1b[4m%s\x1b[0m\n", verification_uri_complete);
    printf("\n\x1b[33m2. Ingrese el código de usuario:\x1b[0m \x1b[1m%s\x1b[0m\n", user_code);
    printf("\n\x1b[33m3. Autorice la aplicación en la página web\x1b[0m\n");
    printf("\n\x1b[90mEsperando autorización (expira en %d segundos)...\x1b[0m\n", expires_in);
    fflush(stdout);

    char access_token[256] = "";
    time_t start_time = time(NULL);
    bool authorized = false;

    while (time(NULL) - start_time < expires_in) {
        printf("\r\x1b[K\x1b[90mConsultando... (%lds)\x1b[0m", time(NULL) - start_time);
        fflush(stdout);

        if (pollinations_poll_token(device_code, access_token)) {
            authorized = true;
            break;
        }

        sleep(interval);
    }

    if (authorized) {
        printf("\n\n\x1b[32m¡Autorización exitosa!\x1b[0m\n");
        printf("Su clave de usuario (sk_...) ha sido guardada.\n");
        
        // We need to update the global variable and the config
        // Since we can't modify the passed conf directly (it's a copy),
        // we'll update the global pollinations_user_key
        strncpy(pollinations_user_key, access_token, 255);
        pollinations_user_key[255] = '\0';
        
        // Also update the opts array value
        strncpy((char*)opts[7].value, access_token, 255); // opts[7] is the user key storage
        ((char*)opts[7].value)[255] = '\0';
    } else {
        printf("\n\n\x1b[31mTiempo agotado o error en la autorización\x1b[0m\n");
    }

    printf("Presione cualquier tecla para continuar...");
    fflush(stdout);
    char dummy;
    read(STDIN_FILENO, &dummy, 1);
    return RETURN_MENU_CODE;
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
            if (strcmp(question, "Pollinations App Key (pk_): ") == 0) {
                char input[256] = "";
                printf("\n%s", question);
                fflush(stdout);
                int n = read(STDIN_FILENO, input, 255);
                if (n > 0) {
                    input[n] = '\0';
                    // Remove trailing newline if present
                    if (input[n-1] == '\n') input[n-1] = '\0';
                    strncpy((char*)opts[i].value, input, 255);
                    ((char*)opts[i].value)[255] = '\0';
                    set_conf(conf, opts[i].question, opts[i].value);
                }
            } else {
                printf("\n%s", question);
                fflush(stdout);
                read(STDIN_FILENO, &c, 1);
                opts[i].value = (void*)c;
                set_conf(conf, opts[i].question, (void*)c);
            }
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
