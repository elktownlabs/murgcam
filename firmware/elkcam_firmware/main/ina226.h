#ifndef _INA226_H_
#define _INA226_H_

#include "esp_err.h"

void ina226_init();
esp_err_t ina226_single_read_voltage(double* vsupply, double* vshunt);
esp_err_t ina226_single_read_current_power(double* current, double* power);
esp_err_t in226_read_devicedie_id(uint16_t* chip_id, uint8_t* die_id);
esp_err_t in226_read_manufacturer_id(uint16_t* id);
esp_err_t ina226_mode_shutdown();
esp_err_t ina226_mode_continuous();
void ina226_start_sampling();
void ina226_stop_sampling();
esp_err_t ina226_get_sampling_values(double* min_voltage_dest, double* avg_current_dest);
esp_err_t ina226_get_startup_values(double* vsupply_on_startup_dest, double* vshunt_on_startup_dest,
    double* current_on_startup_dest, double* power_on_startup_dest);

#endif