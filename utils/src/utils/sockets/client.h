#ifndef SOCKETS_CLIENT_H_
#define SOCKETS_CLIENT_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

/**
 * @brief Se conecta a un servidor en la IP y puerto dados.
 * @param ip   Dirección IP del servidor.
 * @param port Número de puerto del servidor.
 * @return File descriptor de la conexión, o -1 en caso de error.
 */
int connect_to(const char* ip, int port);

#endif /* SOCKETS_CLIENT_H_ */