// Copyright 2024 zjcszn. All rights reserved.
//
// Operation Module Test Suite
// Build with CMake or compile directly:
//   gcc -Wall -Wextra -o operation_test.exe operation_test.c

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// Test Configuration - Define BEFORE including headers
// =============================================================================

#define TEST_BUILD   1
#define OP_USE_RTOS  0
#define OP_LOG_LEVEL 0  // Disable logging for tests.

// Mock critical section.
static int g_critical_count = 0;
#define OP_ENTER_CRITICAL() \
    do {                    \
        g_critical_count++; \
    } while (false)
#define OP_EXIT_CRITICAL()  \
    do {                    \
        g_critical_count--; \
    } while (false)
#define OP_MEMORY_BARRIER() ((void) 0)
#define OP_DELAY_MS(ms)     ((void) (ms))

// Mock platform types.
typedef void *op_queue_t;
typedef uint8_t op_queue_static_t;
typedef void *op_task_t;
typedef uint8_t op_task_static_t;
typedef uint32_t op_stack_t;

#define OP_QUEUE_CREATE_STATIC(len, item_size, storage, queue_buf) ((op_queue_t) (uintptr_t) 1U)
#define OP_QUEUE_SEND(queue, item, timeout)                        (0)
#define OP_QUEUE_RECEIVE(queue, item, timeout)                     (1)
#define OP_WAIT_FOREVER                                            (0xFFFFFFFFUL)
#define OP_TASK_CREATE_STATIC(func, name, stack_depth, param, priority, stack, tcb) \
    ((op_task_t) (uintptr_t) 1U)
#define OP_TASK_PRIORITY_MAX (0U)

// Logging stubs - suppress Clang variadic macro warning.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#define OP_LOGE(...) ((void) 0)
#define OP_LOGW(...) ((void) 0)
#define OP_LOGI(...) ((void) 0)
#define OP_LOGD(...) ((void) 0)

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

// Prevent inclusion of actual headers.
#define OP_PORT_H_

// Include the operation module (FIFO is now internal to operation.c).
#include "operation.c"
#include "operation.h"

// =============================================================================
// Test Framework
// =============================================================================

static int g_test_passed = 0;
static int g_test_failed = 0;

#define TEST(name) static void test_##name(void)

#define RUN_TEST(name)                      \
    do {                                    \
        printf("  Running: %s... ", #name); \
        test_##name();                      \
        printf("PASSED\n");                 \
        g_test_passed++;                    \
    } while (false)

#define ASSERT_EQ(expected, actual)                                                          \
    do {                                                                                     \
        if ((expected) != (actual)) {                                                        \
            printf("FAILED\n    Line %d: Expected %d, got %d\n", __LINE__, (int) (expected), \
                   (int) (actual));                                                          \
            g_test_failed++;                                                                 \
            return;                                                                          \
        }                                                                                    \
    } while (false)

#define ASSERT_TRUE(cond)                                             \
    do {                                                              \
        if (!(cond)) {                                                \
            printf("FAILED\n    Line %d: Expected true\n", __LINE__); \
            g_test_failed++;                                          \
            return;                                                   \
        }                                                             \
    } while (false)

#define ASSERT_FALSE(cond)                                             \
    do {                                                               \
        if (cond) {                                                    \
            printf("FAILED\n    Line %d: Expected false\n", __LINE__); \
            g_test_failed++;                                           \
            return;                                                    \
        }                                                              \
    } while (false)

#define ASSERT_PTR_EQ(expected, actual)                                  \
    do {                                                                 \
        if ((expected) != (actual)) {                                    \
            printf("FAILED\n    Line %d: Pointer mismatch\n", __LINE__); \
            g_test_failed++;                                             \
            return;                                                      \
        }                                                                \
    } while (false)

// =============================================================================
// Mock Callbacks
// =============================================================================

static int g_start_count = 0;
static int g_process_count = 0;
static int g_end_count = 0;
static void *g_received_arg = NULL;
static op_ctrl_t g_received_ctrl = OP_CTRL_NONE;
static int g_process_iterations = 0;

// Completion callback tracking.
static int g_complete_count = 0;
static op_status_t g_complete_status = OP_STATUS_OK;
static void *g_complete_arg = NULL;

static void reset_callback_counters(void) {
    g_start_count = 0;
    g_process_count = 0;
    g_end_count = 0;
    g_received_arg = NULL;
    g_received_ctrl = OP_CTRL_NONE;
    g_process_iterations = 0;
    g_complete_count = 0;
    g_complete_status = OP_STATUS_OK;
    g_complete_arg = NULL;
}

static op_status_t mock_start_ok(void *arg) {
    g_start_count++;
    g_received_arg = arg;
    return OP_STATUS_OK;
}

static op_status_t mock_start_error(void *arg) {
    (void) arg;
    g_start_count++;
    return OP_STATUS_ERROR;
}

static op_status_t mock_process_done(void *arg, op_ctrl_t ctrl) {
    (void) arg;
    g_process_count++;
    g_received_ctrl = ctrl;
    return OP_STATUS_DONE;
}

// Returns BUSY for (iterations-1) times, then returns DONE.
static int g_target_iterations = 3;

static op_status_t mock_process_iterate(void *arg, op_ctrl_t ctrl) {
    (void) arg;
    g_process_count++;
    g_received_ctrl = ctrl;
    g_process_iterations++;

    if (g_process_iterations >= g_target_iterations) {
        return OP_STATUS_DONE;
    }
    return OP_STATUS_BUSY;
}

static op_status_t mock_process_with_ctrl(void *arg, op_ctrl_t ctrl) {
    (void) arg;
    g_process_count++;
    g_received_ctrl = ctrl;

    switch (ctrl) {
        case OP_CTRL_ABORT:
            return OP_STATUS_ABORT;
        case OP_CTRL_PAUSE:
            return OP_STATUS_PAUSE;
        default:
            return OP_STATUS_BUSY;
    }
}

static op_status_t mock_end(void *arg) {
    (void) arg;
    g_end_count++;
    return OP_STATUS_OK;
}

// Completion callback mock.
static void mock_on_complete(op_status_t status, void *arg) {
    g_complete_count++;
    g_complete_status = status;
    g_complete_arg = arg;
}

// =============================================================================
// Test Cases - Initialization
// =============================================================================

TEST(init_success) {
    op_err_t err = op_init();
    ASSERT_EQ(OP_ERR_OK, err);
    ASSERT_TRUE(op_is_init());
}

TEST(init_ctrl_fifo_empty) {
    (void) op_init();
    op_ctrl_t ctrl = op_ctrl_recv();
    ASSERT_EQ(OP_CTRL_NONE, ctrl);
}

TEST(init_repeated) {
    // First init.
    ASSERT_EQ(OP_ERR_OK, op_init());
    ASSERT_TRUE(op_is_init());

    // Second init should also succeed (re-initialization).
    ASSERT_EQ(OP_ERR_OK, op_init());
    ASSERT_TRUE(op_is_init());
}

// =============================================================================
// Test Cases - Control FIFO
// =============================================================================

TEST(ctrl_send_recv) {
    (void) op_init();

    op_err_t err = op_ctrl_send(OP_CTRL_PAUSE);
    ASSERT_EQ(OP_ERR_OK, err);

    op_ctrl_t ctrl = op_ctrl_recv();
    ASSERT_EQ(OP_CTRL_PAUSE, ctrl);

    ctrl = op_ctrl_recv();
    ASSERT_EQ(OP_CTRL_NONE, ctrl);
}

TEST(ctrl_multiple) {
    (void) op_init();

    ASSERT_EQ(OP_ERR_OK, op_ctrl_send(OP_CTRL_PAUSE));
    ASSERT_EQ(OP_ERR_OK, op_ctrl_send(OP_CTRL_RESUME));
    ASSERT_EQ(OP_ERR_OK, op_ctrl_send(OP_CTRL_ABORT));

    ASSERT_EQ(OP_CTRL_PAUSE, op_ctrl_recv());
    ASSERT_EQ(OP_CTRL_RESUME, op_ctrl_recv());
    ASSERT_EQ(OP_CTRL_ABORT, op_ctrl_recv());
    ASSERT_EQ(OP_CTRL_NONE, op_ctrl_recv());
}

TEST(ctrl_fifo_full) {
    (void) op_init();

    // Fill FIFO (size = 8).
    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(OP_ERR_OK, op_ctrl_send(OP_CTRL_PAUSE));
    }

    // Next send should fail.
    ASSERT_EQ(OP_ERR_FULL, op_ctrl_send(OP_CTRL_ABORT));
}

TEST(ctrl_reset) {
    (void) op_init();

    (void) op_ctrl_send(OP_CTRL_PAUSE);
    (void) op_ctrl_send(OP_CTRL_RESUME);

    op_ctrl_reset();

    ASSERT_EQ(OP_CTRL_NONE, op_ctrl_recv());
}

TEST(ctrl_wrap_around) {
    (void) op_init();

    for (int round = 0; round < 5; round++) {
        for (int i = 0; i < 7; i++) {
            ASSERT_EQ(OP_ERR_OK, op_ctrl_send(OP_CTRL_PAUSE));
        }
        for (int i = 0; i < 7; i++) {
            ASSERT_EQ(OP_CTRL_PAUSE, op_ctrl_recv());
        }
    }
    ASSERT_EQ(OP_CTRL_NONE, op_ctrl_recv());
}

// =============================================================================
// Test Cases - Callback Registration
// =============================================================================

TEST(register_success) {
    (void) op_init();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};

    ASSERT_EQ(OP_ERR_OK, op_register(OP_CMD_USER1, &h));
}

TEST(register_invalid_cmd) {
    (void) op_init();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};

    ASSERT_EQ(OP_ERR_INVALID_CMD, op_register(OP_CMD_MAX, &h));
    ASSERT_EQ(OP_ERR_INVALID_CMD, op_register((op_cmd_t) 99, &h));
}

TEST(register_null_handler) {
    (void) op_init();
    ASSERT_EQ(OP_ERR_INVALID_ARG, op_register(OP_CMD_USER1, NULL));
}

TEST(register_overwrite) {
    (void) op_init();
    reset_callback_counters();

    op_handler_t h1 = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    op_handler_t h2 = {
        .on_start = mock_start_error, .on_process = mock_process_done, .on_end = mock_end};

    ASSERT_EQ(OP_ERR_OK, op_register(OP_CMD_USER1, &h1));
    ASSERT_EQ(OP_ERR_OK, op_register(OP_CMD_USER1, &h2));

    // Send and execute.
    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
    (void) op_send(&msg);

    // State machine: IDLE -> get msg -> STARTING.
    op_poll();  // IDLE -> STARTING.
    op_poll();  // STARTING -> call on_start (error) -> ENDING.
    op_poll();  // ENDING -> call on_end -> IDLE.

    // Should use h2 (mock_start_error).
    ASSERT_EQ(1, g_start_count);
    ASSERT_EQ(0, g_process_count);  // Start failed, skip process.
}

// =============================================================================
// Test Cases - Command Send (Single-Slot Mode)
// =============================================================================

TEST(send_success) {
    (void) op_init();

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
    ASSERT_EQ(OP_ERR_OK, op_send(&msg));
}

TEST(send_null) {
    (void) op_init();
    ASSERT_EQ(OP_ERR_INVALID_ARG, op_send(NULL));
}

TEST(send_invalid_cmd) {
    (void) op_init();

    op_msg_t msg = {.cmd = OP_CMD_MAX, .arg = NULL};
    ASSERT_EQ(OP_ERR_INVALID_CMD, op_send(&msg));
}

TEST(send_full_single_slot) {
    (void) op_init();

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};

    // First send should succeed.
    ASSERT_EQ(OP_ERR_OK, op_send(&msg));

#if (OP_MSG_USE_SINGLE_SLOT != 0)
    // In single-slot mode, second send should fail.
    ASSERT_EQ(OP_ERR_FULL, op_send(&msg));
#else
    // In FIFO mode with depth >= 2, second send may succeed.
    // Adjust based on actual OP_MSG_QUEUE_SIZE.
#endif
}

TEST(send_not_init) {
    // Reset context.
    (void) memset(&g_ctx, 0, sizeof(g_ctx));

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
    ASSERT_EQ(OP_ERR_NOT_INIT, op_send(&msg));
}

TEST(send_wait_bare_metal) {
    (void) op_init();

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};

    // In bare-metal mode, send_wait is equivalent to send.
    ASSERT_EQ(OP_ERR_OK, op_send_wait(&msg, 1000));
}

// =============================================================================
// Test Cases - Poll State Machine Execution
// =============================================================================

TEST(poll_no_cmd) {
    (void) op_init();
    reset_callback_counters();

    op_poll();

    ASSERT_EQ(0, g_start_count);
    ASSERT_EQ(0, g_process_count);
    ASSERT_EQ(0, g_end_count);
}

TEST(poll_state_machine_phases) {
    // Verify the state machine transitions correctly through all phases.
    (void) op_init();
    reset_callback_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    int test_arg = 42;
    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = &test_arg};
    (void) op_send(&msg);

    // Phase 1: IDLE -> fetch message -> transition to STARTING.
    ASSERT_EQ(OP_STATE_IDLE, g_ctx.state);
    op_poll();
    ASSERT_EQ(OP_STATE_STARTING, g_ctx.state);
    ASSERT_EQ(0, g_start_count);  // on_start not called yet.

    // Phase 2: STARTING -> call on_start -> transition to RUNNING.
    op_poll();
    ASSERT_EQ(OP_STATE_RUNNING, g_ctx.state);
    ASSERT_EQ(1, g_start_count);

    // Phase 3: RUNNING -> call on_process -> transition to ENDING.
    op_poll();
    ASSERT_EQ(OP_STATE_ENDING, g_ctx.state);
    ASSERT_EQ(1, g_process_count);

    // Phase 4: ENDING -> call on_end -> transition to IDLE.
    op_poll();
    ASSERT_EQ(OP_STATE_IDLE, g_ctx.state);
    ASSERT_EQ(1, g_end_count);

    ASSERT_PTR_EQ(&test_arg, g_received_arg);
}

TEST(poll_start_fails) {
    (void) op_init();
    reset_callback_counters();

    op_handler_t h = {
        .on_start = mock_start_error, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
    (void) op_send(&msg);

    // IDLE -> STARTING.
    op_poll();
    // STARTING -> on_start fails -> ENDING (skip RUNNING).
    op_poll();
    ASSERT_EQ(OP_STATE_ENDING, g_ctx.state);
    ASSERT_EQ(1, g_start_count);
    ASSERT_EQ(0, g_process_count);  // Skipped.

    // ENDING -> on_end -> IDLE.
    op_poll();
    ASSERT_EQ(OP_STATE_IDLE, g_ctx.state);
    ASSERT_EQ(1, g_end_count);
}

TEST(poll_iterate_multiple) {
    (void) op_init();
    reset_callback_counters();
    g_target_iterations = 3;

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_iterate, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
    (void) op_send(&msg);

    // IDLE -> STARTING.
    op_poll();
    // STARTING -> RUNNING.
    op_poll();
    ASSERT_EQ(1, g_start_count);

    // RUNNING: process returns BUSY (iteration 1).
    op_poll();
    ASSERT_EQ(1, g_process_count);
    ASSERT_EQ(OP_STATE_RUNNING, g_ctx.state);

    // RUNNING: process returns BUSY (iteration 2).
    op_poll();
    ASSERT_EQ(2, g_process_count);
    ASSERT_EQ(OP_STATE_RUNNING, g_ctx.state);

    // RUNNING: process returns DONE (iteration 3).
    op_poll();
    ASSERT_EQ(3, g_process_count);
    ASSERT_EQ(OP_STATE_ENDING, g_ctx.state);

    // ENDING -> IDLE.
    op_poll();
    ASSERT_EQ(1, g_end_count);
    ASSERT_EQ(OP_STATE_IDLE, g_ctx.state);
}

TEST(poll_unregistered) {
    (void) op_init();
    reset_callback_counters();

    op_msg_t msg = {.cmd = OP_CMD_USER2, .arg = NULL};
    (void) op_send(&msg);

    // IDLE -> get msg -> handler not registered -> stay IDLE.
    op_poll();

    ASSERT_EQ(0, g_start_count);
    ASSERT_EQ(0, g_process_count);
    ASSERT_EQ(0, g_end_count);
    ASSERT_EQ(OP_STATE_IDLE, g_ctx.state);
}

// =============================================================================
// Test Cases - Control Commands During Execution
// =============================================================================

TEST(poll_with_abort_ctrl) {
    (void) op_init();
    reset_callback_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_with_ctrl, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
    (void) op_send(&msg);

    // IDLE -> STARTING.
    op_poll();
    // STARTING -> RUNNING.
    op_poll();

    // Send abort control.
    (void) op_ctrl_send(OP_CTRL_ABORT);

    // RUNNING -> process receives ABORT -> returns ABORT -> ENDING.
    op_poll();
    ASSERT_EQ(OP_CTRL_ABORT, g_received_ctrl);
    ASSERT_EQ(OP_STATE_ENDING, g_ctx.state);

    // ENDING -> IDLE.
    op_poll();
    ASSERT_EQ(1, g_end_count);
}

TEST(poll_with_pause_ctrl) {
    (void) op_init();
    reset_callback_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_with_ctrl, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
    (void) op_send(&msg);

    op_poll();  // IDLE -> STARTING.
    op_poll();  // STARTING -> RUNNING.

    (void) op_ctrl_send(OP_CTRL_PAUSE);
    op_poll();  // RUNNING -> process receives PAUSE.

    ASSERT_EQ(OP_CTRL_PAUSE, g_received_ctrl);
}

// =============================================================================
// Test Cases - Completion Callback
// =============================================================================

TEST(complete_callback_success) {
    (void) op_init();
    reset_callback_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    int test_arg = 123;
    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = &test_arg, .on_complete = mock_on_complete};
    (void) op_send(&msg);

    // Run through state machine.
    op_poll();  // IDLE -> STARTING.
    op_poll();  // STARTING -> RUNNING.
    op_poll();  // RUNNING -> ENDING.
    op_poll();  // ENDING -> IDLE (callback called here).

    ASSERT_EQ(1, g_complete_count);
    ASSERT_EQ(OP_STATUS_DONE, g_complete_status);
    ASSERT_PTR_EQ(&test_arg, g_complete_arg);
}

TEST(complete_callback_on_error) {
    (void) op_init();
    reset_callback_counters();

    op_handler_t h = {
        .on_start = mock_start_error, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    int test_arg = 456;
    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = &test_arg, .on_complete = mock_on_complete};
    (void) op_send(&msg);

    op_poll();  // IDLE -> STARTING.
    op_poll();  // STARTING (fails) -> ENDING.
    op_poll();  // ENDING -> IDLE.

    ASSERT_EQ(1, g_complete_count);
    ASSERT_EQ(OP_STATUS_ERROR, g_complete_status);
}

TEST(complete_callback_null) {
    (void) op_init();
    reset_callback_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    op_msg_t msg = {
        .cmd = OP_CMD_USER1,
        .arg = NULL,
        .on_complete = NULL  // No callback.
    };
    (void) op_send(&msg);

    op_poll();
    op_poll();
    op_poll();
    op_poll();

    // Should not crash, callback count should be 0.
    ASSERT_EQ(0, g_complete_count);
}

TEST(complete_callback_unregistered_cmd) {
    (void) op_init();
    reset_callback_counters();

    // Don't register handler for USER2.
    int test_arg = 789;
    op_msg_t msg = {.cmd = OP_CMD_USER2, .arg = &test_arg, .on_complete = mock_on_complete};
    (void) op_send(&msg);

    op_poll();  // Unregistered -> callback with ERROR.

    ASSERT_EQ(1, g_complete_count);
    ASSERT_EQ(OP_STATUS_ERROR, g_complete_status);
    ASSERT_PTR_EQ(&test_arg, g_complete_arg);
}

// =============================================================================
// Test Cases - Critical Section Balance
// =============================================================================

TEST(critical_balanced) {
    (void) op_init();

    g_critical_count = 0;

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    (void) op_ctrl_send(OP_CTRL_PAUSE);
    (void) op_ctrl_recv();
    op_ctrl_reset();

    ASSERT_EQ(0, g_critical_count);
}

// =============================================================================
// Test Cases - FIFO Module Internal Tests
// =============================================================================

TEST(fifo_basic) {
    op_fifo_t f;
    uint32_t buf[4];
    uint32_t val;

    ASSERT_TRUE(op_fifo_init(&f, buf, sizeof(uint32_t), 4));
    ASSERT_TRUE(op_fifo_is_empty(&f));
    ASSERT_FALSE(op_fifo_is_full(&f));

    val = 100U;
    ASSERT_TRUE(op_fifo_push(&f, &val));
    ASSERT_FALSE(op_fifo_is_empty(&f));

    ASSERT_TRUE(op_fifo_pop(&f, &val));
    ASSERT_EQ(100, (int) val);
    ASSERT_TRUE(op_fifo_is_empty(&f));
}

TEST(fifo_full_cycle) {
    op_fifo_t f;
    uint32_t buf[4];
    uint32_t val;

    ASSERT_TRUE(op_fifo_init(&f, buf, sizeof(uint32_t), 4));

    // Fill FIFO.
    for (uint32_t i = 0; i < 4; i++) {
        val = i * 10;
        ASSERT_TRUE(op_fifo_push(&f, &val));
    }
    ASSERT_TRUE(op_fifo_is_full(&f));

    // Next push should fail.
    val = 999;
    ASSERT_FALSE(op_fifo_push(&f, &val));

    // Pop all.
    for (uint32_t i = 0; i < 4; i++) {
        ASSERT_TRUE(op_fifo_pop(&f, &val));
        ASSERT_EQ((int) (i * 10), (int) val);
    }
    ASSERT_TRUE(op_fifo_is_empty(&f));
}

TEST(fifo_invalid_capacity) {
    op_fifo_t f;
    uint32_t buf[3];

    // 3 is not power of 2.
    ASSERT_FALSE(op_fifo_init(&f, buf, sizeof(uint32_t), 3));
}

TEST(fifo_null_params) {
    op_fifo_t f;
    uint32_t buf[4];

    ASSERT_FALSE(op_fifo_init(NULL, buf, sizeof(uint32_t), 4));
    ASSERT_FALSE(op_fifo_init(&f, NULL, sizeof(uint32_t), 4));
}

TEST(fifo_wrap_around) {
    op_fifo_t f;
    uint32_t buf[4];
    uint32_t val;

    ASSERT_TRUE(op_fifo_init(&f, buf, sizeof(uint32_t), 4));

    // Multiple rounds of push/pop to test wrap-around.
    for (int round = 0; round < 10; round++) {
        for (uint32_t i = 0; i < 3; i++) {
            val = (uint32_t) round * 100 + i;
            ASSERT_TRUE(op_fifo_push(&f, &val));
        }
        for (uint32_t i = 0; i < 3; i++) {
            ASSERT_TRUE(op_fifo_pop(&f, &val));
            ASSERT_EQ((int) ((uint32_t) round * 100 + i), (int) val);
        }
    }
}

// =============================================================================
// Test Cases - Multiple Commands
// =============================================================================

TEST(multiple_cmds_sequential) {
    (void) op_init();
    reset_callback_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);
    (void) op_register(OP_CMD_USER2, &h);

    // First command.
    op_msg_t msg1 = {.cmd = OP_CMD_USER1, .arg = NULL};
    (void) op_send(&msg1);

    op_poll();  // IDLE -> STARTING.
    op_poll();  // STARTING -> RUNNING.
    op_poll();  // RUNNING -> ENDING.
    op_poll();  // ENDING -> IDLE.

    ASSERT_EQ(1, g_start_count);
    ASSERT_EQ(1, g_end_count);

    // Second command.
    op_msg_t msg2 = {.cmd = OP_CMD_USER2, .arg = NULL};
    (void) op_send(&msg2);

    op_poll();
    op_poll();
    op_poll();
    op_poll();

    ASSERT_EQ(2, g_start_count);
    ASSERT_EQ(2, g_process_count);
    ASSERT_EQ(2, g_end_count);
}

// =============================================================================
// Test Cases - Configuration Verification
// =============================================================================

TEST(config_poll_interval) {
    // Verify OP_POLL_INTERVAL_MS is defined and has expected value.
    ASSERT_EQ(10, OP_POLL_INTERVAL_MS);
}

TEST(config_ctrl_fifo_size) {
    // Verify OP_CTRL_FIFO_SIZE is power of 2.
    ASSERT_TRUE(FIFO_IS_POWER_OF_2(OP_CTRL_FIFO_SIZE));
    ASSERT_EQ(8, OP_CTRL_FIFO_SIZE);
}

TEST(config_msg_queue_mode) {
#if (OP_MSG_QUEUE_SIZE == 1U)
    ASSERT_EQ(1, OP_MSG_USE_SINGLE_SLOT);
#else
    ASSERT_EQ(0, OP_MSG_USE_SINGLE_SLOT);
    ASSERT_TRUE(FIFO_IS_POWER_OF_2(OP_MSG_QUEUE_SIZE));
#endif
}

// =============================================================================
// Main Test Runner
// =============================================================================

int main(void) {
    printf("\n========================================\n");
    printf("Operation Module Test Suite v3\n");
    printf("  Mode: %s\n", OP_MSG_USE_SINGLE_SLOT ? "Single-Slot" : "FIFO");
    printf("  Queue Size: %u\n", (unsigned) OP_MSG_QUEUE_SIZE);
    printf("  Ctrl FIFO Size: %u\n", (unsigned) OP_CTRL_FIFO_SIZE);
    printf("  Poll Interval: %u ms\n", (unsigned) OP_POLL_INTERVAL_MS);
    printf("========================================\n\n");

    printf("[Initialization]\n");
    RUN_TEST(init_success);
    RUN_TEST(init_ctrl_fifo_empty);
    RUN_TEST(init_repeated);

    printf("\n[Control FIFO]\n");
    RUN_TEST(ctrl_send_recv);
    RUN_TEST(ctrl_multiple);
    RUN_TEST(ctrl_fifo_full);
    RUN_TEST(ctrl_reset);
    RUN_TEST(ctrl_wrap_around);

    printf("\n[Callback Registration]\n");
    RUN_TEST(register_success);
    RUN_TEST(register_invalid_cmd);
    RUN_TEST(register_null_handler);
    RUN_TEST(register_overwrite);

    printf("\n[Command Send]\n");
    RUN_TEST(send_success);
    RUN_TEST(send_null);
    RUN_TEST(send_invalid_cmd);
    RUN_TEST(send_full_single_slot);
    RUN_TEST(send_not_init);
    RUN_TEST(send_wait_bare_metal);

    printf("\n[Poll State Machine]\n");
    RUN_TEST(poll_no_cmd);
    RUN_TEST(poll_state_machine_phases);
    RUN_TEST(poll_start_fails);
    RUN_TEST(poll_iterate_multiple);
    RUN_TEST(poll_unregistered);

    printf("\n[Control Commands]\n");
    RUN_TEST(poll_with_abort_ctrl);
    RUN_TEST(poll_with_pause_ctrl);

    printf("\n[Completion Callback]\n");
    RUN_TEST(complete_callback_success);
    RUN_TEST(complete_callback_on_error);
    RUN_TEST(complete_callback_null);
    RUN_TEST(complete_callback_unregistered_cmd);

    printf("\n[Critical Section]\n");
    RUN_TEST(critical_balanced);

    printf("\n[FIFO Module]\n");
    RUN_TEST(fifo_basic);
    RUN_TEST(fifo_full_cycle);
    RUN_TEST(fifo_invalid_capacity);
    RUN_TEST(fifo_null_params);
    RUN_TEST(fifo_wrap_around);

    printf("\n[Integration]\n");
    RUN_TEST(multiple_cmds_sequential);

    printf("\n[Configuration]\n");
    RUN_TEST(config_poll_interval);
    RUN_TEST(config_ctrl_fifo_size);
    RUN_TEST(config_msg_queue_mode);

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed\n", g_test_passed, g_test_failed);
    printf("========================================\n\n");

    return (g_test_failed > 0) ? 1 : 0;
}
