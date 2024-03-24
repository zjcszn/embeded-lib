//
// Created by zjcszn on 2024/1/6.
//

#include "bsp_encoder_polling.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"
#include "FreeRTOS.h"
#include "semphr.h"

typedef enum {
    ENCODER_IDLE,
    ENCODER_ROTATING_DEBOUNCE,
    ENCODER_ROTATING_START,
    ENCODER_ROTATING_COMPLETE,
} encoder_state_t;

typedef struct {
    uint32_t state;
    uint32_t a;
    uint32_t b;
    uint32_t counter;
    uint32_t filter;
    SemaphoreHandle_t sem;
} encoder_handle_t;

static encoder_handle_t encoder_handle;

static void encoder_gpio_init(void) {
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = LL_GPIO_PIN_4 | LL_GPIO_PIN_5;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void encoder_tim_init(void) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = ((SystemCoreClock / 2) / 1000000) - 1;       // 预分频系数：83 < TIMER 时钟：APB1 * 2 = 84Mhz >
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;                     // 计数模式： 递增
    TIM_InitStruct.Autoreload = 999;                                        // 自动重加载值：999
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    TIM_InitStruct.RepetitionCounter = 0;
    LL_TIM_Init(TIM2, &TIM_InitStruct);                                     // 定时器频率：84Mhz / (83 + 1) / (999 + 1) = 1kHz
    LL_TIM_EnableIT_UPDATE(TIM2);                                           // 使能定时器UPDATE中断
    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0x5, 0));
    NVIC_EnableIRQ(TIM2_IRQn);
    LL_TIM_EnableCounter(TIM2);
}

static void encoder_polling_task(void *arg) {
    (void)arg;
    encoder_handle.sem = xSemaphoreCreateBinary();
    assert(encoder_handle.sem != NULL);
    encoder_gpio_init();
    encoder_tim_init();
    while (1) {
        if (xSemaphoreTake(encoder_handle.sem, portMAX_DELAY) == pdTRUE) {
            log_i("counter: %i", encoder_handle.counter);
        }
    }
}

void bsp_encoder_polling_mode_init(void) {
    xTaskCreate(encoder_polling_task, "encoder task", 128, NULL, 24, NULL);
}

void encoder_tim_callback(void) {
    uint32_t a, b;
    a = LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_4);
    b = LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_5);

    switch (encoder_handle.state) {
        case ENCODER_IDLE:
            /* 检测到a，b相电平相反，记录A,B相电平，更新状态机，进行消抖 */
            if (a != b) {
                encoder_handle.a = a;
                encoder_handle.b = b;
                encoder_handle.filter = 1;
                encoder_handle.state = ENCODER_ROTATING_DEBOUNCE;
            }
            break;

        case ENCODER_ROTATING_DEBOUNCE:
            /* 消抖，两次电平检测一致，则判断A，B相已稳定，更新状态机，等待编码器A,B相拉低；否则判断为抖动，退回空闲状态 */
            if (a == encoder_handle.a && b == encoder_handle.b) {
                if (++encoder_handle.filter == 2) {
                    encoder_handle.filter = 0;
                    encoder_handle.state = ENCODER_ROTATING_START;
                }
            } else {
                encoder_handle.state = ENCODER_IDLE;
            }
            break;

        case ENCODER_ROTATING_START:
            /* 如果检测到A,B相同时拉低，则更新状态机，等待A,B相拉高；如果连续两次检测到A,B相为高电平，则判断为误动作，退回空闲状态 */
            if (a == 0 && b == 0) {
                encoder_handle.counter += encoder_handle.a ? 1 : -1;
                xSemaphoreGiveFromISR(encoder_handle.sem, NULL);
                encoder_handle.filter = 0;
                encoder_handle.state = ENCODER_ROTATING_COMPLETE;
            } else if (a == 1 && b == 1) {
                if (++encoder_handle.filter == 2) {
                    encoder_handle.state = ENCODER_IDLE;
                }
            }
            break;

        case ENCODER_ROTATING_COMPLETE:
            /* 如果连续两次检测到A,B相拉高，判断旋转完成 */
            if (a == 1 && b == 1) {
                if (++encoder_handle.filter == 2) {
                    encoder_handle.state = ENCODER_IDLE;
                }
            } else {
                encoder_handle.filter = 0;
            }
            break;

        default:
            break;
    }
}

void TIM2_IRQHandler(void) {
    if (LL_TIM_IsActiveFlag_UPDATE(TIM2) && LL_TIM_IsEnabledIT_UPDATE(TIM2)) {
        encoder_tim_callback();
        LL_TIM_ClearFlag_UPDATE(TIM2);
    }
}


