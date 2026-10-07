#ifndef MESSAGING_PACKET_H_
#define MESSAGING_PACKET_H_

#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include "codes.h"

typedef struct {
    int size;
    void* stream;
} t_buffer;

typedef struct {
    t_msg_code op_code;
    t_buffer* buffer;
} t_packet;

/**
 * @brief Crea un paquete vacío con el código de operación dado.
 *        El buffer interno se inicializa con size=0 y stream=NULL.
 *
 * @param op_code Código de operación que identifica el tipo de mensaje.
 * @return Paquete alocado en heap. Debe liberarse con destroy_packet().
 */
t_packet* create_packet(t_msg_code op_code);

/**
 * @brief Agrega un campo al buffer del paquete.
 *        Internamente almacena [tamaño (4 bytes)][datos (tamaño bytes)] por campo.
 *
 * @param packet Paquete al que se agrega el campo.
 * @param value  Puntero al dato a agregar.
 * @param size   Tamaño en bytes del dato.
 */
void add_to_packet(t_packet* packet, void* value, int size);

/**
 * @brief Serializa y envía un paquete completo por el socket.
 *        Formato enviado: [op_code (4B)][buffer_size (4B)][buffer_stream].
 *
 * @param packet Paquete a enviar. El caller sigue siendo responsable de
 *               liberarlo con destroy_packet().
 * @param fd     File descriptor del socket server destino.
 * @return 0 en caso de éxito, -1 en caso de error.
 */
int send_packet(t_packet* packet, int fd);

/**
 * @brief Recibe el código de operación de un mensaje entrante.
 *        No cierra el socket (cada caller ya lo cierra por su cuenta) en caso de error
 *
 * @param fd File descriptor del socket origen.
 * @return Código de operación, o -1 en caso de error. (recv fallo o lectura parcial)
 */
int receive_operation(int fd);

/**
 * @brief Recibe un buffer crudo desde el socket.
 *        Devuelve NULL si la conexión se perdió durante la recepción.
 *        El buffer es alocado en heap. El caller es responsable de liberarlo.
 *
 * @param size Puntero donde se escribirá el tamaño del buffer recibido.
 * @param fd   File descriptor del socket origen.
 * @return Buffer alocado en heap, o NULL en caso de error.
 */
void* receive_buffer(int* size, int fd);

/**
 * @brief Recibe un paquete y deserializa sus campos en una lista.
 *        Devuelve NULL si receive_buffer() falla (conexión perdida).
 *        Cada elemento de la lista es un campo alocado en heap. El caller
 *        es responsable de liberar con list_destroy_and_destroy_elements().
 *        Cada campo se aloca con al menos sizeof(uint32_t) bytes y un byte extra en 0,
 *        por lo que siempre es seguro leerlo como uint32_t o como string con '\0'.
 *
 * @param fd File descriptor del socket origen.
 * @return t_list* con los campos del paquete, o NULL en caso de error.
 */
t_list* receive_packet(int fd);

/**
 * @brief Libera toda la memoria asociada a un paquete.
 *        Libera el stream interno, el buffer y el paquete mismo.
 *
 * @param packet Paquete previamente alocado en heap.
 */
void destroy_packet(t_packet* packet);

/**
 * @brief Envía un acknowledgment MSG_OK sin payload.
 *
 * @param fd File descriptor del socket destino.
 * @return 0 en caso de éxito, -1 en caso de error.
 */
int send_ack(int fd);

/**
* @brief Envía un mensaje MSG_ERROR con un string como payload.
*        El receptor puede leerlo con receive_ack_or_error() o receive_response_or_error().
*
* @param fd      File descriptor del socket destino.
* @param message Mensaje de error terminado en '\0'.
* @return 0 en caso de éxito, -1 en caso de error.
*/
int send_error(int fd, char* message);


/**
 * @brief Recibe y valida una respuesta del servidor que puede ser MSG_OK o MSG_ERROR.
 *        Drena siempre el buffer en los casos esperados para mantener la secuencia de bytes del socket alineada con el protocolo.
 *
 * @param fd File descriptor del socket origen.
 * @param logger Logger del modulo cliente, donde se imprime el mensaje recibido.
 * @return 0 si MSG_OK, -1 si MSG_ERROR u op_code desconocido.
 */
int receive_ack_or_error(int fd, t_log* logger);

/**
 * @brief Recibe la siguiente operacion esperando MSG_OK con payload o MSG_ERROR
 *        Variante de receive_ack_or_error para request-response (ej. GET_CONTEXT, GET_INSTRUCTION) caller lee el payload
 *
 * @param fd File descriptor del socket origen.
 * @param logger Logger del modulo cliente.
 * @return MSG_OK si el caller debe leer el payload con receive_packet(), -1 si MSG_ERROR o desconexion.
 */
int receive_response_or_error(int fd, t_log* logger);

#endif /* MESSAGING_PACKET_H_ */