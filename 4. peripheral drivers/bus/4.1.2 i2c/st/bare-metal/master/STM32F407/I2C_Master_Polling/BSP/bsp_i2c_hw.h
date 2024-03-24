#ifndef __BSP_I2C_HW_H__
#define __BSP_I2C_HW_H__


/*********************************************** 头文件 **********************************************/

#include "stm32f407xx.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "dwt_delay.h"

/*********************************************** 宏定义 **********************************************/

#define I2C1_GPIO_AF     LL_GPIO_AF_4          // I2C GPIO复用功能选择
#define I2C1_SCL_PORT    GPIOB                 // I2C 时钟线GPIO端口 
#define I2C1_SCL_PIN     LL_GPIO_PIN_8         // I2C 时钟线GPIO引脚
#define I2C1_SDA_PORT    GPIOB                 // I2C 数据线GPIO端口
#define I2C1_SDA_PIN     LL_GPIO_PIN_9         // I2C 数据线GPIO引脚
#define I2C1_SCL_FEQ     400000U               // I2C 时钟频率
#define I2C1_OWN_ADDR    0U                    // I2C 本机地址（从设备模式）

/********************************************** 类型声明 **********************************************/

enum ENUM_I2C_ErrCode {
  I2C_OK,
  I2C_ERR_BUS,     // 总线错误
  I2C_ERR_START,   // 起始信号错误
  I2C_ERR_ADDR,    // 从机地址无响应
  I2C_ERR_NACK,    // 数据NACK响应
  I2C_ERR_RX, 
  I2C_ERR_STOP,
};

enum ENUM_I2C_Direction {
  I2C_TRANSMIT_MODE,    // 发送器模式
  I2C_RECEIVER_MODE,
};


/********************************************** 函数声明 **********************************************/

void bsp_i2c1_init(void);
int  i2c_master_write_poll(uint8_t slave_addr, const uint8_t *src, uint16_t length);
int  i2c_master_read_poll(uint8_t slave_addr, uint8_t *dst, uint16_t length);

#endif