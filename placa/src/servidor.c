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
    server_fd = create_server(placa_config->puerto_escucha);
    if (server_fd < 0) {
        log_error(logger, "No se pudo crear el servidor en el puerto %d", placa_config->puerto_escucha);
        return -1;
    }
    log_info(logger, "Servidor escuchando en el puerto %d", placa_config->puerto_escucha);
    return 0;
}

static const char* nombre_modulo_por_handshake(int op_code) {
    switch (op_code) {
        case HANDSHAKE_CORE:         return "CORE";
        case HANDSHAKE_PLANIFICADOR: return "PLANIFICADOR";
        default:                     return NULL;
    }
}

/**
 * Consume el handshake inicial del cliente y responde MSG_OK.
 * @return Nombre del módulo conectado, o NULL si el handshake es inválido.
 */
static const char* realizar_handshake(int fd) {
    int op_code = receive_operation(fd);
    if (op_code < 0) {
        return NULL;
    }

    // El handshake viaja como paquete: hay que consumir su payload (vacío o
    // con un identificador) para dejar el socket alineado con el próximo mensaje.
    int   size    = 0;
    void* payload = receive_buffer(&size, fd);
    free(payload);

    const char* nombre = nombre_modulo_por_handshake(op_code);
    if (nombre == NULL) {
        log_error(logger, "Handshake inválido (op_code %d) en socket %d", op_code, fd);
        send_error(fd, "Handshake inválido: la Placa solo acepta CORE o PLANIFICADOR");
        return NULL;
    }

    if (send_ack(fd) != 0) {
        log_error(logger, "No se pudo responder el handshake de %s en socket %d", nombre, fd);
        return NULL;
    }

    log_info(logger, "## Módulo: %s", nombre);
    log_debug(logger, "%s conectado en socket %d", nombre, fd);
    return nombre;
}

/**
 * Hilo por cliente: handshake y luego loop de atención de operaciones hasta
 * que el cliente cierre la conexión.
 */
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
