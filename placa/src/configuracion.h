#ifndef PLACA_CONFIGURACION_H_
#define PLACA_CONFIGURACION_H_

#include <stdint.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/log.h>

typedef enum {
    REEMPLAZO_LRU,
    REEMPLAZO_CLOCK_M
} t_algoritmo_reemplazo;

/**
 * Configuración de la Placa, cargada una sola vez al inicio.
 * Los strings apuntan a memoria propia (copiados del t_config), por lo que
 * el t_config se destruye inmediatamente después de cargar.
 */
typedef struct {
    int                   puerto_escucha;
    t_log_level           log_level;
    uint32_t              tam_memoria;
    uint32_t              tam_pagina;
    int                   retardo_memoria;     // milisegundos
    t_algoritmo_reemplazo algoritmo_reemplazo;
    char*                 path_instrucciones;
    char*                 path_offload;
    uint32_t              tam_offload;
    int                   retardo_offload;     // milisegundos
} t_placa_config;

/**
 * @brief Lee el archivo de configuración y valida los campos obligatorios.
 * @return Config alocada en heap, o NULL si falta algún campo o hay valores inválidos.
 *         Los errores se imprimen por stderr (todavía no existe el logger).
 */
t_placa_config* configuracion_cargar(char* path);

void configuracion_destruir(t_placa_config* config);

/** @return "LRU" o "CLOCK-M" */
const char* configuracion_algoritmo_as_string(t_algoritmo_reemplazo algoritmo);

#endif /* PLACA_CONFIGURACION_H_ */
