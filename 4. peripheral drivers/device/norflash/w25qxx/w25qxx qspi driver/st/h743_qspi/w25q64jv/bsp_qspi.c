//
// Created by zjcszn on 2024/2/6.
//

#define LOG_TAG "FLASH"
#define LOG_LVL ELOG_LVL_DEBUG

#include <string.h>
#include "bsp_qspi.h"

#include "elog.h"

#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_hal_mdma.h"
#include "main.h"

qspi_handle_t dev_qspi;

static void prv_qspi_clock_init(void) {
    /* set QSPI Clock sources -> HCLK */
    LL_RCC_SetQSPIClockSource(LL_RCC_QSPI_CLKSOURCE_HCLK);
    
    /* enable QSPI clock */
    __HAL_RCC_QSPI_CLK_ENABLE();
    
    /* reset QSPI interface */
    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();
}

static void prv_qspi_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO Clock */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    
    /* Init Pin of QSPI */
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    
    /* QSPI_BK1 NCS -> PB6 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_10;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* QSPI_CLK -> PB2 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_9;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* QSPI_BK1 IO0\IO1\IO3 -> PD11 PD12 PD13 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_9;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    /* QSPI_BK1 IO2 -> PE2 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_9;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

static void prv_qspi_mdma_init(void) {
    /* enable mdma clock */
    __HAL_RCC_MDMA_CLK_ENABLE();
    
    dev_qspi.hmdma.Instance = MDMA_Channel1;                         /* 使用MDMA的通道1 */
    dev_qspi.hmdma.Init.Request = MDMA_REQUEST_QUADSPI_FIFO_TH;      /* QSPI的FIFO阀值触发中断 */
    dev_qspi.hmdma.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;  /* 使用MDMA的buffer传输 */
    dev_qspi.hmdma.Init.Priority = MDMA_PRIORITY_HIGH;               /* 优先级高 */
    dev_qspi.hmdma.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;/* 小端格式 */
    dev_qspi.hmdma.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;   /* 小端，右对齐 */
    dev_qspi.hmdma.Init.SourceInc = MDMA_SRC_INC_BYTE;               /* 源地址字节递增 */
    dev_qspi.hmdma.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;     /* 源地址数据宽度:字节 */
    dev_qspi.hmdma.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;      /* 源数据单次传输 */
    dev_qspi.hmdma.Init.DestinationInc = MDMA_DEST_INC_DISABLE;      /* 目的地址无自增 */
    dev_qspi.hmdma.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;      /* 目的地址数据宽度:字节 */
    dev_qspi.hmdma.Init.DestBurst = MDMA_DEST_BURST_SINGLE;          /* 目的数据单次传输 */
    dev_qspi.hmdma.Init.BufferTransferLength = 128;                  /* 每次传输128个字节 */
    
    dev_qspi.hmdma.Init.SourceBlockAddressOffset = 0;                /* 用于block传输，buffer传输用不到 */
    dev_qspi.hmdma.Init.DestBlockAddressOffset = 0;                  /* 用于block传输，buffer传输用不到 */
    
    /* 关联MDMA句柄到QSPI句柄里面  */
    __HAL_LINKDMA(&dev_qspi.hqspi, hmdma, dev_qspi.hmdma);
    
    /* 先复位，然后配置MDMA */
    HAL_MDMA_DeInit(&dev_qspi.hmdma);
    HAL_MDMA_Init(&dev_qspi.hmdma);
    
    /* 使能MDMA中断，并配置优先级 */
    NVIC_SetPriority(MDMA_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0X5, 0));
    NVIC_EnableIRQ(MDMA_IRQn);
}

static void prv_qspi_sem_init(void) {
    if (dev_qspi.qspi_tc == NULL) {
        dev_qspi.qspi_tc = xSemaphoreCreateBinary();
    }
    if (dev_qspi.status_match_sem == NULL) {
        dev_qspi.status_match_sem = xSemaphoreCreateBinary();
    }
    assert(dev_qspi.qspi_tc && dev_qspi.status_match_sem);
}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi) {
    if (hqspi->Instance == QUADSPI) {
        prv_qspi_gpio_init();
        prv_qspi_mdma_init();
        prv_qspi_clock_init();
    }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi) {
    if (hqspi->Instance == QUADSPI) {
        __HAL_RCC_QSPI_CLK_DISABLE();
        
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2 | GPIO_PIN_6);
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
        HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2);
        
        if (hqspi->hmdma != NULL) {
            HAL_MDMA_DeInit(hqspi->hmdma);
        }
    }
    
}

void bsp_qspi_init(void) {
    prv_qspi_sem_init();
    
    /* reset QSPI */
    memset(&dev_qspi.hqspi, 0x00, sizeof(QSPI_HandleTypeDef));
    dev_qspi.hqspi.Instance = QUADSPI;
    if (HAL_QSPI_DeInit(&dev_qspi.hqspi) != HAL_OK) {
        Error_Handler(__FILE__, __LINE__);
    }
    
    dev_qspi.hqspi.Init.ClockPrescaler = 1; /* 设置分频系数，240MHz (HCLK3) / (ClockPrescaler+1) = 120MHz */
    dev_qspi.hqspi.Init.FifoThreshold = 32; /* 设置FIFO阈值，范围 1 ~ 32 */
    dev_qspi.hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE; /* 延迟半个时钟周期采样，DDR模式下必须禁止 */
    dev_qspi.hqspi.Init.FlashSize = 22; /* FlashCapacity = 2 ^ (FlashSize + 1), 8MB = 2 ^ (22+1) */
    dev_qspi.hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;  /* 命令之间的 CS 片选至少保持 1 个时钟周期的高电平 */
    dev_qspi.hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;  /* MODE0: 空闲期间时钟保持低电平; MODE3: 空闲期间时钟保持高电平 */
    dev_qspi.hqspi.Init.FlashID = QSPI_FLASH_ID_1;      /* 设置QSPI BANK*/
    dev_qspi.hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;     /* 禁用双BANK */
    
    /* 初始化配置 QSPI */
    if (HAL_QSPI_Init(&dev_qspi.hqspi) != HAL_OK) {
        Error_Handler(__FILE__, __LINE__);
    }
    
    /* 使能QSPI中断 */
    NVIC_SetPriority(QUADSPI_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0XF, 0));
    NVIC_EnableIRQ(QUADSPI_IRQn);
}

int bsp_qspi_wait_tc(void) {
    if (xSemaphoreTake(dev_qspi.qspi_tc, pdMS_TO_TICKS(HAL_QSPI_TIMEOUT_DEFAULT_VALUE))
        != pdTRUE) {
        return 1;
    }
    return 0;
}

int bsp_qspi_wait_busy(void) {
    if (xSemaphoreTake(dev_qspi.status_match_sem, portMAX_DELAY) != pdTRUE) {
        log_e("w25qxx take status match sem failed");
        return 1;
    }
    return 0;
}

void MDMA_IRQHandler(void) {
    HAL_MDMA_IRQHandler(dev_qspi.hqspi.hmdma);
}

void QUADSPI_IRQHandler(void) {
    HAL_QSPI_IRQHandler(&dev_qspi.hqspi);
}

void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(dev_qspi.status_match_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        taskYIELD();
    }
}

void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(dev_qspi.qspi_tc, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        taskYIELD();
    }
}

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(dev_qspi.qspi_tc, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        taskYIELD();
    }
}
