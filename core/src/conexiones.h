#ifndef CORE_CONEXIONES_H_
#define CORE_CONEXIONES_H_

/**
 * @brief Se conecta al Planificador y realiza el handshake (HANDSHAKE_CORE).
 * @return File descriptor de la conexión, o -1 en caso de error.
 */
int conectar_a_planificador(void);

/**
 * @brief Se conecta a la Placa y realiza el handshake (HANDSHAKE_CORE).
 * @return File descriptor de la conexión, o -1 en caso de error.
 */
int conectar_a_placa(void);

#endif /* CORE_CONEXIONES_H_ */
