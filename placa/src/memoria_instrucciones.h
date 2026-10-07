#ifndef PLACA_MEMORIA_INSTRUCCIONES_H_
#define PLACA_MEMORIA_INSTRUCCIONES_H_

#include <stdint.h>
#include <stdbool.h>
#include <commons/collections/list.h>

/**
 * Memoria de Instrucciones: asocia cada Job (por JID) con las instrucciones de
 * su archivo de pseudocódigo. Capacidad ilimitada (no ocupa la memoria de usuario).
 *
 * Todas las operaciones son thread-safe: la atienden en paralelo los hilos de
 * cada Core y del Planificador.
 */

void memoria_instrucciones_init(void);
void memoria_instrucciones_destroy(void);

/**
 * @brief Crea las estructuras del Job y carga en memoria las instrucciones del archivo
 *        <PATH_INSTRUCCIONES>/<nombre_archivo>. Una instrucción por línea ('\n').
 * @return 0 si se creó, -1 si el JID ya existe o no se pudo leer el archivo.
 */
int memoria_instrucciones_crear_job(uint32_t jid, char* nombre_archivo);

/**
 * @brief Devuelve la instrucción número `pc` (base 0) del Job.
 * @return Copia en heap de la instrucción (el caller hace free), o NULL si el
 *         JID no existe o el PC está fuera de rango.
 */
char* memoria_instrucciones_obtener(uint32_t jid, uint32_t pc);

/**
 * @brief Libera las instrucciones y estructuras del Job.
 * @return 0 si existía, -1 si no.
 */
int memoria_instrucciones_finalizar_job(uint32_t jid);

bool memoria_instrucciones_existe_job(uint32_t jid);

int memoria_instrucciones_cantidad_jobs(void);

/**
 * @brief Lista los JIDs registrados, ordenados ascendentemente.
 * @return t_list* de uint32_t* en heap. Liberar con list_destroy_and_destroy_elements(l, free).
 */
t_list* memoria_instrucciones_listar_jids(void);

/** @return Cantidad de instrucciones del Job, o -1 si no existe. */
int memoria_instrucciones_cantidad(uint32_t jid);

#endif /* PLACA_MEMORIA_INSTRUCCIONES_H_ */
