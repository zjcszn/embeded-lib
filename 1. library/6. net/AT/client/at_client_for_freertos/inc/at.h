/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-30     chenyong     first version
 * 2018-08-17     chenyong     multiple client support
 */

#ifndef __AT_H__
#define __AT_H__

#include <stddef.h>
#include <stdarg.h>

#include "FreeRTOS.h"

#include "at_def.h"
#include "at_osal.h"
#include "at_fifo.h"
#include "at_utils.h"

#ifdef __cplusplus
extern "C" {
#endif


#define  AT_SW_VERSION                  "1.3.1"

#define  AT_CMD_NAME_LEN                16
#define  AT_END_MARK_LEN                4

#ifndef  AT_CMD_MAX_LEN
#define  AT_CMD_MAX_LEN                 128
#endif

/* the maximum number of supported AT clients */
#ifndef  AT_CLIENT_NUM_MAX
#define  AT_CLIENT_NUM_MAX              1
#endif

/* if define 1 enable use assert */
#define  AT_ASSERT_EN                   1

/* if define 1 enable output debug log*/
#define  AT_LOG_PRINT_EN                1

#define  AT_NAME_MAX                    16
#define  AT_THREAD_STK_SIZE             1024
#define  AT_THREAD_PRIORITY             configMAX_PRIORITIES / 3 - 1

#if      AT_LOG_PRINT_EN      ==        1
#include <stdio.h>
#define  AT_LOG_PRINT(...)              printf(__VA_ARGS__)
#else
#define  AT_LOG_PRINT(...)              ((void)0U)
#endif


#if      AT_ASSERT_EN        ==         1
#include <assert.h>
#define  AT_ASSERT(exp)                 assert(exp)
#else
#define  AT_ASSERT(exp)                 ((void)0U)
#endif

typedef fifo_queue_t                    at_fifo_t;

enum at_status
{
    AT_STATUS_NOINT = 0,
    AT_STATUS_READY,
    AT_STATUS_SUSPEND,
};
typedef enum at_status at_status_t;


enum at_resp_status
{
    AT_RESP_OK = 0,                   /* AT response end is OK */
    AT_RESP_ERROR = -1,               /* AT response end is ERROR */
    AT_RESP_TIMEOUT = -2,             /* AT response is timeout */
    AT_RESP_BUFF_FULL= -3,            /* AT response buffer is full */
};
typedef enum at_resp_status at_resp_status_t;

struct at_response
{
    /* response buffer */
    char *buf;
    /* the maximum response buffer size, it set by `at_create_resp()` function */
    uint32_t buf_size;
    /* the length of current response buffer */
    uint32_t buf_len;
    /* the number of setting response lines, it set by `at_create_resp()` function
     * == 0: the response data will auto return when received 'OK' or 'ERROR'
     * != 0: the response data will return when received setting lines number data */
    uint32_t line_num;
    /* the count of received response lines */
    uint32_t line_counts;
    /* the maximum response time */
    uint32_t timeout;
};

typedef struct at_response *at_response_t;

struct at_client;

/* URC(Unsolicited Result Code) object, such as: 'RING', 'READY' request by AT server */
struct at_urc
{
    const char *cmd_prefix;
    const char *cmd_suffix;
    void (*func)(struct at_client *client, const char *data, uint32_t size);
};
typedef struct at_urc *at_urc_t;

struct at_urc_table
{
    size_t urc_size;
    const struct at_urc *urc;
};
typedef struct at_urc_table *at_urc_table_t;

struct at_client
{
    at_status_t status;
    char end_sign;

    /* the current received one line data buffer */
    char *recv_line_buf;
    /* The length of the currently received one line data */
    uint32_t recv_line_len;
    /* The maximum supported receive data length */
    uint32_t recv_line_bufsz;
    rtos_sem_t  rx_notice;
    rtos_mutex_t lock;

    at_response_t resp;
    rtos_sem_t resp_notice;
    at_resp_status_t resp_status;

    struct at_urc_table *urc_table;
    uint32_t urc_table_size;

    rtos_thread_t parser;
    uint32_t uart_idx;
    at_fifo_t uart_fifo;
    char name[AT_NAME_MAX];
};
typedef struct at_client *at_client_t;

/* AT client initialize and start*/
int at_client_init(const char *dev_name, uint32_t uart_idx, uint32_t recv_line_bufsz, uint32_t uart_fifo_bufsz);

/* ========================== multiple AT client function ============================ */

/* get AT client object */
at_client_t at_client_get(const char *dev_name);
at_client_t at_client_get_first(void);

/* AT client wait for connection to external devices. */
int at_client_obj_wait_connect(at_client_t client, uint32_t timeout);

/* AT client send or receive data */
uint32_t at_client_obj_send(at_client_t client, const char *buf, uint32_t size);
uint32_t at_client_obj_recv(at_client_t client, char *buf, uint32_t size, uint32_t timeout);

/* set AT client a line end sign */
void at_obj_set_end_sign(at_client_t client, char ch);

/* Set URC(Unsolicited Result Code) table */
int at_obj_set_urc_table(at_client_t client, const struct at_urc * table, uint32_t size);

/* AT client send commands to AT server and waiter response */
int at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *cmd_expr, ...);

/* AT response object create and delete */
at_response_t at_create_resp(uint32_t buf_size, uint32_t line_num, uint32_t timeout);
void at_delete_resp(at_response_t resp);
at_response_t at_resp_set_info(at_response_t resp, uint32_t buf_size, uint32_t line_num, uint32_t timeout);

/* AT response line buffer get and parse response buffer arguments */
const char *at_resp_get_line(at_response_t resp, uint32_t resp_line);
const char *at_resp_get_line_by_kw(at_response_t resp, const char *keyword);
int at_resp_parse_line_args(at_response_t resp, uint32_t resp_line, const char *resp_expr, ...);
int at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...);

/* ========================== single AT client function ============================ */

/**
 * NOTE: These functions can be used directly when there is only one AT client.
 * If there are multiple AT Client in the program, these functions can operate on the first initialized AT client.
 */

#define at_exec_cmd(resp, ...)                   at_obj_exec_cmd(at_client_get_first(), resp, __VA_ARGS__)
#define at_client_wait_connect(timeout)          at_client_obj_wait_connect(at_client_get_first(), timeout)
#define at_client_send(buf, size)                at_client_obj_send(at_client_get_first(), buf, size)
#define at_client_recv(buf, size, timeout)       at_client_obj_recv(at_client_get_first(), buf, size, timeout)
#define at_set_end_sign(ch)                      at_obj_set_end_sign(at_client_get_first(), ch)
#define at_set_urc_table(urc_table, table_sz)    at_obj_set_urc_table(at_client_get_first(), urc_table, table_sz)



/* ========================== User port function ============================ */


#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */
