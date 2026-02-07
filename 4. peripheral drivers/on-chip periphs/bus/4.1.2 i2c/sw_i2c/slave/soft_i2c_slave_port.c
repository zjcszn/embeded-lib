/**
 * @file soft_i2c_slave_port.c
 * @brief Reference implementation of Soft I2C Slave port for STM32 LL
 */

#include "main.h"
#include "soft_i2c_slave.h"

/* Define your GPIO pins here */
#define I2C_PORT GPIOD
#define SCL_PIN  LL_GPIO_PIN_2
#define SDA_PIN  LL_GPIO_PIN_3

static uint8_t port_read_scl(void) {
    return LL_GPIO_IsInputPinSet(I2C_PORT, SCL_PIN);
}

static uint8_t port_read_sda(void) {
    return LL_GPIO_IsInputPinSet(I2C_PORT, SDA_PIN);
}

static void port_set_sda_out(void) {
    LL_GPIO_SetPinMode(I2C_PORT, SDA_PIN, LL_GPIO_MODE_OUTPUT);
}

static void port_set_sda_in(void) {
    LL_GPIO_SetPinMode(I2C_PORT, SDA_PIN, LL_GPIO_MODE_INPUT);
}

static void port_write_sda(uint8_t level) {
    if (level != 0u) {
        LL_GPIO_SetOutputPin(I2C_PORT, SDA_PIN);
    } else {
        LL_GPIO_ResetOutputPin(I2C_PORT, SDA_PIN);
    }
}

const i2c_slave_ops_t stm32_i2c_ops = {.read_scl = port_read_scl,
                                       .read_sda = port_read_sda,
                                       .set_sda_out = port_set_sda_out,
                                       .set_sda_in = port_set_sda_in,
                                       .write_sda = port_write_sda,
                                       .delay = NULL};

/* Example usage and event handler */
void i2c_slave_callback(i2c_slave_ev_t ev, uint8_t *data) {
    static uint8_t reg_addr = 0;
    static uint8_t ram[256];

    switch (ev) {
        case I2C_SLAVE_EV_START:
            /* data contains the address byte (with R/W bit) */
            break;

        case I2C_SLAVE_EV_RECV:
            /* Simple demo: first byte is address, subsequent are data */
            ram[reg_addr++] = *data;
            break;

        case I2C_SLAVE_EV_TRNS:
            /* Provide data to core shift register */
            *data = ram[reg_addr++];
            break;

        case I2C_SLAVE_EV_STOP:
            reg_addr = 0;
            break;

        default:
            break;
    }
}

/*
 * Usage:
 * i2c_slave_t my_i2c;
 * i2c_slave_cfg_t cfg = {
 *     .addr = 0xA0,
 *     .ops = &stm32_i2c_ops,
 *     .callback = i2c_slave_callback
 * };
 * i2c_slave_init(&my_i2c, &cfg);
 *
 * // In EXTI Handlers:
 * void EXTI2_IRQHandler(void) {
 *     if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2)) {
 *         LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
 *         i2c_slave_scl_handler(&my_i2c);
 *     }
 * }
 */
