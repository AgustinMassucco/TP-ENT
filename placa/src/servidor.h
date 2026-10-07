#ifndef PLACA_SERVIDOR_H_
#define PLACA_SERVIDOR_H_

/**
 * @brief Crea el socket de escucha en PUERTO_ESCUCHA.
 * @return 0 en éxito, -1 si no se pudo crear el servidor.
 */
int servidor_iniciar(void);

/**
 * @brief Loop de aceptación de conexiones. Por cada cliente lanza un hilo
 *        detached que atiende sus pedidos hasta que se desconecte.
 *        Nunca retorna: la escucha se mantiene activa toda la ejecución para
 *        permitir que se conecten Cores en cualquier momento.
 *
 * Firma compatible con pthread_create.
 */
void* servidor_aceptar_conexiones(void* arg);

#endif /* PLACA_SERVIDOR_H_ */
