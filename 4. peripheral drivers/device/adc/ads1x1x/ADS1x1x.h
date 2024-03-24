/**
 **********************************************************************************
 * @file   ADS1x1x.h
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

/* Define to prevent recursive inclusion ----------------------------------------*/
#ifndef	_ADS1X1X_H_
#define _ADS1X1X_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ---------------------------------------------------------------------*/
#include <stdint.h>


/* Exported Data Types ----------------------------------------------------------*/
/**
 * @brief  Library functions result data type
 */
typedef enum ADS1x1x_Result_e
{
  ADS1X1X_OK              = 0,
  ADS1X1X_FAIL            = 1,
  ADS1X1X_INVALID_PARAM   = 2,
  ADS1X1X_FEATURE_NOT_AVL = 3,
  ADS1X1X_DATA_NOT_READY  = 4,
} ADS1x1x_Result_t;

/**
 * @brief  Device Type
 */
typedef enum ADS1x1x_Device_e
{
  ADS1X1X_DEVICE_ADS1013 = 0,
  ADS1X1X_DEVICE_ADS1014 = 1,
  ADS1X1X_DEVICE_ADS1015 = 2,
  ADS1X1X_DEVICE_ADS1113 = 3,
  ADS1X1X_DEVICE_ADS1114 = 4,
  ADS1X1X_DEVICE_ADS1115 = 5
} ADS1x1x_Device_t;

/**
 * @brief  Mux Channel
 */
typedef enum ADS1x1x_Mux_e
{
  ADS1X1X_MUX_DIFF_01   = 0, // Differential P = AIN0, N = AIN1 (default)
  ADS1X1X_MUX_DIFF_03   = 1, // Differential P = AIN0, N = AIN3
  ADS1X1X_MUX_DIFF_13   = 2, // Differential P = AIN1, N = AIN3
  ADS1X1X_MUX_DIFF_23   = 3, // Differential P = AIN2, N = AIN3
  ADS1X1X_MUX_SINGLE_0  = 4, // Single-ended AIN0
  ADS1X1X_MUX_SINGLE_1  = 5, // Single-ended AIN1
  ADS1X1X_MUX_SINGLE_2  = 6, // Single-ended AIN2
  ADS1X1X_MUX_SINGLE_3  = 7, // Single-ended AIN3
} ADS1x1x_Mux_t;

/**
 * @brief  PGA Config
 */
typedef enum ADS1x1x_PGA_e
{
  ADS1X1X_PGA_6_144 = 0, 
  ADS1X1X_PGA_4_096 = 1,
  ADS1X1X_PGA_2_048 = 2,
  ADS1X1X_PGA_1_024 = 3,
  ADS1X1X_PGA_0_512 = 4,
  ADS1X1X_PGA_0_256 = 5
} ADS1x1x_PGA_t;

/**
 * @brief  Measuring mode
 */
typedef enum ADS1x1x_Mode_e
{
  ADS1X1X_MODE_CONTINUOUS = 0,
  ADS1X1X_MODE_SINGLESHOT = 1
} ADS1x1x_Mode_t;

/**
 * @brief  Measuring Speed in Periodic mode
 */
typedef enum ADS1x1x_Rate_e
{
  ADS1X1X_RATE_8SPS   = 0,
  ADS1X1X_RATE_16SPS  = 1,
  ADS1X1X_RATE_32SPS  = 2,
  ADS1X1X_RATE_64SPS  = 3,
  ADS1X1X_RATE_128SPS = 4,
  ADS1X1X_RATE_250SPS = 5,
  ADS1X1X_RATE_475SPS = 6,
  ADS1X1X_RATE_860SPS = 7
} ADS1x1x_Rate_t;



/**
 * @brief  Sample
 */
typedef struct ADS1x1x_Sample_s
{
  int16_t Raw;
  float Voltage;
} ADS1x1x_Sample_t;


/**
 * @brief  Function type for Initialize/Deinitialize the platform dependent layer.
 * @retval 
 *         -  0: The operation was successful.
 *         - -1: The operation failed. 
 */
typedef int8_t (*ADS1x1x_PlatformInitDeinit_t)(void);

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
typedef int8_t (*ADS1x1x_PlatformSendReceive_t)(uint8_t Address,
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
typedef struct ADS1x1x_Handler_s
{
  ADS1x1x_Device_t Device;
  ADS1x1x_PGA_t  PGA;
  ADS1x1x_Mode_t Mode;
  uint8_t AddressI2C;

  // Initializes platform dependent layer
  ADS1x1x_PlatformInitDeinit_t PlatformInit;
  // De-initializes platform dependent layer
  ADS1x1x_PlatformInitDeinit_t PlatformDeInit;
  // Send Data to the ADS1x1x
  ADS1x1x_PlatformSendReceive_t PlatformSend;
  // Receive Data from the ADS1x1x
  ADS1x1x_PlatformSendReceive_t PlatformReceive;
} ADS1x1x_Handler_t;



/**
 ==================================================================================
                               ##### Functions #####                               
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
             uint8_t Address);


/**
 * @brief  Deinitialize function
 * @param  Handler: Pointer to handler
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 */
ADS1x1x_Result_t
ADS1x1x_DeInit(ADS1x1x_Handler_t *Handler);


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
ADS1x1x_SetAddressI2C(ADS1x1x_Handler_t *Handler, uint8_t Address);


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
ADS1x1x_GeneralCallReset(ADS1x1x_Handler_t *Handler);


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
ADS1x1x_SetPGA(ADS1x1x_Handler_t *Handler, ADS1x1x_PGA_t PGA);


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
ADS1x1x_SetMode(ADS1x1x_Handler_t *Handler, ADS1x1x_Mode_t Mode);


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
ADS1x1x_SetRate(ADS1x1x_Handler_t *Handler, ADS1x1x_Rate_t Rate);


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
ADS1x1x_SetMux(ADS1x1x_Handler_t *Handler, ADS1x1x_Mux_t Mux);


/**
 * @brief  Start conversion (single shot)
 * @param  Handler: Pointer to handler
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 */
ADS1x1x_Result_t
ADS1x1x_StartConversion(ADS1x1x_Handler_t *Handler);


/**
 * @brief  Checks if the data is ready or not
 * @param  Handler: Pointer to handler
 * @retval ADS1110_Result_t
 *         - ADS1X1X_OK: Date is ready.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 *         - ADS1X1X_DATA_NOT_READY: Data is not ready.
 */
ADS1x1x_Result_t
ADS1x1x_CheckDataReady(ADS1x1x_Handler_t *Handler);


/**
 * @brief  Read sample
 * @param  Handler: Pointer to handler
 * @param  Sample: Pointer to sample buffer
 * @retval ADS1x1x_Result_t
 *         - ADS1X1X_OK: Operation was successful.
 *         - ADS1X1X_FAIL: Failed to send or receive data.
 */
ADS1x1x_Result_t
ADS1x1x_ReadSample(ADS1x1x_Handler_t *Handler, ADS1x1x_Sample_t *Sample);



#ifdef __cplusplus
}
#endif

#endif //! _ADS1X1X_H_
