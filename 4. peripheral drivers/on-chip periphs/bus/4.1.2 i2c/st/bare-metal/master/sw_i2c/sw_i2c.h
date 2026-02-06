#ifndef __SW_I2C_H__
#define __SW_I2C_H__

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported macro ------------------------------------------------------------*/

// Default to no log/assert if not defined externally
#ifndef SOFT_I2C_USE_ASSERT
#define SOFT_I2C_USE_ASSERT 0
#endif

#ifndef SOFT_I2C_USE_LOG
#define SOFT_I2C_USE_LOG 0
#endif

/* Exported types ------------------------------------------------------------*/

typedef enum {
    SOFT_I2C_OK,
    SOFT_I2C_ERR_ADDR,
    SOFT_I2C_ERR_NACK,
    SOFT_I2C_ERR_TIMEOUT,
    SOFT_I2C_ERR_BUS,
    SOFT_I2C_ERR_PARAM,
} sw_i2c_err_t;

/**
 * @brief Software I2C Operations Interface
 * Users must implement these hardware-dependent functions.
 */
typedef struct {
    /**
     * @brief Set SCL pin state
     * @param handle User defined handle passed to init
     * @param state 1: High (Float/Input), 0: Low (Output Low)
     */
    void (*set_scl)(void *handle, uint8_t state);

    /**
     * @brief Set SDA pin state
     * @param handle User defined handle passed to init
     * @param state 1: High (Float/Input), 0: Low (Output Low)
     */
    void (*set_sda)(void *handle, uint8_t state);

    /**
     * @brief Get SCL pin state (for clock stretching)
     * @param handle User defined handle passed to init
     * @return 1: High, 0: Low
     */
    uint8_t (*get_scl)(void *handle);

    /**
     * @brief Get SDA pin state
     * @param handle User defined handle passed to init
     * @return 1: High, 0: Low
     */
    uint8_t (*get_sda)(void *handle);

} sw_i2c_ops_t;

/**
 * @brief Software I2C Device Handle
 */
typedef struct {
    sw_i2c_ops_t ops;     // Low-level operations
    void *user_data;      // User context passed to ops callbacks
    uint16_t dev_addr;    // Slave address (7-bit)
    uint32_t freq_khz;    // Target frequency in kHz
    uint32_t sys_clk_hz;  // System Clock Frequency in Hz

    /* Timing Parameters (internally calculated Ticks) */
    uint32_t ticks_hold;     // Ticks after SCL falling edge
    uint32_t ticks_setup;    // Ticks before SCL rising edge
    uint32_t ticks_high;     // Ticks for SCL High period
    uint32_t ticks_timeout;  // Ticks/Loops for Clock Stretching Timeout

    /**
     * @brief Enable/Disable Clock Stretching support
     *
     * - true (Default): Fully supports clock stretching. If Slave holds SCL Low,
     *   Master waits up to 'ticks_timeout' loop cycles.
     *
     * - false: Minimal support. Master only waits for 'ticks_setup' (approx 1/4 cycle)
     *   to allow for physical rise time. If SCL remains Low after this short period,
     *   it considers the bus stuck/error and returns immediately.
     *   Useful for preventing Master blocking in bare-metal systems with faulty slaves.
     */
    bool enable_clock_stretch;  // Enable/Disable Clock Stretching support
} sw_i2c_t;

/* Exported function declaration ---------------------------------------------*/

/**
 * @brief Initialize Software I2C instance
 *
 * @param i2c_dev    I2C Device Handle
 * @param ops        Function pointers for hardware operations
 * @param user_data  User context to be passed to callbacks (can be NULL)
 * @param freq_khz   Bus frequency in kHz (e.g., 100, 400)
 * @param sys_clk_hz System Clock Frequency in Hz (e.g., 400000000 for 400MHz)
 *                   If 0, may attempt auto-detect if supported by port helper
 * @return sw_i2c_err_t SOFT_I2C_OK on success, ERR_PARAM if sys_clk too low
 */
sw_i2c_err_t sw_i2c_init(sw_i2c_t *i2c_dev, const sw_i2c_ops_t *ops, void *user_data,
                         uint32_t freq_khz, uint32_t sys_clk_hz);

/**
 * @brief Change I2C Bus Frequency
 *
 * @param i2c_dev   I2C Device Handle
 * @param freq_khz  New frequency in kHz
 * @return sw_i2c_err_t OK or ERR_PARAM
 */
sw_i2c_err_t sw_i2c_set_speed(sw_i2c_t *i2c_dev, uint32_t freq_khz);

/**
 * @brief Scan for I2C devices on the bus
 * Prints detected addresses to log if enabled.
 *
 * @param i2c_dev I2C device handle
 * @return sw_i2c_err_t SOFT_I2C_OK
 */
sw_i2c_err_t sw_i2c_scan(sw_i2c_t *i2c_dev);

sw_i2c_err_t sw_i2c_master_reset(sw_i2c_t *i2c_dev);

sw_i2c_err_t sw_i2c_master_write(sw_i2c_t *i2c_dev, const uint8_t *data, uint16_t len);

sw_i2c_err_t sw_i2c_master_mem_write(sw_i2c_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len,
                                     const uint8_t *data, uint16_t len);

sw_i2c_err_t sw_i2c_master_read(sw_i2c_t *i2c_dev, uint8_t *data, uint16_t len);

sw_i2c_err_t sw_i2c_master_mem_read(sw_i2c_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len,
                                    uint8_t *data, uint16_t len);

/**
 * @brief  Check if I2C bus is stuck (SDA Low when SCL High)
 * @param  i2c_dev I2C device handle
 * @return sw_i2c_err_t SOFT_I2C_ERR_BUS if stuck, SOFT_I2C_OK otherwise
 */
sw_i2c_err_t sw_i2c_check_stuck(sw_i2c_t *i2c_dev);

/**
 * @brief  Attempt to unlock the I2C bus by toggling SCL
 *         to clock out stuck data bits from slave.
 * @param  i2c_dev I2C device handle
 * @return sw_i2c_err_t
 */
sw_i2c_err_t sw_i2c_unlock(sw_i2c_t *i2c_dev);

#endif /* __SW_I2C_H__ */