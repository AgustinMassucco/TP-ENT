#include <stdlib.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include "utils/common_utils.h"
#include "client/client.h"
#include "server/server.h"

static void init_module_planificador(char* config_path, t_log** logger, t_config** config);

int main(int argc, char* argv[]) {
    
    if (argc < 3) {
        fprintf(stderr, "ERROR: faltan argumentos.\n");
        fprintf(stderr, "Uso correcto: ./bin/planificador [Archivo Config] [Path Job Inicial]\n");
        exit(EXIT_FAILURE);
    }

    char* job_inicial_path = argv[2];

    /*-------------- LOG Y CONFIG --------------*/
    t_log* logger = NULL;
    t_config* config = NULL;
    init_module_planificador(argv[1], &logger, &config);

    if (access(job_inicial_path, F_OK) != 0) {
        log_error(logger, "No se encontro el archivo de Job inicial: %s", job_inicial_path);
        log_destroy(logger);
        config_destroy(config);
        exit(EXIT_FAILURE);
    } else {
        log_info(logger, "Path del Job inicial recibido: %s", job_inicial_path);
    }

    /*-------------- CONEXION A PLACA --------------*/
    int fd_placa = connect_to_placa(config, logger);
    if (fd_placa == -1) {
        log_error(logger, "ERROR: No se pudo conectar a Placa");
        close(fd_placa); 
        log_destroy(logger);
        config_destroy(config);
        exit(EXIT_FAILURE);
    }

    /*-------------- CONEXION A STORAGE --------------*/
    int fd_storage = connect_to_storage(config, logger);
    if (fd_storage == -1) {
        log_error(logger, "ERROR: No se pudo conectar a Storage");
        close(fd_storage);
        log_destroy(logger);
        config_destroy(config);
        exit(EXIT_FAILURE);
    }

    /*-------------- SERVIDOR MULTIHILO PARA CORES --------------*/
    start_core_server(config, logger); // Bloquea aceptando Cores indefinidamente


    close(fd_placa); 
    close(fd_storage);
    log_destroy(logger);
    config_destroy(config);
    return 0;
}


/**
 * @brief Carga el archivo de  configuracion, incializa el logger y valida que todas las propiedades requeridas esten presentes.
 *        Termina el proceso con exit(EXIT_FAILURE) si alguno falla.
 * @param config_path Ruta al archivo de configuracion
 * @param logger Puntero al logger que se inicializara
 * @param config Puntero al archivo de configuracion que se inicializara
 */
static void init_module_planificador(char* config_path, t_log** logger, t_config** config) {

    *config = config_create(config_path);
    if (*config == NULL) {
        fprintf(stderr, "ERROR: No se pudo encontrar el archivo de configuracion.\n");
        exit(EXIT_FAILURE);
    }

    *logger = initialize_logger("planificador.log", "PLANIFICADOR", *config);
    if (*logger == NULL) {
        fprintf(stderr, "ERROR: No se pudo crear el logger.\n");
        config_destroy(*config);
        exit(EXIT_FAILURE);
    }

    const char* required[] = {
            "PUERTO_ESCUCHA",
            "IP_PLACA",
            "PUERTO_PLACA",
            "IP_STORAGE",
            "PUERTO_STORAGE",
            "LOG_LEVEL",
            "ALGORITMO_PLANIFICACION",
            "RR_QUANTUM",
            "ESTIMACION_INICIAL",
            "HRRN_ALFA",
            "GRADO_MULTIPROGRAMACION",
            "PATH_DATOS",
            "RETARDO_LOADER",
            "PATH_REPORTES"
    };

    for (size_t i = 0; i < sizeof(required) / sizeof(required[0]); i++) {
        if (!config_has_property(*config, required[i])) {
            fprintf(stderr, "ERROR: La propiedad '%s' no se encuentra en el archivo de configuracion.\n", required[i]);
            log_destroy(*logger);
            config_destroy(*config);
            exit(EXIT_FAILURE);
        }
    }
}
