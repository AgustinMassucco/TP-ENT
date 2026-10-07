# Placa

Módulo encargado de administrar la memoria del sistema: memoria de instrucciones
(pseudocódigo de los Jobs) y memoria de usuario (paginación bajo demanda + Offload).

```sh
make
./bin/placa placa.config
```

## Estructura

| Archivo | Responsabilidad |
|---|---|
| `src/main.c` | Inicialización, lanza el hilo aceptador y corre la consola |
| `src/configuracion.[ch]` | Carga y validación del archivo de configuración |
| `src/globals.[ch]` | `logger` y `placa_config` globales (solo lectura luego del init) |
| `src/servidor.[ch]` | Servidor multihilo: acepta conexiones, handshake, un hilo por cliente |
| `src/operaciones.[ch]` | Dispatcher de operaciones: lee el payload, ejecuta y responde |
| `src/memoria_instrucciones.[ch]` | JID → instrucciones del archivo de pseudocódigo (thread-safe) |
| `src/consola.[ch]` | Consola por stdin: `INFO`, `TLS`, `HELP`, `EXIT` |

Pendiente (checkpoint 3): `memoria_usuario` (tablas de páginas, marcos, locking,
reemplazo LRU / CLOCK-M) y `offload` (swap con acceso serializado).

## Protocolo

Los op codes están en `utils/src/utils/messaging/codes.h` (rango `PLACA_*`, 100-199).
Todo mensaje es un `t_packet`: los campos se agregan con `add_to_packet()` en el orden
indicado y se leen con `receive_packet()`. Los enteros son `uint32_t`; los strings
viajan con su `'\0'`.

**Handshake**: el cliente envía un paquete `HANDSHAKE_CORE` o `HANDSHAKE_PLANIFICADOR`
(payload vacío) y la Placa responde `MSG_OK`. Cualquier otro handshake se rechaza con
`MSG_ERROR` y se cierra la conexión.

| Operación | Quién | Payload pedido | Respuesta |
|---|---|---|---|
| `PLACA_CREAR_JOB` | Planificador | `[jid][char* archivo]` | `MSG_OK` / `MSG_ERROR` |
| `PLACA_OBTENER_INSTRUCCION` | Core | `[jid][pc]` | `MSG_OK [char* instruccion]` / `MSG_ERROR` |
| `PLACA_ALLOC` | Planificador | `[jid][tamanio]` | `MSG_OK [dir_logica]` / `MSG_ERROR` |
| `PLACA_FREE` | Planificador | `[jid][dir_logica]` | `MSG_OK` / `MSG_ERROR` |
| `PLACA_OBTENER_MARCO` | Core | `[jid][nro_pagina]` | `MSG_OK [nro_marco]` / `PLACA_PAGE_FAULT` |
| `PLACA_CARGAR_PAGINA` | Planificador | `[jid][nro_pagina]` | `MSG_OK` / `MSG_ERROR` |
| `PLACA_LEER` | Core / Planificador | `[jid][dir_fisica][tamanio]` | `MSG_OK [bytes]` / `MSG_ERROR` |
| `PLACA_ESCRIBIR` | Core / Planificador | `[jid][dir_fisica][tamanio][bytes]` | `MSG_OK` / `MSG_ERROR` |
| `PLACA_DESLOCKEAR_PAGINAS` | Core / Planificador | `[jid]` | `MSG_OK` |
| `PLACA_FINALIZAR_JOB` | Planificador | `[jid]` | `MSG_OK` / `MSG_ERROR` |

Del lado del cliente, para las operaciones con respuesta `MSG_OK` sin payload usar
`receive_ack_or_error()`; para las que devuelven payload, `receive_response_or_error()`
seguido de `receive_packet()`.

Ejemplo (Core pide una instrucción):

```c
t_packet* p = create_packet(PLACA_OBTENER_INSTRUCCION);
add_to_packet(p, &jid, sizeof(uint32_t));
add_to_packet(p, &pc,  sizeof(uint32_t));
send_packet(p, fd_placa);
destroy_packet(p);

if (receive_response_or_error(fd_placa, logger) == MSG_OK) {
    t_list* campos = receive_packet(fd_placa);
    char* instruccion = list_get(campos, 0);
    // ...
    list_destroy_and_destroy_elements(campos, free);
}
```

## Configuración

Ver `placa.config`. `PATH_INSTRUCCIONES` es el directorio base de los archivos de
pseudocódigo (los nombres que llegan en `PLACA_CREAR_JOB` son relativos a él).
