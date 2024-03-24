# MAX44009 Library
MAX44009 sensor driver.
- Read light intensity 


## Hardware Support
It is easy to port this library to any platform. But now it is ready for use in:
- AVR (ATmega32)
- ESP32 (esp-idf)

## How To Use
1. Add `MAX44009.h` and `MAX44009.c` files to your project.  It is optional to use `MAX44009_platform.h` and `MAX44009_platform.c` files (open and config `MAX44009_platform.h` file).
2. Initialize platform-dependent part of handler.
4. Call `MAX44009_Init()`.
5. Call `MAX44009_SetAddressI2C()`.
6. Call other functions and enjoy.

## Example
<details>
<summary>Using MAX44009_platform files</summary>

```c
#include <stdio.h>
#include "MAX44009.h"
#include "MAX44009_platform.h"

int main(void)
{
  MAX44009_Handler_t Handler;
  MAX44009_Sample_t  Sample;

  MAX44009_Platform_Init(&Handler);
  MAX44009_Init(&Handler);
  MAX44009_SetAddressI2C(&Handler, 0);

  while (1)
  {
    SHT1x_ReadSample(&Handler, &Sample);
    printf("Light Intensity: %fLux\r\n\r\n",
           Sample.Lux);
  }

  MAX44009_DeInit(&Handler);
  return 0;
}
```
</details>


<details>
<summary>Without using MAX44009_platform files (esp-idf)</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "MAX44009.h"

#define MAX44009_I2C_NUM   I2C_NUM_1
#define MAX44009_I2C_RATE  100000
#define MAX44009_SCL_GPIO  GPIO_NUM_13
#define MAX44009_SDA_GPIO  GPIO_NUM_14

int8_t
MAX44009_Platform_Init(void)
{
  i2c_config_t conf = {0};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = MAX44009_SDA_GPIO;
  conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
  conf.scl_io_num = MAX44009_SCL_GPIO;
  conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
  conf.master.clk_speed = MAX44009_I2C_RATE;
  if (i2c_param_config(MAX44009_I2C_NUM, &conf) != ESP_OK)
    return -1;
  if (i2c_driver_install(MAX44009_I2C_NUM, conf.mode, 0, 0, 0) != ESP_OK)
    return -1;
  return 0;
}

int8_t
MAX44009_Platform_DeInit(void)
{
  i2c_driver_delete(MAX44009_I2C_NUM);
  gpio_reset_pin(MAX44009_SDA_GPIO);
  gpio_reset_pin(MAX44009_SCL_GPIO);
  return 0;
}

int8_t
MAX44009_Platform_Send(uint8_t Address, uint8_t *Data, uint8_t DataLen)
{
  i2c_cmd_handle_t MAX44009_i2c_cmd_handle = 0;
  Address <<= 1;
  Address &= 0xFE;

  MAX44009_i2c_cmd_handle = i2c_cmd_link_create();
  i2c_master_start(MAX44009_i2c_cmd_handle);
  i2c_master_write(MAX44009_i2c_cmd_handle, &Address, 1, 1);
  i2c_master_write(MAX44009_i2c_cmd_handle, Data, DataLen, 1);
  i2c_master_stop(MAX44009_i2c_cmd_handle);
  if (i2c_master_cmd_begin(MAX44009_I2C_NUM, MAX44009_i2c_cmd_handle, 1000 / portTICK_RATE_MS) != ESP_OK)
  {
    i2c_cmd_link_delete(MAX44009_i2c_cmd_handle);
    return -1;
  }
  i2c_cmd_link_delete(MAX44009_i2c_cmd_handle);
  return 0;
}

int8_t
MAX44009_Platform_Receive(uint8_t Address, uint8_t *Data, uint8_t DataLen)
{
  i2c_cmd_handle_t MAX44009_i2c_cmd_handle = 0;
  Address <<= 1;
  Address |= 0x01;

  MAX44009_i2c_cmd_handle = i2c_cmd_link_create();
  i2c_master_start(MAX44009_i2c_cmd_handle);
  i2c_master_write(MAX44009_i2c_cmd_handle, &Address, 1, 1);
  i2c_master_read(MAX44009_i2c_cmd_handle, Data, DataLen, I2C_MASTER_LAST_NACK);
  i2c_master_stop(MAX44009_i2c_cmd_handle);
  if (i2c_master_cmd_begin(MAX44009_I2C_NUM, MAX44009_i2c_cmd_handle, 1000 / portTICK_RATE_MS) != ESP_OK)
  {
    i2c_cmd_link_delete(MAX44009_i2c_cmd_handle);
    return -1;
  }
  i2c_cmd_link_delete(MAX44009_i2c_cmd_handle);
  return 0;
}


int main(void)
{
  MAX44009_Handler_t Handler;
  MAX44009_Sample_t  Sample;

  Handler.PlatformInit    = MAX44009_Platform_Init;
  Handler.PlatformDeInit  = MAX44009_Platform_DeInit;
  Handler.PlatformSend    = MAX44009_Platform_Send;
  Handler.PlatformReceive = MAX44009_Platform_Receive;

  MAX44009_Init(&Handler);
  MAX44009_SetAddressI2C(&Handler, 0);

  while (1)
  {
    MAX44009_ReadSample(&Handler, &Sample);
    printf("Light Intensity: %fLux\r\n\r\n",
           Sample.Lux);
  }

  MAX44009_DeInit(&Handler);
  return 0;
}
```
</details>
