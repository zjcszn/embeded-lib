//
// Created by zjcszn on 2023/12/31.
//

#define LOG_TAG    "ENCODER"
#define LOG_LVL    ELOG_LVL_INFO

#include "bsp_encoder_exti.h"

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_system.h"
#include "elog.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

static volatile int32_t encoder_cnt;
static SemaphoreHandle_t encoder_sem;

static void encoder_gpio_init(void) {
    LL_AHB1_GRP1_EnableClock(ENCODER_IO_CLK(ENCODER_A));
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = ENCODER_IO_PIN(ENCODER_A) | ENCODER_IO_PIN(ENCODER_B) | ENCODER_IO_PIN(ENCODER_S);
    LL_GPIO_Init(ENCODER_IO_PORT(ENCODER_A), &GPIO_InitStruct);
}

static void encoder_exti_init(void) {
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    // 设置外部中断的IO映射：G.6
    LL_SYSCFG_SetEXTISource(ENCODER_SYSCFG_EXTI_PORT(ENCODER_A), ENCODER_SYSCFG_EXTI_LINE(ENCODER_A));
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT; /* 中断模式 */
    EXTI_InitStruct.Line_0_31 = ENCODER_EXTI_LINE(ENCODER_A);   /* 外部中断线配置 LINE6 */
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;   /* 双边沿触发 */
    EXTI_InitStruct.LineCommand = ENABLE; /* 使能外部中断线 */
    LL_EXTI_Init(&EXTI_InitStruct);
    NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xF, 0));
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

static void encoder_task(void *arg) {
    (void)arg;
    encoder_sem = xSemaphoreCreateBinary();
    assert(encoder_sem != NULL);
    encoder_gpio_init();
    encoder_exti_init();
    while (1) {
        if (xSemaphoreTake(encoder_sem, portMAX_DELAY) == pdTRUE) {
            log_i("counter: %i", encoder_cnt);
        }
    }
}

void bsp_encoder_exti_mode_init(void) {
    xTaskCreate(encoder_task, "encoder task", 128, NULL, 24, NULL);
}

static void encoder_exti_callback(void) {
    static volatile bool flag = false;
    static volatile uint32_t pre_state;

    uint32_t encoder_a, encoder_b;
    encoder_a = LL_GPIO_IsInputPinSet(ENCODER_IO_PORT(ENCODER_A), ENCODER_IO_PIN(ENCODER_A));
    encoder_b = LL_GPIO_IsInputPinSet(ENCODER_IO_PORT(ENCODER_B), ENCODER_IO_PIN(ENCODER_B));

    if (encoder_a == 0) {
        if (flag == false) {
            pre_state = encoder_b;
            flag = true;
        }
    } else {
        if (flag == true && pre_state != encoder_b) {
            encoder_cnt += pre_state ? -1 : 1;
            xSemaphoreGiveFromISR(encoder_sem, NULL);
        }
        flag = false;
    }
}

void EXTI9_5_IRQHandler(void) {
    if (LL_EXTI_IsActiveFlag_0_31(ENCODER_EXTI_LINE(ENCODER_A)) == 1) {
        encoder_exti_callback();
        LL_EXTI_ClearFlag_0_31(ENCODER_EXTI_LINE(ENCODER_A));
    }
}

