
#include "sw_i2c.h"

#ifdef USE_HAL_DRIVER
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"

/* perf_counter removed as delay is now handled internally by NOPs */

/* Internal Context ----------------------------------------------------------*/

/* Internal Context ----------------------------------------------------------*/

typedef struct {
    void *scl_port;
    uint32_t scl_pin;
    void *sda_port;
    uint32_t sda_pin;
} i2c_gpio_ctx_t;

/* Private functions ---------------------------------------------------------*/

static void i2c_gpio_set_scl(void *handle, uint8_t state) {
    i2c_gpio_ctx_t *ctx = (i2c_gpio_ctx_t *) handle;
    if (state) {
        LL_GPIO_SetOutputPin((GPIO_TypeDef *) ctx->scl_port, ctx->scl_pin);
    } else {
        LL_GPIO_ResetOutputPin((GPIO_TypeDef *) ctx->scl_port, ctx->scl_pin);
    }
}

static void i2c_gpio_set_sda(void *handle, uint8_t state) {
    i2c_gpio_ctx_t *ctx = (i2c_gpio_ctx_t *) handle;
    if (state) {
        LL_GPIO_SetOutputPin((GPIO_TypeDef *) ctx->sda_port, ctx->sda_pin);
    } else {
        LL_GPIO_ResetOutputPin((GPIO_TypeDef *) ctx->sda_port, ctx->sda_pin);
    }
}

static uint8_t i2c_gpio_get_scl(void *handle) {
    i2c_gpio_ctx_t *ctx = (i2c_gpio_ctx_t *) handle;
    return LL_GPIO_IsInputPinSet((GPIO_TypeDef *) ctx->scl_port, ctx->scl_pin);
}

static uint8_t i2c_gpio_get_sda(void *handle) {
    i2c_gpio_ctx_t *ctx = (i2c_gpio_ctx_t *) handle;
    return LL_GPIO_IsInputPinSet((GPIO_TypeDef *) ctx->sda_port, ctx->sda_pin);
}

static const sw_i2c_ops_t i2c_gpio_ops = {
    .set_scl = i2c_gpio_set_scl,
    .set_sda = i2c_gpio_set_sda,
    .get_scl = i2c_gpio_get_scl,
    .get_sda = i2c_gpio_get_sda,
};

static void soft_i2c_gpio_clk_enable(GPIO_TypeDef *gpio_x) {
    if (gpio_x == GPIOA) {
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    } else if (gpio_x == GPIOB) {
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
    } else if (gpio_x == GPIOC) {
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    } else if (gpio_x == GPIOD) {
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
    } else if (gpio_x == GPIOE) {
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
    } else if (gpio_x == GPIOF) {
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF);
    } else if (gpio_x == GPIOG) {
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOG);
    } else if (gpio_x == GPIOH) {
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);
    } else if (gpio_x == GPIOI) {
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOI);
    }
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Helper to initialize a default I2C instance using the default GPIO implementation
 */
sw_i2c_err_t sw_i2c_init_default(sw_i2c_t *i2c_dev, const sw_i2c_port_cfg_t *cfg) {
    if (!i2c_dev || !cfg) {
        return SOFT_I2C_ERR_PARAM;
    }

    static i2c_gpio_ctx_t pool[4];
    static uint8_t pool_idx = 0;

    if (pool_idx >= 4) {
        return SOFT_I2C_ERR_BUS;  // Out of instances
    }

    i2c_gpio_ctx_t *ctx = &pool[pool_idx++];
    ctx->scl_port = cfg->scl_port;
    ctx->scl_pin = cfg->scl_pin;
    ctx->sda_port = cfg->sda_port;
    ctx->sda_pin = cfg->sda_pin;

    // Init GPIO hardware
    soft_i2c_gpio_clk_enable((GPIO_TypeDef *) cfg->scl_port);
    soft_i2c_gpio_clk_enable((GPIO_TypeDef *) cfg->sda_port);

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Pin = cfg->scl_pin;
    LL_GPIO_Init((GPIO_TypeDef *) cfg->scl_port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = cfg->sda_pin;
    LL_GPIO_Init((GPIO_TypeDef *) cfg->sda_port, &GPIO_InitStruct);

    uint32_t sys_clk_hz = cfg->sys_clk_hz;

    // Auto-detect system clock if user passes 0
    if (sys_clk_hz == 0) {
        // Assume SystemCoreClock is available in STM32 environment
        // It's a standard CMSIS variable.
        extern uint32_t SystemCoreClock;
        sys_clk_hz = SystemCoreClock;
    }

    return sw_i2c_init(i2c_dev, &i2c_gpio_ops, ctx, cfg->freq_khz, sys_clk_hz);
}

#endif
