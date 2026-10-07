#ifndef OP_CODES_H_
#define OP_CODES_H_

/**
 * Códigos de operación del protocolo.
 *
 * Convención de payload: cada campo se agrega con add_to_packet() y se lee con
 * receive_packet() en el mismo orden. Los enteros son uint32_t (mismo tipo que
 * los registros del Core); los strings viajan con su '\0' final.
 *
 * Respuestas genéricas:
 *   MSG_OK    -> operación exitosa (con o sin payload, según la operación)
 *   MSG_ERROR -> [char* motivo]
 */
typedef enum {
    // --- HANDSHAKES (payload vacío, se responde con MSG_OK) ---
    HANDSHAKE_CORE = 1,
    HANDSHAKE_PLANIFICADOR = 2,
    HANDSHAKE_PLACA = 3,
    HANDSHAKE_STORAGE = 4,

    // --- RESPUESTAS GENERICAS ---
    MSG_OK = 5,
    MSG_ERROR = 6,

    // --- OPERACIONES QUE ATIENDE LA PLACA (rango 100-199) ---

    // Planificador -> Placa: [uint32 jid][char* archivo_pseudocodigo]
    // Respuesta: MSG_OK | MSG_ERROR
    PLACA_CREAR_JOB = 100,

    // Core -> Placa: [uint32 jid][uint32 pc]
    // Respuesta: MSG_OK [char* instruccion] | MSG_ERROR
    PLACA_OBTENER_INSTRUCCION = 101,

    // Planificador -> Placa: [uint32 jid][uint32 tamanio]
    // Respuesta: MSG_OK [uint32 direccion_logica] | MSG_ERROR (sin espacio)
    PLACA_ALLOC = 102,

    // Planificador -> Placa: [uint32 jid][uint32 direccion_logica]
    // Respuesta: MSG_OK | MSG_ERROR
    PLACA_FREE = 103,

    // Core -> Placa: [uint32 jid][uint32 nro_pagina]
    // Respuesta: MSG_OK [uint32 nro_marco] (pagina queda lockeada) | PLACA_PAGE_FAULT
    PLACA_OBTENER_MARCO = 104,

    // Planificador -> Placa: [uint32 jid][uint32 nro_pagina]
    // Respuesta: MSG_OK | MSG_ERROR
    PLACA_CARGAR_PAGINA = 105,

    // Core/Planificador -> Placa: [uint32 jid][uint32 direccion_fisica][uint32 tamanio]
    // Respuesta: MSG_OK [bytes contenido] | MSG_ERROR
    PLACA_LEER = 106,

    // Core/Planificador -> Placa: [uint32 jid][uint32 direccion_fisica][uint32 tamanio][bytes contenido]
    // Respuesta: MSG_OK | MSG_ERROR
    PLACA_ESCRIBIR = 107,

    // Core/Planificador -> Placa: [uint32 jid]
    // Respuesta: MSG_OK
    PLACA_DESLOCKEAR_PAGINAS = 108,

    // Planificador -> Placa: [uint32 jid]
    // Respuesta: MSG_OK | MSG_ERROR
    PLACA_FINALIZAR_JOB = 109,

    // Placa -> Core: respuesta a PLACA_OBTENER_MARCO cuando la página no está residente (sin payload)
    PLACA_PAGE_FAULT = 110,

    // --- OPERACIONES QUE ATIENDE EL STORAGE (rango 200-299) ---
    // El tamaño del contenido viaja como campo propio porque receive_packet()
    // devuelve los campos sin sus longitudes.

    // Planificador -> Storage: [char* nombre][uint32 tamanio][bytes contenido]
    // Respuesta: MSG_OK | MSG_ERROR (sin espacio en FAT o directorio lleno)
    STORAGE_GUARDAR_CHECKPOINT = 200,

    // Planificador -> Storage: [char* nombre]
    // Respuesta: MSG_OK [uint32 tamanio][bytes contenido] | MSG_ERROR (no existe)
    STORAGE_CARGAR_CHECKPOINT = 201,

    // Planificador -> Storage: [char* nombre]
    // Respuesta: MSG_OK | MSG_ERROR (no existe, el Job no se finaliza)
    STORAGE_ELIMINAR_CHECKPOINT = 202,

} t_msg_code;

#endif /* OP_CODES_H_*/
