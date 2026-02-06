#ifndef __SW_I2C_PORT_H__
#define __SW_I2C_PORT_H__

#include "sw_i2c.h"

#ifdef USE_HAL_DRIVER
/**
 * @brief  I2C Port Configuration Structure
 *         Holds platform-specific resource handles and pin definitions.
 */
typedef struct {
    void *scl_port;    ///< GPIO Port for SCL (Platform dependent, e.g. GPIO_TypeDef*)
    uint32_t scl_pin;  ///< GPIO Pin for SCL
    void *sda_port;    ///< GPIO Port for SDA (Platform dependent, e.g. GPIO_TypeDef*)
    uint32_t sda_pin;  ///< GPIO Pin for SDA
} sw_i2c_port_cfg_t;

/**
 * @brief  Helper to initialize a default I2C instance using the default GPIO implementation
 *         Allocates a static context and initializes hardware based on provided config.
 * @param  i2c_dev    Device handle
 * @param  cfg        Configuration structure with port/pin details
 * @param  freq_khz   I2C Frequency in kHz
 * @param  sys_clk_hz System Clock in Hz (pass 0 to auto-detect)
 * @return sw_i2c_err_t
 */
sw_i2c_err_t sw_i2c_init_default(sw_i2c_t *i2c_dev, const sw_i2c_port_cfg_t *cfg, uint32_t freq_khz,
                                 uint32_t sys_clk_hz);
#endif

#endif /* __SW_I2C_PORT_H__ */
