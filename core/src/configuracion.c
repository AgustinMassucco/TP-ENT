#include "configuracion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* CAMPOS_OBLIGATORIOS[] = {
    "LOG_LEVEL", "IP_PLANIFICADOR", "PUERTO_PLANIFICADOR", "IP_PLACA", "PUERTO_PLACA",
    NULL
};

t_core_config* configuracion_cargar(char* path) {
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

    char* log_level_str = config_get_string_value(cfg, "LOG_LEVEL");
    t_log_level log_level = log_level_from_string(log_level_str);
    if ((int) log_level == -1) {
        fprintf(stderr, "LOG_LEVEL inválido: %s\n", log_level_str);
        config_destroy(cfg);
        return NULL;
    }

    t_core_config* config = malloc(sizeof(t_core_config));
    if (config == NULL) {
        config_destroy(cfg);
        return NULL;
    }

    config->log_level           = log_level;
    config->ip_planificador     = strdup(config_get_string_value(cfg, "IP_PLANIFICADOR"));
    config->puerto_planificador = config_get_int_value(cfg, "PUERTO_PLANIFICADOR");
    config->ip_placa            = strdup(config_get_string_value(cfg, "IP_PLACA"));
    config->puerto_placa        = config_get_int_value(cfg, "PUERTO_PLACA");

    config_destroy(cfg);
    return config;
}

void configuracion_destruir(t_core_config* config) {
    if (config == NULL) {
        return;
    }
    free(config->ip_planificador);
    free(config->ip_placa);
    free(config);
}
