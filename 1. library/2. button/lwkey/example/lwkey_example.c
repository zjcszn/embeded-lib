/**
 * @file    lwkey_example.c
 * @brief   lwkey 模块使用示例
 * @note    此文件展示如何配置和使用 lwkey 模块
 */

#include "lwkey.h"

/* ===========================================================================
 * 硬件按键配置表
 * ===========================================================================*/

static const lwkey_hkey_cfg_t hkey_table[HKEY_COUNT] = {
    [HKEY_0] = {.idle_level = 1}, /* 高电平空闲, 按下为低 */
    [HKEY_1] = {.idle_level = 1},
    [HKEY_2] = {.idle_level = 1},
    [HKEY_WKUP] = {.idle_level = 0}, /* 低电平空闲, 按下为高 */
};

/* ===========================================================================
 * 软件按键配置表
 * ===========================================================================*/

static const lwkey_skey_cfg_t skey_table[SKEY_COUNT] = {
    /* 普通按键 */
    [SKEY_0] = {.type = LWKEY_TYPE_NORMAL, .hkey1 = HKEY_0, .hkey2 = 0},
    [SKEY_1] = {.type = LWKEY_TYPE_NORMAL, .hkey1 = HKEY_1, .hkey2 = 0},
    [SKEY_2] = {.type = LWKEY_TYPE_NORMAL, .hkey1 = HKEY_2, .hkey2 = 0},
    [SKEY_WKUP] = {.type = LWKEY_TYPE_NORMAL, .hkey1 = HKEY_WKUP, .hkey2 = 0},
    /* 组合按键: WKUP + KEY0 */
    [SKEY_COMBO_1] = {.type = LWKEY_TYPE_COMBO, .hkey1 = HKEY_WKUP, .hkey2 = HKEY_0},
    /* 组合按键: WKUP + KEY2 */
    [SKEY_COMBO_2] = {.type = LWKEY_TYPE_COMBO, .hkey1 = HKEY_WKUP, .hkey2 = HKEY_2},
};

/* ===========================================================================
 * GPIO 读取函数 (需根据实际硬件实现)
 * ===========================================================================*/

static uint8_t gpio_read_key(uint8_t hkey_id) {
    /* 示例: 根据按键 ID 读取对应 GPIO */
    switch (hkey_id) {
        case HKEY_0:
            return HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin);
        case HKEY_1:
            return HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
        case HKEY_2:
            return HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);
        case HKEY_WKUP:
            return HAL_GPIO_ReadPin(WKUP_GPIO_Port, WKUP_Pin);
        default:
            return 1;
    }
}

/* ===========================================================================
 * 按键事件回调 (可选)
 * ===========================================================================*/

static void key_event_handler(uint8_t skey_id, lwkey_event_t event) {
    switch (event) {
        case LWKEY_EVT_CLICK:
            printf("Key %d: Click\n", skey_id);
            break;
        case LWKEY_EVT_DOUBLE_CLICK:
            printf("Key %d: Double Click\n", skey_id);
            break;
        case LWKEY_EVT_LONG_PRESS:
            printf("Key %d: Long Press\n", skey_id);
            break;
        case LWKEY_EVT_LONG_HOLD:
            printf("Key %d: Long Hold\n", skey_id);
            break;
        default:
            break;
    }
}

/* ===========================================================================
 * 初始化示例
 * ===========================================================================*/

void app_key_init(void) {
    /* 配置结构体 */
    lwkey_cfg_t cfg = {
        .hkey_table = hkey_table,
        .skey_table = skey_table,
        .hkey_count = HKEY_COUNT,
        .skey_count = SKEY_COUNT,
        .read_fn = gpio_read_key,
    };

    /* 初始化模块 */
    if (!lwkey_init(&cfg)) {
        /* 初始化失败处理 */
        return;
    }

#if LWKEY_USE_CALLBACK
    /* 注册全局回调 (所有按键事件都会触发) */
    lwkey_set_callback(SKEY_COUNT, key_event_handler);

    /* 或为特定按键注册回调 */
    // lwkey_set_callback(SKEY_0, key_event_handler);
#endif
}

/* ===========================================================================
 * 使用方式 1: RTOS 任务模式
 * ===========================================================================*/

#if LWKEY_USE_RTOS
/*
 * 在 RTOS 中创建任务:
 *   xTaskCreate(lwkey_task, "lwkey", 128, NULL, 2, NULL);
 *
 * 然后在应用层轮询消息队列:
 */
void app_key_process(void) {
    lwkey_msg_t msg;
    while (lwkey_msg_fetch(&msg)) {
        /* 处理按键事件 */
        switch (msg.event) {
            case LWKEY_EVT_CLICK:
                /* 单击处理 */
                break;
            case LWKEY_EVT_DOUBLE_CLICK:
                /* 双击处理 */
                break;
                /* ... */
        }
    }
}
#endif

/* ===========================================================================
 * 使用方式 2: 裸机轮询模式
 * ===========================================================================*/

#if !LWKEY_USE_RTOS
/*
 * 方式 A: 在定时器中断中调用 lwkey_scan() (推荐)
 *
 * void TIM_IRQHandler(void) {
 *     if (TIM_GetITStatus(TIM, TIM_IT_Update)) {
 *         lwkey_scan();  // 每 10ms 调用一次
 *         TIM_ClearITPendingBit(TIM, TIM_IT_Update);
 *     }
 * }
 *
 * 方式 B: 在主循环中轮询调用
 */
void app_main_loop(void) {
    static uint32_t last_tick = 0;

    while (1) {
        /* 定时调用扫描函数 */
        if (HAL_GetTick() - last_tick >= LWKEY_SCAN_PERIOD_MS) {
            last_tick = HAL_GetTick();
            lwkey_scan();
        }

        /* 处理按键消息 */
        lwkey_msg_t msg;
        while (lwkey_msg_fetch(&msg)) {
            /* 处理事件 */
        }

        /* 其他应用逻辑 */
    }
}
#endif
