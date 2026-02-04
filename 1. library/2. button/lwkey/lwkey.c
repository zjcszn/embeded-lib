/**
 * @file    lwkey.c
 * @brief   轻量级按键管理模块 - 核心实现
 */

#include "lwkey.h"

/* ===========================================================================
 * 内部类型定义
 * ===========================================================================*/

/** 软件按键状态 */
typedef enum {
    STATE_IDLE = 0,     /**< 空闲 */
    STATE_PRESS,        /**< 按下中 */
    STATE_RELEASE_WAIT, /**< 等待双击 */
    STATE_PRESS_AGAIN,  /**< 二次按下 */
    STATE_LONG_PRESS,   /**< 长按中 */
    STATE_BREAK,        /**< 被中断 */
    STATE_BLOCKED,      /**< 阻塞 (其他键先按下) */
} skey_state_t;

/** 按键动作类型 */
typedef enum {
    ACTION_UP = 0, /**< 释放 */
    ACTION_DOWN,   /**< 按下 */
    ACTION_BREAK,  /**< 被其他键中断 */
} skey_action_t;

/** 硬件按键运行时数据 */
typedef struct {
    uint8_t filter_cnt; /**< 消抖计数器 */
} hkey_rt_t;

/** 软件按键运行时数据 */
typedef struct {
    uint8_t state;  /**< 当前状态 */
    uint16_t ticks; /**< 计时器 */
} skey_rt_t;

/* ===========================================================================
 * 模块上下文 (所有状态变量集中管理)
 * ===========================================================================*/

static struct {
    /* 配置信息 (只读) */
    const lwkey_hkey_cfg_t *hkey_cfg; /**< 硬件按键配置表 */
    const lwkey_skey_cfg_t *skey_cfg; /**< 软件按键配置表 */
    uint8_t hkey_count;               /**< 硬件按键数量 */
    uint8_t skey_count;               /**< 软件按键数量 */
    lwkey_read_fn_t read_fn;          /**< GPIO 读取函数 */

    /* 硬件层状态 */
    hkey_rt_t hkey_rt[LWKEY_HKEY_MAX]; /**< 硬件按键运行时数据 */
    uint32_t hkey_status;              /**< 硬件按键状态位图 */

    /* 软件层状态 */
    skey_rt_t skey_rt[LWKEY_SKEY_MAX]; /**< 软件按键运行时数据 */

#if LWKEY_USE_CALLBACK
    /* 回调函数表 */
    lwkey_callback_fn_t callbacks[LWKEY_SKEY_MAX + 1]; /**< +1 用于全局回调 */
#endif

#if LWKEY_USE_FIFO
    /* 消息队列 */
    lwkey_msg_t fifo_buf[LWKEY_FIFO_SIZE];
    volatile uint8_t fifo_r;
    volatile uint8_t fifo_w;
#endif

    /* 初始化标志 */
    bool initialized;
} ctx;

/* ===========================================================================
 * 内部辅助函数
 * ===========================================================================*/

/** 获取硬件按键状态 (从位图) */
static inline uint8_t hkey_get_status(uint8_t id) {
    return (ctx.hkey_status >> id) & 1U;
}

/** 设置硬件按键状态 (翻转位图) */
static inline void hkey_toggle_status(uint8_t id) {
    ctx.hkey_status ^= (1U << id);
}

/** 检查硬件按键电平是否变化 */
static inline bool hkey_level_changed(uint8_t id) {
    uint8_t level = ctx.read_fn(id);
    uint8_t active = (level != ctx.hkey_cfg[id].idle_level);
    return (hkey_get_status(id) != active);
}

/* ===========================================================================
 * 事件分发
 * ===========================================================================*/

static void event_dispatch(uint8_t skey_id, lwkey_event_t event) {
    if (event == LWKEY_EVT_NONE) {
        return;
    }

#if LWKEY_USE_CALLBACK
    /* 按键专属回调 */
    if (ctx.callbacks[skey_id] != NULL) {
        ctx.callbacks[skey_id](skey_id, event);
    }
    /* 全局回调 */
    if (ctx.callbacks[SKEY_COUNT] != NULL) {
        ctx.callbacks[SKEY_COUNT](skey_id, event);
    }
#endif

#if LWKEY_USE_FIFO
    /* SPSC 无锁 FIFO: 生产者只写 fifo_w */
    uint8_t used = (uint8_t) (ctx.fifo_w - ctx.fifo_r);
    if (used < LWKEY_FIFO_SIZE) {
        lwkey_msg_t *msg = &ctx.fifo_buf[ctx.fifo_w & (LWKEY_FIFO_SIZE - 1)];
        msg->id = skey_id;
        msg->event = (uint8_t) event;
        ctx.fifo_w++;
    }
#endif
}

/* ===========================================================================
 * 硬件层处理 - 消抖滤波
 * ===========================================================================*/

static void hkey_scan_handler(void) {
    for (uint8_t i = 0; i < ctx.hkey_count; i++) {
        if (hkey_level_changed(i)) {
            if (++ctx.hkey_rt[i].filter_cnt >= LWKEY_TICKS_DEBOUNCE) {
                hkey_toggle_status(i);
                ctx.hkey_rt[i].filter_cnt = 0;
            }
        } else {
            ctx.hkey_rt[i].filter_cnt = 0;
        }
    }
}

/* ===========================================================================
 * 软件层处理 - 按键动作获取
 * ===========================================================================*/

static skey_action_t skey_get_action(uint8_t skey_id) {
    const lwkey_skey_cfg_t *cfg = &ctx.skey_cfg[skey_id];

    if (cfg->type == LWKEY_TYPE_NORMAL) {
        /* 普通按键: 其他键按下时中断 */
        uint32_t other_mask = ctx.hkey_status & ~(1U << cfg->hkey1);
        if (other_mask != 0) {
            return ACTION_BREAK;
        }
        return hkey_get_status(cfg->hkey1) ? ACTION_DOWN : ACTION_UP;
    } else {
        /* 组合按键: 非相关键按下时中断 */
        uint32_t combo_mask = (1U << cfg->hkey1) | (1U << cfg->hkey2);
        uint32_t other_mask = ctx.hkey_status & ~combo_mask;
        if (other_mask != 0) {
            return ACTION_BREAK;
        }
        /* 主键必须先按下 */
        if (hkey_get_status(cfg->hkey1)) {
            return hkey_get_status(cfg->hkey2) ? ACTION_DOWN : ACTION_UP;
        }
        /* 仅副键按下 -> 中断 */
        if (hkey_get_status(cfg->hkey2)) {
            return ACTION_BREAK;
        }
        return ACTION_UP;
    }
}

/* ===========================================================================
 * 软件层处理 - 状态机
 * ===========================================================================*/

static void skey_fsm_handler(uint8_t skey_id) {
    skey_rt_t *rt = &ctx.skey_rt[skey_id];
    skey_action_t action = skey_get_action(skey_id);

    switch ((skey_state_t) rt->state) {
        case STATE_IDLE:
            if (action == ACTION_DOWN) {
                event_dispatch(skey_id, LWKEY_EVT_PRESS);
                rt->ticks = 0;
                rt->state = STATE_PRESS;
            } else if (action == ACTION_BREAK) {
                rt->state = STATE_BLOCKED;
            }
            break;

        case STATE_PRESS:
            if (action == ACTION_BREAK) {
                rt->state = STATE_BREAK;
            } else if (action == ACTION_UP) {
                /* 检查按键是否启用双击 */
                if (ctx.skey_cfg[skey_id].flags & LWKEY_FLAG_DOUBLE_CLICK) {
                    rt->ticks = 0;
                    rt->state = STATE_RELEASE_WAIT;
                } else {
                    event_dispatch(skey_id, LWKEY_EVT_CLICK);
                    event_dispatch(skey_id, LWKEY_EVT_RELEASE);
                    rt->state = STATE_IDLE;
                }
            } else {
                if (++rt->ticks >= LWKEY_TICKS_LONG_PRESS) {
                    event_dispatch(skey_id, LWKEY_EVT_LONG_PRESS);
                    rt->ticks = 0;
                    rt->state = STATE_LONG_PRESS;
                }
            }
            break;

        case STATE_RELEASE_WAIT:
            if (action == ACTION_DOWN) {
                rt->ticks = 0;
                rt->state = STATE_PRESS_AGAIN;
            } else if (++rt->ticks >= LWKEY_TICKS_DOUBLE_CLICK) {
                event_dispatch(skey_id, LWKEY_EVT_CLICK);
                event_dispatch(skey_id, LWKEY_EVT_RELEASE);
                rt->state = STATE_IDLE;
            }
            break;

        case STATE_PRESS_AGAIN:
            if (action == ACTION_BREAK) {
                rt->state = STATE_BREAK;
            } else if (action == ACTION_UP) {
                event_dispatch(skey_id, LWKEY_EVT_DOUBLE_CLICK);
                event_dispatch(skey_id, LWKEY_EVT_RELEASE);
                rt->state = STATE_IDLE;
            } else {
                if (++rt->ticks >= LWKEY_TICKS_LONG_PRESS) {
                    event_dispatch(skey_id, LWKEY_EVT_LONG_PRESS);
                    rt->ticks = 0;
                    rt->state = STATE_LONG_PRESS;
                }
            }
            break;

        case STATE_LONG_PRESS:
            if (action == ACTION_BREAK) {
                rt->state = STATE_BREAK;
            } else if (action == ACTION_UP) {
                event_dispatch(skey_id, LWKEY_EVT_RELEASE);
                rt->state = STATE_IDLE;
            } else {
#if LWKEY_USE_LONG_HOLD
                if (++rt->ticks >= LWKEY_TICKS_LONG_HOLD) {
                    event_dispatch(skey_id, LWKEY_EVT_LONG_HOLD);
                    rt->ticks = 0;
                }
#endif
            }
            break;

        case STATE_BREAK:
            if (action == ACTION_UP) {
                event_dispatch(skey_id, LWKEY_EVT_RELEASE);
                rt->state = STATE_IDLE;
            }
            break;

        case STATE_BLOCKED:
            if (ctx.hkey_status == 0) {
                rt->state = STATE_IDLE;
            }
            break;

        default:
            rt->state = STATE_IDLE;
            break;
    }
}

static void skey_scan_handler(void) {
    for (uint8_t i = 0; i < ctx.skey_count; i++) {
        skey_fsm_handler(i);
    }
}

/* ===========================================================================
 * 公共 API 实现
 * ===========================================================================*/

bool lwkey_init(const lwkey_cfg_t *cfg) {
    /* 参数校验 */
    if (cfg == NULL || cfg->read_fn == NULL) {
        return false;
    }
    if (cfg->hkey_count == 0 || cfg->hkey_count > LWKEY_HKEY_MAX) {
        return false;
    }
    if (cfg->skey_count == 0 || cfg->skey_count > LWKEY_SKEY_MAX) {
        return false;
    }
    if (cfg->hkey_table == NULL || cfg->skey_table == NULL) {
        return false;
    }

    /* 保存配置 */
    ctx.hkey_cfg = cfg->hkey_table;
    ctx.skey_cfg = cfg->skey_table;
    ctx.hkey_count = cfg->hkey_count;
    ctx.skey_count = cfg->skey_count;
    ctx.read_fn = cfg->read_fn;

    /* 清零运行时状态 */
    ctx.hkey_status = 0;
    for (uint8_t i = 0; i < LWKEY_HKEY_MAX; i++) {
        ctx.hkey_rt[i].filter_cnt = 0;
    }
    for (uint8_t i = 0; i < LWKEY_SKEY_MAX; i++) {
        ctx.skey_rt[i].state = STATE_IDLE;
        ctx.skey_rt[i].ticks = 0;
    }

#if LWKEY_USE_CALLBACK
    for (uint8_t i = 0; i <= SKEY_COUNT; i++) {
        ctx.callbacks[i] = NULL;
    }
#endif

#if LWKEY_USE_FIFO
    ctx.fifo_r = 0;
    ctx.fifo_w = 0;
#endif

    ctx.initialized = true;
    return true;
}

void lwkey_scan(void) {
    if (!ctx.initialized) {
        return;
    }
    hkey_scan_handler();
    skey_scan_handler();
}

#if LWKEY_USE_RTOS
void lwkey_task(void *arg) {
    (void) arg;
    while (1) {
        lwkey_scan();
        LWKEY_DELAY_MS(LWKEY_SCAN_PERIOD_MS);
    }
}
#endif

#if LWKEY_USE_FIFO
bool lwkey_msg_fetch(lwkey_msg_t *msg) {
    if (msg == NULL) {
        return false;
    }
    /* SPSC 无锁 FIFO: 消费者只写 fifo_r */
    if (ctx.fifo_r == ctx.fifo_w) {
        return false;
    }
    lwkey_msg_t *src = &ctx.fifo_buf[ctx.fifo_r & (LWKEY_FIFO_SIZE - 1)];
    msg->id = src->id;
    msg->event = src->event;
    ctx.fifo_r++;
    return true;
}

uint8_t lwkey_msg_count(void) {
    return (uint8_t) (ctx.fifo_w - ctx.fifo_r);
}

void lwkey_msg_clear(void) {
    ctx.fifo_r = ctx.fifo_w; /* 单条赋值为原子操作 */
}
#endif

#if LWKEY_USE_CALLBACK
void lwkey_set_callback(uint8_t skey_id, lwkey_callback_fn_t callback) {
    if (skey_id <= SKEY_COUNT) {
        ctx.callbacks[skey_id] = callback;
    }
}
#endif
