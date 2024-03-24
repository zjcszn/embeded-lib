#ifndef __BSP_I2C_DMA_H__
#define __BSP_I2C_DMA_H__


/*********************************************** 宏定义 **********************************************/

/**I2C1 GPIO Configuration
PB8   ------> I2C1_SCL
PB9   ------> I2C1_SDA
*/

#include <stdint.h>
#define I2C1_TIMEOUT                2000

#define I2C1_GPIO_AF                LL_GPIO_AF_4          // I2C GPIO复用功能选择
#define I2C1_PORT                   GPIOB                 // I2C 时钟线GPIO端口
#define I2C1_SCL_PIN                LL_GPIO_PIN_8         // I2C 时钟线GPIO引脚
#define I2C1_SDA_PIN                LL_GPIO_PIN_7         // I2C 数据线GPIO引脚
#define I2C1_SCL_FEQ                400000U               // I2C 时钟频率
#define I2C1_OWN_ADDR               0U                    // I2C 本机地址（从设备模式）

#define I2C1_EVENT_IRQ_HANDLER      I2C1_EV_IRQHandler
#define I2C1_ERROR_IRQ_HANDLER      I2C1_ER_IRQHandler

#define I2C1_TX_DMA                 DMA1                  // I2C TX DMA
#define I2C1_TX_DMA_STREAM          LL_DMA_STREAM_4       // I2C TX DMA数据流
#define I2C1_TX_DMA_IRQN            DMA1_Stream4_IRQn     // I2C TX DMA中断号
#define I2C1_TX_DMA_IRQ_HANDLER     DMA1_Stream4_IRQHandler

#define I2C1_RX_DMA                 DMA1                  // I2C RX DMA
#define I2C1_RX_DMA_STREAM          LL_DMA_STREAM_5       // I2C RX DMA数据流
#define I2C1_RX_DMA_IRQN            DMA1_Stream5_IRQn     // I2C RX DMA中断号
#define I2C1_RX_DMA_IRQ_HANDLER     DMA1_Stream5_IRQHandler

#define I2C1_ALL_PIN                (I2C1_SCL_PIN | I2C1_SDA_PIN)
#define I2C1_ISR_MASK_BSY_OR_ERR    (I2C_ISR_BUSY | I2C_ISR_ARLO | I2C_ISR_BERR)
/********************************************** 函数声明 *********************************************/


void bsp_i2c_init(void);
int bsp_i2c_bus_check(void);
void bsp_i2c_dmatx_config(uint8_t *txbuf, uint16_t len);
void bsp_i2c_dmarx_config(uint8_t *rxbuf, uint16_t len);
#endif