#ifndef __SOFT_I2C_SLAVE_H__
#define __SOFT_I2C_SLAVE_H__

#include "main.h"
#include <stdint.h>


/* macro define */

#define I2C_RX_BUFSZ          32
#define I2C_SLAVE_SCL         D, 2
#define I2C_SLAVE_SDA         D, 3


/* macro utils */

#define _GPIO_CLOCK(a,b)      LL_AHB1_GRP1_PERIPH_GPIO##a
#define I2C_GPIO_CLOCK(a)     LL_AHB1_GRP1_EnableClock(_GPIO_CLOCK(a))

#define _PORT(a,b)            GPIO##a
#define I2C_GPIO_PORT(a)      _PORT(a)

#define _PIN(a,b)             LL_GPIO_PIN_##b  
#define I2C_GPIO_PIN(a)       _PIN(a)

#define _SYS_EPORT(a,b)       LL_SYSCFG_EXTI_PORT##a  
#define I2C_SYS_EPORT(a)      _SYS_EPORT(a)

#define _SYS_ELINE(a,b)       LL_SYSCFG_EXTI_LINE##b
#define I2C_SYS_ELINE(a)      _SYS_ELINE(a)

#define _EXTI_LINE(a,b)       LL_EXTI_LINE_##b
#define I2C_EXTI_LINE(a)      _EXTI_LINE(a)

#define _EXTI_IRQ(a,b)        EXTI##b##_IRQn             
#define I2C_EXTI_IRQ(a)       _EXTI_IRQ(a)

/* IO operation */
#define READ_SCL()            LL_GPIO_IsInputPinSet(I2C_GPIO_PORT(I2C_SLAVE_SCL), I2C_GPIO_PIN(I2C_SLAVE_SCL))
#define READ_SDA()            LL_GPIO_IsInputPinSet(I2C_GPIO_PORT(I2C_SLAVE_SDA), I2C_GPIO_PIN(I2C_SLAVE_SDA))

#define SET_SDA_OUT()         LL_GPIO_SetPinMode(I2C_GPIO_PORT(I2C_SLAVE_SDA), I2C_GPIO_PIN(I2C_SLAVE_SDA), LL_GPIO_MODE_OUTPUT)
#define SET_SDA_IN()          LL_GPIO_SetPinMode(I2C_GPIO_PORT(I2C_SLAVE_SDA), I2C_GPIO_PIN(I2C_SLAVE_SDA), LL_GPIO_MODE_INPUT)
#define SET_SDA_HIGH()        LL_GPIO_SetOutputPin(I2C_GPIO_PORT(I2C_SLAVE_SDA), I2C_GPIO_PIN(I2C_SLAVE_SDA))
#define SET_SDA_LOW()         LL_GPIO_ResetOutputPin(I2C_GPIO_PORT(I2C_SLAVE_SDA), I2C_GPIO_PIN(I2C_SLAVE_SDA))


/* Slave fsm state */
typedef enum i2c_state
{
  I2C_IDLE,
  I2C_RECV_ADDR,
  I2C_SEND_ADDR_ACK,
  I2C_SEND_DATA,
  I2C_WAIT_DATA_ACK,
  I2C_RECV_DATA,
  I2C_SEND_DATA_ACK,
}i2c_state_t;


/* Slave device handle */
typedef struct i2c_slave_dev
{
  i2c_state_t state;
  uint8_t  dev_addr;
  uint8_t  rec_addr;
  uint8_t  rx_tmp;
  uint8_t *tx_buf;
  uint8_t *tx_cur;
  uint8_t  rx_buf[I2C_RX_BUFSZ];
  uint32_t rx_idx;
  uint8_t  bitcnt;
} i2c_slave_dev_t;

extern i2c_slave_dev_t i2c_slave;


void soft_i2c_slave_init(uint8_t addr, uint8_t *tx_buf);
void soft_i2c_slave_scl_exti_callback(void);
void soft_i2c_slave_sda_exti_callback(void);


#endif