#include "servidor.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <utils/sockets/server.h>
#include <utils/messaging/packet.h>
#include "globals.h"
#include "operaciones.h"

static int server_fd = -1;

int servidor_iniciar(void) {
    server_fd = create_server(storage_config->puerto_escucha);
    if (server_fd < 0) {
        log_error(logger, "No se pudo crear el servidor en el puerto %d", storage_config->puerto_escucha);
        return -1;
    }
    log_info(logger, "Servidor escuchando en el puerto %d", storage_config->puerto_escucha);
    return 0;
}

static const char* realizar_handshake(int fd) {
    int op_code = receive_operation(fd);
    if (op_code < 0) {
        return NULL;
    }

    int   size    = 0;
    void* payload = receive_buffer(&size, fd);
    free(payload);

    if (op_code != HANDSHAKE_PLANIFICADOR) {
        log_error(logger, "Handshake inválido (op_code %d) en socket %d", op_code, fd);
        send_error(fd, "Handshake inválido: el Storage solo acepta al PLANIFICADOR");
        return NULL;
    }

    if (send_ack(fd) != 0) {
        log_error(logger, "No se pudo responder el handshake en socket %d", fd);
        return NULL;
    }

    log_info(logger, "## Módulo: PLANIFICADOR");
    log_debug(logger, "PLANIFICADOR conectado en socket %d", fd);
    return "PLANIFICADOR";
}

static void* atender_cliente(void* arg) {
    int fd = *(int*) arg;
    free(arg);

    const char* nombre = realizar_handshake(fd);
    if (nombre == NULL) {
        close(fd);
        return NULL;
    }

    while (true) {
        int op_code = receive_operation(fd);
        if (op_code < 0) {
            break;
        }
        if (operaciones_atender(fd, op_code, nombre) != 0) {
            break;
        }
    }

    log_info(logger, "Se desconectó %s (socket %d)", nombre, fd);
    close(fd);
    return NULL;
}

void* servidor_aceptar_conexiones(void* arg) {
    while (true) {
        int cliente_fd = accept_connection(server_fd);
        if (cliente_fd < 0) {
            log_error(logger, "Error al aceptar una conexión");
            continue;
        }

        int* fd_arg = malloc(sizeof(int));
        *fd_arg = cliente_fd;

        pthread_t hilo;
        if (pthread_create(&hilo, NULL, atender_cliente, fd_arg) != 0) {
            log_error(logger, "No se pudo crear el hilo para el socket %d", cliente_fd);
            free(fd_arg);
            close(cliente_fd);
            continue;
        }
        pthread_detach(hilo);
    }
    return NULL;
}
