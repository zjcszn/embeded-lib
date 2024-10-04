#include "bsp_spi_master_soft.h"

static SoftSPI_Device_T soft_spi_dev;

/*********************************************** 宏定义 **********************************************/

#define SOFT_SPI_MOSI_H         LL_GPIO_SetOutputPin(SOFT_SPI_MOSI_PORT, SOFT_SPI_MOSI_PIN)   // MOSI置1
#define SOFT_SPI_MOSI_L         LL_GPIO_ResetOutputPin(SOFT_SPI_MOSI_PORT, SOFT_SPI_MOSI_PIN) // MOSI置0

#define SOFT_SPI_NSS_H          LL_GPIO_SetOutputPin(SOFT_SPI_NSS_PORT, SOFT_SPI_NSS_PIN)     // 失能片选信号
#define SOFT_SPI_NSS_L          LL_GPIO_ResetOutputPin(SOFT_SPI_NSS_PORT, SOFT_SPI_NSS_PIN)   // 使能片选信号


#define SOFT_SPI_SCK_1ST        SOFT_SPI_SCK_PORT->BSRR = soft_spi_dev.sck_1        // 生成SCK信号第1边沿
#define SOFT_SPI_SCK_2ST        SOFT_SPI_SCK_PORT->BSRR = soft_spi_dev.sck_2        // 生成SCK信号第2边沿


#define SOFT_SPI_MISO_INPUT()   LL_GPIO_IsInputPinSet(SOFT_SPI_MISO_PORT, SOFT_SPI_MISO_PIN)  // 读取MISO输入

/********************************************** 函数定义 *********************************************/

/**
 * @brief 模拟SPI初始化函数
 * 
 * @param cpol SPI时钟极性 SoftSPI_CPOL_L     | SoftSPI_CPOL_H
 * @param cpha SPI时钟相位 SoftSPI_CPHA_1EDGE | SoftSPI_CPHA_2EDGE
 */
void soft_spi_init(uint8_t cpol, uint8_t cpha) {
  soft_spi_dev.cpol   = cpol;
  soft_spi_dev.cpha   = cpha;
  soft_spi_dev.lock   = SOFT_SPI_UNLOCKED;
  soft_spi_dev.sck_1  = SOFT_SPI_SCK_PIN << ( (soft_spi_dev.cpol) << 4U);
  soft_spi_dev.sck_2  = SOFT_SPI_SCK_PIN << (!(soft_spi_dev.cpol) << 4U);

  SOFT_SPI_ENABLE_CLOCK();                                          // 使能时钟
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;                       // GPIO模式：输出模式
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;             // 推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                           // 上下拉：无
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;             // GPIO速率：VERY HIGH
  GPIO_InitStruct.Pin = SOFT_SPI_NSS_PIN;           
  LL_GPIO_Init(SOFT_SPI_NSS_PORT, &GPIO_InitStruct);                // 初始化NSS引脚
  LL_GPIO_SetOutputPin(SOFT_SPI_NSS_PORT, SOFT_SPI_NSS_PIN);        // NSS置1

  GPIO_InitStruct.Pin = SOFT_SPI_SCK_PIN;                     
  LL_GPIO_Init(SOFT_SPI_SCK_PORT, &GPIO_InitStruct);                // 初始化SCK引脚
  SOFT_SPI_SCK_PORT->BSRR = (SOFT_SPI_SCK_PIN << (!cpol << 4U));    // SCK空闲电平设置

  GPIO_InitStruct.Pin = SOFT_SPI_MOSI_PIN;                          
  LL_GPIO_Init(SOFT_SPI_MOSI_PORT, &GPIO_InitStruct);               // 初始化MOSI引脚
  LL_GPIO_ResetOutputPin(SOFT_SPI_MOSI_PORT, SOFT_SPI_NSS_PIN);     // MOSI置0

  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;                        // GPIO模式：输入模式
  GPIO_InitStruct.Pin = SOFT_SPI_MISO_PIN;                    
  LL_GPIO_Init(SOFT_SPI_MISO_PORT, &GPIO_InitStruct);               // 初始化MISO引脚
}


/**
 * @brief 模拟SPI字节读写函数 
 * 
 * @param c 写入的字节数据
 * @return uint8_t 读取的字节数据
 */
static uint8_t soft_spi_write_read(uint8_t c) {
  uint8_t recv = 0;
  if (soft_spi_dev.cpha == SoftSPI_CPHA_1EDGE) {
    for (int i = 0; i < 8; i++) {
      c & 0x80 ? SOFT_SPI_MOSI_H : SOFT_SPI_MOSI_L;
      c <<= 1;
      SOFT_SPI_SCK_1ST;
      recv = (recv << 1) | (SOFT_SPI_MISO_INPUT());
      SOFT_SPI_SCK_2ST;
    }
  }
  else {
    for (int i = 0; i < 8; i++) {
      SOFT_SPI_SCK_1ST;
      (c & 0x80) ? SOFT_SPI_MOSI_H : SOFT_SPI_MOSI_L;
      c <<= 1;
      SOFT_SPI_SCK_2ST;
      recv = (recv << 1) | (SOFT_SPI_MISO_INPUT());
    }
  }
  return recv;
}

/**
 * @brief 模拟SPI写函数
 * 
 * @param txbuf_addr 待发送的数据缓冲区地址
 * @param txbuf_size 待发送的数据缓冲区大小
 */
void soft_spi_write_then_stop(uint8_t *txbuf_addr, uint32_t txbuf_size) {
  SOFT_SPI_NSS_L;
  while (txbuf_size--){
    soft_spi_write_read(*txbuf_addr++);
  }
  SOFT_SPI_NSS_H;
}

/**
 * @brief 模拟SPI写入并读取函数
 * 
 * @param txbuf_addr 待发送的数据缓冲区地址
 * @param txbuf_size 待发送的数据缓冲区大小
 * @param rxbuf_addr 待接收的数据缓冲区地址
 * @param rxbuf_size 待接收的数据缓冲区大小
 */
void soft_spi_write_then_read(uint8_t *txbuf_addr, uint32_t txbuf_size, uint8_t *rxbuf_addr, uint32_t rxbuf_size) {
  SOFT_SPI_NSS_L;
  while (txbuf_size--){
    soft_spi_write_read(*txbuf_addr++);
  }
  while (rxbuf_size--){
    *rxbuf_addr++ = soft_spi_write_read(0x00);
  }
  SOFT_SPI_NSS_H;
}

/**
 * @brief 模拟SPI连续写入函数
 * 
 * @param txbuf1_addr 待发送的数据缓冲区地址1
 * @param txbuf1_size 待发送的数据缓冲区大小1
 * @param txbuf2_addr 待发送的数据缓冲区地址2
 * @param txbuf2_size 待发送的数据缓冲区大小2
 */
void soft_spi_write_then_write(uint8_t *txbuf1_addr, uint32_t txbuf1_size, uint8_t *txbuf2_addr, uint32_t txbuf2_size) {
  SOFT_SPI_NSS_L;
  while (txbuf1_size--){
    soft_spi_write_read(*txbuf1_addr++);
  }
  while (txbuf2_size--){
    soft_spi_write_read(*txbuf2_addr++);
  }
  SOFT_SPI_NSS_H;
}

