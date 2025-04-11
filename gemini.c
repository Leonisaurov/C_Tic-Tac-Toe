#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "gemini.h"

#include <stdatomic.h>

// #define MODEL_ID "gemini-2.0-flash"
#define MODEL_ID "gemini-2.0-flash-thinking-exp-01-21"
#define GENERATE_CONTENT_API "streamGenerateContent"

// Estructura para acumular la respuesta en memoria
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Función callback para escribir la respuesta en memoria
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        // Error de asignación
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
        { json_data + 167, json_data + 171, json_data + 175 },
        { json_data + 195, json_data + 199, json_data + 203 },
        { json_data + 223, json_data + 227, json_data + 231 },
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

void* gemini_decide(void *args) {
    start_think();

    CONTEXT context = *(CONTEXT*)args;
    Board board = context.board;
    Config conf = context.conf;

    char json_data[] =
        "{\n"
        "    \"contents\": [\n"
        "      {\n"
        "        \"role\": \"user\",\n"
        "        \"parts\": [\n"
        "          {\n"
        "            \"text\": \"Has el mejor movimiento de este 3 en raya, es turno de las o:\\n"
                                " │ │ \\n"
                                "─┼─┼─\\n"
                                " │ │ \\n"
                                "─┼─┼─\\n"
                                " │ │ \"\n"
        "          }\n"
        "        ]\n"
        "      }\n"
        "    ],\n"
        "    \"systemInstruction\": {\n"
        "      \"parts\": [\n"
        "        {\n"
        "            \"text\": \"Se consiso, solo responde con las cordenadas del mejor movimiento, de modo: 'x,y'\"\n"
        "        }\n"
        "      ]\n"
        "    },\n"
        "    \"generationConfig\": {\n"
        "      \"responseMimeType\": \"text/plain\",\n"
        "      \"temperature\": 0.00,\n"
        "    }\n"
        "}\n";

    set_repr(board, conf, json_data, 0, 0);
    set_repr(board, conf, json_data, 1, 0);
    set_repr(board, conf, json_data, 2, 0);
    
    set_repr(board, conf, json_data, 0, 1);
    set_repr(board, conf, json_data, 1, 1);
    set_repr(board, conf, json_data, 2, 1);
    
    set_repr(board, conf, json_data, 0, 2);
    set_repr(board, conf, json_data, 1, 2);
    set_repr(board, conf, json_data, 2, 2);

    json_data[163] = (board.x_turn?'x':'o');

    char *temp = json_data + 543;

    srand(time(NULL));
    *temp = (rand() % 9) + '0';

    srand(time(NULL) * (*temp));
    temp[1] = (rand() % 9) + '0';

    MOVE *move = (MOVE*) malloc(sizeof(MOVE));
    if (move == NULL) return NULL;

    *move = (MOVE) {
        .x = 0,
        .y = 0,
    };

    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);  // Será reallocado en el callback
    chunk.size = 0;

    // Construir la URL
    char url[512];
    snprintf(url, sizeof(url),
             "https://generativelanguage.googleapis.com/v1beta/models/%s:%s?key=%s",
             MODEL_ID, GENERATE_CONTENT_API, getenv("GEMINI_API_KEY"));

    // Cuerpo JSON de la petición (igual que en el script bash)
   
    // Inicializar curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        // Configurar la URL
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Configurar los headers
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Configurar para hacer petición POST con el JSON
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

        // Configurar el callback para capturar la respuesta
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Ejecutar la petición
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() fallo: %s\n", curl_easy_strerror(res));
        } else {
            // Imprimir la respuesta completa (opcional)
            // fprintf(stderr, "Respuesta: %s\n", chunk.memory);

            // Procesar la respuesta JSON con jansson
            json_error_t error;
            json_t *root = json_loads(chunk.memory, 0, &error);
            if(!root) {
                fprintf(stderr, "Error al parsear JSON en la linea %d: %s\n", error.line, error.text);
            } else {
                // Asumiendo que la estructura es un array y queremos extraer:
                // [1].candidates.[0].content.parts.[0].text
                json_t *element = json_array_get(root, 0);
                if(element) {
                    // Extraer "candidates"
                    json_t *candidates = json_object_get(element, "candidates");
                    if(candidates && json_is_array(candidates)) {
                        json_t *candidate0 = json_array_get(candidates, 0);
                        if(candidate0) {
                            json_t *content = json_object_get(candidate0, "content");
                            if(content) {
                                json_t *parts = json_object_get(content, "parts");
                                if(parts && json_is_array(parts)) {
                                    json_t *part0 = json_array_get(parts, 0);
                                    if(part0) {
                                        json_t *text = json_object_get(part0, "text");
                                        if(text && json_is_string(text)) {
                                            const char *resultText = json_string_value(text);
                                            unsigned int n1, n2;
                                            if (sscanf(resultText, "%u,%u", &n1, &n2) == 2) {
                                                json_decref(root);
                                                curl_slist_free_all(headers);
                                                curl_easy_cleanup(curl);
                                                free(chunk.memory);
                                                curl_global_cleanup();

                                                move->x = n1;
                                                move->y = n2;

                                                free(args);
                                                finish_think();
                                                return move;
                                            }
                                        } else {
                                            fprintf(stderr, "No se encontró la clave 'text' o no es de tipo string.\n");
                                        }
                                    } else {
                                        fprintf(stderr, "No se encontró el primer elemento en 'parts'.\n");
                                    }
                                } else {
                                    fprintf(stderr, "No se encontró la clave 'parts' o no es de un array.\n");
                                }
                            } else {
                                fprintf(stderr, "No se encontró la clave 'content'.\n");
                            }
                        } else {
                            fprintf(stderr, "No se encontró el primer candidato en 'candidates'.\n");
                        }
                    } else {
                        fprintf(stderr, "No se encontró la clave 'candidates' o no es de un array.\n");
                    }
                } else {
                    fprintf(stderr, "No se encontró el elemento en la posición 1 del array raíz.\n");
                }
                json_decref(root);
            }
        }
        // Limpieza
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Error al inicializar curl\n");
    }
    free(chunk.memory);
    curl_global_cleanup();

    free(args);
    finish_think();
    return move;
}
