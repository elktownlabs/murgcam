#include "camhandler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "gpios.h"
#include "cam.h"
#include "sensor.h"
#include "configstorage.h"
#include "ov2640.h"

/* camera module pin-out */
#define CAM_PIN_PWDN    (-1)
#define CAM_PIN_RESET   (21)
#define CAM_PIN_XCLK    (6)
#define CAM_PIN_SIOD    (1)
#define CAM_PIN_SIOC    (2)
#define CAM_PIN_D7      (5)
#define CAM_PIN_D6      (7)
#define CAM_PIN_D5      (8)
#define CAM_PIN_D4      (10)
#define CAM_PIN_D3      (12)
#define CAM_PIN_D2      (14)
#define CAM_PIN_D1      (13)
#define CAM_PIN_D0      (11)
#define CAM_PIN_VSYNC   (3)
#define CAM_PIN_HSYNC   (4)
#define CAM_PIN_PCLK    (9)

#define CAM_WIDTH       (1600)
#define CAM_HEIGHT      (1200)


/* camera hardware configuration */
static cam_config_t cam_config = {
    .bit_width    = 8,
    .xclk_fre     = 20 * 1000 * 1000,
    .pin  = {
        .xclk     = CAM_PIN_XCLK,
        .pclk     = CAM_PIN_PCLK,
        .vsync    = CAM_PIN_VSYNC,
        .hsync    = CAM_PIN_HSYNC,
    },
    .pin_data     = {CAM_PIN_D0, CAM_PIN_D1, CAM_PIN_D2, CAM_PIN_D3, CAM_PIN_D4, CAM_PIN_D5, CAM_PIN_D6, CAM_PIN_D7},
    .vsync_invert = true,
    .hsync_invert = false,
    .size = {
        .width    = CAM_WIDTH,
        .high     = CAM_HEIGHT,
    },
    .max_buffer_size = 8 * 1024,
    .task_stack      = 1024,
    .task_pri        = configMAX_PRIORITIES
};


static const char* TAG = "camhandler";

void camhandler_load_config_from_nvs()
{
    ESP_LOGI(TAG, "Configuring camera to global config settings");
    camera_config_t* config = config_cam();
    OV2640_Quality(config->quality);
    ESP_LOGI(TAG, "Setting picture quality to %d", config->quality);
    OV2640_Auto_Exposure(config->auto_exposure);
    ESP_LOGI(TAG, "Setting auto exposure to %d", config->auto_exposure);
    OV2640_Light_Mode(config->light_mode);
    ESP_LOGI(TAG, "Setting light mode to %d", config->light_mode);
    OV2640_Color_Saturation(config->color_saturation);
    ESP_LOGI(TAG, "Setting color saturation to %d", config->color_saturation);
    OV2640_Brightness(config->brightness);
    ESP_LOGI(TAG, "Setting brightness to %d", config->brightness);
    OV2640_Contrast(config->contrast);
    ESP_LOGI(TAG, "Setting contrast to %d", config->contrast);
    OV2640_Hue(config->hue);
    ESP_LOGI(TAG, "Setting hue to %d", config->hue);
    OV2640_Sharpness(config->hue);
    ESP_LOGI(TAG, "Setting sharpness to %d", config->hue);
}

esp_err_t init_cam()
{
    // power on cam and wait a bit for things to settle
    ESP_LOGI(TAG, "Powering on camera and waiting for camera to settle");
    power_cam(1);
    vTaskDelay(4000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Initializing camera");

    /*!< With PingPang buffers, the frame rate is higher, or you can use a separate buffer to save memory */
    cam_config.frame1_buffer = (uint8_t *)heap_caps_malloc(CAM_WIDTH * CAM_HEIGHT * 2 * sizeof(uint8_t), MALLOC_CAP_SPIRAM);
    cam_config.frame2_buffer = (uint8_t *)heap_caps_malloc(CAM_WIDTH * CAM_HEIGHT * 2 * sizeof(uint8_t), MALLOC_CAP_SPIRAM);

    ESP_LOGI(TAG, "Mem Buf: %p %p", cam_config.frame1_buffer, cam_config.frame2_buffer);

    esp_err_t err = cam_init(&cam_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera initialization failed: %d", err);
        return err;
    }


    sensor_t sensor;
    int camera_version = 0;      /*!<If the camera version is determined, it can be set to manual mode */
    SCCB_Init(CAM_PIN_SIOD, CAM_PIN_SIOC);
    sensor.slv_addr = SCCB_Probe();
    ESP_LOGI(TAG, "sensor_id: 0x%x", sensor.slv_addr);

    if (sensor.slv_addr != 0x30 && camera_version != 2640) {
        ESP_LOGE(TAG, "sensor is not supported");
        return ESP_FAIL;
    }

    /*!< Camera: OV2640 */
    ESP_LOGI(TAG, "OV2640 init start...");

    if (OV2640_Init(1, 0) != 0) {
        ESP_LOGE(TAG, "sensor initialization failed. ");
        return ESP_FAIL;
    }

    OV2640_JPEG_Mode(0);
    OV2640_ImageSize_Set(1600, 1200);
    OV2640_ImageWin_Set(0, 0, 1600, 1200);
    OV2640_OutSize_Set(CAM_WIDTH, CAM_HEIGHT);
    OV2640_Light_Mode(4);

    ESP_LOGI(TAG, "sensor initialized and ready");
    return ESP_OK;
}

void shutdown_cam()
{
    power_cam(0);
}