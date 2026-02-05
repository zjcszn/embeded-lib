// Copyright 2024 zjcszn. All rights reserved.
//
// Operation Module Test Suite - RTOS Mode Simulation
// Uses POSIX threads to simulate FreeRTOS on PC
//
// Build (Linux/macOS): gcc -Wall -Wextra -pthread -o operation_test_rtos operation_test_rtos.c
// Build (Windows/MinGW): gcc -Wall -Wextra -o operation_test_rtos.exe operation_test_rtos.c
// -lpthread
//
// Note: Requires pthreads support. On Windows, use MinGW with pthreads-w32 or MSYS2.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// POSIX Thread Compatibility
// =============================================================================

#ifdef _WIN32
#include <windows.h>
// Use Windows native threading for better compatibility.
#define USE_WIN32_THREADS 1
#else
#include <pthread.h>
#include <unistd.h>
#define USE_WIN32_THREADS 0
#endif

// =============================================================================
// Test Configuration - RTOS Mode
// =============================================================================

#define TEST_BUILD   1
#define OP_USE_RTOS  1
#define OP_LOG_LEVEL 0

// =============================================================================
// RTOS Simulation Layer using POSIX/Windows Threads
// =============================================================================

#if USE_WIN32_THREADS

// Windows critical section for thread safety.
static CRITICAL_SECTION g_cs;
static volatile bool g_cs_initialized = false;

#define OP_ENTER_CRITICAL()                   \
    do {                                      \
        if (!g_cs_initialized) {              \
            InitializeCriticalSection(&g_cs); \
            g_cs_initialized = true;          \
        }                                     \
        EnterCriticalSection(&g_cs);          \
    } while (false)
#define OP_EXIT_CRITICAL()  LeaveCriticalSection(&g_cs)
#define OP_MEMORY_BARRIER() MemoryBarrier()
#define OP_DELAY_MS(ms)     Sleep(ms)

// Windows Thread Types.
typedef HANDLE op_task_t;
typedef uint8_t op_task_static_t;  // Not used in simulation.
typedef uint32_t op_stack_t;

// Message Queue simulation using Windows Events and critical section.
typedef struct {
    void *buffer;
    size_t item_size;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    CRITICAL_SECTION lock;
    HANDLE not_empty;  // Event signaled when queue is not empty.
    HANDLE not_full;   // Event signaled when queue is not full.
} sim_queue_t;

typedef sim_queue_t *op_queue_t;
typedef uint8_t op_queue_static_t;  // Not used.

static sim_queue_t g_msg_queue_storage;

static op_queue_t sim_queue_create(size_t capacity, size_t item_size, void *storage) {
    (void) storage;
    sim_queue_t *q = &g_msg_queue_storage;
    q->buffer = malloc(capacity * item_size);
    q->item_size = item_size;
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    InitializeCriticalSection(&q->lock);
    q->not_empty = CreateEvent(NULL, FALSE, FALSE, NULL);
    q->not_full = CreateEvent(NULL, FALSE, TRUE, NULL);
    return q;
}

static int sim_queue_send(op_queue_t q, const void *item, uint32_t timeout) {
    (void) timeout;
    EnterCriticalSection(&q->lock);
    if (q->count >= q->capacity) {
        LeaveCriticalSection(&q->lock);
        return -1;  // Full.
    }
    memcpy((char *) q->buffer + q->tail * q->item_size, item, q->item_size);
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    SetEvent(q->not_empty);
    LeaveCriticalSection(&q->lock);
    return 0;
}

static int sim_queue_receive(op_queue_t q, void *item, uint32_t timeout) {
    DWORD wait_time = (timeout == 0xFFFFFFFFUL) ? INFINITE : timeout;

    while (1) {
        EnterCriticalSection(&q->lock);
        if (q->count > 0) {
            memcpy(item, (char *) q->buffer + q->head * q->item_size, q->item_size);
            q->head = (q->head + 1) % q->capacity;
            q->count--;
            SetEvent(q->not_full);
            LeaveCriticalSection(&q->lock);
            return 0;
        }
        LeaveCriticalSection(&q->lock);

        if (WaitForSingleObject(q->not_empty, wait_time) == WAIT_TIMEOUT) {
            return -1;  // Timeout.
        }
    }
}

#define OP_QUEUE_CREATE_STATIC(len, item_size, storage, queue_buf) \
    sim_queue_create(len, item_size, storage)
#define OP_QUEUE_SEND(queue, item, timeout)    sim_queue_send(queue, item, timeout)
#define OP_QUEUE_RECEIVE(queue, item, timeout) sim_queue_receive(queue, item, timeout)

// Task creation.
typedef void (*task_func_t)(void *);
static DWORD WINAPI thread_wrapper(LPVOID arg) {
    task_func_t func = (task_func_t) arg;
    func(NULL);
    return 0;
}

static op_task_t sim_task_create(void (*func)(void *), const char *name, size_t stack_depth,
                                 void *param, uint32_t priority, void *stack, void *tcb) {
    (void) name;
    (void) stack_depth;
    (void) param;
    (void) priority;
    (void) stack;
    (void) tcb;
    HANDLE thread = CreateThread(NULL, 0, thread_wrapper, (LPVOID) func, 0, NULL);
    return thread;
}

#define OP_TASK_CREATE_STATIC(func, name, stack_depth, param, priority, stack, tcb) \
    sim_task_create(func, name, stack_depth, param, priority, stack, tcb)

#else  // POSIX (Linux/macOS)

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
#define OP_ENTER_CRITICAL() pthread_mutex_lock(&g_mutex)
#define OP_EXIT_CRITICAL()  pthread_mutex_unlock(&g_mutex)
#define OP_MEMORY_BARRIER() __sync_synchronize()
#define OP_DELAY_MS(ms)     usleep((ms) * 1000)

typedef pthread_t op_task_t;
typedef uint8_t op_task_static_t;
typedef uint32_t op_stack_t;

// Message Queue simulation using mutex + condition variable.
typedef struct {
    void *buffer;
    size_t item_size;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} sim_queue_t;

typedef sim_queue_t *op_queue_t;
typedef uint8_t op_queue_static_t;

static sim_queue_t g_msg_queue_storage;

static op_queue_t sim_queue_create(size_t capacity, size_t item_size, void *storage) {
    (void) storage;
    sim_queue_t *q = &g_msg_queue_storage;
    q->buffer = malloc(capacity * item_size);
    q->item_size = item_size;
    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
    return q;
}

static int sim_queue_send(op_queue_t q, const void *item, uint32_t timeout) {
    (void) timeout;
    pthread_mutex_lock(&q->lock);
    if (q->count >= q->capacity) {
        pthread_mutex_unlock(&q->lock);
        return -1;
    }
    memcpy((char *) q->buffer + q->tail * q->item_size, item, q->item_size);
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

static int sim_queue_receive(op_queue_t q, void *item, uint32_t timeout) {
    pthread_mutex_lock(&q->lock);
    while (q->count == 0) {
        if (timeout == 0xFFFFFFFFUL) {
            pthread_cond_wait(&q->not_empty, &q->lock);
        } else {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout / 1000;
            ts.tv_nsec += (timeout % 1000) * 1000000;
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }
            int ret = pthread_cond_timedwait(&q->not_empty, &q->lock, &ts);
            if (ret != 0) {
                pthread_mutex_unlock(&q->lock);
                return -1;  // Timeout.
            }
        }
    }
    memcpy(item, (char *) q->buffer + q->head * q->item_size, q->item_size);
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

#define OP_QUEUE_CREATE_STATIC(len, item_size, storage, queue_buf) \
    sim_queue_create(len, item_size, storage)
#define OP_QUEUE_SEND(queue, item, timeout)    sim_queue_send(queue, item, timeout)
#define OP_QUEUE_RECEIVE(queue, item, timeout) sim_queue_receive(queue, item, timeout)

static void *thread_wrapper(void *arg) {
    void (*func)(void *) = (void (*)(void *)) arg;
    func(NULL);
    return NULL;
}

static op_task_t sim_task_create(void (*func)(void *), const char *name, size_t stack_depth,
                                 void *param, uint32_t priority, void *stack, void *tcb) {
    (void) name;
    (void) stack_depth;
    (void) param;
    (void) priority;
    (void) stack;
    (void) tcb;
    pthread_t thread;
    pthread_create(&thread, NULL, thread_wrapper, (void *) func);
    return thread;
}

#define OP_TASK_CREATE_STATIC(func, name, stack_depth, param, priority, stack, tcb) \
    sim_task_create(func, name, stack_depth, param, priority, stack, tcb)

#endif  // USE_WIN32_THREADS

#define OP_WAIT_FOREVER      (0xFFFFFFFFUL)
#define OP_TASK_PRIORITY_MAX (0U)

// Logging stubs.
#define OP_LOGE(...)         ((void) 0)
#define OP_LOGW(...)         ((void) 0)
#define OP_LOGI(...)         ((void) 0)
#define OP_LOGD(...)         ((void) 0)

#define OP_PORT_H_

// Include operation module.
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
        fflush(stdout);                     \
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

// =============================================================================
// Mock Callbacks with Thread Synchronization
// =============================================================================

#if USE_WIN32_THREADS
static CRITICAL_SECTION g_callback_cs;
static HANDLE g_complete_event;
static volatile bool g_callback_cs_init = false;

static void init_callback_sync(void) {
    if (!g_callback_cs_init) {
        InitializeCriticalSection(&g_callback_cs);
        g_complete_event = CreateEvent(NULL, TRUE, FALSE, NULL);  // Manual reset.
        g_callback_cs_init = true;
    }
}

#define CALLBACK_LOCK()           EnterCriticalSection(&g_callback_cs)
#define CALLBACK_UNLOCK()         LeaveCriticalSection(&g_callback_cs)
#define WAIT_COMPLETE(timeout_ms) WaitForSingleObject(g_complete_event, timeout_ms)
#define SIGNAL_COMPLETE()         SetEvent(g_complete_event)
#define RESET_COMPLETE()          ResetEvent(g_complete_event)
#define DELAY_MS(ms)              Sleep(ms)

#else

static pthread_mutex_t g_callback_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_complete_cond = PTHREAD_COND_INITIALIZER;
static volatile bool g_complete_signaled = false;

static void init_callback_sync(void) {
    // Already initialized statically.
}

#define CALLBACK_LOCK()           pthread_mutex_lock(&g_callback_mutex)
#define CALLBACK_UNLOCK()         pthread_mutex_unlock(&g_callback_mutex)

static int wait_complete_posix(int timeout_ms) {
    pthread_mutex_lock(&g_callback_mutex);
    if (!g_complete_signaled) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        pthread_cond_timedwait(&g_complete_cond, &g_callback_mutex, &ts);
    }
    int result = g_complete_signaled ? 0 : -1;
    pthread_mutex_unlock(&g_callback_mutex);
    return result;
}

#define WAIT_COMPLETE(timeout_ms) wait_complete_posix(timeout_ms)
#define SIGNAL_COMPLETE()                        \
    do {                                         \
        pthread_mutex_lock(&g_callback_mutex);   \
        g_complete_signaled = true;              \
        pthread_cond_signal(&g_complete_cond);   \
        pthread_mutex_unlock(&g_callback_mutex); \
    } while (0)
#define RESET_COMPLETE()                         \
    do {                                         \
        pthread_mutex_lock(&g_callback_mutex);   \
        g_complete_signaled = false;             \
        pthread_mutex_unlock(&g_callback_mutex); \
    } while (0)
#define DELAY_MS(ms) usleep((ms) * 1000)

#endif

// Callback state.
static volatile int g_start_count = 0;
static volatile int g_process_count = 0;
static volatile int g_end_count = 0;
static volatile int g_complete_count = 0;
static volatile op_status_t g_complete_status = OP_STATUS_OK;
static volatile void *g_complete_arg = NULL;

static void reset_counters(void) {
    CALLBACK_LOCK();
    g_start_count = 0;
    g_process_count = 0;
    g_end_count = 0;
    g_complete_count = 0;
    g_complete_status = OP_STATUS_OK;
    g_complete_arg = NULL;
    CALLBACK_UNLOCK();
    RESET_COMPLETE();
}

static op_status_t mock_start_ok(void *arg) {
    CALLBACK_LOCK();
    g_start_count++;
    CALLBACK_UNLOCK();
    (void) arg;
    return OP_STATUS_OK;
}

static op_status_t mock_process_done(void *arg, op_ctrl_t ctrl) {
    CALLBACK_LOCK();
    g_process_count++;
    CALLBACK_UNLOCK();
    (void) arg;
    (void) ctrl;
    return OP_STATUS_DONE;
}

// Simulate work with multiple process calls.
static volatile int g_process_target = 3;

static op_status_t mock_process_iterate(void *arg, op_ctrl_t ctrl) {
    CALLBACK_LOCK();
    g_process_count++;
    int count = g_process_count;
    CALLBACK_UNLOCK();
    (void) arg;
    (void) ctrl;

    if (count >= g_process_target) {
        return OP_STATUS_DONE;
    }
    return OP_STATUS_BUSY;
}

static op_status_t mock_end(void *arg) {
    CALLBACK_LOCK();
    g_end_count++;
    CALLBACK_UNLOCK();
    (void) arg;
    return OP_STATUS_OK;
}

static void mock_on_complete(op_status_t status, void *arg) {
    CALLBACK_LOCK();
    g_complete_count++;
    g_complete_status = status;
    g_complete_arg = arg;
    CALLBACK_UNLOCK();
    SIGNAL_COMPLETE();
}

// =============================================================================
// Test Cases - RTOS Mode
// =============================================================================

TEST(rtos_mode_enabled) {
    ASSERT_EQ(1, OP_USE_RTOS);
}

TEST(rtos_init) {
    op_err_t err = op_init();
    ASSERT_EQ(OP_ERR_OK, err);
    ASSERT_TRUE(op_is_init());

    // Give thread time to start.
    DELAY_MS(50);
}

TEST(rtos_send_and_execute) {
    reset_counters();

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    int test_arg = 42;
    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = &test_arg, .on_complete = mock_on_complete};

    ASSERT_EQ(OP_ERR_OK, op_send(&msg));

    // Wait for completion with timeout.
    WAIT_COMPLETE(1000);

    CALLBACK_LOCK();
    ASSERT_EQ(1, g_start_count);
    ASSERT_EQ(1, g_process_count);
    ASSERT_EQ(1, g_end_count);
    ASSERT_EQ(1, g_complete_count);
    ASSERT_EQ(OP_STATUS_DONE, g_complete_status);
    CALLBACK_UNLOCK();
}

TEST(rtos_multiple_iterations) {
    reset_counters();
    g_process_target = 5;

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_iterate, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL, .on_complete = mock_on_complete};

    ASSERT_EQ(OP_ERR_OK, op_send(&msg));
    WAIT_COMPLETE(2000);

    CALLBACK_LOCK();
    ASSERT_EQ(1, g_start_count);
    ASSERT_TRUE(g_process_count >= 5);  // At least 5 iterations.
    ASSERT_EQ(1, g_end_count);
    CALLBACK_UNLOCK();
}

TEST(rtos_ctrl_abort) {
    reset_counters();
    g_process_target = 1000;  // Would run forever without abort.

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_iterate, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    op_msg_t msg = {.cmd = OP_CMD_USER1, .arg = NULL, .on_complete = mock_on_complete};

    ASSERT_EQ(OP_ERR_OK, op_send(&msg));

    // Wait a bit for execution to start.
    DELAY_MS(100);

    // Send abort.
    // Note: In real RTOS mode, the process callback would check for abort.
    // Our mock doesn't handle abort, so this is just testing the send.
    ASSERT_EQ(OP_ERR_OK, op_ctrl_send(OP_CTRL_ABORT));
}

TEST(rtos_sequential_commands) {
    reset_counters();
    g_process_target = 1;

    op_handler_t h = {
        .on_start = mock_start_ok, .on_process = mock_process_done, .on_end = mock_end};
    (void) op_register(OP_CMD_USER1, &h);

    // Send 3 commands sequentially.
    for (int i = 0; i < 3; i++) {
        reset_counters();

        op_msg_t msg = {
            .cmd = OP_CMD_USER1, .arg = (void *) (intptr_t) i, .on_complete = mock_on_complete};

        ASSERT_EQ(OP_ERR_OK, op_send(&msg));
        WAIT_COMPLETE(1000);

        CALLBACK_LOCK();
        ASSERT_EQ(1, g_complete_count);
        CALLBACK_UNLOCK();
    }
}

// =============================================================================
// Main
// =============================================================================

int main(void) {
    printf("\n========================================\n");
    printf("Operation Module Test Suite - RTOS Mode\n");
    printf("  Platform: %s\n", USE_WIN32_THREADS ? "Windows" : "POSIX");
    printf("  OP_USE_RTOS: %d\n", OP_USE_RTOS);
    printf("========================================\n\n");

    init_callback_sync();

    printf("[RTOS Mode Configuration]\n");
    RUN_TEST(rtos_mode_enabled);

    printf("\n[RTOS Initialization]\n");
    RUN_TEST(rtos_init);

    printf("\n[RTOS Execution]\n");
    RUN_TEST(rtos_send_and_execute);
    RUN_TEST(rtos_multiple_iterations);
    RUN_TEST(rtos_ctrl_abort);
    RUN_TEST(rtos_sequential_commands);

    printf("\n========================================\n");
    printf("Results: %d passed, %d failed\n", g_test_passed, g_test_failed);
    printf("========================================\n\n");

    // Note: In a real test, we should clean up the thread.
    // For simplicity, we just exit.

    return (g_test_failed > 0) ? 1 : 0;
}
