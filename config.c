#include "config.h"
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "game.h"

// Forward declaration
int authorize_pollinations(Config conf);

unsigned short player_1_color[3] = {200, 100, 150};
unsigned short player_2_color[3] = {100, 200, 150};
char player_1_symbol = 'x';
char player_2_symbol = 'o';
char pollinations_app_key[256] = "";
char pollinations_user_key[256] = "";

Option opts[] = {
    {
        .question = "Player 1 symbol: ",
        .type = CHAR_OPTION,
        .value = &player_1_symbol,
        .max_len = 0,
    },
    {
        .question = "Player 1 color: ",
        .type = COLOR_OPTION,
        .value = player_1_color,
        .max_len = 0,
    },
    {
        .question = "Player 2 symbol: ",
        .type = CHAR_OPTION,
        .value = &player_2_symbol,
        .max_len = 0,
    },
    {
        .question = "Player 2 color: ",
        .type = COLOR_OPTION,
        .value = player_2_color,
        .max_len = 0,
    },
    {
        .question = "Pollinations App Key (pk_): ",
        .type = STRING_OPTION,
        .value = pollinations_app_key,
        .max_len = 255,
    },
    {
        .question = "  Authorize with Pollinations  ",
        .type = BUTTON_OPTION,
        .value = authorize_pollinations,
        .max_len = 0,
    },
    {
        .question = "Pollinations User Key (sk_): ",
        .type = STRING_OPTION,
        .value = pollinations_user_key,
        .max_len = 255,
    },
    {
        .question = "  Start Game  ",
        .type = BUTTON_OPTION,
        .value = game,
        .max_len = 0,
    },
    {
        .question = "  Start Game VS AI  ",
        .type = BUTTON_OPTION,
        .value = game_with_ai,
        .max_len = 0,
    },
    {
        .question = NULL,
        .type = CHAR_OPTION,
        .value = NULL,
        .max_len = 0,
    },
};

Option* get_options() {
    return opts;
}

#include <curl/curl.h>
#include <jansson.h>

void set_conf(Config *conf, const char *title, void* any) {
    if (strcmp(title, opts[0].question) == 0) {
        conf->player_1_symbol = *(char*)any;
        return;
    }
    if (strcmp(title, opts[1].question) == 0) {
        unsigned short *color = (unsigned short*)any;
        conf->player_1_color[0] = color[0];
        conf->player_1_color[1] = color[1];
        conf->player_1_color[2] = color[2];
        return;
    }
    if (strcmp(title, opts[2].question) == 0) {
        conf->player_2_symbol = *(char*)any;
        return;
    }
    if (strcmp(title, opts[3].question) == 0) {
        unsigned short *color = (unsigned short*)any;
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
    if (strcmp(title, opts[6].question) == 0) {
        strncpy(conf->pollinations_user_key, (char*)any, 255);
        conf->pollinations_user_key[255] = '\0';
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

// Validate App Key format (should start with pk_)
bool validate_app_key(const char *key) {
    return (key && strlen(key) >= 3 && strncmp(key, "pk_", 3) == 0);
}

// Validate User Key format (should start with sk_)
bool validate_user_key(const char *key) {
    return (key && strlen(key) >= 3 && strncmp(key, "sk_", 3) == 0);
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

    if (!validate_app_key(conf.pollinations_app_key)) {
        printf("\n\x1b[31mError: La App Key debe comenzar con 'pk_'\x1b[0m\n");
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
        printf("  Verifique su conexión a internet y que la App Key sea válida.\n");
        printf("Presione cualquier tecla para continuar...");
        fflush(stdout);
        char dummy;
        read(STDIN_FILENO, &dummy, 1);
        return RETURN_MENU_CODE;
    }

    printf("\n\x1b[32m¡Código obtenido!\x1b[0m\n");
    printf("\n\x1b[33m┌─ Pasos para autorizar ─────────────────────────────┐\x1b[0m\n");
    printf("\x1b[33m│\x1b[0m 1. Abra su navegador en:\n");
    printf("\x1b[33m│\x1b[0m    \x1b[4m%s\x1b[0m\n", verification_uri_complete);
    printf("\x1b[33m│\x1b[0m 2. Ingrese el código de usuario: \x1b[1;36m%s\x1b[0m\n", user_code);
    printf("\x1b[33m│\x1b[0m 3. Autorice la aplicación en la página web\n");
    printf("\x1b[33m└────────────────────────────────────────────────────┘\x1b[0m\n");
    printf("\n\x1b[90mEsperando autorización (expira en %d segundos)...\x1b[0m\n", expires_in);
    fflush(stdout);

    char access_token[256] = "";
    time_t start_time = time(NULL);
    bool authorized = false;
    const char spinner[] = {'|', '/', '-', '\\'};
    int spinner_idx = 0;

    while (time(NULL) - start_time < expires_in) {
        long elapsed = time(NULL) - start_time;
        long remaining = expires_in - elapsed;
        
        printf("\r\x1b[K\x1b[90m[%c] Consultando... (%lds / %ds restantes)\x1b[0m", 
               spinner[spinner_idx], elapsed, remaining);
        fflush(stdout);
        spinner_idx = (spinner_idx + 1) % 4;

        if (pollinations_poll_token(device_code, access_token)) {
            authorized = true;
            break;
        }

        sleep(interval);
    }

    printf("\r\x1b[K"); // Clear the spinner line

    if (authorized) {
        if (!validate_user_key(access_token)) {
            printf("\n\x1b[33mAdvertencia: La clave recibida no tiene el formato esperado (sk_...)\x1b[0m\n");
        }
        
        printf("\n\x1b[32m✓ ¡Autorización exitosa!\x1b[0m\n");
        printf("  Su clave de usuario (\x1b[1msk_...\x1b[0m) ha sido guardada.\n");
        
        // Update the global variable and the opts array
        strncpy(pollinations_user_key, access_token, 255);
        pollinations_user_key[255] = '\0';
        strncpy((char*)opts[7].value, access_token, 255);
        ((char*)opts[7].value)[255] = '\0';
    } else {
        printf("\n\x1b[31m✗ Tiempo agotado o error en la autorización\x1b[0m\n");
        printf("  La autorización expiró después de %d segundos.\n", expires_in);
    }

    printf("\nPresione cualquier tecla para continuar...");
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
            printf("\n%s", question);
            fflush(stdout);
            read(STDIN_FILENO, &c, 1);
            *(char*)opt.value = c;
            set_conf(conf, question, opt.value);
            break;
        case STRING_OPTION:
            {
                char input[256] = "";
                printf("\n%s", question);
                fflush(stdout);
                int n = read(STDIN_FILENO, input, opt.max_len);
                if (n > 0) {
                    input[n] = '\0';
                    // Remove trailing newline if present
                    if (input[n-1] == '\n') input[n-1] = '\0';
                    strncpy((char*)opt.value, input, opt.max_len);
                    ((char*)opt.value)[opt.max_len] = '\0';
                    set_conf(conf, question, opt.value);
                }
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

    // Auto-save config after any change (except buttons)
    if (opt.type != BUTTON_OPTION) {
        config_save(conf, NULL);
    }

    return RETURN_MENU_CODE;
}

// Config file path
static char config_path[512] = "";

const char* config_get_path(void) {
    if (config_path[0] == '\0') {
        const char *home = getenv("HOME");
        if (home) {
            snprintf(config_path, sizeof(config_path), "%s/.config/tictactoe/config.json", home);
        } else {
            snprintf(config_path, sizeof(config_path), "/tmp/tictactoe_config.json");
        }
    }
    return config_path;
}

// Ensure config directory exists
static bool config_ensure_dir(void) {
    const char *path = config_get_path();
    char dir[512];
    strncpy(dir, path, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';
    
    // Find last slash
    char *last_slash = strrchr(dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        
        // Create directory recursively
        char *p = dir;
        while ((p = strchr(p + 1, '/')) != NULL) {
            *p = '\0';
            mkdir(dir, 0755);
            *p = '/';
        }
        mkdir(dir, 0755);
    }
    return true;
}

bool config_save(const Config *conf, const char *path) {
    if (!conf) return false;
    
    const char *save_path = path ? path : config_get_path();
    config_ensure_dir();
    
    FILE *f = fopen(save_path, "w");
    if (!f) {
        fprintf(stderr, "Error al abrir archivo de configuración para escritura: %s\n", save_path);
        return false;
    }
    
    fprintf(f, "{\n");
    fprintf(f, "  \"player_1_symbol\": \"%c\",\n", conf->player_1_symbol);
    fprintf(f, "  \"player_2_symbol\": \"%c\",\n", conf->player_2_symbol);
    fprintf(f, "  \"player_1_color\": [%u, %u, %u],\n", 
            conf->player_1_color[0], conf->player_1_color[1], conf->player_1_color[2]);
    fprintf(f, "  \"player_2_color\": [%u, %u, %u],\n", 
            conf->player_2_color[0], conf->player_2_color[1], conf->player_2_color[2]);
    fprintf(f, "  \"pollinations_app_key\": \"%s\",\n", conf->pollinations_app_key);
    fprintf(f, "  \"pollinations_user_key\": \"%s\"\n", conf->pollinations_user_key);
    fprintf(f, "}\n");
    
    fclose(f);
    return true;
}

bool config_load(Config *conf, const char *path) {
    if (!conf) return false;
    
    const char *load_path = path ? path : config_get_path();
    
    FILE *f = fopen(load_path, "r");
    if (!f) {
        // Config file doesn't exist, use defaults
        return false;
    }
    
    // Read entire file
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (fsize <= 0 || fsize > 10240) {
        fclose(f);
        return false;
    }
    
    char *buffer = malloc(fsize + 1);
    if (!buffer) {
        fclose(f);
        return false;
    }
    
    size_t read = fread(buffer, 1, fsize, f);
    buffer[read] = '\0';
    fclose(f);
    
    // Parse JSON with jansson
    json_error_t error;
    json_t *root = json_loads(buffer, 0, &error);
    free(buffer);
    
    if (!root) {
        fprintf(stderr, "Error al parsear config.json: %s\n", error.text);
        return false;
    }
    
    json_t *j;
    
    j = json_object_get(root, "player_1_symbol");
    if (j && json_is_string(j)) {
        const char *s = json_string_value(j);
        if (s && strlen(s) > 0) conf->player_1_symbol = s[0];
    }
    
    j = json_object_get(root, "player_2_symbol");
    if (j && json_is_string(j)) {
        const char *s = json_string_value(j);
        if (s && strlen(s) > 0) conf->player_2_symbol = s[0];
    }
    
    j = json_object_get(root, "player_1_color");
    if (j && json_is_array(j) && json_array_size(j) == 3) {
        for (int i = 0; i < 3; i++) {
            json_t *v = json_array_get(j, i);
            if (v && json_is_integer(v)) {
                conf->player_1_color[i] = (unsigned short)json_integer_value(v);
            }
        }
    }
    
    j = json_object_get(root, "player_2_color");
    if (j && json_is_array(j) && json_array_size(j) == 3) {
        for (int i = 0; i < 3; i++) {
            json_t *v = json_array_get(j, i);
            if (v && json_is_integer(v)) {
                conf->player_2_color[i] = (unsigned short)json_integer_value(v);
            }
        }
    }
    
    j = json_object_get(root, "pollinations_app_key");
    if (j && json_is_string(j)) {
        strncpy(conf->pollinations_app_key, json_string_value(j), 255);
        conf->pollinations_app_key[255] = '\0';
    }
    
    j = json_object_get(root, "pollinations_user_key");
    if (j && json_is_string(j)) {
        strncpy(conf->pollinations_user_key, json_string_value(j), 255);
        conf->pollinations_user_key[255] = '\0';
    }
    
    json_decref(root);
    return true;
}
