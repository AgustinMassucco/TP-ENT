#include "server.h"

int create_server(int port) {
    //El protocolo de mensajes utiliza 16 bits para el puerto, asi que no se aceptan puertos fuera del rango 1-65535 (2^16-1) (0 es reservado)
    if (port <= 0 || port > 65535) {
        return -1;
    }  
 
    //Convertir el puerto a string para getaddrinfo. 5 dígitos + null terminator. (6 bytes en total)
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    //Preparar hints para getaddrinfo, inicializar el socket en cero y setear hints, configs copiadas del TP0
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    struct addrinfo* server_info;
    if (getaddrinfo(NULL, port_str, &hints, &server_info) != 0) {
        return -1;
    }

    //Crear el socket, bindearlo a la direccion y puerto, y ponerlo a escuchar
    int server_fd = socket(server_info->ai_family,
                           server_info->ai_socktype,
                           server_info->ai_protocol);

    if (server_fd < 0) {
        freeaddrinfo(server_info);
        return -1;
    }

    // Permitir reusar el puerto inmediatamente después de cerrar el servidor, para facilitar pruebas iterativas
    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));

    //Bind puede fallar por varias razones (puerto en uso, permisos insuficientes, etc). En cualquier caso, no hay nada que el servidor pueda hacer para recuperarse, asi que se devuelve error
    if (bind(server_fd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        close(server_fd);
        freeaddrinfo(server_info);
        return -1;
    }
    //Liberar la memoria alocada por getaddrinfo, no es necesaria después de bindear el socket
    freeaddrinfo(server_info);

    //listen puede fallar si el socket no es válido o si el sistema no puede aceptar más conexiones (por ejemplo, por falta de recursos). Dado que el servidor no tiene forma de recuperarse de esto, se devuelve error
    if (listen(server_fd, SOMAXCONN) < 0) {
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int accept_connection(int server_fd) {
    return accept(server_fd, NULL, NULL);
}