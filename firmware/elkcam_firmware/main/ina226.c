#include "ina226.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"

static const char* TAG = "ina226";

// the vsupply voltage is read once during the boot sequence and stored here
// for later transmission to server. Negative means invalid.
static double vsupply_on_startup = -1.0;
static double vshunt_on_startup = -1.0;
static double current_on_startup = -1.0;
static double power_on_startup = -1.0;

// task handle for monitoring task
TaskHandle_t taskHandleIna226 = 0x00;

// chip configuration constants
#define AUX_I2C_SDA_PIN (37)
#define AUX_I2C_SCL_PIN (36)
#define AUX_I2C_FREQ (400*1000)
#define INA226_ADDR (0x40)

#define INA226_CALIBRATION_VALUE (1003)

// static functions
static esp_err_t in226_conversion_ready();
static void ina226_sampling_task(void *pvParameters);
static esp_err_t ina226_mode_continuous_averaging();
static esp_err_t ina226_wait_conversion_ready(uint8_t delay, uint8_t tries);
static esp_err_t ina226_write_calibration_reg(uint16_t value);

/* variables to store results from background sampling loop */
static double min_voltage = 20000.0; /* a very high value, overwritten after first sample is taken */
static double avg_current = 0.0;
static uint32_t num_current_samples = 0;


  /* i2c for ina226 access */
const static i2c_config_t ina226_conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = AUX_I2C_SDA_PIN,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_io_num = AUX_I2C_SCL_PIN,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = AUX_I2C_FREQ,
};


void ina226_init()
{
    esp_err_t err;

    /* init i2c */
	i2c_param_config(I2C_NUM_1, &ina226_conf);
	i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0);

    /* wake up the ic */
    ina226_mode_continuous();

    /* read chip ids */
    uint16_t ina226_manufacturer_id, ina226_chip_id;
    uint8_t ina226_die_id;
    double vsupply, vshunt, current, power;
    err = in226_read_devicedie_id(&ina226_chip_id, &ina226_die_id);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "INA226 chip id: %x, die id: %x", ina226_chip_id, ina226_die_id);
    } else {
        ESP_LOGI(TAG, "INA226 chip/id id could not be read (%d)", err);
    }
    err = in226_read_manufacturer_id(&ina226_manufacturer_id);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "INA226 manufacturer id: %x", ina226_manufacturer_id);
    } else {
        ESP_LOGI(TAG, "INA226 chip/id id could not be read (%d)", err);
    }

    /* set calibration register */
    err = ina226_write_calibration_reg(INA226_CALIBRATION_VALUE);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "INA226 calibration register could not be set (%d)", err);
    }

    /* read voltages, current and power at startup */
    err = ina226_wait_conversion_ready(10, 10);
    if (err == ESP_OK) {
        /* voltages */
        err = ina226_single_read_voltage(&vsupply, &vshunt);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Supply voltage upon boot: %.4fV", vsupply);
            ESP_LOGI(TAG, "Shunt voltage upon boot: %.2fmV", vshunt * 1000.0);
            vsupply_on_startup = vsupply;
            vshunt_on_startup = vshunt;
        } else {
            ESP_LOGE(TAG, "Voltages could not be read from sensor (%x)", err);
        }

        /* current and power */
        err = ina226_single_read_current_power(&current, &power);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Current upon boot: %.2fmA", (current * 1000.0));
            ESP_LOGI(TAG, "Power consumption upon boot: %.2fmW", (power * 1000.0));
            current_on_startup = current;
            power_on_startup = power;
        } else {
            ESP_LOGE(TAG, "Current and power could not be read from sensor (%x)", err);
        }

    } else {
        ESP_LOGE(TAG, "Startup values could not be read");
    }

    /* create power / current monitoring tasks */
    BaseType_t ret = xTaskCreate(ina226_sampling_task, "cam_ina226", 4096, NULL, tskIDLE_PRIORITY+1, &taskHandleIna226);
    if(ret != pdPASS) {
        ESP_LOGE(TAG, "Unable to create current / power monitoring task");
    }
}


esp_err_t in226_read_manufacturer_id(uint16_t* id)
{
    union {
        uint16_t id;
        uint8_t data[2];
    } chipid;
    i2c_cmd_handle_t cmd;

    /* read id */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xfe, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &chipid.data[1], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &chipid.data[0], I2C_MASTER_ACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
    if (err == ESP_OK) if (id) *id = chipid.id;

	i2c_cmd_link_delete(cmd);
    return err;
}


esp_err_t in226_read_devicedie_id(uint16_t* chip_id, uint8_t* die_id)
{
    union {
        struct
        {
            unsigned chip : 12;
            unsigned die : 4;
        } id;
        uint8_t data[2];
    } chipid;
    i2c_cmd_handle_t cmd;

    /* read id */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xfe, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &chipid.data[1], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &chipid.data[0], I2C_MASTER_ACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
    if (err == ESP_OK) {
        if (chip_id) *chip_id = chipid.id.chip;
        if (die_id) *die_id = chipid.id.die;
    }

	i2c_cmd_link_delete(cmd);
    return err;
}


esp_err_t ina226_single_read_voltage(double* vsupply, double* vshunt)
{
    union {
        int16_t voltage;
        uint8_t data[2];
    } voltage;
    i2c_cmd_handle_t cmd;

    esp_err_t err_vsupply = ESP_OK;
    esp_err_t err_vshunt = ESP_OK;

    /* read vsupply voltage */
    if (vsupply) {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, 0x02, true);
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(cmd, &voltage.data[1], I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, &voltage.data[0], I2C_MASTER_ACK);
        i2c_master_stop(cmd);

        esp_err_t err_vsupply = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
        if (err_vsupply == ESP_OK) {
            *vsupply = (40.96 * (double)(voltage.voltage)) / 32767.0;
        }

        i2c_cmd_link_delete(cmd);
    }


    /* read vshunt voltage */
    if (vshunt) {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, 0x01, true);
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(cmd, &voltage.data[1], I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, &voltage.data[0], I2C_MASTER_ACK);
        i2c_master_stop(cmd);

        esp_err_t err_vshunt = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
        if (err_vshunt == ESP_OK) {
            *vshunt = (0.08192 * (double)(voltage.voltage)) / 32767.0;
        }

        i2c_cmd_link_delete(cmd);
    }

    if (err_vshunt != ESP_OK) return err_vshunt;
    if (err_vsupply != ESP_OK) return err_vsupply;
    return ESP_OK;
}


esp_err_t ina226_single_read_current_power(double* current, double* power)
{
    union {
        int16_t current;
        uint16_t power;
        uint8_t data[2];
    } value_register;
    i2c_cmd_handle_t cmd;

    esp_err_t err_current = ESP_OK;
    esp_err_t err_power = ESP_OK;

    /* read current */
    if (current) {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, 0x04, true);
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(cmd, &value_register.data[1], I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, &value_register.data[0], I2C_MASTER_ACK);
        i2c_master_stop(cmd);

        esp_err_t err_current = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
        if (err_current == ESP_OK) {
            *current = (double)(value_register.current) / 10000.0;
        }

        i2c_cmd_link_delete(cmd);
    }


    /* read power */
    if (power) {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, 0x03, true);
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(cmd, &value_register.data[1], I2C_MASTER_ACK);
        i2c_master_read_byte(cmd, &value_register.data[0], I2C_MASTER_ACK);
        i2c_master_stop(cmd);

        esp_err_t err_power = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
        if (err_power == ESP_OK) {
            *power = (double)(value_register.power) * (25.0 / 10000.0);
        }

        i2c_cmd_link_delete(cmd);
    }

    if (err_power != ESP_OK) return err_power;
    if (err_current != ESP_OK) return err_current;
    return ESP_OK;
}


esp_err_t ina226_mode_shutdown()
{
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x41, true);
    i2c_master_write_byte(cmd, 0x20, true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}


esp_err_t ina226_mode_continuous()
{
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x41, true);
    i2c_master_write_byte(cmd, 0x27, true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    return err;
}


// this mode is selected during the data transmission phase
// via cell modem.
static esp_err_t ina226_mode_continuous_averaging()
{
    union __attribute__((packed)) {
        struct __attribute__((packed))
        {
            unsigned mode : 3;
            unsigned vshct : 3;
            unsigned vbusct : 3;
            unsigned avg : 3;
            unsigned reserved1 : 3;
            unsigned rst : 1;
        } bits;
        uint8_t data[2];
    } config_reg;
    i2c_cmd_handle_t cmd;

    // set values
    config_reg.bits.rst = 0;
    config_reg.bits.reserved1 = 0;
    config_reg.bits.avg = 6;  // 512 averages
    config_reg.bits.vbusct = 4; // 1.1ms sample time
    config_reg.bits.vshct = 4; // 1.1ms sample time
    config_reg.bits.mode = 7; // continuous sampling of vshunt and vbus

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, config_reg.data[1], true);
    i2c_master_write_byte(cmd, config_reg.data[0], true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    return err;
}


// this mode is selected during the data transmission phase
// via cell modem.
static esp_err_t ina226_write_calibration_reg(uint16_t value)
{
    union {
        uint16_t value;
        uint8_t data[2];
    } calibration_regs;
    i2c_cmd_handle_t cmd;

    // set values
    calibration_regs.value = value;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x05, true);
    i2c_master_write_byte(cmd, calibration_regs.data[1], true);
    i2c_master_write_byte(cmd, calibration_regs.data[0], true);
    i2c_master_stop(cmd);


    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
    return err;
}


// ESP_OK on conversion ready
// ESP_ERR_TIMEOUT on conversion not yet ready
// ESP_FAIL on error
static esp_err_t in226_conversion_ready()
{
    union {
        struct
        {
            unsigned len : 1;
            unsigned apol : 1;
            unsigned ovf : 1;
            unsigned cvrf : 1;
            unsigned aff : 1;
            unsigned reserved : 5;
            unsigned cnvr : 1;
            unsigned pol : 1;
            unsigned bul : 1;
            unsigned bol : 1;
            unsigned sul : 1;
            unsigned sol : 1;
        } bits;
        uint8_t data[2];
    } mask_enable_reg;
    i2c_cmd_handle_t cmd;

    /* read id */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x06, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (INA226_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &mask_enable_reg.data[1], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &mask_enable_reg.data[0], I2C_MASTER_ACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_1, cmd, 10/portTICK_PERIOD_MS);
    if (err == ESP_OK) {
        if (mask_enable_reg.bits.cvrf)
            err = ESP_OK;
        else {
            err = ESP_ERR_TIMEOUT;
        }
    } else {
        err = ESP_FAIL;
    }

	i2c_cmd_link_delete(cmd);
    return err;
}

static esp_err_t ina226_wait_conversion_ready(uint8_t delay, uint8_t tries)
{
    /* wait until conversion is ready */
    esp_err_t err = ESP_OK;
    for (int i=0; i<tries; i++) {
        err = in226_conversion_ready();
        if (err != ESP_OK) {
            vTaskDelay(delay / portTICK_PERIOD_MS);
            err = ESP_ERR_TIMEOUT;
        } else {
            err = ESP_OK;
            break;
        }
    }
    return err;
}


void ina226_start_sampling()
{
    ESP_LOGI(TAG, "Starting voltage / current sampling");
    BaseType_t ret = xTaskNotify(taskHandleIna226, 0x01, eSetBits);
    if (ret != pdTRUE) {
        ESP_LOGE(TAG, "Error starting sampling loop. Voltage/Current readings will be disabled.");
    }
}


void ina226_stop_sampling()
{
    ESP_LOGI(TAG, "Stopping voltage / current sampling");
    BaseType_t ret = xTaskNotify(taskHandleIna226, 0x02, eSetBits);
    if (ret != pdTRUE) {
        ESP_LOGE(TAG, "Error ending sampling loop.");
    }
}


static void ina226_sampling_task(void *pvParameters)
{
    uint32_t task_notification_value = 0;
    BaseType_t ret;
    esp_err_t err;

    for (;;) {
        // wait for signal to start sampling
        ret = xTaskNotifyWait(0x00, 0x03, &task_notification_value, portMAX_DELAY);
        ESP_LOGI(TAG, "Received notification value %x", task_notification_value);
        if (ret != pdTRUE) continue;
        if ((task_notification_value & 0x01) == 0) continue;

        ESP_LOGI(TAG, "Starting sampling loop. Configuring INA226 for continuous averaging");
        err = ina226_mode_continuous_averaging();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed configuring INA226 for continuous averaging. Voltage/Current readings will be disabled.");
            continue;
        }
        err = ina226_write_calibration_reg(INA226_CALIBRATION_VALUE);
        if (err != ESP_OK) {
            ESP_LOGI(TAG, "INA226 calibration register could not be set (%d)", err);
            continue;
        }

        min_voltage = 20000.0; /* reset to a high value. will be overwritten after first sample is taken */
        avg_current = 0.0;
        num_current_samples = 0;

        for (;;) {
            ESP_LOGD(TAG, "Continuing sampling loop");

            err = ina226_wait_conversion_ready(200, 6);
            if (err != ESP_OK) {
                continue;
            }

            double current;
            err = ina226_single_read_current_power(&current, 0x00);
            if (err == ESP_OK) {
                num_current_samples++;
                avg_current += current;
            } else {
                ESP_LOGE(TAG, "Current / power could not be read from sensor (%x)", err);
            }

            double vsupply;
            err = ina226_single_read_voltage(&vsupply, 0x00);
            if (err == ESP_OK) {
                if (vsupply < min_voltage) min_voltage = vsupply;
            } else {
                ESP_LOGE(TAG, "Supply voltage could not be read from sensor (%x)", err);
            }

            /* wait 500ms for next sample */
            vTaskDelay(500 / portTICK_PERIOD_MS);

            // check if we need to stop the sampling loop
            ret = xTaskNotifyWait(0x00, 0x03, &task_notification_value, 0);
            if (ret == pdFALSE) continue; /* no notification received */
            if ((task_notification_value & 0x02) != 0) {
                if (num_current_samples > 0) {
                    ESP_LOGI(TAG, "Sampling results: Min voltage: %f; Avg. current: %f", min_voltage, avg_current / (double)num_current_samples);
                } else {
                    ESP_LOGE(TAG, "Error in sampling loop: Zero samples recorded.");
                }
                break;
            }
        }
    }
}


esp_err_t ina226_get_sampling_values(double* min_voltage_dest, double* avg_current_dest)
{
    esp_err_t err = ESP_OK;
    if (min_voltage_dest) {
        *min_voltage_dest = min_voltage;
    }
    if (avg_current_dest) {
        if (num_current_samples > 0) {
            *avg_current_dest = avg_current / (double)num_current_samples;
        } else {
            err = ESP_FAIL;
        }
    }

    return err;
}


esp_err_t ina226_get_startup_values(double* vsupply_on_startup_dest, double* vshunt_on_startup_dest,
    double* current_on_startup_dest, double* power_on_startup_dest)
{
    if (vsupply_on_startup_dest) {
        *vsupply_on_startup_dest = vsupply_on_startup;
    }
    if (vshunt_on_startup_dest) {
        *vshunt_on_startup_dest = vshunt_on_startup;
    }
    if (current_on_startup_dest) {
        *current_on_startup_dest = current_on_startup;
    }
    if (power_on_startup_dest) {
        *power_on_startup_dest = power_on_startup;
    }
    return ESP_OK;
}
