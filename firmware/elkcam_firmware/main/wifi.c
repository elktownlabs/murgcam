#include "wifi.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "config.h"
#include "webserver.h"

static const char* TAG = "main";

static wifi_config_t ap_config = {
        .ap = {
        .ssid = CAM_CONFIG_SSID,
        .ssid_len = sizeof(CAM_CONFIG_SSID)-1,
        .channel = 3,
        .authmode = WIFI_AUTH_OPEN,
        .ssid_hidden = 0,
        .max_connection = 1,
        .beacon_interval = 100
        }
};



static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}


void init_wifi()
{
    esp_err_t err = esp_netif_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Net interface could not be initialized: %d", err);
        return;
    }
    ESP_LOGI(TAG, "Net interface initialized");

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Wifi could not be initialized: %d", err);
        return;
    }
    ESP_LOGI(TAG, "Wifi initialized");


    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Wifi event handlers could not be initialized: %d", err);
        return;
    }
    ESP_LOGI(TAG, "Wifi event handlers initialized");


    err = esp_wifi_set_mode(WIFI_MODE_AP);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Wifi AP mode could not be initialized: %d", err);
        return;
    }
    ESP_LOGI(TAG, "Wifi AP mode initialized");

    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Wifi could not be configured: %d", err);
        return;
    }
    ESP_LOGI(TAG, "Wifi configured");

    err = esp_wifi_start();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Wifi could not be started: %d", err);
        return;
    }
    ESP_LOGI(TAG, "Wifi AP initialized successfully");
}