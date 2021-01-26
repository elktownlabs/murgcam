#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define I2C_SCL_IO				27	//19         /*!< gpio number for I2C master clock */
#define I2C_SDA_IO				26	//18         /*!< gpio number for I2C master data  */
#define I2C_FREQ_HZ				100000           /*!< I2C master clock frequency */
#define I2C_PORT_NUM			I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_TX_BUF_DISABLE  	0                /*!< I2C master do not need buffer */
#define I2C_RX_BUF_DISABLE  	0                /*!< I2C master do not need buffer */

#define WRITE_BIT               I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN            0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS           0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                 0x0              /*!< I2C ack value */
#define NACK_VAL                0x1              /*!< I2C nack value */

#define OV9655_ADR              0x30


uint8_t SCCB_Probe()
{
    uint8_t slave_addr = 0x0;
    while(slave_addr < 0x7f) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( slave_addr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_PORT_NUM, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
        printf("Probe id %d.\n", slave_addr);
        if( ret == ESP_OK) {
            return slave_addr;
        }
        slave_addr++;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    return 0x00;
}


void app_main(void)
{
    int i2c_master_port = I2C_PORT_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);

    i2c_driver_install(i2c_master_port, conf.mode,
    		I2C_RX_BUF_DISABLE, I2C_TX_BUF_DISABLE, 0);


    int i = 0;
    while (1) {
        printf("[%d] Running a probe!\n", i);
        uint8_t x = SCCB_Probe();
        printf("[%d] Probe finished: %d!\n", i, x);
        i++;
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
