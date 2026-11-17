#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "ai_module.h"

#include <stdatomic.h>

// Model configuration - using a current Pollinations model
#define POLLINATIONS_MODEL "openai/gpt-5.4-nano"
#define POLLINATIONS_END_POINT "https://gen.pollinations.ai/v1/chat/completions"
#define REQUEST_TIMEOUT_MS 30000
#define MAX_RESPONSE_SIZE (1024 * 1024)  // 1MB max response

// Estructura para acumular la respuesta en memoria
struct MemoryStruct {
    char *memory;
    size_t size;
    size_t max_size;
};

// Función callback para escribir la respuesta en memoria
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    if (mem->size + realsize + 1 > mem->max_size) {
        fprintf(stderr, "Respuesta demasiado grande, truncando\n");
        return 0;  // Signal error by returning 0
    }

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

void set_repr(Board board, Config conf, char *json_data, unsigned int x, unsigned int y) {
    char *board_ref[3][3] = {
        { json_data + 183, json_data + 187, json_data + 191 },
        { json_data + 211, json_data + 215, json_data + 219 },
        { json_data + 239, json_data + 243, json_data + 247 },
    };

    char c = board.board[x][y];
    if (c == INIT_CELL)
        *board_ref[x][y] = ' ';
    else if (c == conf.player_1_symbol)
        *board_ref[x][y] = 'x';
    else if (c == conf.player_2_symbol)
        *board_ref[x][y] = 'y';
}

atomic_int end_thinking = ATOMIC_VAR_INIT(0);

void finish_think() {
    atomic_exchange(&end_thinking, 1);
}

void start_think() {
    atomic_exchange(&end_thinking, 0);
}

bool finished_thinking() {
    return atomic_load(&end_thinking);
}

// Cleanup helper function
static void cleanup_curl(CURL *curl, struct curl_slist *headers, struct MemoryStruct *chunk) {
    if (headers) curl_slist_free_all(headers);
    if (curl) curl_easy_cleanup(curl);
    if (chunk && chunk->memory) free(chunk->memory);
    curl_global_cleanup();
}

void* gemini_decide(void *args) {
    start_think();

    CONTEXT context = *(CONTEXT*)args;
    Board board = context.board;
    Config conf = context.conf;

    // Validate API key
    if (context.api_key == NULL || strlen(context.api_key) == 0) {
        fprintf(stderr, "Error: No hay API key configurada\n");
        free(args);
        finish_think();
        return NULL;
    }

    // Build JSON request using jansson for proper escaping
    json_t *root = json_object();
    json_object_set_new(root, "model", json_string(POLLINATIONS_MODEL));
    json_object_set_new(root, "json", json_true());
    
    json_t *messages = json_array();
    json_t *user_msg = json_object();
    json_object_set_new(user_msg, "role", json_string("user"));
    
    // Build board representation string
    char board_str[512];
    snprintf(board_str, sizeof(board_str),
        "Has el mejor movimiento de este 3 en raya, es turno de las %c:\n"
        " %c │ %c │ %c \n"
        "───┼───┼───\n"
        " %c │ %c │ %c \n"
        "───┼───┼───\n"
        " %c │ %c │ %c \n"
        "Tu respuesta deberá ser en formato JSON, solo responde con tu respuesta en JSON, nada más que eso. "
        "Donde 'x', y 'y', son las coordenadas del movimiento que eliges, recordando que las coordenadas "
        "inician desde la esquina superior izquierda, siendo el punto (0, 0) y terminando en la esquina "
        "inferior derecha con el punto (2, 2). El formato de respuesta es: {\"x\": 0, \"y\": 0}",
        board.x_turn ? 'o' : 'x',
        board.board[0][0] == INIT_CELL ? ' ' : board.board[0][0],
        board.board[1][0] == INIT_CELL ? ' ' : board.board[1][0],
        board.board[2][0] == INIT_CELL ? ' ' : board.board[2][0],
        board.board[0][1] == INIT_CELL ? ' ' : board.board[0][1],
        board.board[1][1] == INIT_CELL ? ' ' : board.board[1][1],
        board.board[2][1] == INIT_CELL ? ' ' : board.board[2][1],
        board.board[0][2] == INIT_CELL ? ' ' : board.board[0][2],
        board.board[1][2] == INIT_CELL ? ' ' : board.board[1][2],
        board.board[2][2] == INIT_CELL ? ' ' : board.board[2][2]
    );
    
    json_object_set_new(user_msg, "content", json_string(board_str));
    json_array_append_new(messages, user_msg);
    json_object_set_new(root, "messages", messages);

    char *json_data = json_dumps(root, JSON_COMPACT);
    json_decref(root);

    if (!json_data) {
        fprintf(stderr, "Error al serializar JSON\n");
        free(args);
        finish_think();
        return NULL;
    }

    MOVE *move = (MOVE*) malloc(sizeof(MOVE));
    if (move == NULL) {
        free(json_data);
        free(args);
        finish_think();
        return NULL;
    }

    *move = (MOVE) { .x = 0, .y = 0 };

    CURL *curl = NULL;
    CURLcode res;
    struct curl_slist *headers = NULL;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0, .max_size = MAX_RESPONSE_SIZE };
    
    if (!chunk.memory) {
        fprintf(stderr, "Error al asignar memoria para respuesta\n");
        free(json_data);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }
    chunk.memory[0] = 0;

    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error al inicializar curl\n");
        free(json_data);
        free(chunk.memory);
        free(move);
        free(args);
        curl_global_cleanup();
        finish_think();
        return NULL;
    }

    // Build Authorization header
    char Authorization[512];
    snprintf(Authorization, sizeof(Authorization), "Authorization: Bearer %s", context.api_key);

    // Configurar la URL
    curl_easy_setopt(curl, CURLOPT_URL, POLLINATIONS_END_POINT);

    // Configurar los headers
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, Authorization);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Configurar para hacer petición POST con el JSON
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

    // Configurar el callback para capturar la respuesta
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Configurar timeouts
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, REQUEST_TIMEOUT_MS);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 10000);

    // Ejecutar la petición
    res = curl_easy_perform(curl);
    
    free(json_data);  // No longer needed

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() fallo: %s\n", curl_easy_strerror(res));
        cleanup_curl(curl, headers, &chunk);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }

    // Procesar la respuesta JSON con jansson
    json_error_t error;
    json_t *response_root = json_loads(chunk.memory, 0, &error);
    cleanup_curl(curl, headers, &chunk);

    if (!response_root) {
        fprintf(stderr, "Error al parsear JSON en la linea %d: %s\n", error.line, error.text);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }

    // Extraer: choices[0].message.content
    json_t *ai_choices = json_object_get(response_root, "choices");
    if (!ai_choices) {
        fprintf(stderr, "No se encontró 'choices' en la respuesta\n");
        json_decref(response_root);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }

    json_t *candidate = json_array_get(ai_choices, 0);
    if (!candidate) {
        fprintf(stderr, "No se encontró ninguna respuesta generada\n");
        json_decref(response_root);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }

    json_t *message = json_object_get(candidate, "message");
    if (!message) {
        fprintf(stderr, "No se encontró el mensaje de respuesta\n");
        json_decref(response_root);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }

    json_t *content = json_object_get(message, "content");
    if (!content || !json_is_string(content)) {
        fprintf(stderr, "No se pudo acceder al contenido del mensaje\n");
        json_decref(response_root);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }

    const char *resultText = json_string_value(content);

    // Parse the inner JSON (the move coordinates)
    json_error_t inner_error;
    json_t *res_root = json_loads(resultText, 0, &inner_error);
    json_decref(response_root);

    if (!res_root) {
        fprintf(stderr, "Error al parsear JSON del movimiento en la linea %d: %s\n", inner_error.line, inner_error.text);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }

    json_t *jnx = json_object_get(res_root, "x");
    json_t *jny = json_object_get(res_root, "y");

    if (!jnx || !jny || !json_is_number(jnx) || !json_is_number(jny)) {
        fprintf(stderr, "Formato de respuesta inválido: faltan coordenadas x,y\n");
        json_decref(res_root);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }

    double x_val = json_number_value(jnx);
    double y_val = json_number_value(jny);
    json_decref(res_root);

    // Validate coordinates
    if (x_val < 0 || x_val > 2 || y_val < 0 || y_val > 2) {
        fprintf(stderr, "Coordenadas fuera de rango: x=%.0f, y=%.0f\n", x_val, y_val);
        free(move);
        free(args);
        finish_think();
        return NULL;
    }

    move->x = (unsigned int)x_val;
    move->y = (unsigned int)y_val;

    free(args);
    finish_think();
    return move;
}