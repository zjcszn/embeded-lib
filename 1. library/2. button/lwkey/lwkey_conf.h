/**
 * @file    lwkey_conf_template.h
 * @brief   lwkey 配置模板文件
 * @note    复制此文件到项目目录并重命名为 lwkey_conf.h，根据实际需求修改配置
 */

#ifndef LWKEY_CONF_H
#define LWKEY_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================================================
 * 平台适配配置
 * ===========================================================================*/

/**
 * @brief 运行模式选择
 *        1 = RTOS 任务模式 (需实现 LWKEY_DELAY_MS 宏)
 *        0 = 裸机轮询模式 (需周期性调用 lwkey_scan)
 */
#define LWKEY_USE_RTOS 0

/**
 * @brief 临界区保护宏 (必须根据平台实现)
 * @note  裸机: 使用中断开关
 *        RTOS: 使用调度锁或互斥锁
 *        测试: 定义 LWKEY_TEST_MODE 使用空操作
 */
#ifdef LWKEY_TEST_MODE
/* 测试环境: 空操作 */
#define LWKEY_ENTER_CRITICAL() \
    do {                       \
    } while (0)
#define LWKEY_EXIT_CRITICAL() \
    do {                      \
    } while (0)
#else
/* 嵌入式环境: 使用中断开关 */
#define LWKEY_ENTER_CRITICAL() \
    do {                       \
        __disable_irq();       \
    } while (0)
#define LWKEY_EXIT_CRITICAL() \
    do {                      \
        __enable_irq();       \
    } while (0)
#endif

/**
 * @brief RTOS 延时宏 (仅 RTOS 模式需要)
 * @note  FreeRTOS: vTaskDelay(pdMS_TO_TICKS(ms))
 *        RT-Thread: rt_thread_mdelay(ms)
 *        uC/OS:     OSTimeDlyHMSM(0, 0, 0, ms)
 */
#if LWKEY_USE_RTOS
#define LWKEY_DELAY_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))
#endif

/* ===========================================================================
 * 功能开关
 * ===========================================================================*/

#define LWKEY_USE_CALLBACK       1 /**< 启用回调机制 */
#define LWKEY_USE_FIFO           1 /**< 启用消息队列 */
#define LWKEY_USE_DOUBLE_CLICK   1 /**< 启用双击检测 */
#define LWKEY_USE_LONG_HOLD      1 /**< 启用长按保持事件 */
#define LWKEY_USE_REPEAT         0 /**< 启用连按检测 (可选) */

/* ===========================================================================
 * 时间参数 (单位: 扫描周期数)
 * ===========================================================================*/

#define LWKEY_SCAN_PERIOD_MS     10  /**< 扫描周期 (毫秒) */
#define LWKEY_TICKS_DEBOUNCE     2   /**< 消抖滤波周期数 */
#define LWKEY_TICKS_LONG_PRESS   100 /**< 长按判定阈值 */
#define LWKEY_TICKS_LONG_HOLD    5   /**< 长按保持事件间隔 */
#define LWKEY_TICKS_DOUBLE_CLICK 30  /**< 双击间隔阈值 */
#define LWKEY_TICKS_REPEAT       20  /**< 连按间隔 (需开启 REPEAT) */

/* ===========================================================================
 * 容量配置
 * ===========================================================================*/

#define LWKEY_FIFO_SIZE          16 /**< 消息队列大小 (必须为2的幂) */
#define LWKEY_HKEY_MAX           8  /**< 最大硬件按键数 */
#define LWKEY_SKEY_MAX           16 /**< 最大软件按键数 */

/* ===========================================================================
 * 按键 ID 定义 (根据实际硬件修改)
 * ===========================================================================*/

/**
 * @brief 硬件按键 ID 枚举
 * @note  对应物理按键，需与 GPIO 读取函数配合
 */
typedef enum {
    HKEY_0 = 0,
    HKEY_1,
    HKEY_2,
    HKEY_WKUP,
    /* 添加更多硬件按键... */
    HKEY_COUNT /**< 硬件按键总数 (自动计算) */
} lwkey_hkey_id_t;

/**
 * @brief 软件按键 ID 枚举
 * @note  可包含普通按键和组合按键
 */
typedef enum {
    SKEY_0 = 0,
    SKEY_1,
    SKEY_2,
    SKEY_WKUP,
    SKEY_COMBO_1, /**< 组合键示例: HKEY_WKUP + HKEY_0 */
    SKEY_COMBO_2, /**< 组合键示例: HKEY_WKUP + HKEY_2 */
    /* 添加更多软件按键... */
    SKEY_COUNT /**< 软件按键总数 (自动计算) */
} lwkey_skey_id_t;

#ifdef __cplusplus
}
#endif

#endif /* LWKEY_CONF_H */
