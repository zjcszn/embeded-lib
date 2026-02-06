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
     * - true: Fully supports clock stretching. If Slave holds SCL Low,
     *   Master waits up to 'ticks_timeout' loop cycles.
     *
     * - false (Default): Minimal support. Master only waits for 'ticks_setup' (approx 1/4 cycle)
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

/**
 * @brief  Reset the I2C master state
 *         (e.g., release bus, reset finite state machine if any)
 * @param  i2c_dev I2C device handle
 * @return sw_i2c_err_t SOFT_I2C_OK
 */
sw_i2c_err_t sw_i2c_master_reset(sw_i2c_t *i2c_dev);

/**
 * @brief  Master Transmit Data
 * @param  i2c_dev I2C device handle
 * @param  data    Pointer to data buffer
 * @param  len     Amount of data to be sent
 * @return sw_i2c_err_t SOFT_I2C_OK or error code
 */
sw_i2c_err_t sw_i2c_master_write(sw_i2c_t *i2c_dev, const uint8_t *data, uint16_t len);

/**
 * @brief  Enable or Disable Clock Stretching
 * @param  i2c_dev I2C device handle
 * @param  enable  true: Enable, false: Disable (Default)
 * @return sw_i2c_err_t
 */
sw_i2c_err_t sw_i2c_clock_stretch_enable(sw_i2c_t *i2c_dev, bool enable);

/**
 * @brief  Set the I2C Slave Address (7-bit)
 * @param  i2c_dev  I2C device handle
 * @param  dev_addr Slave address (7-bit, right aligned)
 * @return sw_i2c_err_t SOFT_I2C_OK
 */
sw_i2c_err_t sw_i2c_set_addr(sw_i2c_t *i2c_dev, uint16_t dev_addr);

/**
 * @brief  Get the current I2C Slave Address
 * @param  i2c_dev I2C device handle
 * @return uint16_t Current Slave address (7-bit)
 */
uint16_t sw_i2c_get_addr(sw_i2c_t *i2c_dev);

/**
 * @brief  Check if an I2C device is ready (ACKs address)
 * @param  i2c_dev  I2C device handle
 * @param  dev_addr Slave address to check
 * @return sw_i2c_err_t SOFT_I2C_OK if ready, SOFT_I2C_ERR_NACK if not
 */
sw_i2c_err_t sw_i2c_is_device_ready(sw_i2c_t *i2c_dev, uint16_t dev_addr);

/**
 * @brief  Master Transmit Data to specific memory address (Register Write)
 * @param  i2c_dev   I2C device handle
 * @param  mem_addr  Internal memory address/Register address
 * @param  addr_len  Address length (1: 8-bit, 2: 16-bit)
 * @param  data      Pointer to data buffer
 * @param  len       Amount of data to be sent
 * @return sw_i2c_err_t SOFT_I2C_OK or error code
 */
sw_i2c_err_t sw_i2c_master_mem_write(sw_i2c_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len,
                                     const uint8_t *data, uint16_t len);

/**
 * @brief  Master Receive Data
 * @param  i2c_dev I2C device handle
 * @param  data    Pointer to data buffer
 * @param  len     Amount of data to be received
 * @return sw_i2c_err_t SOFT_I2C_OK or error code
 */
sw_i2c_err_t sw_i2c_master_read(sw_i2c_t *i2c_dev, uint8_t *data, uint16_t len);

/**
 * @brief  Master Receive Data from specific memory address (Register Read)
 * @param  i2c_dev   I2C device handle
 * @param  mem_addr  Internal memory address/Register address
 * @param  addr_len  Address length (1: 8-bit, 2: 16-bit)
 * @param  data      Pointer to data buffer
 * @param  len       Amount of data to be received
 * @return sw_i2c_err_t SOFT_I2C_OK or error code
 */
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

/**
 * @brief  Write 8-bit value to an 8-bit register
 * @param  i2c_dev  I2C device handle
 * @param  reg_addr Register address (8-bit)
 * @param  val      Value to write (8-bit)
 * @return sw_i2c_err_t
 */
sw_i2c_err_t sw_i2c_write_reg8(sw_i2c_t *i2c_dev, uint8_t reg_addr, uint8_t val);

/**
 * @brief  Read 8-bit value from an 8-bit register
 * @param  i2c_dev  I2C device handle
 * @param  reg_addr Register address (8-bit)
 * @param  val      Pointer to store read value
 * @return sw_i2c_err_t
 */
sw_i2c_err_t sw_i2c_read_reg8(sw_i2c_t *i2c_dev, uint8_t reg_addr, uint8_t *val);

/**
 * @brief  Write 16-bit value to an 8-bit register
 *         (Writes 2 bytes: [Low, High] or [High, Low] depending on device?
 *          Defaulting to Big Endian [MSB, LSB] as is common in I2C)
 * @param  i2c_dev  I2C device handle
 * @param  reg_addr Register address (8-bit)
 * @param  val      Value to write (16-bit)
 * @return sw_i2c_err_t
 */
sw_i2c_err_t sw_i2c_write_reg16(sw_i2c_t *i2c_dev, uint8_t reg_addr, uint16_t val);

/**
 * @brief  Read 16-bit value from an 8-bit register
 *         (Reads 2 bytes: [MSB, LSB])
 * @param  i2c_dev  I2C device handle
 * @param  reg_addr Register address (8-bit)
 * @param  val      Pointer to store read value
 * @return sw_i2c_err_t
 */
sw_i2c_err_t sw_i2c_read_reg16(sw_i2c_t *i2c_dev, uint8_t reg_addr, uint16_t *val);

#endif /* __SW_I2C_H__ */