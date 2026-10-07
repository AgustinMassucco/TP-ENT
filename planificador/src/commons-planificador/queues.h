#ifndef QUEUES_H
#define QUEUES_H

#include <pthread.h>
#include <stdlib.h>
#include <commons/collections/queue.h>

/**
* @brief Estructura que vincula una queue con un mutex para que sea seguro 
*/
typedef struct {
    t_queue* queue;
    pthread_mutex_t mutex;
}t_safe_queue;

/** 
* @brief Crea una t_safe_queue, inicializa su mutex y la devuelve
* @return Puntero a la safe_queue creada, NULL si hubo un error
*/
t_safe_queue* safe_queue_create();

/** 
* @brief realiza un safe push de la queue. Bloquea el mutex, hace el push y luego libera el mutex
* @param safe_queue Puntero a la safe_queue
* @param element Elemento a pushear 
*/
void safe_queue_push(t_safe_queue* safe_queue, void* element);


/** 
* @brief realiza un safe pop de la queue. Bloquea el mutex, hace el pop y luego libera el mutex
* @param safe_queue Puntero a la safe_queue
* @return Elemento que se sacó de la queue, NULL si hubo un error
*/
void* safe_queue_pop(t_safe_queue* safe_queue); 

/** 
* @brief realiza un safe is_empty de la queue. Bloquea el mutex, hace el is_empty y luego libera el mutex
* @param safe_queue Puntero a la safe_queue
* @return True si la queue está vacía o hubo un error y false si no lo está
*/
bool safe_queue_is_empty(t_safe_queue* safe_queue);

/** 
* @brief Bloquea el mutex, hace el queue_size y luego libera el mutex
* @param safe_queue Puntero a la safe_queue
* @return el tamaño de la queue, -1 si hubo un error
*/
int safe_queue_size(t_safe_queue* safe_queue);


/**
* @brief Bloquea el mutex, y realiza un queue_destroy, luego libera el mutex y lo elimina.
* @param safe_queue Puntero a la safe_queue
*/
void safe_queue_destroy(t_safe_queue* safe_queue);

#endif //QUEUES_H