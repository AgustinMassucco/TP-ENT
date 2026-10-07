#include "conexiones.h"

#include <unistd.h>
#include <utils/sockets/client.h>
#include <utils/messaging/packet.h>
#include "globals.h"

/**
 * Abre la conexión, envía HANDSHAKE_CORE (payload vacío) y espera MSG_OK.
 * @param nombre Nombre del módulo destino, usado en los logs ("Planificador"/"Placa").
 */
static int conectar_con_handshake(const char* ip, int puerto, const char* nombre) {
    int fd = connect_to(ip, puerto);
    if (fd == -1) {
        log_error(logger, "No se pudo establecer conexión con %s (%s:%d)", nombre, ip, puerto);
        return -1;
    }

    t_packet* handshake = create_packet(HANDSHAKE_CORE);
    if (handshake == NULL || send_packet(handshake, fd) != 0) {
        log_error(logger, "No se pudo enviar el handshake a %s", nombre);
        destroy_packet(handshake);
        close(fd);
        return -1;
    }
    destroy_packet(handshake);

    if (receive_ack_or_error(fd, logger) != 0) {
        log_error(logger, "%s rechazó el handshake", nombre);
        close(fd);
        return -1;
    }

    log_info(logger, "## Conectado a %s exitosamente", nombre);
    return fd;
}

int conectar_a_planificador(void) {
    return conectar_con_handshake(core_config->ip_planificador, core_config->puerto_planificador, "Planificador");
}

int conectar_a_placa(void) {
    return conectar_con_handshake(core_config->ip_placa, core_config->puerto_placa, "Placa");
}
