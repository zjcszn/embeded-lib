//
// Created by zjcszn on 2024/3/11.
//

#ifndef _DEV_I2C_HW_H_
#define _DEV_I2C_HW_H_

#include <stdint.h>
#include "bsp_i2c_hw.h"

/********************************************** 类型声明 **********************************************/

#ifndef __IO
#define __IO  volatile
#endif

#define I2C_OK                  0
#define I2C_ERR_NOMEM           (1UL << 8)
#define I2C_ERR_PARAM           (1UL << 7)
#define I2C_ERR_TIMEOUT         (1UL << 1)
#define I2C_ERR_NACK            (1UL << 2)
#define I2C_ERR_ARLO            (1UL << 3)
#define I2C_ERR_BERR            (1UL << 4)
#define I2C_ERR_OVR             (1UL << 5)
#define I2C_ERR_DMA             (1UL << 6)

typedef enum {
    I2C_STATE_SEND_REG_ADDR,
    I2C_STATE_TRANSFER_START,
    I2C_STATE_TRANSFER_ERROR,
    I2C_STATE_TRANSFER_COMPLETE,
} i2c_state_t;

typedef enum {
    I2C_MODE_READ,
    I2C_MODE_WRITE,
} i2c_mode_t;

/********************************************** 函数声明 **********************************************/



uint32_t dev_i2c_init(void);
uint32_t i2c_read_dma(uint8_t saddr, uint8_t *pbuf, uint16_t nbytes);
uint32_t i2c_write_dma(uint8_t saddr, uint8_t *pbuf, uint16_t nbytes);
uint32_t i2c_mem_read_dma(uint8_t saddr, uint32_t raddr, uint16_t raddr_len, uint8_t *pbuf, uint16_t nbytes);
uint32_t i2c_mem_write_dma(uint8_t saddr, uint32_t raddr, uint16_t raddr_len, uint8_t *pbuf, uint16_t nbytes);

#endif //_DEV_I2C_HW_H_
