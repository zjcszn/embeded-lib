#include "boot_cfg.h"

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_hal.h"


void bsp_key_init(void) 
{
    BOOT_KEY_CLOCK_EN();
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = BOOT_KEY_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
    LL_GPIO_Init(BOOT_KEY_PORT, &GPIO_InitStruct);
}


bool boot_key_detect(void) 
{
    bsp_key_init();

    int timeout = BOOT_KEY_DETECT_TIME / 10;
    while (LL_GPIO_IsInputPinSet(BOOT_KEY_PORT, BOOT_KEY_PIN) == BOOT_KEY_ACTIVE_LVL)
    {
        if (--timeout == 0) return true;
        HAL_Delay(10);
    }
    return false;
}



