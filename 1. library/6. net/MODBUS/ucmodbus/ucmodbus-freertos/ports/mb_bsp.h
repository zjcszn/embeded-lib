/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                    Copyright 2003-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            uC/Modbus
*
*                                      Board Support Package
*                                         ST STM32F407ZGT6
*
* Filename : mb_bsp.h
* Version  : V2.14.00
*********************************************************************************************************
*/

#ifndef  __MB_BSP_H__
#define  __MB_BSP_H__


#include    "stm32f407xx.h"
#include    "stm32f4xx_ll_bus.h"
#include    "stm32f4xx_ll_usart.h"
#include    "stm32f4xx_ll_rcc.h"
#include    "stm32f4xx_ll_tim.h"
#include    "stm32f4xx_ll_gpio.h"
#include    "stm32f4xx_ll_dma.h"

/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/
                                                                /* ------------------ UART PORT NUMBERS DEFINES ------------ */
#define  MB_BSP_UART_00                        1
#define  MB_BSP_UART_01                        2
#define  MB_BSP_UART_02                        3

                                                                /* ------------------- UART COMMUNICATION MODE ------------- */
#define  MB_BSP_UART_RS232_MODE                1                
#define  MB_BSP_UART_RS485_MODE                2
                                                                /* ----------------- TIMER CONFIGURATION. RTU  MODE -------- */
#define  MB_BSP_TMR_0                          1

#define  MB_BSP_DMA_TIMEOUT                    0xFF

/*
*********************************************************************************************************
*                                      USART DMA CHANNEL DEFINES
*********************************************************************************************************
*/

// 清除DMA中断标志位 辅助宏
#define __DMA_CLEAR_IT_FLAG(stream_id, it, dma)         LL_DMA_ClearFlag_##it##stream_id(dma)
#define DMA_CLEAR_IT_FLAG(stream_id, it, dma)           __DMA_CLEAR_IT_FLAG(stream_id, it, dma)

#define __DMA_IS_ACTIVE_IT_FLAG(stream_id, it, dma)     LL_DMA_IsActiveFlag_##it##stream_id(dma)
#define DMA_IS_ACTIVE_IT_FLAG(stream_id, it, dma)       __DMA_IS_ACTIVE_IT_FLAG(stream_id, it, dma)


#ifdef  MB_BSP_UART_00

#define USART1_ENABLE_GPIO_CLOCK()      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA)      // 使能GPIO时钟
#define USART1_ENABLE_DMA_CLOCK()       LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2)       // 使能DMA时钟
#define USART1_ENABLE_USART_CLOCK()     LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1)     // 使能USART1时钟

#define USART1_GPIO_PORT                GPIOA                // USART1 GPIO端口
#define USART1_GPIO_AF                  LL_GPIO_AF_7         // USART1 GPIO复用选择
#define USART1_IRQN                     USART1_IRQn

#define USART1_RX_GPIO_PIN              LL_GPIO_PIN_10       // RX PIN
#define USART1_RX_DMA                   DMA2                 // RX DMA
#define USART1_RX_DMA_STREAM_ID         2                    // RX DMA数据流ID
#define USART1_RX_DMA_STREAM            LL_DMA_STREAM_2      // RX DMA数据流
#define USART1_RX_DMA_CHANNEL           LL_DMA_CHANNEL_4     // RX DMA通道
#define USART1_RX_DMA_IRQN              DMA2_Stream2_IRQn    // RX DMA数据流中断号

#define USART1_TX_GPIO_PIN              LL_GPIO_PIN_9       // TX PIN
#define USART1_TX_DMA                   DMA2                 // TX DMA
#define USART1_TX_DMA_STREAM_ID         7                    // TX DMA数据流ID
#define USART1_TX_DMA_STREAM            LL_DMA_STREAM_7      // TX DMA数据流
#define USART1_TX_DMA_CHANNEL           LL_DMA_CHANNEL_4     // TX DMA通道
#define USART1_TX_DMA_IRQN              DMA2_Stream7_IRQn    // TX DMA数据流中断号

#endif

#ifdef  MB_BSP_UART_01

#define USART2_ENABLE_GPIO_CLOCK()      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA)      // 使能GPIO时钟
#define USART2_ENABLE_DMA_CLOCK()       LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1)       // 使能DMA时钟
#define USART2_ENABLE_USART_CLOCK()     LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2)     // 使能USART2时钟

#define USART2_GPIO_PORT                GPIOA                // USART2 GPIO端口
#define USART2_GPIO_AF                  LL_GPIO_AF_7         // USART2 GPIO复用选择
#define USART2_IRQN                     USART2_IRQn

#define USART2_RX_GPIO_PIN              LL_GPIO_PIN_3        // RX PIN
#define USART2_RX_DMA                   DMA1                 // RX DMA
#define USART2_RX_DMA_STREAM_ID         5                    // RX DMA数据流ID
#define USART2_RX_DMA_STREAM            LL_DMA_STREAM_5      // RX DMA数据流
#define USART2_RX_DMA_CHANNEL           LL_DMA_CHANNEL_4     // RX DMA通道
#define USART2_RX_DMA_IRQN              DMA1_Stream5_IRQn    // RX DMA数据流中断号

#define USART2_TX_GPIO_PIN              LL_GPIO_PIN_2        // TX PIN
#define USART2_TX_DMA                   DMA1                 // TX DMA
#define USART2_TX_DMA_STREAM_ID         6                    // TX DMA数据流ID
#define USART2_TX_DMA_STREAM            LL_DMA_STREAM_6      // TX DMA数据流
#define USART2_TX_DMA_CHANNEL           LL_DMA_CHANNEL_4     // TX DMA通道
#define USART2_TX_DMA_IRQN              DMA1_Stream6_IRQn    // TX DMA数据流中断号

#endif

#ifdef  MB_BSP_UART_02

#define USART3_ENABLE_GPIO_CLOCK()      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB)      // 使能GPIO时钟
#define USART3_ENABLE_DMA_CLOCK()       LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1)       // 使能DMA时钟
#define USART3_ENABLE_USART_CLOCK()     LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3)     // 使能USART3时钟

#define USART3_GPIO_PORT                GPIOB                // USART2 GPIO端口
#define USART3_GPIO_AF                  LL_GPIO_AF_7         // USART2 GPIO复用选择
#define USART3_IRQN                     USART3_IRQn

#define USART3_RX_GPIO_PIN              LL_GPIO_PIN_11       // RX PIN
#define USART3_RX_DMA                   DMA1                 // RX DMA
#define USART3_RX_DMA_STREAM_ID         1                    // RX DMA数据流ID
#define USART3_RX_DMA_STREAM            LL_DMA_STREAM_1      // RX DMA数据流
#define USART3_RX_DMA_CHANNEL           LL_DMA_CHANNEL_4     // RX DMA通道
#define USART3_RX_DMA_IRQN              DMA1_Stream1_IRQn    // RX DMA数据流中断号

#define USART3_TX_GPIO_PIN              LL_GPIO_PIN_10       // TX PIN
#define USART3_TX_DMA                   DMA1                 // TX DMA
#define USART3_TX_DMA_STREAM_ID         3                    // TX DMA数据流ID
#define USART3_TX_DMA_STREAM            LL_DMA_STREAM_3      // TX DMA数据流
#define USART3_TX_DMA_CHANNEL           LL_DMA_CHANNEL_4     // TX DMA通道
#define USART3_TX_DMA_IRQN              DMA1_Stream3_IRQn    // TX DMA数据流中断号

#endif
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/
#if 0                                                                /*-------------- UART MODE CONFIGURATION ERRORS ------------ */
#ifndef MB_BSP_CFG_UART_00_MODE
    #error  "MB_BSP_CFG_UART_00_MODE           not #define'd in 'app_cfg.h'          "
    #error  "                                  [MUST be  MB_BSP_UART_RS232_MODE   ]  "
    #error  "                                  [     ||  MB_BSP_UART_RS485_MODE   ]  "

#elif   (MB_BSP_CFG_UART_00_MODE != MB_BSP_UART_RS232_MODE  ) && \
        (MB_BSP_CFG_UART_00_MODE != MB_BSP_UART_RS485_MODE  ) 
    #error  "MB_BSP_CFG_UART_00_MODE           illegally #define'd in 'app_cfg.h'    "
    #error  "                                  [MUST be  MB_BSP_UART_RS232_MODE   ]  "
    #error  "                                  [     ||  MB_BSP_UART_RS485_MODE   ]  "
#endif

#ifndef MB_BSP_CFG_UART_01_MODE
    #error  "MB_BSP_CFG_UART_01_MODE           not #define'd in 'app_cfg.h'          "
    #error  "                                  [MUST be  MB_BSP_UART_RS232_MODE   ]  "
    #error  "                                  [     ||  MB_BSP_UART_RS485_MODE   ]  "

#elif   (MB_BSP_CFG_UART_01_MODE != MB_BSP_UART_RS232_MODE  ) && \
        (MB_BSP_CFG_UART_01_MODE != MB_BSP_UART_RS485_MODE  ) 
    #error  "MB_BSP_CFG_UART_01_MODE           illegally #define'd in 'app_cfg.h'    
    #error  "                                  [MUST be  MB_BSP_UART_RS232_MODE   ]  "
    #error  "                                  [     ||  MB_BSP_UART_RS485_MODE   ]  "
#endif

#ifndef MB_BSP_CFG_UART_02_MODE
    #error  "MB_BSP_CFG_UART_02_MODE           not #define'd in 'app_cfg.h'          "
    #error  "                                  [MUST be  MB_BSP_UART_RS232_MODE   ]  "
    #error  "                                  [     ||  MB_BSP_UART_RS485_MODE   ]  "

#elif   (MB_BSP_CFG_UART_02_MODE != MB_BSP_UART_RS232_MODE  ) && \
        (MB_BSP_CFG_UART_02_MODE != MB_BSP_UART_RS485_MODE  ) 
    #error  "MB_BSP_CFG_UART_02_MODE           illegally #define'd in 'app_cfg.h'    "
    #error  "                                  [MUST be  MB_BSP_UART_RS232_MODE   ]  "
    #error  "                                  [     ||  MB_BSP_UART_RS485_MODE   ]  "
#endif

#ifndef MB_BSP_CFG_UART_03_MODE
    #error  "MB_BSP_CFG_UART_03_MODE           not #define'd in 'app_cfg.h'          "
    #error  "                                  [MUST be  MB_BSP_UART_RS232_MODE   ]  "
    #error  "                                  [     ||  MB_BSP_UART_RS485_MODE   ]  "

#elif   (MB_BSP_CFG_UART_03_MODE != MB_BSP_UART_RS232_MODE  ) && \
        (MB_BSP_CFG_UART_03_MODE != MB_BSP_UART_RS485_MODE  ) 
    #error  "MB_BSP_CFG_UART_03_MODE           illegally #define'd in 'app_cfg.h'    "
    #error  "                                  [MUST be  MB_BSP_UART_RS232_MODE   ]  "
    #error  "                                  [     ||  MB_BSP_UART_RS485_MODE   ]  "
#endif

                                                                /*-------------- TIMER SEL CONFIGURATION  ------------ */
#ifndef MS_BSP_CFG_TMR
    #error  "MS_BSP_CFG_TMR           not #define'd in 'app_cfg.h'                  "
    #error  "                                  [MUST be  MB_BSP_TMR_0   ]           "

#elif   (MS_BSP_CFG_TMR != MB_BSP_TMR_0 ) 
    #error  "MS_BSP_CFG_TMR           illegally #define'd in 'app_cfg.h'          "
    #error  "                                  [MUST be  MB_BSP_TMR_0   ]         "
#endif
#endif


#endif
