#include "client.h"

int connect_to(const char* ip, int port) {
    if (ip == NULL) {
        return -1;
    }
    if (port <= 0 || port > 65535) {
        return -1;
    }

    //Convertir el puerto a string para getaddrinfo. 5 dígitos + null terminator. (6 bytes en total)
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* server_info;
    if (getaddrinfo(ip, port_str, &hints, &server_info) != 0) {
        return -1;
    }

    int fd = socket(server_info->ai_family,
                    server_info->ai_socktype,
                    server_info->ai_protocol);

    if (fd < 0) {
        freeaddrinfo(server_info);
        return -1;
    }

    if (connect(fd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        close(fd);
        freeaddrinfo(server_info);
        return -1;
    }

    freeaddrinfo(server_info);
    return fd;
}