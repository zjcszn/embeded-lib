#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_dma.h"
#include "FreeRTOS.h"


#define DMA_CLEAR_IT_FLAG_(sid, flag, dma)   LL_DMA_ClearFlag_ ## flag ## sid(dma)
#define DMA_CLEAR_IT_FLAG(sid, flag, dma)    DMA_CLEAR_IT_FLAG_(sid, flag, dma)

#define UART_ENABLE_GPIO_CLOCK()             LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA) 
#define UART_ENABLE_DMA_CLOCK()              LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2) 
#define UART_ENABLE_UART_CLOCK()             LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1) 


#define UART_ENABLE_CLOCK()           \
  do {                                \
    UART_ENABLE_GPIO_CLOCK();         \
    UART_ENABLE_DMA_CLOCK();          \
    UART_ENABLE_UART_CLOCK();         \
  } while(0)

#define UART                          USART1
#define UART_BAUD                     921600
#define UART_DATAWIDTH                LL_USART_DATAWIDTH_8B
#define UART_PARITY                   LL_USART_PARITY_NONE
#define UART_STOP                     LL_USART_STOPBITS_1

#define UART_GPIO_PORT                GPIOA               
#define UART_GPIO_AF                  LL_GPIO_AF_7         
#define UART_IRQN                     USART1_IRQn

#define UART_RX_GPIO_PIN              LL_GPIO_PIN_10       
#define UART_RX_DMA                   DMA2                 
#define UART_RX_DMA_STREAM_ID         2                    
#define UART_RX_DMA_STREAM            LL_DMA_STREAM_2      
#define UART_RX_DMA_CHANNAL           LL_DMA_CHANNEL_4     
#define UART_RX_DMA_IRQN              DMA2_Stream2_IRQn    

#define UART_TX_GPIO_PIN              LL_GPIO_PIN_9        
#define UART_TX_DMA                   DMA2                 
#define UART_TX_DMA_STREAM_ID         7                    
#define UART_TX_DMA_STREAM            LL_DMA_STREAM_7      
#define UART_TX_DMA_CHANNAL           LL_DMA_CHANNEL_4     
#define UART_TX_DMA_IRQN              DMA2_Stream7_IRQn   

#define UART_NVIC_PRIOR               configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY

#define BSP_UART_ISR                  USART1_IRQHandler
#define BSP_UART_RX_DMA_ISR           DMA2_Stream2_IRQHandler
#define BSP_UART_TX_DMA_ISR           DMA2_Stream7_IRQHandler


extern void bsp_uart_init(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size);
extern void bsp_uart_dmatx_enable(uint8_t *dmatx_buf_addr, uint32_t data_length);
extern uint32_t bsp_uart_dmarx_buf_remain_size(void);



#endif