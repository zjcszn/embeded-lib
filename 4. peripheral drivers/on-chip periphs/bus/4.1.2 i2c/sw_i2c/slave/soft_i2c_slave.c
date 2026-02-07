#include "soft_i2c_slave.h"

#include <stddef.h>

/* Internal utility to reset state */
static void i2c_slave_reset(i2c_slave_t *i2c) {
    i2c->state = I2C_SLAVE_IDLE;
    i2c->bits = 0;
    i2c->shift_reg = 0;
    if (i2c->ops->set_sda_in != NULL) {
        i2c->ops->set_sda_in();
    }
}

int32_t i2c_slave_init(i2c_slave_t *i2c, const i2c_slave_cfg_t *cfg) {
    if ((i2c == NULL) || (cfg == NULL) || (cfg->ops == NULL)) {
        return -1;
    }

    i2c->addr = cfg->addr & 0xFEu;
    i2c->ops = cfg->ops;
    i2c->callback = cfg->callback;

    i2c_slave_reset(i2c);

    return 0;
}

void i2c_slave_scl_handler(i2c_slave_t *i2c) {
    if ((i2c == NULL) || (i2c->ops == NULL)) {
        return;
    }

    /* SCL Rising Edge: Sample Data */
    if (i2c->ops->read_scl() != 0u) {
        switch (i2c->state) {
            case I2C_SLAVE_IDLE:
                /* Wait for START */
                break;

            case I2C_SLAVE_ADDR_MATCH:
            case I2C_SLAVE_DATA_RX:
                if (i2c->bits < 8u) {
                    i2c->shift_reg <<= 1;
                    if (i2c->ops->read_sda() != 0u) {
                        i2c->shift_reg |= 0x01u;
                    }
                    i2c->bits++;
                }
                break;

            case I2C_SLAVE_WAIT_TX_ACK:
                /* Master Sents ACK/NACK */
                if (i2c->ops->read_sda() == 0u) {
                    /* ACK received, continue transmission */
                    if (i2c->callback != NULL) {
                        i2c->callback(I2C_SLAVE_EV_TRNS, &(i2c->shift_reg));
                    }
                    i2c->state = I2C_SLAVE_DATA_TX;
                    i2c->bits = 0;
                } else {
                    /* NACK received, end transmission */
                    i2c_slave_reset(i2c);
                }
                break;

            default:
                break;
        }
    }
    /* SCL Falling Edge: Change Data / Prepare ACK */
    else {
        switch (i2c->state) {
            case I2C_SLAVE_ADDR_MATCH:
                if (i2c->bits >= 8u) {
                    i2c->rx_addr = i2c->shift_reg;
                    if ((i2c->rx_addr & 0xFEu) == i2c->addr) {
                        i2c->is_read = ((i2c->rx_addr & 0x01u) != 0u);
                        /* Send ACK */
                        i2c->ops->set_sda_out();
                        i2c->ops->write_sda(0);
                        i2c->state = I2C_SLAVE_SEND_RX_ACK;

                        if (i2c->callback != NULL) {
                            i2c->callback(I2C_SLAVE_EV_START, &(i2c->rx_addr));
                        }
                    } else {
                        i2c_slave_reset(i2c);
                    }
                }
                break;

            case I2C_SLAVE_DATA_RX:
                if (i2c->bits >= 8u) {
                    /* Prepare ACK */
                    i2c->ops->set_sda_out();
                    i2c->ops->write_sda(0);
                    i2c->state = I2C_SLAVE_SEND_RX_ACK;

                    if (i2c->callback != NULL) {
                        i2c->callback(I2C_SLAVE_EV_RECV, &(i2c->shift_reg));
                    }
                }
                break;

            case I2C_SLAVE_SEND_RX_ACK:
                i2c->bits = 0;
                i2c->shift_reg = 0;
                if (i2c->is_read) {
                    /* Switch to TX */
                    if (i2c->callback != NULL) {
                        i2c->callback(I2C_SLAVE_EV_TRNS, &(i2c->shift_reg));
                    }
                    i2c->state = I2C_SLAVE_DATA_TX;
                    /* First bit of TX data */
                    i2c->ops->set_sda_out();
                    i2c->ops->write_sda((i2c->shift_reg & 0x80u) ? 1u : 0u);
                } else {
                    /* Continue RX */
                    i2c->ops->set_sda_in();
                    i2c->state = I2C_SLAVE_DATA_RX;
                }
                break;

            case I2C_SLAVE_DATA_TX:
                if (i2c->bits < 7u) {
                    i2c->bits++;
                    i2c->ops->write_sda((i2c->shift_reg & (0x80u >> i2c->bits)) ? 1u : 0u);
                } else {
                    /* Prepare for Master ACK */
                    i2c->ops->set_sda_in();
                    i2c->state = I2C_SLAVE_WAIT_TX_ACK;
                }
                break;

            default:
                break;
        }
    }
}

void i2c_slave_sda_handler(i2c_slave_t *i2c) {
    if ((i2c == NULL) || (i2c->ops == NULL)) {
        return;
    }

    /* START/STOP only valid when SCL is HIGH */
    if (i2c->ops->read_scl() != 0u) {
        if (i2c->ops->read_sda() == 0u) {
            /* SDA Falling Edge when SCL is High -> START */
            i2c->state = I2C_SLAVE_ADDR_MATCH;
            i2c->bits = 0;
            i2c->shift_reg = 0;
        } else {
            /* SDA Rising Edge when SCL is High -> STOP */
            if (i2c->callback != NULL) {
                i2c->callback(I2C_SLAVE_EV_STOP, NULL);
            }
            i2c_slave_reset(i2c);
        }
    }
}
