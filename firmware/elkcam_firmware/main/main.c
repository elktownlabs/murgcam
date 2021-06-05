/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_event_base.h"
#include "ina226.h"
#include "config.h"
#include "esp_log.h"
#include "wifi.h"
#include "webserver.h"
#include "events.h"
#include "gpios.h"
#include "cellmodem.h"
#include "httpclient.h"
#include "camhandler.h"
#include "cam.h"
#include "configstorage.h"


#define MODE_CONFIG 1
#define MODE_NORMAL 0
#define STARTUP_MODE MODE_CONFIG

ESP_EVENT_DEFINE_BASE(EVENT_BASE);


/* all data to be sent to the server is collected here */
https_upload_t upload_data;

static const char* TAG = "main";

static int transient_sleep_time = 0;
static xQueueHandle sm_uploadeComplete = 0x00;

static void on_ip_connected(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "IP connection established");
    https_upload(&upload_data);
}


static void on_set_transient_sleep(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    transient_sleep_time =  *((int*)event_data);
    ESP_LOGI(TAG, "Received transient sleep time from server: %d", transient_sleep_time);
}


static void on_transfer_done(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "Transfer to server has finished");
    /* unblock main task */
    xSemaphoreGive(sm_uploadeComplete);
}


void boot_msg()
{
    ESP_LOGI(TAG, "ElkCam Firmware V0.0.1, 03/18/2021 (c) Copyright 2021 Elktown Labs.");
    ESP_LOGI(TAG, "Booting ESP32S2 / FreeRTOS");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "This is %s chip with %d CPU cores, WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

    ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_LOGI(TAG, "Free heap: %d\n", esp_get_free_heap_size());
}


void init_nvs(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}


void init_event_loop(void)
{
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Event loop could not be initialized: %d", err);
        return;
    }
    ESP_ERROR_CHECK(esp_event_handler_instance_register(EVENT_BASE, EVENT_TRANSIENT_SLEEP_COMMAND_RECEIVED, on_set_transient_sleep, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(EVENT_BASE, EVENT_TRANSFER_DONE, on_transfer_done, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_PPP_GOT_IP, &on_ip_connected, NULL, NULL));
    ESP_LOGI(TAG, "Event loop initialized");
}


void app_main(void)
{
    boot_msg();
    ina226_init();
    init_event_loop();
    init_nvs();
    init_gpios();
    config_load_values();

#if STARTUP_MODE == MODE_NORMAL
    /* create the semaphore for waiting until upload to server is complete */
    sm_uploadeComplete = xSemaphoreCreateBinary();

    /* try five times to take a picture */
    for (int i=0; i<5; i++) {
        ESP_LOGI(TAG, "Starting up in regular single picture mode");
        init_cam();

        ESP_LOGI(TAG, "Taking picture");
        camhandler_load_config_from_nvs();
        switch_led(CAM_LED_BLUE, 1);
        cam_start();
        upload_data.buflen = cam_take(&upload_data.bufptr, 5000 / portTICK_PERIOD_MS);
        if (upload_data.buflen == 0) {
            ESP_LOGE(TAG, "Camera timed out");
            upload_data.error = 1;
        }  else {
            /* find end of jpeg stream and truncate data there */
            for (uint32_t i=0; i<upload_data.buflen-1; i++) {
                if (upload_data.bufptr[i] == 0xff && upload_data.bufptr[i+1] == 0xd9) {
                    ESP_LOGI(TAG, "Shortened data from %d to %d.", upload_data.buflen, i+2);
                    upload_data.buflen = i+2;
                    break;
                }
            }
            upload_data.error = 0;
            ESP_LOGI(TAG, "Picture taken. Size = %d", upload_data.buflen);
        }
        switch_led(CAM_LED_BLUE, 0);
        ESP_LOGI(TAG, "Shutting down and switching off camera");
        cam_stop();
        shutdown_cam();
        /* if picture was taken successfully, end the loop */
        if (upload_data.error == 0) break;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    /* increase photo counter */
    system_config_t sys_conf = *(config_system());
    sys_conf.photo_counter++;
    config_write_values(&sys_conf, 0x00, 0x00);


    /* upload to server */
    ESP_LOGI(TAG, "Uploading picture to server");
    ESP_LOGI(TAG, "Power down modem for 2s");
    power_radio(0);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Power up modem");
    power_radio(1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Power key on for 1.2s");
    powerkey_radio(1);
    vTaskDelay(1200 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Power key off and wait 1s");
    powerkey_radio(0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Init modem");
    bool modemInitSuccessful = init_cellmodem();

    // only if connection was successful
    if (modemInitSuccessful) {
        int uploadFinished = 0;
        for (int i=0; i<30; i++) {
            /* wait until upload to server is done */
            if( xSemaphoreTake(sm_uploadeComplete, pdMS_TO_TICKS(1000)) == pdTRUE ) {
                ESP_LOGI(TAG, "Upload has finished. Shutting everything down.");
                uploadFinished = 1;
                break;
            } else {
                ESP_LOGI(TAG, "Upload not yet finished. Retrying.");
            }
        }
        if (!uploadFinished) {
            ESP_LOGI(TAG, "Unable to upload. Something went wrong with the modem");
        }
    }

    /* deinitialize stuff */
    cam_give(upload_data.bufptr);

    /* tear down connection */
    ESP_LOGI(TAG, "Closing modem connection");
    if (modemInitSuccessful) close_cellmodem();

    /* power down modem */
    ESP_LOGI(TAG, "Modem power key on for 1.2s");
    powerkey_radio(1);
    vTaskDelay(1200 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Modem power key off and wait 1s");
    powerkey_radio(0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Power down modem now");
    power_radio(0);

    /* power down ina226 */
    ina226_mode_shutdown();

    /* figure out sleep time */
    uint32_t sleep_time = 0;
    if (transient_sleep_time > 0) {
        /* we have received a transient sleep time from server */
        sleep_time = transient_sleep_time;
        ESP_LOGI(TAG, "Using transient sleep time of %d secs", sleep_time);
    } else {
        /* use normal sleep time */
        sleep_time = config_system()->secs_between_photos;
        ESP_LOGI(TAG, "Using sleep time from config: %d secs", sleep_time);
    }

    /* go back to sleep */
    TickType_t timeAlive = xTaskGetTickCount() * portTICK_PERIOD_MS;
    int actual_time_to_sleep = (sleep_time * 1000) - timeAlive;
    /* sleep at least for a minute */
    if (actual_time_to_sleep <= 60000) {
        ESP_LOGI(TAG, "Time to sleep determined to be %d ms. Need to sleep at least 60000 ms", actual_time_to_sleep);
        actual_time_to_sleep = 60000;
    }
    ESP_LOGI(TAG, "Up for %d ms. Going to sleep for %d ms", timeAlive, actual_time_to_sleep);
    esp_sleep_enable_timer_wakeup(actual_time_to_sleep * 1000);
    esp_deep_sleep_start();


#elif STARTUP_MODE == MODE_CONFIG
    init_wifi();
    init_cam();
    start_webserver();

    uint8_t led_state = 0;
    for (;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* blink led to signal config mode */
        switch_led(CAM_LED_BLUE, led_state);
        led_state = ~led_state;
    }
#endif

}
