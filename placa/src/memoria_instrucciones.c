#include "memoria_instrucciones.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include "globals.h"

typedef struct {
    uint32_t jid;
    char**   instrucciones; // string array terminado en NULL (commons/string.h)
    int      cantidad;
} t_job_instrucciones;

// Clave: JID como string (dictionary de commons solo acepta char*)
static t_dictionary*   jobs;
static pthread_mutex_t mutex_jobs = PTHREAD_MUTEX_INITIALIZER;

static char* clave_jid(uint32_t jid) {
    return string_from_format("%u", jid);
}

static void destruir_job(void* elemento) {
    t_job_instrucciones* job = elemento;
    string_array_destroy(job->instrucciones);
    free(job);
}

void memoria_instrucciones_init(void) {
    jobs = dictionary_create();
}

void memoria_instrucciones_destroy(void) {
    pthread_mutex_lock(&mutex_jobs);
    dictionary_destroy_and_destroy_elements(jobs, destruir_job);
    jobs = NULL;
    pthread_mutex_unlock(&mutex_jobs);
}

/**
 * Lee el archivo línea por línea y arma el array de instrucciones. Se ignoran
 * líneas vacías y se descarta el '\r' de archivos con fin de línea Windows.
 * @return Array de strings o NULL si no se pudo abrir el archivo.
 */
static char** leer_instrucciones(char* path) {
    FILE* archivo = fopen(path, "r");
    if (archivo == NULL) {
        return NULL;
    }

    char** instrucciones = string_array_new();
    char*  linea         = NULL;
    size_t capacidad     = 0;
    ssize_t leidos;

    while ((leidos = getline(&linea, &capacidad, archivo)) != -1) {
        while (leidos > 0 && (linea[leidos - 1] == '\n' || linea[leidos - 1] == '\r')) {
            linea[--leidos] = '\0';
        }
        if (leidos == 0) {
            continue;
        }
        string_array_push(&instrucciones, strdup(linea));
    }

    free(linea);
    fclose(archivo);
    return instrucciones;
}

/**
 * El nombre llega por red y se concatena al PATH_INSTRUCCIONES: se rechaza todo lo
 * que permita salir de ese directorio (path absoluto o componentes "..").
 */
static bool nombre_archivo_valido(char* nombre) {
    if (string_is_empty(nombre) || nombre[0] == '/') {
        return false;
    }
    char** partes = string_split(nombre, "/");
    bool valido = true;
    for (int i = 0; partes[i] != NULL; i++) {
        if (strcmp(partes[i], "..") == 0) {
            valido = false;
            break;
        }
    }
    string_array_destroy(partes);
    return valido;
}

int memoria_instrucciones_crear_job(uint32_t jid, char* nombre_archivo) {
    if (!nombre_archivo_valido(nombre_archivo)) {
        log_error(logger, "JID: %u - Nombre de archivo de pseudocódigo inválido: %s", jid, nombre_archivo);
        return -1;
    }

    char* path = string_from_format("%s%s", placa_config->path_instrucciones, nombre_archivo);
    char** instrucciones = leer_instrucciones(path);
    if (instrucciones == NULL) {
        log_error(logger, "JID: %u - No se pudo abrir el archivo de pseudocódigo: %s", jid, path);
        free(path);
        return -1;
    }
    free(path);

    t_job_instrucciones* job = malloc(sizeof(t_job_instrucciones));
    job->jid           = jid;
    job->instrucciones = instrucciones;
    job->cantidad      = string_array_size(instrucciones);

    char* clave = clave_jid(jid);
    int resultado = 0;

    pthread_mutex_lock(&mutex_jobs);
    if (dictionary_has_key(jobs, clave)) {
        resultado = -1;
    } else {
        dictionary_put(jobs, clave, job);
    }
    pthread_mutex_unlock(&mutex_jobs);

    if (resultado == -1) {
        log_error(logger, "JID: %u - Ya existe un Job con ese ID", jid);
        destruir_job(job);
    }
    free(clave);
    return resultado;
}

char* memoria_instrucciones_obtener(uint32_t jid, uint32_t pc) {
    char* clave = clave_jid(jid);
    char* instruccion = NULL;

    pthread_mutex_lock(&mutex_jobs);
    t_job_instrucciones* job = dictionary_get(jobs, clave);
    if (job != NULL && pc < (uint32_t) job->cantidad) {
        instruccion = strdup(job->instrucciones[pc]);
    }
    pthread_mutex_unlock(&mutex_jobs);

    free(clave);
    return instruccion;
}

int memoria_instrucciones_finalizar_job(uint32_t jid) {
    char* clave = clave_jid(jid);
    int resultado = -1;

    pthread_mutex_lock(&mutex_jobs);
    if (dictionary_has_key(jobs, clave)) {
        dictionary_remove_and_destroy(jobs, clave, destruir_job);
        resultado = 0;
    }
    pthread_mutex_unlock(&mutex_jobs);

    free(clave);
    return resultado;
}

bool memoria_instrucciones_existe_job(uint32_t jid) {
    char* clave = clave_jid(jid);
    pthread_mutex_lock(&mutex_jobs);
    bool existe = dictionary_has_key(jobs, clave);
    pthread_mutex_unlock(&mutex_jobs);
    free(clave);
    return existe;
}

int memoria_instrucciones_cantidad_jobs(void) {
    pthread_mutex_lock(&mutex_jobs);
    int cantidad = dictionary_size(jobs);
    pthread_mutex_unlock(&mutex_jobs);
    return cantidad;
}

int memoria_instrucciones_cantidad(uint32_t jid) {
    char* clave = clave_jid(jid);
    int cantidad = -1;
    pthread_mutex_lock(&mutex_jobs);
    t_job_instrucciones* job = dictionary_get(jobs, clave);
    if (job != NULL) {
        cantidad = job->cantidad;
    }
    pthread_mutex_unlock(&mutex_jobs);
    free(clave);
    return cantidad;
}

// dictionary_iterator no recibe un argumento extra, por eso la lista destino es estática
// (solo se usa con mutex_jobs tomado)
static t_list* lista_en_construccion;

static void agregar_jid(char* clave, void* elemento) {
    uint32_t* jid = malloc(sizeof(uint32_t));
    *jid = ((t_job_instrucciones*) elemento)->jid;
    list_add(lista_en_construccion, jid);
}

static bool jid_menor(void* a, void* b) {
    return *(uint32_t*) a < *(uint32_t*) b;
}

t_list* memoria_instrucciones_listar_jids(void) {
    pthread_mutex_lock(&mutex_jobs);
    lista_en_construccion = list_create();
    dictionary_iterator(jobs, agregar_jid);
    t_list* resultado = lista_en_construccion;
    lista_en_construccion = NULL;
    pthread_mutex_unlock(&mutex_jobs);

    list_sort(resultado, jid_menor);
    return resultado;
}
