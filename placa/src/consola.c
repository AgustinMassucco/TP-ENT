#include "consola.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>
#include "globals.h"
#include "memoria_instrucciones.h"

static void comando_info(void) {
    // TODO (checkpoint 3): tomar estos valores de la memoria de usuario y del Offload
    uint32_t frames_totales   = placa_config->tam_memoria / placa_config->tam_pagina;
    uint32_t frames_libres    = frames_totales;
    uint32_t frames_lockeados = 0;
    uint32_t paginas_offload  = placa_config->tam_offload / placa_config->tam_pagina;
    uint32_t offload_ocupado  = 0;

    printf("=== INFO ===\n");
    printf("Memoria de usuario : %u bytes (%u frames de %u bytes)\n",
           placa_config->tam_memoria, frames_totales, placa_config->tam_pagina);
    printf("  Ocupada          : %.2f%%\n", 100.0 * (frames_totales - frames_libres) / frames_totales);
    printf("  Frames libres    : %u\n", frames_libres);
    printf("  Frames lockeados : %u\n", frames_lockeados);
    printf("  Frames totales   : %u\n", frames_totales);
    printf("Offload            : %u bytes (%u páginas)\n", placa_config->tam_offload, paginas_offload);
    printf("  Ocupado          : %.2f%%\n", 100.0 * offload_ocupado / paginas_offload);
    printf("Algoritmo reemplazo: %s\n", configuracion_algoritmo_as_string(placa_config->algoritmo_reemplazo));
    printf("Jobs actuales      : %d\n", memoria_instrucciones_cantidad_jobs());
}

static void comando_tls(void) {
    t_list* jids = memoria_instrucciones_listar_jids();

    printf("=== TLS ===\n");
    if (list_is_empty(jids)) {
        printf("No hay Jobs registrados\n");
    } else {
        printf("%-8s %-12s %-12s %-14s\n", "JID", "Residentes", "Totales", "Instrucciones");
        for (int i = 0; i < list_size(jids); i++) {
            uint32_t jid = *(uint32_t*) list_get(jids, i);
            // TODO (checkpoint 3): páginas residentes y totales desde la tabla de páginas del Job
            printf("%-8u %-12u %-12u %-14d\n", jid, 0u, 0u, memoria_instrucciones_cantidad(jid));
        }
    }

    list_destroy_and_destroy_elements(jids, free);
}

static void comando_help(void) {
    printf("Comandos disponibles:\n");
    printf("  INFO  - estado de la memoria, el Offload y cantidad de Jobs\n");
    printf("  TLS   - lista de Jobs con sus páginas residentes/totales\n");
    printf("  HELP  - esta ayuda\n");
    printf("  EXIT  - finaliza la Placa\n");
}

/**
 * @return true si la consola debe seguir leyendo comandos, false para terminar.
 */
static bool ejecutar_comando(char* linea) {
    if (string_is_empty(linea)) {
        return true;
    }

    add_history(linea);
    log_debug(logger, "Consola: comando '%s'", linea);

    if (string_equals_ignore_case(linea, "INFO")) {
        comando_info();
    } else if (string_equals_ignore_case(linea, "TLS")) {
        comando_tls();
    } else if (string_equals_ignore_case(linea, "HELP")) {
        comando_help();
    } else if (string_equals_ignore_case(linea, "EXIT")) {
        return false;
    } else {
        printf("Comando desconocido: %s (HELP para ver los comandos)\n", linea);
    }
    return true;
}

void* consola_ejecutar(void* arg) {
    char* linea;
    while ((linea = readline("placa> ")) != NULL) {
        // string_trim realoca: se hace acá para que el free() de abajo use el puntero vigente
        string_trim(&linea);
        bool continuar = ejecutar_comando(linea);
        free(linea);
        if (!continuar) {
            break;
        }
    }
    return NULL;
}
