// Copyright 2024 zjcszn. All rights reserved.
//
// Operation Module Test Suite - Bare-metal FIFO Mode
// This test file tests the bare-metal mode with FIFO queue (OP_MSG_QUEUE_SIZE > 1)
//
// Build: gcc -Wall -Wextra -o operation_test_fifo.exe operation_test_fifo.c

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// Test Configuration - FIFO Mode (OP_MSG_QUEUE_SIZE > 1)
// =============================================================================

#define TEST_BUILD   1
#define OP_USE_RTOS  0
#define OP_LOG_LEVEL 0

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

// Logging stubs.
#define OP_LOGE(...)         ((void) 0)
#define OP_LOGW(...)         ((void) 0)
#define OP_LOGI(...)         ((void) 0)
#define OP_LOGD(...)         ((void) 0)

#define OP_PORT_H_

// Include operation.h for types.
#include "operation.h"

// =============================================================================
// Inline operation.c with modified OP_MSG_QUEUE_SIZE
// We copy the essential parts and modify the queue size.
// =============================================================================

// Configuration - FORCE FIFO MODE.
#define OP_MSG_QUEUE_SIZE      4U  // FIFO mode (power of 2, > 1)
#define OP_CTRL_FIFO_SIZE      8U
#define OP_THREAD_STACK_SIZE   4096U
#define OP_POLL_INTERVAL_MS    10U

// Internal FIFO Implementation.
#define FIFO_IS_POWER_OF_2(x)  (((x) != 0U) && (((x) & ((x) - 1U)) == 0U))

// FIFO mode enabled (not single-slot).
#define OP_MSG_USE_SINGLE_SLOT 0

// Generic FIFO structure.
typedef struct {
    void *buf;
    uint32_t item_size;
    uint32_t capacity;
    uint32_t mask;
    volatile uint32_t rd;
    volatile uint32_t wr;
} op_fifo_t;

static inline bool op_fifo_init(op_fifo_t *f, void *buf, uint32_t item_size, uint32_t capacity) {
    if ((f == NULL) || (buf == NULL) || (!FIFO_IS_POWER_OF_2(capacity))) {
        return false;
    }
    f->buf = buf;
    f->item_size = item_size;
    f->capacity = capacity;
    f->mask = capacity - 1U;
    f->rd = 0U;
    f->wr = 0U;
    return true;
}

static inline void op_fifo_reset(op_fifo_t *f) {
    if (f != NULL) {
        f->rd = 0U;
        f->wr = 0U;
    }
}

static inline bool op_fifo_is_empty(const op_fifo_t *f) {
    return (f->rd == f->wr);
}

static inline bool op_fifo_is_full(const op_fifo_t *f) {
    return ((f->wr - f->rd) >= f->capacity);
}

static inline bool op_fifo_push(op_fifo_t *f, const void *item) {
    if ((f == NULL) || (item == NULL) || op_fifo_is_full(f)) {
        return false;
    }
    uint32_t idx = f->wr & f->mask;
    uint8_t *dst = (uint8_t *) f->buf + (idx * f->item_size);
    (void) memcpy(dst, item, f->item_size);
    f->wr++;
    return true;
}

static inline bool op_fifo_pop(op_fifo_t *f, void *item) {
    if ((f == NULL) || op_fifo_is_empty(f)) {
        return false;
    }
    uint32_t idx = f->rd & f->mask;
    const uint8_t *src = (const uint8_t *) f->buf + (idx * f->item_size);
    if (item != NULL) {
        (void) memcpy(item, src, f->item_size);
    }
    f->rd++;
    return true;
}

// State machine states.
typedef enum { OP_STATE_IDLE, OP_STATE_STARTING, OP_STATE_RUNNING, OP_STATE_ENDING } op_state_t;

// Module context.
typedef struct {
    bool initialized;
    op_fifo_t ctrl_fifo;
    op_ctrl_t ctrl_buf[OP_CTRL_FIFO_SIZE];
    op_handler_t handlers[OP_CMD_MAX];
    op_msg_t current_msg;

    // FIFO mode.
    op_fifo_t msg_fifo;
    op_msg_t msg_buf[OP_MSG_QUEUE_SIZE];
    op_state_t state;
    op_status_t last_status;
} op_ctx_t;

static op_ctx_t g_ctx;

// Helper functions.
static inline op_err_t op_validate_msg(const op_msg_t *msg) {
    if (msg == NULL)
        return OP_ERR_INVALID_ARG;
    if (msg->cmd >= OP_CMD_MAX)
        return OP_ERR_INVALID_CMD;
    return OP_ERR_OK;
}

static inline op_err_t op_check_init(void) {
    if (!g_ctx.initialized)
        return OP_ERR_NOT_INIT;
    return OP_ERR_OK;
}

static inline void op_notify_complete(const op_msg_t *msg, op_status_t status) {
    if ((msg != NULL) && (msg->on_complete != NULL)) {
        msg->on_complete(status, msg->arg);
    }
}

static inline const op_handler_t *op_get_handler(op_cmd_t cmd) {
    if (cmd >= OP_CMD_MAX)
        return NULL;
    const op_handler_t *h = &g_ctx.handlers[cmd];
    if ((h->on_start == NULL) || (h->on_process == NULL) || (h->on_end == NULL)) {
        return NULL;
    }
    return h;
}

// Control FIFO API.
void op_ctrl_reset(void) {
    op_fifo_reset(&g_ctx.ctrl_fifo);
    OP_MEMORY_BARRIER();
}

op_err_t op_ctrl_send(op_ctrl_t ctrl) {
    if (!op_fifo_push(&g_ctx.ctrl_fifo, &ctrl)) {
        return OP_ERR_FULL;
    }
    OP_MEMORY_BARRIER();
    return OP_ERR_OK;
}

op_ctrl_t op_ctrl_recv(void) {
    op_ctrl_t ctrl = OP_CTRL_NONE;
    (void) op_fifo_pop(&g_ctx.ctrl_fifo, &ctrl);
    OP_MEMORY_BARRIER();
    return ctrl;
}

// Registration.
op_err_t op_register(op_cmd_t cmd, const op_handler_t *handler) {
    if (cmd >= OP_CMD_MAX)
        return OP_ERR_INVALID_CMD;
    if (handler == NULL)
        return OP_ERR_INVALID_ARG;
    OP_ENTER_CRITICAL();
    g_ctx.handlers[cmd] = *handler;
    OP_EXIT_CRITICAL();
    return OP_ERR_OK;
}

bool op_is_init(void) {
    return g_ctx.initialized;
}

// Send (FIFO mode).
op_err_t op_send(const op_msg_t *msg) {
    op_err_t err = op_check_init();
    if (err != OP_ERR_OK)
        return err;

    err = op_validate_msg(msg);
    if (err != OP_ERR_OK)
        return err;

    if (!op_fifo_push(&g_ctx.msg_fifo, msg)) {
        return OP_ERR_FULL;
    }
    OP_MEMORY_BARRIER();
    return OP_ERR_OK;
}

op_err_t op_send_wait(const op_msg_t *msg, uint32_t timeout) {
    (void) timeout;
    return op_send(msg);
}

// Poll (FIFO mode).
void op_poll(void) {
    if (!g_ctx.initialized)
        return;

    switch (g_ctx.state) {
        case OP_STATE_IDLE: {
            if (op_fifo_pop(&g_ctx.msg_fifo, &g_ctx.current_msg)) {
                OP_MEMORY_BARRIER();
                const op_handler_t *h = op_get_handler(g_ctx.current_msg.cmd);
                if (h == NULL) {
                    op_notify_complete(&g_ctx.current_msg, OP_STATUS_ERROR);
                    break;
                }
                g_ctx.state = OP_STATE_STARTING;
            }
            break;
        }

        case OP_STATE_STARTING: {
            op_ctrl_reset();
            const op_handler_t *h = op_get_handler(g_ctx.current_msg.cmd);
            g_ctx.last_status = h->on_start(g_ctx.current_msg.arg);
            if (g_ctx.last_status == OP_STATUS_OK) {
                g_ctx.state = OP_STATE_RUNNING;
            } else {
                g_ctx.state = OP_STATE_ENDING;
            }
            break;
        }

        case OP_STATE_RUNNING: {
            const op_handler_t *h = op_get_handler(g_ctx.current_msg.cmd);
            op_ctrl_t ctrl = op_ctrl_recv();
            g_ctx.last_status = h->on_process(g_ctx.current_msg.arg, ctrl);
            if (g_ctx.last_status != OP_STATUS_BUSY) {
                g_ctx.state = OP_STATE_ENDING;
            }
            break;
        }

        case OP_STATE_ENDING: {
            const op_handler_t *h = op_get_handler(g_ctx.current_msg.cmd);
            (void) h->on_end(g_ctx.current_msg.arg);
            op_notify_complete(&g_ctx.current_msg, g_ctx.last_status);
            g_ctx.state = OP_STATE_IDLE;
            break;
        }

        default:
            g_ctx.state = OP_STATE_IDLE;
            break;
    }
}

// Initialize.
op_err_t op_init(void) {
    (void) memset(&g_ctx, 0, sizeof(g_ctx));

    if (!op_fifo_init(&g_ctx.ctrl_fifo, g_ctx.ctrl_buf, sizeof(op_ctrl_t), OP_CTRL_FIFO_SIZE)) {
        return OP_ERR_INIT_FAIL;
    }

    if (!op_fifo_init(&g_ctx.msg_fifo, g_ctx.msg_buf, sizeof(op_msg_t), OP_MSG_QUEUE_SIZE)) {
        return OP_ERR_INIT_FAIL;
    }

    g_ctx.state = OP_STATE_IDLE;
    g_ctx.initialized = true;
    return OP_ERR_OK;
}

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
static int g_complete_count = 0;
static op_status_t g_complete_status = OP_STATUS_OK;
static void *g_complete_arg = NULL;

static void reset_counters(void) {
    g_start_count = 0;
    g_process_count = 0;
    g_end_count = 0;
    g_received_arg = NULL;
    g_complete_count = 0;
    g_complete_status = OP_STATUS_OK;
    g_complete_arg = NULL;
}

static op_status_t mock_start_ok(void *arg) {
    g_start_count++;
    g_received_arg = arg;
    return OP_STATUS_OK;
}

static op_status_t mock_process_done(void *arg, op_ctrl_t ctrl) {
    (void) arg;
    (void) ctrl;
    g_process_count++;
    return OP_STATUS_DONE;
}

static op_status_t mock_end(void *arg) {
    (void) arg;
    g_end_count++;
    return OP_STATUS_OK;
}

static void mock_on_complete(op_status_t status, void *arg) {
    g_complete_count++;
    g_complete_status = status;
    g_complete_arg = arg;
}

// Helper: run state machine until idle.
static void run_until_idle(void) {
    int max_iterations = 100;
    while (g_ctx.state != OP_STATE_IDLE && max_iterations-- > 0) {
        op_poll();
    }
    // One more poll to process any messages in IDLE state.
    op_poll();
    while (g_ctx.state != OP_STATE_IDLE && max_iterations-- > 0) {
        op_poll();
    }
}

// =============================================================================
// Test Cases - FIFO Mode Specific
// =============================================================================

TEST(fifo_mode_enabled) {
    ASSERT_EQ(0, OP_MSG_USE_SINGLE_SLOT);
    ASSERT_EQ(4, OP_MSG_QUEUE_SIZE);
    ASSERT_TRUE(FIFO_IS_POWER_OF_2(OP_MSG_QUEUE_SIZE));
}

TEST(fifo_init) {
    op_err_t err = op_init();
    ASSERT_EQ(OP_ERR_OK, err);
    ASSERT_TRUE(op_is_init());
}

TEST(fifo_send_multiple_messages) {
    (void) op_init();
    reset_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    // Send 4 messages (queue capacity).
    for (int i = 0; i < 4; i++) {
        op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = (void *) (intptr_t) (i + 1)};
        ASSERT_EQ(OP_ERR_OK, op_send(&msg));
    }

    // Fifth message should fail.
    op_msg_t msg5 = {.cmd = OP_CMD_USER1, .arg = (void *) 5};
    ASSERT_EQ(OP_ERR_FULL, op_send(&msg5));
}

TEST(fifo_execute_in_order) {
    (void) op_init();
    reset_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    // Send 3 messages with different args.
    int arg1 = 100, arg2 = 200, arg3 = 300;
    op_msg_t msg1 = {.cmd = OP_CMD_USER1, .arg = &arg1};
    op_msg_t msg2 = {.cmd = OP_CMD_USER1, .arg = &arg2};
    op_msg_t msg3 = {.cmd = OP_CMD_USER1, .arg = &arg3};

    (void) op_send(&msg1);
    (void) op_send(&msg2);
    (void) op_send(&msg3);

    // Execute first message.
    run_until_idle();
    ASSERT_EQ(1, g_start_count);
    ASSERT_PTR_EQ(&arg1, g_received_arg);

    // Execute second message.
    run_until_idle();
    ASSERT_EQ(2, g_start_count);
    ASSERT_PTR_EQ(&arg2, g_received_arg);

    // Execute third message.
    run_until_idle();
    ASSERT_EQ(3, g_start_count);
    ASSERT_PTR_EQ(&arg3, g_received_arg);
}

TEST(fifo_partial_fill_drain) {
    (void) op_init();
    reset_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    for (int cycle = 0; cycle < 3; cycle++) {
        op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
        ASSERT_EQ(OP_ERR_OK, op_send(&msg));
        ASSERT_EQ(OP_ERR_OK, op_send(&msg));

        run_until_idle();
        run_until_idle();
    }

    ASSERT_EQ(6, g_start_count);
    ASSERT_EQ(6, g_end_count);
}

TEST(fifo_wrap_around_stress) {
    (void) op_init();
    reset_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    for (int i = 0; i < 50; i++) {
        op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = (void *) (intptr_t) i};
        ASSERT_EQ(OP_ERR_OK, op_send(&msg));
        run_until_idle();
    }

    ASSERT_EQ(50, g_start_count);
    ASSERT_EQ(50, g_end_count);
}

TEST(fifo_with_completion_callbacks) {
    (void) op_init();
    reset_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    for (int i = 0; i < 3; i++) {
        op_msg_t msg = {.cmd = OP_CMD_USER1,
                        .arg = (void *) (intptr_t) (i + 1),
                        .on_complete = mock_on_complete};
        (void) op_send(&msg);
    }

    for (int i = 0; i < 3; i++) {
        run_until_idle();
    }

    ASSERT_EQ(3, g_complete_count);
}

TEST(fifo_queue_state_consistency) {
    (void) op_init();
    reset_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    // Fill queue.
    for (int i = 0; i < 4; i++) {
        op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
        ASSERT_EQ(OP_ERR_OK, op_send(&msg));
    }

    // Execute 2 messages.
    run_until_idle();
    run_until_idle();
    ASSERT_EQ(2, g_start_count);

    // Should be able to send 2 more.
    for (int i = 0; i < 2; i++) {
        op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
        ASSERT_EQ(OP_ERR_OK, op_send(&msg));
    }

    // Third send should fail.
    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL};
    ASSERT_EQ(OP_ERR_FULL, op_send(&msg));
}

// =============================================================================
// Main
// =============================================================================

int main(void) {
    printf("\n========================================\n");
    printf("Operation Module Test Suite - FIFO Mode\n");
    printf("  Mode: %s\n", OP_MSG_USE_SINGLE_SLOT ? "Single-Slot" : "FIFO");
    printf("  Queue Size: %u\n", (unsigned) OP_MSG_QUEUE_SIZE);
    printf("========================================\n\n");

    printf("[FIFO Mode Configuration]\n");
    RUN_TEST(fifo_mode_enabled);
    RUN_TEST(fifo_init);

    printf("\n[FIFO Send/Receive]\n");
    RUN_TEST(fifo_send_multiple_messages);
    RUN_TEST(fifo_execute_in_order);
    RUN_TEST(fifo_partial_fill_drain);
    RUN_TEST(fifo_wrap_around_stress);
    RUN_TEST(fifo_with_completion_callbacks);
    RUN_TEST(fifo_queue_state_consistency);

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed\n", g_test_passed, g_test_failed);
    printf("========================================\n\n");

    return (g_test_failed > 0) ? 1 : 0;
}
