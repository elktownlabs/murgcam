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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_modem_dce_service.h"
#include "esp_modem.h"

typedef struct {
    uint32_t bcch;
    uint32_t rxl;
    uint32_t rxq;
    uint32_t mcc;
    uint32_t mnc;
    uint32_t bsic;
    uint32_t cellid;
    uint32_t rla;
    uint32_t txp;
    uint32_t lac;
    uint32_t ta;
    uint32_t voltage;
} sim800_engineering_data_t;

/**
 * @brief Create and initialize SIM800 object
 *
 * @param dte Modem DTE object
 * @return modem_dce_t* Modem DCE object
 */
modem_dce_t *sim800_init(modem_dte_t *dte);
esp_err_t sim800_get_engineering_data(sim800_engineering_data_t* data);

#ifdef __cplusplus
}
#endif
