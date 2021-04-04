#ifndef _CAMHANDLER_H_
#define _CAMHANDLER_H_

#include "esp_err.h"

esp_err_t init_cam();
void shutdown_cam();
void camhandler_load_config_from_nvs();

#endif