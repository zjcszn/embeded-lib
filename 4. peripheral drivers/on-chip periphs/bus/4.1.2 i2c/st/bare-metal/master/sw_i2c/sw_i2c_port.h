#ifndef __SW_I2C_PORT_H__
#define __SW_I2C_PORT_H__

#include "sw_i2c.h"

#ifdef USE_HAL_DRIVER
#include "stm32h7xx_ll_gpio.h"

/**
 * @brief  Helper to initialize a default I2C instance using the internal STM32 LL implementation
 * @param  i2c_dev    Device handle
 * @param  scl_port   GPIO Port for SCL
 * @param  scl_pin    GPIO Pin for SCL
 * @param  sda_port   GPIO Port for SDA
 * @param  sda_pin    GPIO Pin for SDA
 * @param  freq_khz   I2C Frequency in kHz
 * @param  sys_clk_hz System Clock in Hz (pass 0 to auto-detect via SystemCoreClock)
 * @return sw_i2c_err_t
 */
sw_i2c_err_t sw_i2c_init_default(sw_i2c_t *i2c_dev, GPIO_TypeDef *scl_port, uint32_t scl_pin,
                                 GPIO_TypeDef *sda_port, uint32_t sda_pin, uint32_t freq_khz,
                                 uint32_t sys_clk_hz);
#endif

#endif /* __SW_I2C_PORT_H__ */
