#ifndef __BSP_USART_H__
#define __BSP_USART_H__

/************************************************ INCLUDE ************************************************/

#include "stm32f407xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_bus.h"

/************************************************* 宏定义 *************************************************/

#define USART1_RS485_ENABLE             0       // 1：使能串口485 0：关闭串口485
#define USART2_RS485_ENABLE             0       // 1：使能串口485 0：关闭串口485

/******************************************* USART1 配置 *******************************************/

#define USART1_ENABLE_GPIO_CLOCK()      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA)      // 使能GPIO时钟
#define USART1_ENABLE_USART_CLOCK()     LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1)     // 使能USART1时钟

// GPIO | USART 时钟使能
#define USART1_ENABLE_CLOCK()     \
  do {                            \
    USART1_ENABLE_GPIO_CLOCK();   \
    USART1_ENABLE_USART_CLOCK();  \
  } while(0)

#define USART1_RX_GPIO_PORT             GPIOA                // USART1 RX GPIO端口
#define USART1_RX_GPIO_PIN              LL_GPIO_PIN_10       // USART1 RX PIN
#define USART1_RX_GPIO_AF               LL_GPIO_AF_7         // USART1 RX GPIO复用选择

#define USART1_TX_GPIO_PORT             GPIOA                // USART1 TX GPIO端口
#define USART1_TX_GPIO_PIN              LL_GPIO_PIN_9        // USART1 TX PIN
#define USART1_TX_GPIO_AF               LL_GPIO_AF_7         // USART1 RX GPIO复用选择

#define USART1_IRQN                     USART1_IRQn


/****************************************** USART2 GPIO/DMA 配置 ******************************************/

#define USART2_ENABLE_GPIO_CLOCK()      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA)      // 使能GPIO时钟
#define USART2_ENABLE_USART_CLOCK()     LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2)     // 使能USART2时钟
#define USART2_ENABLE_RS485_CLOCK()     LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG)      // 使能RS485 RE引脚时钟

#define USART2_ENABLE_CLOCK()     \
  do {                            \
    USART2_ENABLE_GPIO_CLOCK();   \
    USART2_ENABLE_RS485_CLOCK();  \
    USART2_ENABLE_USART_CLOCK();  \
  } while(0)

#define USART2_RS485_RE_PORT            GPIOG                // USART2 RS485 RE引脚端口
#define USART2_RS485_RE_PIN             LL_GPIO_PIN_8        // USART2 RS485 RE引脚PIN

#define USART2_RX_GPIO_PORT             GPIOA                // USART2 RX GPIO端口
#define USART2_RX_GPIO_PIN              LL_GPIO_PIN_3        // USART2 RX PIN
#define USART2_RX_GPIO_AF               LL_GPIO_AF_7         // USART2 RX GPIO复用选择

#define USART2_TX_GPIO_PORT             GPIOA                // USART2 TX GPIO端口
#define USART2_TX_GPIO_PIN              LL_GPIO_PIN_2        // USART2 TX PIN
#define USART2_TX_GPIO_AF               LL_GPIO_AF_7         // USART2 TX GPIO复用选择

#define USART2_IRQN                     USART2_IRQn

#define USART2_RS485_TX()               LL_GPIO_SetOutputPin(USART2_RS485_RE_PORT, USART2_RS485_RE_PIN)
#define USART2_RS485_RX()               LL_GPIO_ResetOutputPin(USART2_RS485_RE_PORT, USART2_RS485_RE_PIN)


/************************************************* 外部接口 *************************************************/

void bsp_usart1_init(void);
void bsp_usart2_init(void);

#endif
