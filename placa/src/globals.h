#ifndef PLACA_GLOBALS_H_
#define PLACA_GLOBALS_H_

#include <commons/log.h>
#include "configuracion.h"

/**
 * Estado global del módulo. Se inicializa en main() antes de lanzar cualquier
 * hilo y no se modifica después, por lo que puede leerse sin sincronización.
 */
extern t_log*          logger;
extern t_placa_config* placa_config;

#endif /* PLACA_GLOBALS_H_ */
