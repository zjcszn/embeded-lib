#ifndef __BSP_TIM_DMA_H__
#define __BSP_TIM_DMA_H__

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_system.h"

#define PWM_GPIO          GPIOE
#define PWM_PIN           LL_GPIO_PIN_ALL

#define PWM_TIM           TIM1
#define PWM_TIM_CHANNEL   LL_TIM_CHANNEL_CH1

#define PWM_DMA           DMA2
#define PWM_DMA_STREAM    LL_DMA_STREAM_5
#define PWM_DMA_CHANNEL   LL_DMA_CHANNEL_6



void bsp_pwm_dma_init(void);
void bsp_pwm_dma_start(uint32_t *dmabuf_addr, uint32_t dmabuf_size, uint32_t pwm_freq);
void bsp_pwm_dma_stop(void);

#endif