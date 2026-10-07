#include "utils/common_utils.h"

t_log_level config_get_log_level(t_config* config) {
    if (config == NULL || !config_has_property(config, "LOG_LEVEL")) {
        return LOG_LEVEL_INFO; // Default 
    }

    char* log_level_str = config_get_string_value(config, "LOG_LEVEL");
    if (log_level_str == NULL) {
        return LOG_LEVEL_INFO; // Default
    }
    
    return log_level_from_string(log_level_str); 
}

t_log* initialize_logger(char* file, char* module_name, t_config* config) {
    t_log_level log_level = config_get_log_level(config);

    t_log* logger = log_create(file, module_name, true, log_level);
    if(logger == NULL) {
        fprintf(stderr, "ERROR: No se pudo crear el logger.\n");
        return NULL;
    }

    log_info(logger, "## Logger incializado");
    return logger;
}