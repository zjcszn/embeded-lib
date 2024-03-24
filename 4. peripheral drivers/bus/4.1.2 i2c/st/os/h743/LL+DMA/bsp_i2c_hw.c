//
// Created by zjcszn on 2024/3/10.
//
#define LOG_TAG "I2C"
#define LOG_LVL ELOG_LVL_INFO

#include "bsp_i2c_hw.h"

#include "FreeRTOS.h"
#include "task.h"
#include "perf_counter.h"

#include "stm32h7xx.h"
#include "stm32h7xx_ll_i2c.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_dma.h"
#include "elog.h"

/*********************************************** 宏定义 **********************************************/

#define I2C_ENABLE_CLOCK()        \
    do {                        \
        LL_RCC_SetI2CClockSource(LL_RCC_I2C123_CLKSOURCE_PLL3R);  \
        LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);      \
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);       \
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);       \
    } while(0)

#define I2C_SCL_L()     \
    do {                \
        LL_GPIO_ResetOutputPin(I2C1_PORT, I2C1_SCL_PIN);    \
        delay_us(5);    \
    } while(0)

#define I2C_SCL_H()     \
    do {                \
        LL_GPIO_SetOutputPin(I2C1_PORT, I2C1_SCL_PIN);      \
        delay_us(5);    \
    } while(0)
/********************************************** 函数声明 *********************************************/



/********************************************** 函数定义 *********************************************/

static void i2c_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = I2C1_SCL_PIN | I2C1_SDA_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Alternate = I2C1_GPIO_AF;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FAST;
    LL_GPIO_Init(I2C1_PORT, &GPIO_InitStruct);
}

static void i2c_nvic_init(void) {
    /* enable i2c interrupt */
    NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xF, 0));
    NVIC_EnableIRQ(I2C1_EV_IRQn);
    NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xF, 0));
    NVIC_EnableIRQ(I2C1_ER_IRQn);
    
    /* enable dma interrupt */
    NVIC_SetPriority(I2C1_TX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xF, 0));
    NVIC_EnableIRQ(I2C1_TX_DMA_IRQN);
    NVIC_SetPriority(I2C1_RX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xF, 0));
    NVIC_EnableIRQ(I2C1_RX_DMA_IRQN);
}

static void i2c_periph_init(void) {
    LL_I2C_InitTypeDef I2C_InitStruct = {0};
    
    LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);  /* 复位I2C */
    LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);
    
    LL_I2C_EnableClockStretching(I2C1);    /* 主模式下必须使能 */
    LL_I2C_EnableAutoEndMode(I2C1); /* 自动结束模式：NBYTES数据传输完成时，自动发送停止位，RELOAD位置1时无效 */
    LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);  /* 设置设备地址2 */
    LL_I2C_DisableOwnAddress2(I2C1);    /* 禁用设备地址2，不对该地址应答（从模式） */
    LL_I2C_DisableGeneralCall(I2C1);    /* 禁止广播呼叫， 不对地址0b0000000应答 */
    
    I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
    I2C_InitStruct.Timing = 0x00200E34;
    I2C_InitStruct.OwnAddress1 = I2C1_OWN_ADDR;
    I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;   /* 使能模拟滤波器 */
    I2C_InitStruct.DigitalFilter = 0;
    I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
    LL_I2C_Init(I2C1, &I2C_InitStruct);
    
    LL_I2C_Enable(I2C1);
}

static void i2c_dma_config(void) {
    LL_DMA_InitTypeDef DMA_InitStruct = {0};
    
    LL_DMA_DeInit(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);
    DMA_InitStruct.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    DMA_InitStruct.Priority = LL_DMA_PRIORITY_LOW;
    DMA_InitStruct.Mode = LL_DMA_MODE_NORMAL;
    DMA_InitStruct.PeriphRequest = LL_DMAMUX1_REQ_I2C1_RX;
    DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&(I2C1->RXDR);
    DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_MEMORY_INCREMENT;
    DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    DMA_InitStruct.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
    LL_DMA_Init(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, &DMA_InitStruct);
    
    LL_DMA_DeInit(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);
    DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    DMA_InitStruct.Priority = LL_DMA_PRIORITY_LOW;
    DMA_InitStruct.Mode = LL_DMA_MODE_NORMAL;
    DMA_InitStruct.PeriphRequest = LL_DMAMUX1_REQ_I2C1_TX;
    DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&(I2C1->TXDR);
    DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    DMA_InitStruct.FIFOMode = LL_DMA_FIFOMODE_DISABLE;
    LL_DMA_Init(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, &DMA_InitStruct);
}

static void i2c_low_level_init(void) {
    i2c_gpio_init();
    i2c_dma_config();
    i2c_nvic_init();
    i2c_periph_init();
}

static void i2c_reset(void) {
    LL_I2C_Disable(I2C1);
    LL_GPIO_SetOutputPin(I2C1_PORT, I2C1_SCL_PIN);
    LL_GPIO_SetOutputPin(I2C1_PORT, I2C1_SDA_PIN);
    LL_GPIO_SetPinMode(I2C1_PORT, I2C1_SCL_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(I2C1_PORT, I2C1_SDA_PIN, LL_GPIO_MODE_OUTPUT);
    
    for (int i = 0; i < 10; i++) {
        I2C_SCL_L();
        I2C_SCL_H();
    }
    i2c_low_level_init();
}

void bsp_i2c_init(void) {
    I2C_ENABLE_CLOCK();
    i2c_low_level_init();
}

int bsp_i2c_bus_check(void) {
    bool timeout = false;
    TickType_t now = xTaskGetTickCount();
    
    while (I2C1->ISR & I2C1_ISR_MASK_BSY_OR_ERR || ((I2C1_PORT->IDR & I2C1_ALL_PIN) != I2C1_ALL_PIN)) {
        if (timeout == true) {
            return -1;
        }
        if (xTaskGetTickCount() - now > pdMS_TO_TICKS(I2C1_TIMEOUT)) {
            i2c_reset();
            timeout = true;
            log_e("i2c bus error, reset now.");
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return 0;
}

void bsp_i2c_dmarx_config(uint8_t *rxbuf, uint16_t len) {
    LL_DMA_DisableStream(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);
    LL_I2C_DisableDMAReq_RX(I2C1);
    
    LL_DMA_SetMemoryAddress(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, (uint32_t)rxbuf);
    LL_DMA_SetDataLength(I2C1_RX_DMA, I2C1_RX_DMA_STREAM, len);
    
    I2C1_RX_DMA->HIFCR = 0x00000F40U;
    
    LL_DMA_EnableIT_TE(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);
    LL_DMA_EnableIT_TC(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);
    
    LL_DMA_EnableStream(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);
    LL_I2C_EnableDMAReq_RX(I2C1);
}

void bsp_i2c_dmatx_config(uint8_t *txbuf, uint16_t len) {
    LL_DMA_DisableStream(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);
    LL_I2C_DisableDMAReq_TX(I2C1);
    
    LL_DMA_SetMemoryAddress(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, (uint32_t)txbuf);
    LL_DMA_SetDataLength(I2C1_TX_DMA, I2C1_TX_DMA_STREAM, len);
    
    I2C1_TX_DMA->HIFCR = 0x0000003FU;
    
    LL_DMA_EnableIT_TE(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);
    
    LL_DMA_EnableStream(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);
    LL_I2C_EnableDMAReq_TX(I2C1);
}