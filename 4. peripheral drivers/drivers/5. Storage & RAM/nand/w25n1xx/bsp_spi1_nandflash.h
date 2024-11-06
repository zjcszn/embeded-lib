#ifndef __BSP_SPI_NANDFLASH_H__
#define __BSP_SPI_NANDFLASH_H__

#include "stdint.h"

#define NANDFLASH_SPI               SPI1

#define NANDFLASH_SCK_PORT          GPIOA
#define NANDFLASH_SCK_PIN           LL_GPIO_PIN_1

#define NANDFLASH_MISO_PORT         GPIOA
#define NANDFLASH_MISO_PIN          LL_GPIO_PIN_6

#define NANDFLASH_MOSI_PORT         GPIOA
#define NANDFLASH_MOSI_PIN          LL_GPIO_PIN_7

#define NANDFLASH_CS_PORT           GPIOA
#define NANDFLASH_CS_PIN            LL_GPIO_PIN_4

#if 1
#define NANDFLASH_PORT              NANDFLASH_SCK_PORT
#else
#error "NANDFLASH SPI PORT ERROR"
#endif


static inline void NAND_FLASH_CS_H(void) {
    LL_GPIO_SetOutputPin(NANDFLASH_CS_PORT, NANDFLASH_CS_PIN);
}

static inline void NAND_FLASH_CS_L(void) {
    LL_GPIO_ResetOutputPin(NANDFLASH_CS_PORT, NANDFLASH_CS_PIN); 
}    

void bsp_spi_nandflash_init(void);
uint8_t bsp_spi_nandflash_read_write_byte(uint8_t c);
bool bsp_spi_nandflash_read_buffer(uint8_t * buf, uint32_t len);
bool bsp_spi_nandflash_write_buffer(uint8_t * buf, uint32_t len);


#endif