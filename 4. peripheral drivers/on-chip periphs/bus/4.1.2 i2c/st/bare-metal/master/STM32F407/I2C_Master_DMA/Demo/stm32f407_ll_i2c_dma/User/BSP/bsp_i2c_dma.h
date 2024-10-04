#ifndef __BSP_I2C_DMA_H__
#define __BSP_I2C_DMA_H__

#include "stm32f407xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_gpio.h"
#include "dwt_delay.h"

/*********************************************** 宏定义 **********************************************/

/**I2C1 GPIO Configuration
PB8   ------> I2C1_SCL
PB9   ------> I2C1_SDA
*/

#define I2C1_TIMEOUT                (SystemCoreClock * 10U / 1000U)
#define I2C1_CLK_DELAY              (10U)

#define I2C1_GPIO_AF                LL_GPIO_AF_4          // I2C GPIO复用功能选择
#define I2C1_SCL_PORT               GPIOB                 // I2C 时钟线GPIO端口 
#define I2C1_SCL_PIN                LL_GPIO_PIN_8         // I2C 时钟线GPIO引脚
#define I2C1_SDA_PORT               GPIOB                 // I2C 数据线GPIO端口
#define I2C1_SDA_PIN                LL_GPIO_PIN_9         // I2C 数据线GPIO引脚
#define I2C1_SCL_FEQ                400000U               // I2C 时钟频率
#define I2C1_OWN_ADDR               0U                    // I2C 本机地址（从设备模式）

#define I2C1_TX_DMA                 DMA1                  // I2C TX DMA
#define I2C1_TX_DMA_STREAM_ID       6                     // I2C TX DMA数据流ID
#define I2C1_TX_DMA_STREAM          LL_DMA_STREAM_6       // I2C TX DMA数据流
#define I2C1_TX_DMA_CHANNAL         LL_DMA_CHANNEL_1      // I2C TX DMA通道
#define I2C1_TX_DMA_IRQN            DMA1_Stream6_IRQn     // I2C TX DMA中断号

#define I2C1_RX_DMA                 DMA1                  // I2C RX DMA
#define I2C1_RX_DMA_STREAM_ID       5                     // I2C RX DMA数据流ID
#define I2C1_RX_DMA_STREAM          LL_DMA_STREAM_5       // I2C RX DMA数据流
#define I2C1_RX_DMA_CHANNAL         LL_DMA_CHANNEL_1      // I2C RX DMA通道
#define I2C1_RX_DMA_IRQN            DMA1_Stream5_IRQn     // I2C RX DMA中断号

#define i2c_delay_us(us)            dwt_delay_us(us)
#define i2c_delay_ms(ms)            dwt_delay_ms(ms)


// 清除DMA中断标志位 辅助宏
#define __DMA_CLEAR_IT_FLAG(stream_id, it, dma)         LL_DMA_ClearFlag_##it##stream_id(dma)
#define DMA_CLEAR_IT_FLAG(stream_id, it, dma)           __DMA_CLEAR_IT_FLAG(stream_id, it, dma)



/********************************************** 函数声明 *********************************************/


void bsp_i2c_init(void);
void i2c_dmarx_config(void);
void i2c_dmatx_config(void);
int  i2c_bus_check(void);
void i2c_bus_reset(void);

#endif