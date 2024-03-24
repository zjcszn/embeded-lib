/**
 **********************************************************************************
 * @file   ADS1110.c
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  ADS1110 ACD driver
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
#include "ADS1110.h"



/* Private Constants ------------------------------------------------------------*/
/**
 * @brief  Configuration register bits
 */
#define ADS1110_CONFIG_REG_ST_DRDY  7
#define ADS1110_CONFIG_REG_SC       4
#define ADS1110_CONFIG_REG_DR1      3
#define ADS1110_CONFIG_REG_DR0      2
#define ADS1110_CONFIG_REG_PGA1     1
#define ADS1110_CONFIG_REG_PGA0     0


/**
 * @brief  Reference voltage
 */
#define ADS1110_REF_VOLTAGE   2.048f



/**
 ==================================================================================
                       ##### Private Functions #####                               
 ==================================================================================
 */

static inline int8_t
ADS1110_ReadAll(ADS1110_Handler_t *Handler, uint8_t *Data)
{
  return Handler->PlatformReceive(Handler->AddressI2C, Data, 3);
}

static int8_t
ADS1110_ReadConfigurationRegister(ADS1110_Handler_t *Handler, uint8_t *Data)
{
  uint8_t Buffer[3] = {0};
  int8_t RetVal = 0;

  RetVal = ADS1110_ReadAll(Handler, Buffer);
  *Data = Buffer[2];

  return RetVal;
}

static int8_t
ADS1110_WriteConfigurationRegister(ADS1110_Handler_t *Handler, uint8_t Data)
{
  return Handler->PlatformSend(Handler->AddressI2C, &Data, 1);
}

static ADS1110_Result_t
ADS1110_DoCheckDataReady(ADS1110_Handler_t *Handler, uint8_t ConfigReg)
{
  if (ConfigReg & (1 << ADS1110_CONFIG_REG_ST_DRDY))
    return ADS1110_DATA_NOT_READY;
  
  return ADS1110_OK;
}

static void
ADS1110_ConvSample(ADS1110_Handler_t *Handler, ADS1110_Sample_t *Sample)
{
  float ResDiv = 0.0f;
  float Gain = 0.0f;

  switch (Handler->Gain)
  {
  case ADS1110_GAIN_1:
    Gain = 1.0f;
    break;

  case ADS1110_GAIN_2:
    Gain = 2.0f;
    break;

  case ADS1110_GAIN_4:
    Gain = 4.0f;
    break;

  case ADS1110_GAIN_8:
    Gain = 8.0f;
    break;
  }

  switch (Handler->Rate)
  {
  case ADS1110_RATE_240MPS:
    ResDiv = 2048.0f; 
    break;

  case ADS1110_RATE_60MPS:
    ResDiv = 8192.0f;
    break;

  case ADS1110_RATE_30MPS:
    ResDiv = 16384.0f;
    break;

  case ADS1110_RATE_15MPS:
    ResDiv = 32768.0f;
    break;
  }
  
  Sample->Voltage = ((float)(Sample->Raw) * (ADS1110_REF_VOLTAGE / ResDiv)) / Gain;
}



/**
 ==================================================================================
                            ##### Public Functions #####                           
 ==================================================================================
 */

/**
 * @brief  Initializer function
 * @param  Handler: Pointer to handler
 * @param  Address: The address depends on the ordering number on the ADS1110
 *                  package.
 * 
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 *         - ADS1110_FAIL: Failed to send or receive data.
 *         - ADS1110_INVALID_PARAM: One of parameters is invalid.
 */
ADS1110_Result_t
ADS1110_Init(ADS1110_Handler_t *Handler, ADS1110_Address_t Address)
{
  if (!Handler->PlatformSend ||
      !Handler->PlatformReceive)
    return ADS1110_INVALID_PARAM;

  if ((Address >= 0 && Address <= 7))
    Handler->AddressI2C = ADS1110_ADDRESS_A0 + Address;
  else if ((Address >= ADS1110_ADDRESS_A0 && Address <= ADS1110_ADDRESS_A7))
    Handler->AddressI2C = Address;
  else
    return ADS1110_INVALID_PARAM;

  if (Handler->PlatformInit)
  {
    if (Handler->PlatformInit() != 0)
      return ADS1110_FAIL;
  }

  Handler->Mode = ADS1110_MODE_CONTINUOUS;
  Handler->Gain = ADS1110_GAIN_1;
  Handler->Rate = ADS1110_RATE_15MPS;
  
  return ADS1110_OK;
}

/**
 * @brief  Deinitialize function
 * @param  Handler: Pointer to handler
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 *         - ADS1110_FAIL: Failed to send or receive data.
 */
ADS1110_Result_t
ADS1110_DeInit(ADS1110_Handler_t *Handler)
{
  if (Handler->PlatformDeInit)
  {
    if (Handler->PlatformDeInit() != 0)
      return ADS1110_FAIL;
  }
  return ADS1110_OK;
}

/**
 * @brief  Send general call reset command
 * @note   This command will send to address 0x00 and may adversely affect all
 *         devices on the bus that support the general call address 0x00. Use this
 *         function carefully.
 * 
 * @note   After calling this function, you must initialize the handler with
 *         ADS1110_Init.
 *         
 * @param  Handler: Pointer to handler
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 */
ADS1110_Result_t
ADS1110_GeneralCallReset(ADS1110_Handler_t *Handler)
{
  uint8_t CMD = 0x06;

  Handler->PlatformSend(0x00, &CMD, 1);

  return ADS1110_OK;
}

/**
 * @brief  Set measurement mode
 * @param  Handler: Pointer to handler
 * @param  Mode: Specify measurement mode
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 *         - ADS1110_FAIL: Failed to send or receive data.
 *         - ADS1110_INVALID_PARAM: One of parameters is invalid.
 */
ADS1110_Result_t
ADS1110_SetMode(ADS1110_Handler_t *Handler, ADS1110_Mode_t Mode)
{
  uint8_t ConfigReg = 0;

  if (ADS1110_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1110_FAIL;

  switch (Mode)
  {
  case ADS1110_MODE_CONTINUOUS:
    ConfigReg &= ~(1 << ADS1110_CONFIG_REG_SC);
    break;

  case ADS1110_MODE_SINGLESHOT:
    ConfigReg |= (1 << ADS1110_CONFIG_REG_SC);
    break;

  default:
    return ADS1110_INVALID_PARAM;
    break;
  }

  if (ADS1110_WriteConfigurationRegister(Handler, ConfigReg) < 0)
    return ADS1110_FAIL;

  Handler->Mode = Mode;

  return ADS1110_OK;
}

/**
 * @brief  Set PGA gain
 * @param  Handler: Pointer to handler
 * @param  Gain: Specify PGA gain
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 *         - ADS1110_FAIL: Failed to send or receive data.
 *         - ADS1110_INVALID_PARAM: One of parameters is invalid.
 */
ADS1110_Result_t
ADS1110_SetGain(ADS1110_Handler_t *Handler, ADS1110_Gain_t Gain)
{
  uint8_t ConfigReg = 0;

  if (ADS1110_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1110_FAIL;

  ConfigReg &= ~(1 << ADS1110_CONFIG_REG_PGA1);
  ConfigReg &= ~(1 << ADS1110_CONFIG_REG_PGA0);

  switch (Gain)
  {
  case ADS1110_GAIN_1:
    break;

  case ADS1110_GAIN_2:
    ConfigReg |= (1 << ADS1110_CONFIG_REG_PGA0);
    break;

  case ADS1110_GAIN_4:
    ConfigReg |= (1 << ADS1110_CONFIG_REG_PGA1);
    break;

  case ADS1110_GAIN_8:
    ConfigReg |= (1 << ADS1110_CONFIG_REG_PGA0);
    ConfigReg |= (1 << ADS1110_CONFIG_REG_PGA1);
    break;

  default:
    return ADS1110_INVALID_PARAM;
    break;
  }

  if (ADS1110_WriteConfigurationRegister(Handler, ConfigReg) < 0)
    return ADS1110_FAIL;

  Handler->Gain = Gain;

  return ADS1110_OK;
}

/**
 * @brief  Set measurement rate
 * @param  Handler: Pointer to handler
 * @param  Rate: Specify measurement rate
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 *         - ADS1110_FAIL: Failed to send or receive data.
 *         - ADS1110_INVALID_PARAM: One of parameters is invalid.
 */
ADS1110_Result_t
ADS1110_SetRate(ADS1110_Handler_t *Handler, ADS1110_Rate_t Rate)
{
  uint8_t ConfigReg = 0;

  if (ADS1110_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1110_FAIL;

  ConfigReg &= ~(1 << ADS1110_CONFIG_REG_DR1);
  ConfigReg &= ~(1 << ADS1110_CONFIG_REG_DR0);

  switch (Rate)
  {
  case ADS1110_RATE_240MPS:
    break;

  case ADS1110_RATE_60MPS:
    ConfigReg |= (1 << ADS1110_CONFIG_REG_DR0);
    break;

  case ADS1110_RATE_30MPS:
    ConfigReg |= (1 << ADS1110_CONFIG_REG_DR1);
    break;

  case ADS1110_RATE_15MPS:
    ConfigReg |= (1 << ADS1110_CONFIG_REG_DR0);
    ConfigReg |= (1 << ADS1110_CONFIG_REG_DR1);
    break;

  default:
    return ADS1110_INVALID_PARAM;
    break;
  }

  if (ADS1110_WriteConfigurationRegister(Handler, ConfigReg) < 0)
    return ADS1110_FAIL;

  Handler->Rate = Rate;

  return ADS1110_OK;
}

/**
 * @brief  Start conversion
 * @param  Handler: Pointer to handler
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 *         - ADS1110_FAIL: Failed to send or receive data.
 */
ADS1110_Result_t
ADS1110_StartConversion(ADS1110_Handler_t *Handler)
{
  uint8_t ConfigReg = 0;

  if (ADS1110_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1110_FAIL;

  ConfigReg |= (1 << ADS1110_CONFIG_REG_ST_DRDY);

  if (ADS1110_WriteConfigurationRegister(Handler, ConfigReg) < 0)
    return ADS1110_FAIL;

  return ADS1110_OK;
}

/**
 * @brief  Checks if the data is ready or not
 * @param  Handler: Pointer to handler
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Date is ready.
 *         - ADS1110_FAIL: Failed to send or receive data.
 *         - ADS1110_DATA_NOT_READY: Data is not ready.
 */
ADS1110_Result_t
ADS1110_CheckDataReady(ADS1110_Handler_t *Handler)
{
  uint8_t ConfigReg = 0;

  if (ADS1110_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1110_FAIL;

  return ADS1110_DoCheckDataReady(Handler, ConfigReg);
}

/**
 * @brief  Read sample
 * @param  Handler: Pointer to handler
 * @param  Sample: Pointer to sample buffer
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 *         - ADS1110_FAIL: Failed to send or receive data.
 *         - ADS1110_DATA_NOT_READY: Data is not ready.
 */
ADS1110_Result_t
ADS1110_ReadSample(ADS1110_Handler_t *Handler, ADS1110_Sample_t *Sample)
{
  uint8_t Buffer[3] = {0};

  if (ADS1110_ReadAll(Handler, Buffer) < 0)
    return ADS1110_FAIL;

  if (ADS1110_DoCheckDataReady(Handler, Buffer[2]) == ADS1110_DATA_NOT_READY)
    return ADS1110_DATA_NOT_READY;

  Sample->Raw = (int16_t) ((Buffer[0] << 8) | Buffer[1]);
  ADS1110_ConvSample(Handler, Sample);

  return ADS1110_OK;
}
