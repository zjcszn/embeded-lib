#include "soft_i2c_slave.h"

i2c_slave_dev_t i2c_slave;

void soft_i2c_slave_gpio_init(void);

void soft_i2c_slave_exti_init(void);

void soft_i2c_slave_init(uint8_t addr, uint8_t *tx_buf) {
    i2c_slave.dev_addr = addr & 0xFE;
    i2c_slave.tx_buf = tx_buf;

    soft_i2c_slave_gpio_init();
    soft_i2c_slave_exti_init();
}

void soft_i2c_slave_gpio_init(void) {
    I2C_GPIO_CLOCK(I2C_SLAVE_SCL);
    I2C_GPIO_CLOCK(I2C_SLAVE_SDA);
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Pin = I2C_GPIO_PIN(I2C_SLAVE_SCL);
    LL_GPIO_Init(I2C_GPIO_PORT(I2C_SLAVE_SCL), &GPIO_InitStruct);

    GPIO_InitStruct.Pin = I2C_GPIO_PIN(I2C_SLAVE_SDA);
    LL_GPIO_Init(I2C_GPIO_PORT(I2C_SLAVE_SDA), &GPIO_InitStruct);
}

void soft_i2c_slave_exti_init(void) {
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

    LL_SYSCFG_SetEXTISource(I2C_SYS_EPORT(I2C_SLAVE_SCL), I2C_SYS_ELINE(I2C_SLAVE_SCL));
    LL_SYSCFG_SetEXTISource(I2C_SYS_EPORT(I2C_SLAVE_SDA), I2C_SYS_ELINE(I2C_SLAVE_SDA));

    EXTI_InitStruct.Line_0_31 = I2C_EXTI_LINE(I2C_SLAVE_SCL) | I2C_EXTI_LINE(I2C_SLAVE_SDA);
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);

    NVIC_SetPriority(I2C_EXTI_IRQ(I2C_SLAVE_SCL), NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(I2C_EXTI_IRQ(I2C_SLAVE_SCL));
    NVIC_SetPriority(I2C_EXTI_IRQ(I2C_SLAVE_SDA), NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(I2C_EXTI_IRQ(I2C_SLAVE_SDA));
}

void soft_i2c_slave_scl_exti_callback(void) {
    /* scl rising */
    if (READ_SCL() == 1u) {
        switch (i2c_slave.state) {
            case I2C_RECV_ADDR:
            case I2C_RECV_DATA:
                if (i2c_slave.bitcnt++ < 8) {
                    i2c_slave.rx_tmp <<= 1;
                    i2c_slave.rx_tmp |= READ_SDA();
                }
                break;

            case I2C_WAIT_DATA_ACK:
                /* master send ack*/
                if (READ_SDA() == 0u) {
                    i2c_slave.state = I2C_SEND_DATA;
                }
                /* master send nack */
                else {
                    i2c_slave.state = I2C_IDLE;
                }
                break;

            default:
                break;
        }
    }
        /* scl falling */
    else {
        switch (i2c_slave.state) {
            case I2C_SEND_DATA:
                if (i2c_slave.bitcnt < 8) {
                    SET_SDA_OUT();
                    if (*i2c_slave.tx_cur & (1 << (7 - i2c_slave.bitcnt))) {
                        SET_SDA_HIGH();
                    } else {
                        SET_SDA_LOW();
                    }
                    i2c_slave.bitcnt++;
                } else {
                    SET_SDA_IN();
                    i2c_slave.bitcnt = 0;
                    i2c_slave.tx_cur++;
                    i2c_slave.state = I2C_WAIT_DATA_ACK;
                }
                break;

            case I2C_RECV_ADDR:
                if (i2c_slave.bitcnt >= 8) {
                    i2c_slave.rec_addr = i2c_slave.rx_tmp;

                    /* compare slave address */
                    if ((i2c_slave.rec_addr & 0xFE) == i2c_slave.dev_addr) {
                        SET_SDA_OUT();
                        SET_SDA_LOW();
                        i2c_slave.state = I2C_SEND_ADDR_ACK;
                    } else {
                        i2c_slave.state = I2C_IDLE;
                    }
                    i2c_slave.bitcnt = 0;
                    i2c_slave.rx_tmp = 0;
                }
                break;

            case I2C_RECV_DATA:
                if (i2c_slave.bitcnt >= 8) {
                    /* to do */
                    /* if want stop receive, modify here to send nack & set state to I2C_IDLE */

                    SET_SDA_OUT();
                    SET_SDA_LOW();
                    i2c_slave.rx_buf[i2c_slave.rx_idx++] = i2c_slave.rx_tmp;
                    i2c_slave.bitcnt = 0;
                    i2c_slave.rx_tmp = 0;
                    i2c_slave.state = I2C_SEND_DATA_ACK;
                }
                break;

            case I2C_SEND_ADDR_ACK:
                if (i2c_slave.rec_addr & 0x01) {
                    /* to do */
                    /* add code to select tx buffer */

                    i2c_slave.tx_cur = i2c_slave.tx_buf;
                    if (*i2c_slave.tx_cur & (1 << (7 - i2c_slave.bitcnt))) {
                        SET_SDA_HIGH();
                    } else {
                        SET_SDA_LOW();
                    }
                    i2c_slave.bitcnt++;
                    i2c_slave.state = I2C_SEND_DATA;
                } else {
                    SET_SDA_IN();
                    i2c_slave.state = I2C_RECV_DATA;
                }
                break;

            case I2C_SEND_DATA_ACK:
                SET_SDA_IN();
                i2c_slave.state = I2C_RECV_DATA;
                break;

            default:
                break;
        }
    }
}

void soft_i2c_slave_sda_exti_callback(void) {
    if (READ_SCL() == 1u) {
        /* sda falling */
        if (READ_SDA() == 0u) {
            switch (i2c_slave.state) {
                case I2C_IDLE:
                    i2c_slave.rx_idx = 0;

                case I2C_RECV_DATA:
                    i2c_slave.bitcnt = 0;
                    i2c_slave.rx_tmp = 0;
                    i2c_slave.state = I2C_RECV_ADDR;
                    break;

                default:
                    break;
            }
        }
            /* sda rising */
        else {
            i2c_slave.state = I2C_IDLE;
        }
    }
}