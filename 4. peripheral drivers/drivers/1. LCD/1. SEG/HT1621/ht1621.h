#ifndef __HT1621_H__
#define __HT1621_H__


#include "stdint.h"

#define tCLK_WR             5   /* 5us */
#define tCLK_RD             7   /* 7us */

#define HT1621_ADDR_LEN     6
#define HT1621_DATA_LEN     4
#define HT1621_ADDR_SHIFT   (8 - HT1621_ADDR_LEN)

#define HT1621_CS_PORT      GPIOA
#define HT1621_CS_PIN       LL_GPIO_PIN_4

#define HT1621_DATA_PORT    GPIOA
#define HT1621_DATA_PIN     LL_GPIO_PIN_7

#define HT1621_WR_PORT      GPIOA
#define HT1621_WR_PIN       LL_GPIO_PIN_1

#define HT1621_RD_PORT      GPIOB
#define HT1621_RD_PIN       LL_GPIO_PIN_4

#define HT1621_COMMONS_2            ((uint8_t)0X00)      
#define HT1621_COMMONS_3            ((uint8_t)0X01)      
#define HT1621_COMMONS_4            ((uint8_t)0X02)      

#define HT1621_ID_WRITE             ((uint8_t)5 << 5)
#define HT1621_ID_READ              ((uint8_t)6 << 5)
#define HT1621_ID_COMMAND           ((uint8_t)4 << 5)

#define HT1621_COMMAND_SYS_DIS      ((uint8_t)0x00)
#define HT1621_COMMAND_SYS_EN       ((uint8_t)0x01)
#define HT1621_COMMAND_LCD_OFF      ((uint8_t)0x02)
#define HT1621_COMMAND_LCD_ON       ((uint8_t)0x03)
#define HT1621_COMMAND_XTAL_32      ((uint8_t)0x14)
#define HT1621_COMMAND_RC_256       ((uint8_t)0x18)
#define HT1621_COMMAND_EXT_256      ((uint8_t)0x1C)

#define HT1621_COMMAND_BIAS_1_2     ((uint8_t)(0x20 | (HT1621_COMMONS_4 << 2)))
#define HT1621_COMMAND_BIAS_1_3     ((uint8_t)(0x20 | (HT1621_COMMONS_4 << 2) | 1))

void ht1621_init(void);
void ht1621_write(uint8_t addr, uint8_t data);
void ht1621_read(uint8_t addr, uint8_t *data);
void ht1621_read_modify_write(uint8_t addr, uint8_t data, uint8_t mask);
void ht1621_display_all_seg(void);
void ht1621_clear_all_seg(void);

#endif