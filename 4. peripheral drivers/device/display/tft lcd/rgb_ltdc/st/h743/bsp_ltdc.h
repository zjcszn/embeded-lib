//
// Created by zjcszn on 2024/2/21.
//

#ifndef _BSP_LTDC_H_
#define _BSP_LTDC_H_

#include "stdint.h"

/* LCD timing */
#define HBP  88
#define HFP  40
#define VBP  32
#define VFP  13
#define HSW  48
#define VSW  3

/* LCD size */
#define LCD_WIDTH   800
#define LCD_HEIGHT  480

/* LCD frame buffer address */
#define LCD_FRAME_BUFFER         0xC0000000

#define LCD_BACKLIGHT_ON()       LL_GPIO_SetOutputPin(GPIOH, LL_GPIO_PIN_6)
#define LCD_BACKLIGHT_OFF()      LL_GPIO_ResetOutputPin(GPIOH, LL_GPIO_PIN_6)

int bsp_ltdc_init(void);
void bsp_ltdc_set_framebuf(uint32_t fbuf);
void bsp_ltdc_wait_reload(void);

#endif //_BSP_LTDC_H_
