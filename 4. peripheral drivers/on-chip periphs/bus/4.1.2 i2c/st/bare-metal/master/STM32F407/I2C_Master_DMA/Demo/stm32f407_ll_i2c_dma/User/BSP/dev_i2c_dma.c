#include "dev_i2c_dma.h"

/*********************************************** 宏定义 ***********************************************/

// I2C总线锁定
#define __I2C_LOCK()                        \
  do {                                      \
    if (i2c_dev.lock == I2C_LOCKED) {       \
      return I2C_ERR_BUSY;                  \
    }                                       \
    else {                                  \
      i2c_dev.lock = I2C_LOCKED;            \
    }                                       \
  } while (0U)

// I2C总线解锁
#define __I2C_UNLOCK()                      \
  do {                                      \
      i2c_dev.lock = I2C_UNLOCKED;          \
  } while (0U)

/******************************************** 静态全局变量 *********************************************/

I2C_Device_T i2c_dev;

/********************************************** 函数声明 **********************************************/

void i2c_dmatx_enable(uint8_t *dmatx_buf_addr, uint16_t dmatx_buf_size);
void i2c_dmarx_enable(uint8_t *dmarx_buf_addr, uint16_t dmarx_buf_size);

/********************************************** 函数定义 **********************************************/

/**
 * @brief I2C主机发送函数（DMA模式）
 * 
 * @param slave_addr 从机地址
 * @param buf_addr 待发送的数据缓冲区地址
 * @param buf_size 待发送的数据缓冲区大小
 * @return int I2C_ERR_BUSY 总线忙碌 | I2C_OK 开始发送
 */
int i2c_write_dma(uint8_t slave_addr, uint8_t *buf_addr, uint16_t buf_size) {
  __I2C_LOCK();
  i2c_dev.direction       = I2C_TX;
  i2c_dev.slave_addr      = slave_addr;
  i2c_dev.error           = I2C_OK;
  i2c_dev.mem_addr_en     = 0;
  i2c_dev.data_buf_addr   = buf_addr;
  i2c_dev.data_buf_size   = buf_size;
  i2c_dev.state           = I2C_START; 

  while (i2c_bus_check());
  LL_I2C_EnableIT_EVT(I2C1);                // 使能事件中断
  LL_I2C_EnableIT_ERR(I2C1);                // 使能错误中断
  LL_I2C_GenerateStartCondition(I2C1);      // 发出起始信号
  return I2C_OK;
}

/**
 * @brief I2C主机发送函数：内存写入版（DMA模式）
 * 
 * @param slave_addr 从机地址
 * @param mem_addr 目标内存地址
 * @param mem_addr_bytes 目标内存地址字节数
 * @param buf_addr 待发送的数据缓冲区地址
 * @param buf_size 待发送的数据缓冲区大小
 * @return int I2C_ERR_BUSY 总线忙碌 | I2C_OK 开始发送
 */
int i2c_mem_write_dma(uint8_t slave_addr, uint32_t mem_addr, uint8_t mem_addr_bytes, uint8_t *buf_addr, uint16_t buf_size) {
  __I2C_LOCK();
  i2c_dev.direction       = I2C_TX;
  i2c_dev.slave_addr      = slave_addr;
  i2c_dev.error           = I2C_OK;
  i2c_dev.mem_addr_en     = 1;
  i2c_dev.mem_addr_bytes  = mem_addr_bytes;
  i2c_dev.data_buf_addr   = buf_addr;
  i2c_dev.data_buf_size   = buf_size;
  i2c_dev.state           = I2C_START; 
  for (int i = 0; i < mem_addr_bytes; i++) {
    i2c_dev.mem_addr[i] = (uint8_t)((mem_addr >> ((mem_addr_bytes - i - 1) * 8U)) & 0xFF);
  }

  while (i2c_bus_check());
  LL_I2C_EnableIT_EVT(I2C1);                // 使能事件中断
  LL_I2C_EnableIT_ERR(I2C1);                // 使能错误中断
  LL_I2C_GenerateStartCondition(I2C1);      // 发出起始信号
  return I2C_OK;
}

/**
 * @brief I2C主机读取函数（DMA模式）
 * 
 * @param slave_addr 从机地址
 * @param buf_addr 待接收的数据缓冲区地址
 * @param buf_size 待接收的数据缓冲区大小
 * @return int I2C_ERR_BUSY 总线忙碌 | I2C_OK 开始发送
 */
int i2c_read_dma(uint8_t slave_addr, uint8_t *buf_addr, uint16_t buf_size) {
  __I2C_LOCK();
  i2c_dev.direction       = I2C_RX;
  i2c_dev.slave_addr      = slave_addr;
  i2c_dev.error           = I2C_OK;
  i2c_dev.mem_addr_bytes  = 0;
  i2c_dev.data_buf_addr   = buf_addr;
  i2c_dev.data_buf_size   = buf_size;
  i2c_dev.state           = I2C_START; 
 
  while (i2c_bus_check());
  LL_I2C_EnableIT_EVT(I2C1);                // 使能事件中断
  LL_I2C_EnableIT_ERR(I2C1);                // 使能错误中断
  LL_I2C_GenerateStartCondition(I2C1);      // 发出起始信号
  return I2C_OK;
}

/**
 * @brief I2C主机读取函数：内存读取版（DMA模式）
 * 
 * @param slave_addr 从机地址
 * @param mem_addr 目标内存地址
 * @param mem_addr_bytes 目标内存地址字节数
 * @param buf_addr 待接收的数据缓冲区地址
 * @param buf_size 待接收的数据缓冲区大小
 * @return int I2C_ERR_BUSY 总线忙碌 | I2C_OK 开始发送
 */
int i2c_mem_read_dma(uint8_t slave_addr, uint32_t mem_addr, uint8_t mem_addr_bytes, uint8_t *buf_addr, uint16_t buf_size) {
  __I2C_LOCK();
  i2c_dev.direction       = I2C_RX;
  i2c_dev.slave_addr      = slave_addr;
  i2c_dev.error           = I2C_OK;
  i2c_dev.mem_addr_en     = 1;
  i2c_dev.mem_addr_bytes  = mem_addr_bytes;
  i2c_dev.data_buf_addr   = buf_addr;
  i2c_dev.data_buf_size   = buf_size;
  i2c_dev.state           = I2C_START; 
  for (int i = 0; i < mem_addr_bytes; i++) {
    i2c_dev.mem_addr[i] = (uint8_t)((mem_addr >> ((mem_addr_bytes - i - 1) * 8U)) & 0xFF);
  }
  
  while (i2c_bus_check());
  LL_I2C_EnableIT_EVT(I2C1);                // 使能事件中断
  LL_I2C_EnableIT_ERR(I2C1);                // 使能错误中断
  LL_I2C_GenerateStartCondition(I2C1);      // 发出起始信号
  return I2C_OK;
}

/**
 * @brief I2C事件中断回调函数
 * 
 */
void i2c1_evt_isr(void) {
  // 起始信号事件中断
  if (LL_I2C_IsActiveFlag_SB(I2C1)) {
    if (i2c_dev.direction == I2C_TX || i2c_dev.mem_addr_en) {         // 发送写地址
      LL_I2C_TransmitData8(I2C1, i2c_dev.slave_addr & 0xFE);      
    }
    else {
      LL_I2C_TransmitData8(I2C1, i2c_dev.slave_addr | 0x01);          // 发送读地址
    }
    i2c_dev.state = I2C_SEND_DEV_ADDR;
  }

  // 从机地址响应事件中断
  if (LL_I2C_IsActiveFlag_ADDR(I2C1)) {
    LL_I2C_DisableIT_EVT(I2C1);                                       // 关闭I2C事件中断
    if (i2c_dev.mem_addr_en) {                                        // 如果使能了目标内存地址，则开启DMA发送目标内存地址
      i2c_dev.state = I2C_SEND_MEM_ADDR;
      i2c_dmatx_enable((uint8_t *)&i2c_dev.mem_addr, i2c_dev.mem_addr_bytes);
    }
    else if (i2c_dev.direction == I2C_TX) {                           // 如果是发送函数，则开启DMA发送数据
      i2c_dev.state = I2C_SEND_DATA;
      i2c_dmatx_enable(i2c_dev.data_buf_addr, i2c_dev.data_buf_size);
    }
    else if (i2c_dev.direction == I2C_RX) {                           // 如果是接收函数，则开启DMA接收数据
      i2c_dev.state = I2C_RECV_DATA;
      if (i2c_dev.data_buf_size >= 2) {
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);                 // 如果传输字节数大于1，则使能ACK响应和LAST位
        LL_I2C_EnableLastDMA(I2C1);
      }
      else {                                                          // 如果传输字节数等于1，则使能NACK响应
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
      }
      i2c_dmarx_enable(i2c_dev.data_buf_addr, i2c_dev.data_buf_size);
    }
    LL_I2C_ClearFlag_ADDR(I2C1);                                      // 清除ADDR标志
  }
}

/**
 * @brief I2C错误中断回调函数
 * 
 */
void i2c1_err_isr(void) {

  // 仲裁丢失错误 及 总线错误
  if (LL_I2C_IsActiveFlag_ARLO(I2C1) || LL_I2C_IsActiveFlag_BERR(I2C1)) {
    i2c_dev.error = I2C_ERR_BUS;
    i2c_bus_reset();                                                  // 如果发生仲裁丢失或者总线错误，则立即复位外设并解锁I2C总线
    __I2C_UNLOCK();
  }

  // NACK响应处理
  if (LL_I2C_IsActiveFlag_AF(I2C1)) {
    i2c_dev.error = I2C_ERR_NACK;
    if (i2c_dev.state == I2C_SEND_DEV_ADDR) {                         // 如果发送从机地址后无从机应答，则发出停止信号并解锁I2C总线
      LL_I2C_GenerateStopCondition(I2C1);
      LL_I2C_DisableIT_EVT(I2C1);
      LL_I2C_DisableIT_ERR(I2C1);
      __I2C_UNLOCK();
    }
    else {
      LL_DMA_DisableStream(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);          // 如果发送数据后收到NACK响应，则关闭DMA数据流，停止发送
    }
    LL_I2C_ClearFlag_AF(I2C1);                                        // 清除AF标志
  }
}

/**
 * @brief DMA发送完成中断 回调函数
 * 
 */
void i2c1_dmatx_tc_isr(void) {
  LL_DMA_DisableIT_TC(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);               // 关闭DMA发送完成中断
  if (i2c_dev.error == I2C_OK) {
    int timeout = 20000;
    while (!LL_I2C_IsActiveFlag_BTF(I2C1) && !LL_I2C_IsActiveFlag_TXE(I2C1)) {
      if(timeout-- == 0) break;
    }
    if (i2c_dev.state == I2C_SEND_MEM_ADDR) {                         // 如果I2C状态是I2C_SEND_MEM_ADDR，则继续完成后续通讯 
      i2c_dev.mem_addr_en = 0;
      if (i2c_dev.direction == I2C_RX) {                              // 如果传输方向是RX，则生成ReStart信号，在EVT中断中开启DMA接收数据
        i2c_dev.state = I2C_RESTART;
        LL_I2C_EnableIT_EVT(I2C1);                                    // 使能EVT中断
        LL_I2C_GenerateStartCondition(I2C1);                          // 发出ReStart信号
      }
      else {
        i2c_dev.state = I2C_SEND_DATA;                                // 开启DMA发送数据
        i2c_dmatx_enable(i2c_dev.data_buf_addr, i2c_dev.data_buf_size);
      }
      return;
    }    
  }
  i2c_dev.state = I2C_STOP;
  LL_I2C_GenerateStopCondition(I2C1);                                 // 发出停止信号
  LL_I2C_DisableDMAReq_TX(I2C1);                                      // 关闭I2C发送DMA请求
  LL_I2C_DisableIT_ERR(I2C1);                                         // 关闭I2C错误中断
  __I2C_UNLOCK();
}

/**
 * @brief DMA接收完成中断 回调函数
 * 
 */
void i2c1_dmarx_tc_isr(void) {
  LL_DMA_DisableIT_TC(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);               // 关闭DMA发送完成中断
	i2c_dev.state = I2C_STOP;
  LL_I2C_GenerateStopCondition(I2C1);                                 // 发出停止信号
	LL_I2C_DisableDMAReq_RX(I2C1);                                      // 关闭I2C接收DMA请求
  LL_I2C_DisableIT_ERR(I2C1);                                         // 关闭I2C错误中断
  __I2C_UNLOCK();
}

/**
 * @brief 使能发送DMA
 * 
 * @param dmatx_buf_addr 发送缓冲区地址 
 * @param dmatx_buf_size 发送缓冲区大小
 */
void i2c_dmatx_enable(uint8_t *dmatx_buf_addr, uint16_t dmatx_buf_size) {
  LL_DMA_DisableStream(I2C1_TX_DMA, I2C1_TX_DMA_STREAM); 
  while (LL_DMA_IsEnabledStream(I2C1_TX_DMA, I2C1_TX_DMA_STREAM));                          // 检查DMA数据流是否关闭
  LL_I2C_DisableDMAReq_TX(I2C1);                                                            // 禁止I2C TX DMA请求
  LL_DMA_SetMemoryAddress(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, (uint32_t)dmatx_buf_addr);       // 设置内存地址
  LL_DMA_SetDataLength(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, dmatx_buf_size);                    // 设置DMA传输数据长度

  DMA_CLEAR_IT_FLAG(I2C1_TX_DMA_STREAM_ID, TC,  I2C1_TX_DMA);                               // 清除发送完成标志
  DMA_CLEAR_IT_FLAG(I2C1_TX_DMA_STREAM_ID, HT,  I2C1_TX_DMA);                               // 清除半满标志
  DMA_CLEAR_IT_FLAG(I2C1_TX_DMA_STREAM_ID, TE,  I2C1_TX_DMA);                               // 清除发送错误标志
  DMA_CLEAR_IT_FLAG(I2C1_TX_DMA_STREAM_ID, DME, I2C1_TX_DMA);                               // 清除传输方向错误标志
  DMA_CLEAR_IT_FLAG(I2C1_TX_DMA_STREAM_ID, FE,  I2C1_TX_DMA);                               // 清除FIFO错误标志
  
  LL_DMA_EnableIT_TC(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);                                      // 使能DMA发送完成中断
  LL_DMA_EnableStream(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);                                     // 使能DMA数据流
  LL_I2C_EnableDMAReq_TX(I2C1);                                                             // 使能I2C TX DMA请求
}
/**
 * @brief 使能接收DMA
 * 
 * @param dmarx_buf_addr 接收缓冲区地址
 * @param dmarx_buf_size 接收缓冲区大小
 */
void i2c_dmarx_enable(uint8_t *dmarx_buf_addr, uint16_t dmarx_buf_size) {
  LL_DMA_DisableStream(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);  
  while (LL_DMA_IsEnabledStream(I2C1_RX_DMA, I2C1_RX_DMA_STREAM));                          // 检查DMA数据流是否关闭  
  LL_I2C_DisableDMAReq_RX(I2C1);                                                            // 禁止I2C TX DMA请求
  LL_DMA_SetMemoryAddress(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, (uint32_t)dmarx_buf_addr);       // 设置内存地址
  LL_DMA_SetDataLength(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, dmarx_buf_size);                    // 设置DMA传输数据长度

  DMA_CLEAR_IT_FLAG(I2C1_RX_DMA_STREAM_ID, TC,  I2C1_RX_DMA);                               // 清除发送完成标志
  DMA_CLEAR_IT_FLAG(I2C1_RX_DMA_STREAM_ID, HT,  I2C1_RX_DMA);                               // 清除半满标志
  DMA_CLEAR_IT_FLAG(I2C1_RX_DMA_STREAM_ID, TE,  I2C1_RX_DMA);                               // 清除发送错误标志
  DMA_CLEAR_IT_FLAG(I2C1_RX_DMA_STREAM_ID, DME, I2C1_RX_DMA);                               // 清除传输方向错误标志
  DMA_CLEAR_IT_FLAG(I2C1_RX_DMA_STREAM_ID, FE,  I2C1_RX_DMA);                               // 清除FIFO错误标志
  
  LL_DMA_EnableIT_TC(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);                                      // 使能DMA发送完成中断
  LL_DMA_EnableStream(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);                                     // 使能DMA数据流
  LL_I2C_EnableDMAReq_RX(I2C1);                                                             // 使能I2C RX DMA请求 
}
