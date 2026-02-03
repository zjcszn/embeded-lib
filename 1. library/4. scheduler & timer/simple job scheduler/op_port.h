// Copyright 2024 zjcszn. All rights reserved.
//
// Licensed under the MIT License. See LICENSE file for details.

/**
 * @file    op_port.h
 * @brief   Platform Abstraction Layer for Operation Module
 * @author  zjcszn
 * @date    2024/12/28
 */

#ifndef OP_PORT_H_
#define OP_PORT_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// =============================================================================
// Platform Configuration
// =============================================================================

// Platform selection: 1 for RTOS environment, 0 for bare-metal.
#ifndef OP_USE_RTOS
#define OP_USE_RTOS 1
#endif

// =============================================================================
// Debug Logging Configuration
// =============================================================================

#define OP_LOG_NONE  0U  // Logging disabled.
#define OP_LOG_ERROR 1U  // Error messages only.
#define OP_LOG_WARN  2U  // Warnings and errors.
#define OP_LOG_INFO  3U  // Info, warnings, and errors.
#define OP_LOG_DEBUG 4U  // All messages.

// Current log level (override at compile time with -DOP_LOG_LEVEL=x).
#ifndef OP_LOG_LEVEL
#define OP_LOG_LEVEL OP_LOG_NONE
#endif

// Log output function (user can override).
#ifndef OP_LOG_PRINTF
#include <stdio.h>
#define OP_LOG_PRINTF(fmt, ...) (void) printf(fmt, ##__VA_ARGS__)
#endif

// =============================================================================
// Variadic Macro Compatibility
// =============================================================================
// The ##__VA_ARGS__ extension (GNU) removes the trailing comma when __VA_ARGS__
// is empty, allowing OP_LOGE("message") without extra arguments.
// Clang warns about this with -Wgnu-zero-variadic-macro-arguments.
// We suppress this warning to maintain clean, intuitive API calls.

// Suppress Clang warning for GNU variadic macro extension.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

// Log implementation macros.
// Usage: OP_LOGE("simple message") or OP_LOGE("value=%d", val)
#if (OP_LOG_LEVEL >= OP_LOG_ERROR)
#define OP_LOGE(fmt, ...) OP_LOG_PRINTF("[OP][E] " fmt "\r\n", ##__VA_ARGS__)
#else
#define OP_LOGE(...) ((void) 0)
#endif

#if (OP_LOG_LEVEL >= OP_LOG_WARN)
#define OP_LOGW(fmt, ...) OP_LOG_PRINTF("[OP][W] " fmt "\r\n", ##__VA_ARGS__)
#else
#define OP_LOGW(...) ((void) 0)
#endif

#if (OP_LOG_LEVEL >= OP_LOG_INFO)
#define OP_LOGI(fmt, ...) OP_LOG_PRINTF("[OP][I] " fmt "\r\n", ##__VA_ARGS__)
#else
#define OP_LOGI(...) ((void) 0)
#endif

#if (OP_LOG_LEVEL >= OP_LOG_DEBUG)
#define OP_LOGD(fmt, ...) OP_LOG_PRINTF("[OP][D] " fmt "\r\n", ##__VA_ARGS__)
#else
#define OP_LOGD(...) ((void) 0)
#endif

// Restore Clang diagnostic settings.
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

// =============================================================================
// RTOS Environment (FreeRTOS)
// =============================================================================
#if (OP_USE_RTOS != 0)

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// Enter critical section.
#define OP_ENTER_CRITICAL() taskENTER_CRITICAL()

// Exit critical section.
#define OP_EXIT_CRITICAL()  taskEXIT_CRITICAL()

// Memory barrier for volatile access.
#define OP_MEMORY_BARRIER() __asm volatile("" ::: "memory")

// Task delay in milliseconds.
#define OP_DELAY_MS(ms)     vTaskDelay(pdMS_TO_TICKS(ms))

// Queue handle type.
typedef QueueHandle_t op_queue_t;

// Static queue structure.
typedef StaticQueue_t op_queue_static_t;

// Task handle type.
typedef TaskHandle_t op_task_t;

// Static task structure.
typedef StaticTask_t op_task_static_t;

// Stack element type.
typedef StackType_t op_stack_t;

// Create static queue.
#define OP_QUEUE_CREATE_STATIC(len, item_size, storage, queue_buf) \
    xQueueCreateStatic((len), (item_size), (storage), (queue_buf))

// Send to queue (returns 0 on success).
#define OP_QUEUE_SEND(queue, item, timeout) \
    ((xQueueSend((queue), (item), (timeout)) == pdTRUE) ? 0 : 1)

// Receive from queue (returns 0 on success).
#define OP_QUEUE_RECEIVE(queue, item, timeout) \
    ((xQueueReceive((queue), (item), (timeout)) == pdTRUE) ? 0 : 1)

// Infinite wait timeout.
#define OP_WAIT_FOREVER portMAX_DELAY

// Create static task.
#define OP_TASK_CREATE_STATIC(func, name, stack_depth, param, priority, stack, tcb) \
    xTaskCreateStatic((func), (name), (stack_depth), (param), (priority), (stack), (tcb))

// Maximum task priority.
#define OP_TASK_PRIORITY_MAX (configMAX_PRIORITIES - 1)

// =============================================================================
// Bare-Metal Environment
// =============================================================================
#else

// Enter critical section (user should override for their platform).
#ifndef OP_ENTER_CRITICAL
#define OP_ENTER_CRITICAL() \
    do {                    \
        __disable_irq();    \
    } while (false)
#endif

// Exit critical section (user should override for their platform).
#ifndef OP_EXIT_CRITICAL
#define OP_EXIT_CRITICAL() \
    do {                   \
        __enable_irq();    \
    } while (false)
#endif

// Memory barrier.
#ifndef OP_MEMORY_BARRIER
#define OP_MEMORY_BARRIER() __asm volatile("" ::: "memory")
#endif

// Delay in milliseconds (user should implement).
#ifndef OP_DELAY_MS
#define OP_DELAY_MS(ms)                      \
    do {                                     \
        volatile uint32_t _i = (ms) * 1000U; \
        while (_i > 0U) {                    \
            _i--;                            \
        }                                    \
    } while (false)
#endif

// Queue handle type (not used in bare-metal).
typedef void *op_queue_t;

// Static queue structure (not used in bare-metal).
typedef uint8_t op_queue_static_t;

// Task handle type (not used in bare-metal).
typedef void *op_task_t;

// Static task structure (not used in bare-metal).
typedef uint8_t op_task_static_t;

// Stack element type (not used in bare-metal).
typedef uint32_t op_stack_t;

// Create static queue (stub for bare-metal).
#define OP_QUEUE_CREATE_STATIC(len, item_size, storage, queue_buf) ((op_queue_t) (uintptr_t) 1U)

// Send to queue (stub for bare-metal).
#define OP_QUEUE_SEND(queue, item, timeout)                        (0)

// Receive from queue (stub for bare-metal).
#define OP_QUEUE_RECEIVE(queue, item, timeout)                     (1)

// Infinite wait timeout.
#define OP_WAIT_FOREVER                                            (0xFFFFFFFFUL)

// Create static task (stub for bare-metal).
#define OP_TASK_CREATE_STATIC(func, name, stack_depth, param, priority, stack, tcb) \
    ((op_task_t) (uintptr_t) 1U)

// Maximum task priority (not used in bare-metal).
#define OP_TASK_PRIORITY_MAX (0U)

#endif  // OP_USE_RTOS

#endif  // OP_PORT_H_
