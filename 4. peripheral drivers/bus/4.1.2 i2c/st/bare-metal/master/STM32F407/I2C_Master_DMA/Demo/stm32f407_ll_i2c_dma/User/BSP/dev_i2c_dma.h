#ifndef __DEV_I2C_DMA_H__
#define __DEV_I2C_DMA_H__

#include "bsp_i2c_dma.h"

/********************************************** 类型声明 **********************************************/

#ifndef __IO
#define __IO  volatile
#endif

// I2C状态
enum ENUM_I2C_State {
  I2C_START,            // 起始阶段
  I2C_RESTART,          // ReStart阶段
  I2C_SEND_DEV_ADDR,    // 发送从机地址
  I2C_SEND_MEM_ADDR,    // 发送内存地址
  I2C_SEND_DATA,        // 发送数据
  I2C_RECV_DATA,        // 接收数据
  I2C_STOP,             // 停止阶段
};


// I2C错误代码
enum ENUM_I2C_ErrCode {
  I2C_OK,
  I2C_ERR_BUSY,         // 总线忙碌
  I2C_ERR_BUS,          // 总线错误
  I2C_ERR_START,        // 起始信号错误
  I2C_ERR_ADDR,         // 发送从机地址无响应
  I2C_ERR_NACK,         // 发送数据 NACK响应
  I2C_ERR_RX,           // 接收数据错误
  I2C_ERR_STOP,         // 停止信号错误
};

// I2C锁状态
enum ENUM_I2C_LockStatus {
  I2C_UNLOCKED,         // 解锁状态
  I2C_LOCKED,           // 锁定状态
};

// I2C传输方向
enum ENUM_I2C_Direction {
  I2C_TX,               // 发送 
  I2C_RX,               // 接收
};

// I2C_Device类型
typedef struct {
  __IO  uint8_t   state;              // I2C通讯状态
  __IO  uint8_t   lock;               // I2C锁定标志
  __IO  uint8_t   error;              // I2C错误标志 
  __IO  uint8_t   direction;          // 传输方向
  __IO  uint8_t   slave_addr;         // 从机地址
  __IO  uint8_t   mem_addr_en;        // 目标内存地址使能
  __IO  uint8_t   mem_addr[4];        // 目标内存地址
  __IO  uint8_t   mem_addr_bytes;     // 目标内存地址长度
        uint8_t  *data_buf_addr;      // 数据缓冲区地址
  __IO  uint16_t  data_buf_size;      // 数据缓冲区大小
  void  (*tc_callback) (void);        // 传输完成回调函数（备用）
}I2C_Device_T;


/********************************************** 函数声明 **********************************************/

int  i2c_write_dma(uint8_t slave_addr, uint8_t *buf_addr, uint16_t buf_size);
int  i2c_mem_write_dma(uint8_t slave_addr, uint32_t mem_addr, uint8_t mem_addr_bytes, uint8_t *buf_addr, uint16_t buf_size);
int  i2c_read_dma(uint8_t slave_addr, uint8_t *buf_addr, uint16_t buf_size);
int  i2c_mem_read_dma(uint8_t slave_addr, uint32_t mem_addr, uint8_t mem_addr_bytes, uint8_t *buf_addr, uint16_t buf_size);

void i2c1_err_isr(void);
void i2c1_evt_isr(void);
void i2c1_dmatx_tc_isr(void);
void i2c1_dmarx_tc_isr(void);

#endif