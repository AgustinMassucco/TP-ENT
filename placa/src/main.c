#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "globals.h"
#include "configuracion.h"
#include "servidor.h"
#include "consola.h"
#include "memoria_instrucciones.h"

/**
 * Placa: ./bin/placa [Archivo Config]
 *
 * Hilos:
 *   - principal : inicializa todo y corre la consola por stdin (INFO / TLS)
 *   - aceptador : acepta conexiones de Planificador y Cores, y lanza un hilo por cliente
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s [Archivo Config]\n", argv[0]);
        return EXIT_FAILURE;
    }

    placa_config = configuracion_cargar(argv[1]);
    if (placa_config == NULL) {
        return EXIT_FAILURE;
    }

    logger = log_create("placa.log", "PLACA", true, placa_config->log_level);
    if (logger == NULL) {
        fprintf(stderr, "No se pudo crear el logger\n");
        configuracion_destruir(placa_config);
        return EXIT_FAILURE;
    }

    log_info(logger, "Iniciando Placa - Memoria: %u bytes - Página: %u bytes - Reemplazo: %s",
             placa_config->tam_memoria, placa_config->tam_pagina,
             configuracion_algoritmo_as_string(placa_config->algoritmo_reemplazo));

    memoria_instrucciones_init();

    if (servidor_iniciar() != 0) {
        memoria_instrucciones_destroy();
        log_destroy(logger);
        configuracion_destruir(placa_config);
        return EXIT_FAILURE;
    }

    pthread_t hilo_aceptador;
    if (pthread_create(&hilo_aceptador, NULL, servidor_aceptar_conexiones, NULL) != 0) {
        log_error(logger, "No se pudo crear el hilo aceptador");
        memoria_instrucciones_destroy();
        log_destroy(logger);
        configuracion_destruir(placa_config);
        return EXIT_FAILURE;
    }
    pthread_detach(hilo_aceptador);

    // La consola corre en el hilo principal: cuando termina (EXIT o EOF) se apaga la Placa
    consola_ejecutar(NULL);

    log_info(logger, "Finalizando Placa");
    memoria_instrucciones_destroy();
    log_destroy(logger);
    configuracion_destruir(placa_config);
    return EXIT_SUCCESS;
}
