#include "queues.h"

t_safe_queue* safe_queue_create() {
    t_safe_queue* safe_queue = malloc(sizeof(t_safe_queue));
    if (safe_queue == NULL) return NULL;

    safe_queue->queue = queue_create();
    pthread_mutex_init(&safe_queue->mutex, NULL);
    return safe_queue;
}

void safe_queue_push(t_safe_queue* safe_queue, void* element) {
    if (safe_queue == NULL) return;

    //------- Zona Crítica -------
    pthread_mutex_lock(&safe_queue->mutex);
    queue_push(safe_queue->queue, element);
    pthread_mutex_unlock(&safe_queue->mutex);
    //------- Fin Zona Crítica -------
}

void* safe_queue_pop(t_safe_queue* safe_queue) {
    if (safe_queue == NULL) return NULL;

    //------- Zona Crítica -------
    pthread_mutex_lock(&safe_queue->mutex);
    void* element = queue_pop(safe_queue->queue);
    pthread_mutex_unlock(&safe_queue->mutex);
    //------- Fin Zona Crítica -------

    return element;
}

bool safe_queue_is_empty(t_safe_queue* safe_queue) {
    if (safe_queue == NULL) return true;

    //------- Zona Crítica -------
    pthread_mutex_lock(&safe_queue->mutex);
    bool is_empty = queue_is_empty(safe_queue->queue);
    pthread_mutex_unlock(&safe_queue->mutex);
    //------- Fin Zona Crítica -------

    return is_empty;
}

int safe_queue_size(t_safe_queue* safe_queue) {
    if (safe_queue == NULL) return -1;

    //------- Zona Crítica -------
    pthread_mutex_lock(&safe_queue->mutex);
    int size = queue_size(safe_queue->queue);
    pthread_mutex_unlock(&safe_queue->mutex);
    //------- Fin Zona Crítica -------

    return size;
}

void safe_queue_destroy(t_safe_queue* safe_queue) {
    if (safe_queue == NULL) return;

    //------- Zona Crítica -------
    pthread_mutex_lock(&safe_queue->mutex);
    queue_destroy(safe_queue->queue);
    pthread_mutex_unlock(&safe_queue->mutex);
    //------- Fin Zona Crítica -------

    pthread_mutex_destroy(&safe_queue->mutex);
    free(safe_queue);
}