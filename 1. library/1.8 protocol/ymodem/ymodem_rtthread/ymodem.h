/*
 * COPYRIGHT (C) 2011-2022, Real-Thread Information Technology Ltd
 * All rights reserved
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-04-14     Grissiom     initial implementation
 * 2019-12-09     Steven Liu   add YMODEM send protocol
 * 2022-08-04     Meco Man     move error codes to rym_code to silence warnings
 * 2023-03-23     zjcszn       port to freertos platform
 */

#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <string.h>
#include "rym_port.h"

#define RYM_USE_XSHELL      1

/* The word "RYM" is stand for "Real-YModem". */
enum rym_code
{
    RYM_CODE_NONE = 0x00,
    RYM_CODE_SOH  = 0x01,
    RYM_CODE_STX  = 0x02,
    RYM_CODE_EOT  = 0x04,
    RYM_CODE_ACK  = 0x06,
    RYM_CODE_NAK  = 0x15,
    RYM_CODE_CAN  = 0x18,
    RYM_CODE_C    = 0x43,
};

/* RYM error code */
enum rym_error
{
    RYM_OK        = 0x00,
    RYM_ERROR     = 0x70,

    RYM_ERR_TMO   = 0x71, /* timeout on handshake */
    RYM_ERR_CODE  = 0x72, /* wrong code, wrong SOH, STX etc */
    RYM_ERR_SEQ   = 0x73, /* wrong sequence number */
    RYM_ERR_CRC   = 0x74, /* wrong CRC checksum */
    RYM_ERR_DSZ   = 0x75, /* not enough data received */
    RYM_ERR_CAN   = 0x76, /* the transmission is aborted by user */
    RYM_ERR_ACK   = 0x77, /* wrong answer, wrong ACK or C */
    RYM_ERR_FILE  = 0x78, /* transmit file invalid */
    RYM_ERR_MEM   = 0x79, /* no enough memory space to alloc */
    RYM_ERR_DEV   = 0x7A, /* serial device not ready */
};

/* device id of rym */
#define RYM_DEV     0

/* how many millisecond wait for chars between packet. */
#ifndef RYM_WAIT_CHR_TIME
#define RYM_WAIT_CHR_TIME   (1000 * 1)
#endif
/* how many millisecond wait for between packet. */
#ifndef RYM_WAIT_PKG_TIME
#define RYM_WAIT_PKG_TIME   (1000 * 3)
#endif
/* how many millisecond between two handshake code. */
#ifndef RYM_CHD_INTV_TIME
#define RYM_CHD_INTV_TIME   (1000 * 1)
#endif

/* timeout of the clear buffer operation */
#ifndef RYM_CLEAR_BUFFER_TIMEOUT
#define RYM_CLEAR_BUFFER_TIMEOUT        (50u)
#endif

/* how many CAN be sent when user active end the session. */
#ifndef RYM_END_SESSION_SEND_CAN_NUM
#define RYM_END_SESSION_SEND_CAN_NUM    ( 7u)
#endif

/* how many retries were made when the error occurred */
#ifndef RYM_MAX_ERRORS
#define RYM_MAX_ERRORS                  ( 5u)
#endif

enum rym_stage
{
    RYM_STAGE_NONE = 0,
    /* set when C is send */
    RYM_STAGE_ESTABLISHING,
    /* set when we've got the packet 0 and sent ACK and second C */
    RYM_STAGE_ESTABLISHED,
    /* set when the sender respond to our second C and recviever got a real
     * data packet. */
    RYM_STAGE_TRANSMITTING,
    /* set when the sender send a EOT */
    RYM_STAGE_FINISHING,
    /* set when transmission is really finished, i.e., after the NAK, C, final
     * NULL packet stuff. */
    RYM_STAGE_FINISHED,
};

struct _rym_ctx;
typedef struct _rym_ctx *rym_ctx_t;

/* When receiving files, the buf will be the data received from ymodem protocol
 * and the len is the data size.
 *
 * When sending files, the len is the buf size in RYM. The callback need to
 * fill the buf with data to send. Returning RYM_CODE_EOT will terminate the
 * transfer and the buf will be discarded. Any other return values will cause
 * the transfer continue.
 */
typedef enum rym_code(*rym_callback)(rym_ctx_t ctx, unsigned char *buf, unsigned long len);

/* Currently RYM only support one transfer session(ctx) for simplicity.
 *
 * In case we could support multiple sessions in The future, the first argument
 * of APIs are (struct rym_ctx*).
 */
struct _rym_ctx
{
    rym_callback on_data;
    rym_callback on_begin;
    rym_callback on_end;
    enum rym_stage stage;
    unsigned char *buf;    
    rym_semp_t sem;
    unsigned char seq;
};

/* recv a file on device dev with ymodem session ctx.
 *
 * If an error happens, you can get where it is failed from ctx->stage.
 *
 * @param on_begin The callback will be invoked when the first packet arrived.
 * This packet often contain file names and the size of the file, if the sender
 * support it. So if you want to save the data to a file, you may need to
 * create the file on need. It is the on_begin's responsibility to parse the
 * data content. The on_begin can be NULL, in which case the transmission will
 * continue without any side-effects.
 *
 * @param on_data The callback will be invoked on the packets received.  The
 * callback should save the data to the destination. The return value will be
 * sent to the sender and in turn, only RYM_{ACK,CAN} is valid. When on_data is
 * NULL, RYM will barely send ACK on every packet and have no side-effects.
 *
 * @param on_end The callback will be invoked when one transmission is
 * finished. The data should be 128 bytes of NULL. You can do some cleaning job
 * in this callback such as closing the file. The return value of this callback
 * is ignored. As above, this parameter can be NULL if you don't need such
 * function.
 *
 * @param handshake_timeout the timeout when hand shaking. The unit is in
 * second.
 */
int rym_recv(rym_ctx_t ctx, rym_callback on_begin, rym_callback on_data, rym_callback on_end,
                int handshake_timeout);

/* send a file on device dev with ymodem session ctx.
 *
 * If an error happens, you can get where it is failed from ctx->stage.
 *
 * @param on_begin The callback will be invoked when the first packet is sent.
 * This packet often contain file names and the size of the file. It is the
 * on_begin's responsibility to parse the basic information of the file. The
 * on_begin can not be NULL.
 *
 * @param on_data The callback will be invoked when the data packets is sent.
 * The callback should read file system and prepare the data packets. The
 * on_data can not be NULL.
 *
 * @param on_end The callback will be invoked when one transmission is
 * finished. The data should be 128 bytes of NULL. The on_end can not be NULL.
 *
 * @param handshake_timeout the timeout when hand shaking. The unit is in
 * second.
 */
int rym_send(rym_ctx_t ctx, rym_callback on_begin, rym_callback on_data, rym_callback on_end,
                int handshake_timeout);

#endif
