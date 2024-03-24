#ifndef	 __W25P80_H
#define	__W25P80_H
#include "type.h"
#include "api.h"

#define FLASH_CS_PORT 				GPIOC
#define FLASH_CS_PIN 				GPIO_Pin_1

#define FLASH_CLK_PORT 			GPIOB
#define FLASH_CLK_PIN 				GPIO_Pin_13
#define FLASH_CLK_PIN_SOURCE 		GPIO_PinSource13

#define FLASH_MISO_PORT 			GPIOC
#define FLASH_MISO_PIN 				GPIO_Pin_2
#define FLASH_MISO_PIN_SOURCE  	GPIO_PinSource2

#define FLASH_MOSI_PORT 			GPIOC
#define FLASH_MOSI_PIN 				GPIO_Pin_3
#define FLASH_MOSI_PIN_SOURCE  	GPIO_PinSource3

#define FLASH_WP_PORT 				GPIOC
#define FLASH_WP_PIN 				GPIO_Pin_0



#define FLASH_CS_ENABLE	 	GPIO_CLR(FLASH_CS_PORT,FLASH_CS_PIN)
#define FLASH_CS_DISABLE	GPIO_SET(FLASH_CS_PORT,FLASH_CS_PIN)

#define FLASH_WP_ENABLE		GPIO_CLR(FLASH_WP_PORT,FLASH_WP_PIN)
#define FLASH_WP_DISABLE	GPIO_SET(FLASH_WP_PORT,FLASH_WP_PIN)


#define FLASH_ROOM_SIZE				(1024*1024)

#define FIRMWARE_BASE_ADDR			(512*1024)


#define ssize_t int

int  m25p80_init(void);
int m25p80_write(u32 to, size_t len,const u_char *buf);
int m25p80_read(u32 from, size_t len, u_char *buf);
int m25p80_erase(u32 addr, int len);
uint32_t m25p80_read_id(void);
int read_firmware(u32 offset, u32 len, u8 * pbuf);
int test_write_firmware_to_spi_flash(void);

#endif

