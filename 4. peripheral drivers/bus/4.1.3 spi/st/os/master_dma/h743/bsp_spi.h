//
// Created by zjcszn on 2024/2/4.
//

#ifndef _BSP_SPI_H_
#define _BSP_SPI_H_

#include <stdint.h>

#define SPI_TEST

#define SPI1_TX_DMA             DMA1
#define SPI1_TX_DMA_STREAM      LL_DMA_STREAM_2
#define SPI1_TX_DMA_STREAM_ID   2
#define SPI1_TX_DMA_IRQN        DMA1_Stream2_IRQn
#define SPI1_TX_DMA_IRQHandler  DMA1_Stream2_IRQHandler

#define SPI1_RX_DMA             DMA1
#define SPI1_RX_DMA_STREAM      LL_DMA_STREAM_3
#define SPI1_RX_DMA_STREAM_ID   3
#define SPI1_RX_DMA_IRQN        DMA1_Stream3_IRQn
#define SPI1_RX_DMA_IRQHandler  DMA1_Stream3_IRQHandler

enum ENUM_SPI_State {
    SPI_STATE_IDLE,
    SPI_STATE_POLLING_START,
    SPI_STATE_DMA_START,
    SPI_STATE_DMA_TC,
    SPI_STATE_DMA_ERR,
    SPI_STATE_MOD_ERR,
    SPI_STATE_TIMEOUT,
};

enum ENUM_SPI_Error {
    SPI_OK,
    SPI_ERROR,
};

#ifdef SPI_TEST
void spi_master_test(void);
#endif

int spi_master_init(void);
int spi_master_write(uint8_t *txbuf_addr, uint32_t len);
int spi_master_read(uint8_t *rxbuf_addr, uint32_t len);

#endif //_BSP_SPI_H_
