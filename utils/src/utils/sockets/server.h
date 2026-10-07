#ifndef SOCKETS_SERVER_H_
#define SOCKETS_SERVER_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

/**
 * @brief Crea un socket servidor escuchando en el puerto dado.
 * @param port Número de puerto en el que escuchar.
 * @return File descriptor del servidor, o -1 en caso de error.
 */
int create_server(int port);

/**
 * @brief Acepta una conexión entrante. Bloquea hasta que un cliente se conecte.
 * @param server_fd File descriptor del servidor devuelto por create_server().
 * @return File descriptor del cliente, o -1 en caso de error.
 */
int accept_connection(int server_fd);

#endif /* SOCKETS_SERVER_H_ */