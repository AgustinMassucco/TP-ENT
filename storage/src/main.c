#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "globals.h"
#include "configuracion.h"
#include "servidor.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s [Archivo Config]\n", argv[0]);
        return EXIT_FAILURE;
    }

    storage_config = configuracion_cargar(argv[1]);
    if (storage_config == NULL) {
        return EXIT_FAILURE;
    }

    logger = log_create("storage.log", "STORAGE", true, storage_config->log_level);
    if (logger == NULL) {
        fprintf(stderr, "No se pudo crear el logger\n");
        configuracion_destruir(storage_config);
        return EXIT_FAILURE;
    }

    log_info(logger, "Iniciando Storage - Bloques: %u de %u bytes - Directorio: %u bloques - Path: %s",
             storage_config->cant_bloques, storage_config->tam_bloque,
             storage_config->bloques_directorio, storage_config->path_storage);

    if (servidor_iniciar() != 0) {
        log_destroy(logger);
        configuracion_destruir(storage_config);
        return EXIT_FAILURE;
    }

    pthread_t hilo_aceptador;
    if (pthread_create(&hilo_aceptador, NULL, servidor_aceptar_conexiones, NULL) != 0) {
        log_error(logger, "No se pudo crear el hilo aceptador");
        log_destroy(logger);
        configuracion_destruir(storage_config);
        return EXIT_FAILURE;
    }
    pthread_join(hilo_aceptador, NULL);

    log_info(logger, "Finalizando Storage");
    log_destroy(logger);
    configuracion_destruir(storage_config);
    return EXIT_SUCCESS;
}
