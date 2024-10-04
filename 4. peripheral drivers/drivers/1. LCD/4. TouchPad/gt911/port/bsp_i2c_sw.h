#ifndef __BSP_I2C_SW_H__
#define __BSP_I2C_SW_H__


/* Includes ------------------------------------------------------------------*/

#include "stm32f407xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "dwt_delay.h"

/* Exported macro ------------------------------------------------------------*/

// Use i2c assert
#define SOFT_I2C_USE_ASSERT       1        

// Use i2c log
#define SOFT_I2C_USE_LOG          1 

// I2C us delay function
#define SOFT_I2C_DELAY(us)    dwt_delay_us(us)


/* Exported types ------------------------------------------------------------*/

typedef enum {
  SOFT_I2C_OK,
  SOFT_I2C_ERR_ADDR,
  SOFT_I2C_ERR_NACK,
} soft_i2c_err_t;


typedef struct {
  GPIO_TypeDef *port;
  uint32_t      pin;
} soft_i2c_pin_t;


typedef struct {
  uint16_t  dev_addr;
  uint16_t  ck_delay;
  soft_i2c_pin_t scl;
  soft_i2c_pin_t sda;
  uint8_t   mem_addr[4];
}soft_i2c_dev_t;

/* Exported function declaration ---------------------------------------------*/



void soft_i2c_init(soft_i2c_dev_t *i2c_dev, uint16_t i2c_freq);
int  soft_i2c_master_write(soft_i2c_dev_t *i2c_dev, const uint8_t *wr_data, uint16_t data_len);
int  soft_i2c_master_mem_write(soft_i2c_dev_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len, const uint8_t *wr_data, uint16_t data_len);
int  soft_i2c_master_read(soft_i2c_dev_t *i2c_dev, uint8_t *rd_data, uint16_t data_len);
int  soft_i2c_master_mem_read(soft_i2c_dev_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len, uint8_t *rd_data, uint16_t data_len);



#endif