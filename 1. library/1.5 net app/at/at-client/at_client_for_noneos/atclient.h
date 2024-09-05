#ifndef __AT_CLIENT_H__
#define __AT_CLIENT_H__


#include "stdint.h"

#define AT_CLIENT_RX_BUFSZ    (4096)


typedef enum {
    AT_RET_OK,
    AT_RET_ERROR,
    AT_RET_ETIMEOUT,
    AT_RET_ENOMEM,
    AT_RET_NULL,
} at_err_t;


typedef enum {
    AT_CMD_STATE_IDLE,
    AT_CMD_STATE_SEND,
    AT_CMD_STATE_WAIT_RESP,
    AT_CMD_STATE_COMPLETE,
} at_cmd_state_t;

typedef enum {
    AT_CMD_TYPE_NORMAL,
    AT_CMD_TYPE_HTTP,
    AT_CMD_TYPE_MQTT,
} at_cmd_type_t;

struct atc_cmd;

typedef void(*at_cmd_cb_t)(struct atc_cmd *cmd);

typedef struct atc_cmd{
    at_cmd_type_t type;
    volatile at_cmd_state_t state;
    const char *cmd_buf;    /* 命令指针 */
    uint8_t  *data_buf;     /* 附加数据指针 */
    uint8_t  *resp_buf;     /* 响应数据缓冲区 */
    uint16_t  cmd_len;      /* 命令长度*/
    uint16_t  data_len;
    uint16_t  resp_bufsz;
    uint16_t  resp_len;     /* 接收长度 */
    uint16_t  timeout;
    uint32_t  deadline;
    uint32_t  fwsize;    /* 文件长度，发送HTTP用 */
    volatile  at_err_t ret;
    struct atc_cmd *next;
    at_cmd_cb_t cb;
} at_cmd_t;

typedef struct {
    uint8_t rxbuf[AT_CLIENT_RX_BUFSZ];
    uint16_t rxlen;
    uint8_t  pre_c;
    uint8_t  ok;
    uint16_t payload_pos;
    uint16_t temp_cnt;      /* http接收计数器 */
    uint8_t  http_state;    /* http标志 */
    uint16_t http_msg_len;  /* http报文长度 */
} at_rx_handle_t;


void at_cmd_send(at_cmd_t *cmd_handle, at_cmd_type_t type, const char *cmd_str, uint16_t cmd_len, uint8_t *resp_buf, uint16_t resp_bufsz, uint16_t timeout, at_cmd_cb_t cb);
void at_cmd_send_ex(at_cmd_t *cmd_handle, const char *cmd_str, uint16_t cmd_len, uint8_t *resp_buf, uint16_t resp_bufsz, uint8_t *data_buf, uint16_t data_len, uint16_t timeout, at_cmd_cb_t cb);

int  at_client_task(void *args);



#endif