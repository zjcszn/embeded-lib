#include "bsp_led.h"
#include "lwwdg.h"
#include "stdbool.h"
#include "boot_osal.h"

#define LOG_LVL   ELOG_LVL_DEBUG
#define LOG_TAG  "LED"

#include "boot_cfg.h"

static uint32_t blink_delay = 300;

_Noreturn static void bsp_led_task(void *args) {
    (void) args;
    static lwwdg_wdg_t  wdg_led;

    lwwdg_add(&wdg_led, BSP_WDG_TASK_TIMEOUT);
    for (int led = 1; ; led = !led)
    {
        LED_GPIO_PORT->BSRR = LED_GPIO_PIN << (led << 4);
        rtos_delayms(blink_delay);
        lwwdg_reload(&wdg_led);
    }
}

void bsp_led_init(void) 
{
    LED_CLOCK_ENABLE();
    LL_GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin = LED_GPIO_PIN;
    gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW; 
    LL_GPIO_Init(LED_GPIO_PORT, &gpio_init);
    xTaskCreate(bsp_led_task, "LED Signal", LED_TASK_STK_SIZE, NULL, 1, NULL);
}

void set_led_freq(uint32_t freq)
{
    blink_delay = freq;
}




