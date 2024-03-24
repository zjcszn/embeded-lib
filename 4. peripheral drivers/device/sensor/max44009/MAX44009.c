/**
 **********************************************************************************
 * @file   MAX44009.c
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  MAX44009 Ambient Light sensor handling
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
#include "MAX44009.h"



/* Private Constants ------------------------------------------------------------*/
/**
 * @brief  Internal register addresses
 */
#define MAX44009_INT_STATUS_REG   0x00
#define MAX44009_INT_ENABLE_REG   0x01
#define MAX44009_CONFIG_REG       0x02
#define MAX44009_LUX_HIGH_REG     0x03
#define MAX44009_LUX_LOW_REG      0x04
#define MAX44009_UP_THRESH_REG    0x05
#define MAX44009_DOWN_THRESH_REG  0x06
#define MAX44009_THRESH_TIME_REG  0x07

/**
 * @brief  I2C device addresses
 */
#define MAX44009_I2C_ADDRESS_A   0x4A
#define MAX44009_I2C_ADDRESS_B   0x4B



/**
 ==================================================================================
                           ##### Private Functions #####                           
 ==================================================================================
 */

static MAX44009_Result_t
MAX44009_WriteReg(MAX44009_Handler_t *Handler, uint8_t RegAddress, uint8_t Data)
{
  uint8_t Buffer[2];

  Buffer[0] = RegAddress; // send register address to set RTC pointer
  Buffer[1] = Data; // send register address to set RTC pointer
  if (Handler->PlatformSend(Handler->AddressI2C, Buffer, 2) != 0)
    return MAX44009_FAIL;
  return MAX44009_OK;
}

static MAX44009_Result_t
MAX44009_ReadReg(MAX44009_Handler_t *Handler, uint8_t RegAddress, uint8_t *Data)
{
  if (Handler->PlatformSend(Handler->AddressI2C, &RegAddress, 1) != 0)
    return MAX44009_FAIL;
  if (Handler->PlatformReceive(Handler->AddressI2C, Data, 1) != 0)
    return MAX44009_FAIL;
  return MAX44009_OK;
}



/**
 ==================================================================================
                  ##### Public Measurement Functions #####                         
 ==================================================================================
 */

/**
 * @brief  Readout of Measurement Results
 * @param  Handler: Pointer to handler
 * @param  Sample: Pointer to sample structure
 * @retval MAX44009_Result_t
 *         - MAX44009_OK: Operation was successful.
 *         - MAX44009_FAIL: Failed to send or receive data.
 */
MAX44009_Result_t
MAX44009_ReadSample(MAX44009_Handler_t *Handler, MAX44009_Sample_t *Sample)
{
  uint8_t LuxHighReg = 0;
  uint8_t LuxLowReg = 0;
  uint16_t LuxB = 0;

  if (MAX44009_ReadReg(Handler, MAX44009_LUX_HIGH_REG, &LuxHighReg) != MAX44009_OK)
    return MAX44009_FAIL;

  if (MAX44009_ReadReg(Handler, MAX44009_LUX_LOW_REG, &LuxLowReg) != MAX44009_OK)
    return MAX44009_FAIL;

  Sample->Exponent = LuxHighReg >> 4;
  Sample->Mantissa = (LuxHighReg << 4) | LuxLowReg;
  LuxB = (uint16_t)(Sample->Mantissa) << (Sample->Exponent);
  Sample->Lux = LuxB * 0.045;

  return MAX44009_OK;
}



/**
 ==================================================================================
               ##### Public Control and Status Functions #####                     
 ==================================================================================
 */

/**
 * @brief  Initializes Handler platform-independent part
 * @param  Handler: Pointer to handler
 * @retval MAX44009_Result_t
 *         - MAX44009_OK: Operation was successful.
 *         - MAX44009_FAIL: Failed to send or receive data.
 */
MAX44009_Result_t
MAX44009_Init(MAX44009_Handler_t *Handler)
{
  Handler->AddressI2C = MAX44009_I2C_ADDRESS_A;

  if (Handler->PlatformInit() != 0)
    return MAX44009_FAIL;
  return MAX44009_OK;
}

/**
 * @brief  De-initializes the Handler
 * @param  Handler: Pointer to handler
 * @retval MAX44009_Result_t
 *         - MAX44009_OK: Operation was successful.
 */
MAX44009_Result_t
MAX44009_DeInit(MAX44009_Handler_t *Handler)
{
  if (Handler->PlatformDeInit() != 0)
    return MAX44009_FAIL;
  return MAX44009_OK;
}

/**
 * @brief  Set address of MAX44009
 * @param  Handler: Pointer to handler
 * @param  Address: The address depends on ADDR pin state. You should use one of
 *                  this options:
 *         - 0: This address used when ADDR is connected GND
 *         - 1: This address used when ADDR is connected VCC
 * 
 * @retval MAX44009_Result_t
 *         - MAX44009_OK: Operation was successful.
 */
MAX44009_Result_t
MAX44009_SetAddressI2C(MAX44009_Handler_t *Handler, uint8_t Address)
{
  if (Address == 0)
    Handler->AddressI2C = MAX44009_I2C_ADDRESS_A;
  else
    Handler->AddressI2C = MAX44009_I2C_ADDRESS_B;
  return MAX44009_OK;
}
