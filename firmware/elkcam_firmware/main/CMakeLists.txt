idf_component_register(SRCS "main.c" "wifi.c" "webserver.c" "gpios.c" "cellmodem.c" "httpclient.c" "ina226.c" "camhandler.c" "configstorage.c"
                    INCLUDE_DIRS ""
                    PRIV_REQUIRES "cam" "sensors" "spi_flash" "esp_wifi" "nvs_flash" "esp_http_server" "modem" "json" "tinyhttp")
