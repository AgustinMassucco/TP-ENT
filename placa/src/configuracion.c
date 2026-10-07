#include "configuracion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>

static const char* CAMPOS_OBLIGATORIOS[] = {
    "PUERTO_ESCUCHA", "LOG_LEVEL", "TAM_MEMORIA", "TAM_PAGINA", "RETARDO_MEMORIA",
    "ALGORITMO_REEMPLAZO", "PATH_INSTRUCCIONES", "PATH_OFFLOAD", "TAM_OFFLOAD", "RETARDO_OFFLOAD",
    NULL
};

static bool es_potencia_de_dos(uint32_t n) {
    return n != 0 && (n & (n - 1)) == 0;
}

/**
 * Garantiza que el path termine en '/' para poder concatenar nombres de archivo
 * directamente (el enunciado da los ejemplos con barra final, pero no lo exige).
 */
static char* path_directorio_normalizado(char* path) {
    if (string_ends_with(path, "/")) {
        return strdup(path);
    }
    return string_from_format("%s/", path);
}

t_placa_config* configuracion_cargar(char* path) {
    t_config* cfg = config_create(path);
    if (cfg == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo de configuración: %s\n", path);
        return NULL;
    }

    for (int i = 0; CAMPOS_OBLIGATORIOS[i] != NULL; i++) {
        if (!config_has_property(cfg, (char*) CAMPOS_OBLIGATORIOS[i])) {
            fprintf(stderr, "Falta el campo %s en el archivo de configuración\n", CAMPOS_OBLIGATORIOS[i]);
            config_destroy(cfg);
            return NULL;
        }
    }

    t_placa_config* config = malloc(sizeof(t_placa_config));
    if (config == NULL) {
        config_destroy(cfg);
        return NULL;
    }

    config->puerto_escucha     = config_get_int_value(cfg, "PUERTO_ESCUCHA");
    config->log_level          = log_level_from_string(config_get_string_value(cfg, "LOG_LEVEL"));
    config->tam_memoria        = (uint32_t) config_get_int_value(cfg, "TAM_MEMORIA");
    config->tam_pagina         = (uint32_t) config_get_int_value(cfg, "TAM_PAGINA");
    config->retardo_memoria    = config_get_int_value(cfg, "RETARDO_MEMORIA");
    config->path_instrucciones = path_directorio_normalizado(config_get_string_value(cfg, "PATH_INSTRUCCIONES"));
    config->path_offload       = strdup(config_get_string_value(cfg, "PATH_OFFLOAD"));
    config->tam_offload        = (uint32_t) config_get_int_value(cfg, "TAM_OFFLOAD");
    config->retardo_offload    = config_get_int_value(cfg, "RETARDO_OFFLOAD");

    char* algoritmo = config_get_string_value(cfg, "ALGORITMO_REEMPLAZO");
    bool algoritmo_valido = true;
    if (strcmp(algoritmo, "LRU") == 0) {
        config->algoritmo_reemplazo = REEMPLAZO_LRU;
    } else if (strcmp(algoritmo, "CLOCK-M") == 0) {
        config->algoritmo_reemplazo = REEMPLAZO_CLOCK_M;
    } else {
        fprintf(stderr, "ALGORITMO_REEMPLAZO inválido: %s (valores posibles: LRU, CLOCK-M)\n", algoritmo);
        algoritmo_valido = false;
    }

    config_destroy(cfg);

    if (!algoritmo_valido) {
        configuracion_destruir(config);
        return NULL;
    }

    // log_level_from_string devuelve -1 si el string no matchea ningún nivel
    if ((int) config->log_level < 0) {
        fprintf(stderr, "LOG_LEVEL inválido\n");
        configuracion_destruir(config);
        return NULL;
    }

    if (!es_potencia_de_dos(config->tam_memoria) || !es_potencia_de_dos(config->tam_pagina)) {
        fprintf(stderr, "TAM_MEMORIA y TAM_PAGINA deben ser potencias de 2\n");
        configuracion_destruir(config);
        return NULL;
    }

    if (config->tam_pagina > config->tam_memoria) {
        fprintf(stderr, "TAM_PAGINA (%u) no puede ser mayor que TAM_MEMORIA (%u)\n",
                config->tam_pagina, config->tam_memoria);
        configuracion_destruir(config);
        return NULL;
    }

    if (config->tam_offload % config->tam_pagina != 0) {
        fprintf(stderr, "TAM_OFFLOAD (%u) debe ser múltiplo de TAM_PAGINA (%u)\n",
                config->tam_offload, config->tam_pagina);
        configuracion_destruir(config);
        return NULL;
    }

    return config;
}

void configuracion_destruir(t_placa_config* config) {
    if (config == NULL) {
        return;
    }
    free(config->path_instrucciones);
    free(config->path_offload);
    free(config);
}

const char* configuracion_algoritmo_as_string(t_algoritmo_reemplazo algoritmo) {
    return algoritmo == REEMPLAZO_LRU ? "LRU" : "CLOCK-M";
}
