#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <commons/string.h>
#include <utils/messaging/packet.h>

#include "globals.h"
#include "configuracion.h"
#include "conexiones.h"

static void finalizar_core(void) {
    if (fd_planificador != -1) close(fd_planificador);
    if (fd_placa != -1) close(fd_placa);
    if (logger != NULL) log_destroy(logger);
    configuracion_destruir(core_config);
}

/**
 * Queda a la espera de mensajes del Planificador hasta que este se desconecte.
 * TODO (checkpoint 2): recibir JID + contexto de ejecución y correr el ciclo de instrucción.
 */
static void esperar_jobs(void) {
    while (true) {
        int op_code = receive_operation(fd_planificador);
        if (op_code < 0) {
            break;
        }

        // Se consume el payload completo para mantener alineado el stream del socket
        t_list* campos = receive_packet(fd_planificador);
        if (campos == NULL) {
            break;
        }
        log_debug(logger, "Operación %d recibida del Planificador (%d campos), aún no implementada",
                  op_code, list_size(campos));
        list_destroy_and_destroy_elements(campos, free);
    }

    log_info(logger, "Se desconectó el Planificador");
}

/**
 * Core: ./bin/core [Archivo Config] [Identificador]
 */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s [Archivo Config] [Identificador]\n", argv[0]);
        return EXIT_FAILURE;
    }

    core_id = argv[2];

    core_config = configuracion_cargar(argv[1]);
    if (core_config == NULL) {
        return EXIT_FAILURE;
    }

    // Cada Core tiene su propio archivo de log identificado por su ID
    char* log_path    = string_from_format("core_%s.log", core_id);
    char* nombre_proc = string_from_format("CORE %s", core_id);
    logger = log_create(log_path, nombre_proc, true, core_config->log_level);
    free(log_path);
    free(nombre_proc);
    if (logger == NULL) {
        fprintf(stderr, "No se pudo crear el logger\n");
        finalizar_core();
        return EXIT_FAILURE;
    }

    log_info(logger, "Iniciando Core %s", core_id);

    fd_planificador = conectar_a_planificador();
    if (fd_planificador == -1) {
        finalizar_core();
        return EXIT_FAILURE;
    }

    fd_placa = conectar_a_placa();
    if (fd_placa == -1) {
        finalizar_core();
        return EXIT_FAILURE;
    }

    esperar_jobs();

    log_info(logger, "Finalizando Core %s", core_id);
    finalizar_core();
    return EXIT_SUCCESS;
}
