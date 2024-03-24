//
// Created by zjcszn on 2024/1/8.
//

#define LOG_LVL ELOG_LVL_INFO
#define LOG_TAG "TOUCH"

#include "bsp_touchkey.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "bsp_led.h"

static QueueHandle_t touchkey_queue;
static uint32_t base_val;

static void touchkey_gpio_init(void) {
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;           /* 设置 GPIO 为推挽输出模式 (拉低IO时，可能会导致下冲振铃，可考虑浮空下拉) */
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pin = TPAD_PIN;
    LL_GPIO_Init(TPAD_PORT, &GPIO_InitStruct);
    LL_GPIO_ResetOutputPin(TPAD_PORT, TPAD_PIN);
}

static void touchkey_tim_init(void) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 0;                                           /* 预分频系数：0 */
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;                     /* 计数模式： 递增 */
    TIM_InitStruct.Autoreload = 0xFFFFFFFF;                                 /* 自动重加载值：Max */
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    TIM_InitStruct.RepetitionCounter = 0;
    LL_TIM_Init(TIM2, &TIM_InitStruct);

    LL_TIM_IC_InitTypeDef TIM_IC_InitStruct = {0};
    TIM_IC_InitStruct.ICPrescaler = 0;
    TIM_IC_InitStruct.ICFilter = 0;
    TIM_IC_InitStruct.ICPolarity = LL_TIM_IC_POLARITY_RISING;               /* 上升沿捕获 */
    TIM_IC_InitStruct.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;          /* CC1绑定到T1输入 */
    LL_TIM_IC_Init(TIM2, LL_TIM_CHANNEL_CH1, &TIM_IC_InitStruct);

    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0x5, 0));
    NVIC_EnableIRQ(TIM2_IRQn);
    LL_TIM_EnableIT_CC1(TIM2);      /* 通过TIM2 CH1输入捕获，获得充电时间 */
    LL_TIM_EnableCounter(TIM2);
}

static void touchkey_reset(void) {
    LL_GPIO_ResetOutputPin(TPAD_PORT, TPAD_PIN);
    LL_GPIO_SetPinMode(TPAD_PORT, TPAD_PIN, LL_GPIO_MODE_OUTPUT);
    vTaskDelay(pdMS_TO_TICKS(2));   /* 放电时间 2ms */

    LL_TIM_SetCounter(TIM2, 0);     /* 重置计数器 */
    LL_TIM_ClearFlag_CC1(TIM2);     /* 清除CC1中断标志 */

    LL_GPIO_SetAFPin_0_7(TPAD_PORT, TPAD_PIN, LL_GPIO_AF_1);        /* 设置GPIO为复用模式，映射到定时器捕获端口*/
    LL_GPIO_SetPinMode(TPAD_PORT, TPAD_PIN, LL_GPIO_MODE_ALTERNATE);
}

static bool touchkey_get_charge_time(uint32_t *charge_time) {
    uint32_t val[10];
    uint32_t min = ~0ULL, max = 0;
    uint32_t sum = 0;

    // 采样10次，并经中位值平均滤波算法处理，抑制偶然的脉冲干扰
    for (int i = 0; i < 10; i++) {
        touchkey_reset();
        if (xQueueReceive(touchkey_queue, &val[i], pdMS_TO_TICKS(10)) == pdFALSE) {
            return false;
        };
        min = min < val[i] ? min : val[i];
        max = max > val[i] ? max : val[i];
        sum += val[i];
        log_d("raw[%d]:%u", i, val[i]);
    }
    *charge_time = (sum - min - max) / 8;   /* 剔除最大及最小值后，取平均值 */
    log_d("\r\ntouch=%u\r\n", *charge_time);
    return true;
}

static void touchkey_task(void *arg) {
    touchkey_queue = xQueueCreate(1, sizeof(uint32_t));
    assert(touchkey_queue != NULL);
    touchkey_gpio_init();
    touchkey_tim_init();

    // 初始化触摸式按键基值
    vTaskDelay(pdMS_TO_TICKS(100));
    while (touchkey_get_charge_time(&base_val) != true);
    log_i("base val = %u", base_val);

    uint32_t read_val;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (touchkey_get_charge_time(&read_val) == true) {
            /* todo */
        };

        log_i("\r\ntouch=%u\r\n", read_val);

        if (read_val > (base_val + base_val / 10)) {
            bsp_led_control(LED_RED, 1);
        } else {
            bsp_led_control(LED_RED, 0);
        }

    }
}

void bsp_touchkey_init(void) {
    xTaskCreate(touchkey_task, "touch button task", 256, NULL, 24, NULL);
}

void TIM2_IRQHandler(void) {
    if (LL_TIM_IsEnabledIT_CC1(TIM2) && LL_TIM_IsActiveFlag_CC1(TIM2)) {
        uint32_t val = LL_TIM_IC_GetCaptureCH1(TIM2);
        xQueueSendFromISR(touchkey_queue, &val, NULL);
        LL_TIM_ClearFlag_CC1(TIM2);
    }
}