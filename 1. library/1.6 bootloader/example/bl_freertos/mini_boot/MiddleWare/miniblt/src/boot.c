#include "boot.h"
#include "components.h"

#include "main.h"
#include "system_stm32f4xx.h"

static uint32_t boot_flag;


static void jump_to_app(void);

static void jump_to_iap(void);

static void mcu_core_init(void);


_Noreturn void boot_start(void) {
    fal_init();
    boot_params_init();
    boot_params_read(PARAM_INDEX_BOOT_FLAG, &boot_flag);
    boot_params_reset();

    switch (boot_flag) {
        case BOOT_FLAG_JMP_TO_APP:
            jump_to_app();
            break;

        case BOOT_FLAG_JMP_TO_IAP:
            jump_to_iap();
            break;

        default:
            break;
    }

    mcu_core_init();
    user_bootloader_start(&boot_flag);

    while (1) {
        /* can not arrive here */
    }

}

/**
 * @brief init core clock
 * 
 */
static void mcu_core_init(void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    extern void SystemClock_Config(void);
    SystemClock_Config();

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(SysTick_IRQn);
    SysTick_Config(SystemCoreClock / 1000);
}

/**
 * @brief jump function
 *
 * @param _start vector start address
 */
static void system_jump(uint32_t _start) {
    static uint32_t vector;
    static reset_handler_t reset_handler;

    vector = _start;
    reset_handler = (reset_handler_t) (*(uint32_t *) (vector + 4));

    /* Set main stack pointer to initial stack value */
    __set_MSP(*(uint32_t *) vector);

    /* Use MSP and Privileged in thread mode */
    __set_CONTROL(0);

    /* Set NVIC vector table */
    SCB->VTOR = vector;

    /* Start the application, we will not return from this function */
    reset_handler();
}

/**
 * @brief jump to application
 * 
 */
static void jump_to_app(void) {
    fal_partition_t app_part;
    if ((app_part = fal_partition_find(FLASH_PARTITION_APP)) != NULL) {
        uint32_t app_base = FLASH_BASE + app_part->offset;
        system_jump(app_base + ((mbl_header_t) app_base)->fw_offset);
    }
    log_e("jump to application failed");
}

/**
 * @brief jump to system bootloader
 * 
 */
static void jump_to_iap(void) {
    system_jump(SYSTEM_BOOT_LOADER_ADDR);
    log_e("jump to system bootloader failed");
}








