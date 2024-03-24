#include "bsp_i2c_sw.h"

/******************************************** 私有全局变量 *********************************************/

static uint8_t i2c_delay_time = 1;

/********************************************** 函数声明 **********************************************/

void    soft_i2c_master_start(void);
void    soft_i2c_master_stop(void);
int     soft_i2c_master_send_byte(uint8_t byte);
int     soft_i2c_master_wait_ack(void);
void    soft_i2c_master_send_ack(void);
void    soft_i2c_master_send_nack(void);
uint8_t soft_i2c_master_recv_byte(void);

/********************************************** 函数定义 **********************************************/

/**
 * @brief 模拟I2C初始化程序 
 * 
 */
void soft_i2c_init(void) {

  i2c_delay_time = 500000U / SOFT_I2C_SCL_FEQ;                  // 计算I2C时钟延迟周期
  if (!i2c_delay_time) {
    i2c_delay_time = 1;
  }

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);          // 使能GPIO时钟

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;                   // GPIO模式：输出模式
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;        // GPIO输出模式：开漏输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                       // GPIO上下拉：无
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;         // GPIO速率：VERY HIGH
  GPIO_InitStruct.Pin = SOFT_I2C_SCL_PIN;                       // I2C SCL PIN
  LL_GPIO_Init(SOFT_I2C_SCL_PORT, &GPIO_InitStruct);            // I2C SCL PIN初始化
  GPIO_InitStruct.Pin = SOFT_I2C_SDA_PIN;                       // I2C SDA PIN
  LL_GPIO_Init(SOFT_I2C_SDA_PORT, &GPIO_InitStruct);            // I2C SDA PIN初始化

  I2C_SDA_H();
  I2C_SCL_H();
}

/**
 * @brief 模拟I2C主机发送函数（轮询方式）
 * 
 * @param slave_addr 从机地址
 * @param src 待发送的源数据地址
 * @param length 待发送的数据长度
 * @return int 
 */
int soft_i2c_master_write(uint8_t slave_addr, const uint8_t *src, uint16_t length) {
  soft_i2c_master_start();
  if (soft_i2c_master_send_byte(slave_addr)) return SOFT_I2C_ERR_ADDR;
  for (int i = 0; i < length; i++) {
    if (soft_i2c_master_send_byte(*src++)) return SOFT_I2C_ERR_NACK;
  }
  soft_i2c_master_stop();
  return SOFT_I2C_OK;
}

/**
 * @brief 模拟I2C主机读取函数（轮询方式）
 * 
 * @param slave_addr 从机地址
 * @param dst 目标缓冲区地址
 * @param length 待读取的数据长度
 * @return int 
 */
int soft_i2c_master_read(uint8_t slave_addr, uint8_t *dst, uint16_t length) {
  soft_i2c_master_start();
  if (soft_i2c_master_send_byte(slave_addr | 1)) return SOFT_I2C_ERR_ADDR;
  for (int i = 0; i < length - 1; i++) {
    *dst++ = soft_i2c_master_recv_byte();
    soft_i2c_master_send_ack();
  }
  *dst = soft_i2c_master_recv_byte();
  soft_i2c_master_send_nack();
  soft_i2c_master_stop();
  return SOFT_I2C_OK;
}

/**
 * @brief 发送起始信号
 * 
 */
void soft_i2c_master_start(void) {
  I2C_SCL_H();
  I2C_SDA_H();
  I2C_DELAY();
  I2C_SDA_L();
}

/**
 * @brief 发送停止信号
 * 
 */
void soft_i2c_master_stop(void) {
  I2C_SCL_L();
  I2C_SDA_L();
  I2C_DELAY();
  I2C_SCL_H();
  I2C_DELAY();
  I2C_SDA_H();
}

/**
 * @brief 主机发送字节数据
 * 
 * @param byte 
 * @return int 
 */
int soft_i2c_master_send_byte(uint8_t byte) {
  for (int i = 0; i < 8; i++) {
    I2C_SCL_L();
    byte & 0x80 ? I2C_SDA_H() : I2C_SDA_L();
    I2C_DELAY();
    I2C_SCL_H();
    I2C_DELAY();
    byte <<= 1U;
  }
  if (soft_i2c_master_wait_ack()) {
    soft_i2c_master_stop();
    return SOFT_I2C_ERR_NACK;
  }
  return SOFT_I2C_OK;
}

/**
 * @brief 主机等待ACK信号
 * 
 * @return int 
 */
int soft_i2c_master_wait_ack(void) {
  I2C_SCL_L();
  I2C_SDA_H();
  I2C_DELAY();
  I2C_SCL_H();
  I2C_DELAY();
  return I2C_SDA_READ();
}

/**
 * @brief 主机发送ACK信号
 * 
 */
void soft_i2c_master_send_ack(void) {
  I2C_SCL_L();
  I2C_SDA_L();
  I2C_DELAY();
  I2C_SCL_H();
  I2C_DELAY();
  I2C_SCL_L();
  I2C_SDA_H();
}

/**
 * @brief 主机发送NACK信号
 * 
 */
void soft_i2c_master_send_nack(void) {
  I2C_SCL_L();
  I2C_SDA_H();
  I2C_DELAY();
  I2C_SCL_H();
  I2C_DELAY();
  I2C_SCL_L();
  I2C_SDA_H();
}

/**
 * @brief 主机接收字节数据
 * 
 * @return uint8_t 
 */
uint8_t soft_i2c_master_recv_byte(void) {
  uint8_t recv_tmp;
  for (int i = 0; i < 8; i++) {
    I2C_SCL_L();
    I2C_DELAY();
    I2C_SCL_H();
    I2C_DELAY();
    recv_tmp = (recv_tmp << 1) | I2C_SDA_READ();
  }
  return recv_tmp;
}

