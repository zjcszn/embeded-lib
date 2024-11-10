/*
*********************************************************************************************************
*
*    模块名称 : W25Q256 QSPI驱动模块
*    文件名称 : bsp_qspi_w25q256.c
*    版    本 : V1.0
*    说    明 : 使用CPU的QSPI总线驱动串行FLASH，提供基本的读写函数，采用4线方式
*
*    修改记录 :
*        版本号  日期        作者     说明
*        V1.0    2019-02-12  armfly  正式发布
*        V1.1    2019-07-07  armfly  增加H7-TOOL引脚定义。
*
*    Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_pwr.h"
#include "stm32h7xx_hal_qspi.h"
#include "bsp_qspi_w25q256.h"
#include "w25qxx.h"

/* 
    STM32-V7开发板接线

    PG6/QUADSPI_BK1_NCS     AF10
    PF10/QUADSPI_CLK        AF9
    PF8/QUADSPI_BK1_IO0     AF10
    PF9/QUADSPI_BK1_IO1     AF10
    PF7/QUADSPI_BK1_IO2     AF9
    PF6/QUADSPI_BK1_IO3     AF9

    W25Q256JV有512块，每块有16个扇区，每个扇区Sector有16页，每页有256字节，共计32MB
        
    H7-TOOL开发板接线

    PG6/QUADSPI_BK1_NCS     AF10
    PB2/QUADSPI_CLK         AF9
    PD11/QUADSPI_BK1_IO0    AF10
    PD12/QUADSPI_BK1_IO1    AF10
    PF7/QUADSPI_BK1_IO2     AF9
    PD13/QUADSPI_BK1_IO3    AF9
*/

/* QSPI引脚和时钟相关配置宏定义 */
#if 0
#define QSPI_CLK_ENABLE()               __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE()              __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOG_CLK_ENABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_BK1_D0_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_BK1_D1_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_BK1_D2_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D3_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()

#define QSPI_MDMA_CLK_ENABLE()          __HAL_RCC_MDMA_CLK_ENABLE()
#define QSPI_FORCE_RESET()              __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()            __HAL_RCC_QSPI_RELEASE_RESET()

#define QSPI_CS_PIN                     GPIO_PIN_6
#define QSPI_CS_GPIO_PORT               GPIOG
#define QSPI_CS_GPIO_AF                 GPIO_AF10_QUADSPI

#define QSPI_CLK_PIN                    GPIO_PIN_2
#define QSPI_CLK_GPIO_PORT              GPIOB
#define QSPI_CLK_GPIO_AF                GPIO_AF9_QUADSPI

#define QSPI_BK1_D0_PIN                 GPIO_PIN_11
#define QSPI_BK1_D0_GPIO_PORT           GPIOD
#define QSPI_BK1_D0_GPIO_AF             GPIO_AF9_QUADSPI

#define QSPI_BK1_D1_PIN                 GPIO_PIN_12
#define QSPI_BK1_D1_GPIO_PORT           GPIOD
#define QSPI_BK1_D1_GPIO_AF             GPIO_AF9_QUADSPI

#define QSPI_BK1_D2_PIN                 GPIO_PIN_7
#define QSPI_BK1_D2_GPIO_PORT           GPIOF
#define QSPI_BK1_D2_GPIO_AF             GPIO_AF9_QUADSPI

#define QSPI_BK1_D3_PIN                 GPIO_PIN_13
#define QSPI_BK1_D3_GPIO_PORT           GPIOD
#define QSPI_BK1_D3_GPIO_AF             GPIO_AF9_QUADSPI
#else
#define QSPI_CLK_ENABLE()               __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE()              __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_BK1_D0_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D1_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D2_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D3_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOF_CLK_ENABLE()

#define QSPI_MDMA_CLK_ENABLE()          __HAL_RCC_MDMA_CLK_ENABLE()
#define QSPI_FORCE_RESET()              __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()            __HAL_RCC_QSPI_RELEASE_RESET()

#define QSPI_CS_PIN                     GPIO_PIN_6
#define QSPI_CS_GPIO_PORT               GPIOB
#define QSPI_CS_GPIO_AF                 GPIO_AF10_QUADSPI

#define QSPI_CLK_PIN                    GPIO_PIN_2
#define QSPI_CLK_GPIO_PORT              GPIOB
#define QSPI_CLK_GPIO_AF                GPIO_AF9_QUADSPI

#define QSPI_BK1_D0_PIN                 GPIO_PIN_8
#define QSPI_BK1_D0_GPIO_PORT           GPIOF
#define QSPI_BK1_D0_GPIO_AF             GPIO_AF10_QUADSPI

#define QSPI_BK1_D1_PIN                 GPIO_PIN_9
#define QSPI_BK1_D1_GPIO_PORT           GPIOF
#define QSPI_BK1_D1_GPIO_AF             GPIO_AF10_QUADSPI

#define QSPI_BK1_D2_PIN                 GPIO_PIN_7
#define QSPI_BK1_D2_GPIO_PORT           GPIOF
#define QSPI_BK1_D2_GPIO_AF             GPIO_AF9_QUADSPI

#define QSPI_BK1_D3_PIN                 GPIO_PIN_6
#define QSPI_BK1_D3_GPIO_PORT           GPIOF
#define QSPI_BK1_D3_GPIO_AF             GPIO_AF9_QUADSPI
#endif


/* 供本文件调用的全局变量 */
QSPI_HandleTypeDef QSPIHandle;




/*
*********************************************************************************************************
*    函 数 名: bsp_InitQSPI_W25Q256
*    功能说明: QSPI Flash硬件初始化，配置基本参数
*    形    参: 无
*    返 回 值: 0 表示成功， 1 表示失败
*********************************************************************************************************
*/
int bsp_InitQSPI_W25Q256(void)
{
    uint32_t i;
    char *p;
    
    /* 将句柄手动清零，防止作为全局变量的时候没有清零 */
    p = (char *)&QSPIHandle;
    for (i = 0; i < sizeof(QSPI_HandleTypeDef); i++)
    {
        *p++ = 0;
    }
  
    /* 复位QSPI */
    QSPIHandle.Instance = QUADSPI;   
    
    if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK)
    {
        return 1;
    }

    /* 设置时钟速度，QSPI clock = 200MHz / (ClockPrescaler+1) = 100MHz */
    QSPIHandle.Init.ClockPrescaler = 1;

    /* 设置FIFO阀值，范围1 - 32 */
    QSPIHandle.Init.FifoThreshold = 32;

    /* 
        QUADSPI在FLASH驱动信号后过半个CLK周期才对FLASH驱动的数据采样。
        在外部信号延迟时，这有利于推迟数据采样。
    */
    QSPIHandle.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;

    /* Flash大小是2^(FlashSize + 1) = 2^25 = 32MB */
    QSPIHandle.Init.FlashSize = QSPI_FLASH_SIZE - 1; //QSPI_FLASH_SIZE - 1; 2020-03-04, 需要扩大一倍，否则内存映射方位最后1个地址时，会异常

    /* 命令之间的CS片选至少保持1个时钟周期的高电平 */
    QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;

    /*
       MODE0: 表示片选信号空闲期间，CLK时钟信号是低电平
       MODE3: 表示片选信号空闲期间，CLK时钟信号是高电平
    */
    QSPIHandle.Init.ClockMode = QSPI_CLOCK_MODE_3;

    /* QSPI有两个BANK，这里使用的BANK1 */
    QSPIHandle.Init.FlashID = QSPI_FLASH_ID_1;

    /* V7开发板仅使用了BANK1，这里是禁止双BANK */
    QSPIHandle.Init.DualFlash = QSPI_DUALFLASH_DISABLE;

    /* 初始化配置QSPI */
    if (HAL_QSPI_Init(&QSPIHandle) != HAL_OK)
    {
        return 1;
    }
		
    if (w25qxx_init() != W25X_OK)
    {
            return 1;
    }
    
    return 0;
}




/*
*********************************************************************************************************
*    函 数 名: HAL_QSPI_MspInit
*    功能说明: QSPI底层初始化，被HAL_QSPI_Init调用的底层函数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* 使能QPSI时钟  */
    QSPI_CLK_ENABLE();

    /* 复位时钟接口 */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();    
    
    /* 使能GPIO时钟 */
    QSPI_CS_GPIO_CLK_ENABLE();
    QSPI_CLK_GPIO_CLK_ENABLE();
    QSPI_BK1_D0_GPIO_CLK_ENABLE();
    QSPI_BK1_D1_GPIO_CLK_ENABLE();
    QSPI_BK1_D2_GPIO_CLK_ENABLE();
    QSPI_BK1_D3_GPIO_CLK_ENABLE();

    /* QSPI CS GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = QSPI_CS_GPIO_AF;
    HAL_GPIO_Init(QSPI_CS_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI CLK GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_CLK_PIN;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = QSPI_CLK_GPIO_AF;
    HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI D0 GPIO pin 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_BK1_D0_PIN;
    GPIO_InitStruct.Alternate = QSPI_BK1_D0_GPIO_AF;
    HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI D1 GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_BK1_D1_PIN;
    GPIO_InitStruct.Alternate = QSPI_BK1_D1_GPIO_AF;
    HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI D2 GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_BK1_D2_PIN;
    GPIO_InitStruct.Alternate = QSPI_BK1_D2_GPIO_AF;
    HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &GPIO_InitStruct);

    /* QSPI D3 GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_BK1_D3_PIN;
    GPIO_InitStruct.Alternate = QSPI_BK1_D3_GPIO_AF;
    HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &GPIO_InitStruct);
}

/*
*********************************************************************************************************
*    函 数 名: HAL_QSPI_MspDeInit
*    功能说明: QSPI底层复位，被HAL_QSPI_Init调用的底层函数
*    形    参: hqspi QSPI_HandleTypeDef类型句柄
*    返 回 值: 无
*********************************************************************************************************
*/
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi)
{
    /* 复位QSPI引脚 */
    HAL_GPIO_DeInit(QSPI_CS_GPIO_PORT, QSPI_CS_PIN);
    HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D0_GPIO_PORT, QSPI_BK1_D0_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D1_GPIO_PORT, QSPI_BK1_D1_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D2_GPIO_PORT, QSPI_BK1_D2_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D3_GPIO_PORT, QSPI_BK1_D3_PIN);

    /* 复位QSPI */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();

    /* 关闭QSPI时钟 */
    QSPI_CLK_DISABLE();
}

/*
*********************************************************************************************************
*    函 数 名: sf_EraseSector
*    功能说明: 擦除指定的扇区，扇区大小64KB
*    形    参: _uiSectorAddr : 扇区地址，以64KB为单位的地址
*    返 回 值: 无
*********************************************************************************************************
*/
int QSPI_EraseSector(uint32_t address)
{
    if (w25qxx_block64k_erase(address) != W25X_OK)
		{
				return 1;
		}

    return 0;
}

/*
*********************************************************************************************************
*    函 数 名: QSPI_EraseChip
*    功能说明: 擦除整片
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
int QSPI_EraseChip(void)
{
    if (w25qxx_chip_erase() != W25X_OK)
		{
				return 1;
		}

    return 0;
}

/*
*********************************************************************************************************
*    函 数 名: QSPI_WriteBuffer
*    功能说明: 页编程，页大小256字节，任意页都可以写入
*    形    参: _pBuf : 数据源缓冲区；
*              _uiWrAddr ：目标区域首地址，即页首地址，比如0， 256, 512等。
*              _usSize ：数据个数，不能超过页面大小，范围1 - 256。
*    返 回 值: 0:成功， 1：失败
*********************************************************************************************************
*/
int QSPI_WriteBuffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
    if (w25qxx_page_program(_pBuf, _uiWriteAddr, _usWriteSize) != W25X_OK)
		{
				return 1;
		}

    return 0;
}

/*
*********************************************************************************************************
*    函 数 名: QSPI_ReadBuffer
*    功能说明: 连续读取若干字节，字节个数不能超出芯片容量。
*    形    参: _pBuf : 数据源缓冲区。
*              _uiReadAddr ：起始地址。
*              _usSize ：数据个数, 可以大于PAGE_SIZE, 但是不能超出芯片总容量。
*    返 回 值: 无
*********************************************************************************************************
*/
int QSPI_ReadBuffer(uint8_t *_pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
    if (w25qxx_read(_pBuf, _uiReadAddr, _uiSize) != W25X_OK)
		{
				return 1;
		}
    
    return 0;
}


/*
*********************************************************************************************************
*    函 数 名: QSPI_MemoryMapped
*    功能说明: QSPI内存映射，地址 0x90000000
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
int QSPI_MemoryMapped(void)
{
    if (w25qxx_memory_mapped_enable() != W25X_OK)
		{
				return 1;
		}

    return 0;
}

/*
*********************************************************************************************************
*    函 数 名: QSPI_QuitMemoryMapped
*    功能说明: QSPI内存映射，地址 0x90000000
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
int QSPI_QuitMemoryMapped(void)
{
    HAL_QSPI_Abort(&QSPIHandle);
 
    return 0;
}
          
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
