#include "httpclient.h"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "esp_crt_bundle.h"
#include "cJSON.h"
#include "http.h"
#include "configstorage.h"
#include "events.h"
#include "cam.h"
#include "sim800.h"
#include "ina226.h"
#include "config.h"


/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "wwv-schwarzwald.de"
#define WEB_PORT "443"
#define WEB_URL "https://wwv-schwarzwald.de/webcam/upload"

#define MULTIPART_BOUNDARY "------------------------8a7c2b940ad18aad"

static const char *TAG = "httpclient";

static const char *REQUEST = "POST " WEB_URL " HTTP/1.1\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: elktown-webcam/1.0 esp32\r\n"
    "Content-Type: multipart/form-data; boundary=" MULTIPART_BOUNDARY "\r\n"
    "Authorization: Basic dXBsb2FkZXI6U3VwZXJTZWNyZXRQYXNzd29yZA==\r\n"
    "Accept: */*\r\n"
    "Content-Length: %d\r\n"
    "\r\n";

static const char *DISPOSITION_META = "Content-Disposition: form-data; name=\"meta\"; filename=\"meta.txt\"\r\n";
static const char *DISPOSITION_IMG = "Content-Disposition: form-data; name=\"photo\"; filename=\"photo.jpg\"\r\n";

int http_return_code = -1;


static void update_config_from_json(cJSON* json)
{
    cJSON *data;
    camera_config_t cam_conf = *(config_cam());
    system_config_t sys_conf = *(config_system());
    cell_config_t cell_conf = *(config_cell());

    data = cJSON_GetObjectItemCaseSensitive(json, "sys_secs_between_photos");
    if (cJSON_IsNumber(data)) {
        ESP_LOGI(TAG, "Updating sys_secs_between_photos to %d", data->valueint);
        sys_conf.secs_between_photos = data->valueint;
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cam_quality");
    if (cJSON_IsNumber(data)) {
        ESP_LOGI(TAG, "Updating cam_quality to %d", data->valueint);
        cam_conf.quality = data->valueint;
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cam_auto_exposure");
    if (cJSON_IsNumber(data)) {
        ESP_LOGI(TAG, "Updating cam_auto_exposure to %d", data->valueint);
        cam_conf.auto_exposure = data->valueint;
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cam_light_mode");
    if (cJSON_IsNumber(data)) {
        ESP_LOGI(TAG, "Updating cam_light_mode to %d", data->valueint);
        cam_conf.light_mode = data->valueint;
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cam_color_saturation");
    if (cJSON_IsNumber(data)) {
        ESP_LOGI(TAG, "Updating cam_color_saturation to %d", data->valueint);
        cam_conf.color_saturation = data->valueint;
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cam_brightness");
    if (cJSON_IsNumber(data)) {
        ESP_LOGI(TAG, "Updating cam_brightness to %d", data->valueint);
        cam_conf.brightness = data->valueint;
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cam_contrast");
    if (cJSON_IsNumber(data)) {
        ESP_LOGI(TAG, "Updating cam_contrast to %d", data->valueint);
        cam_conf.contrast = data->valueint;
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cell_apn_auth");
    if (cJSON_IsNumber(data)) {
        ESP_LOGI(TAG, "Updating cell_apn_auth to %d", data->valueint);
        cell_conf.apn_auth = data->valueint;
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cell_pin");
    if (cJSON_IsString(data)) {
        ESP_LOGI(TAG, "Updating cell_pin to %s", data->valuestring);
        strncpy(cell_conf.pin, data->valuestring, sizeof(cell_conf.pin));
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cell_apn");
    if (cJSON_IsString(data)) {
        ESP_LOGI(TAG, "Updating cell_apn to %s", data->valuestring);
        strncpy(cell_conf.apn, data->valuestring, sizeof(cell_conf.apn));
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cell_apn_user");
    if (cJSON_IsString(data)) {
        ESP_LOGI(TAG, "Updating cell_apn_user to %s", data->valuestring);
        strncpy(cell_conf.apn_user, data->valuestring, sizeof(cell_conf.apn_user));
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cell_apn_pass");
    if (cJSON_IsString(data)) {
        ESP_LOGI(TAG, "Updating cell_apn_pass to %s", data->valuestring);
        strncpy(cell_conf.apn_pass, data->valuestring, sizeof(cell_conf.apn_pass));
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cell_remote_address");
    if (cJSON_IsString(data)) {
        ESP_LOGI(TAG, "Updating cell_remote_address to %s", data->valuestring);
        strncpy(cell_conf.remote_address, data->valuestring, sizeof(cell_conf.remote_address));
    }
    data = cJSON_GetObjectItemCaseSensitive(json, "cell_remote_url");
    if (cJSON_IsString(data)) {
        ESP_LOGI(TAG, "Updating cell_remote_url to %s", data->valuestring);
        strncpy(cell_conf.remote_url, data->valuestring, sizeof(cell_conf.remote_url));
    }

    config_write_values(&sys_conf, &cell_conf, &cam_conf);
}


static void* response_realloc(void* opaque, void* ptr, int size)
{
    ESP_LOGI(TAG, "response_realloc");
    return realloc(ptr, size);
}

static void response_body(void* opaque, const char* data, int size)
{
    if (http_return_code != 200) {
        ESP_LOGI(TAG, "Not using content since return code was %d", http_return_code);
    }
    /* parse the data we've received */
    ESP_LOGI(TAG, "received response body with size %d", size);
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGI(TAG, "Error parsing json received from server: %s", error_ptr);
        }
    } else {
        /* update configuration items with values received from server */
        update_config_from_json(json);

        /* check some more values that could be sent from server which aren't configuration items */
        cJSON* data = cJSON_GetObjectItemCaseSensitive(json, "sys_secs_between_photos_transient");
        if (cJSON_IsNumber(data)) {
            ESP_LOGI(TAG, "Received transient sleep time from server: %d", data->valueint);
            esp_err_t err = esp_event_post(EVENT_BASE, EVENT_TRANSIENT_SLEEP_COMMAND_RECEIVED, &(data->valueint), sizeof(data->valueint), portMAX_DELAY);
            if (err != ESP_OK) {
                ESP_LOGI(TAG, "Error informing main thread about new transient sleep time (%d)", err);
            }
        }

        cJSON_Delete(json);
    }
}

static void response_header(void* opaque, const char* ckey, int nkey, const char* cvalue, int nvalue)
{
}

static void response_code(void* opaque, int code)
{
    ESP_LOGI(TAG, "Received response code %d from server.", code);
    http_return_code = code;
}

static const struct http_funcs responseFuncs = {
    response_realloc,
    response_body,
    response_header,
    response_code,
};


esp_err_t write_bytes(mbedtls_ssl_context* ssl, const char* ptr, size_t len)
{
    int ret;
    size_t written_bytes = 0;
    do {
        ret = mbedtls_ssl_write(ssl, (const unsigned char *)ptr + written_bytes, len - written_bytes);
        //for (int i=0; i<len; i++) putc(ptr[i], stdout);
        if (ret >= 0) {
            //ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
            ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
            return ESP_FAIL;
        }
    } while(written_bytes < len);

    return ESP_OK;
}


esp_err_t send_data(mbedtls_ssl_context* ssl, https_upload_t* content)
{
	cJSON *root;
	root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "firmware", ELKCAM_SW_VERSION);
    /* add data from cell module */
    sim800_engineering_data_t sim800_engineering_data;
    if (sim800_get_engineering_data(&sim800_engineering_data) == ESP_OK) {
        cJSON_AddNumberToObject(root, "gsm_bcch", sim800_engineering_data.bcch);
        cJSON_AddNumberToObject(root, "gsm_rxl", sim800_engineering_data.rxl);
        cJSON_AddNumberToObject(root, "gsm_rxq", sim800_engineering_data.rxq);
        cJSON_AddNumberToObject(root, "gsm_mcc", sim800_engineering_data.mcc);
        cJSON_AddNumberToObject(root, "gsm_mnc", sim800_engineering_data.mnc);
        cJSON_AddNumberToObject(root, "gsm_bsic", sim800_engineering_data.bsic);
        cJSON_AddNumberToObject(root, "gsm_cellid", sim800_engineering_data.cellid);
        cJSON_AddNumberToObject(root, "gsm_rla", sim800_engineering_data.rla);
        cJSON_AddNumberToObject(root, "gsm_txp", sim800_engineering_data.txp);
        cJSON_AddNumberToObject(root, "gsm_lac", sim800_engineering_data.lac);
        cJSON_AddNumberToObject(root, "gsm_ta", sim800_engineering_data.ta);
        cJSON_AddNumberToObject(root, "gsm_voltage", sim800_engineering_data.voltage);
    }
    /* add data from voltage/current sampling during ppp negotiation */
    double ina226_min_voltage, ina226_avg_current;
    if (ina226_get_sampling_values(&ina226_min_voltage, &ina226_avg_current) == ESP_OK) {
        cJSON_AddNumberToObject(root, "voltage_min_cell_negotiation", ina226_min_voltage);
        cJSON_AddNumberToObject(root, "current_avg_cell_negotiation", ina226_avg_current);
    }
    /* add data from voltage/current sampling during startup */
    double vsupply_on_startup, vshunt_on_startup, current_on_startup, power_on_startup;
    if (ina226_get_startup_values(&vsupply_on_startup, &vshunt_on_startup, &current_on_startup, &power_on_startup) == ESP_OK) {
        cJSON_AddNumberToObject(root, "vsupply_startup", vsupply_on_startup);
        cJSON_AddNumberToObject(root, "vshunt_startup", vshunt_on_startup);
        cJSON_AddNumberToObject(root, "current_startup", current_on_startup);
        cJSON_AddNumberToObject(root, "power_startup", power_on_startup);
    }
    cJSON_AddNumberToObject(root, "camera_error", content->error);
	const char *my_json_string = cJSON_Print(root);
	ESP_LOGI(TAG, "my_json_string\n%s",my_json_string);

    int combinedsize = 0;
    combinedsize += 2 + strlen(MULTIPART_BOUNDARY) + 2 + strlen(DISPOSITION_META) + 2 + strlen(my_json_string);
    combinedsize += 4 + strlen(MULTIPART_BOUNDARY) + 2 + strlen(DISPOSITION_IMG) + 2 + content->buflen;
    combinedsize += 4 + strlen(MULTIPART_BOUNDARY) + 4;
    ESP_LOGI(TAG, "Combined size expected: %d", combinedsize);
    char* buf = heap_caps_malloc(2048, MALLOC_CAP_SPIRAM);
    snprintf(buf, 2048, REQUEST, combinedsize);
    int x = 0;
    write_bytes(ssl, buf, strlen(buf));
    snprintf(buf, 2048, "--%s\r\n", MULTIPART_BOUNDARY);
    write_bytes(ssl, buf, strlen(buf)); x += strlen(buf);
    snprintf(buf, 2048, "%s\r\n", DISPOSITION_META);
    write_bytes(ssl, buf, strlen(buf)); x += strlen(buf);
    write_bytes(ssl, my_json_string, strlen(my_json_string)); x += strlen(my_json_string);
    snprintf(buf, 2048, "\r\n--%s\r\n", MULTIPART_BOUNDARY);
    write_bytes(ssl, buf, strlen(buf)); x += strlen(buf);
    snprintf(buf, 2048, "%s\r\n", DISPOSITION_IMG);
    write_bytes(ssl, buf, strlen(buf)); x += strlen(buf);
    write_bytes(ssl, (char*)content->bufptr, content->buflen); x += content->buflen;
    snprintf(buf, 2048, "\r\n--%s--\r\n", MULTIPART_BOUNDARY);
    write_bytes(ssl, buf, strlen(buf)); x += strlen(buf);
    ESP_LOGI(TAG, "Actual size: %d", x);
	cJSON_Delete(root);
    return ESP_OK;
}


static void https_get_task(void *pvParameters)
{
    https_upload_t* content = (https_upload_t*)pvParameters;

    char buf[512];
    int ret, flags, len;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    ESP_LOGI(TAG, "Seeding the random number generator");

    mbedtls_ssl_config_init(&conf);

    mbedtls_entropy_init(&entropy);
    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                    NULL, 0)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        abort();
    }

    ESP_LOGI(TAG, "Attaching the certificate bundle...");

    ret = esp_crt_bundle_attach(&conf);

    if(ret < 0)
    {
        ESP_LOGE(TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -ret);
        abort();
    }

    ESP_LOGI(TAG, "Setting hostname for TLS session...");

     /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
        abort();
    }

    ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

    if((ret = mbedtls_ssl_config_defaults(&conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }

    /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
       a warning if CA verification fails but it will continue to connect.

       You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
    */
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
#endif

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }


    mbedtls_net_init(&server_fd);

    ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

    if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                    WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
        goto exit;
    }

    ESP_LOGI(TAG, "Connected.");

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");

    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
            goto exit;
        }
    }

    ESP_LOGI(TAG, "Verifying peer X.509 certificate...");

    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
        /* In real life, we probably want to close connection if ret != 0 */
        ESP_LOGW(TAG, "Failed to verify peer certificate!");
        bzero(buf, sizeof(buf));
        mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
        ESP_LOGW(TAG, "verification info: %s", buf);
    }
    else {
        ESP_LOGI(TAG, "Certificate verified.");
    }

    ESP_LOGI(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));
    ESP_LOGI(TAG, "Writing HTTP request...");
    send_data(&ssl, content);
    ESP_LOGI(TAG, "Reading HTTP response...");

    struct http_roundtripper rt;
    int http_needmore = 1;
    int http_read;
    http_init(&rt, responseFuncs, 0x00);
    do
    {
        len = sizeof(buf) - 1;
        bzero(buf, sizeof(buf));
        ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

        if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
            continue;

        if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
            ret = 0;
            break;
        }

        if(ret < 0)
        {
            ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
            break;
        }

        if(ret == 0)
        {
            ESP_LOGI(TAG, "connection closed");
            break;
        }

        len = ret;
        ESP_LOGI(TAG, "%d bytes read", len);
        if (http_needmore) {
            http_needmore = http_data(&rt, buf, len, &http_read);
            ESP_LOGI(TAG, "%d bytes read and passed to http. Needsmore = %d", len, http_needmore);
            if (!http_needmore) {
                ret = 0;
                break;
            }
        } else {
            ESP_LOGI(TAG, "%d bytes read and discarded as http says it's done", len);
        }

    } while(1);

    mbedtls_ssl_close_notify(&ssl);

exit:
    mbedtls_ssl_session_reset(&ssl);
    mbedtls_net_free(&server_fd);

    if(ret != 0)
    {
        mbedtls_strerror(ret, buf, 100);
        ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
    }

    /* tell server that we are done */
    esp_err_t err = esp_event_post(EVENT_BASE, EVENT_TRANSFER_DONE, 0x00, 0, portMAX_DELAY);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error informing main thread that transfer has finished (%d)", err);
    }
    vTaskDelete(NULL);
}

void https_upload(https_upload_t* content)
{
    xTaskCreate(&https_get_task, "https_get_task", 8192, (void*)content, 5, NULL);
}
