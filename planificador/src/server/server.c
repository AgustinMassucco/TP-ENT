/*Servidor multihilo para peticiones de los CORES */
#include "server.h"

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "utils/sockets/server.h"
#include "utils/messaging/packet.h"
#include "utils/messaging/codes.h"

typedef struct {
    int fd;
    t_log* logger;
} t_core_conn_args;

/**
 * @brief Funcion ejecutada en el hilo dedicado a un Core.
 *        Valida el handshake inicial, loguea la conexion y luego queda
 *        a la espera de futuras operaciones hasta que el Core se desconecte.
 * @param arg Puntero a un t_core_conn_args (liberado dentro de la funcion).
 */
static void* atender_core(void* arg) {
    t_core_conn_args* args = (t_core_conn_args*) arg;
    int fd = args->fd;
    t_log* logger = args->logger;
    free(args);

    int op_code = receive_operation(fd);
    if (op_code != HANDSHAKE_CORE) {
        log_error(logger, "Handshake invalido de un Core (op_code recibido: %d)", op_code);
        send_error(fd, "Handshake invalido, se esperaba HANDSHAKE_CORE");
        close(fd);
        return NULL;
    }

    log_info(logger, "## Modulo: CORE");
    send_ack(fd);

    /* Se mantiene la conexion abierta a la espera de futuras operaciones
     * (syscalls, page fault, etc.). Esa logica se implementa en checkpoints
     * posteriores; por ahora solo se detecta la desconexion del Core. */
    while (receive_operation(fd) != -1) {
        /* TODO: despachar operaciones del Core (C2+) */
    }

    log_info(logger, "Se desconecto un Core");
    close(fd);
    return NULL;
}

void start_core_server(t_config* config, t_log* logger) {
    int puerto = config_get_int_value(config, "PUERTO_ESCUCHA");

    int server_fd = create_server(puerto);
    if (server_fd == -1) {
        log_error(logger, "No se pudo iniciar el servidor de Cores en el puerto %d", puerto);
        return;
    }

    log_info(logger, "Servidor de Cores escuchando en el puerto %d", puerto);

    while (1) {
        int client_fd = accept_connection(server_fd);
        if (client_fd == -1) {
            log_error(logger, "Error al aceptar una conexion entrante de un Core");
            continue;
        }

        t_core_conn_args* args = malloc(sizeof(t_core_conn_args));
        if (args == NULL) {
            log_error(logger, "No se pudo reservar memoria para atender la conexion");
            close(client_fd);
            continue;
        }
        args->fd = client_fd;
        args->logger = logger;

        pthread_t thread;
        if (pthread_create(&thread, NULL, atender_core, args) != 0) {
            log_error(logger, "No se pudo crear el hilo para atender la conexion");
            free(args);
            close(client_fd);
            continue;
        }
        pthread_detach(thread);
    }
}
