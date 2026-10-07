#include "packet.h"

#define MAX_PACKET_PAYLOAD_SIZE (16 * 1024 * 1024) // 16MB, límite arbitrario para evitar paquetes muy grandes

/**
 * @brief Crea un t_buffervacio en heap
 * @return Buffer alocado o NULL si el malloc falla
*/
static t_buffer* create_buffer(void) {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    if(buffer == NULL) {
        return NULL;
    }
    buffer->size   = 0;
    buffer->stream = NULL;
    return buffer;
}

t_packet* create_packet(t_msg_code op_code) {
    t_packet* packet = malloc(sizeof(t_packet));
    if(packet == NULL) {
        return NULL;
    }

    packet->op_code = op_code;
    packet->buffer = create_buffer();
    if(packet->buffer == NULL) {
        free(packet);
        return NULL;
    }
    return packet;
}

void add_to_packet(t_packet* packet, void* value, int size) {
    if (packet == NULL || packet->buffer == NULL || value == NULL || size <= 0) {
        return;
    }

    //tamaño nuevo del stream después de agregar el campo: tamaño viejo + tamaño del nuevo campo + 4 bytes para almacenar el tamaño del nuevo campo
    size_t new_size = (size_t) packet->buffer->size + (size_t) size + sizeof(int);

    //si realloc falla, no se pisa el stream viejo (queda usable con el tamaño viejo) y se devuelve NULL
    void* new_stream = realloc(packet->buffer->stream, new_size);
    if (new_stream == NULL) {
        return;
    }

    //se guarda el tamaño y el stream nuevo, solo si realloc tuvo éxito
    packet->buffer->stream = new_stream;
    memcpy(packet->buffer->stream + packet->buffer->size, &size, sizeof(int));
    memcpy(packet->buffer->stream + packet->buffer->size + sizeof(int), value, size);
    packet->buffer->size += size + sizeof(int);
}

int receive_operation(int fd) {
    int op_code;
    //recv con MSG_WAITALL espera a recibir todos los bytes pedidos o un error
    int err = recv(fd, &op_code, sizeof(int), MSG_WAITALL);

    //Solo se acepta lectura completa del int
    if (err <= 0 || (size_t) err != sizeof(int)) {
        return -1;
    }

    return op_code;

}

void* receive_buffer(int* size, int fd) {
    if (size == NULL) {
        return NULL;
    }
    *size = 0;

    //Se recibe el tamaño del buffer (int) y validar que no sea negativo ni demasiado grande
    int local_size;
    int err = recv(fd, &local_size, sizeof(int), MSG_WAITALL);
    if (err <= 0 || (size_t) err != sizeof(int)) {
        return NULL;
    }

    //Rechazar size negativo o demasiado grande para evitar mallocs gigantes
    if (local_size < 0 || local_size > MAX_PACKET_PAYLOAD_SIZE) {
        return NULL;
    }

    *size = local_size;
    if (local_size == 0) {
        return NULL;
    }

    void* buffer = malloc(local_size);
    if (buffer == NULL) {
        *size = 0;
        return NULL;
    }

    //lectura de payload del buffer. Se acepta solo lectura completa del buffer pedido
    err = recv(fd, buffer, local_size, MSG_WAITALL);
    if (err <= 0 || err != local_size) {
        free(buffer);
        *size = 0;
        return NULL;
    }
    return buffer;
}

t_list* receive_packet(int fd) {
    int size   = 0;
    int offset = 0;

    t_list* values = list_create();
    if (values == NULL) {
        return NULL;
    }

    //caso valido: payload vacio (size=0, buffer=NULL). Se devuelve lista vacia
    void* buffer = receive_buffer(&size, fd);
    if (buffer == NULL && size == 0) {
        return values;
    }

    //size != 0 pero buffer == NULL: error de recepcion
    if (buffer == NULL) {
        list_destroy(values);
        return NULL;
    }

    while (offset < size) {
        //Se valida que el campo entre antes de leerlo. Desplazamiento + tamaño del campo (int) no puede superar el tamaño total del buffer
        if (offset + (int) sizeof(int) > size){
            list_destroy_and_destroy_elements(values, free);
            free(buffer);
            return NULL;
        }
        
        int field_size;
        memcpy(&field_size, buffer + offset, sizeof(int));
        //desplazamiento del tamaño del campo (int)
        offset += sizeof(int);

        //Se valida que el campo completo entre en el buffer
        if (field_size < 0 || field_size > size - offset){
            list_destroy_and_destroy_elements(values, free);
            free(buffer);
            return NULL;
        }
        
        //Se reserva al menos sizeof(uint32_t) bytes y uno extra en 0: asi el receptor puede leer
        //el campo como uint32_t o como string terminado en '\0' sin salirse del buffer aunque el
        //emisor haya mandado un campo mas corto de lo esperado. calloc deja el padding en cero.
        size_t alloc_size = (field_size > (int) sizeof(uint32_t) ? (size_t) field_size : sizeof(uint32_t)) + 1;
        char* value = calloc(1, alloc_size);
        if (value == NULL) {
            list_destroy_and_destroy_elements(values, free);
            free(buffer);
            return NULL;
        }

        if (field_size > 0) {
            memcpy(value, buffer + offset, field_size);
        }
        offset += field_size;
        list_add(values, value);
    }
    free(buffer);
    return values;
}


/**
 * @brief Aplana el packet a un buffer contiguo de bytes con el formato: [op_code (int)][buffer_size (int)][buffer_stream (bytes)]
 * @param packet Packet a serializar
 * @param bytes  Tamaño total esperado del buffer resultante (debe ser igual a packet->buffer->size + 2*sizeof(int))
 * @return Buffer alocado con el contenido serializado, o NULL si el malloc falla.
*/
static void* serialize_packet(t_packet* packet, int bytes) {
    void* buffer = malloc(bytes);
    if (buffer == NULL) {
        return NULL;
    }

    int offset = 0;
    memcpy(buffer + offset, &(packet->op_code), sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &(packet->buffer->size), sizeof(int));
    offset += sizeof(int);

    if (packet->buffer->size > 0) {
        memcpy(buffer + offset, packet->buffer->stream, packet->buffer->size);
        offset += packet->buffer->size;
    }

    return buffer;
}

int send_packet(t_packet* packet, int fd) {
    if (packet == NULL || packet->buffer == NULL) {
        return -1;
    }

    int   bytes   = packet->buffer->size + 2 * sizeof(int);
    void* to_send = serialize_packet(packet, bytes);
    if (to_send == NULL) {
        return -1;
    }

    //send puede enviar menos bytes que los pedidos; hay que llamar send en un loop hasta enviar todo el buffer.
    int total_sent = 0;
    while (total_sent < bytes) {
        int sent = send(fd, to_send + total_sent, bytes - total_sent, MSG_NOSIGNAL);
        if (sent <= 0) {
            free(to_send);
            return -1;
        }
        total_sent += sent;
    }

    free(to_send);
    return 0;
}

void destroy_packet(t_packet* packet) {
    if (packet == NULL) {
        return;
    }
    if (packet->buffer != NULL) {
        free(packet->buffer->stream);
        free(packet->buffer);
    }
    free(packet);
}

int send_ack(int fd) {
    t_packet* packet = create_packet(MSG_OK);
    if (packet == NULL) {
        return -1;
    }

    int result = send_packet(packet, fd);
    destroy_packet(packet);
    return result;
}

int send_error(int fd, char* message) {
   t_packet* packet = create_packet(MSG_ERROR);
   if (packet == NULL) {
       return -1;
   }

   add_to_packet(packet, message, strlen(message) + 1);
   int result = send_packet(packet, fd);
   destroy_packet(packet);
   return result;
}

int receive_ack_or_error(int fd, t_log* logger) {
    int op = receive_operation(fd);

    if (op == MSG_OK) {
        int   size         = 0;
        void* empty_buffer = receive_buffer(&size, fd);
        if (empty_buffer != NULL) {
            free(empty_buffer);
            log_error(logger, "ERROR: se recibio un ACK con payload inesperado");
        }
        return 0;
    }

    if (op == MSG_ERROR) {
        t_list* fields = receive_packet(fd);
        if (fields != NULL && !list_is_empty(fields)) {
            char* error_msg = list_get(fields, 0);
            log_error(logger, "Respuesta del servidor: %s", error_msg);
        } else {
            log_error(logger, "Se recibio MSG_ERROR del servidor sin mensaje");
        }
        if (fields != NULL) {
            list_destroy_and_destroy_elements(fields, free);
        }
        return -1;
    }

    log_error(logger, "Respuesta inesperada del servidor. OpCode: %d", op);
    return -1;
}

int receive_response_or_error(int fd, t_log* logger) {
    int op = receive_operation(fd);

    if (op == MSG_OK) {
        //el caller leera el payload esperado después de utilizar esta función
        return MSG_OK;
    }

    if (op == MSG_ERROR) {
        //se drena el payload del error para mantener la sincronizacion del buffer
        t_list* fields = receive_packet(fd);
        if (fields != NULL && !list_is_empty(fields)) {
            char* error_msg = list_get(fields, 0);
            log_error(logger, "Respuesta del servidor: %s", error_msg);
        } else {
            log_error(logger, "Se recibio MSG_ERROR del servidor sin mensaje");
        }
        if (fields != NULL) {
            list_destroy_and_destroy_elements(fields, free);
        }
        return -1;
    }

    log_error(logger, "Respuesta inesperada del servidor. OpCode: %d", op);
    return -1;
}