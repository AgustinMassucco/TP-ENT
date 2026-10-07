#ifndef SRC_COMMMON_UTILS_H
#define SRC_COMMMON_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>


/**
 * @brief Obtiene el nivel de log configurado en el archivo de configuración
 * @param config Puntero al archivo de configuración
 * @return el nivel de log del archivo config, o LOG_LEVEL_INFO si no se encuentra configurado
 */
t_log_level config_get_log_level(t_config* config);

/**
 * @brief Inicializa un logger con el nivel de log configurado en el archivo de configuración
 * @param file Nombre del archivo de log
 * @param module_name Nombre del módulo que crea el logger
 * @param config Puntero al archivo de configuración
 * @return un puntero al logger inicializado, o NULL en caso de error
 */
t_log* initialize_logger(char* file, char* module_name, t_config* config);

#endif /* SRC_COMMMON_UTILS_H */    