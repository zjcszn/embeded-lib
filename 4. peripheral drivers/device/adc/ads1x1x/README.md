# ADS1x1x Library
Driver for ADS1013, ADS1014, ADS1015, ADS1113, ADS1114, ADS1115 ADCs.


## Hardware Support
It is easy to port this library to any platform. But now it is ready for use in:
- STM32 (HAL)
- ESP32 (esp-idf)
- AVR (ATmega32)

## How To Use
1. Add `ADS1x1x.h` and `ADS1x1x.c` files to your project.  It is optional to use `ADS1x1x_platform.h` and `ADS1x1x_platform.c` files (open and config `ADS1x1x_platform.h` file).
2. Initialize platform-dependent part of handler.
4. Call `ADS1x1x_Init()`.
5. Call other functions and enjoy.

## Example
<details>
<summary>Using ADS1x1x_platform files</summary>

```c
#include <stdio.h>
#include "ADS1x1x.h"
#include "ADS1x1x_platform.h"

int main(void)
{
  ADS1x1x_Handler_t Handler = {0};
  ADS1x1x_Sample_t  Sample = {0};

  ADS1x1x_Platform_Init(&Handler);
  ADS1x1x_Init(&Handler, ADS1X1X_DEVICE_ADS1115, 0);
  ADS1x1x_SetMode(&Handler, ADS1X1X_MODE_CONTINUOUS);
  ADS1x1x_SetPGA(&Handler, ADS1X1X_PGA_2_048);
  ADS1x1x_SetRate(&Handler, ADS1X1X_RATE_16SPS);

  while (1)
  {
    ADS1x1x_SetMux(&Handler, ADS1X1X_MUX_SINGLE_0);
    ADS1x1x_StartConversion(&Handler);
    while (ADS1x1x_CheckDataReady(&Handler) == ADS1X1X_DATA_NOT_READY);
    ADS1x1x_ReadSample(&Handler, &Sample);
    printf("Voltage: %fV\r\n\r\n",
           Sample.Voltage);
  }

  ADS1x1x_DeInit(&Handler);
  return 0;
}
```
</details>


<details>
<summary>Without using ADS1x1x_platform files (esp-idf)</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "ADS1x1x.h"

#define ADS1X1X_I2C_NUM   I2C_NUM_1
#define ADS1X1X_I2C_RATE  100000
#define ADS1X1X_SCL_GPIO  GPIO_NUM_13
#define ADS1X1X_SDA_GPIO  GPIO_NUM_14

int8_t
ADS1x1x_Platform_Init(void)
{
  i2c_config_t conf = {0};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = ADS1X1X_SDA_GPIO;
  conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
  conf.scl_io_num = ADS1X1X_SCL_GPIO;
  conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
  conf.master.clk_speed = ADS1X1X_I2C_RATE;
  if (i2c_param_config(ADS1X1X_I2C_NUM, &conf) != ESP_OK)
    return -1;
  if (i2c_driver_install(ADS1X1X_I2C_NUM, conf.mode, 0, 0, 0) != ESP_OK)
    return -1;
  return 0;
}

int8_t
ADS1x1x_Platform_DeInit(void)
{
  i2c_driver_delete(ADS1X1X_I2C_NUM);
  gpio_reset_pin(ADS1X1X_SDA_GPIO);
  gpio_reset_pin(ADS1X1X_SCL_GPIO);
  return 0;
}

int8_t
ADS1x1x_Platform_Send(uint8_t Address, uint8_t *Data, uint8_t DataLen)
{
  i2c_cmd_handle_t ADS1x1x_i2c_cmd_handle = {0};
  Address <<= 1;
  Address &= 0xFE;

  ADS1x1x_i2c_cmd_handle = i2c_cmd_link_create();
  i2c_master_start(ADS1x1x_i2c_cmd_handle);
  i2c_master_write(ADS1x1x_i2c_cmd_handle, &Address, 1, 1);
  i2c_master_write(ADS1x1x_i2c_cmd_handle, Data, DataLen, 1);
  i2c_master_stop(ADS1x1x_i2c_cmd_handle);
  if (i2c_master_cmd_begin(ADS1X1X_I2C_NUM, ADS1x1x_i2c_cmd_handle, 1000 / portTICK_RATE_MS) != ESP_OK)
  {
    i2c_cmd_link_delete(ADS1x1x_i2c_cmd_handle);
    return -1;
  }
  i2c_cmd_link_delete(ADS1x1x_i2c_cmd_handle);
  return 0;
}

int8_t
ADS1x1x_Platform_Receive(uint8_t Address, uint8_t *Data, uint8_t DataLen)
{
  i2c_cmd_handle_t ADS1x1x_i2c_cmd_handle = {0};
  Address <<= 1;
  Address |= 0x01;

  ADS1x1x_i2c_cmd_handle = i2c_cmd_link_create();
  i2c_master_start(ADS1x1x_i2c_cmd_handle);
  i2c_master_write(ADS1x1x_i2c_cmd_handle, &Address, 1, 1);
  i2c_master_read(ADS1x1x_i2c_cmd_handle, Data, DataLen, I2C_MASTER_LAST_NACK);
  i2c_master_stop(ADS1x1x_i2c_cmd_handle);
  if (i2c_master_cmd_begin(ADS1X1X_I2C_NUM, ADS1x1x_i2c_cmd_handle, 1000 / portTICK_RATE_MS) != ESP_OK)
  {
    i2c_cmd_link_delete(ADS1x1x_i2c_cmd_handle);
    return -1;
  }
  i2c_cmd_link_delete(ADS1x1x_i2c_cmd_handle);
  return 0;
}


int main(void)
{
  ADS1x1x_Handler_t Handler = {0};
  ADS1x1x_Sample_t  Sample = {0};

  Handler.PlatformInit    = ADS1x1x_Platform_Init;
  Handler.PlatformDeInit  = ADS1x1x_Platform_DeInit;
  Handler.PlatformSend    = ADS1x1x_Platform_Send;
  Handler.PlatformReceive = ADS1x1x_Platform_Receive;

  ADS1x1x_Platform_Init(&Handler);
  ADS1x1x_Init(&Handler, ADS1X1X_DEVICE_ADS1115, 0);
  ADS1x1x_SetMode(&Handler, ADS1X1X_MODE_CONTINUOUS);
  ADS1x1x_SetPGA(&Handler, ADS1X1X_PGA_2_048);
  ADS1x1x_SetRate(&Handler, ADS1X1X_RATE_16SPS);

  while (1)
  {
    ADS1x1x_SetMux(&Handler, ADS1X1X_MUX_SINGLE_0);
    ADS1x1x_StartConversion(&Handler);
    while (ADS1x1x_CheckDataReady(&Handler) == ADS1X1X_DATA_NOT_READY);
    ADS1x1x_ReadSample(&Handler, &Sample);
    printf("Voltage: %fV\r\n\r\n",
           Sample.Voltage);
  }

  ADS1x1x_DeInit(&Handler);
  return 0;
}
```
</details>
