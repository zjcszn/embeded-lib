#ifndef __BSP_SPI_MASTER_POLL_H__
#define __BSP_SPI_MASTER_POLL_H__


#include "stm32f407xx.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"

#define SPI1_CS_PIN     LL_GPIO_PIN_14          // W25Q128 片选信号
#define SPI1_CS_GPIO    GPIOB

#define SPI1_CLK_PIN    LL_GPIO_PIN_3           // W25Q128 CLK信号
#define SPI1_MISO_PIN   LL_GPIO_PIN_4           // W25Q128 MISO信号
#define SPI1_MOSI_PIN   LL_GPIO_PIN_5           // W25Q128 MOSI信号

#define SPI1_GPIO       GPIOB
#define SPI1_GPIO_AF    LL_GPIO_AF_5




#define SPI1_CS_ENABLED()                 LL_GPIO_ResetOutputPin(SPI1_CS_GPIO, SPI1_CS_PIN)
#define SPI1_CS_DISABLED()                LL_GPIO_SetOutputPin(SPI1_CS_GPIO, SPI1_CS_PIN)

void    bsp_spi1_init(void);
void    spi_master_write_poll(uint8_t *send_data, uint32_t length);
void    spi_master_read_poll(uint8_t *recv_data, uint32_t length);
void    spi_master_write_read_poll(uint8_t *send_data, uint32_t send_len, uint8_t *recv_data, uint32_t recv_len);
uint8_t spi_master_write_read_byte(uint8_t c);

#endif