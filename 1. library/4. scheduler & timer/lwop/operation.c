// Copyright 2024 zjcszn. All rights reserved.
//
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file    operation.c
 * @brief   Job Scheduler / Operation Module Implementation
 * @author  zjcszn
 * @date    2024/12/28
 */

#include "operation.h"

#include <string.h>

// =============================================================================
// Configuration
// =============================================================================

// Command message queue depth.
// - Set to 1: Uses single-slot mode (no FIFO, lowest overhead).
// - Set to 2+: Uses FIFO mode (must be power of 2).
// NOTE: Single-slot mode only accepts one pending message at a time.
#define OP_MSG_QUEUE_SIZE     1U

// Control command FIFO depth (must be power of 2).
#define OP_CTRL_FIFO_SIZE     8U

// Operation thread stack size in bytes.
#define OP_THREAD_STACK_SIZE  4096U

// Polling interval for RTOS mode when no control command available (ms).
// This prevents busy-wait and yields CPU time to other tasks.
#define OP_POLL_INTERVAL_MS   10U

// =============================================================================
// Internal FIFO Implementation
// =============================================================================

// Compile-time check for power of 2.
#define FIFO_IS_POWER_OF_2(x) (((x) != 0U) && (((x) & ((x) - 1U)) == 0U))

// Auto-select message queue implementation.
// When OP_MSG_QUEUE_SIZE == 1, use single-slot mode (no power-of-2 constraint).
// When OP_MSG_QUEUE_SIZE > 1, use FIFO mode (requires power-of-2).
#if (OP_MSG_QUEUE_SIZE == 1U)
#define OP_MSG_USE_SINGLE_SLOT 1
#else
#define OP_MSG_USE_SINGLE_SLOT 0
#endif

// Compile-time validation.
#if !FIFO_IS_POWER_OF_2(OP_CTRL_FIFO_SIZE)
#error "OP_CTRL_FIFO_SIZE must be a power of 2"
#endif

// Only validate power-of-2 for FIFO mode.
#if (OP_MSG_USE_SINGLE_SLOT == 0) && !FIFO_IS_POWER_OF_2(OP_MSG_QUEUE_SIZE)
#error "OP_MSG_QUEUE_SIZE must be a power of 2 when using FIFO mode (size > 1)"
#endif

// Generic FIFO structure.
typedef struct {
    void *buf;             // Pointer to data buffer.
    uint32_t item_size;    // Size of each item in bytes.
    uint32_t capacity;     // Maximum number of items.
    uint32_t mask;         // Index mask (capacity - 1).
    volatile uint32_t rd;  // Read index.
    volatile uint32_t wr;  // Write index.
} op_fifo_t;

// Initialize a FIFO.
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

// Reset FIFO to empty state.
static inline void op_fifo_reset(op_fifo_t *f) {
    if (f != NULL) {
        f->rd = 0U;
        f->wr = 0U;
    }
}

// Check if FIFO is empty.
static inline bool op_fifo_is_empty(const op_fifo_t *f) {
    return (f->rd == f->wr);
}

// Check if FIFO is full.
static inline bool op_fifo_is_full(const op_fifo_t *f) {
    return ((f->wr - f->rd) >= f->capacity);
}

// Push an item into FIFO.
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

// Pop an item from FIFO.
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

// =============================================================================
// Module Context Structure
// =============================================================================

#if (OP_USE_RTOS == 0)
// Bare-metal state machine states.
typedef enum {
    OP_STATE_IDLE,      // Waiting for new message.
    OP_STATE_STARTING,  // About to call on_start.
    OP_STATE_RUNNING,   // Calling on_process each poll.
    OP_STATE_ENDING     // About to call on_end.
} op_state_t;
#endif

// Operation module context.
typedef struct {
    bool initialized;  // Initialization flag.

    // Control command FIFO.
    op_fifo_t ctrl_fifo;
    op_ctrl_t ctrl_buf[OP_CTRL_FIFO_SIZE];

    // Callback handlers.
    op_handler_t handlers[OP_CMD_MAX];

    // Currently executing message (shared by both RTOS and bare-metal modes).
    op_msg_t current_msg;

#if (OP_USE_RTOS != 0)
    // RTOS mode: message queue and thread.
    op_queue_t msg_queue;
    op_queue_static_t queue_static;
    uint8_t queue_storage[OP_MSG_QUEUE_SIZE * sizeof(op_msg_t)];
    op_task_t task_handle;
    op_task_static_t task_static;
    op_stack_t task_stack[OP_THREAD_STACK_SIZE / sizeof(op_stack_t)];
#else
    // Bare-metal mode: state machine.
#if (OP_MSG_USE_SINGLE_SLOT != 0)
    // Single-slot mode: simple pending flag instead of FIFO.
    op_msg_t pending_msg;       // Pending message slot.
    volatile bool msg_pending;  // True if pending_msg contains a valid message.
#else
    // FIFO mode: multiple message buffering.
    op_fifo_t msg_fifo;
    op_msg_t msg_buf[OP_MSG_QUEUE_SIZE];
#endif
    op_state_t state;         // Current state machine state.
    op_status_t last_status;  // Last operation status for completion callback.
#endif
} op_ctx_t;

// Module context instance.
static op_ctx_t g_ctx;

// =============================================================================
// Private Helper Functions (Code Reuse)
// =============================================================================

// Validate message parameters.
static inline op_err_t op_validate_msg(const op_msg_t *msg) {
    if (msg == NULL) {
        OP_LOGE("validate: null message");
        return OP_ERR_INVALID_ARG;
    }
    if (msg->cmd >= OP_CMD_MAX) {
        OP_LOGE("validate: invalid cmd %d", (int) msg->cmd);
        return OP_ERR_INVALID_CMD;
    }
    return OP_ERR_OK;
}

// Check if module is ready to send.
static inline op_err_t op_check_init(void) {
    if (!g_ctx.initialized) {
        OP_LOGE("check_init: not initialized");
        return OP_ERR_NOT_INIT;
    }
    return OP_ERR_OK;
}

// Common initialization logic.
static op_err_t op_init_common(void) {
    // Clear context (also clears handlers to NULL).
    (void) memset(&g_ctx, 0, sizeof(g_ctx));

    // Initialize control FIFO.
    if (!op_fifo_init(&g_ctx.ctrl_fifo, g_ctx.ctrl_buf, sizeof(op_ctrl_t), OP_CTRL_FIFO_SIZE)) {
        OP_LOGE("init: ctrl fifo init failed");
        return OP_ERR_INIT_FAIL;
    }

    return OP_ERR_OK;
}

// Invoke completion callback if provided.
static inline void op_notify_complete(const op_msg_t *msg, op_status_t status) {
    if ((msg != NULL) && (msg->on_complete != NULL)) {
        msg->on_complete(status, msg->arg);
    }
}

// Get handler for command (returns NULL if not registered).
static inline const op_handler_t *op_get_handler(op_cmd_t cmd) {
    if (cmd >= OP_CMD_MAX) {
        return NULL;
    }
    const op_handler_t *h = &g_ctx.handlers[cmd];
    if ((h->on_start == NULL) || (h->on_process == NULL) || (h->on_end == NULL)) {
        return NULL;
    }
    return h;
}

// =============================================================================
// RTOS Mode: Operation Execution
// =============================================================================
#if (OP_USE_RTOS != 0)

// Execute a single command with its callbacks (RTOS mode).
// Uses yield delay to avoid busy-wait.
static void op_execute(const op_msg_t *msg) {
    if (msg == NULL) {
        OP_LOGE("execute: null message");
        return;
    }

    const op_handler_t *h = op_get_handler(msg->cmd);
    if (h == NULL) {
        OP_LOGW("execute: cmd %d not registered", (int) msg->cmd);
        op_notify_complete(msg, OP_STATUS_ERROR);
        return;
    }

    OP_LOGI("execute: starting cmd %d", (int) msg->cmd);

    // Reset control FIFO before starting.
    op_ctrl_reset();

    // Start phase.
    op_status_t status = h->on_start(msg->arg);

    // Process phase - loop until completion.
    if (status == OP_STATUS_OK) {
        do {
            op_ctrl_t ctrl = op_ctrl_recv();
            // Yield CPU when no control command to avoid busy-wait.
            if (ctrl == OP_CTRL_NONE) {
                OP_DELAY_MS(OP_POLL_INTERVAL_MS);
            }
            status = h->on_process(msg->arg, ctrl);
        } while (status == OP_STATUS_BUSY);
    }

    // End phase - always called.
    (void) h->on_end(msg->arg);

    // Notify completion.
    op_notify_complete(msg, status);

    OP_LOGI("execute: cmd %d finished with status %d", (int) msg->cmd, (int) status);
}

#endif  // OP_USE_RTOS

// =============================================================================
// Public API - Control FIFO (Lock-free SPSC pattern)
// =============================================================================

void op_ctrl_reset(void) {
    // Reset is only called at operation start, safe without lock.
    op_fifo_reset(&g_ctx.ctrl_fifo);
    OP_MEMORY_BARRIER();
    OP_LOGD("ctrl_reset");
}

op_err_t op_ctrl_send(op_ctrl_t ctrl) {
    // Producer side (ISR or other context).
    // SPSC: only one producer, no lock needed.
    if (!op_fifo_push(&g_ctx.ctrl_fifo, &ctrl)) {
        OP_LOGW("ctrl_send: fifo full");
        return OP_ERR_FULL;
    }
    OP_MEMORY_BARRIER();  // Ensure write is visible to consumer.
    OP_LOGD("ctrl_send: ctrl=%d", (int) ctrl);
    return OP_ERR_OK;
}

op_ctrl_t op_ctrl_recv(void) {
    // Consumer side (operation thread/poll).
    // SPSC: only one consumer, no lock needed.
    op_ctrl_t ctrl = OP_CTRL_NONE;
    (void) op_fifo_pop(&g_ctx.ctrl_fifo, &ctrl);
    OP_MEMORY_BARRIER();  // Ensure read ordering.
    return ctrl;
}

// =============================================================================
// Public API - Callback Registration
// =============================================================================

op_err_t op_register(op_cmd_t cmd, const op_handler_t *handler) {
    if (cmd >= OP_CMD_MAX) {
        OP_LOGE("register: invalid cmd %d", (int) cmd);
        return OP_ERR_INVALID_CMD;
    }

    if (handler == NULL) {
        OP_LOGE("register: null handler");
        return OP_ERR_INVALID_ARG;
    }

    OP_ENTER_CRITICAL();
    g_ctx.handlers[cmd] = *handler;
    OP_EXIT_CRITICAL();

    OP_LOGD("register: cmd %d registered", (int) cmd);
    return OP_ERR_OK;
}

// =============================================================================
// Public API - Initialization
// =============================================================================

bool op_is_init(void) {
    return g_ctx.initialized;
}

// =============================================================================
// RTOS Mode Implementation
// =============================================================================
#if (OP_USE_RTOS != 0)

op_err_t op_send(const op_msg_t *msg) {
    op_err_t err = op_check_init();
    if (err != OP_ERR_OK) {
        return err;
    }

    err = op_validate_msg(msg);
    if (err != OP_ERR_OK) {
        return err;
    }

    if (OP_QUEUE_SEND(g_ctx.msg_queue, msg, 0U) != 0) {
        OP_LOGW("send: queue full");
        return OP_ERR_FULL;
    }

    OP_LOGD("send: cmd %d sent", (int) msg->cmd);
    return OP_ERR_OK;
}

op_err_t op_send_wait(const op_msg_t *msg, uint32_t timeout) {
    op_err_t err = op_check_init();
    if (err != OP_ERR_OK) {
        return err;
    }

    err = op_validate_msg(msg);
    if (err != OP_ERR_OK) {
        return err;
    }

    if (OP_QUEUE_SEND(g_ctx.msg_queue, msg, timeout) != 0) {
        OP_LOGW("send_wait: timeout");
        return OP_ERR_TIMEOUT;
    }

    OP_LOGD("send_wait: cmd %d sent", (int) msg->cmd);
    return OP_ERR_OK;
}

// Operation thread entry (RTOS mode).
static void op_thread(void *arg) {
    (void) arg;
    OP_LOGI("thread started");

    while (true) {
        if (OP_QUEUE_RECEIVE(g_ctx.msg_queue, &g_ctx.current_msg, OP_WAIT_FOREVER) == 0) {
            op_execute(&g_ctx.current_msg);
        }
    }
}

op_err_t op_init(void) {
    // Common initialization.
    op_err_t err = op_init_common();
    if (err != OP_ERR_OK) {
        return err;
    }

    // Create message queue.
    g_ctx.msg_queue = OP_QUEUE_CREATE_STATIC(OP_MSG_QUEUE_SIZE, sizeof(op_msg_t),
                                             &g_ctx.queue_storage[0], &g_ctx.queue_static);

    if (g_ctx.msg_queue == NULL) {
        OP_LOGE("init: queue create failed");
        return OP_ERR_INIT_FAIL;
    }

    // Create operation thread.
    g_ctx.task_handle =
        OP_TASK_CREATE_STATIC(op_thread, "op_task", OP_THREAD_STACK_SIZE / sizeof(op_stack_t), NULL,
                              OP_TASK_PRIORITY_MAX, &g_ctx.task_stack[0], &g_ctx.task_static);

    if (g_ctx.task_handle == NULL) {
        OP_LOGE("init: task create failed");
        return OP_ERR_INIT_FAIL;
    }

    g_ctx.initialized = true;
    OP_LOGI("initialized (RTOS mode)");

    return OP_ERR_OK;
}

// =============================================================================
// Bare-Metal Mode Implementation (Non-blocking State Machine)
// =============================================================================
#else

op_err_t op_send(const op_msg_t *msg) {
    op_err_t err = op_check_init();
    if (err != OP_ERR_OK) {
        return err;
    }

    err = op_validate_msg(msg);
    if (err != OP_ERR_OK) {
        return err;
    }

#if (OP_MSG_USE_SINGLE_SLOT != 0)
    // Single-slot mode: check if slot is available.
    if (g_ctx.msg_pending) {
        OP_LOGW("send: slot busy");
        return OP_ERR_FULL;
    }
    g_ctx.pending_msg = *msg;
    OP_MEMORY_BARRIER();
    g_ctx.msg_pending = true;
#else
    // FIFO mode: SPSC assumes single producer context.
    if (!op_fifo_push(&g_ctx.msg_fifo, msg)) {
        OP_LOGW("send: queue full");
        return OP_ERR_FULL;
    }
    OP_MEMORY_BARRIER();
#endif

    OP_LOGD("send: cmd %d sent", (int) msg->cmd);
    return OP_ERR_OK;
}

op_err_t op_send_wait(const op_msg_t *msg, uint32_t timeout) {
    // In bare-metal mode, timeout is not supported.
    (void) timeout;
    return op_send(msg);
}

// Non-blocking poll - executes one state transition per call.
void op_poll(void) {
    if (!g_ctx.initialized) {
        return;
    }

    switch (g_ctx.state) {
        case OP_STATE_IDLE: {
            // Try to get next message.
#if (OP_MSG_USE_SINGLE_SLOT != 0)
            // Single-slot mode: check pending flag.
            if (g_ctx.msg_pending) {
                g_ctx.current_msg = g_ctx.pending_msg;
                g_ctx.msg_pending = false;
                OP_MEMORY_BARRIER();
#else
            // FIFO mode: pop from queue.
            if (op_fifo_pop(&g_ctx.msg_fifo, &g_ctx.current_msg)) {
                OP_MEMORY_BARRIER();
#endif
                const op_handler_t *h = op_get_handler(g_ctx.current_msg.cmd);
                if (h == NULL) {
                    OP_LOGW("poll: cmd %d not registered", (int) g_ctx.current_msg.cmd);
                    op_notify_complete(&g_ctx.current_msg, OP_STATUS_ERROR);
                    break;  // Stay in IDLE.
                }

                OP_LOGI("poll: starting cmd %d", (int) g_ctx.current_msg.cmd);
                g_ctx.state = OP_STATE_STARTING;
            }
            break;
        }

        case OP_STATE_STARTING: {
            // Reset control FIFO and call on_start.
            op_ctrl_reset();
            const op_handler_t *h = op_get_handler(g_ctx.current_msg.cmd);
            g_ctx.last_status = h->on_start(g_ctx.current_msg.arg);

            if (g_ctx.last_status == OP_STATUS_OK) {
                g_ctx.state = OP_STATE_RUNNING;
            } else {
                // Start failed, go directly to end.
                OP_LOGW("poll: on_start failed with status %d", (int) g_ctx.last_status);
                g_ctx.state = OP_STATE_ENDING;
            }
            break;
        }

        case OP_STATE_RUNNING: {
            // Call on_process once per poll.
            const op_handler_t *h = op_get_handler(g_ctx.current_msg.cmd);
            op_ctrl_t ctrl = op_ctrl_recv();
            g_ctx.last_status = h->on_process(g_ctx.current_msg.arg, ctrl);

            if (g_ctx.last_status != OP_STATUS_BUSY) {
                // Operation finished (OK, ERROR, ABORT, etc.).
                OP_LOGI("poll: on_process finished with status %d", (int) g_ctx.last_status);
                g_ctx.state = OP_STATE_ENDING;
            }
            // If BUSY, stay in RUNNING for next poll.
            break;
        }

        case OP_STATE_ENDING: {
            // Call on_end and notify completion.
            const op_handler_t *h = op_get_handler(g_ctx.current_msg.cmd);
            (void) h->on_end(g_ctx.current_msg.arg);

            // Notify completion callback.
            op_notify_complete(&g_ctx.current_msg, g_ctx.last_status);

            OP_LOGI("poll: cmd %d finished", (int) g_ctx.current_msg.cmd);
            g_ctx.state = OP_STATE_IDLE;
            break;
        }

        default:
            // Should never happen, reset to idle.
            g_ctx.state = OP_STATE_IDLE;
            break;
    }
}

op_err_t op_init(void) {
    // Common initialization.
    op_err_t err = op_init_common();
    if (err != OP_ERR_OK) {
        return err;
    }

#if (OP_MSG_USE_SINGLE_SLOT != 0)
    // Single-slot mode: just clear pending flag.
    g_ctx.msg_pending = false;
    OP_LOGI("initialized (bare-metal mode, single-slot)");
#else
    // FIFO mode: initialize message FIFO.
    if (!op_fifo_init(&g_ctx.msg_fifo, g_ctx.msg_buf, sizeof(op_msg_t), OP_MSG_QUEUE_SIZE)) {
        OP_LOGE("init: msg fifo init failed");
        return OP_ERR_INIT_FAIL;
    }
    OP_LOGI("initialized (bare-metal mode, fifo depth=%u)", (unsigned) OP_MSG_QUEUE_SIZE);
#endif

    g_ctx.state = OP_STATE_IDLE;
    g_ctx.initialized = true;

    return OP_ERR_OK;
}

#endif  // OP_USE_RTOS
