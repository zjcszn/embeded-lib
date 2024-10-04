/**
 **********************************************************************************
 * @file   ADS1x1x.c
 * @author Hossein.M (https://github.com/Hossein-M98)
 * @brief  ADS1013, ADS1014, ADS1015, ADS1113, ADS1114, ADS1115 driver
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
#include "ADS1x1x.h"


/* Private Constants ------------------------------------------------------------*/
/**
 * @brief  I2C device addresses
 */
#define ADS1X1X_I2C_ADDRESS_BASE 0x48

/**
 * @brief  Register addresses
*/
#define ADS1X1X_REG_ADDR_CONVERSION 0
#define ADS1X1X_REG_ADDR_CONFIG     1
#define ADS1X1X_REG_ADDR_LO_THRESH  2
#define ADS1X1X_REG_ADDR_HI_THRESH  3

/**
 * @brief  Configuration register bits
 */
#define ADS1X1X_CONFIG_REG_OS         15
#define ADS1X1X_CONFIG_REG_MUX2       14
#define ADS1X1X_CONFIG_REG_MUX1       13
#define ADS1X1X_CONFIG_REG_MUX0       12
#define ADS1X1X_CONFIG_REG_PGA2       11
#define ADS1X1X_CONFIG_REG_PGA1       10
#define ADS1X1X_CONFIG_REG_PGA0       9
#define ADS1X1X_CONFIG_REG_MODE       8
#define ADS1X1X_CONFIG_REG_DR2        7
#define ADS1X1X_CONFIG_REG_DR1        6
#define ADS1X1X_CONFIG_REG_DR0        5
#define ADS1X1X_CONFIG_REG_COMP_MODE  4
#define ADS1X1X_CONFIG_REG_COMP_POL   3
#define ADS1X1X_CONFIG_REG_COMP_LAT   2
#define ADS1X1X_CONFIG_REG_COMP_QUE1  1
#define ADS1X1X_CONFIG_REG_COMP_QUE0  0

/**
 * @brief  Reference voltage
 */
#define ADS1X1X_REF_VOLTAGE 2.048f


/* Private Macro ----------------------------------------------------------------*/
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif



/**
 ==================================================================================
                       ##### Private Functions #####
 ==================================================================================
 */

static int8_t
ADS1x1x_WriteReg(ADS1x1x_Handler_t *Handler,
                 uint8_t StartReg, uint16_t Reg)
{
  uint8_t Buffer[3] = {0};

  Buffer[0] = StartReg; // send register address to set RTC pointer

  Buffer[1] = (uint8_t)(Reg >> 8);
  Buffer[2] = (uint8_t)Reg;

  return Handler->PlatformSend(Handler->AddressI2C,
                               Buffer, 3);
}

static int8_t
ADS1x1x_ReadReg(ADS1x1x_Handler_t *Handler,
                uint8_t StartReg, uint16_t *Reg)
{
  int8_t RetVal = 0;
  uint8_t Buffer[2] = {0};

  RetVal = Handler->PlatformSend(Handler->AddressI2C,
                                 &StartReg, 1);
  if (RetVal < 0)
    return RetVal;

  RetVal = Handler->PlatformReceive(Handler->AddressI2C,
                                    Buffer, 2);
  if (RetVal < 0)
    return RetVal;

  *Reg = (uint16_t)(Buffer[0] << 8);
  *Reg |= (uint16_t)Buffer[1];

  return 0;
}

static inline int8_t
ADS1x1x_ReadConversionRegister(ADS1x1x_Handler_t *Handler, uint16_t *Reg)
{
  return ADS1x1x_ReadReg(Handler, ADS1X1X_REG_ADDR_CONVERSION, Reg);
}

static inline int8_t
ADS1x1x_ReadConfigurationRegister(ADS1x1x_Handler_t *Handler, uint16_t *Reg)
{
  return ADS1x1x_ReadReg(Handler, ADS1X1X_REG_ADDR_CONFIG, Reg);
}

static inline int8_t
ADS1x1x_WriteConfigurationRegister(ADS1x1x_Handler_t *Handler, uint16_t Reg)
{
  return ADS1x1x_WriteReg(Handler, ADS1X1X_REG_ADDR_CONFIG, Reg);
}

static inline void
ADS1x1x_ConvSample(ADS1x1x_Handler_t *Handler, ADS1x1x_Sample_t *Sample)
{
  const float FS_Table[8] =
      {6.144f, 4.096f, 2.048f, 1.024f, 0.512f, 0.256f, 0.256f, 0.256f};
  float Rage = 0;
  
  if (Handler->Device == ADS1X1X_DEVICE_ADS1113 ||
      Handler->Device == ADS1X1X_DEVICE_ADS1114 ||
      Handler->Device == ADS1X1X_DEVICE_ADS1115)
    Rage = 32768.0f;
  else if (Handler->Device == ADS1X1X_DEVICE_ADS1013 ||
           Handler->Device == ADS1X1X_DEVICE_ADS1014 ||
           Handler->Device == ADS1X1X_DEVICE_ADS1015)
    Rage = 2048.0f;

  Sample->Voltage = ((float)(Sample->Raw) / Rage) * FS_Table[Handler->PGA];
}



/**
 ==================================================================================
                            ##### Public Functions #####
 ==================================================================================
 */

/**
 * @brief  Initializer function
 * @param  Handler: Pointer to handler
 * @param  Device: Device type
 * @param  Address: The address depends on ADDR pin state.You should use one of
 *                  this options:
 *         - 0: This address used when ADDR is connected GND
 *         - 1: This address used when ADDR is connected VDD
 *         - 2: This address used when ADDR is connected SDA
 *         - 3: This address used when ADDR is connected SCL
 * 
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 *         - ADS1X1X_INVALID_PARAM: One of parameters is invalid.
 */
ADS1x1x_Result_t
ADS1x1x_Init(ADS1x1x_Handler_t *Handler, ADS1x1x_Device_t Device,
             uint8_t Address)
{
  ADS1x1x_Result_t Result = ADS1X1X_OK;

  if (!Handler->PlatformSend ||
      !Handler->PlatformReceive ||
      (Device != ADS1X1X_DEVICE_ADS1013 &&
       Device != ADS1X1X_DEVICE_ADS1014 &&
       Device != ADS1X1X_DEVICE_ADS1015 &&
       Device != ADS1X1X_DEVICE_ADS1113 &&
       Device != ADS1X1X_DEVICE_ADS1114 &&
       Device != ADS1X1X_DEVICE_ADS1115))
    return ADS1X1X_INVALID_PARAM;

  Result = ADS1x1x_SetAddressI2C(Handler, Address);
  if (Result != ADS1X1X_OK)
    return Result;

  if (Handler->PlatformInit)
  {
    if (Handler->PlatformInit() != 0)
      return ADS1X1X_FAIL;
  }

  Handler->Device = Device;
  Handler->PGA = ADS1X1X_PGA_2_048;
  Handler->Mode = ADS1X1X_MODE_SINGLESHOT;

  // Set default configuration
  if (ADS1x1x_WriteConfigurationRegister(Handler, 0x8583) < 0)
    return ADS1X1X_FAIL;

  return ADS1X1X_OK;
}

/**
 * @brief  Deinitialize function
 * @param  Handler: Pointer to handler
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 */
ADS1x1x_Result_t
ADS1x1x_DeInit(ADS1x1x_Handler_t *Handler)
{
  if (Handler->PlatformDeInit)
  {
    if (Handler->PlatformDeInit() != 0)
      return ADS1X1X_FAIL;
  }
  return ADS1X1X_OK;
}

/**
 * @brief  Set I2C Address
 * @param  Handler: Pointer to handler
 * @param  Address: The address depends on ADDR pin state.You should use one of
 *                  this options:
 *         - 0: This address used when ADDR is connected GND
 *         - 1: This address used when ADDR is connected VDD
 *         - 2: This address used when ADDR is connected SDA
 *         - 3: This address used when ADDR is connected SCL
 * 
 * @retval SHT3x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_INVALID_PARAM: One of parameters is invalid.
 */
ADS1x1x_Result_t
ADS1x1x_SetAddressI2C(ADS1x1x_Handler_t *Handler, uint8_t Address)
{
  switch (Address)
  {
  case 0:
  case ADS1X1X_I2C_ADDRESS_BASE:
  case ADS1X1X_I2C_ADDRESS_BASE << 1:
    Handler->AddressI2C = ADS1X1X_I2C_ADDRESS_BASE;
    break;

  case 1:
  case (ADS1X1X_I2C_ADDRESS_BASE + 1):
  case (ADS1X1X_I2C_ADDRESS_BASE + 1) << 1:
    Handler->AddressI2C = (ADS1X1X_I2C_ADDRESS_BASE + 1);
    break;
    
  case 2:
  case (ADS1X1X_I2C_ADDRESS_BASE + 2):
  case (ADS1X1X_I2C_ADDRESS_BASE + 2) << 1:
    Handler->AddressI2C = (ADS1X1X_I2C_ADDRESS_BASE + 2);
    break;

  case 3:
  case (ADS1X1X_I2C_ADDRESS_BASE + 3):
  case (ADS1X1X_I2C_ADDRESS_BASE + 3) << 1:
    Handler->AddressI2C = (ADS1X1X_I2C_ADDRESS_BASE + 3);
    break;
  
  default:
    return ADS1X1X_INVALID_PARAM;
  }

  return ADS1X1X_OK;
}

/**
 * @brief  Send general call reset command
 * @note   This command will send to address 0x00 and may adversely affect all
 *         devices on the bus that support the general call address 0x00. Use this
 *         function carefully.
 *
 * @note   After calling this function, you must initialize the handler with
 *         ADS1x1x_Init.
 *
 * @param  Handler: Pointer to handler
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 */
ADS1x1x_Result_t
ADS1x1x_GeneralCallReset(ADS1x1x_Handler_t *Handler)
{
  uint8_t CMD = 0x06;

  Handler->PlatformSend(0x00, &CMD, 1);

  return ADS1X1X_OK;
}

/**
 * @brief  Set PGA configuration
 * @param  Handler: Pointer to handler
 * @param  PGA: Specify PGA configuration
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 *         - ADS1X1X_INVALID_PARAM: One of parameters is invalid.
 *         - ADS1X1X_FEATURE_NOT_AVL: Feature not available.
 */
ADS1x1x_Result_t
ADS1x1x_SetPGA(ADS1x1x_Handler_t *Handler, ADS1x1x_PGA_t PGA)
{
  uint16_t ConfigReg = 0;

  if (PGA > 7)
    return ADS1X1X_INVALID_PARAM;

  if (Handler->Device != ADS1X1X_DEVICE_ADS1014 &&
      Handler->Device != ADS1X1X_DEVICE_ADS1015 &&
      Handler->Device != ADS1X1X_DEVICE_ADS1114 &&
      Handler->Device != ADS1X1X_DEVICE_ADS1115)
    return ADS1X1X_FEATURE_NOT_AVL;
  
  if (ADS1x1x_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1X1X_FAIL;

  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_OS);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_PGA2);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_PGA1);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_PGA0);

  ConfigReg |= (PGA << ADS1X1X_CONFIG_REG_PGA0);


  if (ADS1x1x_WriteConfigurationRegister(Handler, ConfigReg) < 0)
    return ADS1X1X_FAIL;

  Handler->PGA = PGA;

  return ADS1X1X_OK;
}

/**
 * @brief  Set measurement mode
 * @param  Handler: Pointer to handler
 * @param  Mode: Specify measurement mode
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 *         - ADS1X1X_INVALID_PARAM: One of parameters is invalid.
 */
ADS1x1x_Result_t
ADS1x1x_SetMode(ADS1x1x_Handler_t *Handler, ADS1x1x_Mode_t Mode)
{
  uint16_t ConfigReg = 0;

  if (Mode > 1)
    return ADS1X1X_INVALID_PARAM;

  if (ADS1x1x_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1X1X_FAIL;

  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_OS);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_MODE);
  ConfigReg |= (Mode << ADS1X1X_CONFIG_REG_MODE);

  if (ADS1x1x_WriteConfigurationRegister(Handler, ConfigReg) < 0)
    return ADS1X1X_FAIL;

  Handler->Mode = Mode;

  return ADS1X1X_OK;
}

/**
 * @brief  Set measurement rate
 * @param  Handler: Pointer to handler
 * @param  Rate: Specify measurement rate
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 *         - ADS1X1X_INVALID_PARAM: One of parameters is invalid.
 */
ADS1x1x_Result_t
ADS1x1x_SetRate(ADS1x1x_Handler_t *Handler, ADS1x1x_Rate_t Rate)
{
  uint16_t ConfigReg = 0;

  if (Rate > 7)
    return ADS1X1X_INVALID_PARAM;

  if (ADS1x1x_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1X1X_FAIL;

  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_OS);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_DR2);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_DR1);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_DR0);

  ConfigReg |= (Rate << ADS1X1X_CONFIG_REG_DR0);

  if (ADS1x1x_WriteConfigurationRegister(Handler, ConfigReg) < 0)
    return ADS1X1X_FAIL;

  return ADS1X1X_OK;
}

/**
 * @brief  Set Mux channel
 * @param  Handler: Pointer to handler
 * @param  Mux: Specify Mux channel
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 *         - ADS1X1X_INVALID_PARAM: One of parameters is invalid.
 *         - ADS1X1X_FEATURE_NOT_AVL: Feature not available.
 */
ADS1x1x_Result_t
ADS1x1x_SetMux(ADS1x1x_Handler_t *Handler, ADS1x1x_Mux_t Mux)
{
  uint16_t ConfigReg = 0;

  if (Handler->Device != ADS1X1X_DEVICE_ADS1015 &&
      Handler->Device != ADS1X1X_DEVICE_ADS1115)
    return ADS1X1X_FEATURE_NOT_AVL;

  if (Mux > 7)
    return ADS1X1X_INVALID_PARAM;

  if (ADS1x1x_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1X1X_FAIL;

  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_OS);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_MUX2);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_MUX1);
  ConfigReg &= ~(1 << ADS1X1X_CONFIG_REG_MUX0);

  ConfigReg |= (Mux << ADS1X1X_CONFIG_REG_MUX0);

  if (ADS1x1x_WriteConfigurationRegister(Handler, ConfigReg) < 0)
    return ADS1X1X_FAIL;

  if (ADS1x1x_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1X1X_FAIL;

  return ADS1X1X_OK;
}

/**
 * @brief  Start conversion (single shot)
 * @param  Handler: Pointer to handler
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 */
ADS1x1x_Result_t
ADS1x1x_StartConversion(ADS1x1x_Handler_t *Handler)
{
  uint16_t ConfigReg = 0;

  if (Handler->Mode == ADS1X1X_MODE_CONTINUOUS)
    return ADS1X1X_OK;

  if (ADS1x1x_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1X1X_FAIL;

  ConfigReg |= (1 << ADS1X1X_CONFIG_REG_OS);

  if (ADS1x1x_WriteConfigurationRegister(Handler, ConfigReg) < 0)
    return ADS1X1X_FAIL;

  return ADS1X1X_OK;
}

/**
 * @brief  Checks if the data is ready or not
 * @param  Handler: Pointer to handler
 * @retval ADS1110_Result_t
 *         - ADS1X1X_OK: Date is ready.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 *         - ADS1X1X_DATA_NOT_READY: Data is not ready.
 */
ADS1x1x_Result_t
ADS1x1x_CheckDataReady(ADS1x1x_Handler_t *Handler)
{
  uint16_t ConfigReg = 0;

  if (Handler->Mode == ADS1X1X_MODE_CONTINUOUS)
    return ADS1X1X_OK;

  if (ADS1x1x_ReadConfigurationRegister(Handler, &ConfigReg) < 0)
    return ADS1X1X_FAIL;

  if (ConfigReg & (1 << ADS1X1X_CONFIG_REG_OS))
    return ADS1X1X_OK;

  return ADS1X1X_DATA_NOT_READY;
}

/**
 * @brief  Read sample
 * @param  Handler: Pointer to handler
 * @param  Sample: Pointer to sample buffer
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 */
ADS1x1x_Result_t
ADS1x1x_ReadSample(ADS1x1x_Handler_t *Handler, ADS1x1x_Sample_t *Sample)
{
  if (ADS1x1x_ReadConversionRegister(Handler, (uint16_t *) &(Sample->Raw)) < 0)
    return ADS1X1X_FAIL;

  ADS1x1x_ConvSample(Handler, Sample);

  return ADS1X1X_OK;
}
