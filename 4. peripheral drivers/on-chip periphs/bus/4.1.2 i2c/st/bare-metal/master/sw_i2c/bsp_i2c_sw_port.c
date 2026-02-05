
#include "bsp_i2c_sw.h"

#ifdef USE_HAL_DRIVER
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"

/* perf_counter removed as delay is now handled internally by NOPs */

/* Internal Context ----------------------------------------------------------*/

typedef struct {
    GPIO_TypeDef *scl_port;
    uint32_t scl_pin;
    GPIO_TypeDef *sda_port;
    uint32_t sda_pin;
} stm32_i2c_ctx_t;

/* Private functions ---------------------------------------------------------*/

static void stm32_set_scl(void *handle, uint8_t state) {
    stm32_i2c_ctx_t *ctx = (stm32_i2c_ctx_t *) handle;
    if (state)
        LL_GPIO_SetOutputPin(ctx->scl_port, ctx->scl_pin);
    else
        LL_GPIO_ResetOutputPin(ctx->scl_port, ctx->scl_pin);
}

static void stm32_set_sda(void *handle, uint8_t state) {
    stm32_i2c_ctx_t *ctx = (stm32_i2c_ctx_t *) handle;
    if (state)
        LL_GPIO_SetOutputPin(ctx->sda_port, ctx->sda_pin);
    else
        LL_GPIO_ResetOutputPin(ctx->sda_port, ctx->sda_pin);
}

static uint8_t stm32_get_scl(void *handle) {
    stm32_i2c_ctx_t *ctx = (stm32_i2c_ctx_t *) handle;
    return LL_GPIO_IsInputPinSet(ctx->scl_port, ctx->scl_pin);
}

static uint8_t stm32_get_sda(void *handle) {
    stm32_i2c_ctx_t *ctx = (stm32_i2c_ctx_t *) handle;
    return LL_GPIO_IsInputPinSet(ctx->sda_port, ctx->sda_pin);
}

static const sw_i2c_ops_t stm32_ops = {
    .set_scl = stm32_set_scl,
    .set_sda = stm32_set_sda,
    .get_scl = stm32_get_scl,
    .get_sda = stm32_get_sda,
};

static void soft_i2c_gpio_clk_enable(GPIO_TypeDef *gpio_x) {
    if (gpio_x == GPIOA)
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    else if (gpio_x == GPIOB)
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
    else if (gpio_x == GPIOC)
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    else if (gpio_x == GPIOD)
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
    else if (gpio_x == GPIOE)
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
    else if (gpio_x == GPIOF)
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF);
    else if (gpio_x == GPIOG)
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOG);
    else if (gpio_x == GPIOH)
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);
    else if (gpio_x == GPIOI)
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOI);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Helper to initialize a default I2C instance using the internal STM32 LL implementation
 */
sw_i2c_err_t sw_i2c_init_default(sw_i2c_t *i2c_dev, GPIO_TypeDef *scl_port, uint32_t scl_pin,
                                 GPIO_TypeDef *sda_port, uint32_t sda_pin, uint32_t freq_khz,
                                 uint32_t sys_clk_hz) {
    static stm32_i2c_ctx_t pool[4];
    static uint8_t pool_idx = 0;

    if (pool_idx >= 4)
        return SOFT_I2C_ERR_BUS;  // Out of instances

    stm32_i2c_ctx_t *ctx = &pool[pool_idx++];
    ctx->scl_port = scl_port;
    ctx->scl_pin = scl_pin;
    ctx->sda_port = sda_port;
    ctx->sda_pin = sda_pin;

    // Init GPIO hardware
    soft_i2c_gpio_clk_enable(scl_port);
    soft_i2c_gpio_clk_enable(sda_port);

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Pin = scl_pin;
    LL_GPIO_Init(scl_port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = sda_pin;
    LL_GPIO_Init(sda_port, &GPIO_InitStruct);

    // Auto-detect system clock if user passes 0
    if (sys_clk_hz == 0) {
        // Assume SystemCoreClock is available in STM32 environment
        // It's a standard CMSIS variable.
        extern uint32_t SystemCoreClock;
        sys_clk_hz = SystemCoreClock;
    }

    return sw_i2c_init(i2c_dev, &stm32_ops, ctx, freq_khz, sys_clk_hz);
}

#endif
