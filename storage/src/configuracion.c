#include "configuracion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <commons/string.h>

static const char* CAMPOS_OBLIGATORIOS[] = {
    "PUERTO_ESCUCHA", "LOG_LEVEL", "PATH_STORAGE", "CANT_BLOQUES", "TAM_BLOQUE",
    "BLOQUES_DIRECTORIO", "RETARDO_ACCESO_BLOQUE", "RETARDO_COMMIT",
    NULL
};

#define TAM_BLOQUE_MINIMO 32

static bool es_potencia_de_dos(uint32_t n) {
    return n != 0 && (n & (n - 1)) == 0;
}

static char* path_directorio_normalizado(char* path) {
    if (string_ends_with(path, "/")) {
        return strdup(path);
    }
    return string_from_format("%s/", path);
}

static bool hay_lugar_para_bloques_de_datos(t_storage_config* config) {
    uint64_t bytes_fat   = (uint64_t) config->cant_bloques * sizeof(uint32_t);
    uint64_t bloques_fat = (bytes_fat + config->tam_bloque - 1) / config->tam_bloque;

    uint64_t bloques_reservados = 1 + bloques_fat + config->bloques_directorio;
    if (bloques_reservados >= config->cant_bloques) {
        fprintf(stderr,
                "La configuración no deja bloques de datos: superbloque (1) + FAT (%lu) + "
                "directorio (%u) ocupan %lu de %u bloques\n",
                (unsigned long) bloques_fat, config->bloques_directorio,
                (unsigned long) bloques_reservados, config->cant_bloques);
        return false;
    }
    return true;
}

t_storage_config* configuracion_cargar(char* path) {
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

    t_storage_config* config = malloc(sizeof(t_storage_config));
    if (config == NULL) {
        config_destroy(cfg);
        return NULL;
    }

    config->puerto_escucha        = config_get_int_value(cfg, "PUERTO_ESCUCHA");
    config->log_level             = log_level_from_string(config_get_string_value(cfg, "LOG_LEVEL"));
    config->path_storage          = path_directorio_normalizado(config_get_string_value(cfg, "PATH_STORAGE"));
    config->cant_bloques          = (uint32_t) config_get_int_value(cfg, "CANT_BLOQUES");
    config->tam_bloque            = (uint32_t) config_get_int_value(cfg, "TAM_BLOQUE");
    config->bloques_directorio    = (uint32_t) config_get_int_value(cfg, "BLOQUES_DIRECTORIO");
    config->retardo_acceso_bloque = config_get_int_value(cfg, "RETARDO_ACCESO_BLOQUE");
    config->retardo_commit        = config_get_int_value(cfg, "RETARDO_COMMIT");

    config_destroy(cfg);

    if ((int) config->log_level < 0) {
        fprintf(stderr, "LOG_LEVEL inválido\n");
        configuracion_destruir(config);
        return NULL;
    }

    if (!es_potencia_de_dos(config->tam_bloque) || config->tam_bloque < TAM_BLOQUE_MINIMO) {
        fprintf(stderr, "TAM_BLOQUE debe ser potencia de 2 y no menor a 32 bytes\n");
        configuracion_destruir(config);
        return NULL;
    }

    if (config->bloques_directorio == 0) {
        fprintf(stderr, "BLOQUES_DIRECTORIO debe ser mayor a 0\n");
        configuracion_destruir(config);
        return NULL;
    }

    if (!hay_lugar_para_bloques_de_datos(config)) {
        configuracion_destruir(config);
        return NULL;
    }

    return config;
}

void configuracion_destruir(t_storage_config* config) {
    if (config == NULL) {
        return;
    }
    free(config->path_storage);
    free(config);
}
