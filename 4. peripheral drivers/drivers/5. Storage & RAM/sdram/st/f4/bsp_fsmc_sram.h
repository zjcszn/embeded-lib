#ifndef __BSP_FSMC_SRAM_H__
#define __BSP_FSMC_SRAM_H__


#include "stdint.h"
#include "stm32f407xx.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_fsmc.h"
#include "stm32f4xx_hal_sram.h"


void bsp_fsmc_sram_init(void);


#endif