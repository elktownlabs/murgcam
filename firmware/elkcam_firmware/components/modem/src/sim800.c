// Copyright 2015-2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_modem_dce_service.h"
#include "sim800.h"

#define MODEM_RESULT_CODE_POWERDOWN "POWER DOWN"

static sim800_engineering_data_t sim800_engineering_data;
static int sim800_engineering_data_filled = 0;


/**
 * @brief Macro defined for error checking
 *
 */
static const char *DCE_TAG = "sim800";
#define DCE_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                \
    {                                                                                 \
        if (!(a))                                                                     \
        {                                                                             \
            ESP_LOGE(DCE_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                            \
        }                                                                             \
    } while (0)

/**
 * @brief SIM800 Modem
 *
 */
typedef struct {
    void *priv_resource; /*!< Private resource */
    modem_dce_t parent;  /*!< DCE parent class */
} sim800_modem_dce_t;


/**
 * @brief Handle response from AT+CSQ
 */
static esp_err_t sim800_handle_csq(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    sim800_modem_dce_t *sim800_dce = __containerof(dce, sim800_modem_dce_t, parent);
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+CSQ", strlen("+CSQ"))) {
        /* store value of rssi and ber */
        uint32_t **csq = sim800_dce->priv_resource;
        /* +CSQ: <rssi>,<ber> */
        sscanf(line, "%*s%d,%d", csq[0], csq[1]);
        err = ESP_OK;
    }
    return err;
}

/**
 * @brief Handle response from AT+CBC
 */
static esp_err_t sim800_handle_cbc(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    sim800_modem_dce_t *sim800_dce = __containerof(dce, sim800_modem_dce_t, parent);
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+CBC", strlen("+CBC"))) {
        /* store value of bcs, bcl, voltage */
        uint32_t **cbc = sim800_dce->priv_resource;
        /* +CBC: <bcs>,<bcl>,<voltage> */
        sscanf(line, "%*s%d,%d,%d", cbc[0], cbc[1], cbc[2]);
        sim800_engineering_data.voltage = *(cbc[2]);
        err = ESP_OK;
    }
    return err;
}

/**
 * @brief Handle response from +++
 */
static esp_err_t sim800_handle_exit_data_mode(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_NO_CARRIER)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    }
    return err;
}

/**
 * @brief Handle response from ATD*99#
 */
static esp_err_t sim800_handle_atd_ppp(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_CONNECT)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    }
    return err;
}

/**
 * @brief Handle response from AT+CGMM
 */
static esp_err_t sim800_handle_cgmm(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else {
        int len = snprintf(dce->name, MODEM_MAX_NAME_LENGTH, "%s", line);
        if (len > 2) {
            /* Strip "\r\n" */
            strip_cr_lf_tail(dce->name, len);
            err = ESP_OK;
        }
    }
    return err;
}

/**
 * @brief Handle response from AT+CGSN
 */
static esp_err_t sim800_handle_cgsn(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else {
        int len = snprintf(dce->imei, MODEM_IMEI_LENGTH + 1, "%s", line);
        if (len > 2) {
            /* Strip "\r\n" */
            strip_cr_lf_tail(dce->imei, len);
            err = ESP_OK;
        }
    }
    return err;
}

/**
 * @brief Handle response from AT+CPIN?
 */
static esp_err_t sim800_handle_cpinquery(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+CPIN: READY", strlen("+CPIN: READY"))) {
        dce->need_pin = 0;
        err = ESP_OK;
    }  else if (!strncmp(line, "+CPIN: SIM PIN", strlen("+CPIN: SIM PIN"))) {
        dce->need_pin = 1;
        err = ESP_OK;
    }
    return err;
}


static esp_err_t sim800_handle_cpinenter(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = ESP_OK;
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+CPIN: READY", strlen("+CPIN: READY"))) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
        err = ESP_OK;
    }
    return err;
}


static esp_err_t sim800_handle_waitregistration(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        if (dce->registered > 0)
            err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
        else
            err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+CREG: ", strlen("+CREG: "))) {
        int value = line[9] - '0';
        if ((value ==1) || (value == 5)) {
            dce->registered = 1;
        }
        err = ESP_OK;
    }
    return err;
}


/**
 * @brief Handle response from AT+CIMI
 */
static esp_err_t sim800_handle_cimi(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else {
        int len = snprintf(dce->imsi, MODEM_IMSI_LENGTH + 1, "%s", line);
        if (len > 2) {
            /* Strip "\r\n" */
            strip_cr_lf_tail(dce->imsi, len);
            err = ESP_OK;
        }
    }
    return err;
}

/**
 * @brief Handle response from AT+COPS?
 */
static esp_err_t sim800_handle_cops(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+COPS", strlen("+COPS"))) {
        /* there might be some random spaces in operator's name, we can not use sscanf to parse the result */
        /* strtok will break the string, we need to create a copy */
        size_t len = strlen(line);
        char *line_copy = malloc(len + 1);
        strcpy(line_copy, line);
        /* +COPS: <mode>[, <format>[, <oper>]] */
        char *str_ptr = NULL;
        char *p[3];
        uint8_t i = 0;
        /* strtok will broke string by replacing delimiter with '\0' */
        p[i] = strtok_r(line_copy, ",", &str_ptr);
        while (p[i]) {
            p[++i] = strtok_r(NULL, ",", &str_ptr);
        }
        if (i >= 3) {
            int len = snprintf(dce->oper, MODEM_MAX_OPERATOR_LENGTH, "%s", p[2]);
            if (len > 2) {
                /* Strip "\r\n" */
                strip_cr_lf_tail(dce->oper, len);
                err = ESP_OK;
            }
        }
        free(line_copy);
    }
    return err;
}



static esp_err_t sim800_handle_ceng_activation(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    }
    return err;
}


/**
 * @brief Handle response from AT+COPS?
 */
static esp_err_t sim800_handle_ceng(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+CENG", strlen("+CENG"))) {
        ESP_LOGD(DCE_TAG, "Engineering Line: %s", line);
        if (!strncmp(line, "+CENG: 0,\"", strlen("+CENG: 0,\""))) {
            uint32_t id;
            sscanf(line, "+CENG: %d,\"%d,%d,%d,%d,%d,%d,%x,%d,%d,%x,%d\"",
                &id, &sim800_engineering_data.bcch, &sim800_engineering_data.rxl, &sim800_engineering_data.rxq,
                &sim800_engineering_data.mcc, &sim800_engineering_data.mnc, &sim800_engineering_data.bsic, &sim800_engineering_data.cellid,
                &sim800_engineering_data.rla, &sim800_engineering_data.txp, &sim800_engineering_data.lac, &sim800_engineering_data.ta);
            sim800_engineering_data_filled = 1;
            ESP_LOGD(DCE_TAG, "Engineering data: [%d]: bcch=%d, rxl=%d, rxq=%d, mcc=%d, mnc=%d, bsic=%d, cellid=%d, rla=%d, txp=%d, lac=%d, ta=%d",
                id, sim800_engineering_data.bcch, sim800_engineering_data.rxl, sim800_engineering_data.rxq,
                sim800_engineering_data.mcc, sim800_engineering_data.mnc, sim800_engineering_data.bsic, sim800_engineering_data.cellid,
                sim800_engineering_data.rla, sim800_engineering_data.txp, sim800_engineering_data.lac, sim800_engineering_data.ta);
        }
        err = ESP_OK;
    }
    return err;
}


/**
 * @brief Handle response from AT+CPOWD=1
 */
static esp_err_t sim800_handle_power_down(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_POWERDOWN)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    }
    return err;
}

/**
 * @brief Get signal quality
 *
 * @param dce Modem DCE object
 * @param rssi received signal strength indication
 * @param ber bit error ratio
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_get_signal_quality(modem_dce_t *dce, uint32_t *rssi, uint32_t *ber)
{
    modem_dte_t *dte = dce->dte;
    sim800_modem_dce_t *sim800_dce = __containerof(dce, sim800_modem_dce_t, parent);
    uint32_t *resource[2] = {rssi, ber};
    sim800_dce->priv_resource = resource;
    dce->handle_line = sim800_handle_csq;
    DCE_CHECK(dte->send_cmd(dte, "AT+CSQ\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "inquire signal quality failed", err);
    ESP_LOGD(DCE_TAG, "inquire signal quality ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Get battery status
 *
 * @param dce Modem DCE object
 * @param bcs Battery charge status
 * @param bcl Battery connection level
 * @param voltage Battery voltage
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_get_battery_status(modem_dce_t *dce, uint32_t *bcs, uint32_t *bcl, uint32_t *voltage)
{
    modem_dte_t *dte = dce->dte;
    sim800_modem_dce_t *sim800_dce = __containerof(dce, sim800_modem_dce_t, parent);
    uint32_t *resource[3] = {bcs, bcl, voltage};
    sim800_dce->priv_resource = resource;
    dce->handle_line = sim800_handle_cbc;
    DCE_CHECK(dte->send_cmd(dte, "AT+CBC\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "inquire battery status failed", err);
    ESP_LOGD(DCE_TAG, "inquire battery status ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Set Working Mode
 *
 * @param dce Modem DCE object
 * @param mode woking mode
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_set_working_mode(modem_dce_t *dce, modem_mode_t mode)
{
    modem_dte_t *dte = dce->dte;
    switch (mode) {
    case MODEM_COMMAND_MODE:
        dce->handle_line = sim800_handle_exit_data_mode;
        DCE_CHECK(dte->send_cmd(dte, "+++", MODEM_COMMAND_TIMEOUT_MODE_CHANGE) == ESP_OK, "send command failed", err);
        DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "enter command mode failed", err);
        ESP_LOGD(DCE_TAG, "enter command mode ok");
        dce->mode = MODEM_COMMAND_MODE;
        break;
    case MODEM_PPP_MODE:
        dce->handle_line = sim800_handle_atd_ppp;
        DCE_CHECK(dte->send_cmd(dte, "ATD*99#\r", MODEM_COMMAND_TIMEOUT_MODE_CHANGE) == ESP_OK, "send command failed", err);
        DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "enter ppp mode failed", err);
        ESP_LOGD(DCE_TAG, "enter ppp mode ok");
        dce->mode = MODEM_PPP_MODE;
        break;
    default:
        ESP_LOGW(DCE_TAG, "unsupported working mode: %d", mode);
        goto err;
        break;
    }
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Power down
 *
 * @param sim800_dce sim800 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_power_down(modem_dce_t *dce)
{
    modem_dte_t *dte = dce->dte;
    dce->handle_line = sim800_handle_power_down;
    DCE_CHECK(dte->send_cmd(dte, "AT+CPOWD=1\r", MODEM_COMMAND_TIMEOUT_POWEROFF) == ESP_OK, "send command failed", err);
    DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "power down failed", err);
    ESP_LOGD(DCE_TAG, "power down ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Get DCE module name
 *
 * @param sim800_dce sim800 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_get_module_name(sim800_modem_dce_t *sim800_dce)
{
    modem_dte_t *dte = sim800_dce->parent.dte;
    sim800_dce->parent.handle_line = sim800_handle_cgmm;
    DCE_CHECK(dte->send_cmd(dte, "AT+CGMM\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sim800_dce->parent.state == MODEM_STATE_SUCCESS, "get module name failed", err);
    ESP_LOGD(DCE_TAG, "get module name ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Get DCE module IMEI number
 *
 * @param sim800_dce sim800 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_get_imei_number(sim800_modem_dce_t *sim800_dce)
{
    modem_dte_t *dte = sim800_dce->parent.dte;
    sim800_dce->parent.handle_line = sim800_handle_cgsn;
    DCE_CHECK(dte->send_cmd(dte, "AT+CGSN\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sim800_dce->parent.state == MODEM_STATE_SUCCESS, "get imei number failed", err);
    ESP_LOGD(DCE_TAG, "get imei number ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}


static esp_err_t sim800_query_pin(sim800_modem_dce_t *sim800_dce)
{
    modem_dte_t *dte = sim800_dce->parent.dte;
    sim800_dce->parent.handle_line = sim800_handle_cpinquery;
    DCE_CHECK(dte->send_cmd(dte, "AT+CPIN?\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sim800_dce->parent.state == MODEM_STATE_SUCCESS, "set pin failed", err);
    ESP_LOGD(DCE_TAG, "query pin ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}


static esp_err_t sim800_enter_pin(sim800_modem_dce_t *sim800_dce)
{
    modem_dte_t *dte = sim800_dce->parent.dte;
    if (!dte->pin[0]) {
        ESP_LOGE(DCE_TAG, "no pin configured");
        return ESP_FAIL;
    }
    char pincmd[32];
    snprintf(pincmd, sizeof(pincmd), "AT+CPIN=%s\r", dte->pin);
    sim800_dce->parent.handle_line = sim800_handle_cpinenter;
    DCE_CHECK(dte->send_cmd(dte, pincmd, MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sim800_dce->parent.state == MODEM_STATE_SUCCESS, "set pin failed", err);
    ESP_LOGD(DCE_TAG, "set pin ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}


static esp_err_t sim800_wait_registration(sim800_modem_dce_t *sim800_dce)
{
    modem_dte_t *dte = sim800_dce->parent.dte;
    sim800_dce->parent.handle_line = sim800_handle_waitregistration;
    DCE_CHECK(dte->send_cmd(dte, "AT+CREG?\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sim800_dce->parent.state == MODEM_STATE_SUCCESS, "waiting for network registration failed", err);
    ESP_LOGD(DCE_TAG, "network registered");
    return ESP_OK;
err:
    return ESP_FAIL;
}


/**
 * @brief Get DCE module IMSI number
 *
 * @param sim800_dce sim800 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_get_imsi_number(sim800_modem_dce_t *sim800_dce)
{
    modem_dte_t *dte = sim800_dce->parent.dte;
    sim800_dce->parent.handle_line = sim800_handle_cimi;
    DCE_CHECK(dte->send_cmd(dte, "AT+CIMI\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(sim800_dce->parent.state == MODEM_STATE_SUCCESS, "get imsi number failed", err);
    ESP_LOGD(DCE_TAG, "get imsi number ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Get Operator's name
 *
 * @param sim800_dce sim800 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_get_operator_name(sim800_modem_dce_t *sim800_dce)
{
    modem_dte_t *dte = sim800_dce->parent.dte;
    sim800_dce->parent.handle_line = sim800_handle_cops;
    DCE_CHECK(dte->send_cmd(dte, "AT+COPS?\r", MODEM_COMMAND_TIMEOUT_OPERATOR) == ESP_OK, "send command failed", err);
    DCE_CHECK(sim800_dce->parent.state == MODEM_STATE_SUCCESS, "get network operator failed", err);
    ESP_LOGD(DCE_TAG, "get network operator ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}


/**
 * @brief Get Operator's name
 *
 * @param sim800_dce sim800 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_activate_engineering_data(sim800_modem_dce_t *sim800_dce)
{
    modem_dte_t *dte = sim800_dce->parent.dte;
    sim800_dce->parent.handle_line = sim800_handle_ceng_activation;
    DCE_CHECK(dte->send_cmd(dte, "AT+CENG=1,1\r", MODEM_COMMAND_TIMEOUT_OPERATOR) == ESP_OK, "send command failed", err);
    DCE_CHECK(sim800_dce->parent.state == MODEM_STATE_SUCCESS, "get engineering data failed", err);
    ESP_LOGD(DCE_TAG, "get engineering data ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}


/**
 * @brief Get engeneering data from modem
 *
 * @param sim800_dce sim800 object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim800_query_engineering_data(sim800_modem_dce_t *sim800_dce)
{
    modem_dte_t *dte = sim800_dce->parent.dte;
    sim800_dce->parent.handle_line = sim800_handle_ceng;
    DCE_CHECK(dte->send_cmd(dte, "AT+CENG?\r", MODEM_COMMAND_TIMEOUT_OPERATOR) == ESP_OK, "send command failed", err);
    DCE_CHECK(sim800_dce->parent.state == MODEM_STATE_SUCCESS, "get engineering data failed", err);
    ESP_LOGD(DCE_TAG, "get engineering data ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}


/**
 * @brief Deinitialize SIM800 object
 *
 * @param dce Modem DCE object
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on fail
 */
static esp_err_t sim800_deinit(modem_dce_t *dce)
{
    sim800_modem_dce_t *sim800_dce = __containerof(dce, sim800_modem_dce_t, parent);
    if (dce->dte) {
        dce->dte->dce = NULL;
    }
    free(sim800_dce);
    return ESP_OK;
}

modem_dce_t *sim800_init(modem_dte_t *dte)
{
    DCE_CHECK(dte, "DCE should bind with a DTE", err);
    /* malloc memory for sim800_dce object */
    sim800_modem_dce_t *sim800_dce = calloc(1, sizeof(sim800_modem_dce_t));
    DCE_CHECK(sim800_dce, "calloc sim800_dce failed", err);
    /* Bind DTE with DCE */
    sim800_dce->parent.dte = dte;
    dte->dce = &(sim800_dce->parent);
    /* Bind methods */
    sim800_dce->parent.handle_line = NULL;
    sim800_dce->parent.sync = esp_modem_dce_sync;
    sim800_dce->parent.echo_mode = esp_modem_dce_echo;
    sim800_dce->parent.store_profile = esp_modem_dce_store_profile;
    sim800_dce->parent.set_flow_ctrl = esp_modem_dce_set_flow_ctrl;
    sim800_dce->parent.define_pdp_context = esp_modem_dce_define_pdp_context;
    sim800_dce->parent.hang_up = esp_modem_dce_hang_up;
    sim800_dce->parent.get_signal_quality = sim800_get_signal_quality;
    sim800_dce->parent.get_battery_status = sim800_get_battery_status;
    sim800_dce->parent.set_working_mode = sim800_set_working_mode;
    sim800_dce->parent.power_down = sim800_power_down;
    sim800_dce->parent.deinit = sim800_deinit;
    /* Sync between DTE and DCE */
    DCE_CHECK(esp_modem_dce_sync(&(sim800_dce->parent)) == ESP_OK, "sync failed", err_io);
    /* Close echo */
    DCE_CHECK(esp_modem_dce_echo(&(sim800_dce->parent), false) == ESP_OK, "close echo mode failed", err_io);
    /* Get Module name */
    DCE_CHECK(sim800_get_module_name(sim800_dce) == ESP_OK, "get module name failed", err_io);
    /* Get IMEI number */
    DCE_CHECK(sim800_get_imei_number(sim800_dce) == ESP_OK, "get imei failed", err_io);
    /* Query if PIN is active */
    DCE_CHECK(sim800_query_pin(sim800_dce) == ESP_OK, "query pin failed", err_io);
    /* Enter PIN */
    if (sim800_dce->parent.need_pin) {
        DCE_CHECK(sim800_enter_pin(sim800_dce) == ESP_OK, "enter pin failed", err_io);
    }
    /* Wait for network registration */
    DCE_CHECK(sim800_wait_registration(sim800_dce) == ESP_OK, "waiting for registration timed out", err_io);
    /* Get IMSI number */
    DCE_CHECK(sim800_get_imsi_number(sim800_dce) == ESP_OK, "get imsi failed", err_io);
    /* Get operator name */
    DCE_CHECK(sim800_get_operator_name(sim800_dce) == ESP_OK, "get operator name failed", err_io);
    /* Get engineering mode details */
    DCE_CHECK(sim800_activate_engineering_data(sim800_dce) == ESP_OK, "activating engineering data failed", err_io);
    DCE_CHECK(sim800_query_engineering_data(sim800_dce) == ESP_OK, "get engineering data failed", err_io);
    return &(sim800_dce->parent);
err_io:
    free(sim800_dce);
err:
    return NULL;
}


esp_err_t sim800_get_engineering_data(sim800_engineering_data_t* data)
{
    esp_err_t res = ESP_FAIL;
    if (data) {
        if (sim800_engineering_data_filled) {
            *data = sim800_engineering_data;
            res = ESP_OK;
        }
    }
    return res;
}
