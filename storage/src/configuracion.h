#ifndef STORAGE_CONFIGURACION_H_
#define STORAGE_CONFIGURACION_H_

#include <stdint.h>
#include <commons/config.h>
#include <commons/log.h>

typedef struct {
    int         puerto_escucha;
    t_log_level log_level;
    char*       path_storage;
    uint32_t    cant_bloques;
    uint32_t    tam_bloque;
    uint32_t    bloques_directorio;
    int         retardo_acceso_bloque;
    int         retardo_commit;
} t_storage_config;

t_storage_config* configuracion_cargar(char* path);

void configuracion_destruir(t_storage_config* config);

#endif
