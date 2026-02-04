/**
 * @file    lwkey_test.c
 * @brief   lwkey 模块单元测试
 * @note    使用模拟框架进行离线测试，无需硬件
 */

#include "lwkey.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/*============================================================================
 * 模拟框架
 *============================================================================*/

/* 测试专用配置: 只使用 3 个硬件按键和 3 个软件按键 */
#define TEST_HKEY_COUNT    3 /* HKEY_0, HKEY_1, HKEY_2 */
#define TEST_SKEY_COUNT    3 /* 2 个普通按键 + 1 个组合键 */

/* 测试使用的软件按键 ID (索引表内) */
#define TEST_SKEY_SINGLE_0 0 /* 对应 HKEY_0 的普通按键 */
#define TEST_SKEY_SINGLE_1 1 /* 对应 HKEY_1 的普通按键 */
#define TEST_SKEY_COMBO    2 /* HKEY_0 + HKEY_1 组合键 */

/* 模拟 GPIO 状态 */
static uint8_t mock_gpio[TEST_HKEY_COUNT] = {1, 1, 1}; /* 初始为高电平(未按下) */

/* 设置模拟 GPIO 值 */
static void mock_gpio_set(uint8_t hkey_id, uint8_t level) {
    if (hkey_id < TEST_HKEY_COUNT) {
        mock_gpio[hkey_id] = level;
    }
}

/* 模拟 GPIO 读取函数 */
static uint8_t mock_gpio_read(uint8_t hkey_id) {
    if (hkey_id < TEST_HKEY_COUNT) {
        return mock_gpio[hkey_id];
    }
    return 1;
}

/* 事件记录器 */
#define EVENT_LOG_SIZE 32

static struct {
    lwkey_msg_t events[EVENT_LOG_SIZE];
    uint8_t count;
} event_log;

static void event_log_reset(void) {
    memset(&event_log, 0, sizeof(event_log));
}

static void event_log_callback(uint8_t skey_id, lwkey_event_t event) {
    if (event_log.count < EVENT_LOG_SIZE) {
        event_log.events[event_log.count].id = skey_id;
        event_log.events[event_log.count].event = (uint8_t) event;
        event_log.count++;
    }
}

/* 验证事件序列 */
static bool verify_events(const lwkey_msg_t *expected, uint8_t count) {
    if (event_log.count != count) {
        fprintf(stderr, "  Event count mismatch: expected %d, got %d\n", count, event_log.count);
        fprintf(stderr, "  Actual events:\n");
        for (uint8_t j = 0; j < event_log.count; j++) {
            fprintf(stderr, "    [%d] id=%d, evt=%d\n", j, event_log.events[j].id,
                    event_log.events[j].event);
        }
        return false;
    }
    for (uint8_t i = 0; i < count; i++) {
        if (event_log.events[i].id != expected[i].id ||
            event_log.events[i].event != expected[i].event) {
            fprintf(stderr, "  Event[%d] mismatch: expected (id=%d,evt=%d), got (id=%d,evt=%d)\n",
                    i, expected[i].id, expected[i].event, event_log.events[i].id,
                    event_log.events[i].event);
            return false;
        }
    }
    return true;
}

/* 执行 N 次扫描 */
static void run_scans(int n) {
    for (int i = 0; i < n; i++) {
        lwkey_scan();
    }
}

/* 模拟按键按下 (包含消抖时间) */
static void mock_key_press(uint8_t hkey_id) {
    mock_gpio_set(hkey_id, 0);           /* 低电平有效 */
    run_scans(LWKEY_TICKS_DEBOUNCE + 1); /* 超过消抖时间 */
}

/* 模拟按键释放 (包含消抖时间) */
static void mock_key_release(uint8_t hkey_id) {
    mock_gpio_set(hkey_id, 1); /* 高电平空闲 */
    run_scans(LWKEY_TICKS_DEBOUNCE + 1);
}

/*============================================================================
 * 测试用例
 *============================================================================*/

/* 配置表 */
static const lwkey_hkey_cfg_t test_hkey_table[TEST_HKEY_COUNT] = {
    [HKEY_0] = {.idle_level = 1},
    [HKEY_1] = {.idle_level = 1},
    [HKEY_2] = {.idle_level = 1},
};

static const lwkey_skey_cfg_t test_skey_table[TEST_SKEY_COUNT] = {
    [TEST_SKEY_SINGLE_0] = {.type = LWKEY_TYPE_NORMAL,
                            .hkey1 = HKEY_0,
                            .hkey2 = 0,
                            .flags = LWKEY_FLAG_DOUBLE_CLICK},
    [TEST_SKEY_SINGLE_1] = {.type = LWKEY_TYPE_NORMAL,
                            .hkey1 = HKEY_1,
                            .hkey2 = 0,
                            .flags = LWKEY_FLAG_DOUBLE_CLICK},
    [TEST_SKEY_COMBO] = {.type = LWKEY_TYPE_COMBO,
                         .hkey1 = HKEY_0,
                         .hkey2 = HKEY_1,
                         .flags = LWKEY_FLAG_DOUBLE_CLICK},
};

static void test_setup(void) {
    /* 重置 GPIO */
    mock_gpio[0] = 1;
    mock_gpio[1] = 1;
    mock_gpio[2] = 1;

    /* 初始化模块 */
    lwkey_cfg_t cfg = {
        .hkey_table = test_hkey_table,
        .skey_table = test_skey_table,
        .hkey_count = TEST_HKEY_COUNT,
        .skey_count = TEST_SKEY_COUNT,
        .read_fn = mock_gpio_read,
    };
    lwkey_init(&cfg);

    /* 设置全局回调 (使用 SKEY_COUNT 作为全局回调 ID，与 lwkey.c 内部一致) */
    lwkey_set_callback(SKEY_COUNT, event_log_callback);

    /* 清空事件记录 */
    event_log_reset();
    lwkey_msg_clear();
}

/*----------------------------------------------------------------------------
 * TC01: 初始化测试
 *----------------------------------------------------------------------------*/
static void test_init(void) {
    printf("TC01: Init Test... ");

    /* 正常初始化 */
    lwkey_cfg_t cfg = {
        .hkey_table = test_hkey_table,
        .skey_table = test_skey_table,
        .hkey_count = TEST_HKEY_COUNT,
        .skey_count = TEST_SKEY_COUNT,
        .read_fn = mock_gpio_read,
    };
    assert(lwkey_init(&cfg) == true);

    /* 空指针测试 */
    assert(lwkey_init(NULL) == false);

    /* 无效配置测试 */
    cfg.read_fn = NULL;
    assert(lwkey_init(&cfg) == false);

    printf("PASS\n");
}

/*----------------------------------------------------------------------------
 * TC02: 单击测试
 *----------------------------------------------------------------------------*/
static void test_single_click(void) {
    printf("TC02: Single Click Test (Target=%d ticks)... ", LWKEY_TICKS_DOUBLE_CLICK);
    test_setup();

    /* 按下 KEY0 */
    mock_key_press(HKEY_0);

    /* 释放 KEY0 */
    mock_key_release(HKEY_0);

    /* 等待双击超时 */
    run_scans(LWKEY_TICKS_DOUBLE_CLICK + 10);

    /* 验证事件序列: PRESS -> CLICK -> RELEASE */
    lwkey_msg_t expected[] = {
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_PRESS},
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_CLICK},
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_RELEASE},
    };
    assert(verify_events(expected, 3));

    printf("PASS\n");
}

/*----------------------------------------------------------------------------
 * TC03: 双击测试
 *----------------------------------------------------------------------------*/
static void test_double_click(void) {
    printf("TC03: Double Click Test... ");
    test_setup();

    /* 第一次按下释放 */
    mock_key_press(HKEY_0);
    mock_key_release(HKEY_0);

    /* 等待一小段时间 (小于双击超时) */
    run_scans(2);

    /* 第二次按下释放 */
    mock_key_press(HKEY_0);
    mock_key_release(HKEY_0);

    /* 验证事件序列: PRESS -> DOUBLE_CLICK -> RELEASE */
    lwkey_msg_t expected[] = {
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_PRESS},
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_DOUBLE_CLICK},
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_RELEASE},
    };
    assert(verify_events(expected, 3));

    printf("PASS\n");
}

/*----------------------------------------------------------------------------
 * TC04: 长按测试
 *----------------------------------------------------------------------------*/
static void test_long_press(void) {
    printf("TC04: Long Press Test... ");
    test_setup();

    /* 按下 KEY0 */
    mock_key_press(HKEY_0);

    /* 保持按下直到长按触发 */
    run_scans(LWKEY_TICKS_LONG_PRESS + 1);

    /* 释放 */
    mock_key_release(HKEY_0);

    /* 验证事件序列: PRESS -> LONG_PRESS -> RELEASE */
    lwkey_msg_t expected[] = {
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_PRESS},
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_LONG_PRESS},
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_RELEASE},
    };
    assert(verify_events(expected, 3));

    printf("PASS\n");
}

/*----------------------------------------------------------------------------
 * TC05: 长按保持测试
 *----------------------------------------------------------------------------*/
static void test_long_hold(void) {
    printf("TC05: Long Hold Test... ");
    test_setup();

    /* 按下 KEY0 */
    mock_key_press(HKEY_0);

    /* 触发长按 */
    run_scans(LWKEY_TICKS_LONG_PRESS + 1);

    /* 继续保持触发 LONG_HOLD */
    run_scans(LWKEY_TICKS_LONG_HOLD + 1);

    /* 释放 */
    mock_key_release(HKEY_0);

    /* 验证事件序列: PRESS -> LONG_PRESS -> LONG_HOLD -> RELEASE */
    lwkey_msg_t expected[] = {
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_PRESS},
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_LONG_PRESS},
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_LONG_HOLD},
        {TEST_SKEY_SINGLE_0, LWKEY_EVT_RELEASE},
    };
    assert(verify_events(expected, 4));

    printf("PASS\n");
}

/*----------------------------------------------------------------------------
 * TC06: 组合键测试
 *----------------------------------------------------------------------------*/
static void test_combo_key(void) {
    printf("TC06: Combo Key Test... ");
    test_setup();

    /* 按下主键 HKEY_0 */
    mock_key_press(HKEY_0);

    /* 按下副键 HKEY_1 (触发组合键) */
    mock_key_press(HKEY_1);

    /* 释放副键 */
    mock_key_release(HKEY_1);

    /* 等待双击超时 */
    run_scans(LWKEY_TICKS_DOUBLE_CLICK + 1);

    /* 释放主键 */
    mock_key_release(HKEY_0);

    /* 验证组合键事件 */
    bool found_combo_press = false;
    bool found_combo_click = false;
    for (uint8_t i = 0; i < event_log.count; i++) {
        if (event_log.events[i].id == TEST_SKEY_COMBO) {
            if (event_log.events[i].event == LWKEY_EVT_PRESS)
                found_combo_press = true;
            if (event_log.events[i].event == LWKEY_EVT_CLICK)
                found_combo_click = true;
        }
    }
    assert(found_combo_press && found_combo_click);

    printf("PASS\n");
}

/*----------------------------------------------------------------------------
 * TC07: 按键互斥测试
 *----------------------------------------------------------------------------*/
static void test_key_mutex(void) {
    printf("TC07: Key Mutex Test... ");
    test_setup();

    /* 按下 KEY0 */
    mock_key_press(HKEY_0);

    /* 在 KEY0 按下期间按下 KEY1，KEY0 应被中断 */
    mock_key_press(HKEY_1);

    /* 释放所有 */
    mock_key_release(HKEY_0);
    mock_key_release(HKEY_1);

    /* KEY0 应该收到 PRESS 和 RELEASE (被中断) */
    /* KEY1 应该被阻塞 */
    bool found_key0_press = false;
    bool found_key0_release = false;
    for (uint8_t i = 0; i < event_log.count; i++) {
        if (event_log.events[i].id == TEST_SKEY_SINGLE_0) {
            if (event_log.events[i].event == LWKEY_EVT_PRESS)
                found_key0_press = true;
            if (event_log.events[i].event == LWKEY_EVT_RELEASE)
                found_key0_release = true;
        }
    }
    assert(found_key0_press && found_key0_release);

    printf("PASS\n");
}

/*----------------------------------------------------------------------------
 * TC08: 消抖测试
 *----------------------------------------------------------------------------*/
static void test_debounce(void) {
    printf("TC08: Debounce Test... ");
    test_setup();

    /* 模拟抖动: 快速切换电平，不足消抖时间 */
    for (int i = 0; i < 3; i++) {
        mock_gpio_set(HKEY_0, 0);
        lwkey_scan();
        mock_gpio_set(HKEY_0, 1);
        lwkey_scan();
    }

    /* 不应有事件产生 */
    assert(event_log.count == 0);

    printf("PASS\n");
}

/*----------------------------------------------------------------------------
 * TC09: FIFO 队列测试
 *----------------------------------------------------------------------------*/
static void test_fifo(void) {
    printf("TC09: FIFO Test... ");
    test_setup();

    /* 产生多个事件 */
    for (int i = 0; i < 3; i++) {
        mock_key_press(HKEY_0);
        mock_key_release(HKEY_0);
        run_scans(LWKEY_TICKS_DOUBLE_CLICK + 1);
    }

    /* 验证 FIFO 中的消息数量 */
    assert(lwkey_msg_count() > 0);

    /* 取出所有消息 */
    lwkey_msg_t msg;
    int count = 0;
    while (lwkey_msg_fetch(&msg)) {
        count++;
    }
    assert(count > 0);
    assert(lwkey_msg_count() == 0);

    printf("PASS\n");
}

/*----------------------------------------------------------------------------
 * TC10: 回调测试
 *----------------------------------------------------------------------------*/
static void test_callback(void) {
    printf("TC10: Callback Test... ");
    test_setup();

    /* 已在 setup 中设置全局回调 */

    /* 产生事件 */
    mock_key_press(HKEY_0);
    mock_key_release(HKEY_0);
    run_scans(LWKEY_TICKS_DOUBLE_CLICK + 1);

    /* 验证回调被调用 */
    assert(event_log.count > 0);

    printf("PASS\n");
}

/*============================================================================
 * 主函数
 *============================================================================*/

int main(void) {
    printf("\n========================================\n");
    printf("   lwkey Unit Test Suite\n");
    printf("========================================\n\n");

    test_init();
    test_single_click();
    test_double_click();
    test_long_press();
    test_long_hold();
    test_combo_key();
    test_key_mutex();
    test_debounce();
    test_fifo();
    test_callback();

    printf("\n========================================\n");
    printf("   ALL TESTS PASSED\n");
    printf("========================================\n");
    return 0;
}
