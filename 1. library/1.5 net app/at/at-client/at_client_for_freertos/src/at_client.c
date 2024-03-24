/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-03-30     chenyong     first version
 * 2018-04-12     chenyong     add client implement
 * 2018-08-17     chenyong     multiple client support
 * 2021-03-17     Meco Man     fix a buf of leaking memory
 * 2021-07-14     Sszl         fix a buf of leaking memory
 * 2023-03-02     zzjcszn      porting to freertos
 */

#include "at.h"

#include <stdlib.h>
#include <string.h>


#define AT_RESP_END_OK                 "OK"
#define AT_RESP_END_ERROR              "ERROR"
#define AT_RESP_END_FAIL               "FAIL"
#define AT_END_CR_LF                   "\r\n"

static struct at_client at_client_table[AT_CLIENT_NUM_MAX] = { 0 };


/**
 * Create response object.
 *
 * @param buf_size the maximum response buffer size
 * @param line_num the number of setting response lines
 *         = 0: the response data will auto return when received 'OK' or 'ERROR'
 *        != 0: the response data will return when received setting lines number data
 * @param timeout the maximum response time
 *
 * @return != NULL: response object
 *          = NULL: no memory
 */
at_response_t at_create_resp(uint32_t buf_size, uint32_t line_num, uint32_t timeout)
{
    at_response_t resp = NULL;

    resp = (at_response_t) rtos_calloc(1, sizeof(struct at_response));
    if (resp == NULL)
    {
        AT_LOG_PRINT("AT create response object failed! No memory for response object!");
        return NULL;
    }

    resp->buf = (char *) rtos_calloc(1, buf_size);
    if (resp->buf == NULL)
    {
        AT_LOG_PRINT("AT create response object failed! No memory for response buffer!");
        rtos_free(resp);
        return NULL;
    }

    resp->buf_size = buf_size;
    resp->line_num = line_num;
    resp->line_counts = 0;
    resp->timeout = timeout;

    return resp;
}

/**
 * Delete and free response object.
 *
 * @param resp response object
 */
void at_delete_resp(at_response_t resp)
{
    if (resp && resp->buf)
    {
        rtos_free(resp->buf);
    }

    if (resp)
    {
        rtos_free(resp);
        resp = NULL;
    }
}

/**
 * Set response object information
 *
 * @param resp response object
 * @param buf_size the maximum response buffer size
 * @param line_num the number of setting response lines
 *         = 0: the response data will auto return when received 'OK' or 'ERROR'
 *        != 0: the response data will return when received setting lines number data
 * @param timeout the maximum response time
 *
 * @return  != NULL: response object
 *           = NULL: no memory
 */
at_response_t at_resp_set_info(at_response_t resp, uint32_t buf_size, uint32_t line_num, uint32_t timeout)
{
    char *p_temp;
    AT_ASSERT(resp);

    if (resp->buf_size != buf_size)
    {
        resp->buf_size = buf_size;

        p_temp = (char *) rtos_realloc(resp->buf, buf_size);
        if (p_temp == NULL)
        {
            AT_LOG_PRINT("No memory for realloc response buffer size(%d).", buf_size);
            return NULL;
        }
        else
        {
            resp->buf = p_temp;
        }
    }

    resp->line_num = line_num;
    resp->timeout = timeout;

    return resp;
}

/**
 * Get one line AT response buffer by line number.
 *
 * @param resp response object
 * @param resp_line line number, start from '1'
 *
 * @return != NULL: response line buffer
 *          = NULL: input response line error
 */
const char *at_resp_get_line(at_response_t resp, uint32_t resp_line)
{
    char *resp_buf = resp->buf;
    char *resp_line_buf = NULL;
    uint32_t line_num = 1;

    AT_ASSERT(resp);

    if (resp_line > resp->line_counts || resp_line <= 0)
    {
        AT_LOG_PRINT("AT response get line failed! Input response line(%d) error!", resp_line);
        return NULL;
    }

    for (line_num = 1; line_num <= resp->line_counts; line_num++)
    {
        if (resp_line == line_num)
        {
            resp_line_buf = resp_buf;

            return resp_line_buf;
        }

        resp_buf += strlen(resp_buf) + 1;
    }

    return NULL;
}

/**
 * Get one line AT response buffer by keyword
 *
 * @param resp response object
 * @param keyword query keyword
 *
 * @return != NULL: response line buffer
 *          = NULL: no matching data
 */
const char *at_resp_get_line_by_kw(at_response_t resp, const char *keyword)
{
    char *resp_buf = resp->buf;
    char *resp_line_buf = NULL;
    uint32_t line_num = 1;

    AT_ASSERT(resp);
    AT_ASSERT(keyword);

    for (line_num = 1; line_num <= resp->line_counts; line_num++)
    {
        if (strstr(resp_buf, keyword))
        {
            resp_line_buf = resp_buf;

            return resp_line_buf;
        }

        resp_buf += strlen(resp_buf) + 1;
    }

    return NULL;
}

/**
 * Get and parse AT response buffer arguments by line number.
 *
 * @param resp response object
 * @param resp_line line number, start from '1'
 * @param resp_expr response buffer expression
 *
 * @return -1 : input response line number error or get line buffer error
 *          0 : parsed without match
 *         >0 : the number of arguments successfully parsed
 */
int at_resp_parse_line_args(at_response_t resp, uint32_t resp_line, const char *resp_expr, ...)
{
    va_list args;
    int resp_args_num = 0;
    const char *resp_line_buf = NULL;

    AT_ASSERT(resp);
    AT_ASSERT(resp_expr);

    if ((resp_line_buf = at_resp_get_line(resp, resp_line)) == NULL)
    {
        return -1;
    }

    va_start(args, resp_expr);

    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;
}

/**
 * Get and parse AT response buffer arguments by keyword.
 *
 * @param resp response object
 * @param keyword query keyword
 * @param resp_expr response buffer expression
 *
 * @return -1 : input keyword error or get line buffer error
 *          0 : parsed without match
 *         >0 : the number of arguments successfully parsed
 */
int at_resp_parse_line_args_by_kw(at_response_t resp, const char *keyword, const char *resp_expr, ...)
{
    va_list args;
    int resp_args_num = 0;
    const char *resp_line_buf = NULL;

    AT_ASSERT(resp);
    AT_ASSERT(resp_expr);

    if ((resp_line_buf = at_resp_get_line_by_kw(resp, keyword)) == NULL)
    {
        return -1;
    }

    va_start(args, resp_expr);

    resp_args_num = vsscanf(resp_line_buf, resp_expr, args);

    va_end(args);

    return resp_args_num;
}

/**
 * Send commands to AT server and wait response.
 *
 * @param client current AT client object
 * @param resp AT response object, using NULL when you don't care response
 * @param cmd_expr AT commands expression
 *
 * @return 0 : success
 *        -1 : response status error
 *        -2 : wait timeout
 *        -7 : enter AT CLI mode
 */
int at_obj_exec_cmd(at_client_t client, at_response_t resp, const char *cmd_expr, ...)
{
    va_list args;
    uint32_t cmd_size = 0;
    int result = AT_EOK;
    const char *cmd = NULL;

    AT_ASSERT(cmd_expr);

    if (client == NULL)
    {
        AT_LOG_PRINT("input AT Client object is NULL, please create or get AT Client object!");
        return -AT_ERROR;
    }

    /* check AT device status */
    if (client->status != AT_STATUS_READY)
    {
        return -AT_EBUSY;
    }

    rtos_mutex_take(client->lock, RTOS_WAITING_FOREVER, RTOS_FROM_THREAD);

    client->resp_status = AT_RESP_OK;

    if (resp != NULL)
    {
        resp->buf_len = 0;
        resp->line_counts = 0;
    }

    client->resp = resp;

    /* send data */
    va_start(args, cmd_expr);
    at_vprintfln(client->uart_idx, cmd_expr, args);
    va_end(args);

    if (resp != NULL)
    {
        if (rtos_sem_take(client->resp_notice, resp->timeout, RTOS_FROM_THREAD) != AT_EOK)
        {
            cmd = at_get_last_cmd(&cmd_size);
            AT_LOG_PRINT("execute command (%.*s) timeout (%d ticks)!", cmd_size, cmd, resp->timeout);
            client->resp_status = AT_RESP_TIMEOUT;
            result = -AT_ETIMEOUT;
            goto __exit;
        }
        if (client->resp_status != AT_RESP_OK)
        {
            cmd = at_get_last_cmd(&cmd_size);
            AT_LOG_PRINT("execute command (%.*s) failed!", cmd_size, cmd);
            result = -AT_ERROR;
            goto __exit;
        }
    }

__exit:
    client->resp = NULL;

    rtos_mutex_give(client->lock, RTOS_FROM_THREAD);

    return result;
}

/**
 * Waiting for connection to external devices.
 *
 * @param client current AT client object
 * @param timeout millisecond for timeout
 *
 * @return 0 : success
 *        -2 : timeout
 *        -5 : no memory
 */
int at_client_obj_wait_connect(at_client_t client, uint32_t timeout)
{
    int result = AT_EOK;
    at_response_t resp = NULL;
    uint32_t start_time = 0;
    char *client_name = client->name;

    if (client == NULL)
    {
        AT_LOG_PRINT("input AT client object is NULL, please create or get AT Client object!");
        return -AT_ERROR;
    }

    /* check AT device status */
    if (client->status != AT_STATUS_READY)
    {
        return -AT_EBUSY;
    }

    resp = at_create_resp(64, 0, 300);
    if (resp == NULL)
    {
        AT_LOG_PRINT("no memory for AT client(%s) response object.", client_name);
        return -AT_ENOMEM;
    }

    rtos_mutex_take(client->lock, RTOS_WAITING_FOREVER, RTOS_FROM_THREAD);
    client->resp = resp;

    start_time = rtos_tick_get();

    while (1)
    {
        /* Check whether it is timeout */
        
        if (rtos_tick_get() - start_time > rtos_tick_from_ms(timeout))
        {
            AT_LOG_PRINT("wait AT client(%s) connect timeout(%d tick).", client_name, timeout);
            result = -AT_ETIMEOUT;
            break;
        }

        /* Check whether it is already connected */
        resp->buf_len = 0;
        resp->line_counts = 0;
        at_utils_send(client->uart_idx, 0, "AT\r\n", 4);

        if (rtos_sem_take(client->resp_notice, resp->timeout, RTOS_FROM_THREAD) != AT_EOK)
            continue;
        else
            break;
    }

    at_delete_resp(resp);

    client->resp = NULL;

    rtos_mutex_give(client->lock, RTOS_FROM_THREAD);

    return result;
}

/**
 * Send data to AT server, send data don't have end sign(eg: \r\n).
 *
 * @param client current AT client object
 * @param buf   send data buffer
 * @param size  send fixed data size
 *
 * @return >0: send data size
 *         =0: send failed
 */
uint32_t at_client_obj_send(at_client_t client, const char *buf, uint32_t size)
{
    uint32_t len;

    AT_ASSERT(buf);

    if (client == NULL)
    {
        AT_LOG_PRINT("input AT Client object is NULL, please create or get AT Client object!");
        return 0;
    }

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("sendline", buf, size);
#endif

    rtos_mutex_take(client->lock, RTOS_WAITING_FOREVER, RTOS_FROM_THREAD);

    len = at_utils_send(client->uart_idx, 0, buf, size);

    rtos_mutex_give(client->lock, RTOS_FROM_THREAD);

    return len;
}

static int at_client_getchar(at_client_t client, char *ch, uint32_t timeout)
{
    int result = AT_EOK;

    while (fifo_queue_getc(client->uart_fifo, ch) == 0)
    {
        result = rtos_sem_take(client->rx_notice, timeout, RTOS_FROM_THREAD);
        if (result != AT_EOK)
        {
            return -AT_ETIMEOUT;
        }
    }

    return AT_EOK;
}

/**
 * AT client receive fixed-length data.
 *
 * @param client current AT client object
 * @param buf   receive data buffer
 * @param size  receive fixed data size
 * @param timeout  receive data timeout (ms)
 *
 * @note this function can only be used in execution function of URC data
 *
 * @return >0: receive data size
 *         =0: receive failed
 */
uint32_t at_client_obj_recv(at_client_t client, char *buf, uint32_t size, uint32_t timeout)
{
    uint32_t len = 0;

    AT_ASSERT(buf);

    if (client == NULL)
    {
        AT_LOG_PRINT("input AT Client object is NULL, please create or get AT Client object!");
        return 0;
    }

    while (1)
    {
        uint32_t read_len;

        read_len = fifo_queue_read(client->uart_fifo, buf + len, size);
        if(read_len > 0)
        {
            len += read_len;
            size -= read_len;
            if(size == 0)
                break;

            continue;
        }

        if(rtos_sem_take(client->rx_notice, timeout, RTOS_FROM_THREAD) != AT_EOK)
            break;
    }

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("urc_recv", buf, len);
#endif

    return len;
}

/**
 *  AT client set end sign.
 *
 * @param client current AT client object
 * @param ch the end sign, can not be used when it is '\0'
 */
void at_obj_set_end_sign(at_client_t client, char ch)
{
    if (client == NULL)
    {
        AT_LOG_PRINT("input AT Client object is NULL, please create or get AT Client object!");
        return;
    }

    client->end_sign = ch;
}

/**
 * set URC(Unsolicited Result Code) table
 *
 * @param client current AT client object
 * @param table URC table
 * @param size table size
 */
int at_obj_set_urc_table(at_client_t client, const struct at_urc *urc_table, uint32_t table_sz)
{
    uint32_t idx;

    if (client == NULL)
    {
        AT_LOG_PRINT("input AT Client object is NULL, please create or get AT Client object!");
        return -AT_ERROR;
    }

    for (idx = 0; idx < table_sz; idx++)
    {
        AT_ASSERT(urc_table[idx].cmd_prefix);
        AT_ASSERT(urc_table[idx].cmd_suffix);
    }

    if (client->urc_table_size == 0)
    {
        client->urc_table = (struct at_urc_table *) rtos_calloc(1, sizeof(struct at_urc_table));
        if (client->urc_table == NULL)
        {
            return -AT_ENOMEM;
        }

        client->urc_table[0].urc = urc_table;
        client->urc_table[0].urc_size = table_sz;
        client->urc_table_size++;
    }
    else
    {
        struct at_urc_table *new_urc_table = NULL;

        /* realloc urc table space */
        new_urc_table = (struct at_urc_table *) rtos_realloc(client->urc_table,client->urc_table_size * sizeof(struct at_urc_table) + sizeof(struct at_urc_table));
        if (new_urc_table == NULL)
        {
            return -AT_ENOMEM;
        }
        client->urc_table = new_urc_table;
        client->urc_table[client->urc_table_size].urc = urc_table;
        client->urc_table[client->urc_table_size].urc_size = table_sz;
        client->urc_table_size++;

    }

    return AT_EOK;
}

/**
 * get AT client object by AT device name.
 *
 * @dev_name AT client device name
 *
 * @return AT client object
 */
at_client_t at_client_get(const char *dev_name)
{
    int idx = 0;

    AT_ASSERT(dev_name);

    for (idx = 0; idx < AT_CLIENT_NUM_MAX; idx++)
    {
        if (strcmp(at_client_table[idx].name, dev_name) == 0)
        {
            return &at_client_table[idx];
        }
    }

    return NULL;
}

/**
 * get first AT client object in the table.
 *
 * @return AT client object
 */
at_client_t at_client_get_first(void)
{
    if (at_client_table[0].status == AT_STATUS_NOINT)
    {
        return NULL;
    }

    return &at_client_table[0];
}

static const struct at_urc *get_urc_obj(at_client_t client)
{
    uint32_t i, j, prefix_len, suffix_len;
    uint32_t bufsz;
    char *buffer = NULL;
    const struct at_urc *urc = NULL;
    struct at_urc_table *urc_table = NULL;

    if (client->urc_table == NULL)
    {
        return NULL;
    }

    buffer = client->recv_line_buf;
    bufsz = client->recv_line_len;

    for (i = 0; i < client->urc_table_size; i++)
    {
        for (j = 0; j < client->urc_table[i].urc_size; j++)
        {
            urc_table = client->urc_table + i;
            urc = urc_table->urc + j;

            prefix_len = strlen(urc->cmd_prefix);
            suffix_len = strlen(urc->cmd_suffix);
            if (bufsz < prefix_len + suffix_len)
            {
                continue;
            }
            if ((prefix_len ? !strncmp(buffer, urc->cmd_prefix, prefix_len) : 1)
                    && (suffix_len ? !strncmp(buffer + bufsz - suffix_len, urc->cmd_suffix, suffix_len) : 1))
            {
                return urc;
            }
        }
    }

    return NULL;
}

static int at_recv_readline(at_client_t client)
{
    uint32_t read_len = 0;
    char ch = 0, last_ch = 0;
    int is_full = AT_FALSE;

    memset(client->recv_line_buf, 0x00, client->recv_line_bufsz);
    client->recv_line_len = 0;

    while (1)
    {
        at_client_getchar(client, &ch, RTOS_WAITING_FOREVER);

        if (read_len < client->recv_line_bufsz)
        {
            client->recv_line_buf[read_len++] = ch;
            client->recv_line_len = read_len;
        }
        else
        {
            is_full = AT_TRUE;
        }

        /* is newline or URC data */
        if ((ch == '\n' && last_ch == '\r') || (client->end_sign != 0 && ch == client->end_sign)
                || get_urc_obj(client))
        {
            if (is_full)
            {
                AT_LOG_PRINT("read line failed. The line data length is out of buffer size(%d)!", client->recv_line_bufsz);
                memset(client->recv_line_buf, 0x00, client->recv_line_bufsz);
                client->recv_line_len = 0;
                return -AT_EFULL;
            }
            break;
        }
        last_ch = ch;
    }

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("recvline", client->recv_line_buf, read_len);
#endif

    return read_len;
}

static void client_parser(at_client_t client)
{
    const struct at_urc *urc;

    while(1)
    {
        if (at_recv_readline(client) > 0)
        {
            if ((urc = get_urc_obj(client)) != NULL)
            {
                /* current receive is request, try to execute related operations */
                if (urc->func != NULL)
                {
                    urc->func(client, client->recv_line_buf, client->recv_line_len);
                }
            }
            else if (client->resp != NULL)
            {
                at_response_t resp = client->resp;

                char end_ch = client->recv_line_buf[client->recv_line_len - 1];

                /* current receive is response */
                client->recv_line_buf[client->recv_line_len - 1] = '\0';
                if (resp->buf_len + client->recv_line_len < resp->buf_size)
                {
                    /* copy response lines, separated by '\0' */
                    memcpy(resp->buf + resp->buf_len, client->recv_line_buf, client->recv_line_len);

                    /* update the current response information */
                    resp->buf_len += client->recv_line_len;
                    resp->line_counts++;
                }
                else
                {
                    client->resp_status = AT_RESP_BUFF_FULL;
                    AT_LOG_PRINT("Read response buffer failed. The Response buffer size is out of buffer size(%d)!", resp->buf_size);
                }
                /* check response result */
                if ((client->end_sign != 0) && (end_ch == client->end_sign) && (resp->line_num == 0))
                {
                    /* get the end sign, return response state END_OK.*/
                    client->resp_status = AT_RESP_OK;
                }
                else if (memcmp(client->recv_line_buf, AT_RESP_END_OK, strlen(AT_RESP_END_OK)) == 0
                        && resp->line_num == 0)
                {
                    /* get the end data by response result, return response state END_OK. */
                    client->resp_status = AT_RESP_OK;
                }
                else if (strstr(client->recv_line_buf, AT_RESP_END_ERROR)
                        || (memcmp(client->recv_line_buf, AT_RESP_END_FAIL, strlen(AT_RESP_END_FAIL)) == 0))
                {
                    client->resp_status = AT_RESP_ERROR;
                }
                else if (resp->line_counts == resp->line_num && resp->line_num)
                {
                    /* get the end data by response line, return response state END_OK.*/
                    client->resp_status = AT_RESP_OK;
                }
                else
                {
                    continue;
                }

                client->resp = NULL;
                rtos_sem_give(client->resp_notice, RTOS_FROM_THREAD);
            }
            else
            {
                AT_LOG_PRINT("unrecognized line: %.*s", client->recv_line_len, client->recv_line_buf);
            }
        }
    }
}

static int at_client_rx_ind(uint32_t uart_idx, void *rxbuf, uint32_t size)
{
    int idx = 0;
    at_client_t client = &at_client_table[0];

    for (idx = 0; idx < AT_CLIENT_NUM_MAX; idx++, client++)
    {
        if (client->uart_idx == uart_idx && client->status && size > 0)
        {
            if (fifo_queue_write(client->uart_fifo, rxbuf, size) == 0)
            {
                break;
            }

            if (client->status == AT_STATUS_READY)
            {
                rtos_sem_give(client->rx_notice, RTOS_FROM_ISR);
            }
            else
            {
                /* give semaphore to pass-through task */
            }
        }
    }

    return AT_EOK;
}

/* initialize the client object parameters */
static int at_client_para_init(at_client_t client)
{
    int result = AT_EOK;
    static int at_client_num = 0;
    char name[AT_NAME_MAX];

    client->status = AT_STATUS_NOINT;

    client->recv_line_len = 0;
    client->recv_line_buf = (char *) rtos_calloc(1, client->recv_line_bufsz);
    if (client->recv_line_buf == NULL)
    {
        AT_LOG_PRINT("AT client initialize failed! No memory for receive buffer.");
        result = -AT_ENOMEM;
        goto __exit;
    }

    client->lock = rtos_mutex_create();
    if (client->lock == NULL)
    {
        AT_LOG_PRINT("AT client initialize failed! at_client_recv_lock create failed!");
        result = -AT_ENOMEM;
        goto __exit;
    }

    client->rx_notice = rtos_sem_create(0);
    if (client->rx_notice == NULL)
    {
        AT_LOG_PRINT("AT client initialize failed! at_client_notice semaphore create failed!");
        result = -AT_ENOMEM;
        goto __exit;
    }

    client->resp_notice = rtos_sem_create(0);
    if (client->resp_notice == NULL)
    {
        AT_LOG_PRINT("AT client initialize failed! at_client_resp semaphore create failed!");
        result = -AT_ENOMEM;
        goto __exit;
    }

    client->urc_table = NULL;
    client->urc_table_size = 0;

    snprintf(name, AT_NAME_MAX, "%s%d", "at_clnt", at_client_num);
    client->parser = rtos_thread_create((const char*)name,
                                         AT_THREAD_STK_SIZE,
                                         AT_THREAD_PRIORITY,
                                        (rtos_entry_t)client_parser,
                                        (void*) client);
    if (client->parser == NULL)
    {
        result = -AT_ENOMEM;
        goto __exit;
    }

__exit:
    if (result != AT_EOK)
    {
        if (client->lock)
        {
            rtos_mutex_delete(client->lock);
        }

        if (client->rx_notice)
        {
            rtos_sem_delete(client->rx_notice);
        }

        if (client->resp_notice)
        {
            rtos_sem_delete(client->resp_notice);
        }

        if (client->recv_line_buf)
        {
            rtos_free(client->recv_line_buf);
        }

        memset(client, 0x00, sizeof(struct at_client));
    }
    else
    {
        at_client_num++;
    }

    return result;
}

/**
 * AT client initialize.
 *
 * @param dev_name AT client device name
 * @param recv_line_bufsz the maximum number of receive buffer length
 *
 * @return 0 : initialize success
 *        -1 : initialize failed
 *        -5 : no memory
 */
int at_client_init(const char *dev_name, uint32_t uart_idx, uint32_t recv_line_bufsz, uint32_t uart_fifo_bufsz)
{
    int idx = 0;
    int result = AT_EOK;
    at_client_t client = NULL;

    AT_ASSERT(dev_name);
    AT_ASSERT(recv_line_bufsz > 0);
    AT_ASSERT(uart_fifo_bufsz > 0);

    if (at_client_get(dev_name) != NULL)
    {
        return result;
    }

    snprintf(client->name, AT_NAME_MAX, "%s", dev_name);

    for (idx = 0; idx < AT_CLIENT_NUM_MAX && at_client_table[idx].status; idx++);

    if (idx >= AT_CLIENT_NUM_MAX)
    {
        AT_LOG_PRINT("AT client initialize failed! Check the maximum number(%d) of AT client.", AT_CLIENT_NUM_MAX);
        result = -AT_EFULL;
        goto __exit;
    }

    client = &at_client_table[idx];

    /* creat uart receive fifo */
    client->uart_fifo = (at_fifo_t)fifo_queue_create(uart_fifo_bufsz);
    if (client->uart_fifo == NULL)
    {
        AT_LOG_PRINT("AT client initialize failed! No memory for uart fifo buffer.");
        result = -AT_ENOMEM;
        goto __exit;
    }

    client->recv_line_bufsz = recv_line_bufsz;

    result = at_client_para_init(client);
    if (result != AT_EOK)
    {
        goto __exit;
    }

    /* find and init uart device */
    client->uart_idx = uart_idx;
    at_device_open(uart_idx);

__exit:
    if (result == AT_EOK)
    {
        client->status = AT_STATUS_READY;
        AT_LOG_PRINT("AT client(V%s) on device %s initialize success.", AT_SW_VERSION, dev_name);
    }
    else
    {
        AT_LOG_PRINT("AT client(V%s) on device %s initialize failed(%d).", AT_SW_VERSION, dev_name, result);
    }

    return result;
}

