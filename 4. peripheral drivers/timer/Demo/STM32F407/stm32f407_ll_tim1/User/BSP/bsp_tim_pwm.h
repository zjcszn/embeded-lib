#ifndef __BSP_TIM_PWM_H__
#define __BSP_TIM_PWM_H__


#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"


void bsp_pwm_freq_update(TIM_TypeDef *tim_x, uint32_t pwm_freq);
void bsp_pwm_duty_update(TIM_TypeDef *tim_x, uint32_t channel, uint32_t duty_cycle);

void bsp_pwm_output_init(TIM_TypeDef *tim_x, uint32_t channel, GPIO_TypeDef *gpio_x, uint32_t pin);
void bsp_pwm_output_start(TIM_TypeDef *tim_x, uint32_t channel, uint32_t pwm_feq, uint32_t duty_cycle);
void bsp_pwm_output_stop(TIM_TypeDef *tim_x, uint32_t channel);

void bsp_pwm_output_dual_init(TIM_TypeDef *tim_x, uint32_t channel, GPIO_TypeDef *gpiop_x, uint32_t ppin, GPIO_TypeDef *gpion_x, uint32_t npin);
void bsp_pwm_output_dual_start(TIM_TypeDef *tim_x, uint32_t channel, uint32_t pwm_feq, uint32_t duty_cycle, uint32_t deadtime);
void bsp_pwm_output_dual_stop(TIM_TypeDef *tim_x, uint32_t channel);



#endif