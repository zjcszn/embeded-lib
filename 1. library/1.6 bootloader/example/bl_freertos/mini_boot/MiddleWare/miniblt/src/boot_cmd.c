#include <stdint.h>
#define LOG_LVL ELOG_LVL_INFO
#define LOG_TAG "BOOT"

#include "boot.h"
#include "components.h"

void system_reset_to_app(void) {
    if (firmware_verify_on_flash(FLASH_PARTITION_APP) == true) {
        rtos_delayms(10);
        boot_params_write(PARAM_INDEX_BOOT_FLAG, BOOT_FLAG_JMP_TO_APP);
        NVIC_SystemReset();
    }
    log_e("app verify failed, Can't jump to app");
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), reboot, system_reset_to_app, reboot to app);

void fs_mount(void) {
    static FATFS fdev_sd;
    FRESULT ret = f_mount(&fdev_sd, "0:", 1);
    log_i("fatfs mount %s [%u]", ret ? "failed" : "success", ret);
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), mount, fs_mount, mount file system);

void fs_unmount(void) {
    FRESULT ret = f_unmount("0:");
    log_i("fatfs unmount %s [%u]", ret ? "failed" : "success", ret);
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), unmount, fs_unmount, unmount file system);

void update_by_sdcard(const char *fpath) {
    if (fpath != NULL) {
        /* copy firmware to application partition */
        if (firmware_verify_on_file(fpath) && firmware_update_on_file(fpath, FLASH_PARTITION_APP)) {
            log_i("update firmware success");
            system_reset_to_app();
        }
    }
    log_i("update firmware failed");
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), update, update_by_sdcard, update firmware from sdcard);

void ls(void) {
    static DIR dir;
    static FILINFO fno;
    uint16_t year;
    uint8_t  month, day, hour, min, sec;

    if (f_opendir(&dir, "0:") == FR_OK) {
        elog_raw("\r\n%-32s%-24s%-8s\r\n", "Name", "Time", "Size");
        f_rewinddir(&dir);
        while ((f_readdir(&dir, &fno) == FR_OK) && fno.fname[0] != 0) {
            year = 1980 + (fno.fdate >> 9);
            month  = (fno.fdate & 0x01E0) >> 5;
            day = fno.fdate & 0x001F;
            hour = fno.ftime >> 11;
            min = (fno.ftime & 0x07E0) >> 5;
            sec = (fno.ftime & 0x001F) << 1;
            elog_raw("%-32s%04u/%02u/%02u %02u:%02u:%02u     %-8u\r\n",
                     fno.fname, year, month, day, hour, min, sec, fno.fsize);
        }
        f_closedir(&dir);
    } else {
        log_e("can't open dir, check if fatfs is mounted");
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC) | SHELL_CMD_DISABLE_RETURN, ls, ls, list directory);
