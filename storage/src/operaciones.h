#ifndef STORAGE_OPERACIONES_H_
#define STORAGE_OPERACIONES_H_

#include <utils/messaging/codes.h>

int operaciones_atender(int fd, t_msg_code op_code, const char* nombre_modulo);

#endif
