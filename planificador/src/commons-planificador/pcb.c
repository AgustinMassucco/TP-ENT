#include "pcb.h"

t_pcb* pcb_create (uint32_t jid, const char* path, double estimacion_inicial, t_log* logger){
    t_pcb* pcb = malloc(sizeof(t_pcb));
        if (pcb == NULL){log_error(logger,"[ERROR] - Fallo al crear el pcb del job %d",jid); return NULL;}
    
        pcb->jid = jid;
        pcb->estado = STATE_NEW;
        pcb->path_instruccion = strdup(path);
        pcb ->socket_core_asociado = -1; //Como todavia no tienen ningún CORE asignado es -1

        memset(&(pcb->registros),0,sizeof(t_registers)); // Inicializa todos los registros con 0

        memset(&(pcb->estadisticas),0,sizeof(t_job_stats)); // Inicializa todas las estadisticas en 0
        pcb->estadisticas.tiempo_ingreso_new = atoi(temporal_get_string_time("%H:%M:%S:%MS")); //Hora de llegada a la cola de NEW

        pcb->estimacion_proxima_rafaga = estimacion_inicial;
        pcb->timestamp_llegada_ready = 0; 

        log_debug(logger,"[DEBUG] - PCB del job %d inicializado",jid);
        return pcb;
}

void pcb_change_state (t_pcb* pcb, t_state new_state, t_log* logger){
    if (pcb == NULL){log_error(logger,"[ERROR] - No se reconoce el pcb del job %d",pcb->jid); return;}
    
    //-------------------------------- LOG OBLIGATORIO --------------------------------

    log_info(logger, "##(%d) Pasa del estado %s al estado %s", pcb->jid, state_to_string(pcb->estado) , state_to_string(new_state));
    
    pcb->estado = new_state;
}

const char* state_to_string(t_state estado){
    switch (estado) {
        case STATE_NEW: return "NEW";
        case STATE_READY: return "READY";
        case STATE_EXEC: return "EXEC";
        case STATE_BLOCK: return "BLOCK";
        case STATE_EXIT: return "EXIT";
        default: return "UNKNOWN";
    }
}

void pcb_destroy (t_pcb* pcb){
    if (pcb == NULL) return;
    if (pcb->path_instruccion) free(pcb->path_instruccion);
    free(pcb);
}