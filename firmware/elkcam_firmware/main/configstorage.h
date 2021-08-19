#ifndef _CONFIGSTORAGE_H_
#define _CONFIGSTORAGE_H_

#include <stdint.h>
#include "esp_system.h"
#include "esp_err.h"

typedef struct {
    uint8_t quality; /* 0 - 63 */
    uint8_t pixeltiming; /* 0 - 30 */
    uint8_t auto_exposure; /* 0 - 4 */
    uint8_t light_mode; /* 0 - 4 */
    uint8_t color_saturation;
    uint8_t brightness;
    uint8_t contrast;
    uint8_t hue;
    uint8_t sharpness;
} camera_config_t;


typedef struct {
    char pin[16];
    char apn[64];
    char apn_user[64];
    char apn_pass[64];
    uint8_t apn_auth;
    char remote_address[128];
    char remote_url[128];
} cell_config_t;


typedef struct {
    uint16_t secs_between_photos;
    uint16_t min_voltage;
    uint32_t photo_counter;
} system_config_t;


void config_load_values();
camera_config_t* config_cam();
cell_config_t* config_cell();
system_config_t* config_system();
void config_write_values(system_config_t* sys_conf, cell_config_t* cell_conf, camera_config_t* cam_conf);




#endif