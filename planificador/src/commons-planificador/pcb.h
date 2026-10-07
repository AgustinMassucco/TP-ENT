#ifndef PCB_H
#define PCB_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> 
#include <string.h>
#include <commons/log.h>
#include <commons/temporal.h>

/* ------------------- Estructuras ------------------- */

//Estados de un job
/**
 * @brief Estructura para los posibles estados de un job
 * */
typedef enum {
    STATE_NEW,
    STATE_READY,
    STATE_EXEC,
    STATE_BLOCK,
    STATE_EXIT
}t_state;

/**
* @brief Estructura con los registros de un job
*/
typedef struct {
    uint32_t PC; //Program Counter
    uint32_t P0, P1, P2, P3, P4, P5; //Direcciones lógicas
    uint32_t AX, BX, CX, DX; //Registros númericos de propositos general
    uint32_t E1, E2, E3; //Operando de las instrucciones de entrenamiento
}t_registers;

//Estadisticas de un job
typedef struct {
    int64_t tiempo_ingreso_new; //Cuando ingresó a la cola de new
    int64_t tiempo_ingreso_ejecucion; //Cuando comenzó a ejecutarse
    int64_t tiempo_fin_ejecucion; //Cuando finalizó la ejecución
    int64_t tiempo_acumulado_ready; //Cuanto estuvo en ready
    
    int32_t max_memoria_dinamica; //Máximo de memoria dinamica ocupada en un momento
    
    int32_t cantidad_syscalls; //Cantidad de syscalls solicitadas
    int32_t cantidad_page_faults; //Cantidad de page faults generados
    int32_t transiciones_ready_exec; //Cantidad de transiciones de STATE_READY => STATE_EXEC
}t_job_stats;

//PCB
typedef struct{
    uint32_t jid; //Job id
    t_state estado; //Estado 
    char* path_instruccion; //Path instrucciones
    t_registers registros; //Registros
    t_job_stats estadisticas; //Estadisticas 
    int socket_core_asociado; //Socket al core que está trabajando este job. -1 Si no tiene ningun core asociado

    double estimacion_proxima_rafaga; // HRRN
    int64_t timestamp_llegada_ready; // Para acumulado tiempo_acumulado_ready de t_job_stats estadisticas
}t_pcb;


/* ------------------- FUNCIONES ------------------- */

/** 
* @brief Inicializa el pcb de un job con el id, el path del pseudocodigo y una estimacion para HRRN. Pone todos los registros en 0 y la llegada de ready en 0
* @param jid Job id del job para el que se está creando el pcb
* @param path Path al pseudocodigo del job
* @param estimacion_inicial Estimacion inicial para HRRN, se lee desde el archivo config del planificador
* @param logger Logger activo
* @return Retorna un puntero al pcb inicializado o NULL si falló la creacion.
*/
t_pcb* pcb_create (uint32_t jid, const char* path, double estimacion_inicial,t_log* logger);

/**
 * @brief Cambia el estado del job
 * @param pcb Pcb del job que cambia de estado
 * @param new_sate Estado al que cambia 
 * @param logger Logger activo
 */
void pcb_change_state (t_pcb* pcb, t_state new_state, t_log* logger);

/** 
 * @brief Traduce el estado recibido a string
 * @param estado Estado que traducirá a string
 * @return El equivalente en string al estado pasado como parametro o "UNKNOWN" si no se reconoce el estado
*/
const char* state_to_string(t_state estado);

/**
 * @brief Libera la memoria del puntero a un pcb
 * @param pcb Puntero al pcb que se quiere liberar la memoria
 */
void pcb_destroy (t_pcb* pcb);

#endif // PCB_H
