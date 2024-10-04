#include "bsp_i2c_dma.h"


/*********************************************** 宏定义 **********************************************/

#define I2C_ENABLE_GPIO_CLOCK()           LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB)   // 使能GPIO时钟
#define I2C_ENABLE_DMA_CLOCK()            LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1)    // 使能DMA时钟
#define I2C_ENABLE_PERIPH_CLOCK()         LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1)    // 使能I2C时钟

#define I2C_ENABLE_CLOCK()        \
  do {                            \
    I2C_ENABLE_GPIO_CLOCK();      \
    I2C_ENABLE_DMA_CLOCK();       \
    I2C_ENABLE_PERIPH_CLOCK();    \
  } while(0)

/********************************************** 函数声明 *********************************************/

void i2c_gpio_init(void);
void i2c_nvic_init(void);
void i2c_periph_init(void);

/********************************************** 函数定义 *********************************************/

/**
 * @brief I2C初始化程序
 * 
 */
void bsp_i2c_init(void) {
  I2C_ENABLE_CLOCK();     // 使能时钟
  i2c_gpio_init();        // GPIO初始化
  i2c_dmarx_config();     // DMA初始化
  i2c_dmatx_config();
  i2c_nvic_init();        // NVIC初始化
  i2c_periph_init();      // I2C初始化
}

void i2c_gpio_init(void) {
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
}

void i2c_nvic_init(void) {
  NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));         // 使能I2C事件中断
  NVIC_EnableIRQ(I2C1_EV_IRQn);
  NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));         // 使能I2C错误中断
  NVIC_EnableIRQ(I2C1_ER_IRQn);
  NVIC_SetPriority(I2C1_RX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));     // 使能I2C RX.DMA中断
  NVIC_EnableIRQ(I2C1_RX_DMA_IRQN);
  NVIC_SetPriority(I2C1_TX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));     // 使能I2C TX.DMA中断
  NVIC_EnableIRQ(I2C1_TX_DMA_IRQN);
}

void i2c_periph_init(void) {
  LL_I2C_InitTypeDef I2C_InitStruct = {0};
  LL_I2C_DeInit(I2C1);
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

void i2c_dmarx_config(void) {
  /*
  LL_DMA_DisableStream(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);                                                // 关闭DMA数据流
  while (LL_DMA_IsEnabledStream(I2C1_RX_DMA, I2C1_RX_DMA_STREAM));                                      // 检查DMA数据流是否关闭 
  */
  LL_DMA_DeInit(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);                                                       // DMA 配置初始化
  LL_DMA_SetChannelSelection(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, I2C1_RX_DMA_CHANNAL);                     // DMA 通道选择
  LL_DMA_SetDataTransferDirection(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);  // DMA 数据传输方向：外设->内存
  LL_DMA_SetStreamPriorityLevel(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);               // DMA 数据流优先级：MEDIUM
  LL_DMA_SetMode(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, LL_DMA_MODE_NORMAL);                                  // DMA 模式：正常模式
  LL_DMA_SetPeriphIncMode(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);                  // DMA 外设地址自增：否
  LL_DMA_SetMemoryIncMode(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);                    // DMA 内存地址自增：是
  LL_DMA_SetPeriphSize(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);                        // DMA 外设数据宽度：BYTE
  LL_DMA_SetMemorySize(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);                        // DMA 内存数据宽度：BYTE
  LL_DMA_DisableFifoMode(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);                                              // DMA 硬件FIFO配置：关闭
  LL_DMA_SetPeriphAddress(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, LL_I2C_DMA_GetRegAddr(I2C1));                // 设置外设地址
}

void i2c_dmatx_config(void) {
  /*
  LL_DMA_DisableStream(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);                                                // 关闭DMA数据流
  while (LL_DMA_IsEnabledStream(I2C1_TX_DMA, I2C1_TX_DMA_STREAM));                                      // 检查DMA数据流是否关闭 
  */
  LL_DMA_DeInit(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);                                                       // DMA 反初始化
  LL_DMA_SetChannelSelection(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, I2C1_TX_DMA_CHANNAL);                     // DMA 通道选择
  LL_DMA_SetDataTransferDirection(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);  // DMA 数据传输方向：内存->外设
  LL_DMA_SetStreamPriorityLevel(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);               // DMA 数据流优先级：MEDIUM
  LL_DMA_SetMode(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, LL_DMA_MODE_NORMAL);                                  // DMA 模式：正常模式
  LL_DMA_SetPeriphIncMode(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);                  // DMA 外设地址自增：否
  LL_DMA_SetMemoryIncMode(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);                    // DMA 内存地址自增：是
  LL_DMA_SetPeriphSize(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);                        // DMA 外设数据宽度：BYTE
  LL_DMA_SetMemorySize(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);                        // DMA 内存数据宽度：BYTE
  LL_DMA_DisableFifoMode(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);                                              // DMA 硬件FIFO配置：关闭
  LL_DMA_SetPeriphAddress(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, LL_I2C_DMA_GetRegAddr(I2C1));                // 设置外设地址
}

void i2c_bus_reset(void) {
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
    i2c_delay_us(I2C1_CLK_DELAY);
    LL_GPIO_SetOutputPin(I2C1_SCL_PORT, I2C1_SCL_PIN);
    i2c_delay_us(I2C1_CLK_DELAY);
  }
  bsp_i2c_init();
}

int i2c_bus_check(void) {
  int ticks = I2C1_TIMEOUT;
  while ((LL_I2C_IsActiveFlag_ARLO(I2C1) || LL_I2C_IsActiveFlag_BERR(I2C1) || LL_I2C_IsActiveFlag_BUSY(I2C1))) {
    if (ticks-- == 0) {
      i2c_bus_reset();  
      return 1;
    }
  }
  return 0;
}

