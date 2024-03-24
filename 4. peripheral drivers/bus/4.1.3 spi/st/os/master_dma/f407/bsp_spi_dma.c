#include "bsp_spi_dma.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "semphr.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_spi.h"
#include <stdint.h>

static SPI_Device_T spi_dev = {0};
static SemaphoreHandle_t spi_sem;
static SemaphoreHandle_t spi_mux;
static uint8_t dummy = 0x00;
/********************************************** 宏定义 ***********************************************/

#define DMA_CLEAR_IT_FLAG_CONCAT(id, it, dmax)      LL_DMA_ClearFlag_##it##id(dmax)
#define DMA_CLEAR_IT_FLAG(id, it, dmax)             DMA_CLEAR_IT_FLAG_CONCAT(id, it, dmax)

#define SPI_ENABLE_GPIO_CLOCK()                     LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB)   // 使能GPIO时钟
#define SPI_ENABLE_PERIPH_CLOCK()                   LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1)    // 使能SPI时钟
#define SPI_ENABLE_DMA_CLOCK()                      LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2)    // 使能DMA时钟

#define SPI_ENABLE_CLOCK()                      \
    do {                                        \
        SPI_ENABLE_GPIO_CLOCK();                \
        SPI_ENABLE_DMA_CLOCK();                 \
        SPI_ENABLE_PERIPH_CLOCK();              \
    } while(0U)

#define SPI_LOCK()                              \
    do {                                        \
        xSemaphoreTake(spi_mux, portMAX_DELAY); \
        SPI_CS_ENABLE();                        \
    } while (0U)

#define SPI_UNLOCK()                            \
    do {                                        \
        SPI_CS_DISABLE();                       \
        xSemaphoreGive(spi_mux);                \
    } while (0U)

#define SPI_CLEAR_DMA_IT_FLAG()                                     \
    do {                                                            \
        DMA_CLEAR_IT_FLAG(SPI1_RX_DMA_STREAM_ID, FE, SPI1_RX_DMA);  \
        DMA_CLEAR_IT_FLAG(SPI1_RX_DMA_STREAM_ID, TE, SPI1_RX_DMA);  \
        DMA_CLEAR_IT_FLAG(SPI1_RX_DMA_STREAM_ID, HT, SPI1_RX_DMA);  \
        DMA_CLEAR_IT_FLAG(SPI1_RX_DMA_STREAM_ID, TC, SPI1_RX_DMA);  \
        DMA_CLEAR_IT_FLAG(SPI1_TX_DMA_STREAM_ID, FE, SPI1_TX_DMA);  \
        DMA_CLEAR_IT_FLAG(SPI1_TX_DMA_STREAM_ID, TE, SPI1_TX_DMA);  \
        DMA_CLEAR_IT_FLAG(SPI1_TX_DMA_STREAM_ID, HT, SPI1_TX_DMA);  \
        DMA_CLEAR_IT_FLAG(SPI1_TX_DMA_STREAM_ID, TC, SPI1_TX_DMA);  \
    } while (0U)

/******************************************** 私有函数声明 *********************************************/

static void spi_cs_init(void);
static void spi_gpio_init(void);
static void spi_nvic_init(void);
static void spi_periph_init(void);
static void spi_dmarx_config(void);
static void spi_dmatx_config(void);
static void spi_dmarx_enable(uint8_t *rxbuf_addr, uint16_t rxbuf_size);
static void spi_dmatx_enable(uint8_t *txbuf_addr, uint16_t txbuf_size);
static void spi_wait_trans_complete(void);

/********************************************** 函数定义 *********************************************/

/**
 * @brief SPI初始化程序
 * 
 */
void spi_master_init(void) {
    SPI_ENABLE_CLOCK();         // 使能时钟
    spi_cs_init();              // 初始化 CS引脚
    spi_gpio_init();            // 初始化 CLK | MOSI | MISO 引脚
    spi_dmarx_config();         // 初始化 RX DMA
    spi_dmatx_config();         // 初始化 TX DMA
    spi_nvic_init();            // 初始化 NVIC
    spi_periph_init();          // 初始化 SPI
    
    spi_sem = xSemaphoreCreateBinary();
    spi_mux = xSemaphoreCreateMutex();
}

/**
 * @brief SPI GPIO初始化程序
 * 
 */
static void spi_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = SPI1_CLK_PIN | SPI1_MISO_PIN | SPI1_MOSI_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                          // GPIO模式：复用模式
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;                   // GPIO速率：VeryHigh
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                   // GPIO输出模式：推挽输出
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                                 // GPIO上下拉：无
    GPIO_InitStruct.Alternate = SPI1_GPIO_AF;                               // GPIO复用选择：AF5
    LL_GPIO_Init(SPI1_GPIO, &GPIO_InitStruct);                              // 初始化 CLK | MISO | MOSI
}

/**
 * @brief SPI NVIC初始化程序
 * 
 */
static void spi_nvic_init(void) {
    NVIC_SetPriority(SPI1_RX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xE, 0));     // 使能SPI RX.DMA中断
    NVIC_EnableIRQ(SPI1_RX_DMA_IRQN);
    NVIC_SetPriority(SPI1_TX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xE, 0));     // 使能SPI TX.DMA中断
    NVIC_EnableIRQ(SPI1_TX_DMA_IRQN);
}

/**
 * @brief SPI 外设初始化程序
 * 
 */
static void spi_periph_init(void) {
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;                  // SPI传输方向：全双工
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;                               // SPI模式：主机模式
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;                       // SPI数据宽度：8bit
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;                     // SPI时钟极性：空闲时低电平
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;                         // SPI时钟相位：第一个时钟沿采样
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;                                   // SPI片选信号：软件控制
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV4;                // SPI波特率：fPCLK / 4
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;                             // SPI比特序：先发 MSB
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;          // SPI CRC校验功能：禁止
    SPI_InitStruct.CRCPoly = 10;                                            // SPI CRC多项式：X3 + X7
    LL_SPI_Init(SPI1, &SPI_InitStruct);                                     // 初始化SPI
    LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);                     // SPI协议标准：MotoRola
    LL_SPI_Enable(SPI1);                                                    // 使能SPI
}

/**
 * @brief DMA RX配置程序
 * 
 */
static void spi_dmarx_config(void) {
    LL_DMA_DeInit(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);
    LL_DMA_SetChannelSelection(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, SPI1_RX_DMA_CHANNAL);
    LL_DMA_SetDataTransferDirection(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);
    LL_DMA_SetMode(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);
    LL_DMA_SetPeriphAddress(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_SPI_DMA_GetRegAddr(SPI1));
}

/**
 * @brief DMA TX配置程序
 * 
 */
static void spi_dmatx_config(void) {
    LL_DMA_DeInit(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);
    LL_DMA_SetChannelSelection(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, SPI1_TX_DMA_CHANNAL);
    LL_DMA_SetDataTransferDirection(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetStreamPriorityLevel(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);
    LL_DMA_SetMode(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);
    LL_DMA_SetPeriphAddress(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_SPI_DMA_GetRegAddr(SPI1));
}

/**
 * @brief 使能发送DMA
 * 
 */
static void spi_dmatx_enable(uint8_t *txbuf_addr, uint16_t txbuf_size) {
    LL_SPI_DisableDMAReq_TX(SPI1);                                                            // 禁止SPI TX.DMA请求
    LL_SPI_DisableDMAReq_RX(SPI1);
    
    LL_DMA_DisableStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);
    LL_DMA_DisableStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);
    while (LL_DMA_IsEnabledStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM));
    while (LL_DMA_IsEnabledStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM));
    
    LL_DMA_SetMemoryAddress(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, (uint32_t)txbuf_addr);           // 设置内存地址
    LL_DMA_SetDataLength(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, txbuf_size);                        // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetMemoryAddress(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, (uint32_t)&dummy);               // 设置内存地址
    LL_DMA_SetDataLength(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, txbuf_size);                        // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_MEMORY_NOINCREMENT);
    
    SPI_CLEAR_DMA_IT_FLAG();
    
    LL_DMA_EnableIT_TC(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                      // 使能DMA发送完成中断
    LL_DMA_EnableIT_TE(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                      // 使能DMA发送完成中断

#if (SPI_INTERNAL_NSS == 1)
    SPI_CS_ENABLE();                                                                          // 使能片选信号
#endif
    
    LL_DMA_EnableStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                     // 使能DMA数据流
    LL_DMA_EnableStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                     // 使能DMA数据流
    LL_SPI_EnableDMAReq_RX(SPI1);                                                             // 使能SPI RX DMA请求
    LL_SPI_EnableDMAReq_TX(SPI1);                                                             // 使能SPI TX DMA请求
}

/**
 * @brief 使能接收DMA
 * 
 */
static void spi_dmarx_enable(uint8_t *rxbuf_addr, uint16_t rxbuf_size) {
    LL_SPI_DisableDMAReq_TX(SPI1);
    LL_SPI_DisableDMAReq_RX(SPI1);
    
    LL_DMA_DisableStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);
    LL_DMA_DisableStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);
    while (LL_DMA_IsEnabledStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM));
    while (LL_DMA_IsEnabledStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM));
    
    dummy = 0xFF;
    LL_DMA_SetMemoryAddress(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, (uint32_t)&dummy);               // 设置内存地址
    LL_DMA_SetDataLength(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, rxbuf_size);                        // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_MEMORY_NOINCREMENT);
    LL_DMA_SetMemoryAddress(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, (uint32_t)rxbuf_addr);           // 设置内存地址
    LL_DMA_SetDataLength(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, rxbuf_size);                        // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    
    SPI_CLEAR_DMA_IT_FLAG();
    
    LL_DMA_EnableIT_TC(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                      // 使能DMA发送完成中断
    LL_DMA_EnableIT_TE(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                      // 使能DMA发送完成中断

#if (SPI_INTERNAL_NSS == 1)
    SPI_CS_ENABLE();                                                                          // 使能片选信号
#endif
    
    (void)LL_SPI_ReceiveData8(SPI1);
    
    LL_DMA_EnableStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                     // 使能DMA数据流
    LL_DMA_EnableStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                     // 使能DMA数据流
    LL_SPI_EnableDMAReq_RX(SPI1);                                                             // 使能SPI RX DMA请求
    LL_SPI_EnableDMAReq_TX(SPI1);                                                             // 使能SPI TX DMA请求
}

/**
 * @brief SPI DMA发送函数
 * 
 * @param txbuf_addr 待发送的数据缓冲区地址
 * @param txbuf_size 待发送的数据缓冲区大小
 */
int spi_master_write_then_stop_dma(uint8_t *txbuf_addr, uint16_t txbuf_size) {
    SPI_LOCK();
    spi_dev.mode = SPI_WRITE_THEN_STOP;
    spi_dev.txbuf_addr = txbuf_addr;
    spi_dev.txbuf_size = txbuf_size;
    spi_dev.state = SPI_DMA_TX1;
    spi_dmatx_enable(spi_dev.txbuf_addr, spi_dev.txbuf_size);
    spi_wait_trans_complete();
    int ret = spi_dev.state == SPI_DMA_TC ? SPI_OK : SPI_ERROR;
    SPI_UNLOCK();
    return ret;
}

/**
 * @brief SPI DMA发送并读取函数
 * 
 * @param txbuf_addr 待发送的数据缓冲区地址
 * @param txbuf_size 待发送的数据缓冲区大小
 * @param rxbuf_addr 待接收的数据缓冲区地址
 * @param rxbuf_size 待接收的数据缓冲区大小
 */
int spi_master_write_then_read_dma(uint8_t *txbuf_addr, uint16_t txbuf_size, uint8_t *rxbuf_addr, uint16_t rxbuf_size) {
    SPI_LOCK();
    spi_dev.mode = SPI_WRITE_THEN_READ;
    spi_dev.txbuf_addr = txbuf_addr;
    spi_dev.txbuf_size = txbuf_size;
    spi_dev.rxbuf_addr = rxbuf_addr;
    spi_dev.rxbuf_size = rxbuf_size;
    spi_dev.state = SPI_DMA_TX1;
    spi_dmatx_enable(spi_dev.txbuf_addr, spi_dev.txbuf_size);
    spi_wait_trans_complete();
    int ret = spi_dev.state == SPI_DMA_TC ? SPI_OK : SPI_ERROR;
    SPI_UNLOCK();
    return ret;
}

/**
 * @brief SPI DMA发送函数
 * 
 * @param txbuf1_addr 第一个待发送的数据缓冲区地址
 * @param txbuf1_size 第一个待发送的数据缓冲区大小
 * @param txbuf2_addr 第二个待发送的数据缓冲区地址
 * @param txbuf2_size 第二个待发送的数据缓冲区大小
 */
int spi_master_write_then_write_dma(uint8_t *txbuf1_addr, uint16_t txbuf1_size, uint8_t *txbuf2_addr, uint16_t txbuf2_size) {
    SPI_LOCK();
    spi_dev.mode = SPI_WRITE_THEN_WRITE;
    spi_dev.txbuf_addr = txbuf1_addr;
    spi_dev.txbuf_size = txbuf1_size;
    spi_dev.rxbuf_addr = txbuf2_addr;
    spi_dev.rxbuf_size = txbuf2_size;
    spi_dev.state = SPI_DMA_TX1;
    spi_dmatx_enable(spi_dev.txbuf_addr, spi_dev.txbuf_size);
    spi_wait_trans_complete();
    int ret = spi_dev.state == SPI_DMA_TC ? SPI_OK : SPI_ERROR;
    SPI_UNLOCK();
    return ret;
}
/**
 * @brief DMA接收完成中断处理函数
 * 
 */
static void spi_dmarx_tc_isr(void) {
    LL_DMA_DisableIT_TE(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                     // 禁止DMA RX传输错误中断
    LL_DMA_DisableIT_TC(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                     // 禁止DMA RX传输完成中断
#if (SPI_INTERNAL_NSS == 1)
    SPI_CS_DISABLE();                                                                         // 失能片选信号
#endif
    spi_dev.state = SPI_DMA_TC;                                                               // DMA传输完成
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        taskYIELD();
    }
}

void spi_dmarx_te_isr(void) {
    LL_DMA_DisableIT_TE(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                     // 禁止DMA RX传输错误中断
    LL_DMA_DisableIT_TC(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                     // 禁止DMA RX传输完成中断
#if (SPI_INTERNAL_NSS == 1)
    SPI_CS_DISABLE();                                                                         // 失能片选信号
#endif
    spi_dev.state = SPI_DMA_ERR;                                                              // DMA传输完成
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        taskYIELD();
    }
}

/**
 * @brief DMA发送完成中断处理函数
 * 
 */
static void spi_dmatx_tc_isr(void) {
    LL_DMA_DisableIT_TE(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                     // 禁止DMA TX传输错误中断
    LL_DMA_DisableIT_TC(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                     // 禁止DMA TX传输完成中断
    if (spi_dev.mode == SPI_WRITE_THEN_READ) {                                                // 如果需要继续接收数据，则使能接收DMA
        spi_dev.state = SPI_DMA_RX;
        spi_dmarx_enable(spi_dev.rxbuf_addr, spi_dev.rxbuf_size);
        return;
    } else if (spi_dev.mode == SPI_WRITE_THEN_WRITE && spi_dev.state == SPI_DMA_TX1) {        // 如果需要继续发送数据，则使能发送DMA
        spi_dev.state = SPI_DMA_TX2;
        spi_dmatx_enable(spi_dev.rxbuf_addr, spi_dev.rxbuf_size);
        return;
    }
#if (SPI_INTERNAL_NSS == 1)
    SPI_CS_DISABLE();                                                                         // 失能片选信号
#endif
    spi_dev.state = SPI_DMA_TC;                                                               // DMA传输完成
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        taskYIELD();
    }
}

void spi_dmatx_te_isr(void) {
    LL_DMA_DisableIT_TE(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                     // 禁止DMA TX传输错误中断
    LL_DMA_DisableIT_TC(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                     // 禁止DMA TX传输完成中断
#if (SPI_INTERNAL_NSS == 1)
    SPI_CS_DISABLE();                                                                         // 失能片选信号
#endif
    spi_dev.state = SPI_DMA_ERR;                                                               // DMA传输完成
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        taskYIELD();
    }
}

/**
 * @brief 检查SPI DMA传输是否完成
 * 
 * @return int 1表示传输完成 0表示传输未完成
 */
static void spi_wait_trans_complete(void) {
    xSemaphoreTake(spi_sem, portMAX_DELAY);
}

/**
 * @brief 初始化CS引脚程序
 * 
 */
static void spi_cs_init(void) {
    LL_GPIO_SetOutputPin(SPI1_CS_GPIO, SPI1_CS_PIN);
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = SPI1_CS_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;                                             // GPIO模式：输出模式
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;                                   // GPIO速率：VeryHigh
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                                   // GPIO输出模式：推挽输出
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                                                 // GPIO上下拉：无
    LL_GPIO_Init(SPI1_CS_GPIO, &GPIO_InitStruct);                                           // 初始化CS引脚
}

void DMA2_Stream0_IRQHandler(void) {
    if (LL_DMA_IsEnabledIT_TE(SPI1_RX_DMA, SPI1_RX_DMA_STREAM) && LL_DMA_IsActiveFlag_TE0(SPI1_RX_DMA)) {
        spi_dmarx_te_isr();
        return;
    }
    
    if (LL_DMA_IsEnabledIT_TC(SPI1_RX_DMA, SPI1_RX_DMA_STREAM) && LL_DMA_IsActiveFlag_TC0(SPI1_RX_DMA)) {
        spi_dmarx_tc_isr();
    }
}

void DMA2_Stream5_IRQHandler(void) {
    if (LL_DMA_IsEnabledIT_TE(SPI1_TX_DMA, SPI1_TX_DMA_STREAM) && LL_DMA_IsActiveFlag_TE5(SPI1_TX_DMA)) {
        spi_dmatx_te_isr();
        return;
    }
    
    if (LL_DMA_IsEnabledIT_TC(SPI1_TX_DMA, SPI1_TX_DMA_STREAM) && LL_DMA_IsActiveFlag_TC5(SPI1_TX_DMA)) {
        spi_dmatx_tc_isr();
    }
}