#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

#include "stm32f407xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "custom_button.h"


void bsp_key_init(void);
void bsp_key_callback(void);
void bsp_key_poll(void *args);

#endif