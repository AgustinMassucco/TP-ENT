#ifndef STORAGE_SERVIDOR_H_
#define STORAGE_SERVIDOR_H_

int servidor_iniciar(void);

void* servidor_aceptar_conexiones(void* arg);

#endif
