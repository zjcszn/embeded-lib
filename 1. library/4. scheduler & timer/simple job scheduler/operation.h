//
// Created by zjcszn on 2024/12/28.
//

#ifndef __OPERATION_H__
#define __OPERATION_H__

#include <stddef.h>

typedef enum {
    OP_STATUS_OK,
    OP_STATUS_COMPLETE,
    OP_STATUS_ERROR,
    OP_STATUS_ABORT,
    OP_STATUS_PAUSE,
    OP_STATUS_IN_PROGRESS,
} op_status_t;

typedef enum {
    OP_COMMAND_USER1,
    OP_COMMAND_USER2,
    OP_COMMAND_END,
} op_command_t;

typedef enum {
    OP_CTRL_NONE,
    OP_CTRL_PAUSE,
    OP_CTRL_RESUME,
    OP_CTRL_ABORT,
} op_ctrl_t;

typedef struct {
    op_command_t command;
    void *args
} op_message_t;

typedef op_status_t (*op_start_cb_t) (op_message_t *msg);
typedef op_status_t (*op_process_cb_t) (op_message_t *msg, op_ctrl_t ctrl);
typedef op_status_t (*op_end_cb_t) (op_message_t *msg);


int operation_command_send_noblock(op_message_t *msg);
int operation_command_send(op_message_t *msg, size_t timeout);
int operation_ctrl_command_send(op_ctrl_t cmd) ;
int operation_thread_init(void);


#endif //__OPERATION_H__
