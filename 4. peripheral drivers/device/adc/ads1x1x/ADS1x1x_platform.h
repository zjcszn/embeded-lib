/**
 **********************************************************************************
 * @file   ADS1x1x_platform.h
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

/* Define to prevent recursive inclusion ----------------------------------------*/
#ifndef	_ADS1X1X_PLATFORM_H_
#define _ADS1X1X_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ---------------------------------------------------------------------*/
#include "ADS1x1x.h"


/* Functionality Options --------------------------------------------------------*/
/**
 * @brief  Specify the target platform
 * @note   Uncomment the line below according to the target platform
 */ 
// #define ADS1X1X_PLATFORM_STM32_HAL
// #define ADS1X1X_PLATFORM_ESP32_IDF
// #define ADS1X1X_PLATFORM_AVR


#if defined(ADS1X1X_PLATFORM_STM32_HAL)
#define ADS1X1X_HI2C      hi2c2
#elif defined(ADS1X1X_PLATFORM_ESP32_IDF)
#define ADS1X1X_I2C_NUM   I2C_NUM_0
#define ADS1X1X_I2C_RATE  100000
#define ADS1X1X_SCL_GPIO  GPIO_NUM_27
#define ADS1X1X_SDA_GPIO  GPIO_NUM_33
#elif defined(ADS1X1X_PLATFORM_AVR)
#define ADS1X1X_CPU_CLK   8000000UL
#define ADS1X1X_I2C_RATE  100000
#endif



/**
 ==================================================================================
                             ##### Functions #####                                 
 ==================================================================================
 */

void
ADS1x1x_Platform_Init(ADS1x1x_Handler_t *Handler);


#ifdef __cplusplus
}
#endif


#endif //! _ADS1X1X_PLATFORM_H_
