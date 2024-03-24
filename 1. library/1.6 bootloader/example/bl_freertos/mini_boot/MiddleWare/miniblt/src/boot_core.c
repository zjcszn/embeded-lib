#include "boot_cmd.h"
#include "stm32f4xx_ll_rcc.h"

#define   LOG_LVL   ELOG_LVL_DEBUG
#define   LOG_TAG  "BOOT"

#include "components.h"
#include "port.h"
#include "boot.h"


void user_bootloader_init(void *args) {
    uint32_t boot_flag = *(uint32_t *) args;

    /* init watch dog */
    //bsp_wdg_init();

    /* init uart & log */
    dev_uart_init();
    bsp_elog_init();

    /* check iwdg reset flag */
    if (LL_RCC_IsActiveFlag_IWDGRST()) {
        log_e("iwdg reset!");
        LL_RCC_ClearResetFlags();
    }

    /* check update flag */
    if (boot_flag == BOOT_FLAG_UPDATE_APP) {
        firmware_update();
    }

    /* check boot key */
    if (boot_flag == BOOT_FLAG_NONE && boot_key_detect() == false) {
        system_reset_to_app();
    }

    /* led signal init */
    bsp_led_init();
    /* mini shell init */
    bsp_shell_init();
    /* mount file system */
    fs_mount();
    rtos_thread_delete(NULL);
}


void user_bootloader_start(void *args) {
    rtos_thread_create("miniblt init", 4096, RTOS_MAX_PRIOR, user_bootloader_init, args);
    rtos_thread_start();
}
