#include "bsp_usart.h"


/****************************************** USART1 GPIO/DMA 初始化 ******************************************/

void bsp_usart1_gpio_init(void);
void bsp_usart1_uart_init(void);
void bsp_usart1_nvic_init(void);
void bsp_usart1_dmarx_config(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size);

/**
 * @brief 串口初始化程序：GPIO初始化 | DMA初始化 | NVIC初始化 | USART初始化
 * 
 * @param dmarx_buf_addr RX DMA缓冲区地址
 * @param dmarx_buf_size RX DMA缓冲区大小
 */
void bsp_usart1_init(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size) {

  USART1_ENABLE_CLOCK();

  bsp_usart1_gpio_init();
  bsp_usart1_dmarx_config(dmarx_buf_addr, dmarx_buf_size);
  bsp_usart1_nvic_init();
  bsp_usart1_uart_init();

  LL_USART_EnableDMAReq_RX(USART1);         // 使能串口RX DMA传输
  LL_USART_EnableIT_IDLE(USART1);           // 使能串口空闲中断
  LL_USART_Enable(USART1);                  // 使能串口
}

/**
 * @brief usart1 GPIO初始化
 * 
 */
void bsp_usart1_gpio_init(void) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = USART1_RX_GPIO_PIN | USART1_TX_GPIO_PIN;        // RX_PIN | TX_PIN
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                        // GPIO 模式: 复用模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;                      // GPIO 速度：High
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                 // GPIO 输出模式：推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;                               // GPIO 上下拉：上拉
  GPIO_InitStruct.Alternate = USART1_GPIO_AF;                           // GPIO 复用功能设置：AF7
  LL_GPIO_Init(USART1_GPIO_PORT, &GPIO_InitStruct);                     // GPIO 初始化
}

/**
 * @brief usart1 串口初始化
 * 
 */
void bsp_usart1_uart_init(void) {
  LL_USART_InitTypeDef USART_InitStruct = {0};
  USART_InitStruct.BaudRate = 115200;                                   // 波特率：115200
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;                   // 数据宽度：8bit（含校验位）
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;                      // 停止位：1bit
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;                       // 校验位：无
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;        // 传输方向：双工
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;       // 硬件流控制：无
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;             // 过采样：16bit
  LL_USART_Init(USART1, &USART_InitStruct);                             // 串口初始化
  LL_USART_ConfigAsyncMode(USART1);                                     // 设置USART模式：异步模式
}

/**
 * @brief usart1 串口中断及DMA中断初始化
 * 
 */
void bsp_usart1_nvic_init(void) {
  NVIC_SetPriority(USART1_RX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));     // 设置串口RX DMA中断优先级
  NVIC_EnableIRQ(USART1_RX_DMA_IRQN);                                                              // 使能串口RX DMA中断 
  NVIC_SetPriority(USART1_TX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));     // 设置串口TX DMA中断优先级
  NVIC_EnableIRQ(USART1_TX_DMA_IRQN);                                                              // 使能串口TX DMA中断
  NVIC_SetPriority(USART1_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));             // 设置串口中断优先级
  NVIC_EnableIRQ(USART1_IRQN);                                                                     // 使能串口中断       
}

/**
 * @brief usart1 rx DMA配置
 * 
 * @param dmarx_buf_addr rx DMA缓冲区地址
 * @param dmarx_buf_size rx DMA缓冲区大小
 */
void bsp_usart1_dmarx_config(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size) {

  LL_DMA_DeInit(USART1_RX_DMA, USART1_RX_DMA_STREAM);                                                       // DMA 反初始化
  LL_DMA_SetChannelSelection(USART1_RX_DMA, USART1_RX_DMA_STREAM, USART1_RX_DMA_CHANNAL);                   // DMA 通道选择：通道4
  LL_DMA_SetDataTransferDirection(USART1_RX_DMA, USART1_RX_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);  // DMA 数据传输方向：外设->内存
  LL_DMA_SetStreamPriorityLevel(USART1_RX_DMA, USART1_RX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);               // DMA 数据流优先级：MEDIUM
  LL_DMA_SetMode(USART1_RX_DMA, USART1_RX_DMA_STREAM, LL_DMA_MODE_CIRCULAR);                                // DMA 模式：循环模式
  LL_DMA_SetPeriphIncMode(USART1_RX_DMA, USART1_RX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);                  // DMA 外设地址自增：否
  LL_DMA_SetMemoryIncMode(USART1_RX_DMA, USART1_RX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);                    // DMA 内存地址自增：是
  LL_DMA_SetPeriphSize(USART1_RX_DMA, USART1_RX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);                        // DMA 外设数据宽度：BYTE
  LL_DMA_SetMemorySize(USART1_RX_DMA, USART1_RX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);                        // DMA 内存数据宽度：BYTE
  LL_DMA_DisableFifoMode(USART1_RX_DMA, USART1_RX_DMA_STREAM);                                              // DMA 硬件FIFO配置：关闭

  LL_DMA_SetPeriphAddress(USART1_RX_DMA, USART1_RX_DMA_STREAM, LL_USART_DMA_GetRegAddr(USART1));            // 设置外设地址
  LL_DMA_SetMemoryAddress(USART1_RX_DMA, USART1_RX_DMA_STREAM, (uint32_t)dmarx_buf_addr);                   // 设置内存地址
  LL_DMA_SetDataLength(USART1_RX_DMA, USART1_RX_DMA_STREAM, dmarx_buf_size);                                // 设置DMA传输数据长度

  LL_DMA_EnableIT_HT(USART1_RX_DMA, USART1_RX_DMA_STREAM);                                                  // 使能DMA半满中断
  LL_DMA_EnableIT_TC(USART1_RX_DMA, USART1_RX_DMA_STREAM);                                                  // 使能DMA发送完成中断
  LL_DMA_EnableStream(USART1_RX_DMA, USART1_RX_DMA_STREAM);                                                 // 使能DMA数据流

}

/**
 * @brief usart1 tx DMA配置
 * 
 * @param dmatx_buf_addr tx DMA缓冲区地址
 * @param data_length tx 发送字节长度
 */
void bsp_usart1_dmatx_config(uint8_t *dmatx_buf_addr, uint32_t data_length) {

  LL_DMA_DisableStream(USART1_TX_DMA, USART1_TX_DMA_STREAM);                                                // 关闭DMA数据流
  while (LL_DMA_IsEnabledStream(USART1_TX_DMA, USART1_TX_DMA_STREAM));                                      // 检查DMA数据流是否关闭
  LL_USART_DisableDMAReq_TX(USART1);                                                                        // 关闭串口TX DMA请求
  LL_DMA_SetChannelSelection(USART1_TX_DMA, USART1_TX_DMA_STREAM, USART1_TX_DMA_CHANNAL);                   // DMA 通道选择：通道4 
  LL_DMA_SetDataTransferDirection(USART1_TX_DMA, USART1_TX_DMA_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);  // DMA 数据传输方向：内存->外设
  LL_DMA_SetStreamPriorityLevel(USART1_TX_DMA, USART1_TX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);               // DMA 数据流优先级：MEDIUM
  LL_DMA_SetMode(USART1_TX_DMA, USART1_TX_DMA_STREAM, LL_DMA_MODE_NORMAL);                                  // DMA 模式：正常模式
  LL_DMA_SetPeriphIncMode(USART1_TX_DMA, USART1_TX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);                  // DMA 外设地址自增：否
  LL_DMA_SetMemoryIncMode(USART1_TX_DMA, USART1_TX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);                    // DMA 内存地址自增：是
  LL_DMA_SetPeriphSize(USART1_TX_DMA, USART1_TX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);                        // DMA 外设数据宽度：BYTE
  LL_DMA_SetMemorySize(USART1_TX_DMA, USART1_TX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);                        // DMA 内存数据宽度：BYTE
  LL_DMA_DisableFifoMode(USART1_TX_DMA, USART1_TX_DMA_STREAM);                                              // DMA 硬件FIFO配置：关闭

  LL_DMA_SetPeriphAddress(USART1_TX_DMA, USART1_TX_DMA_STREAM, LL_USART_DMA_GetRegAddr(USART1));            // 设置外设地址
  LL_DMA_SetMemoryAddress(USART1_TX_DMA, USART1_TX_DMA_STREAM, (uint32_t)dmatx_buf_addr);                   // 设置内存地址
  LL_DMA_SetDataLength(USART1_TX_DMA, USART1_TX_DMA_STREAM, data_length);                                   // 设置DMA传输数据长度

  DMA_CLEAR_IT_FLAG(USART1_TX_DMA_STREAM_ID, TC,  USART1_TX_DMA);         // 清除发送完成标志
  DMA_CLEAR_IT_FLAG(USART1_TX_DMA_STREAM_ID, HT,  USART1_TX_DMA);         // 清除半满标志
  DMA_CLEAR_IT_FLAG(USART1_TX_DMA_STREAM_ID, TE,  USART1_TX_DMA);         // 清除发送错误标志
  DMA_CLEAR_IT_FLAG(USART1_TX_DMA_STREAM_ID, DME, USART1_TX_DMA);         // 清除传输方向错误标志
  DMA_CLEAR_IT_FLAG(USART1_TX_DMA_STREAM_ID, FE,  USART1_TX_DMA);         // 清除FIFO错误标志
      
  LL_DMA_EnableIT_TC(USART1_TX_DMA, USART1_TX_DMA_STREAM);                // 使能发送完成中断
  LL_DMA_EnableStream(USART1_TX_DMA, USART1_TX_DMA_STREAM);               // 使能DMA数据流
  LL_USART_EnableDMAReq_TX(USART1);                                       // 使能串口TX DMA传输
} 

uint16_t bsp_usart1_dmarx_buf_remain_size(void) {
  return LL_DMA_GetDataLength(USART1_RX_DMA, USART1_RX_DMA_STREAM);
}

/****************************************** USART2 GPIO/DMA 初始化 ******************************************/


void bsp_usart2_gpio_init(void);
void bsp_usart2_uart_init(void);
void bsp_usart2_nvic_init(void);
void bsp_usart2_dmarx_config(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size);

/**
 * @brief 串口初始化程序：GPIO初始化 | DMA初始化 | NVIC初始化 | USART初始化
 * 
 * @param dmarx_buf_addr RX DMA缓冲区地址
 * @param dmarx_buf_size RX DMA缓冲区大小
 */
void bsp_usart2_init(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size) {

  USART2_ENABLE_CLOCK();

  bsp_usart2_gpio_init();
  bsp_usart2_dmarx_config(dmarx_buf_addr, dmarx_buf_size);
  bsp_usart2_nvic_init();
  bsp_usart2_uart_init();

  LL_USART_EnableDMAReq_RX(USART2);         // 使能串口RX DMA传输
  LL_USART_EnableIT_IDLE(USART2);           // 使能串口空闲中断
  LL_USART_Enable(USART2);                  // 使能串口
}

/**
 * @brief usart2 GPIO初始化
 * 
 */
void bsp_usart2_gpio_init(void) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = USART2_RX_GPIO_PIN | USART2_TX_GPIO_PIN;        // RX_PIN | TX_PIN
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                        // GPIO 模式: 复用模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;                      // GPIO 速度：High
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                 // GPIO 输出模式：推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;                               // GPIO 上下拉：上拉
  GPIO_InitStruct.Alternate = USART2_GPIO_AF;                           // GPIO 复用功能设置：AF7
  LL_GPIO_Init(USART2_GPIO_PORT, &GPIO_InitStruct);                     // GPIO 初始化
}

/**
 * @brief usart2 串口初始化
 * 
 */
void bsp_usart2_uart_init(void) {
  LL_USART_InitTypeDef USART_InitStruct = {0};
  USART_InitStruct.BaudRate = 115200;                                   // 波特率：115200
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;                   // 数据宽度：8bit（含校验位）
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;                      // 停止位：1bit
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;                       // 校验位：无
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;        // 传输方向：双工
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;       // 硬件流控制：无
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;             // 过采样：16bit
  LL_USART_Init(USART2, &USART_InitStruct);                             // 串口初始化
  LL_USART_ConfigAsyncMode(USART2);                                     // 设置USART模式：异步模式
}

/**
 * @brief usart2 串口中断及DMA中断初始化
 * 
 */
void bsp_usart2_nvic_init(void) {
  NVIC_SetPriority(USART2_RX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));     // 设置串口RX DMA中断优先级
  NVIC_EnableIRQ(USART2_RX_DMA_IRQN);                                                              // 使能串口RX DMA中断 
  NVIC_SetPriority(USART2_TX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 1));     // 设置串口TX DMA中断优先级
  NVIC_EnableIRQ(USART2_TX_DMA_IRQN);                                                              // 使能串口TX DMA中断
  NVIC_SetPriority(USART2_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));             // 设置串口中断优先级
  NVIC_EnableIRQ(USART2_IRQN);                                                                     // 使能串口中断       
}

/**
 * @brief usart2 rx DMA配置
 * 
 * @param dmarx_buf_addr rx DMA缓冲区地址
 * @param dmarx_buf_size rx DMA缓冲区大小
 */
void bsp_usart2_dmarx_config(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size) {

  LL_DMA_DeInit(USART2_RX_DMA, USART2_RX_DMA_STREAM);                                                       // DMA 反初始化
  LL_DMA_SetChannelSelection(USART2_RX_DMA, USART2_RX_DMA_STREAM, USART2_RX_DMA_CHANNAL);                   // DMA 通道选择：通道4
  LL_DMA_SetDataTransferDirection(USART2_RX_DMA, USART2_RX_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);  // DMA 数据传输方向：外设->内存
  LL_DMA_SetStreamPriorityLevel(USART2_RX_DMA, USART2_RX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);               // DMA 数据流优先级：MEDIUM
  LL_DMA_SetMode(USART2_RX_DMA, USART2_RX_DMA_STREAM, LL_DMA_MODE_CIRCULAR);                                // DMA 模式：循环模式
  LL_DMA_SetPeriphIncMode(USART2_RX_DMA, USART2_RX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);                  // DMA 外设地址自增：否
  LL_DMA_SetMemoryIncMode(USART2_RX_DMA, USART2_RX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);                    // DMA 内存地址自增：是
  LL_DMA_SetPeriphSize(USART2_RX_DMA, USART2_RX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);                        // DMA 外设数据宽度：BYTE
  LL_DMA_SetMemorySize(USART2_RX_DMA, USART2_RX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);                        // DMA 内存数据宽度：BYTE
  LL_DMA_DisableFifoMode(USART2_RX_DMA, USART2_RX_DMA_STREAM);                                              // DMA 硬件FIFO配置：关闭

  LL_DMA_SetPeriphAddress(USART2_RX_DMA, USART2_RX_DMA_STREAM, LL_USART_DMA_GetRegAddr(USART2));            // 设置外设地址
  LL_DMA_SetMemoryAddress(USART2_RX_DMA, USART2_RX_DMA_STREAM, (uint32_t)dmarx_buf_addr);                   // 设置内存地址
  LL_DMA_SetDataLength(USART2_RX_DMA, USART2_RX_DMA_STREAM, dmarx_buf_size);                                // 设置DMA传输数据长度

  LL_DMA_EnableIT_HT(USART2_RX_DMA, USART2_RX_DMA_STREAM);                                                  // 使能DMA半满中断
  LL_DMA_EnableIT_TC(USART2_RX_DMA, USART2_RX_DMA_STREAM);                                                  // 使能DMA发送完成中断
  LL_DMA_EnableStream(USART2_RX_DMA, USART2_RX_DMA_STREAM);                                                 // 使能DMA数据流

}

/**
 * @brief usart2 tx DMA配置
 * 
 * @param dmatx_buf_addr tx DMA缓冲区地址
 * @param data_length tx 发送字节长度
 */
void bsp_usart2_dmatx_config(uint8_t *dmatx_buf_addr, uint32_t data_length) {

  LL_DMA_DisableStream(USART2_TX_DMA, USART2_TX_DMA_STREAM);                                                // 关闭DMA数据流
  while (LL_DMA_IsEnabledStream(USART2_TX_DMA, USART2_TX_DMA_STREAM));                                      // 检查DMA数据流是否关闭
   
  LL_DMA_SetChannelSelection(USART2_TX_DMA, USART2_TX_DMA_STREAM, USART2_TX_DMA_CHANNAL);                   // DMA 通道选择：通道4 
  LL_DMA_SetDataTransferDirection(USART2_TX_DMA, USART2_TX_DMA_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);  // DMA 数据传输方向：内存->外设
  LL_DMA_SetStreamPriorityLevel(USART2_TX_DMA, USART2_TX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);               // DMA 数据流优先级：MEDIUM
  LL_DMA_SetMode(USART2_TX_DMA, USART2_TX_DMA_STREAM, LL_DMA_MODE_NORMAL);                                  // DMA 模式：正常模式
  LL_DMA_SetPeriphIncMode(USART2_TX_DMA, USART2_TX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);                  // DMA 外设地址自增：否
  LL_DMA_SetMemoryIncMode(USART2_TX_DMA, USART2_TX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);                    // DMA 内存地址自增：是
  LL_DMA_SetPeriphSize(USART2_TX_DMA, USART2_TX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);                        // DMA 外设数据宽度：BYTE
  LL_DMA_SetMemorySize(USART2_TX_DMA, USART2_TX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);                        // DMA 内存数据宽度：BYTE
  LL_DMA_DisableFifoMode(USART2_TX_DMA, USART2_TX_DMA_STREAM);                                              // DMA 硬件FIFO配置：关闭

  LL_DMA_SetPeriphAddress(USART2_TX_DMA, USART2_TX_DMA_STREAM, LL_USART_DMA_GetRegAddr(USART2));            // 设置外设地址
  LL_DMA_SetMemoryAddress(USART2_TX_DMA, USART2_TX_DMA_STREAM, (uint32_t)dmatx_buf_addr);                   // 设置内存地址
  LL_DMA_SetDataLength(USART2_TX_DMA, USART2_TX_DMA_STREAM, data_length);                                   // 设置DMA传输数据长度

  DMA_CLEAR_IT_FLAG(USART2_TX_DMA_STREAM_ID, TC,  USART2_TX_DMA);         // 清除发送完成标志
  DMA_CLEAR_IT_FLAG(USART2_TX_DMA_STREAM_ID, HT,  USART2_TX_DMA);         // 清除半满标志
  DMA_CLEAR_IT_FLAG(USART2_TX_DMA_STREAM_ID, TE,  USART2_TX_DMA);         // 清除发送错误标志
  DMA_CLEAR_IT_FLAG(USART2_TX_DMA_STREAM_ID, DME, USART2_TX_DMA);         // 清除传输方向错误标志
  DMA_CLEAR_IT_FLAG(USART2_TX_DMA_STREAM_ID, FE,  USART2_TX_DMA);         // 清除FIFO错误标志
      
  LL_DMA_EnableIT_TC(USART2_TX_DMA, USART2_TX_DMA_STREAM);                // 使能发送完成中断
  LL_DMA_EnableStream(USART2_TX_DMA, USART2_TX_DMA_STREAM);               // 使能DMA数据流
  LL_USART_EnableDMAReq_TX(USART2);                                       // 使能串口TX DMA传输

} 

uint16_t bsp_usart2_dmarx_buf_remain_size(void) {
  return LL_DMA_GetDataLength(USART2_RX_DMA, USART2_RX_DMA_STREAM);
}