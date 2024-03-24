#include "bsp_i2c_hw.h"

/*********************************************** 宏定义 ***********************************************/

#define I2C_TIMEOUT             20000U    // 超时时间   
#define I2C_DELAY               10U       // 延迟时间

#define i2c_delay_us(us)        dwt_delay_us(us)
#define i2c_delay_ms(ms)        dwt_delay_ms(ms)

static int  i2c_timeout;

/********************************************** 函数声明 **********************************************/

static void i2c1_bus_reset(void);
static int  i2c1_check_bus(void);

/********************************************** 函数定义 **********************************************/

/**
 * @brief 硬件I2C初始化
 * 
 */
void bsp_i2c1_init(void) {
  // 使能时钟
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);          // 使能GPIO时钟
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);           // 使能I2C时钟

  // GPIO初始化
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                // GPIO模式：复用模式
  GPIO_InitStruct.Alternate = I2C1_GPIO_AF;                     // GPIO复用选择：AF4
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;        // GPIO输出模式：开漏输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                       // GPIO上下拉：无
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;         // GPIO速率：VERY HIGH
  GPIO_InitStruct.Pin = I2C1_SCL_PIN;                           // I2C SCL PIN
  LL_GPIO_Init(I2C1_SCL_PORT, &GPIO_InitStruct);                // I2C SCL PIN初始化
  GPIO_InitStruct.Pin = I2C1_SDA_PIN;                           // I2C SDA PIN
  LL_GPIO_Init(I2C1_SDA_PORT, &GPIO_InitStruct);                // I2C SDA PIN初始化

  // NVIC初始化
  NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));
  NVIC_DisableIRQ(I2C1_EV_IRQn);                                // 禁止I2C事件中断

  // I2C初始化
  LL_I2C_InitTypeDef I2C_InitStruct = {0};
  LL_I2C_SetOwnAddress2(I2C1, 0x00);                            // 本机地址2设置  
  LL_I2C_DisableOwnAddress2(I2C1);                              // 关闭双地址
  LL_I2C_DisableGeneralCall(I2C1);                              // 禁止应答广播地址
  LL_I2C_DisableClockStretching(I2C1);                          // 禁止时钟延展：禁止从机将SCL拉低来暂停传输，直至释放SCL
  I2C_InitStruct.OwnAddress1 = I2C1_OWN_ADDR;                   // 本机地址1设置
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;              // I2C外设模式：I2C模式 | SMBUS主从模式
  I2C_InitStruct.ClockSpeed = I2C1_SCL_FEQ;                     // I2C时钟速率：标准模式 ≤100khz | 快速模式 >100khz && <400khz
  I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;                // I2C快速模式的占空比[L:H]: 2:1 | 16:9
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;                  // I2C应答使能
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;         // I2C寻址模式：7位 | 10位
  LL_I2C_Init(I2C1, &I2C_InitStruct);                           // I2C初始化

  LL_I2C_Enable(I2C1);                                          // 使能I2C
}

/**
 * @brief I2C总线复位程序
 * 
 */
static void i2c1_bus_reset(void) {
  LL_I2C_Disable(I2C1);                                         // 关闭I2C
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_GPIO_SetOutputPin(I2C1_SCL_PORT, I2C1_SCL_PIN);            // SCL线拉高
  LL_GPIO_SetOutputPin(I2C1_SDA_PORT, I2C1_SDA_PIN);            // SDA线拉高
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;                   // GPIO模式：输出模式
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;        // GPIO输出模式：开漏输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                       // GPIO上下拉：无
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;         // GPIO速率：VERY HIGH
  GPIO_InitStruct.Pin = I2C1_SCL_PIN;                           // I2C SCL PIN
  LL_GPIO_Init(I2C1_SCL_PORT, &GPIO_InitStruct);                // I2C SCL PIN初始化
  GPIO_InitStruct.Pin = I2C1_SDA_PIN;                           // I2C SDA PIN
  LL_GPIO_Init(I2C1_SDA_PORT, &GPIO_InitStruct);                // I2C SDA PIN初始化
  for (int i = 0; i < 10; i++) {
    LL_GPIO_ResetOutputPin(I2C1_SCL_PORT, I2C1_SCL_PIN);        // 发送9个SCL时钟
    i2c_delay_us(I2C_DELAY);
    LL_GPIO_SetOutputPin(I2C1_SCL_PORT, I2C1_SCL_PIN);
    i2c_delay_us(I2C_DELAY);
  }
  LL_I2C_EnableReset(I2C1);                                     // SWRST置位
  LL_I2C_DisableReset(I2C1);                                    // SWRST复位
  bsp_i2c1_init();                                              // 外设初始化
}

/**
 * @brief 检测总线是否忙碌或者错误，如有异常则复位总线及外设 
 * 
 * @return int 
 */
static int i2c1_check_bus(void) {
  i2c_timeout = I2C_TIMEOUT;
  while ((LL_I2C_IsActiveFlag_ARLO(I2C1) || LL_I2C_IsActiveFlag_BERR(I2C1) || LL_I2C_IsActiveFlag_BUSY(I2C1))) {
    if (i2c_timeout-- == 0) {
      i2c1_bus_reset();  
      return I2C_ERR_BUS;
    }
  }
  return I2C_OK;
}

/**
 * @brief I2C主机发送函数（轮询方式）
 * 
 * @param slave_addr 从机地址
 * @param src 待发送的源数据地址
 * @param length 待发送的数据长度
 * @return int 
 */
int i2c_master_write_poll(uint8_t slave_addr, const uint8_t *src, uint16_t length) {
  
  while (i2c1_check_bus());                                               // 检查总线

  LL_I2C_GenerateStartCondition(I2C1);                                    // 发出起始信号
  i2c_timeout = I2C_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_SB(I2C1)) {                                 // 检查起始信号
    if (i2c_timeout-- == 0) return I2C_ERR_START;
  }

  LL_I2C_TransmitData8(I2C1, slave_addr | I2C_TRANSMIT_MODE);             // 发送从机地址
  i2c_timeout = I2C_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {                               // 检查地址响应
    if (LL_I2C_IsActiveFlag_AF(I2C1) || i2c_timeout-- == 0) {
      LL_I2C_ClearFlag_AF(I2C1);
      LL_I2C_GenerateStopCondition(I2C1);
      return I2C_ERR_ADDR;
    }
  }
  LL_I2C_ClearFlag_ADDR(I2C1);

  for (int i = 0; i < length; i++) {
    i2c_timeout = I2C_TIMEOUT;
    while (!LL_I2C_IsActiveFlag_TXE(I2C1)) {                              // 检查发送空标志
      if (LL_I2C_IsActiveFlag_AF(I2C1) || i2c_timeout-- == 0) {
        LL_I2C_ClearFlag_AF(I2C1);
        LL_I2C_GenerateStopCondition(I2C1);
        return I2C_ERR_NACK;
      }
    }
    LL_I2C_TransmitData8(I2C1, *src++);                                   // 发送数据
  }

  i2c_timeout = I2C_TIMEOUT;                                               
  while (!LL_I2C_IsActiveFlag_TXE(I2C1) || !LL_I2C_IsActiveFlag_BTF(I2C1)) {        
    if (LL_I2C_IsActiveFlag_AF(I2C1) || i2c_timeout-- == 0) {
      LL_I2C_ClearFlag_AF(I2C1);
      break;
    }
  }
  LL_I2C_GenerateStopCondition(I2C1);                                     // 发送停止信号

  return I2C_OK;
}

/**
 * @brief I2C主机读取函数（轮询方式）
 * 
 * @param slave_addr 从机地址
 * @param dst 目标缓冲区地址
 * @param length 待读取的数据长度
 * @return int 
 */
int i2c_master_read_poll(uint8_t slave_addr, uint8_t *dst, uint16_t length) {
  while (i2c1_check_bus());                                               // 检查总线

  LL_I2C_GenerateStartCondition(I2C1);                                    // 发出起始信号
  i2c_timeout = I2C_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_SB(I2C1)) {                                 // 检查起始信号
    if (i2c_timeout-- == 0) return I2C_ERR_START;
  }

  LL_I2C_TransmitData8(I2C1, slave_addr | I2C_RECEIVER_MODE);             // 发送从机地址
  i2c_timeout = I2C_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {                               // 检查地址响应
    if (LL_I2C_IsActiveFlag_AF(I2C1) || i2c_timeout-- == 0) {
      LL_I2C_ClearFlag_AF(I2C1);
      LL_I2C_GenerateStopCondition(I2C1);
      return I2C_ERR_ADDR;
    }
  }
  LL_I2C_ClearFlag_ADDR(I2C1);

  if (length > 1) {
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);                         // 设置ACK应答
    for (int i = 0; i < length - 1; i++) {
      i2c_timeout = I2C_TIMEOUT;
      while (!LL_I2C_IsActiveFlag_RXNE(I2C1)) {
        if (i2c_timeout-- == 0) {
          LL_I2C_GenerateStopCondition(I2C1);
          return I2C_ERR_RX;
        }
      }
      *dst++ = LL_I2C_ReceiveData8(I2C1);                                 // 接收字节数据
    }
  }

  LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);                          // 设置NACK应答
  LL_I2C_GenerateStopCondition(I2C1);                                     // 发送停止信号
  i2c_timeout == I2C_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_RXNE(I2C1)) {
    if (i2c_timeout-- == 0) {
      LL_I2C_GenerateStopCondition(I2C1);
      return I2C_ERR_RX;
    }
  }
  *dst = LL_I2C_ReceiveData8(I2C1);                                       // 接收最后一个字节数据

  return I2C_OK;
}