#include "bsp_spi_master_poll.h"

/*********************************************** 宏定义 **********************************************/

#define SPI_ENABLE_GPIO_CLOCK()           LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB)   // 使能GPIO时钟
#define SPI_ENABLE_PERIPH_CLOCK()         LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1)    // 使能SPI时钟

#define SPI_ENABLE_CLOCK()        \
  do {                            \
    SPI_ENABLE_GPIO_CLOCK();      \
    SPI_ENABLE_PERIPH_CLOCK();    \
  } while(0U)

#define DUMMY_DATA                        0xFF

/********************************************** 函数定义 *********************************************/


void bsp_spi1_init(void)
{
  /* Enable Clock */
  SPI_ENABLE_CLOCK();

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = SPI1_CLK_PIN | SPI1_MISO_PIN | SPI1_MOSI_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                          // GPIO模式：复用模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;                   // GPIO速率：VeryHigh
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                   // GPIO输出模式：推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                                 // GPIO上下拉：无
  GPIO_InitStruct.Alternate = SPI1_GPIO_AF;                               // GPIO复用选择：AF5
  LL_GPIO_Init(SPI1_GPIO, &GPIO_InitStruct);                              // 初始化 CLK | MISO | MOSI

  SPI1_CS_DISABLED();
  GPIO_InitStruct.Pin = SPI1_CS_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;                             // GPIO模式：输出模式
  LL_GPIO_Init(SPI1_CS_GPIO, &GPIO_InitStruct);                           // 初始化 CS

/*
  // SPI1 interrupt Init
  NVIC_SetPriority(SPI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));        // 使能SPI NVIC中断
  NVIC_EnableIRQ(SPI1_IRQn);
*/

  /* SPI1 peripheral init*/
  LL_SPI_InitTypeDef  SPI_InitStruct  = {0};
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;                  // SPI传输方向：全双工
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;                               // SPI模式：主机模式
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;                       // SPI数据宽度：8bit
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;                     // SPI时钟极性：空闲时低电平
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;                         // SPI时钟相位：第一个时钟沿采样
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;                                   // SPI片选信号：软件控制
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;                // SPI波特率：fPCLK / 8
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;                             // SPI比特序：先发MSB
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;          // SPI CRC校验功能：禁止
  SPI_InitStruct.CRCPoly = 10;                                            // SPI CRC多项式：X3 + X7
  LL_SPI_Init(SPI1, &SPI_InitStruct);                                     // 初始化SPI
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);                     // SPI协议标准：MotoRola
  LL_SPI_Enable(SPI1);                                                    // 使能SPI
}

void spi_master_write_poll(uint8_t *send_data, uint32_t length) {
  while (LL_SPI_IsActiveFlag_BSY(SPI1));
  LL_SPI_TransmitData8(SPI1, *send_data++);                               // 发送第一个字节数据
  for (int i = 0; i < length - 1; i++) {                                  // 发送剩余字节数据
    while (!LL_SPI_IsActiveFlag_TXE(SPI1));
    LL_SPI_TransmitData8(SPI1, *send_data++);
    while (!LL_SPI_IsActiveFlag_RXNE(SPI1));
    (void)LL_SPI_ReceiveData8(SPI1);
  }
  while (!LL_SPI_IsActiveFlag_RXNE(SPI1));
  (void)LL_SPI_ReceiveData8(SPI1);
}

void spi_master_read_poll(uint8_t *recv_data, uint32_t length) {
  while (LL_SPI_IsActiveFlag_BSY(SPI1));
  LL_SPI_TransmitData8(SPI1, DUMMY_DATA);                                  // 发送脏数据
  for (int i = 0; i < length - 1; i++) {
    while (!LL_SPI_IsActiveFlag_TXE(SPI1));
    LL_SPI_TransmitData8(SPI1, DUMMY_DATA);
    while (!LL_SPI_IsActiveFlag_RXNE(SPI1));
    *recv_data++ = LL_SPI_ReceiveData8(SPI1);                              // 接收字节数据
  }
  while (!LL_SPI_IsActiveFlag_RXNE(SPI1));
  *recv_data = LL_SPI_ReceiveData8(SPI1);                                  // 接收最后一个字节数据
}

void spi_master_write_read_poll(uint8_t *send_data, uint32_t send_len, uint8_t *recv_data, uint32_t recv_len) {
  while (LL_SPI_IsActiveFlag_BSY(SPI1));
  LL_SPI_TransmitData8(SPI1, *send_data++);                                // 发送第一个字节数据
  for (int i = 0; i < send_len - 1; i++) {
    while (!LL_SPI_IsActiveFlag_TXE(SPI1));
    LL_SPI_TransmitData8(SPI1, *send_data++);                              // 发送剩余字节数据
    while (!LL_SPI_IsActiveFlag_RXNE(SPI1));
    (void)LL_SPI_ReceiveData8(SPI1);
  }

  while (!LL_SPI_IsActiveFlag_TXE(SPI1));
  LL_SPI_TransmitData8(SPI1, DUMMY_DATA);                                  // 发送脏数据
  while (!LL_SPI_IsActiveFlag_RXNE(SPI1));
  (void)LL_SPI_ReceiveData8(SPI1);

  for (int i = 0; i < recv_len - 1; i++) {
    while (!LL_SPI_IsActiveFlag_TXE(SPI1));
    LL_SPI_TransmitData8(SPI1, DUMMY_DATA);
    while (!LL_SPI_IsActiveFlag_RXNE(SPI1));
    *recv_data++ = LL_SPI_ReceiveData8(SPI1);                              // 接收字节数据                         
  }
  while (!LL_SPI_IsActiveFlag_RXNE(SPI1));
  *recv_data = LL_SPI_ReceiveData8(SPI1);                                  // 接收最后一个字节数据
}

uint8_t spi_master_write_read_byte(uint8_t c) {
  while (LL_SPI_IsActiveFlag_BSY(SPI1));
  LL_SPI_TransmitData8(SPI1, c);
  while (!LL_SPI_IsActiveFlag_RXNE(SPI1));
  return LL_SPI_ReceiveData8(SPI1);
}