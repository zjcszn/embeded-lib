/**
 * @file test_multi_task.c
 * @brief multi_task 模块的单元测试套件
 * @note  包含基础功能、边界条件及过载保护机制的测试
 */

#include <stdio.h>
#include <string.h>
#include "multi_task.h"

/*============================== Test Configuration =================================*/
/* 测试用例开关，置 1 开启，置 0 关闭 */
#define TEST_CASE_BASIC_FUNC      1   // 基础功能：周期与单次任务
#define TEST_CASE_PARAM_CHECK     1   // 参数校验：空指针等
#define TEST_CASE_MAX_LIMIT       1   // 边界测试：任务池满
#define TEST_CASE_PRIORITY        1   // 排序测试：不同 Deadline 的执行顺序
#define TEST_CASE_OVERLOAD        1   // 核心测试：验证滞后追赶与过载重置逻辑

/* 日志与断言宏封装 */
#define LOG_TAG "[TEST]"

#define TEST_LOG(fmt, ...) \
    printf(LOG_TAG " " fmt "\n", ##__VA_ARGS__)

#define TEST_ERROR(fmt, ...) \
    printf(LOG_TAG " [ERROR] " fmt "\n", ##__VA_ARGS__)

#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        TEST_ERROR("Assertion Failed: %s (Line %d)", msg, __LINE__); \
        return; \
    } \
} while(0)

/* 模拟临界区（测试环境通常为单线程，可为空） */
/* 注意：如果 multi_task.h 中没有定义这些宏，需要在编译选项中定义，或在此处覆盖 */
#ifndef TASK_ENTER_CRITICAL
#define TASK_ENTER_CRITICAL()
#endif
#ifndef TASK_EXIT_CRITICAL
#define TASK_EXIT_CRITICAL()
#endif

/*============================== Global Variables for Tests =================================*/
static volatile int g_task_a_cnt = 0;
static volatile int g_task_b_cnt = 0;
static volatile uint32_t g_last_run_tick = 0;

/*============================== Mock Tasks =================================*/

/* 简单的计数任务 */
int task_func_a(void *arg) {
    g_task_a_cnt++;
    /* 记录最后一次运行的时间 */
    g_last_run_tick = task_get_tick(); 
    // TEST_LOG("Task A Executed at tick: %u", g_last_run_tick);
    return 0; // 继续运行
}

int task_func_b(void *arg) {
    g_task_b_cnt++;
    // TEST_LOG("Task B Executed at tick: %u", task_get_tick());
    return 0;
}

/* 返回非0值的任务（请求自我删除） */
int task_func_oneshot(void *arg) {
    g_task_a_cnt++;
    TEST_LOG("One-shot task running...");
    return 1; // 返回 1 表示停止/删除
}

/*============================== Helper Functions =================================*/

/* 模拟时间流逝 N ms */
void mock_time_pass(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        task_tick_update();
    }
}

/*============================== Test Cases =================================*/

#if TEST_CASE_BASIC_FUNC
void test_basic_functionality(void) {
    TEST_LOG("=== Test Case: Basic Functionality ===");
    
    task_system_init();
    g_task_a_cnt = 0;
    g_task_b_cnt = 0;

    /* 创建周期任务 A: 延迟0ms，周期10ms */
    task_create(task_func_a, NULL, 0, 10);
    /* 创建单次任务 B: 延迟5ms，周期0ms (单次) */
    task_create(task_func_b, NULL, 5, 0);

    /* 模拟 2ms */
    mock_time_pass(2);
    task_scheduler();
    TEST_ASSERT(g_task_a_cnt == 1, "Task A should run immediately (delay 0)");
    TEST_ASSERT(g_task_b_cnt == 0, "Task B should execute yet");

    /* 模拟再过 4ms (总 6ms) */
    mock_time_pass(4);
    task_scheduler();
    TEST_ASSERT(g_task_b_cnt == 1, "Task B should have run (delay 5)");
    
    /* 模拟再过 10ms (总 16ms) */
    mock_time_pass(10);
    task_scheduler();
    TEST_ASSERT(g_task_a_cnt == 2, "Task A should run 2nd time (period 10)");
    TEST_ASSERT(g_task_b_cnt == 1, "Task B should NOT run again (oneshot)");

    TEST_LOG("Result: PASS");
}
#endif

#if TEST_CASE_PARAM_CHECK
void test_param_check(void) {
    TEST_LOG("=== Test Case: Parameter Check ===");
    task_system_init();

    task_status_t ret;

    /* 测试空函数指针 */
    ret = task_create(NULL, NULL, 0, 100);
    TEST_ASSERT(ret == TASK_ERR_PARAM, "Should reject NULL func");

    /* 测试删除空任务 */
    ret = task_delete(NULL);
    TEST_ASSERT(ret == TASK_ERR_PARAM, "Should reject delete NULL");

    /* 测试删除不存在的任务 */
    ret = task_delete(task_func_a);
    TEST_ASSERT(ret == TASK_ERR_NOT_FOUND, "Should return NOT_FOUND for non-existent task");

    TEST_LOG("Result: PASS");
}
#endif

#if TEST_CASE_MAX_LIMIT
int dummy_task(void *arg) { return 0; }
/* 需要定义不同的函数地址，或者修改调度器允许相同函数(当前调度器不允许) 
 * 这里我们通过不同的函数指针来模拟
 */
int dummy_task_0(void *a){ return 0; }
int dummy_task_1(void *a){ return 0; }
int dummy_task_2(void *a){ return 0; }
int dummy_task_3(void *a){ return 0; }
int dummy_task_4(void *a){ return 0; }
int dummy_task_5(void *a){ return 0; }
int dummy_task_6(void *a){ return 0; }
int dummy_task_7(void *a){ return 0; }
int dummy_task_8(void *a){ return 0; }
int dummy_task_9(void *a){ return 0; }
int dummy_task_overflow(void *a){ return 0; }

void test_max_limit(void) {
    TEST_LOG("=== Test Case: Max Limit (Pool Size: %d) ===", TASK_MAX_NUM);
    task_system_init();

    task_func_t tasks[] = {
        dummy_task_0, dummy_task_1, dummy_task_2, dummy_task_3, dummy_task_4,
        dummy_task_5, dummy_task_6, dummy_task_7, dummy_task_8, dummy_task_9
    };

    /* 填满任务池 */
    for (int i = 0; i < TASK_MAX_NUM; i++) {
        task_status_t ret = task_create(tasks[i], NULL, 100, 100);
        if (ret != TASK_OK) {
            TEST_ERROR("Failed to create task index %d", i);
            return;
        }
    }

    /* 尝试添加第 11 个任务 */
    task_status_t ret = task_create(dummy_task_overflow, NULL, 100, 100);
    TEST_ASSERT(ret == TASK_ERR_FULL, "Should return ERR_FULL when pool is full");

    /* 删除一个任务 */
    task_delete(dummy_task_0);

    /* 再次尝试添加 */
    ret = task_create(dummy_task_overflow, NULL, 100, 100);
    TEST_ASSERT(ret == TASK_OK, "Should allow creation after deletion");

    TEST_LOG("Result: PASS");
}
#endif

#if TEST_CASE_PRIORITY
void test_priority_ordering(void) {
    TEST_LOG("=== Test Case: Execution Ordering ===");
    task_system_init();
    
    /* * 场景：插入顺序为 A(Delay 20), B(Delay 10)
     * 期望执行顺序：B 先运行，A 后运行 (基于 Deadline 排序)
     */
    
    /* 为了验证顺序，我们在任务中使用 log 打印，或者检查 trace 数组。
     * 这里简化为：先运行到 Time=15，检查B运行A未运行。
     */
    g_task_a_cnt = 0;
    g_task_b_cnt = 0;

    task_create(task_func_a, NULL, 20, 0); // Delay 20
    task_create(task_func_b, NULL, 10, 0); // Delay 10

    mock_time_pass(15);
    task_scheduler();

    TEST_ASSERT(g_task_b_cnt == 1, "Task B (Delay 10) should have run at Tick 15");
    TEST_ASSERT(g_task_a_cnt == 0, "Task A (Delay 20) should NOT have run at Tick 15");

    mock_time_pass(10); // Total 25
    task_scheduler();
    TEST_ASSERT(g_task_a_cnt == 1, "Task A (Delay 20) should have run at Tick 25");

    TEST_LOG("Result: PASS");
}
#endif

#if TEST_CASE_OVERLOAD
void test_overload_protection(void) {
    TEST_LOG("=== Test Case: Lag Catch-up vs Overload Protection ===");
    task_system_init();
    g_task_a_cnt = 0;
    
    /* * 设置一个周期为 10 ticks 的任务 
     * 初始 Deadline = current(0) + 10 = 10
     */
    uint32_t period = 10;
    task_create(task_func_a, NULL, 10, period);

    /* --- 场景 1: 轻微滞后 (Catch-up) --- */
    /* 模拟过了 12 ticks (Deadline 是 10，滞后 2 ticks) */
    /* 逻辑：2 < 10 (Period)，应执行，且下一次 Deadline = 10 + 10 = 20 (严格周期) */
    mock_time_pass(12); // Now = 12
    task_scheduler();   // 执行第一次
    
    TEST_ASSERT(g_task_a_cnt == 1, "Task should execute 1st time");
    
    /* 此时 Deadline 应该被设定为 20。
     * 我们再模拟 8 ticks (Now = 20)，应该正好再次触发
     */
    mock_time_pass(8); // Now = 20
    task_scheduler();
    TEST_ASSERT(g_task_a_cnt == 2, "Task should execute 2nd time exactly at tick 20 (Catch-up worked)");


    /* --- 场景 2: 严重过载 (Overload Reset) --- */
    /* * 当前 Now = 20, 下次理论 Deadline = 30。
     * 我们让系统"卡死"或睡眠很久，直接跳过 50 ticks。
     * Now = 70。滞后 = 70 - 30 = 40。
     * 逻辑：40 > 10 (Period)，触发过载保护。
     * 行为：执行任务，但下一次 Deadline 重置为 Now + Period = 70 + 10 = 80。
     * 而不是补齐中间丢失的 30, 40, 50, 60...
     */
    mock_time_pass(50); // Now = 70
    TEST_LOG("Simulating massive lag (Now=70, Expected=30)...");
    
    int cnt_before = g_task_a_cnt; // 应该是 2
    task_scheduler(); 
    /* 任务会执行一次 */
    TEST_ASSERT(g_task_a_cnt == cnt_before + 1, "Task should execute once after lag");

    /* * 关键验证：如果它是“追赶”模式，Deadline 此时是 40，
     * 再次调用 scheduler 会立即执行（因为 70 >= 40）。
     * 如果它是“重置”模式，Deadline 是 80，再次调用 scheduler 不会执行。
     */
    task_scheduler();
    TEST_ASSERT(g_task_a_cnt == cnt_before + 1, "Task should NOT burst execute (Overload Protection Active)");

    /* 验证它是否在 80 时恢复正常 */
    mock_time_pass(10); // Now = 80
    task_scheduler();
    TEST_ASSERT(g_task_a_cnt == cnt_before + 2, "Task should resume normal schedule at Now+Period");

    TEST_LOG("Result: PASS");
}
#endif

/*============================== Main Entry =================================*/

int main(void) {
    TEST_LOG("Starting Multi-Task Unit Tests...");
    TEST_LOG("-------------------------------");

#if TEST_CASE_BASIC_FUNC
    test_basic_functionality();
#endif

#if TEST_CASE_PARAM_CHECK
    test_param_check();
#endif

#if TEST_CASE_MAX_LIMIT
    test_max_limit();
#endif

#if TEST_CASE_PRIORITY
    test_priority_ordering();
#endif

#if TEST_CASE_OVERLOAD
    test_overload_protection();
#endif

    TEST_LOG("-------------------------------");
    TEST_LOG("All Tests Completed.");
    return 0;
}