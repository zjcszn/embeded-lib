#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#include "main.h"

#define LED_GPIO_PORT           GPIOF
#define LED_GPIO_PIN            LL_GPIO_PIN_10
#define LED_CLOCK_ENABLE()      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF)

#define LED_TASK_STK_SIZE       32

void    bsp_led_init(void);


#endif