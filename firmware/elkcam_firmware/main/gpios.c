#include "gpios.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char* TAG = "gpios";

#define GPIO_POWER_RADIO (19)
#define GPIO_POWERKEY_RADIO (33)
#define GPIO_POWER_CAM (20)
#define GPIO_RESET_CAM (21)

static const int ledgpios[] = {35, 34};


void init_gpios()
{
    const int n_ledgpios = sizeof(ledgpios) / sizeof(ledgpios[0]);

    /* configure gpios for leds */
    gpio_config_t io_conf = {
        .mode=GPIO_MODE_OUTPUT_OD,
        .pin_bit_mask=0ULL,
        .pull_up_en = 0,
        .pull_down_en = 0
    };

    for (int i=0; i<n_ledgpios; ++i) {
        int gpio_num = ledgpios[i];
        io_conf.pin_bit_mask |= (1ULL<<gpio_num);
        esp_err_t err = gpio_set_level(gpio_num, 1);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Unable to set led gpio level. rc=%x", err);
        }
    }

    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Unable to init led gpios. rc=%x", err);
        return;
    }


    /* configure gpios for power domain switches */
    gpio_set_direction(GPIO_POWER_CAM, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_POWER_CAM, 0);
    gpio_set_direction(GPIO_POWER_RADIO, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(GPIO_POWER_RADIO, 1);
    gpio_set_direction(GPIO_POWERKEY_RADIO, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(GPIO_POWERKEY_RADIO, 1);

    /* configure gpio for cam reset */
    gpio_set_direction(GPIO_RESET_CAM, GPIO_MODE_OUTPUT_OD);
    gpio_pullup_en(GPIO_RESET_CAM);
    gpio_set_level(GPIO_RESET_CAM, 1);
}


void switch_led(int led, int on)
{
    uint8_t gpionum = 0xff;
    switch (led) {
    case CAM_LED_BLUE:
        gpionum = ledgpios[0];
        break;
    case CAM_LED_RED:
        gpionum = ledgpios[1];
        break;
    default:
        break;
    }

    if (gpionum != 0xff) {
        esp_err_t err = gpio_set_level(gpionum, on ? 0 : 1);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Unable to init led gpios. rc=%x", err);
        }
    } else {
        ESP_LOGE(TAG, "Invalid led switched");
    }
}


void power_cam(int power)
{
    gpio_set_level(GPIO_POWER_CAM, power ? 1 : 0);
}

void power_radio(int power)
{
    gpio_set_level(GPIO_POWER_RADIO, power ? 0: 1);
}


void powerkey_radio(int on)
{
    gpio_set_level(GPIO_POWERKEY_RADIO, on ? 0 : 1);
}

void reset_cam(int reset)
{
    gpio_set_level(GPIO_POWER_CAM, reset ? 0 : 1);
}