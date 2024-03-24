/**
 **********************************************************************************
 * @file   MAX44009_platform.h
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

/* Define to prevent recursive inclusion ----------------------------------------*/
#ifndef _MAX44009_PLATFORM_H_
#define _MAX44009_PLATFORM_H_

#ifdef __cplusplus
extern "C"
{
#endif



/* Includes ---------------------------------------------------------------------*/
#include <stdint.h>
#include "MAX44009.h"


/* Functionality Options --------------------------------------------------------*/
/**
 * @brief  Specify the target platform
 * @note   Uncomment the line below according to the target platform
 */
// #define MAX44009_PLATFORM_AVR
#define MAX44009_PLATFORM_ESP32_IDF


#if defined(MAX44009_PLATFORM_AVR)
/**
 * @brief  Specify I2C options of AVR
 */
#define MAX44009_CPU_CLK      8000000UL

#elif defined(MAX44009_PLATFORM_ESP32_IDF)
/**
 * @brief  Specify I2C options of ESP32
 */
#define MAX44009_I2C_NUM      I2C_NUM_1
#define MAX44009_I2C_RATE     100000
#define MAX44009_SCL_GPIO     GPIO_NUM_22
#define MAX44009_SDA_GPIO     GPIO_NUM_21
#endif



/**
 ==================================================================================
                             ##### Functions #####                                 
 ==================================================================================
 */

MAX44009_Result_t
MAX44009_Platform_Init(MAX44009_Handler_t *Handler);



#ifdef __cplusplus
}
#endif

#endif  //! _MAX44009_PLATFORM_H_
