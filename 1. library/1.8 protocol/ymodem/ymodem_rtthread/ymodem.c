/*
 * COPYRIGHT (C) 2011-2023, Real-Thread Information Technology Ltd
 * All rights reserved
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-04-14     Grissiom     initial implementation
 * 2019-12-09     Steven Liu   add YMODEM send protocol
 * 2023-03-23     zjcszn       port to freertos platform
 */

#include "ymodem.h"

#ifdef YMODEM_USING_CRC_TABLE
static const unsigned short ccitt_table[256] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
static unsigned short CRC16(unsigned char *q, int len)
{
    unsigned short crc = 0;

    while (len-- > 0)
        crc = (crc << 8) ^ ccitt_table[((crc >> 8) ^ *q++) & 0xff];
    return crc;
}
#else
static unsigned short CRC16(unsigned char *q, int len)
{
    unsigned short crc;
    char i;

    crc = 0;
    while (--len >= 0)
    {
        crc = crc ^ (int) * q++ << 8;
        i = 8;
        do
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        }
        while (--i);
    }

    return (crc);
}
#endif

// we could only use global varible because we could not use
// rt_device_t->user_data(it is used by the serial driver)...
static rym_ctx_t rym_ctx;
static bool rym_rx_indi;

void rym_rx_notify(void)
{
    if (rym_ctx && rym_rx_indi)
    {
        rym_semp_give(rym_ctx->sem);
    }
}

/* SOH/STX + seq + payload + crc */
#define _RYM_SOH_PKG_SZ (1+2+128+2)
#define _RYM_STX_PKG_SZ (1+2+1024+2)

static unsigned char _rym_getchar(
    rym_ctx_t ctx,
    unsigned long  timeout)
{
    unsigned char c;
    while (rym_serial_read(RYM_DEV, &c, 1) != 1)
    {
        if (rym_semp_take(ctx->sem, timeout) != true)
        {
            return RYM_CODE_NONE;
        }
    }
    return c;
}

/* the caller should at least alloc _RYM_STX_PKG_SZ buffer */
static unsigned long _rym_read_data(
    rym_ctx_t ctx,
    unsigned long len)
{
    /* we should already have had the code */
    unsigned char *buf = ctx->buf + 1;
    unsigned long readlen = 0;

    do
    {
        readlen += rym_serial_read(RYM_DEV, buf + readlen, len - readlen);
        if (readlen >= len)
            return readlen;
    }
    while (rym_semp_take(ctx->sem, RYM_WAIT_CHR_TIME) == true);

    return readlen;
}

static void _rym_clear_buffer(rym_ctx_t ctx)
{
    /* wait for no data to recive */
    while (rym_semp_take(ctx->sem, RYM_CLEAR_BUFFER_TIMEOUT) == true);

    /* clear serial recive buffer */
    rym_serial_clear(RYM_DEV);
}

static int _rym_send_packet(
    rym_ctx_t ctx,
    unsigned char code,
    unsigned char index)
{
    unsigned short send_crc;
    unsigned long  writelen = 0;
    unsigned long  packetlen = 0;

    switch(code)
    {
    case RYM_CODE_SOH:
        packetlen = _RYM_SOH_PKG_SZ;
        break;
    case RYM_CODE_STX:
        packetlen = _RYM_STX_PKG_SZ;
        break;
    default:
        return RYM_ERROR;
    }

    send_crc = CRC16(ctx->buf + 3, packetlen - 5);
    ctx->buf[0] =  code;
    ctx->buf[1] =  index;
    ctx->buf[2] = ~index;
    ctx->buf[packetlen - 2] = (unsigned char)(send_crc >> 8);
    ctx->buf[packetlen - 1] = (unsigned char)(send_crc & 0xff);

    do
    {
        writelen += rym_serial_write(RYM_DEV, ctx->buf + writelen, packetlen - writelen);
    }
    while (writelen < packetlen);

    return RYM_OK;
}

static unsigned long _rym_putchar(rym_ctx_t ctx, unsigned char code)
{
    return rym_serial_write(RYM_DEV, &code, 1);
}

static int _rym_do_recv_handshake(
    rym_ctx_t ctx,
    int tm_sec)
{
    unsigned char  code;
    unsigned long  i;
    unsigned short recv_crc, cal_crc;
    unsigned long  data_sz;
    unsigned long  tick;

    ctx->stage = RYM_STAGE_ESTABLISHING;
    /* send C every second, so the sender could know we are waiting for it. */
    for (i = 0; i < tm_sec; i++)
    {
        _rym_putchar(ctx, RYM_CODE_C);
        code = _rym_getchar(ctx, RYM_CHD_INTV_TIME);
        if (code == RYM_CODE_SOH)
        {
            data_sz = _RYM_SOH_PKG_SZ;
            break;
        }
        else if (code == RYM_CODE_STX)
        {
            data_sz = _RYM_STX_PKG_SZ;
            break;
        }
    }
    if (i == tm_sec)
    {
        return RYM_ERR_TMO;
    }

    /* receive all data */

    /* automatic exit after receiving specified length data, timeout: 100ms */
    i = _rym_read_data(ctx, data_sz - 1);

    if (i != (data_sz - 1)) 
    {
        return RYM_ERR_DSZ;
    }

    /* sanity check */
    if (ctx->buf[1] != 0 || ctx->buf[2] != 0xFF)
    {
        return RYM_ERR_SEQ;
    }


    recv_crc = (unsigned short)(*(ctx->buf + data_sz - 2) << 8) | *(ctx->buf + data_sz - 1);
    cal_crc = CRC16(ctx->buf + 3, data_sz - 5);
    if (recv_crc != cal_crc)
    {
        return RYM_ERR_CRC;        
    }

    /* congratulations, check passed. */
    if (ctx->on_begin && ctx->on_begin(ctx, ctx->buf + 3, data_sz - 5) != RYM_CODE_ACK)
    {
        return RYM_ERR_CAN;
    }

    return RYM_OK;
}

static int _rym_do_send_handshake(
    rym_ctx_t ctx,
    int tm_sec)
{
    unsigned char code;
    unsigned long i;
    unsigned long data_sz;
    unsigned char getc_ack;

    ctx->stage = RYM_STAGE_ESTABLISHING;
    data_sz = _RYM_SOH_PKG_SZ;

    /* receive C every second */
    for (i = 0; i < tm_sec; i++)
    {
        code = _rym_getchar(ctx, RYM_CHD_INTV_TIME);
        if (code == RYM_CODE_C)
        {
            break;
        }
    }
    if (i == tm_sec)
    {
        return RYM_ERR_TMO;
    }

    /* congratulations, check passed. */
    if (ctx->on_begin && ctx->on_begin(ctx, ctx->buf + 3, data_sz - 5) != RYM_CODE_SOH)
        return RYM_ERR_CODE;

    code = RYM_CODE_SOH;
    _rym_send_packet(ctx, code, 0);

    getc_ack = _rym_getchar(ctx, RYM_WAIT_CHR_TIME);

    if (getc_ack != RYM_CODE_ACK)
    {
        return RYM_ERR_ACK;
    }

    getc_ack = _rym_getchar(ctx, RYM_WAIT_CHR_TIME);

    if (getc_ack != RYM_CODE_C)
    {
        return RYM_ERR_ACK;
    }

    ctx->stage = RYM_STAGE_ESTABLISHED;

    return RYM_OK;
}

static int _rym_recv_trans_data(
    rym_ctx_t ctx,
    unsigned long data_sz,
    unsigned char *code)
{
    const unsigned long tsz = 2 + data_sz + 2;
    unsigned short recv_crc;

    /* seq + data + crc */
    unsigned long i = _rym_read_data(ctx, tsz);
    if (i != tsz)
    {
        return RYM_ERR_DSZ;
    }

    if (((ctx->buf[1] ^ ctx->buf[2]) == 0xFF) && (ctx->buf[1] == ctx->seq))
    {
        recv_crc = (unsigned short)(*(ctx->buf + tsz - 1) << 8) | *(ctx->buf + tsz);
        if (recv_crc != CRC16(ctx->buf + 3, data_sz))
        {
            return RYM_ERR_CRC;
        }

        /* congratulations, check passed. */
        if (ctx->on_data)
        {
            *code = ctx->on_data(ctx, ctx->buf + 3, data_sz);
        }
        else
        {
            *code = RYM_CODE_ACK;    
        }
    }
    else
    {
        return RYM_ERR_SEQ;
    }

    return RYM_OK;
}

static int _rym_do_recv_trans(rym_ctx_t ctx)
{
    _rym_putchar(ctx, RYM_CODE_ACK);
    _rym_putchar(ctx, RYM_CODE_C);
    unsigned long err_cnt = 0;

    while (1)
    {
        long err;
        unsigned char code;
        unsigned long data_sz, i;

        code = _rym_getchar(ctx,
                              RYM_WAIT_PKG_TIME);
        switch (code)
        {
            case RYM_CODE_SOH:
                data_sz = 128;
                break;
            case RYM_CODE_STX:
                data_sz = 1024;
                break;
            case RYM_CODE_EOT:
                return RYM_OK;
            default:
                err_cnt++;
                if(err_cnt > RYM_MAX_ERRORS)
                {
                    return RYM_ERR_CODE;/* Abort communication */
                }
                else
                {
                    _rym_clear_buffer(ctx); /* Clear serial buffer */
                    _rym_putchar(ctx, RYM_CODE_NAK);/* Ask for a packet */
                    continue;
                }
        };

        err = _rym_recv_trans_data(ctx, data_sz, &code);
        if (err != RYM_OK)
        {
            err_cnt++;
            if(err_cnt > RYM_MAX_ERRORS)
            {
                return err;/* Abort communication */
            }
            else
            {
                _rym_clear_buffer(ctx); /* Clear serial buffer */
                _rym_putchar(ctx, RYM_CODE_NAK);/* Ask for a packet */
                continue;
            }
        }
        else
        {
            err_cnt = 0;
        }

        switch (code)
        {
            case RYM_CODE_CAN:
                /* the spec require multiple CAN */
                for (i = 0; i < RYM_END_SESSION_SEND_CAN_NUM; i++)
                {
                    _rym_putchar(ctx, RYM_CODE_CAN);
                }
                return RYM_ERR_CAN;
            case RYM_CODE_ACK:
                ctx->seq++;
                _rym_putchar(ctx, RYM_CODE_ACK);
                break;
            default:
                /* wrong code */
                break;
        }
    }
}

static int _rym_do_send_trans(rym_ctx_t ctx)
{
    ctx->stage = RYM_STAGE_TRANSMITTING;
    unsigned char code;
    unsigned long data_sz;
    unsigned char getc_ack;
    unsigned char i;
    data_sz = _RYM_STX_PKG_SZ;

    if (!ctx->on_data)
    {
        return RYM_ERR_CODE;
    }
    while (1)
    {
        code = ctx->on_data(ctx, ctx->buf + 3, data_sz - 5);

        if ((code != RYM_CODE_SOH) && (code != RYM_CODE_STX))
        {
            return RYM_ERR_CODE;
        }

        for (i = 0; i < RYM_MAX_ERRORS; i++)
        {
            _rym_send_packet(ctx, code, ctx->seq);
            getc_ack = _rym_getchar(ctx, RYM_WAIT_CHR_TIME);

            if (getc_ack == RYM_CODE_ACK)
            {
                ctx->seq++;
                break;
            }
            if (getc_ack != RYM_CODE_NAK)
            {
                return RYM_ERR_ACK;
            }
        }

        if (i == RYM_MAX_ERRORS)
        {
            return RYM_ERR_ACK;
        }

        if (ctx->stage == RYM_STAGE_FINISHING)
            break;
    }

    return RYM_OK;
}

static int _rym_do_recv_fin(rym_ctx_t ctx)
{
    unsigned char code;
    unsigned short recv_crc;
    unsigned long i;
    unsigned long data_sz;

    ctx->stage = RYM_STAGE_FINISHING;
    /* we already got one EOT in the caller. invoke the callback if there is
     * one. */
    if (ctx->on_end)
        ctx->on_end(ctx, ctx->buf + 3, 128);

    _rym_putchar(ctx, RYM_CODE_NAK);
    code = _rym_getchar(ctx, RYM_WAIT_PKG_TIME);
    if (code != RYM_CODE_EOT)
        return RYM_ERR_CODE;

    _rym_putchar(ctx, RYM_CODE_ACK);
    _rym_putchar(ctx, RYM_CODE_C);

    code = _rym_getchar(ctx, RYM_WAIT_PKG_TIME);
    if (code == RYM_CODE_SOH)
    {
        data_sz = _RYM_SOH_PKG_SZ;
    }
    else if (code == RYM_CODE_STX)
    {
        data_sz = _RYM_STX_PKG_SZ;
    }
    else
        return RYM_ERR_CODE;

    i = _rym_read_data(ctx, data_sz - 1);
    if (i != (data_sz - 1))
        return RYM_ERR_DSZ;

    /* sanity check
     */
    if (ctx->buf[1] != 0 || ctx->buf[2] != 0xFF)
        return RYM_ERR_SEQ;

    recv_crc = (unsigned short)(*(ctx->buf + data_sz - 2) << 8) | *(ctx->buf + data_sz - 1);
    if (recv_crc != CRC16(ctx->buf + 3, data_sz - 5))
        return RYM_ERR_CRC;

    /*next file transmission*/
    if (ctx->buf[3] != 0)
    {
        if (ctx->on_begin && ctx->on_begin(ctx, ctx->buf + 3, data_sz - 5) != RYM_CODE_ACK)
            return RYM_ERR_CAN;
        ctx->seq = 0x01;
        return RYM_OK;
    }

    /* congratulations, check passed. */
    ctx->stage = RYM_STAGE_FINISHED;

    /* put the last ACK */
    _rym_putchar(ctx, RYM_CODE_ACK);

#if (RYM_USE_XSHELL == 1)
    /* if use xshell,  send 'O' char to complete stop transmission */
    _rym_putchar(ctx, 'O');
#endif

    return RYM_OK;
}

static int _rym_do_send_fin(rym_ctx_t ctx)
{
    unsigned char code;
    unsigned long data_sz;

    data_sz = _RYM_SOH_PKG_SZ;

    _rym_putchar(ctx, RYM_CODE_EOT);
    if (_rym_getchar(ctx, RYM_WAIT_CHR_TIME) != RYM_CODE_NAK)
    {
        return RYM_ERR_ACK;
    }

    _rym_putchar(ctx, RYM_CODE_EOT);
    if (_rym_getchar(ctx, RYM_WAIT_CHR_TIME) != RYM_CODE_ACK)
    {
        return RYM_ERR_ACK;
    }
    if (_rym_getchar(ctx, RYM_WAIT_CHR_TIME) != RYM_CODE_C)
    {
        return RYM_ERR_ACK;
    }

    if (ctx->on_end && ctx->on_end(ctx, ctx->buf + 3, data_sz - 5) != RYM_CODE_SOH)
        return RYM_ERR_CODE;

    code = RYM_CODE_SOH;

    _rym_send_packet(ctx, code, 0);
    
    /* get final ack response of receiver */
    (void)_rym_getchar(ctx, RYM_WAIT_CHR_TIME);

    ctx->stage = RYM_STAGE_FINISHED;
    return RYM_OK;
}

static int _rym_do_recv(
    rym_ctx_t ctx,
    int handshake_timeout)
{
    int err;

    ctx->stage = RYM_STAGE_NONE;

    ctx->buf = rym_malloc(_RYM_STX_PKG_SZ);
    if (ctx->buf == NULL)
        return RYM_ERR_MEM;

    err = _rym_do_recv_handshake(ctx, handshake_timeout);
    if (err != RYM_OK)
    {
        rym_free(ctx->buf);
        return err;
    }
    ctx->stage = RYM_STAGE_ESTABLISHED;

    while (1)
    {
        err = _rym_do_recv_trans(ctx);

        err = _rym_do_recv_fin(ctx);
        if (err != RYM_OK)
        {
            rym_free(ctx->buf);
            return err;
        }

        if (ctx->stage == RYM_STAGE_FINISHED)
            break;
    }
    rym_free(ctx->buf);
    return err;
}

static int _rym_do_send(
    rym_ctx_t ctx,
    int handshake_timeout)
{
    int err;

    ctx->stage = RYM_STAGE_NONE;

    ctx->buf = rym_malloc(_RYM_STX_PKG_SZ);
    if (ctx->buf == NULL)
        return RYM_ERR_MEM;

    err = _rym_do_send_handshake(ctx, handshake_timeout);
    if (err != RYM_OK)
    {
        rym_free(ctx->buf);
        return err;
    }

    err = _rym_do_send_trans(ctx);
    if (err != RYM_OK)
    {
        rym_free(ctx->buf);
        return err;
    }

    err = _rym_do_send_fin(ctx);
    if (err != RYM_OK)
    {
        rym_free(ctx->buf);
        return err;
    }

    rym_free(ctx->buf);
    return err;
}

void rym_set_rx_indicate(bool flag)
{
    rym_enter_critical();
    rym_rx_indi = flag ? true : false;
    rym_exit_critical();
}

int rym_recv(rym_ctx_t ctx, rym_callback on_begin, rym_callback on_data, rym_callback on_end,
    int handshake_timeout)
{
    int res = RYM_ERROR;
    
    if ((ctx == NULL) || (rym_semp_create(&(ctx->sem)) != true)) 
    {
        return res;
    }

    ctx->on_begin = on_begin;
    ctx->on_data  = on_data;
    ctx->on_end   = on_end;
    ctx->seq      = 0x01;

    rym_ctx = ctx;
    rym_set_rx_indicate(true);

    if (rym_serial_lock(RYM_DEV) == true)
    {
        res = _rym_do_recv(ctx, handshake_timeout);
        _rym_clear_buffer(ctx);
        rym_serial_unlock(RYM_DEV);
    }

    rym_set_rx_indicate(false);
    rym_semp_delete(&(ctx->sem));
    rym_ctx = NULL;
    return res;
}

int rym_send(rym_ctx_t ctx, rym_callback on_begin, rym_callback on_data, rym_callback on_end,
    int handshake_timeout)
{
    int res = RYM_ERROR;

    if ((ctx == NULL) || (rym_semp_create(&(ctx->sem)) != true))
    {
        return res;
    } 
    
    ctx->on_begin = on_begin;
    ctx->on_data  = on_data;
    ctx->on_end   = on_end;
    ctx->seq      = 0x01;

    rym_ctx = ctx;   
    rym_set_rx_indicate(true);

    if (rym_serial_lock(RYM_DEV) == true)
    {
        res = _rym_do_send(ctx, handshake_timeout);
        _rym_clear_buffer(ctx);
        rym_serial_unlock(RYM_DEV);
    }

    rym_set_rx_indicate(false);
    rym_semp_delete(&(ctx->sem));
    rym_ctx = NULL;
    return res;
}

