#include "operaciones.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils/messaging/packet.h>
#include "globals.h"

static bool validar_campos(int fd, t_list* campos, int minimos, const char* operacion) {
    if (list_size(campos) >= minimos) {
        return true;
    }
    log_error(logger, "%s: payload inválido (%d campos, se esperaban %d)",
              operacion, list_size(campos), minimos);
    send_error(fd, "Payload inválido");
    return false;
}

static void atender_guardar_checkpoint(int fd, t_list* campos) {
    if (!validar_campos(fd, campos, 3, "GUARDAR_CHECKPOINT")) return;

    char*    nombre  = (char*) list_get(campos, 0);
    uint32_t tamanio = *(uint32_t*) list_get(campos, 1);

    log_info(logger, "## Solicitud Guardar Checkpoint: %s", nombre);

    log_warning(logger, "Guardar Checkpoint %s (%u bytes) aún no implementado", nombre, tamanio);
    send_error(fd, "Operación no implementada");
}

static void atender_cargar_checkpoint(int fd, t_list* campos) {
    if (!validar_campos(fd, campos, 1, "CARGAR_CHECKPOINT")) return;

    char* nombre = (char*) list_get(campos, 0);

    log_info(logger, "## Solicitud Cargar Checkpoint: %s", nombre);

    log_warning(logger, "Cargar Checkpoint %s aún no implementado", nombre);
    send_error(fd, "Operación no implementada");
}

static void atender_eliminar_checkpoint(int fd, t_list* campos) {
    if (!validar_campos(fd, campos, 1, "ELIMINAR_CHECKPOINT")) return;

    char* nombre = (char*) list_get(campos, 0);

    log_info(logger, "## Solicitud Eliminacion Checkpoint: %s", nombre);

    log_warning(logger, "Eliminar Checkpoint %s aún no implementado", nombre);
    send_error(fd, "Operación no implementada");
}

int operaciones_atender(int fd, t_msg_code op_code, const char* nombre_modulo) {
    t_list* campos = receive_packet(fd);
    if (campos == NULL) {
        return -1;
    }

    switch (op_code) {
        case STORAGE_GUARDAR_CHECKPOINT:
            atender_guardar_checkpoint(fd, campos);
            break;
        case STORAGE_CARGAR_CHECKPOINT:
            atender_cargar_checkpoint(fd, campos);
            break;
        case STORAGE_ELIMINAR_CHECKPOINT:
            atender_eliminar_checkpoint(fd, campos);
            break;
        default:
            log_error(logger, "Operación desconocida (%d) recibida de %s", op_code, nombre_modulo);
            send_error(fd, "Operación desconocida");
            break;
    }

    list_destroy_and_destroy_elements(campos, free);
    return 0;
}
