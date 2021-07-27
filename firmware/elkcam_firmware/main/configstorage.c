#include "configstorage.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "config.h"
#include "configstorage_const.h"
#include <string.h>
#include <stdint.h>

#define NVS_HANDLE "cam"

static const char* TAG = "configstorage";

/* forward function declarations */
static void log_read_result(esp_err_t result, esp_err_t* global_result, char* handle);
static void log_write_result(esp_err_t result, esp_err_t* global_result, char* handle);
static esp_err_t read_cam_config(camera_config_t* data);
static esp_err_t write_cam_config(camera_config_t* data);
static esp_err_t read_cell_config(cell_config_t* data);
static esp_err_t write_cell_config(cell_config_t* data);
static esp_err_t read_system_config(system_config_t* data);
static esp_err_t write_system_config(system_config_t* data);

/* configuration read from nvs */
static camera_config_t current_cam_config;
static cell_config_t current_cell_config;
static system_config_t current_system_config;


static void log_read_result(esp_err_t result, esp_err_t* global_result, char* handle)
{
    if (result == ESP_OK) {
    } else if (result == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "%s not yet stored in config memory (%s)", handle, esp_err_to_name(result));
    } else {
        ESP_LOGE(TAG, "%s could no be read from config memory (%s)", handle, esp_err_to_name(result));
        if (global_result) *global_result = result;
    }
}


static void log_write_result(esp_err_t result, esp_err_t* global_result, char* handle)
{
    if (result == ESP_OK) {
    } else {
        ESP_LOGE(TAG, "%s could no be written to config memory (%s)", handle, esp_err_to_name(result));
        if (global_result) *global_result = result;
    }
}


static esp_err_t read_cam_config(camera_config_t* data)
{
    nvs_handle_t handle;
    esp_err_t err, global_err;

    if (!data) return ESP_FAIL;

    /* set defaults */
    data->quality = CAM_DEFAULT_QUALITY;
    data->auto_exposure = CAM_DEFAULT_AUTO_EXPOSURE;
    data->light_mode = CAM_DEFAULT_LIGHT_MODE;
    data->color_saturation = CAM_DEFAULT_COLOR_SATURATION;
    data->pixeltiming = CAM_DEFAULT_PIXELTIMING;
    data->brightness = CAM_DEFAULT_BRIGHTNESS;
    data->contrast = CAM_DEFAULT_CONTRAST;
    data->hue = CAM_DEFAULT_HUE;
    data->sharpness = CAM_DEFAULT_SHARPNESS;

    /* open nvs */
    err = nvs_open(NVS_HANDLE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error opening nvs config storage (%s)", esp_err_to_name(err));
        return err;
    }
    global_err = ESP_OK;

    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CAM_QUALITY, &data->quality);
    log_read_result(err, &global_err, "Cam Config: quality");

    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CAM_AUTO_EXPOSURE, &data->auto_exposure);
    log_read_result(err, &global_err, "Cam Config: auto exposure");


    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CAM_LIGHT_MODE, &data->light_mode);
    log_read_result(err, &global_err, "Cam Config: light mode");

    data->color_saturation = CAM_DEFAULT_COLOR_SATURATION;
    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CAM_COLOR_SATURATION, &data->color_saturation);
    log_read_result(err, &global_err, "Cam Config: color saturation");

    data->pixeltiming = CAM_DEFAULT_PIXELTIMING;
    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CAM_PIXELTIMING, &data->pixeltiming);
    log_read_result(err, &global_err, "Cam Config: pixel timing");

    data->brightness = CAM_DEFAULT_BRIGHTNESS;
    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CAM_BRIGHTNESS, &data->brightness);
    log_read_result(err, &global_err, "Cam Config: color saturation");

    data->contrast = CAM_DEFAULT_CONTRAST;
    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CAM_CONTRAST, &data->contrast);
    log_read_result(err, &global_err, "Cam Config: contrast");

    data->hue = CAM_DEFAULT_HUE;
    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CAM_HUE, &data->hue);
    log_read_result(err, &global_err, "Cam Config: hue");

    data->sharpness = CAM_DEFAULT_SHARPNESS;
    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CAM_SHARPNESS, &data->sharpness);
    log_read_result(err, &global_err, "Cam Config: sharpness");


    nvs_close(handle);
    return global_err;
}


static esp_err_t write_cam_config(camera_config_t* data)
{
    nvs_handle_t handle;
    esp_err_t err, global_err;

    if (!data) return ESP_FAIL;

    /* open nvs */
    err = nvs_open(NVS_HANDLE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening nvs config storage (%d)", err);
        return err;
    }
    global_err = ESP_OK;

    if (data->quality > 63) data->quality = 63;
    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CAM_QUALITY, data->quality);
    log_write_result(err, &global_err, "Cam Config: quality");

    if (data->auto_exposure > 4) data->auto_exposure = 4;
    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CAM_AUTO_EXPOSURE, data->auto_exposure);
    log_write_result(err, &global_err, "Cam Config: auto exposure");

    if (data->light_mode > 4) data->light_mode = 4;
    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CAM_LIGHT_MODE, data->light_mode);
    log_write_result(err, &global_err, "Cam Config: light mode");

    if (data->color_saturation > 9) data->color_saturation = 9;
    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CAM_COLOR_SATURATION, data->color_saturation);
    log_write_result(err, &global_err, "Cam Config: color saturation");

    if (data->pixeltiming > 0x0e) data->pixeltiming = 0x0e;
    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CAM_PIXELTIMING, data->pixeltiming);
    log_write_result(err, &global_err, "Cam Config: pixel timing");

    if (data->brightness > 9) data->brightness = 9;
    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CAM_BRIGHTNESS, data->brightness);
    log_write_result(err, &global_err, "Cam Config: color saturation");

    if (data->contrast > 9) data->contrast = 9;
    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CAM_CONTRAST, data->contrast);
    log_write_result(err, &global_err, "Cam Config: contrast");

    if (data->hue > 5) data->hue = 5;
    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CAM_HUE, data->hue);
    ESP_LOGI(TAG, "Hue: %d", data->hue);
    log_write_result(err, &global_err, "Cam Config: hue");

    if (data->sharpness > 7) data->sharpness = 7;
    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CAM_SHARPNESS, data->sharpness);
    ESP_LOGI(TAG, "Sharpness: %d", data->sharpness);
    log_write_result(err, &global_err, "Cam Config: sharpness");

    nvs_commit(handle);
    nvs_close(handle);
    return global_err;
}


static esp_err_t read_cell_config(cell_config_t* data)
{
    nvs_handle_t handle;
    esp_err_t err, global_err;
    size_t str_length;

    if (!data) return ESP_FAIL;

    /* set defaults */
    strncpy(data->pin, CAM_CELL_PIN, sizeof(data->pin));
    strncpy(data->apn, CAM_CELL_APN, sizeof(data->apn));
    strncpy(data->apn_user, CAM_CELL_APN_USER, sizeof(data->apn_user));
    strncpy(data->apn_pass, CAM_CELL_APN_PASS, sizeof(data->apn_pass));
    strncpy(data->remote_address, CAM_CELL_REMOTE_ADDRESS, sizeof(data->remote_address));
    strncpy(data->remote_url, CAM_CELL_REMOTE_URL, sizeof(data->remote_url));
    data->apn_auth = CAM_CELL_APN_AUTH;

    /* open nvs */
    err = nvs_open(NVS_HANDLE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error opening nvs config storage (%d)", err);
        return err;
    }
    global_err = ESP_OK;

    str_length = sizeof(data->pin);
    err = nvs_get_str(handle, CONFIGSTORAGE_KEY_CELL_PIN, data->pin, &str_length);
    log_read_result(err, &global_err, "Cell Config: PIN");

    str_length = sizeof(data->apn);
    err = nvs_get_str(handle, CONFIGSTORAGE_KEY_CELL_APN, data->apn, &str_length);
    log_read_result(err, &global_err, "Cell Config: APN");

    str_length = sizeof(data->apn_user);
    err = nvs_get_str(handle, CONFIGSTORAGE_KEY_CELL_APN_USER, data->apn_user, &str_length);
    log_read_result(err, &global_err, "Cell Config: APN user");

    str_length = sizeof(data->apn_pass);
    err = nvs_get_str(handle, CONFIGSTORAGE_KEY_CELL_APN_PASS, data->apn_pass, &str_length);
    log_read_result(err, &global_err, "Cell Config: APN password");

    err = nvs_get_u8(handle, CONFIGSTORAGE_KEY_CELL_APN_AUTH, &data->apn_auth);
    log_read_result(err, &global_err, "Cell Config: APN authentication method");

    str_length = sizeof(data->remote_address);
    err = nvs_get_str(handle, CONFIGSTORAGE_KEY_CELL_REMOTE_ADDRESS, data->remote_address, &str_length);
    log_read_result(err, &global_err, "Cell Config: Remote address");

    str_length = sizeof(data->remote_url);
    err = nvs_get_str(handle, CONFIGSTORAGE_KEY_CELL_REMOTE_URL, data->remote_url, &str_length);
    log_read_result(err, &global_err, "Cell Config: Remote url");

    nvs_close(handle);
    return global_err;
}


static esp_err_t write_cell_config(cell_config_t* data)
{
    nvs_handle_t handle;
    esp_err_t err, global_err;

    if (!data) return ESP_FAIL;

    /* open nvs */
    err = nvs_open(NVS_HANDLE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening nvs config storage (%d)", err);
        return err;
    }
    global_err = ESP_OK;

    err = nvs_set_str(handle, CONFIGSTORAGE_KEY_CELL_PIN, data->pin);
    log_write_result(err, &global_err, "Cell Config: PIN");

    err = nvs_set_str(handle, CONFIGSTORAGE_KEY_CELL_APN, data->apn);
    log_write_result(err, &global_err, "Cell Config: APN");

    err = nvs_set_str(handle, CONFIGSTORAGE_KEY_CELL_APN_USER, data->apn_user);
    log_write_result(err, &global_err, "Cell Config: APN user");

    err = nvs_set_str(handle, CONFIGSTORAGE_KEY_CELL_APN_PASS, data->apn_pass);
    log_write_result(err, &global_err, "Cell Config: APN password");

    err = nvs_set_u8(handle, CONFIGSTORAGE_KEY_CELL_APN_AUTH, data->apn_auth);
    log_write_result(err, &global_err, "Cell Config: APN authentication method");

    err = nvs_set_str(handle, CONFIGSTORAGE_KEY_CELL_REMOTE_ADDRESS, data->remote_address);
    log_write_result(err, &global_err, "Cell Config: Remote address");

    err = nvs_set_str(handle, CONFIGSTORAGE_KEY_CELL_REMOTE_URL, data->remote_url);
    log_write_result(err, &global_err, "Cell Config: Remote url");

    nvs_commit(handle);
    nvs_close(handle);
    return global_err;
}


static esp_err_t read_system_config(system_config_t* data)
{
    nvs_handle_t handle;
    esp_err_t err, global_err;

    if (!data) return ESP_FAIL;

    /* set defaults */
    data->secs_between_photos = SYS_DEFAULT_SECS_BETWEEN_PHOTOS;
    data->photo_counter = 0;

    /* open nvs */
    err = nvs_open(NVS_HANDLE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error opening nvs config storage (%d)", err);
        return err;
    }
    global_err = ESP_OK;

    err = nvs_get_u16(handle, CONFIGSTORAGE_KEY_SYS_SECS_BETWEEN_PHOTOS, &data->secs_between_photos);
    log_read_result(err, &global_err, "System Config: secs between photos");

    err = nvs_get_u32(handle, CONFIGSTORAGE_KEY_SYS_PHOTO_COUNTER, &data->photo_counter);
    log_read_result(err, &global_err, "System Config: photo counter");

    nvs_close(handle);
    return global_err;
}


static esp_err_t write_system_config(system_config_t* data)
{
    nvs_handle_t handle;
    esp_err_t err, global_err;

    if (!data) return ESP_FAIL;

    /* open nvs */
    err = nvs_open(NVS_HANDLE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening nvs config storage (%d)", err);
        return err;
    }
    global_err = ESP_OK;

    err = nvs_set_u16(handle, CONFIGSTORAGE_KEY_SYS_SECS_BETWEEN_PHOTOS, data->secs_between_photos);
    log_write_result(err, &global_err, "System Config: secs between photos");

    data->photo_counter = 0;
    err = nvs_set_u32(handle, CONFIGSTORAGE_KEY_SYS_PHOTO_COUNTER, data->photo_counter);
    log_write_result(err, &global_err, "System Config: photo counter");

    nvs_commit(handle);
    nvs_close(handle);
    return global_err;
}


void config_load_values()
{
    esp_err_t err;

    /* initialize data structures */
    memset(&current_cam_config, 0x00, sizeof(camera_config_t));
    memset(&current_cell_config, 0x00, sizeof(cell_config_t));
    memset(&current_system_config, 0x00, sizeof(system_config_t));

    err = read_cam_config(&current_cam_config);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Unable to load camera configuration values (%s).", esp_err_to_name(err));
    }
    err = read_cell_config(&current_cell_config);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Unable to load cell configuration values (%s).", esp_err_to_name(err));
    }
    err = read_system_config(&current_system_config);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Unable to load system configuration values (%s).", esp_err_to_name(err));
    }
}


camera_config_t* config_cam()
{
    return &current_cam_config;
}


cell_config_t* config_cell()
{
    return &current_cell_config;
}


system_config_t* config_system()
{
    return &current_system_config;
}


void config_write_values(system_config_t* sys_conf, cell_config_t* cell_conf, camera_config_t* cam_conf)
{
    esp_err_t err;
    int res;
    /* compare system config */
    if (sys_conf) {
        res = memcmp(sys_conf, &current_system_config, sizeof(system_config_t));
        if (res != 0) {
            err = write_system_config(sys_conf);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Unable to write system configuration to nvs (%s).", esp_err_to_name(err));
            }
        } else {
            ESP_LOGI(TAG, "System configuration has not changed.");
        }
    }
    /* compare cell config */
    if (cell_conf) {
        res = memcmp(cell_conf, &current_cell_config, sizeof(cell_config_t));
        if (res != 0) {
            err = write_cell_config(cell_conf);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Unable to write cell configuration to nvs (%s).", esp_err_to_name(err));
            }
        } else {
            ESP_LOGI(TAG, "Cell configuration has not changed.");
        }
    }
    /* compare cell config */
    if (cam_conf) {
        res = memcmp(cam_conf, &current_cam_config, sizeof(camera_config_t));
        if (res != 0) {
            err = write_cam_config(cam_conf);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Unable to write cam configuration to nvs (%s).", esp_err_to_name(err));
            }
        } else {
            ESP_LOGI(TAG, "Cam configuration has not changed.");
        }
    }
}





