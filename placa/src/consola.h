#ifndef PLACA_CONSOLA_H_
#define PLACA_CONSOLA_H_

/**
 * @brief Consola interactiva por stdin. Comandos soportados:
 *          INFO  -> porcentaje de memoria/offload ocupada, frames libres/lockeados/totales, Jobs actuales
 *          TLS   -> lista de Jobs: ID, páginas residentes y páginas totales
 *          HELP  -> lista de comandos
 *          EXIT  -> termina la Placa
 *        Retorna cuando se cierra stdin o se ingresa EXIT.
 *
 * Firma compatible con pthread_create.
 */
void* consola_ejecutar(void* arg);

#endif /* PLACA_CONSOLA_H_ */
