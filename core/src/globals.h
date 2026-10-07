#ifndef CORE_GLOBALS_H_
#define CORE_GLOBALS_H_

#include <commons/log.h>
#include "configuracion.h"

/**
 * Estado global del módulo. Se inicializa en main() antes de empezar a
 * ejecutar Jobs y no se modifica después, por lo que puede leerse sin sincronización.
 */
extern t_log*         logger;
extern t_core_config* core_config;
extern char*          core_id;          // Identificador recibido por parámetro

extern int fd_planificador;
extern int fd_placa;

#endif /* CORE_GLOBALS_H_ */
