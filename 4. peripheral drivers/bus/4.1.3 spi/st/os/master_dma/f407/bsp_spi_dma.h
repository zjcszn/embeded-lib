#ifndef __BSP_SPI_MASTER_DMA_H__
#define __BSP_SPI_MASTER_DMA_H__

#include "stm32f407xx.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_dma.h"
#include <stdint.h>

#ifndef __IO
#define __IO  volatile
#endif

#define SPI_INTERNAL_NSS              0

#define SPI1_CS_PIN                   LL_GPIO_PIN_14       // W25Q128 片选信号
#define SPI1_CS_GPIO                  GPIOB

#define SPI1_CLK_PIN                  LL_GPIO_PIN_3        // W25Q128 CLK信号
#define SPI1_MISO_PIN                 LL_GPIO_PIN_4        // W25Q128 MISO信号
#define SPI1_MOSI_PIN                 LL_GPIO_PIN_5        // W25Q128 MOSI信号
#define SPI1_GPIO                     GPIOB                // SPI1 GPIO端口
#define SPI1_GPIO_AF                  LL_GPIO_AF_5         // SPI1 GPIO复用选择
#define SPI1_IRQN                     SPI1_IRQn

#define SPI1_RX_DMA                   DMA2                 // RX DMA
#define SPI1_RX_DMA_STREAM_ID         0                    // RX DMA数据流ID
#define SPI1_RX_DMA_STREAM            LL_DMA_STREAM_0      // RX DMA数据流
#define SPI1_RX_DMA_CHANNAL           LL_DMA_CHANNEL_3     // RX DMA通道
#define SPI1_RX_DMA_IRQN              DMA2_Stream0_IRQn    // RX DMA数据流中断号

#define SPI1_TX_DMA                   DMA2                 // TX DMA
#define SPI1_TX_DMA_STREAM_ID         5                    // TX DMA数据流ID
#define SPI1_TX_DMA_STREAM            LL_DMA_STREAM_5      // TX DMA数据流
#define SPI1_TX_DMA_CHANNAL           LL_DMA_CHANNEL_3     // TX DMA通道
#define SPI1_TX_DMA_IRQN              DMA2_Stream5_IRQn    // TX DMA数据流中断号

#define W25Q128_CS_L                  LL_GPIO_ResetOutputPin(SPI1_CS_GPIO, SPI1_CS_PIN)     // 使能片选信号
#define W25Q128_CS_H                  LL_GPIO_SetOutputPin(SPI1_CS_GPIO, SPI1_CS_PIN)       // 失能片选信号

#define SPI_CS_ENABLE()               W25Q128_CS_L
#define SPI_CS_DISABLE()              W25Q128_CS_H

enum ENUM_SPI_State {
  SPI_DMA_TX1,
  SPI_DMA_TX2,
  SPI_DMA_RX,
  SPI_DMA_TC,
  SPI_DMA_ERR,
};

enum ENUM_SPI_Mode {
  SPI_WRITE_THEN_STOP,
  SPI_WRITE_THEN_READ,
  SPI_WRITE_THEN_WRITE,
};

enum ENUM_SPI_Error {
  SPI_OK,
  SPI_ERROR,
};

typedef struct {
  uint8_t mode;             // 传输模式：WR & RD | WR & STOP | WR & WR
  __IO uint8_t state;       // SPI传输状态标志
  uint16_t txbuf_size;      // SPI发送缓冲区大小
  uint16_t rxbuf_size;      // SPI接收缓冲区大小（可复用为次发送缓冲区大小）
  uint8_t *txbuf_addr;      // SPI发送缓冲区地址
  uint8_t *rxbuf_addr;      // SPI接收缓冲区地址（可复用为次发送缓冲区地址）
} SPI_Device_T;

void spi_master_init(void);

int spi_master_write_then_stop_dma(uint8_t *txbuf_addr, uint16_t txbuf_size);
int spi_master_write_then_read_dma(uint8_t *txbuf_addr, uint16_t txbuf_size, uint8_t *rxbuf_addr, uint16_t rxbuf_size);
int spi_master_write_then_write_dma(uint8_t *txbuf1_addr, uint16_t txbuf1_size, uint8_t *txbuf2_addr, uint16_t txbuf2_size);

#endif