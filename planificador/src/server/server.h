/*Servidor multihilo para peticiones de los CORES */
#ifndef SERVER_H
#define SERVER_H

#include <commons/config.h>
#include <commons/log.h>

/**
 * @brief Inicia el servidor de Cores: crea el socket de escucha en PUERTO_ESCUCHA
 *        y queda aceptando conexiones indefinidamente, delegando cada una a un
 *        hilo propio (detached) para que las conexiones concurrentes no se bloqueen
 *        entre si.
 * @param config Puntero al archivo de configuracion (usa PUERTO_ESCUCHA)
 * @param logger Logger activo del modulo, usado tambien dentro de cada hilo
 */
void start_core_server(t_config* config, t_log* logger);

#endif /* SERVER_H */