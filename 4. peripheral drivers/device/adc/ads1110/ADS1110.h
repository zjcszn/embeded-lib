/**
 **********************************************************************************
 * @file   ADS1110.h
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

/* Define to prevent recursive inclusion ----------------------------------------*/
#ifndef	_ADS1110_H_
#define _ADS1110_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ---------------------------------------------------------------------*/
#include <stdint.h>


/* Exported Constants -----------------------------------------------------------*/
/**
 * @brief  I2C device addresses
 */
#define ADS1110_I2C_ADDRESS_BASE    0x48


/* Exported Data Types ----------------------------------------------------------*/
/**
 * @brief  Library functions result data type
 */
typedef enum ADS1110_Result_e
{
  ADS1110_OK              = 0,
  ADS1110_FAIL            = 1,
  ADS1110_INVALID_PARAM   = 2,
  ADS1110_DATA_NOT_READY  = 3
} ADS1110_Result_t;

/**
 * @brief  I2C Address
 */
typedef enum ADS1110_Address_e
{
  ADS1110_ADDRESS_A0 = ADS1110_I2C_ADDRESS_BASE,
  ADS1110_ADDRESS_A1,
  ADS1110_ADDRESS_A2,
  ADS1110_ADDRESS_A3,
  ADS1110_ADDRESS_A4,
  ADS1110_ADDRESS_A5,
  ADS1110_ADDRESS_A6,
  ADS1110_ADDRESS_A7,
} ADS1110_Address_t;

/**
 * @brief  Measuring mode
 */
typedef enum ADS1110_Mode_e
{
  ADS1110_MODE_CONTINUOUS = 0,
  ADS1110_MODE_SINGLESHOT = 1
} ADS1110_Mode_t;

/**
 * @brief  Measuring Speed in Periodic mode
 */
typedef enum ADS1110_Rate_e
{
  ADS1110_RATE_240MPS  = 0,
  ADS1110_RATE_60MPS   = 1,
  ADS1110_RATE_30MPS   = 2,
  ADS1110_RATE_15MPS   = 3
} ADS1110_Rate_t;


/**
 * @brief  PGA Gain
 */
typedef enum ADS1110_Gain_e
{
  ADS1110_GAIN_1  = 0,
  ADS1110_GAIN_2  = 1,
  ADS1110_GAIN_4  = 2,
  ADS1110_GAIN_8  = 3,
} ADS1110_Gain_t;


/**
 * @brief  Sample
 */
typedef struct ADS1110_Sample_s
{
  int16_t Raw;
  float Voltage;
} ADS1110_Sample_t;


/**
 * @brief  Function type for Initialize/Deinitialize the platform dependent layer.
 * @retval 
 *         -  0: The operation was successful.
 *         - -1: The operation failed. 
 */
typedef int8_t (*ADS1110_PlatformInitDeinit_t)(void);

/**
 * @brief  Function type for Send/Receive data to/from the slave.
 * @param  Address: Address of slave (0 <= Address <= 127)
 * @param  Data: Pointer to data
 * @param  Len: data len in Bytes
 * @retval 
 *         -  0: The operation was successful.
 *         - -1: Failed to send/receive.
 *         - -2: Bus is busy.
 *         - -3: Slave doesn't ACK the transfer.
 */
typedef int8_t (*ADS1110_PlatformSendReceive_t)(uint8_t Address,
                                                uint8_t *Data, uint8_t Len);


/**
 * @brief  Handler data type
 * @note   User must initialize this this functions before using library:
 *         - PlatformInit
 *         - PlatformDeInit
 *         - PlatformSend
 *         - PlatformReceive
 * @note   If success the functions must return 0 
 */
typedef struct ADS1110_Handler_s
{
  ADS1110_Address_t AddressI2C;
  ADS1110_Mode_t Mode;
  ADS1110_Gain_t Gain;
  ADS1110_Rate_t Rate;

  // Initializes platform dependent layer
  ADS1110_PlatformInitDeinit_t PlatformInit;
  // De-initializes platform dependent layer
  ADS1110_PlatformInitDeinit_t PlatformDeInit;
  // Send Data to the ADS1110
  ADS1110_PlatformSendReceive_t PlatformSend;
  // Receive Data from the ADS1110
  ADS1110_PlatformSendReceive_t PlatformReceive;
} ADS1110_Handler_t;



/**
 ==================================================================================
                               ##### Functions #####                               
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
ADS1110_Init(ADS1110_Handler_t *Handler, ADS1110_Address_t Address);


/**
 * @brief  Deinitialize function
 * @param  Handler: Pointer to handler
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 *         - ADS1110_FAIL: Failed to send or receive data.
 */
ADS1110_Result_t
ADS1110_DeInit(ADS1110_Handler_t *Handler);


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
ADS1110_GeneralCallReset(ADS1110_Handler_t *Handler);


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
ADS1110_SetMode(ADS1110_Handler_t *Handler, ADS1110_Mode_t Mode);


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
ADS1110_SetGain(ADS1110_Handler_t *Handler, ADS1110_Gain_t Gain);


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
ADS1110_SetRate(ADS1110_Handler_t *Handler, ADS1110_Rate_t Rate);


/**
 * @brief  Start conversion
 * @param  Handler: Pointer to handler
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Operation was successful.
 *         - ADS1110_FAIL: Failed to send or receive data.
 */
ADS1110_Result_t
ADS1110_StartConversion(ADS1110_Handler_t *Handler);


/**
 * @brief  Checks if the data is ready or not
 * @param  Handler: Pointer to handler
 * @retval ADS1110_Result_t
 *         - ADS1110_OK: Date is ready.
 *         - ADS1110_FAIL: Failed to send or receive data.
 *         - ADS1110_DATA_NOT_READY: Data is not ready.
 */
ADS1110_Result_t
ADS1110_CheckDataReady(ADS1110_Handler_t *Handler);


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
ADS1110_ReadSample(ADS1110_Handler_t *Handler, ADS1110_Sample_t *Sample);



#ifdef __cplusplus
}
#endif

#endif //! _ADS1110_H_
