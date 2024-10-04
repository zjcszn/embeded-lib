#ifndef __BOOT_H__
#define __BOOT_H__

#include <stdint.h>

#include "boot_cfg.h"
#include "boot_osal.h"
#include "boot_params.h"
#include "boot_core.h"
#include "boot_cmd.h"

#include "boot_update.h"
#include "boot_verify.h"

struct mbl_header
{
    /* magic word of file header */
    uint8_t type[4];
    /* firmware create time */
    uint32_t ctime;
    /* firmware version */
    uint32_t version;
    /* algorithm of crypto or compress */
    uint32_t algo;
    /* aes-256 crypto iv */
    uint8_t  aes_iv[16];
    /* firmware signature*/
    uint8_t  fw_sign[128];
    /* firmware raw size */
    uint32_t raw_size;
    /* firmware compress size */
    uint32_t com_size;
    /* firmware offset of 'mbl' file */
    uint32_t fw_offset;
    /* file header crc */
    uint32_t head_crc;
    /* padding bytes */
    uint8_t  padding[80];
};
typedef struct mbl_header *mbl_header_t;

#define MBL_HEADER_HEADCRC_OFFSET  ((uint32_t)(&(((mbl_header_t)0U)->head_crc)))
#define MBL_HEADER_FW_SIGN_OFFSET  ((uint32_t)(&(((mbl_header_t)0U)->fw_sign)))


typedef enum 
{
    BOOT_FLAG_NONE        = 0x0000,
    BOOT_FLAG_JMP_TO_APP  = 0xABCD,
    BOOT_FLAG_JMP_TO_IAP  = 0x1234,
    BOOT_FLAG_UPDATE_APP  = 0X5678,
    BOOT_FLAG_MINI_SHELL  = 0x254E,
} boot_flag_t;

typedef enum 
{
    PARAM_INDEX_BOOT_FLAG,
    /* other flags index */
} param_index_t;

typedef void(*reset_handler_t)(void);

_Noreturn void boot_start(void);

#endif