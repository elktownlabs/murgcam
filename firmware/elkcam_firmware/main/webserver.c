#include "webserver.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "events.h"
#include "cam.h"
#include "camhandler.h"

static const char *TAG = "httpserver";

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Host %s requested a picture.", buf);
        }
        free(buf);
    }


    uint8_t *cam_buf = NULL;
    ESP_LOGI(TAG, "Taking picture");
    camhandler_load_config_from_nvs();
    cam_start();
    size_t data_len = cam_take(&cam_buf);
    ESP_LOGI(TAG, "Picture taken. Size = %d", data_len);

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Content-Type", "image/jpeg");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    httpd_resp_send(req, (const char*)cam_buf, data_len);

    cam_give(cam_buf);
    cam_stop();

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}


static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "Hello World!"
};




httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}


void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}
