/**
 **********************************************************************************
 * @file   ADS1x1x_platform.c
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  ADS1013, ADS1014, ADS1015, ADS1113, ADS1114, ADS1115 driver platform
 *         dependent part
 **********************************************************************************
 *
 * Copyright (c) 2023 Mahda Embedded System (MIT License)
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
#include "ADS1x1x_platform.h"
#if defined(ADS1X1X_PLATFORM_STM32_HAL)
#include "main.h"
#elif defined(ADS1X1X_PLATFORM_ESP32_IDF)
#include "sdkconfig.h"
#include "esp_system.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#elif defined(ADS1X1X_PLATFORM_AVR)
#include <avr/io.h>
#include <util/delay.h>
#endif


/* Private Constants ------------------------------------------------------------*/
#if defined(ADS1X1X_PLATFORM_STM32_HAL)
#define ADS1X1X_TIMEOUT 100
#endif


/* Private Macro ----------------------------------------------------------------*/
#ifndef _BV
#define _BV(bit) (1<<(bit))
#endif

#ifndef cbi
#define cbi(reg,bit) reg &= ~(_BV(bit))
#endif

#ifndef sbi
#define sbi(reg,bit) reg |= (_BV(bit))
#endif

#ifndef CHECKBIT
#define CHECKBIT(reg,bit) ((reg & _BV(bit)) ? 1 : 0)
#endif



/**
 ==================================================================================
                         ##### Public Functions #####                              
 ==================================================================================
 */

static int8_t
Platform_Init(void)
{
#if defined(ADS1X1X_PLATFORM_ESP32_IDF)
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
#elif defined(ADS1X1X_PLATFORM_AVR)
  TWBR = (uint8_t)(ADS1X1X_CPU_CLK - 1600000) / (2 * ADS1X1X_I2C_RATE);
#endif
  return 0;
}


static int8_t
Platform_DeInit(void)
{
#if defined(ADS1X1X_PLATFORM_ESP32_IDF)
  i2c_driver_delete(ADS1X1X_I2C_NUM);
  gpio_reset_pin(ADS1X1X_SDA_GPIO);
  gpio_reset_pin(ADS1X1X_SCL_GPIO);
#endif
  return 0;
}


static int8_t
Platform_WriteData(uint8_t Address, uint8_t *Data, uint8_t DataLen)
{
#if defined(ADS1X1X_PLATFORM_STM32_HAL)
  extern I2C_HandleTypeDef ADS1X1X_HI2C;
  Address <<= 1;
  if (HAL_I2C_Master_Transmit(&ADS1X1X_HI2C, Address, Data, DataLen, ADS1X1X_TIMEOUT))
    return -1;
#elif defined(ADS1X1X_PLATFORM_ESP32_IDF)
  i2c_cmd_handle_t ADS1x1x_i2c_cmd_handle = 0;
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
#elif defined(ADS1X1X_PLATFORM_AVR)
  uint8_t DataCounter = 0;

  TWCR = _BV(TWEN) | _BV(TWSTA) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
  while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends

  TWDR = Address<<1;                  // set data in data register to sending
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
  while (!CHECKBIT(TWCR, TWINT));

  for (DataCounter = 0; DataCounter < DataLen; DataCounter++)
  {
    TWDR = Data[DataCounter];                  // set data in data register to sending
    TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
    while (!CHECKBIT(TWCR, TWINT));
  }
  
  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO); // send the STOP mode bit
#endif

  return 0;
}


static int8_t
Platform_ReadData(uint8_t Address, uint8_t *Data, uint8_t DataLen)
{
#if defined(ADS1X1X_PLATFORM_STM32_HAL)
  extern I2C_HandleTypeDef ADS1X1X_HI2C;
  Address <<= 1;
  if (HAL_I2C_Master_Receive(&ADS1X1X_HI2C, Address, Data, DataLen, ADS1X1X_TIMEOUT))
    return -1;
#elif defined(ADS1X1X_PLATFORM_ESP32_IDF)
  i2c_cmd_handle_t ADS1x1x_i2c_cmd_handle = 0;
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
#elif defined(ADS1X1X_PLATFORM_AVR)
  uint8_t DataCounter = 0;

  TWCR = _BV(TWEN) | _BV(TWSTA) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
  while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends

  TWDR = (Address<<1) | 0x01;                  // set data in data register to sending
  TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
  while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends

  for (DataCounter = 0; DataCounter < DataLen - 1; DataCounter++)
  {
    TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT); // TWI enable *** acknowledge enable
    while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends
    Data[DataCounter] = TWDR;
  }ADS1x1x_i2c_cmd_handle
  TWCR = _BV(TWEN) | _BV(TWINT); // TWI enable
  while (!CHECKBIT(TWCR, TWINT)); // wait until the process ends
  Data[DataCounter] = TWDR;

  TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO); // send the STOP mode bit
#endif

  return 0;
}


void
ADS1x1x_Platform_Init(ADS1x1x_Handler_t *Handler)
{
  Handler->PlatformInit = Platform_Init;
  Handler->PlatformDeInit = Platform_DeInit;
  Handler->PlatformSend = Platform_WriteData;
  Handler->PlatformReceive = Platform_ReadData;
}
