#include "boot_osal.h"

#define LOG_LVL ELOG_LVL_INFO
#define LOG_TAG "BOOT"

#include "boot_verify.h"
#include "boot.h"
#include "boot_cmd.h"
#include "components.h"
#include "ff.h"
#include <stdint.h>

void firmware_update(void) {
    /* copy firmware to application partition */
    if (firmware_verify_on_flash(FLASH_PARTITION_DOWNLOAD) && firmware_update_on_flash()) {
        system_reset_to_app();
    }
    log_e("firmware update from download partition failed");
}

/**
 * @brief   copy firmware from download partition to app partition
 * @return  true on success | false on failed
 */
bool firmware_update_on_flash(void) {
    uint32_t repeat = 0;
    uint32_t btw;
    const uint8_t *src;
    fal_partition_t dst_part, src_part;

    if ((dst_part = fal_partition_find(FLASH_PARTITION_APP)) == NULL) {
        return false;
    }
    if ((src_part = fal_partition_find(FLASH_PARTITION_DOWNLOAD)) == NULL) {
        return false;
    }

    src = (const uint8_t *) (FLASH_BASE + src_part->offset);
    btw = sizeof(struct mbl_header) + ((mbl_header_t) (src))->raw_size;

    if (fal_partition_erase_all(dst_part) < 0) {
        return false;
    }

    while (fal_partition_write(dst_part, 0, src, btw) != btw) {
        if (repeat++ > 3) return false;
    }
    return true;
}

/**
 * @brief   copy firmware from sd card to on-chip flash
 * @param fpath firmware file path
 * @param part_name name of taget partition
 * @return true on success | false on failed
 */
bool firmware_update_on_file(const char *fpath, const char *part_name) {
    bool ret = false;
    if (!fpath) return ret;

    FIL *fw;
    uint8_t *f_buf;
    uint32_t wr_offset = 0;
    uint32_t br, repeat;

    fw = rtos_malloc(sizeof(FIL));
    f_buf = rtos_malloc(sizeof(uint8_t) * 1024);

    if (!fw || !f_buf) goto __exit;

    if (f_open(fw, fpath, FA_READ | FA_OPEN_EXISTING) == FR_OK) {
        fal_partition_t part = fal_partition_find(part_name);
        if (part != NULL) {
            fal_partition_erase_all(part);
            f_lseek(fw, 0);
            while ((f_read(fw, f_buf, 512, &br) == FR_OK) && br) {
                repeat = 0;
                while (fal_partition_write(part, wr_offset, f_buf, br) != br) {
                    if (repeat++ > 3) return false;
                }
                wr_offset += br;
            }
            ret = true;
        }
        f_close(fw);
    }

    __exit:
    if (f_buf) rtos_free(f_buf);
    if (fw) rtos_free(fw);
    return ret;
}



