#include <string.h>
#include "cellmodem.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_netif_ppp.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_modem.h"
#include "esp_modem_netif.h"
#include "config.h"
#include "sim800.h"
#include "ina226.h"

static const char* TAG = "cellmodem";

#define SIM800_TX_GPIO 17
#define SIM800_RX_GPIO 18


static EventGroupHandle_t event_group = NULL;
static const int CONNECT_BIT = BIT0;
static const int STOP_BIT = BIT1;

static void *modem_netif_adapter = NULL;
static modem_dte_t *dte = NULL;


static void modem_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    uint8_t* x = (uint8_t*)event_data;
    switch (event_id) {
    case ESP_MODEM_EVENT_PPP_START:
        ESP_LOGI(TAG, "Modem PPP Started");
        // start voltage / current sampling
        ina226_start_sampling();
        break;
    case ESP_MODEM_EVENT_PPP_STOP:
        ESP_LOGI(TAG, "Modem PPP Stopped");
        xEventGroupSetBits(event_group, STOP_BIT);
        // stop voltage / current sampling.
        // during regular operations, the sampling is stopped in IP_EVENT_PPP_GOT_IP
        // this just catches error states where we never got an IP
        ina226_stop_sampling();
        break;
    case ESP_MODEM_EVENT_UNKNOWN:
        ESP_LOGW(TAG, "Unknown line received: %s (%x %x %x %x %x %x)", (char *)event_data, x[0], x[1], x[2], x[3], x[4], x[5]);
        break;
    default:
        break;
    }
}


static void on_ppp_changed(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "PPP state changed event %d", event_id);
    if (event_id == NETIF_PPP_ERRORUSER) {
        /* User interrupted event from esp-netif */
        esp_netif_t *netif = event_data;
        ESP_LOGI(TAG, "User interrupted event from netif:%p", netif);
    }
}


static void on_ip_event(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "IP event! %d", event_id);
    if (event_id == IP_EVENT_PPP_GOT_IP) {
        esp_netif_dns_info_t dns_info;

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        esp_netif_t *netif = event->esp_netif;

        ESP_LOGI(TAG, "Modem Connect to PPP Server");
        ESP_LOGI(TAG, "IP          : " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Netmask     : " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(TAG, "Gateway     : " IPSTR, IP2STR(&event->ip_info.gw));
        esp_netif_get_dns_info(netif, 0, &dns_info);
        ESP_LOGI(TAG, "Name Server1: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        esp_netif_get_dns_info(netif, 1, &dns_info);
        ESP_LOGI(TAG, "Name Server2: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        xEventGroupSetBits(event_group, CONNECT_BIT);

        // stop voltage / current sampling
        ina226_stop_sampling();
    } else if (event_id == IP_EVENT_PPP_LOST_IP) {
        // stop voltage / current sampling.
        // during regular operations, the sampling is stopped in IP_EVENT_PPP_GOT_IP
        // this just catches error states where we never got an IP
        ina226_stop_sampling();
        ESP_LOGI(TAG, "Modem Disconnect from PPP Server");
    } else if (event_id == IP_EVENT_GOT_IP6) {
        ESP_LOGI(TAG, "GOT IPv6 event!");
        ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
        ESP_LOGI(TAG, "Got IPv6 address " IPV6STR, IPV62STR(event->ip6_info.ip));
    }
}


void close_cellmodem()
{
    ESP_ERROR_CHECK(esp_modem_stop_ppp(dte));
    xEventGroupWaitBits(event_group, STOP_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
    esp_modem_netif_teardown(modem_netif_adapter);
}

_Bool init_cellmodem()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &on_ip_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID, &on_ppp_changed, NULL));

    event_group = xEventGroupCreate();

    // Init netif object
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_PPP();
    esp_netif_t *esp_netif = esp_netif_new(&cfg);
    assert(esp_netif);


    /* create dte object */
    esp_modem_dte_config_t config = ESP_MODEM_DTE_DEFAULT_CONFIG();
    /* setup UART specific configuration based on kconfig options */
    strcpy(config.pin, CAM_GSM_PIN);
    config.tx_io_num = SIM800_TX_GPIO;
    config.rx_io_num = SIM800_RX_GPIO;
    config.rx_buffer_size = 1024;
    config.tx_buffer_size = 512;
    config.pattern_queue_size = 20;
    config.event_queue_size = 30;
    config.event_task_stack_size = 2048;
    config.event_task_priority = 5;
    config.line_buffer_size = config.rx_buffer_size / 2;
    ESP_LOGI(TAG, "Init DTE");
    dte = esp_modem_dte_init(&config);
    /* Register event handler */
    ESP_ERROR_CHECK(esp_modem_set_event_handler(dte, modem_event_handler, ESP_EVENT_ANY_ID, NULL));
    /* create dce object */
    ESP_LOGI(TAG, "Init SIM800");

    modem_dce_t *dce = NULL;
    int numRetries = 30;
    while ((!dce) && (numRetries-- > 0)) {
        dce = sim800_init(dte);
        if (!dce) {
            ESP_LOGI(TAG, "Modem not yet initialized.");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if (dce) {
        ESP_ERROR_CHECK(dce->set_flow_ctrl(dce, MODEM_FLOW_CONTROL_NONE));
        ESP_ERROR_CHECK(dce->store_profile(dce));
        /* Print Module ID, Operator, IMEI, IMSI */
        ESP_LOGI(TAG, "Module: %s", dce->name);
        ESP_LOGI(TAG, "Operator: %s", dce->oper);
        ESP_LOGI(TAG, "IMEI: %s", dce->imei);
        ESP_LOGI(TAG, "IMSI: %s", dce->imsi);
        /* Get signal quality */
        uint32_t rssi = 0, ber = 0;
        ESP_ERROR_CHECK(dce->get_signal_quality(dce, &rssi, &ber));
        ESP_LOGI(TAG, "rssi: %d, ber: %d", rssi, ber);
        /* Get battery voltage */
        uint32_t voltage = 0, bcs = 0, bcl = 0;
        ESP_ERROR_CHECK(dce->get_battery_status(dce, &bcs, &bcl, &voltage));
        ESP_LOGI(TAG, "Battery voltage: %d mV", voltage);

        // authenticate
        #if 0
        esp_netif_auth_type_t auth_type = NETIF_PPP_AUTHTYPE_PAP;
        esp_netif_ppp_set_auth(esp_netif, auth_type, CONFIG_EXAMPLE_MODEM_PPP_AUTH_USERNAME, CONFIG_EXAMPLE_MODEM_PPP_AUTH_PASSWORD);
        #endif

        modem_netif_adapter = esp_modem_netif_setup(dte);
        esp_modem_netif_set_default_handlers(modem_netif_adapter, esp_netif);
        /* attach the modem to the network interface */
        esp_netif_attach(esp_netif, modem_netif_adapter);
        /* Wait for IP address */
        xEventGroupWaitBits(event_group, CONNECT_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
        return true;
    } else {
        ESP_LOGI(TAG, "Modem initialization failed. Not uploading.");
        return false;
    }
}