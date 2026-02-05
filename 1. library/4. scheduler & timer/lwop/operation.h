// Copyright 2024 zjcszn. All rights reserved.
//
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file    operation.h
 * @brief   Job Scheduler / Operation Module
 * @author  zjcszn
 * @date    2024/12/28
 *
 * @details This module provides a simple job scheduling mechanism with:
 *          - Command-based operation execution
 *          - Start/Process/End callback pattern
 *          - Control commands (pause/resume/abort)
 *          - Support for both RTOS and bare-metal environments
 */

#ifndef OPERATION_H_
#define OPERATION_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "op_port.h"

// =============================================================================
// Type Definitions
// =============================================================================

// Operation status codes (returned by callbacks).
typedef enum {
    OP_STATUS_OK = 0,  // Operation started successfully.
    OP_STATUS_DONE,    // Operation completed.
    OP_STATUS_ERROR,   // Operation error occurred.
    OP_STATUS_ABORT,   // Operation was aborted.
    OP_STATUS_PAUSE,   // Operation is paused.
    OP_STATUS_BUSY     // Operation still in progress.
} op_status_t;

// Error codes for API functions.
typedef enum {
    OP_ERR_OK = 0,       // No error.
    OP_ERR_FULL,         // Queue/FIFO is full.
    OP_ERR_TIMEOUT,      // Operation timed out.
    OP_ERR_INVALID_CMD,  // Invalid command ID.
    OP_ERR_INVALID_ARG,  // Invalid argument (NULL pointer, etc.).
    OP_ERR_NOT_INIT,     // Module not initialized.
    OP_ERR_INIT_FAIL     // Initialization failed.
} op_err_t;

// Command identifiers. Extend this enum for additional commands.
// OP_CMD_MAX must always be the last entry.
typedef enum {
    OP_CMD_USER1 = 0,  // User-defined command 1.
    OP_CMD_USER2,      // User-defined command 2.
    OP_CMD_MAX         // Maximum command count (not a valid command).
} op_cmd_t;

// Control commands for runtime operation control.
typedef enum {
    OP_CTRL_NONE = 0,  // No control command.
    OP_CTRL_PAUSE,     // Pause operation.
    OP_CTRL_RESUME,    // Resume operation.
    OP_CTRL_ABORT      // Abort operation.
} op_ctrl_t;

// Completion callback type.
// Called when operation finishes with final status and user argument.
typedef void (*op_complete_fn_t)(op_status_t status, void *arg);

// Operation message structure.
typedef struct {
    op_cmd_t cmd;                  // Command to execute.
    void *arg;                     // Command argument (user-defined).
    op_complete_fn_t on_complete;  // Optional completion callback (can be NULL).
} op_msg_t;

// Callback function types.
typedef op_status_t (*op_start_fn_t)(void *arg);
typedef op_status_t (*op_process_fn_t)(void *arg, op_ctrl_t ctrl);
typedef op_status_t (*op_end_fn_t)(void *arg);

// Callback registration structure.
typedef struct {
    op_start_fn_t on_start;      // Called once when operation starts.
    op_process_fn_t on_process;  // Called repeatedly during operation.
    op_end_fn_t on_end;          // Called once when operation ends.
} op_handler_t;

// =============================================================================
// Public API
// =============================================================================

// Initialize the operation module.
// Returns OP_ERR_OK on success, error code otherwise.
// Must be called before any other API function.
op_err_t op_init(void);

// Check if module is initialized.
bool op_is_init(void);

// Register callbacks for a command.
// cmd: Command ID (must be < OP_CMD_MAX).
// handler: Pointer to handler structure.
// Returns OP_ERR_OK on success, error code otherwise.
op_err_t op_register(op_cmd_t cmd, const op_handler_t *handler);

// Send a command (non-blocking).
// msg: Pointer to message structure.
// Returns OP_ERR_OK on success, OP_ERR_FULL if queue is full.
op_err_t op_send(const op_msg_t *msg);

// Send a command with timeout (RTOS mode only).
// msg: Pointer to message structure.
// timeout: Timeout in ticks.
// Returns OP_ERR_OK on success, OP_ERR_TIMEOUT on timeout.
op_err_t op_send_wait(const op_msg_t *msg, uint32_t timeout);

// Send a control command (pause/resume/abort).
// ctrl: Control command.
// Returns OP_ERR_OK on success, OP_ERR_FULL if FIFO is full.
op_err_t op_ctrl_send(op_ctrl_t ctrl);

// Receive a control command (called by process callback).
// Returns control command, OP_CTRL_NONE if FIFO is empty.
op_ctrl_t op_ctrl_recv(void);

// Reset control command FIFO.
void op_ctrl_reset(void);

#if (OP_USE_RTOS == 0)
// Poll for and execute pending commands (bare-metal mode only).
// Call this periodically from the main loop in bare-metal mode.
void op_poll(void);
#endif

#endif  // OPERATION_H_
