//
// Created by zjcszn on 2023/12/31.
//

#ifndef _BSP_BSP_ENCODER_EXTI_H_
#define _BSP_BSP_ENCODER_EXTI_H_

#define ENCODER_A   G,6
#define ENCODER_B   G,7
#define ENCODER_S   G,8

#define GPIO_PORT(a, b)                 GPIO##a
#define ENCODER_IO_PORT(a)              GPIO_PORT(a)
#define GPIO_PIN(a, b)                  LL_GPIO_PIN_##b
#define ENCODER_IO_PIN(a)               GPIO_PIN(a)
#define EXTI_SOURCES_PORT(a, b)         LL_SYSCFG_EXTI_PORT##a
#define ENCODER_SYSCFG_EXTI_PORT(a)     EXTI_SOURCES_PORT(a)
#define EXTI_SOURCES_LINE(a, b)         LL_SYSCFG_EXTI_LINE##b
#define ENCODER_SYSCFG_EXTI_LINE(a)     EXTI_SOURCES_LINE(a)
#define EXTI_LINE(a, b)                 LL_EXTI_LINE_##b
#define ENCODER_EXTI_LINE(a)            EXTI_LINE(a)
#define GPIO_CLOCK(a, b)                LL_AHB1_GRP1_PERIPH_GPIO##a
#define ENCODER_IO_CLK(a)               GPIO_CLOCK(a)

void bsp_encoder_exti_mode_init(void);

#endif //_BSP_BSP_ENCODER_EXTI_H_
