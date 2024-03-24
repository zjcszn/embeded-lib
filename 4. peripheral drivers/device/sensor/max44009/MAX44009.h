/**
 **********************************************************************************
 * @file   MAX44009.h
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

/* Define to prevent recursive inclusion ----------------------------------------*/
#ifndef _MAX44009_H_
#define _MAX44009_H_

#ifdef __cplusplus
extern "C" {
#endif



/* Includes ---------------------------------------------------------------------*/
#include <stdint.h>


/* Exported Data Types ----------------------------------------------------------*/
/**
 * @brief  Handler data type
 * @note   User must initialize this this functions before using library:
 *         - PlatformInit
 *         - PlatformDeInit
 *         - PlatformSend
 *         - PlatformReceive
 *         - PlatformCRC
 *         - PlatformDelay
 * @note   If success the functions must return 0 
 */
typedef struct MAX44009_Handler_s
{
  uint8_t AddressI2C;

  // Initializes platform dependent part
  int8_t (*PlatformInit)(void);
  // De-initializes platform dependent part
  int8_t (*PlatformDeInit)(void);
  // Send Data to the slave with the address of Address. (0 <= Address <= 127)
  int8_t (*PlatformSend)(uint8_t Address, uint8_t *Data, uint8_t Len);
  // Receive Data from the slave with the address of Address. (0 <= Address <= 127)
  int8_t (*PlatformReceive)(uint8_t Address, uint8_t *Data, uint8_t Len);
} MAX44009_Handler_t;

typedef enum MAX44009_Result_e
{
  MAX44009_OK   = 0,
  MAX44009_FAIL = 1,
} MAX44009_Result_t;

typedef struct MAX44009_Sample_s
{
  uint8_t Exponent;
  uint8_t Mantissa;
  float   Lux;
} MAX44009_Sample_t;



/**
 ==================================================================================
                       ##### Measurement Functions #####                           
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
MAX44009_ReadSample(MAX44009_Handler_t *Handler, MAX44009_Sample_t *Sample);



/**
 ==================================================================================
                    ##### Control and Status Functions #####                       
 ==================================================================================
 */

/**
 * @brief  Initializes the Handler platform-independent part
 * @param  Handler: Pointer to handler
 * @retval MAX44009_Result_t
 *         - MAX44009_OK: Operation was successful.
 *         - MAX44009_FAIL: Failed to send or receive data.
 */
MAX44009_Result_t
MAX44009_Init(MAX44009_Handler_t *Handler);


/**
 * @brief  De-initializes the Handler
 * @param  Handler: Pointer to handler
 * @retval MAX44009_Result_t
 *         - MAX44009_OK: Operation was successful.
 */
MAX44009_Result_t
MAX44009_DeInit(MAX44009_Handler_t *Handler);


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
MAX44009_SetAddressI2C(MAX44009_Handler_t *Handler, uint8_t Address);



#ifdef __cplusplus
}
#endif

#endif  //! _MAX44009_H_
