#include <stdlib.h>
#include "globals.h"

t_log*         logger      = NULL;
t_core_config* core_config = NULL;
char*          core_id     = NULL;

int fd_planificador = -1;
int fd_placa        = -1;
