#ifndef PLACA_OPERACIONES_H_
#define PLACA_OPERACIONES_H_

#include <utils/messaging/codes.h>

/**
 * @brief Atiende una operación ya identificada por su op_code: lee el payload
 *        del socket, ejecuta la operación y envía la respuesta al cliente.
 *
 * @param fd            Socket del cliente (Core o Planificador).
 * @param op_code       Código de operación recibido con receive_operation().
 * @param nombre_modulo Nombre del módulo cliente, para los logs.
 * @return 0 si se pudo procesar (aunque la respuesta haya sido MSG_ERROR),
 *         -1 si se perdió la conexión al leer el payload.
 */
int operaciones_atender(int fd, t_msg_code op_code, const char* nombre_modulo);

#endif /* PLACA_OPERACIONES_H_ */
