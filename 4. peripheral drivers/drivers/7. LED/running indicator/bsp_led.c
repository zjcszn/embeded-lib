//
// Created by zjcszn on 2024/1/27.
//

#include "bsp_led.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"
#include "FreeRTOS.h"
#include "task.h"

static uint32_t led_freq;

static void prv_led_gpio_init(void) {
    USER_LED_CLOCK_ENABLE();
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pin = USER_LED_PIN;
    LL_GPIO_Init(USER_LED_PORT, &GPIO_InitStruct);
    LL_GPIO_SetOutputPin(USER_LED_PORT, USER_LED_PIN);
}

static void prv_led_task(void *args) {
    (void)args;
    uint8_t flag = 0;
    TickType_t xLastWakeTime, xTimePeriod;

    bsp_led_freq_set(10);
    xLastWakeTime = xTaskGetTickCount();
    for (;; flag = !flag) {
        xTimePeriod = 1000 / led_freq;
        USER_LED_PORT->BSRR = (USER_LED_PIN) << (flag << 4);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(xTimePeriod));
    }
}

void bsp_led_freq_set(uint32_t freq) {
    led_freq = (freq == 0) ? 1 : (freq > 20) ? 20 : freq;
}

void bsp_led_init(void) {
    BaseType_t xReturn;

    prv_led_gpio_init();
    xReturn = xTaskCreate(prv_led_task, "led task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    configASSERT(xReturn == pdTRUE);
}