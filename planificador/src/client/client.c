/*Se debe conectar a PLACA y a STORAGE*/
#include "client.h"

int connect_to_placa(t_config* config, t_log* logger) {
    char* placa_ip = config_get_string_value(config, "IP_PLACA");
    int placa_port = config_get_int_value(config, "PUERTO_PLACA");

    int fd_placa = connect_to(placa_ip,placa_port);
    if (fd_placa == -1) { log_error(logger, "ERROR: No se pudo establecer conexion socket con PLACA (%s:%d)", placa_ip, placa_port); 
        return -1; 
    }

    t_packet* packet = create_packet(HANDSHAKE_PLANIFICADOR);
    if (send_packet(packet, fd_placa) == -1) {
        log_error(logger, "ERROR: No se puedo enviar handshake a PLACA");
        destroy_packet(packet);
        close(fd_placa);
        return -1;
    }

    destroy_packet(packet);
    
    if (receive_ack_or_error(fd_placa, logger) == -1) {
        log_error(logger, "ERROR: No se recibio handshake de PLACA");
        close(fd_placa);
        return -1;
    }
    
    log_info(logger, "## Conectado a Placa exitosamente");

    return fd_placa;
}

int connect_to_storage(t_config* config, t_log* logger) {
    char* storage_ip = config_get_string_value(config, "IP_STORAGE");
    int storage_port = config_get_int_value(config, "PUERTO_STORAGE");

    int fd_storage = connect_to(storage_ip,storage_port);
    if (fd_storage == -1) { log_error(logger, "ERROR: No se pudo establecer conexion socket con STORAGE (%s:%d)", storage_ip, storage_port); 
        return -1; 
    }

    t_packet* packet = create_packet(HANDSHAKE_PLANIFICADOR);
    if (send_packet(packet, fd_storage) == -1) {
        log_error(logger, "ERROR: No se puedo enviar handshake a STORAGE");
        destroy_packet(packet);
        close(fd_storage);
        return -1;
    }

    destroy_packet(packet);
    
    if (receive_ack_or_error(fd_storage, logger) == -1) {
        log_error(logger, "ERROR: No se recibio handshake de STORAGE");
        close(fd_storage);
        return -1;
    }
    
    log_info(logger, "## Conectado a Storage exitosamente");

    return fd_storage;
}