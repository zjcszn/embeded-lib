#include "bsp_usart.h"


/******************************************* USART1 初始化 *******************************************/

void bsp_usart1_gpio_init(void);
void bsp_usart1_uart_init(void);
void bsp_usart1_nvic_init(void);

/**
 * @brief 串口初始化程序：GPIO初始化 | NVIC初始化 | USART初始化
 * 
 */
void bsp_usart1_init(void) {
  USART1_ENABLE_CLOCK();

  bsp_usart1_gpio_init();
  bsp_usart1_nvic_init();
  bsp_usart1_uart_init();

  LL_USART_EnableIT_RXNE(USART1);           // 使能串口RX非空中断
  LL_USART_Enable(USART1);                  // 使能串口
}

/**
 * @brief usart1 GPIO初始化
 * 
 */
void bsp_usart1_gpio_init(void) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                        // GPIO 模式: 复用模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;                      // GPIO 速度：High
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                 // GPIO 输出模式：推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;                               // GPIO 上下拉：上拉

  GPIO_InitStruct.Pin = USART1_RX_GPIO_PIN;                             // RX_PIN
  GPIO_InitStruct.Alternate = USART1_RX_GPIO_AF;                        // GPIO 复用功能设置：AF7
  LL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);                  // RX GPIO 初始化

  GPIO_InitStruct.Pin = USART1_TX_GPIO_PIN;                             // TX_PIN
  GPIO_InitStruct.Alternate = USART1_TX_GPIO_AF;                        // GPIO 复用功能设置：AF7
  LL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);                  // TX GPIO 初始化
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
 * @brief usart1 串口中断初始化
 * 
 */
void bsp_usart1_nvic_init(void) {
  NVIC_SetPriority(USART1_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));             // 设置串口中断优先级
  NVIC_EnableIRQ(USART1_IRQN);                                                                     // 使能串口中断       
}

/******************************************* USART2 初始化 *******************************************/


void bsp_usart2_gpio_init(void);
void bsp_usart2_uart_init(void);
void bsp_usart2_nvic_init(void);
void bsp_usart2_rs485_init(void);

/**
 * @brief 串口初始化程序：GPIO初始化 | RS485初始化 | NVIC初始化 | USART初始化
 * 
 */
void bsp_usart2_init(void) {

  USART2_ENABLE_CLOCK();

  bsp_usart2_gpio_init();
  bsp_usart2_rs485_init();
  bsp_usart2_nvic_init();
  bsp_usart2_uart_init();

  LL_USART_EnableIT_RXNE(USART2);           // 使能串口RX非空中断
  LL_USART_Enable(USART2);                  // 使能串口
}

/**
 * @brief usart2 GPIO初始化
 * 
 */
void bsp_usart2_gpio_init(void) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                        // GPIO 模式: 复用模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;                      // GPIO 速度：High
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                 // GPIO 输出模式：推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;                               // GPIO 上下拉：上拉

  GPIO_InitStruct.Pin = USART2_RX_GPIO_PIN;                             // RX_PIN
  GPIO_InitStruct.Alternate = USART2_RX_GPIO_AF;                        // GPIO 复用功能设置：AF7
  LL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);                  // RX GPIO 初始化

  GPIO_InitStruct.Pin = USART2_TX_GPIO_PIN;                             // TX_PIN
  GPIO_InitStruct.Alternate = USART2_TX_GPIO_AF;                        // GPIO 复用功能设置：AF7
  LL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);                  // TX GPIO 初始化
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
 * @brief usart2 串口中断初始化
 * 
 */
void bsp_usart2_nvic_init(void) {
  NVIC_SetPriority(USART2_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));             // 设置串口中断优先级
  NVIC_EnableIRQ(USART2_IRQN);                                                                     // 使能串口中断       
}

/**
 * @brief usart2 RS485初始化
 * 
 */
void bsp_usart2_rs485_init(void) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;                           // GPIO 模式: 输出模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;                      // GPIO 速度：High
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                 // GPIO 输出模式：推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                               // GPIO 上下拉：无
  GPIO_InitStruct.Pin = USART2_RS485_RE_PIN;                            // GPIO PIN: RS485 RE
  LL_GPIO_Init(USART2_RS485_RE_PORT, &GPIO_InitStruct);                 // RS485 RE GPIO 初始化
}
