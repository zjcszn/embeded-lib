//
// Created by zjcszn on 2024/2/4.
//

#include "bsp_spi.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_dma.h"
#include "w5500_port.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "perf_counter.h"
#include "elog.h"

static volatile uint8_t spi_state;
static SemaphoreHandle_t spi_dmatc_sem;
static SemaphoreHandle_t spi_lock;
static uint8_t dummy;

#define SPI_LOCK()                                  \
    do {                                            \
        xSemaphoreTake(spi_lock, portMAX_DELAY);    \
    } while (0U)

#define SPI_UNLOCK()                                \
    do {                                            \
        xSemaphoreGive(spi_lock);                   \
    } while (0U)

#define SPI_ENABLE_CLOCK()                          \
    do {                                            \
        LL_AHB1_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);    \
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);     \
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);     \
    } while (0U)

#define SPI_DMA_IT_FLAG_CLEAR()                     \
    do {                                            \
        DMA1->LIFCR = (0xF7DULL << 16);             \
    } while (0U)

#define SPI_FIFO_RESET()                            \
    do {                                            \
        SPI1->CR1 &= ~0x1UL;                        \
        SPI1->CR1 |=  0x1UL;                        \
    } while (0U)

#define SPI_DMA_IT_DISABLE()                        \
    do {                                            \
        LL_DMA_DisableIT_TC(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);   \
        LL_DMA_DisableIT_TE(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);   \
        LL_DMA_DisableIT_TE(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);   \
    } while (0U)

#define SPI_DMA_IT_ENABLE()                         \
    do {                                            \
        LL_DMA_EnableIT_TC(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);   \
        LL_DMA_EnableIT_TE(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);   \
        LL_DMA_EnableIT_TE(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);   \
    } while (0U)

#define SPI_DMA_SEMAPHORE_RELEASE()                 \
    do {                                            \
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;                      \
        xSemaphoreGiveFromISR(spi_dmatc_sem, &xHigherPriorityTaskWoken);    \
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);                       \
    } while (0U)

static void spi_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct;
    
    /* init mosi miso pin */
    GPIO_InitStruct.Pin = W5500_PIN_MOSI | W5500_PIN_MISO;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(W5500_GPIO_PORT, &GPIO_InitStruct);
    
    /* init sclk pin */
    GPIO_InitStruct.Pin = W5500_PIN_SCLK;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN; /* cpol = 0, sclk: pull down */
    LL_GPIO_Init(W5500_GPIO_PORT, &GPIO_InitStruct);
    
    /* init scs pin: soft control, no alternate */
    GPIO_InitStruct.Pin = W5500_PIN_SCS;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;   /* scs: pull up */
    GPIO_InitStruct.Alternate = 0;
    LL_GPIO_Init(W5500_GPIO_PORT, &GPIO_InitStruct);
}

static void spi_nvic_init(void) {
    NVIC_SetPriority(SPI1_RX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xE, 0));
    NVIC_EnableIRQ(SPI1_RX_DMA_IRQN);
    NVIC_SetPriority(SPI1_TX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xE, 0));
    NVIC_EnableIRQ(SPI1_TX_DMA_IRQN);
}

static void spi_periph_init(void) {
    LL_SPI_InitTypeDef SPI_InitStruct;
    
    /* reset spi */
    LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_SPI1);
    LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_SPI1);
    
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;    /* APB2 (120Mhz) / 8 = 15Mhz */
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;     /* W5500: MSB first */
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;           /* cs: software control */
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW; /* mode 0: cpol 0, cpha 0 */
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;     /* W5500 support mode 0 or 3 */
    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;  /* full duplex */
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly = 0;
    LL_SPI_Init(SPI1, &SPI_InitStruct);
    
    LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_SetFIFOThreshold(SPI1, LL_SPI_FIFO_TH_01DATA);
    
    /* Lock GPIO for master to avoid glitches on the clock output */
    LL_SPI_EnableGPIOControl(SPI1);
    LL_SPI_EnableMasterRxAutoSuspend(SPI1);
    
    /* Set number of date to transmit, 0 is unlimited */
    LL_SPI_SetTransferSize(SPI1, 0);
}

static void spi_dmarx_config(void) {
    LL_DMA_DeInit(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);
    LL_DMA_InitTypeDef DMARX_InitStruct = {0};
    DMARX_InitStruct.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    DMARX_InitStruct.Priority = LL_DMA_PRIORITY_LOW;
    DMARX_InitStruct.Mode = LL_DMA_MODE_NORMAL;
    DMARX_InitStruct.PeriphRequest = LL_DMAMUX1_REQ_SPI1_RX;
    DMARX_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&(SPI1->RXDR);
    DMARX_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    DMARX_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    DMARX_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    DMARX_InitStruct.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
    LL_DMA_Init(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, &DMARX_InitStruct);
}

static void spi_dmatx_config(void) {
    LL_DMA_DeInit(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);
    LL_DMA_InitTypeDef DMATX_InitStruct = {0};
    DMATX_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    DMATX_InitStruct.Priority = LL_DMA_PRIORITY_LOW;
    DMATX_InitStruct.Mode = LL_DMA_MODE_NORMAL;
    DMATX_InitStruct.PeriphRequest = LL_DMAMUX1_REQ_SPI1_TX;
    DMATX_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&(SPI1->TXDR);
    DMATX_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    DMATX_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    DMATX_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    DMATX_InitStruct.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
    LL_DMA_Init(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, &DMATX_InitStruct);
}

static void spi_write_by_dma(uint8_t *txbuf_addr, uint16_t txbuf_size) {
    LL_SPI_Disable(SPI1);
    
    spi_state = SPI_STATE_DMA_START;
    LL_DMA_SetMemoryAddress(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, (uint32_t)txbuf_addr);           // 设置内存地址
    LL_DMA_SetDataLength(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, txbuf_size);                        // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetMemoryAddress(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, (uint32_t)&dummy);               // 设置内存地址
    LL_DMA_SetDataLength(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, txbuf_size);                        // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_MEMORY_NOINCREMENT);
    
    SPI_DMA_IT_FLAG_CLEAR();
    SPI_DMA_IT_ENABLE();
    
    LL_SPI_EnableDMAReq_RX(SPI1);                                                             // 使能SPI RX DMA请求
    LL_DMA_EnableStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                     // 使能DMA数据流
    LL_DMA_EnableStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                     // 使能DMA数据流
    LL_SPI_EnableDMAReq_TX(SPI1);                                                             // 使能SPI TX DMA请求
    
    LL_SPI_Enable(SPI1);
    LL_SPI_StartMasterTransfer(SPI1);
}

static void spi_read_by_dma(uint8_t *rxbuf_addr, uint16_t rxbuf_size) {
    LL_SPI_Disable(SPI1);
    
    dummy = 0xFF;
    spi_state = SPI_STATE_DMA_START;
    LL_DMA_SetMemoryAddress(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, (uint32_t)&dummy);               // 设置内存地址
    LL_DMA_SetDataLength(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, rxbuf_size);                        // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_MEMORY_NOINCREMENT);
    LL_DMA_SetMemoryAddress(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, (uint32_t)rxbuf_addr);           // 设置内存地址
    LL_DMA_SetDataLength(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, rxbuf_size);                        // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    
    SPI_DMA_IT_FLAG_CLEAR();
    SPI_DMA_IT_ENABLE();
    
    LL_SPI_EnableDMAReq_RX(SPI1);                                                             // 使能SPI RX DMA请求
    LL_DMA_EnableStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                     // 使能DMA数据流
    LL_DMA_EnableStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                     // 使能DMA数据流
    LL_SPI_EnableDMAReq_TX(SPI1);                                                             // 使能SPI TX DMA请求
    
    LL_SPI_Enable(SPI1);
    LL_SPI_StartMasterTransfer(SPI1);
}

static void spi_write_read_by_dma(uint8_t *txbuf_addr, uint8_t *rxbuf_addr, uint16_t size) {
    LL_SPI_Disable(SPI1);
    
    spi_state = SPI_STATE_DMA_START;
    LL_DMA_SetMemoryAddress(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, (uint32_t)txbuf_addr);           // 设置内存地址
    LL_DMA_SetDataLength(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, size);                              // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_TX_DMA, SPI1_TX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetMemoryAddress(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, (uint32_t)rxbuf_addr);          // 设置内存地址
    LL_DMA_SetDataLength(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, size);                              // 设置DMA传输数据长度
    LL_DMA_SetMemoryIncMode(SPI1_RX_DMA, SPI1_RX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);
    
    SPI_DMA_IT_FLAG_CLEAR();
    SPI_DMA_IT_ENABLE();
    
    LL_SPI_EnableDMAReq_RX(SPI1);                                                             // 使能SPI RX DMA请求
    LL_DMA_EnableStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);                                     // 使能DMA数据流
    LL_DMA_EnableStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);                                     // 使能DMA数据流
    LL_SPI_EnableDMAReq_TX(SPI1);                                                             // 使能SPI TX DMA请求
    
    LL_SPI_Enable(SPI1);
    LL_SPI_StartMasterTransfer(SPI1);
}

int spi_write_read_byte(uint8_t bw, uint8_t *br) {
    SPI_FIFO_RESET();
    LL_SPI_TransmitData8(SPI1, bw);
    LL_SPI_StartMasterTransfer(SPI1);
    while (LL_SPI_IsActiveFlag_RXP(SPI1) == RESET);
    *br = LL_SPI_ReceiveData8(SPI1);
    LL_SPI_SuspendMasterTransfer(SPI1);
    return SPI_OK;
}

static int spi_wait_dma_tc(void) {
    int ret = SPI_OK;
    
    if (xSemaphoreTake(spi_dmatc_sem, pdMS_TO_TICKS(2000)) != pdTRUE) {
        SPI_DMA_IT_DISABLE();
        spi_state = SPI_STATE_TIMEOUT;
    }
    
    LL_SPI_SuspendMasterTransfer(SPI1);                     /* SUSPEND spi transfer */
    LL_SPI_DisableDMAReq_TX(SPI1);                          /* disable SPI_DMA request */
    LL_SPI_DisableDMAReq_RX(SPI1);
    LL_DMA_DisableStream(SPI1_TX_DMA, SPI1_TX_DMA_STREAM);  /* disable dma stream */
    LL_DMA_DisableStream(SPI1_RX_DMA, SPI1_RX_DMA_STREAM);
    
    if (spi_state != SPI_STATE_DMA_TC) {
        log_e("spi dma read or write error");
        ret = SPI_ERROR;
    }
    
    spi_state = SPI_STATE_IDLE;
    return ret;
}

int spi_master_init(void) {
    SPI_ENABLE_CLOCK();         // 使能时钟
    
    spi_gpio_init();            // 初始化 CLK | MOSI | MISO 引脚
    spi_dmarx_config();         // 初始化 RX DMA
    spi_dmatx_config();         // 初始化 TX DMA
    spi_nvic_init();            // 初始化 NVIC
    spi_periph_init();          // 初始化 SPI
    
    if (spi_dmatc_sem == NULL) {
        spi_dmatc_sem = xSemaphoreCreateBinary();
    }
    if (spi_lock == NULL) {
        spi_lock = xSemaphoreCreateMutex();
    }
    
    return (spi_dmatc_sem && spi_lock) ? SPI_OK : SPI_ERROR;
}

int spi_master_write(uint8_t *txbuf_addr, uint32_t len) {
    int ret;
    if (len == 0) return SPI_ERROR;
    
    SPI_LOCK();
    if (len == 1) {
        uint8_t tmp;
        ret = spi_write_read_byte(*txbuf_addr, &tmp);
        (void)tmp;
    } else {
        uint16_t btw;
        uint32_t cnt = len;
        uint32_t off = 0;
        
        do {
            btw = (cnt > 0xFFFFUL) ? (uint16_t)0xFFFFU : (uint16_t)cnt;
            spi_write_by_dma(txbuf_addr + off, btw);
            if ((ret = spi_wait_dma_tc()) != SPI_OK) {
                break;
            }
            off += btw;
            cnt -= btw;
        } while (cnt > 0);
    }
    SPI_UNLOCK();
    return ret;
}

int spi_master_read(uint8_t *rxbuf_addr, uint32_t len) {
    int ret;
    if (len == 0) return SPI_ERROR;
    
    SPI_LOCK();
    if (len == 1) {
        ret = spi_write_read_byte(0xFF, rxbuf_addr);
    } else {
        uint16_t btr;
        uint32_t cnt = len;
        uint32_t off = 0;
        
        do {
            btr = cnt > 0xFFFFUL ? (uint16_t)0xFFFFU : (uint16_t)cnt;
            spi_read_by_dma(rxbuf_addr + off, btr);
            if ((ret = spi_wait_dma_tc()) != SPI_OK) {
                break;
            }
            off += btr;
            cnt -= btr;
        } while (cnt > 0);
        
        SCB_InvalidateDCache_by_Addr(rxbuf_addr, len);
    }
    SPI_UNLOCK();
    return ret;
}

int spi_master_write_read(uint8_t *txbuf_addr, uint8_t *rxbuf_addr, uint32_t len) {
    int ret;
    if (len == 0) return SPI_ERROR;
    
    SPI_LOCK();
    if (len == 1) {
        ret = spi_write_read_byte(*txbuf_addr, rxbuf_addr);
    } else {
        uint16_t btr;
        uint32_t cnt = len;
        uint32_t off = 0;
        
        do {
            btr = cnt > 0xFFFFUL ? (uint16_t)0xFFFFU : (uint16_t)cnt;
            spi_write_read_by_dma(txbuf_addr + off, rxbuf_addr + off, btr);
            if ((ret = spi_wait_dma_tc()) != SPI_OK) {
                break;
            }
            off += btr;
            cnt -= btr;
        } while (cnt > 0);
        
        SCB_InvalidateDCache_by_Addr(rxbuf_addr, len);
    }
    SPI_UNLOCK();
    return ret;
}

void SPI1_RX_DMA_IRQHandler(void) {
    if (LL_DMA_IsEnabledIT_TE(SPI1_RX_DMA, SPI1_RX_DMA_STREAM) && LL_DMA_IsActiveFlag_TE3(SPI1_RX_DMA)) {
        spi_state = SPI_STATE_DMA_ERR;
        goto isr_exit;
    }
    if (LL_DMA_IsEnabledIT_TC(SPI1_RX_DMA, SPI1_RX_DMA_STREAM) && LL_DMA_IsActiveFlag_TC3(SPI1_RX_DMA)) {
        spi_state = SPI_STATE_DMA_TC;
        goto isr_exit;
    }

isr_exit:
    SPI_DMA_IT_DISABLE();
    SPI_DMA_SEMAPHORE_RELEASE();
}

void SPI1_TX_DMA_IRQHandler(void) {
    if (LL_DMA_IsEnabledIT_TE(SPI1_TX_DMA, SPI1_TX_DMA_STREAM) && LL_DMA_IsActiveFlag_TE2(SPI1_TX_DMA)) {
        spi_state = SPI_STATE_DMA_ERR;
        SPI_DMA_IT_DISABLE();
        SPI_DMA_SEMAPHORE_RELEASE();
    }
}

#ifdef SPI_TEST

#include <stdlib.h>
#include <string.h>

#define TEST_BUFFER_SIZE    1024

ALIGN_32BYTES(static uint8_t spi_tx[TEST_BUFFER_SIZE]);
ALIGN_32BYTES(static uint8_t spi_rx[TEST_BUFFER_SIZE]);

bool spi_master_compare(uint8_t *src, uint8_t *dst, uint32_t len) {
    for (int i = 0; i < len; i++) {
        if (src[i] != dst[i]) {
            log_e("tx[%u]: %02X, rx[%u]: %02X", &src[i] - spi_tx, src[i], &dst[i] - spi_rx, dst[i]);
            return false;
        }
    }
    return true;
}

void spi_master_test(void) {
    /* init tx data */
    srand(get_system_ticks());
    for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
        spi_tx[i] = rand() & 0xFF;
    }
    
    log_i("========== SPI TEST START ==========");
    log_i("test 1 : align 32 bytes, polling mode");
    for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
        spi_master_write_read(&spi_tx[i], &spi_rx[i], 1);
    }
    log_i("result: [%s]", spi_master_compare(spi_tx, spi_rx, TEST_BUFFER_SIZE) ? "PASS" : "FAILED");
    memset(&spi_rx[0], 0, TEST_BUFFER_SIZE);
    SCB_CleanDCache_by_Addr((uint32_t *)&spi_rx[0], TEST_BUFFER_SIZE);
    
    log_i("test 2 : align 32 bytes, dma mode");
    spi_master_write_read(&spi_tx[0], &spi_rx[0], TEST_BUFFER_SIZE);
    log_i("result: [%s]", spi_master_compare(&spi_tx[0], &spi_rx[0], TEST_BUFFER_SIZE) ? "PASS" : "FAILED");
    memset(&spi_rx[0], 0, TEST_BUFFER_SIZE);
    SCB_CleanDCache_by_Addr((uint32_t *)&spi_rx[0], TEST_BUFFER_SIZE);
    
    for (int i = 1; i <= 8; i++) {
        log_i("test %-2d: align  %d bytes, dma mode", i + 2, i);
        spi_master_write_read(&spi_tx[i], &spi_rx[i], TEST_BUFFER_SIZE - i);
        log_i("result: [%s]", spi_master_compare(&spi_tx[i], &spi_rx[i], TEST_BUFFER_SIZE - i) ? "PASS" : "FAILED");
        memset(&spi_rx[0], 0, TEST_BUFFER_SIZE);
        SCB_CleanDCache_by_Addr((uint32_t *)&spi_rx[0], TEST_BUFFER_SIZE);
    }
    log_i("=========== SPI TEST END ===========");
    
}

#endif



