#ifndef __BSP_SPI_MASTER_SOFT_H__
#define __BSP_SPI_MASTER_SOFT_H__

#include "stm32f407xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"

#ifndef __IO
#define __IO  volatile
#endif

#define SOFT_SPI_SCK_PIN            LL_GPIO_PIN_3
#define SOFT_SPI_SCK_PORT           GPIOB
#define SOFT_SPI_MISO_PIN           LL_GPIO_PIN_4
#define SOFT_SPI_MISO_PORT          GPIOB
#define SOFT_SPI_MOSI_PIN           LL_GPIO_PIN_5
#define SOFT_SPI_MOSI_PORT          GPIOB
#define SOFT_SPI_NSS_PIN            LL_GPIO_PIN_14
#define SOFT_SPI_NSS_PORT           GPIOB

#define SOFT_SPI_ENABLE_CLOCK()     LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB)

enum ENUM_SoftSPI_CPOL {
  SoftSPI_CPOL_L,
  SoftSPI_CPOL_H,
};

enum ENUM_SoftSPI_CPHA {
  SoftSPI_CPHA_1EDGE,
  SoftSPI_CPHA_2EDGE,
};

enum  ENUM_SodtSPI_Lock {
  SOFT_SPI_LOCKED,
  SOFT_SPI_UNLOCKED,
};

typedef struct {
  __IO uint8_t   cpol;
  __IO uint8_t   cpha;
  __IO uint8_t   lock;
  __IO uint8_t   error;   // not used
  __IO uint32_t  sck_1;
  __IO uint32_t  sck_2;
}SoftSPI_Device_T;


void soft_spi_init(uint8_t cpol, uint8_t cpha);
void soft_spi_write_then_stop(uint8_t *txbuf_addr, uint32_t txbuf_size);
void soft_spi_write_then_read(uint8_t *txbuf_addr, uint32_t txbuf_size, uint8_t *rxbuf_addr, uint32_t rxbuf_size);
void soft_spi_write_then_write(uint8_t *txbuf1_addr, uint32_t txbuf1_size, uint8_t *txbuf2_addr, uint32_t txbuf2_size);


#endif