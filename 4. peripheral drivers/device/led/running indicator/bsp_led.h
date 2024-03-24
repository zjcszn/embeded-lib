//
// Created by zjcszn on 2024/1/27.
//

#ifndef _BSP_LED_H_
#define _BSP_LED_H_

#include "stdint.h"

#define USER_LED_PORT   GPIOH
#define USER_LED_PIN    LL_GPIO_PIN_7
#define USER_LED_CLOCK_ENABLE() \
do {                            \
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);\
} while(0)

void bsp_led_freq_set(uint32_t freq);

void bsp_led_init(void);

#endif //_BSP_LED_H_
