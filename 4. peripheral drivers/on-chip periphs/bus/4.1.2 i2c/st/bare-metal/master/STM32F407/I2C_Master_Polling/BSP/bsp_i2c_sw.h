#ifndef __BSP_I2C_SW_H__
#define __BSP_I2C_SW_H__


/*********************************************** 头文件 **********************************************/

#include "stm32f407xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "dwt_delay.h"

/*********************************************** 宏定义 **********************************************/


#define SOFT_I2C_SCL_PORT     GPIOB                 // 模拟I2C 时钟线GPIO端口 
#define SOFT_I2C_SCL_PIN      LL_GPIO_PIN_8         // 模拟I2C 时钟线GPIO引脚
#define SOFT_I2C_SDA_PORT     GPIOB                 // 模拟I2C 数据线GPIO端口
#define SOFT_I2C_SDA_PIN      LL_GPIO_PIN_9         // 模拟I2C 数据线GPIO引脚
#define SOFT_I2C_SCL_FEQ      400000U               // 模拟I2C 时钟频率

#define I2C_SCL_HIGH()        LL_GPIO_SetOutputPin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN)       // SCL拉高
#define I2C_SCL_LOW()         LL_GPIO_ResetOutputPin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN)     // SCL拉低
#define I2C_SDA_HIGH()        LL_GPIO_SetOutputPin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN)       // SDA拉高
#define I2C_SDA_LOW()         LL_GPIO_ResetOutputPin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN)     // SDA拉低
#define I2C_SDA_READ()        LL_GPIO_IsInputPinSet(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN)      // 读取SDA输入数据

#define I2C_DELAY()           dwt_delay_us(i2c_delay_time)  // I2C延迟函数

#define SOFT_I2C_OK           0                     // I2C 返回正确
#define SOFT_I2C_ERR_NACK     1                     // I2C NACK响应
#define SOFT_I2C_ERR_ADDR     2                     // I2C 从机地址无响应

/********************************************** 函数声明 **********************************************/

// 模拟I2C外部接口

void    soft_i2c_init(void);
int     soft_i2c_master_write(uint8_t slave_addr, const uint8_t *src, uint16_t length);
int     soft_i2c_master_read(uint8_t slave_addr, uint8_t *dst, uint16_t length);

#endif