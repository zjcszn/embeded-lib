//
// Created by zjcszn on 2024/12/28.
//

#include "operation.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define OPERATION_COMMAND_POOL_LENGTH       1
#define OPERATION_CTRL_POOL_LENGTH          8
#define OPERATION_THREAD_STACK_SIZE         4096

typedef struct {
    op_ctrl_t *buffer;
    volatile uint32_t r;
    volatile uint32_t w;
    uint32_t size;
    uint32_t mask;
} op_fifo_t;

static op_message_t op_message;
static QueueHandle_t op_command_mq;
static StaticQueue_t xCommandQueueBuffer;
static uint8_t ucCommandQueueStorage[OPERATION_COMMAND_POOL_LENGTH * sizeof(op_message_t)];

static op_ctrl_t op_ctrl_buffer[OPERATION_CTRL_POOL_LENGTH];
static op_fifo_t op_ctrl_fifo;

static TaskHandle_t op_thread_handler;
static StaticTask_t xOperationTCB;
static StackType_t ulOperationStackBuffer[OPERATION_THREAD_STACK_SIZE / sizeof(StackType_t)];

static op_start_cb_t op_start[OP_COMMAND_END] = {};
static op_process_cb_t op_process[OP_COMMAND_END] = {};
static op_end_cb_t op_end[OP_COMMAND_END] = {};

void operation_ctrl_fifo_reset(void) {
    op_ctrl_fifo.w = 0;
    op_ctrl_fifo.r = 0;
}

/**
 * @brief send operation command to message queue, no block
 * @param msg pointer of msg struct
 * @return 0 is send ok, otherwise is send failed
 */
int operation_command_send_noblock(op_message_t *msg) {
    if (xQueueSend(op_command_mq, msg, 0) != pdTRUE) {
        return 1;
    }

    return 0;
}

/**
 * @brief send operation command to message queue, by freertos queue
 * @param msg pointer of msg struct
 * @param timeout
 * @return 0 is send ok, otherwise is send failed
 */
int operation_command_send(op_message_t *msg, size_t timeout) {
    if (xQueueSend(op_command_mq, msg, timeout) != pdTRUE) {
        return 1;
    }

    return 0;
}

/**
 * @brief send control command to pause or abort operation
 * @param cmd OP_CTRL_XXX
 * @return 0 is send ok, otherwise is fifo full
 */
int operation_ctrl_command_send(op_ctrl_t cmd) {
    if (op_ctrl_fifo.size == (op_ctrl_fifo.w - op_ctrl_fifo.r)) {
        return 1;
    }

    op_ctrl_fifo.buffer[op_ctrl_fifo.w & op_ctrl_fifo.mask] = cmd;
    op_ctrl_fifo.w++;

    return 0;
}

/**
 * @brief get control command from fifo
 * @param pbuf
 * @return
 */
op_ctrl_t operation_ctrl_command_recv(void) {
    op_ctrl_t c;

    if (op_ctrl_fifo.r == op_ctrl_fifo.w) {
        return OP_CTRL_NONE;
    }

    c = op_ctrl_fifo.buffer[op_ctrl_fifo.r & op_ctrl_fifo.mask];
    op_ctrl_fifo.r++;

    return c;
}

static void operation_thread(void *args) {
    op_status_t ret;
    op_command_t cmd;
    op_ctrl_t ctrl;
    (void)args;

    while (1) {
        xQueueReceive(op_command_mq, &op_message, portMAX_DELAY);

        cmd = op_message.command;
        if ((cmd >= OP_COMMAND_END) || (!op_start[cmd]) || (!op_process[cmd]) || (!op_end[cmd])) {
            continue;
        }

        operation_ctrl_fifo_reset();
        ret = op_start[cmd](op_message.args);

        if (ret == OP_STATUS_OK) {
            do {
                ctrl = operation_ctrl_command_recv();

                if (op_process[cmd](op_message.args, ctrl) != OP_STATUS_IN_PROGRESS) {
                    break;
                }
            } while (1);
        }

        op_end[cmd](op_message.args);
    }
}

/**
 * @brief init operation thread
 * @return 0 init ok, otherwise is failed
 */
int operation_thread_init(void) {

    /* init control command fifo */
    op_ctrl_fifo.size = OPERATION_CTRL_POOL_LENGTH;
    op_ctrl_fifo.buffer = op_ctrl_buffer;
    op_ctrl_fifo.r = 0;
    op_ctrl_fifo.w = 0;
    op_ctrl_fifo.mask = OPERATION_CTRL_POOL_LENGTH - 1;

    /* init operation command message queue */
    op_command_mq = xQueueCreateStatic(OPERATION_COMMAND_POOL_LENGTH, sizeof(op_message_t), &ucCommandQueueStorage[0], &xCommandQueueBuffer);
    if (op_command_mq == NULL) {
        return 1;
    }

    /* create thread */
    op_thread_handler = xTaskCreateStatic(operation_thread,
                                          "op thread",
                                          OPERATION_THREAD_STACK_SIZE / sizeof(StackType_t),
                                          NULL,
                                          configMAX_PRIORITIES - 1,
                                          &ulOperationStackBuffer[0],
                                          &xOperationTCB);

    if (op_thread_handler == NULL) {
        return 1;
    }

    return 0;
}
