#ifndef SOFT_I2C_SLAVE_H
#define SOFT_I2C_SLAVE_H

#include <stdbool.h>
#include <stdint.h>


/**
 * @brief I2C Slave events
 */
typedef enum {
    I2C_SLAVE_EV_START, /* START/RESTART detected */
    I2C_SLAVE_EV_RECV,  /* Data received from master */
    I2C_SLAVE_EV_TRNS,  /* Master request data (transmit) */
    I2C_SLAVE_EV_STOP,  /* STOP detected */
    I2C_SLAVE_EV_ERR    /* Protocol error or NACK */
} i2c_slave_ev_t;

/**
 * @brief I2C Slave status
 */
typedef enum {
    I2C_SLAVE_IDLE,
    I2C_SLAVE_ADDR_MATCH,
    I2C_SLAVE_DATA_TX,
    I2C_SLAVE_DATA_RX,
    I2C_SLAVE_WAIT_TX_ACK,
    I2C_SLAVE_SEND_RX_ACK
} i2c_slave_state_t;

/**
 * @brief I2C Slave hardware operations
 */
typedef struct {
    uint8_t (*read_scl)(void);
    uint8_t (*read_sda)(void);
    void (*set_sda_out)(void);
    void (*set_sda_in)(void);
    void (*write_sda)(uint8_t level);
    void (*delay)(void);
} i2c_slave_ops_t;

/**
 * @brief I2C Slave configuration
 */
typedef struct {
    uint8_t addr;
    const i2c_slave_ops_t *ops;
    void (*callback)(i2c_slave_ev_t ev, uint8_t *data);
} i2c_slave_cfg_t;

/**
 * @brief I2C Slave handle
 */
typedef struct {
    i2c_slave_state_t state;
    uint8_t addr;    /* Device address (matched against) */
    uint8_t rx_addr; /* Received address byte */
    uint8_t shift_reg;
    uint8_t bits; /* Bit counter */
    bool is_read;
    const i2c_slave_ops_t *ops;
    void (*callback)(i2c_slave_ev_t ev, uint8_t *data);
} i2c_slave_t;

/* API Functions */

/**
 * @brief Initialize the I2C slave handle
 */
int32_t i2c_slave_init(i2c_slave_t *i2c, const i2c_slave_cfg_t *cfg);

/**
 * @brief SCL line edge interrupt handler
 */
void i2c_slave_scl_handler(i2c_slave_t *i2c);

/**
 * @brief SDA line edge interrupt handler
 */
void i2c_slave_sda_handler(i2c_slave_t *i2c);

#endif /* SOFT_I2C_SLAVE_H */
