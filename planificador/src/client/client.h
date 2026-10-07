/*Se debe conectar a PLACA y a STORAGE*/
#ifndef CLIENT_H
#define CLIENT_H

#include <commons/log.h>
#include <commons/config.h>
#include "utils/sockets/client.h"
#include "utils/messaging/packet.h"
#include "utils/messaging/codes.h"
#include "utils/common_utils.h"


/*
 * @brief Conecta al servidor de la PLACA
 * @param config Puntero al archivo de configuracion
 * @param logger Logger activo
 * @return el file descriptor de la conexion con PLACA, o -1 en caso de error
 */
int connect_to_placa(t_config* config, t_log* logger); 

/* 
 * @brief Conecta al servidor de la STORAGE
 * @param config Puntero al archivo de configuracion
 * @param logger Logger activo
 * @return el file descriptor de la conexion con STORAGE, o -1 en caso de error
 */
int connect_to_storage(t_config* config, t_log* logger);


#endif /* CLIENT_H */