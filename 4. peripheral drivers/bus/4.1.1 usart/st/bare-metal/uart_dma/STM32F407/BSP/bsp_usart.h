#ifndef __BSP_USART_H__
#define __BSP_USART_H__

/************************************************ INCLUDE ************************************************/

#include "stm32f407xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_bus.h"

/************************************************* 辅助宏 *************************************************/

// 清除DMA中断标志位 辅助宏
#define __DMA_CLEAR_IT_FLAG(stream_id, it_flag, dma)         LL_DMA_ClearFlag_##it_flag##stream_id(dma)
#define DMA_CLEAR_IT_FLAG(stream_id, it_flag, dma)           __DMA_CLEAR_IT_FLAG(stream_id, it_flag, dma)

/****************************************** USART1 GPIO/DMA 配置 ******************************************/

#define USART1_ENABLE_GPIO_CLOCK()      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA)      // 使能GPIO时钟
#define USART1_ENABLE_DMA_CLOCK()       LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2)       // 使能DMA时钟
#define USART1_ENABLE_USART_CLOCK()     LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1)     // 使能USART1时钟

// GPIO | DMA | USART 时钟使能
#define USART1_ENABLE_CLOCK()     \
  do {                            \
    USART1_ENABLE_GPIO_CLOCK();   \
    USART1_ENABLE_DMA_CLOCK();    \
    USART1_ENABLE_USART_CLOCK();  \
  } while(0)

#define USART1_GPIO_PORT                GPIOA                // USART1 GPIO端口
#define USART1_GPIO_AF                  LL_GPIO_AF_7         // USART1 GPIO复用选择
#define USART1_IRQN                     USART1_IRQn

#define USART1_RX_GPIO_PIN              LL_GPIO_PIN_10       // RX PIN
#define USART1_RX_DMA                   DMA2                 // RX DMA
#define USART1_RX_DMA_STREAM_ID         2                    // RX DMA数据流ID
#define USART1_RX_DMA_STREAM            LL_DMA_STREAM_2      // RX DMA数据流
#define USART1_RX_DMA_CHANNAL           LL_DMA_CHANNEL_4     // RX DMA通道
#define USART1_RX_DMA_IRQN              DMA2_Stream2_IRQn    // RX DMA数据流中断号

#define USART1_TX_GPIO_PIN              LL_GPIO_PIN_9       // TX PIN
#define USART1_TX_DMA                   DMA2                 // TX DMA
#define USART1_TX_DMA_STREAM_ID         7                    // TX DMA数据流ID
#define USART1_TX_DMA_STREAM            LL_DMA_STREAM_7      // TX DMA数据流
#define USART1_TX_DMA_CHANNAL           LL_DMA_CHANNEL_4     // TX DMA通道
#define USART1_TX_DMA_IRQN              DMA2_Stream7_IRQn    // TX DMA数据流中断号

/****************************************** USART2 GPIO/DMA 配置 ******************************************/

#define USART2_ENABLE_GPIO_CLOCK()      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA)      // 使能GPIO时钟
#define USART2_ENABLE_DMA_CLOCK()       LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1)       // 使能DMA时钟
#define USART2_ENABLE_USART_CLOCK()     LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2)     // 使能USART2时钟

#define USART2_ENABLE_CLOCK()     \
  do {                            \
    USART2_ENABLE_GPIO_CLOCK();   \
    USART2_ENABLE_DMA_CLOCK();    \
    USART2_ENABLE_USART_CLOCK();  \
  } while(0)

#define USART2_GPIO_PORT                GPIOA                // USART2 GPIO端口
#define USART2_GPIO_AF                  LL_GPIO_AF_7         // USART2 GPIO复用选择
#define USART2_IRQN                     USART2_IRQn

#define USART2_RX_GPIO_PIN              LL_GPIO_PIN_3        // RX PIN
#define USART2_RX_DMA                   DMA1                 // RX DMA
#define USART2_RX_DMA_STREAM_ID         5                    // RX DMA数据流ID
#define USART2_RX_DMA_STREAM            LL_DMA_STREAM_5      // RX DMA数据流
#define USART2_RX_DMA_CHANNAL           LL_DMA_CHANNEL_4     // RX DMA通道
#define USART2_RX_DMA_IRQN              DMA1_Stream5_IRQn    // RX DMA数据流中断号

#define USART2_TX_GPIO_PIN              LL_GPIO_PIN_2        // TX PIN
#define USART2_TX_DMA                   DMA1                 // TX DMA
#define USART2_TX_DMA_STREAM_ID         6                    // TX DMA数据流ID
#define USART2_TX_DMA_STREAM            LL_DMA_STREAM_6      // TX DMA数据流
#define USART2_TX_DMA_CHANNAL           LL_DMA_CHANNEL_4     // TX DMA通道
#define USART2_TX_DMA_IRQN              DMA1_Stream6_IRQn    // TX DMA数据流中断号

/************************************************* 外部接口 *************************************************/

void bsp_usart1_init(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size);
void bsp_usart1_dmatx_config(uint8_t *dmatx_buf_addr, uint32_t data_length);
uint16_t bsp_usart1_dmarx_buf_remain_size(void);

void bsp_usart2_init(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size);
void bsp_usart2_dmatx_config(uint8_t *dmatx_buf_addr, uint32_t data_length);
uint16_t bsp_usart2_dmarx_buf_remain_size(void);

#endif