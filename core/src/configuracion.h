#ifndef CORE_CONFIGURACION_H_
#define CORE_CONFIGURACION_H_

#include <commons/config.h>
#include <commons/log.h>

/**
 * Configuración del Core, cargada una sola vez al inicio.
 * Los strings apuntan a memoria propia (copiados del t_config), por lo que
 * el t_config se destruye inmediatamente después de cargar.
 */
typedef struct {
    t_log_level log_level;
    char*       ip_planificador;
    int         puerto_planificador;
    char*       ip_placa;
    int         puerto_placa;
} t_core_config;

/**
 * @brief Lee el archivo de configuración y valida los campos obligatorios.
 * @return Config alocada en heap, o NULL si falta algún campo o hay valores inválidos.
 *         Los errores se imprimen por stderr (todavía no existe el logger).
 */
t_core_config* configuracion_cargar(char* path);

void configuracion_destruir(t_core_config* config);

#endif /* CORE_CONFIGURACION_H_ */
