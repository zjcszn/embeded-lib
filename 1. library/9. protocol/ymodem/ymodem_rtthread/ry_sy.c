/*
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-09     Steven Liu   the first version
 * 2021-04-14     Meco Man     Check the file path's legitimacy of 'sy' command
 */

#include <ymodem.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ff.h"
#include "shell.h"

#define   LOG_TAG        "YMODEM"
#define   LOG_LVL         ELOG_LVL_ERROR

#include "elog.h"

#define DFS_PATH_MAX    255

struct custom_ctx
{
    struct _rym_ctx parent;
    FIL fd;
    unsigned long flen;
    unsigned char fpath_len;
    char fpath[DFS_PATH_MAX];
};

static int rym_fpath_cpy(char *dst, const char *src, unsigned long max_len)
{
    if (max_len > 0)
    {
        int i;
        for (i = 1; (*src) && (i < max_len); i++)
        {
            *dst++ = *src++;
        }
        *dst = '\0';

        return i;    
    }
    return 0;
}

static enum rym_code _rym_recv_begin(
    struct _rym_ctx *ctx,
    unsigned char *buf,
    unsigned long len)
{
    FRESULT fres;
    struct custom_ctx *cctx = (struct custom_ctx *)ctx;
    
    if (strlen((const char*)buf) > (DFS_PATH_MAX - (cctx->fpath_len + 1)))
    {
        return RYM_CODE_CAN;
    }

    memset(cctx->fpath + cctx->fpath_len, 0, DFS_PATH_MAX - cctx->fpath_len);
    strcpy(cctx->fpath, (const char*)buf);

    fres = f_open(&(cctx->fd), cctx->fpath, FA_WRITE | FA_CREATE_ALWAYS);
    if (fres != FR_OK)
    {
        log_e("error creating file: %d", fres);
        return RYM_CODE_CAN;
	}
    
    cctx->flen = strtoul((const char*)(buf + strlen((const char*)buf) + 1), NULL, 10);
    if (cctx->flen == 0)
        cctx->flen = -1;

    return RYM_CODE_ACK;
}

static enum rym_code _rym_recv_data(
    struct _rym_ctx *ctx,
    unsigned char *buf,
    unsigned long len)
{
    unsigned int bw;
    FRESULT fres;
    struct custom_ctx *cctx = (struct custom_ctx *)ctx;

    if (cctx->flen == -1)
    {
        fres = f_write(&(cctx->fd), buf, len, &bw);
    }
    else
    {
        int wlen = len > cctx->flen ? cctx->flen : len;
        fres = f_write(&(cctx->fd), buf, wlen, &bw);
        cctx->flen -= wlen;
    }
    
    if ((fres != FR_OK) || (f_sync(&(cctx->fd)) != FR_OK))
    {
        log_e("error write file: %d", fres);
        return RYM_CODE_CAN;
    }
    return RYM_CODE_ACK;
}

static enum rym_code _rym_recv_end(
    struct _rym_ctx *ctx,
    unsigned char *buf,
    unsigned long len)
{
    struct custom_ctx *cctx = (struct custom_ctx *)ctx;

    f_close(&(cctx->fd));
    return RYM_CODE_ACK;
}


static enum rym_code _rym_send_begin(
    struct _rym_ctx *ctx,
    unsigned char *buf,
    unsigned long len)
{
    FRESULT fres;
    struct custom_ctx *cctx = (struct custom_ctx *)ctx;

    fres = f_open(&(cctx->fd), cctx->fpath, FA_READ | FA_OPEN_EXISTING);
    if (fres != FR_OK)
    {
        log_e("error open file: %d\n", fres);
        return RYM_ERR_FILE;
    }
    memset(buf, 0, len);

    FILINFO *finfo = rym_malloc(sizeof(FILINFO));
    if (finfo == NULL)
    {
        return RYM_ERR_NOMEM;
    }

    if (f_stat(cctx->fpath, finfo) != FR_OK)
    {
        rym_free(finfo);
        return RYM_ERR_FILE;
    }

    sprintf((char *)buf, "%s%c%llu", finfo->fname, '\0', finfo->fsize);
    rym_free(finfo);
    return RYM_CODE_SOH;
}

static enum rym_code _rym_send_data(
    struct _rym_ctx *ctx,
    unsigned char *buf,
    unsigned long len)
{
    struct custom_ctx *cctx = (struct custom_ctx *)ctx;
    unsigned int br;
    FRESULT fres;

    fres = f_read(&(cctx->fd), buf, len, &br);
    if (fres != FR_OK)
    {
        log_e("error read file: %d", fres);
        return RYM_ERR_FILE;
    }

    if (br < len)
    {
        memset(buf + br, 0x1A, len - br);
        ctx->stage = RYM_STAGE_FINISHING;
        f_close(&(cctx->fd));
    }

    if (br > 128)
    {
        return RYM_CODE_STX;
    }
    return RYM_CODE_SOH;
}

static enum rym_code _rym_send_end(
    struct _rym_ctx *ctx,
    unsigned char *buf,
    unsigned long len)
{
    memset(buf, 0, len);

    return RYM_CODE_SOH;
}


static int rym_download_file(const char *file_path)
{
    int res;
    assert(file_path);

    struct custom_ctx *ctx = rym_calloc(1, sizeof(struct custom_ctx));

    if (!ctx)
    {
        log_e("rym malloc failed\n");
        return RYM_ERR_NOMEM;
    }

    ctx->fpath_len = rym_fpath_cpy(ctx->fpath, file_path ? file_path : "0:", DFS_PATH_MAX - 1);
    ctx->fpath[ctx->fpath_len] = '/';
    
    res = rym_recv(&ctx->parent, _rym_recv_begin, _rym_recv_data, _rym_recv_end, 100);
    rym_free(ctx);

    return res;
}

static int rym_upload_file(const char *file_path)
{
    int res = 0;
    assert(file_path);

    struct custom_ctx *ctx = rym_calloc(1, sizeof(*ctx));

    if (!ctx)
    {
        log_e("rym malloc failed\n");
        return RYM_ERR_NOMEM;
    }
    rym_fpath_cpy(ctx->fpath, file_path, DFS_PATH_MAX);

    res = rym_send(&ctx->parent, _rym_send_begin, _rym_send_data, _rym_send_end, 100);
    rym_free(ctx);

    return res;
}

static int ry(uint8_t argc, char **argv)
{
    int res;
    const char *file_path;
    
    file_path = argc < 2 ? NULL : argv[1];
    log_i("%s", file_path ? file_path : "0:/");
    res = rym_download_file(file_path);

    return res;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), ry, ry, ymodem receive e.g: ry file_path.);


static int sy(uint8_t argc, char **argv)
{
    int res;
    const char *file_path;

    if (argc < 2)
    {
        log_e("invalid file path.\n");
        return -RYM_ERROR;
    }

    file_path = argv[1];
    log_i("%s", file_path);
    res = rym_upload_file(file_path);

    return res;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), sy, sy, ymodem send e.g: sy file_path);


