#include "operaciones.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utils/messaging/packet.h>
#include "globals.h"
#include "memoria_instrucciones.h"

static void aplicar_retardo_memoria(void) {
    usleep(placa_config->retardo_memoria * 1000);
}

/**
 * Valida que el payload tenga al menos `minimos` campos. Si no, responde
 * MSG_ERROR al cliente para no dejarlo colgado esperando.
 */
static bool validar_campos(int fd, t_list* campos, int minimos, const char* operacion) {
    if (list_size(campos) >= minimos) {
        return true;
    }
    log_error(logger, "%s: payload inválido (%d campos, se esperaban %d)",
              operacion, list_size(campos), minimos);
    send_error(fd, "Payload inválido");
    return false;
}

static uint32_t campo_uint32(t_list* campos, int indice) {
    return *(uint32_t*) list_get(campos, indice);
}

static char* campo_string(t_list* campos, int indice) {
    return (char*) list_get(campos, indice);
}

// ---------------------------------------------------------------------------
// Memoria de instrucciones
// ---------------------------------------------------------------------------

static void atender_crear_job(int fd, t_list* campos) {
    if (!validar_campos(fd, campos, 2, "CREAR_JOB")) return;

    uint32_t jid     = campo_uint32(campos, 0);
    char*    archivo = campo_string(campos, 1);

    if (memoria_instrucciones_crear_job(jid, archivo) != 0) {
        send_error(fd, "No se pudo crear el Job");
        return;
    }

    log_info(logger, "## JID: %u - Job Creado", jid);
    send_ack(fd);
}

static void atender_obtener_instruccion(int fd, t_list* campos) {
    if (!validar_campos(fd, campos, 2, "OBTENER_INSTRUCCION")) return;

    uint32_t jid = campo_uint32(campos, 0);
    uint32_t pc  = campo_uint32(campos, 1);

    char* instruccion = memoria_instrucciones_obtener(jid, pc);
    if (instruccion == NULL) {
        log_error(logger, "JID: %u - No existe la instrucción %u", jid, pc);
        send_error(fd, "Instrucción inexistente");
        return;
    }

    aplicar_retardo_memoria();
    log_info(logger, "## JID: %u - Obtener instrucción: %u - Instrucción: %s", jid, pc, instruccion);

    t_packet* respuesta = create_packet(MSG_OK);
    add_to_packet(respuesta, instruccion, strlen(instruccion) + 1);
    send_packet(respuesta, fd);
    destroy_packet(respuesta);
    free(instruccion);
}

static void atender_finalizar_job(int fd, t_list* campos) {
    if (!validar_campos(fd, campos, 1, "FINALIZAR_JOB")) return;

    uint32_t jid = campo_uint32(campos, 0);

    // TODO (checkpoint 3): liberar marcos, espacio en Offload y tabla de páginas del Job
    if (memoria_instrucciones_finalizar_job(jid) != 0) {
        log_error(logger, "JID: %u - Se pidió finalizar un Job inexistente", jid);
        send_error(fd, "Job inexistente");
        return;
    }

    log_info(logger, "## JID: %u - Job Finalizado", jid);
    send_ack(fd);
}

// ---------------------------------------------------------------------------
// Memoria de usuario (checkpoint 3)
// ---------------------------------------------------------------------------

static void atender_no_implementada(int fd, t_msg_code op_code, const char* nombre_modulo) {
    log_warning(logger, "Operación %d solicitada por %s aún no implementada", op_code, nombre_modulo);
    send_error(fd, "Operación no implementada");
}

// ---------------------------------------------------------------------------

int operaciones_atender(int fd, t_msg_code op_code, const char* nombre_modulo) {
    // Todas las operaciones traen payload (aunque sea vacío): se consume completo
    // antes de procesar para mantener alineado el stream del socket.
    t_list* campos = receive_packet(fd);
    if (campos == NULL) {
        return -1;
    }

    switch (op_code) {
        case PLACA_CREAR_JOB:
            atender_crear_job(fd, campos);
            break;
        case PLACA_OBTENER_INSTRUCCION:
            atender_obtener_instruccion(fd, campos);
            break;
        case PLACA_FINALIZAR_JOB:
            atender_finalizar_job(fd, campos);
            break;
        case PLACA_ALLOC:
        case PLACA_FREE:
        case PLACA_OBTENER_MARCO:
        case PLACA_CARGAR_PAGINA:
        case PLACA_LEER:
        case PLACA_ESCRIBIR:
        case PLACA_DESLOCKEAR_PAGINAS:
            atender_no_implementada(fd, op_code, nombre_modulo);
            break;
        default:
            log_error(logger, "Operación desconocida (%d) recibida de %s", op_code, nombre_modulo);
            send_error(fd, "Operación desconocida");
            break;
    }

    list_destroy_and_destroy_elements(campos, free);
    return 0;
}
