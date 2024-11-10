/*
*********************************************************************************************************
*
*	模块名称 : FlashPrg
*	文件名称 : FlashPrg.c
*	版    本 : V1.0
*	说    明 : Flash编程接口
*
*	修改记录 :
*		版本号  日期         作者       说明
*		V1.0    2020-11-06  Eric2013   正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "FlashOS.h"
#include "bsp.h"
#include "w25qxx.h"



/*
*********************************************************************************************************
*	函 数 名: Init
*	功能说明: Flash编程初始化
*	形    参: adr Flash基地址，芯片首地址。
*             clk 时钟频率
*             fnc 函数代码，1 - Erase, 2 - Program, 3 - Verify
*	返 回 值: 0 表示成功， 1表示失败
*********************************************************************************************************
*/
int Init (unsigned long adr, unsigned long clk, unsigned long fnc) 
{
    int result = 0;
 
    /* 系统初始化 */
    SystemInit(); 

    /* 时钟初始化 */
    result = SystemClock_Config();
    if (result  != 0)
    {
        return 1;        
    }

    /* W25Q256初始化 */
    result = bsp_InitQSPI_W25Q256();
    if (result != 0)
    {
        return 1;
    }
  

    /* 内存映射 */    
    result = QSPI_MemoryMapped(); 
    if (result != 0)
    {
        return 1;
    }

    return 0;
}

/*
*********************************************************************************************************
*	函 数 名: UnInit
*	功能说明: 复位初始化
*	形    参: fnc 函数代码，1 - Erase, 2 - Program, 3 - Verify
*	返 回 值: 0 表示成功， 1表示失败
*********************************************************************************************************
*/
int UnInit (unsigned long fnc) 
{ 
    int result = 0;

    /* W25Q256初始化 */
    result = bsp_InitQSPI_W25Q256();
    if (result != 0)
    {
        return 1;
    }
		

    /* 内存映射 */    
    result = QSPI_MemoryMapped(); 
    if (result != 0)
    {
        return 1;
    }
    
    return (0);
}

/*
*********************************************************************************************************
*	函 数 名: EraseChip
*	功能说明: 整个芯片擦除
*	形    参: 无
*	返 回 值: 0 表示成功， 1表示失败
*********************************************************************************************************
*/
int EraseChip (void) 
{	
    int result = 0;
    
    /* W25Q256初始化 */
    result = bsp_InitQSPI_W25Q256();
    if (result != 0)
    {
        return 1;
    }
                                             
    result = QSPI_EraseChip(); 
    if (result != 0)
    {
        return 1;
    }    

    /* 内存映射 */    
    result = QSPI_MemoryMapped(); 
    if (result != 0)
    {
        return 1;
    }
    
    return result;      
}

/*
*********************************************************************************************************
*	函 数 名: EraseSector
*	功能说明: 扇区擦除
*	形    参: adr 擦除地址
*	返 回 值: 无
*********************************************************************************************************
*/
int EraseSector (unsigned long adr) 
{	
    int result = 0;

    /* 地址要在操作的芯片范围内 */
    if (adr < QSPI_FLASH_MEM_ADDR || adr >= QSPI_FLASH_MEM_ADDR + QSPI_FLASH_SIZES)
    {
        return 1;
    }
    
    adr -= QSPI_FLASH_MEM_ADDR;
    
    /* W25Q256初始化 */
    result = bsp_InitQSPI_W25Q256();
    if (result != 0)
    {
        return 1;
    }
    
    /* 扇区擦除 */
    result = QSPI_EraseSector(adr);  
    if (result != 0)
    {
        return 1;
    }    

    /* 内存映射 */    
    result = QSPI_MemoryMapped(); 
    if (result != 0)
    {
        return 1;
    }
    
    return 0;   
}

/*
*********************************************************************************************************
*	函 数 名: ProgramPage
*	功能说明: 页编程
*	形    参: adr 页起始地址
*             sz  页大小
*             buf 要写入的数据地址
*	返 回 值: 无
*********************************************************************************************************
*/
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) 
{
    int size;
    int result = 0;

    /* 地址要在操作的芯片范围内 */    
    if (adr < QSPI_FLASH_MEM_ADDR || adr >= QSPI_FLASH_MEM_ADDR + QSPI_FLASH_SIZES)
    {
        return 1;
    }
   
    /* W25Q256初始化 */
    result = bsp_InitQSPI_W25Q256();
    if (result != 0)
    {
        return 1;
    }
        
    adr -= QSPI_FLASH_MEM_ADDR;
    size =  sz;
    
    /* 页编程 */
    while(size > 0)
    {
        if (QSPI_WriteBuffer(buf, adr, 256) == 1)
        {
            QSPI_MemoryMapped(); 
            
            return 1;   
        }
        size -= 256;
        adr += 256;
        buf += 256;
    }
    
    /* 内存映射 */    
    result = QSPI_MemoryMapped(); 
    if (result != 0)
    {
        return 1;
    }
    
    return (0);                      
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
