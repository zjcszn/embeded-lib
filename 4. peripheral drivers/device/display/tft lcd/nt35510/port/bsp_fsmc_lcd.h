#ifndef __BSP_FSMC_NT_35510__
#define __BSP_FSMC_NT_35510__


#include "stdint.h"
#include "stm32f407xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_fsmc.h"
#include "stm32f4xx_hal_sram.h"



#define NT35510_BL_PIN        LL_GPIO_PIN_15
#define NT35510_BL_PORT       GPIOB

#define NT35510_CS_PIN        LL_GPIO_PIN_12
#define NT35510_CS_PORT       GPIOG



void bsp_fsmc_lcd_init(void);
















#endif