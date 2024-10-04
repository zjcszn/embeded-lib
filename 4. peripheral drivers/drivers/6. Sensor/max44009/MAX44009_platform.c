/**
 **********************************************************************************
 * @file   MAX44009_platform.c
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  MAX44009 Ambient Light sensor driver platform dependent part
 **********************************************************************************
 *
 * Copyright (c) 2021 Mahda Embedded System (MIT License)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************************************
 */

/* Includes ---------------------------------------------------------------------*/
#include "MAX44009_platform.h"

#if defined(MAX44009_PLATFORM_AVR)
#include <avr/io.h>
#elif defined(MAX44009_PLATFORM_ESP32_IDF)
#include "sdkconfig.h"
#include "esp_system.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#endif



/**
 ==================================================================================
                        ##### Private Functions #####                              
 ==================================================================================
 */

static int8_t
Platform_Init(void)
{
#if defined(MAX44009_PLATFORM_AVR)
  TWBR = (uint8_t)(MAX44009_CPU_CLK - 1600000) / (2 * 100000);
#elif defined(MAX44009_PLATFORM_ESP32_IDF)
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
#endif
  return 0;
}


static int8_t
Platform_DeInit(void)
{
#if defined(MAX44009_PLATFORM_AVR)
#elif defined(MAX44009_PLATFORM_ESP32_IDF)
  i2c_driver_delete(MAX44009_I2C_NUM);
  gpio_reset_pin(MAX44009_SDA_GPIO);
  gpio_reset_pin(MAX44009_SCL_GPIO);
#endif
  return 0;
}


static int8_t
Platform_Send(uint8_t Address, uint8_t *Data, uint8_t Len)
{
#if defined(MAX44009_PLATFORM_AVR)
  uint8_t DataCounter = 0;

  TWCR = _BV(TWEN) | _BV(TWSTA) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
  while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends

  TWDR = Address<<1;                  // set data in data register to sending
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
  while (!CHECKBIT(TWCR, TWINT));

  for (DataCounter = 0; DataCounter < Len; DataCounter++)
  {
    TWDR = Data[DataCounter];                  // set data in data register to sending
    TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
    while (!CHECKBIT(TWCR, TWINT));
  }
  
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO); // send the STOP mode bit

#elif defined(MAX44009_PLATFORM_ESP32_IDF)
  i2c_cmd_handle_t MAX44009_i2c_cmd_handle = 0;
  Address <<= 1;
  Address &= 0xFE;

  MAX44009_i2c_cmd_handle = i2c_cmd_link_create();
  i2c_master_start(MAX44009_i2c_cmd_handle);
  i2c_master_write(MAX44009_i2c_cmd_handle, &Address, 1, 1);
  i2c_master_write(MAX44009_i2c_cmd_handle, Data, Len, 1);
  i2c_master_stop(MAX44009_i2c_cmd_handle);
  if (i2c_master_cmd_begin(MAX44009_I2C_NUM, MAX44009_i2c_cmd_handle, 1000 / portTICK_RATE_MS) != ESP_OK)
  {
    i2c_cmd_link_delete(MAX44009_i2c_cmd_handle);
    return -1;
  }
  i2c_cmd_link_delete(MAX44009_i2c_cmd_handle);
#endif
  return 0;
}

static int8_t
PlatformReceive(uint8_t Address, uint8_t *Data, uint8_t Len)
{
#if defined(MAX44009_PLATFORM_AVR)
  uint8_t DataCounter = 0;

  TWCR = _BV(TWEN) | _BV(TWSTA) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
  while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends

  TWDR = (Address<<1) | 0x01;                  // set data in data register to sending
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
  while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends

  for (DataCounter = 0; DataCounter < Len - 1; DataCounter++)
  {
    TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
    while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends
    Data[DataCounter] = TWDR;
  }
  TWCR = _BV(TWEN) | _BV(TWINT); // TWI enable
  while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends
  Data[DataCounter] = TWDR;

  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO); // send the STOP mode bit
#elif defined(MAX44009_PLATFORM_ESP32_IDF)
  i2c_cmd_handle_t MAX44009_i2c_cmd_handle = 0;
  Address <<= 1;
  Address |= 0x01;

  MAX44009_i2c_cmd_handle = i2c_cmd_link_create();
  i2c_master_start(MAX44009_i2c_cmd_handle);
  i2c_master_write(MAX44009_i2c_cmd_handle, &Address, 1, 1);
  i2c_master_read(MAX44009_i2c_cmd_handle, Data, Len, I2C_MASTER_LAST_NACK);
  i2c_master_stop(MAX44009_i2c_cmd_handle);
  if (i2c_master_cmd_begin(MAX44009_I2C_NUM, MAX44009_i2c_cmd_handle, 1000 / portTICK_RATE_MS) != ESP_OK)
  {
    i2c_cmd_link_delete(MAX44009_i2c_cmd_handle);
    return -1;
  }
  i2c_cmd_link_delete(MAX44009_i2c_cmd_handle);
#endif
  return 0;
}



/**
 ==================================================================================
                         ##### Public Functions #####                              
 ==================================================================================
 */

MAX44009_Result_t
MAX44009_Platform_Init(MAX44009_Handler_t *Handler)
{
  Handler->PlatformInit = Platform_Init;
  Handler->PlatformDeInit = Platform_DeInit;
  Handler->PlatformSend = Platform_Send;
  Handler->PlatformReceive = PlatformReceive;
  return MAX44009_OK;
}
