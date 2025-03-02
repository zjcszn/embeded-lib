#ifndef BSP_SPI_TM7707_H_
#define BSP_SPI_TM7707_H_

#include "stdint.h"

#define SPI_TM7707_CLK_PORT    GPIOC
#define SPI_TM7707_CLK_PIN     LL_GPIO_PIN_10

#define SPI_TM7707_MOSI_PORT   GPIOB
#define SPI_TM7707_MOSI_PIN    LL_GPIO_PIN_5

#define SPI_TM7707_MISO_PORT   GPIOB
#define SPI_TM7707_MISO_PIN    LL_GPIO_PIN_4

void bsp_spi_tm7707_init(void);
void bsp_spi_tm7707_write_then_stop(uint8_t *data, uint16_t size);
void bsp_spi_tm7707_write_then_read(uint8_t *tx_data, uint16_t tx_size, uint8_t *rx_data, uint16_t rx_size);


#endif /* BSP_SPI_TM7707_H_ */