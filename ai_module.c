#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "ai_module.h"

#include <stdatomic.h>

// #define MODEL_ID "gemini-2.0-flash"
#define MODEL_ID "gemini-2.0-flash-thinking-exp-01-21"
#define GENERATE_CONTENT_API "streamGenerateContent"

#define POLLINATIONS_API_KEY "pk_SuqotQyyUiac3CFo"
#define POLLINATIONS_END_POINT "https://gen.pollinations.ai/v1/chat/completions"

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

void* gemini_decide(void *args) {
    start_think();

    CONTEXT context = *(CONTEXT*)args;
    Board board = context.board;
    Config conf = context.conf;
    // {
    //     "model": "nova-fast", 
    //     "json": true, 
    //     "messages": [
    //         {
    //             "role": "user", 
    //             "content": "Estoy pensando en un numero, tienes que adivinarlo, responde en formato JSON, de la siguiente forma: { 'choice': 'numero que hayas desidido'  }. No tiene que ser exacto, solo lo primero que puedas pensar, no digas mas, que la respuesta en JSON."
    //         }
    //     ]
    // }

    char json_data[] =
        "{\n"
        "   \"model\": \"nova-fast\",\n"
        "   \"json\": true,\n"
        "   \"messages\": [\n"
        // "       {\n"
        // "           \"role\": \"system\",\n"
        // "           \"content\": \"Se consiso, solo responde con las cordenadas del mejor movimiento, de modo: 'x,y'\"\n"
        // "       },\n"
        "       {\n"
        "           \"role\": \"user\",\n"
        "           \"content\": \"Has el mejor movimiento de este 3 en raya, es turno de las o:\\n"
                                " │ │ \\n"
                                "─┼─┼─\\n"
                                " │ │ \\n"
                                "─┼─┼─\\n"
                                " │ │ \\n"
                                "Tu respuesta debera de ser en formato JSON, solo responde con tu respuesta en JSON, nada mas que eso, asegurate de que el formato sea correcto. Donde 'x', y 'y', son las cordenadas de el movimiento que eliges, recordando que las cordenadas inician desde la esquina superior izquierda, siendo el punto (0, 0) y terminando en la esquina inferior derecha con el punto (2, 2), el formato de respuesta es el siguiente:\\n"
                                "{'x': 0, 'y': 0}\"\n"
        "       }\n"
        "   ]\n"
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

    json_data[179] = (board.x_turn?'x':'o');

    // fprintf(stderr, "%s", json_data);
    // return NULL;

    // char *temp = json_data + 543;
    //
    // srand(time(NULL));
    // *temp = (rand() % 9) + '0';
    //
    // srand(time(NULL) * (*temp));
    // temp[1] = (rand() % 9) + '0';

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
    // char url[512];
    // snprintf(url, sizeof(url),
    //          "https://generativelanguage.googleapis.com/v1beta/models/%s:%s?key=%s",
    //          MODEL_ID, GENERATE_CONTENT_API, getenv("GEMINI_API_KEY"));

    // Cuerpo JSON de la petición (igual que en el script bash)

    char Authorization[256];
    int n = snprintf(Authorization, sizeof(Authorization), "Authorization: Bearer %s", POLLINATIONS_API_KEY);
    Authorization[n] = '\0';
   
    // Inicializar curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
//         // Configurar la URL
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

        // Ejecutar la petición
        fprintf(stderr, "ANTES DE CURL");
        res = curl_easy_perform(curl);
        fprintf(stderr, "DESPUES DE CURL");
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() fallo: %s\n", curl_easy_strerror(res));
        } else {
            // Imprimir la respuesta completa (opcional)
            fprintf(stderr, "\nRespuesta: %s\n", chunk.memory);

            // Procesar la respuesta JSON con jansson
            json_error_t error;
            json_t *root = json_loads(chunk.memory, 0, &error);
            if(!root) {
                fprintf(stderr, "Error al parsear JSON en la linea %d: %s\n", error.line, error.text);
            } else {
                // Asumiendo que la estructura es un array y queremos extraer:
                // choices[0].message.content
                json_t *ai_choices = json_object_get(root, "choices");
                if(ai_choices) {
                    json_t *candidate = json_array_get(ai_choices, 0);
                    if(candidate) {
                        json_t *message = json_object_get(candidate, "message");
                        if(message) {
                            json_t *content = json_object_get(message, "content");
                            if(content && json_is_string(content)) {
                                const char *resultText = json_string_value(content);

                                json_error_t error;
                                json_t *res_root = json_loads(resultText, 0, &error);
                                if(!res_root) {
                                    fprintf(stderr, "Error al parsear JSON en la linea %d: %s\n", error.line, error.text);
                                } else {
                                    json_t *jnx = json_object_get(res_root, "x"), *jny = json_object_get(res_root, "y");

                                    move->x = json_number_value(jnx);
                                    move->y = json_number_value(jny);

                                    json_decref(root);
                                    json_decref(res_root);
                                    curl_slist_free_all(headers);
                                    curl_easy_cleanup(curl);
                                    free(chunk.memory);
                                    curl_global_cleanup();

                                    free(args);
                                    finish_think();
                                    return move;
                                }
                            } else {
                                fprintf(stderr, "No se pudo acceder al contenido del mensaje");
                            }
                        } else {
                            fprintf(stderr, "No se encontró el mensaje de respuesta");
                        }
                    } else {
                        fprintf(stderr, "No se encontró ninguna respuesta generada");
                    }
                } else {
                    fprintf(stderr, "No se encontró ninguna generacion de contenido.\n");
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
