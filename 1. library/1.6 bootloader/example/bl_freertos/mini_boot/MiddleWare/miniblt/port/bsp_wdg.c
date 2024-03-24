#include "boot.h"
#include "components.h"
#include "lwwdg.h"
#include "stm32f4xx_ll_iwdg.h"

_Noreturn static void bsp_wdg_task(void *args) {
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while (1) {
        if (lwwdg_process()) {
            LL_IWDG_ReloadCounter(IWDG);
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(BSP_WDG_PERIOD));
    }
}

void bsp_wdg_init(void) {
    
    /* start core iwdg: max timeout 2048 ms */
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_32);
    LL_IWDG_SetReloadCounter(IWDG, 0x0FFF);
    LL_IWDG_DisableWriteAccess(IWDG);
    LL_IWDG_Enable(IWDG);

    /* init soft wdg */
    lwwdg_init();

    /* create feed dog task */
    xTaskCreate(bsp_wdg_task, "feed dog", 32, NULL, configMAX_PRIORITIES - 1, NULL);
}

