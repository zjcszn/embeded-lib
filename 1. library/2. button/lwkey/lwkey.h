/**
 * @file    lwkey.h
 * @brief   轻量级按键管理模块 - 公共 API
 * @note    支持单击、双击、长按、组合键，支持 RTOS 和裸机环境
 */

#ifndef LWKEY_H
#define LWKEY_H

#include <stdbool.h>
#include <stdint.h>

#include "lwkey_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================================================
 * 类型定义
 * ===========================================================================*/

/**
 * @brief 按键事件类型
 */
typedef enum {
    LWKEY_EVT_NONE = 0,     /**< 无事件 */
    LWKEY_EVT_PRESS,        /**< 按下 */
    LWKEY_EVT_RELEASE,      /**< 释放 */
    LWKEY_EVT_CLICK,        /**< 单击 (按下后释放) */
    LWKEY_EVT_DOUBLE_CLICK, /**< 双击 (需在 skey_cfg.flags 中启用) */
    LWKEY_EVT_LONG_PRESS,   /**< 长按开始 */
#if LWKEY_USE_LONG_HOLD
    LWKEY_EVT_LONG_HOLD, /**< 长按保持 */
#endif
#if LWKEY_USE_REPEAT
    LWKEY_EVT_REPEAT, /**< 连按 */
#endif
} lwkey_event_t;

/**
 * @brief 软件按键类型
 */
typedef enum {
    LWKEY_TYPE_NORMAL = 0, /**< 普通按键 (单个硬件键) */
    LWKEY_TYPE_COMBO,      /**< 组合按键 (双键组合) */
} lwkey_skey_type_t;

/**
 * @brief 软件按键特性标志位
 */
#define LWKEY_FLAG_DOUBLE_CLICK (1U << 0) /**< 启用双击检测 */

/**
 * @brief 按键消息结构
 */
typedef struct {
    uint8_t id;    /**< 软件按键 ID */
    uint8_t event; /**< 事件类型 (lwkey_event_t) */
} lwkey_msg_t;

/**
 * @brief 硬件按键配置
 */
typedef struct {
    uint8_t idle_level; /**< 空闲电平: 0=低电平有效, 1=高电平有效 */
} lwkey_hkey_cfg_t;

/**
 * @brief 软件按键配置
 */
typedef struct {
    uint8_t type;  /**< 按键类型 (lwkey_skey_type_t) */
    uint8_t hkey1; /**< 主硬件按键 ID */
    uint8_t hkey2; /**< 副硬件按键 ID (组合键使用, 普通键填 0) */
    uint8_t flags; /**< 特性标志位 (LWKEY_FLAG_*) */
} lwkey_skey_cfg_t;

/**
 * @brief GPIO 读取函数类型
 * @param hkey_id 硬件按键 ID
 * @return 当前电平值 (0 或 1)
 */
typedef uint8_t (*lwkey_read_fn_t)(uint8_t hkey_id);

#if LWKEY_USE_CALLBACK
/**
 * @brief 事件回调函数类型
 * @param skey_id 软件按键 ID
 * @param event   事件类型
 */
typedef void (*lwkey_callback_fn_t)(uint8_t skey_id, lwkey_event_t event);
#endif

/**
 * @brief 模块初始化配置结构
 */
typedef struct {
    const lwkey_hkey_cfg_t *hkey_table; /**< 硬件按键配置表 */
    const lwkey_skey_cfg_t *skey_table; /**< 软件按键配置表 */
    uint8_t hkey_count;                 /**< 硬件按键数量 */
    uint8_t skey_count;                 /**< 软件按键数量 */
    lwkey_read_fn_t read_fn;            /**< GPIO 读取函数 */
} lwkey_cfg_t;

/* ===========================================================================
 * API 函数
 * ===========================================================================*/

/**
 * @brief  初始化 lwkey 模块
 * @param  cfg 配置结构体指针
 * @retval true: 成功, false: 参数无效
 */
bool lwkey_init(const lwkey_cfg_t *cfg);

/**
 * @brief  按键扫描处理函数
 * @note   - RTOS 模式: 内部自动调用
 *         - 裸机模式: 需周期性调用 (周期 = LWKEY_SCAN_PERIOD_MS)
 */
void lwkey_scan(void);

#if LWKEY_USE_RTOS
/**
 * @brief  RTOS 任务入口函数
 * @param  arg 任务参数 (未使用)
 */
void lwkey_task(void *arg);
#endif

#if LWKEY_USE_FIFO
/**
 * @brief  从消息队列获取按键事件
 * @param  msg 消息接收缓冲区
 * @retval true: 获取成功, false: 队列为空
 */
bool lwkey_msg_fetch(lwkey_msg_t *msg);

/**
 * @brief  获取消息队列中的消息数量
 * @retval 待处理消息数
 */
uint8_t lwkey_msg_count(void);

/**
 * @brief  清空消息队列
 */
void lwkey_msg_clear(void);
#endif

#if LWKEY_USE_CALLBACK
/**
 * @brief  注册按键事件回调
 * @param  skey_id  软件按键 ID, 传入 SKEY_COUNT 表示全局回调
 * @param  callback 回调函数, 传入 NULL 取消注册
 */
void lwkey_set_callback(uint8_t skey_id, lwkey_callback_fn_t callback);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LWKEY_H */
