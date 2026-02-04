/**
 * @file    lwkey_conf.h
 * @brief   lwkey 测试环境专用配置
 * @note    此配置用于 PC 端单元测试，禁用所有硬件相关功能
 */

#ifndef LWKEY_CONF_H
#define LWKEY_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================================================
 * 平台适配配置 (测试环境)
 * ===========================================================================*/

/**
 * @brief 运行模式选择
 *        测试环境使用裸机模式
 */
#define LWKEY_USE_RTOS 0

/**
 * @brief 临界区保护宏 (测试环境下为空操作)
 */
#define LWKEY_ENTER_CRITICAL() \
    do {                       \
    } while (0)
#define LWKEY_EXIT_CRITICAL() \
    do {                      \
    } while (0)

/* ===========================================================================
 * 功能开关
 * ===========================================================================*/

#define LWKEY_USE_CALLBACK       1 /**< 启用回调机制 */
#define LWKEY_USE_FIFO           1 /**< 启用消息队列 */
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
 * 按键 ID 定义 (测试用)
 * ===========================================================================*/

/**
 * @brief 硬件按键 ID 枚举
 */
typedef enum {
    HKEY_0 = 0,
    HKEY_1,
    HKEY_2,
    HKEY_WKUP,
    HKEY_COUNT /**< 硬件按键总数 (自动计算) */
} lwkey_hkey_id_t;

/**
 * @brief 软件按键 ID 枚举
 */
typedef enum {
    SKEY_0 = 0,
    SKEY_1,
    SKEY_2,
    SKEY_WKUP,
    SKEY_COMBO_1, /**< 组合键示例: HKEY_WKUP + HKEY_0 */
    SKEY_COMBO_2, /**< 组合键示例: HKEY_WKUP + HKEY_2 */
    SKEY_COUNT    /**< 软件按键总数 (自动计算) */
} lwkey_skey_id_t;

#ifdef __cplusplus
}
#endif

#endif /* LWKEY_CONF_H */
