#include "atclient.h"
#include "bsp_wifi.h"
#include "multi_task.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "atclient_urc.h"
#include "stdbool.h"


static void at_send_task(void);
static void at_recv_task(void);



typedef struct {
    at_cmd_t *head;
    at_cmd_t *tail;
} at_cmd_list_t;



static at_rx_handle_t at_rx_handle;     /* at接收状态机句柄 */
static at_cmd_list_t  at_cmd_list;      /* at命令链表 */

// uint8_t mqtt_topic[50], mqtt_buf[512];


/**
 * @brief 将命令插入链表尾
 */
static void at_cmd_append(at_cmd_t *cmd_handle) {
    if (at_cmd_list.head == NULL) {
        at_cmd_list.head = cmd_handle;
        at_cmd_list.tail = cmd_handle;        
    } else {
        at_cmd_list.tail->next = cmd_handle;
        at_cmd_list.tail = cmd_handle;
    }
}

/**
 * @brief 从链表头删除一条已执行的命令
 */
static void at_cmd_remove(void) {
    at_cmd_t *cmd = at_cmd_list.head;

    if(cmd != NULL) {
        at_cmd_list.head = cmd->next;
        
        if(cmd->next == NULL) {
            at_cmd_list.tail = NULL;
        }
    }
}


/**
 * @brief 发送at命令（扩展模式，附加发送数据）
 * 
 * @param cmd_handle cmd结构体指针
 * @param cmd_str 命令字符串（\r\n结束）
 * @param cmd_len 命令字节数
 * @param resp_buf 响应消息缓冲区指针
 * @param resp_bufsz 缓冲区大小
 * @param data_buf 待发送的数据缓冲区指针
 * @param data_len 数据长度
 * @param timeout 超时时间
 */
void at_cmd_send_ex(at_cmd_t *cmd_handle, const char *cmd_str, uint16_t cmd_len, uint8_t *resp_buf, uint16_t resp_bufsz, uint8_t *data_buf, uint16_t data_len, uint16_t timeout, at_cmd_cb_t cb) {
    cmd_handle->type = AT_CMD_TYPE_NORMAL;
    cmd_handle->state = AT_CMD_STATE_SEND;
    cmd_handle->next = NULL;
    cmd_handle->cmd_buf = cmd_str;
    cmd_handle->cmd_len = cmd_len;
    cmd_handle->resp_buf = resp_buf;
    cmd_handle->resp_bufsz = resp_bufsz;
    cmd_handle->resp_len = 0;
    cmd_handle->timeout = timeout;
    cmd_handle->ret = AT_RET_NULL;
    cmd_handle->data_buf = data_buf;
    cmd_handle->data_len = data_len;
    cmd_handle->cb = cb;

    at_cmd_append(cmd_handle);
}

/**
 * @brief 发送at命令（标准命令）
 * 
 * @param cmd_handle cmd结构体指针
 * @param cmd_str 命令字符串
 * @param cmd_len 命令长度
 * @param resp_buf 响应消息缓冲区指针
 * @param resp_bufsz 缓冲区大小
 * @param timeout 超时时间
 */
void at_cmd_send(at_cmd_t *cmd_handle, at_cmd_type_t type, const char *cmd_str, uint16_t cmd_len, uint8_t *resp_buf, uint16_t resp_bufsz, uint16_t timeout, at_cmd_cb_t cb) {
    cmd_handle->type = type;
    cmd_handle->state = AT_CMD_STATE_SEND;
    cmd_handle->next = NULL;
    cmd_handle->cmd_buf = cmd_str;
    cmd_handle->cmd_len = cmd_len;
    cmd_handle->resp_buf = resp_buf;
    cmd_handle->resp_bufsz = resp_bufsz;
    cmd_handle->resp_len = 0;
    cmd_handle->timeout = timeout;
    cmd_handle->ret = AT_RET_NULL;
    cmd_handle->data_buf = NULL;
    cmd_handle->cb = cb;

    at_cmd_append(cmd_handle);
}

/**
 * @brief at client状态机（接收解析/命令发送）
 * 
 * @param args 
 */
int at_client_task(void *args) {
    at_recv_task();
    at_send_task();
    return 0;
}




/**
 * @brief at client 命令发送任务
 * 
 */
void at_send_task(void) {

    at_cmd_t *cmd = at_cmd_list.head;

    // if (bsp_wifi_is_enable() == false) {
    //     while (cmd != NULL) {
    //         cmd->state = AT_CMD_STATE_IDLE;
    //         cmd->state = AT_RET_ETIMEOUT;
    //         at_cmd_remove();
    //         cmd = at_cmd_list.head;
    //     }
    //     return;
    // }

    if (cmd == NULL) {
        return;
    }

    switch (cmd->state) {

        case AT_CMD_STATE_SEND:
            bsp_wifi_send((uint8_t *)cmd->cmd_buf, cmd->cmd_len);
            cmd->deadline = get_system_ticks() + cmd->timeout;
            cmd->state = AT_CMD_STATE_WAIT_RESP;
            break;

        case AT_CMD_STATE_WAIT_RESP:
            if ((int32_t)(cmd->deadline - get_system_ticks()) < 0) {
                cmd->ret = AT_RET_ETIMEOUT;
                cmd->state = AT_CMD_STATE_COMPLETE;
                at_cmd_remove();
            }
            break;

        case AT_CMD_STATE_COMPLETE:
            at_cmd_remove();
            break;

        default:
            break;
    }

    if ((cmd->state == AT_CMD_STATE_COMPLETE) && cmd->cb) {
        cmd->cb(cmd);
    }
}

/**
 * @brief 从串口fifo读一条报文（"\r\n"分隔）
 * 
 * @return uint8_t 1表示读取成功， 0表示尚未完整读取报文
 */
static uint8_t at_message_read(void) {
    uint8_t c;

    // if (bsp_wifi_is_enable() == false) {
    //     (void)bsp_wifi_getc(&c);
    // }

    if (at_rx_handle.ok == 0) {
        while (bsp_wifi_getc(&c) == 1) {

            at_rx_handle.rxbuf[at_rx_handle.rxlen++] = c;

            /* 超出接收缓冲，则丢弃已接收的数据 */
            if (at_rx_handle.rxlen >= AT_CLIENT_RX_BUFSZ) {
                at_rx_handle.rxlen = 0;
                at_rx_handle.pre_c = 0;
                continue;
            }

            /* 如果接受的报文是HTTP数据，则对接收的数据计数 */
            if ((at_rx_handle.http_state == 2) && (at_rx_handle.temp_cnt > 0)) {
                at_rx_handle.temp_cnt--;
            }

            if ((at_rx_handle.pre_c == '\r') && (c == '\n')) {
                /* 如果是空报文"\r\n"，则丢弃该报文 */
                if (at_rx_handle.rxlen == 2) {
                    at_rx_handle.rxlen = 0;
                    at_rx_handle.pre_c = 0;
                    continue;
                }

                if ((at_rx_handle.http_state != 2) || ((at_rx_handle.http_state == 2) && (at_rx_handle.temp_cnt == 0))) {
                    at_rx_handle.ok = 1;
                    return 1;
                }

            } 

            at_rx_handle.pre_c = c;


            /* 解析HTTP报文 */
            if (at_rx_handle.rxlen == 12) {
                if (strncmp((const char *)at_rx_handle.rxbuf, "+HTTPCLIENT:", 12) == 0) {
                    at_rx_handle.http_state = 1;
                    at_rx_handle.http_msg_len = 0xFFFF;
                    at_rx_handle.payload_pos = at_rx_handle.rxlen;
                } else {
                    at_rx_handle.http_state = 0;
                }
            }

            /* 获取http消息长度 */
            if (at_rx_handle.http_state == 1) {
                if ((at_rx_handle.http_msg_len == 0XFFFF) && (c == ',')) {
                    at_rx_handle.rxbuf[at_rx_handle.rxlen - 1] = '\0';
                    at_rx_handle.http_msg_len = atoi((const char *)&at_rx_handle.rxbuf[at_rx_handle.payload_pos]);
                    at_rx_handle.temp_cnt = at_rx_handle.http_msg_len;
                    at_rx_handle.payload_pos = at_rx_handle.rxlen;
                    at_rx_handle.http_state = 2;
                }
            }
        }
    }

    return 0;
}


/**
 * @brief at client 报文解析状态机
 * 
 */
static void at_recv_task(void) {
    at_cmd_t *cmd = at_cmd_list.head;

    if (at_message_read() == 1) {

        /* 检查是否是urc数据 */
        const at_urc_t *urc = &urc_prefix[0];
        while (urc->str != NULL) {
            if (strncmp((const char *)at_rx_handle.rxbuf, urc->str, urc->len) == 0) {
                (urc->cb)(&at_rx_handle);
                goto exit;
            }
            urc++;
        }
        
        /* 检查命令列表中是否存在等待答复的命令 */
        if ((cmd != NULL) && (cmd->state == AT_CMD_STATE_WAIT_RESP)) {

            if ((at_rx_handle.rxlen == 4) && (strncmp((const char *)at_rx_handle.rxbuf, "OK\r\n", 4) == 0)) {

                if (cmd->data_buf != NULL) {
                    bsp_wifi_send(cmd->data_buf, cmd->data_len);
                    cmd->data_buf = NULL;
                } else {
                    cmd->ret = AT_RET_OK;
                    cmd->state = AT_CMD_STATE_COMPLETE;
                }

            } else if ((at_rx_handle.rxlen == 7) && (strncmp((const char *)at_rx_handle.rxbuf, "ERROR\r\n", 7) == 0)) {

                cmd->ret = AT_RET_ERROR;
                cmd->state = AT_CMD_STATE_COMPLETE;

            } else if ((at_rx_handle.rxlen == 13) && (strncmp((const char *)at_rx_handle.rxbuf, "+MQTTPUB:OK\r\n", 13) == 0)) {

                cmd->ret = AT_RET_OK;
                cmd->state = AT_CMD_STATE_COMPLETE;

            } else if ((at_rx_handle.rxlen == 16) && (strncmp((const char *)at_rx_handle.rxbuf, "+MQTTPUB:ERROR\r\n", 16) == 0)) {

                cmd->ret = AT_RET_ERROR;
                cmd->state = AT_CMD_STATE_COMPLETE;

            } else if ((cmd->type == AT_CMD_TYPE_HTTP) && (strncmp((const char *)at_rx_handle.rxbuf, "ContentRange:bytes", 18) == 0)) {

                char *start;
                start = strstr((const char *)at_rx_handle.rxbuf, "/");
                if (start != NULL ) {
                    at_rx_handle.rxbuf[at_rx_handle.rxlen - 2] = '\0';
                    cmd->fwsize = atoi((const char *)(start + 1));
                }

            } else  {
                if (cmd->resp_buf != NULL) {
                    if ((cmd->type == AT_CMD_TYPE_HTTP) && (at_rx_handle.http_state == 2)) {
                        if ((cmd->resp_len + at_rx_handle.http_msg_len) > cmd->resp_bufsz) {    
                            /* 当接收数据大于buffer大小时，返回错误 */
                            // cmd->ret = AT_RET_ENOMEM;
                            // cmd->state = AT_CMD_STATE_COMPLETE;
                        } else {
                            memcpy(cmd->resp_buf + cmd->resp_len, &at_rx_handle.rxbuf[at_rx_handle.payload_pos], at_rx_handle.http_msg_len);
                            cmd->resp_len += at_rx_handle.http_msg_len;
                        }

                    } else { 
                        if ((cmd->resp_len + at_rx_handle.rxlen) > (cmd->resp_bufsz - 1)) {    
                            /* 当接收数据大于buffer大小时，返回错误 */
                            // cmd->ret = AT_RET_ENOMEM;
                            // cmd->state = AT_CMD_STATE_COMPLETE;
                        } else {
                            memcpy(cmd->resp_buf + cmd->resp_len, at_rx_handle.rxbuf, at_rx_handle.rxlen);
                            cmd->resp_len += at_rx_handle.rxlen;
                            cmd->resp_buf[cmd->resp_len++] = '\0';   /* 用'\0'分割响应报文 */
                        }
                    }

                }
            }
        }

exit:
        /* 复位rx handle */
        at_rx_handle.http_state = 0;
        at_rx_handle.rxlen = 0;
        at_rx_handle.pre_c = 0;
        at_rx_handle.ok = 0;
    }
}