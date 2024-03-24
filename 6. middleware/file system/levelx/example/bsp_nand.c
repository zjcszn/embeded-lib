#include "bsp.h"

NAND_HandleTypeDef NAND_Handler;    //NAND FLASH句柄

uint8_t NAND_Init(void)
{
    FMC_NAND_PCC_TimingTypeDef ComSpaceTiming,AttSpaceTiming;

    NAND_Handler.Instance=FMC_NAND_DEVICE;
    NAND_Handler.Init.NandBank=FMC_NAND_BANK3;                          //NAND挂在BANK3上
    NAND_Handler.Init.Waitfeature=FMC_NAND_PCC_WAIT_FEATURE_DISABLE;    //关闭等待特性
    NAND_Handler.Init.MemoryDataWidth=FMC_NAND_PCC_MEM_BUS_WIDTH_8;     //8位数据宽度
    NAND_Handler.Init.EccComputation=FMC_NAND_ECC_DISABLE;              //禁止ECC
    NAND_Handler.Init.ECCPageSize=FMC_NAND_ECC_PAGE_SIZE_512BYTE;       //ECC页大小为512字节
    NAND_Handler.Init.TCLRSetupTime=5;                                 //设置TCLR(tCLR=CLE到RE的延时)=(TCLR+TSET+2)*THCLK,THCLK=1/200M=5ns
    NAND_Handler.Init.TARSetupTime=5;                                  //设置TAR(tAR=ALE到RE的延时)=(TAR+TSET+1)*THCLK,THCLK=1/200M=5n。

    NAND_Handler.Config.PageSize = 2048;
    NAND_Handler.Config.SpareAreaSize = 64;
    NAND_Handler.Config.BlockSize = 64;
    NAND_Handler.Config.BlockNbr = 1024;
    NAND_Handler.Config.PlaneNbr = 1;
    NAND_Handler.Config.PlaneSize = 1024;
    NAND_Handler.Config.ExtraCommandEnable = ENABLE;

    ComSpaceTiming.SetupTime=10;         //建立时间
    ComSpaceTiming.WaitSetupTime=10;    //等待时间
    ComSpaceTiming.HoldSetupTime=10;    //保持时间
    ComSpaceTiming.HiZSetupTime=10;     //高阻态时间

    AttSpaceTiming.SetupTime=10;         //建立时间
    AttSpaceTiming.WaitSetupTime=10;    //等待时间
    AttSpaceTiming.HoldSetupTime=10;    //保持时间
    AttSpaceTiming.HiZSetupTime=10;     //高阻态时间

    HAL_NAND_Init(&NAND_Handler,&ComSpaceTiming,&AttSpaceTiming);
    HAL_NAND_Reset(&NAND_Handler);
	NAND_ModeSet(4);			        //设置为MODE4,高速模式

    return 0;
}

//配置MPU的region
void NAND_MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_Initure;

	HAL_MPU_Disable();							//配置MPU之前先关闭MPU,配置完成以后在使能MPU

	//配置RAM为region1，大小为256MB，此区域可读写
	MPU_Initure.Enable=MPU_REGION_ENABLE;			//使能region
	MPU_Initure.Number=NAND_REGION_NUMBER;			//设置region，NAND使用的region0
	MPU_Initure.BaseAddress=NAND_ADDRESS_START;		//region基地址
	MPU_Initure.Size=NAND_REGION_SIZE;				//region大小
	MPU_Initure.SubRegionDisable=0X00;
	MPU_Initure.TypeExtField=MPU_TEX_LEVEL0;
	MPU_Initure.AccessPermission=MPU_REGION_FULL_ACCESS;	//此region可读写
	MPU_Initure.DisableExec=MPU_INSTRUCTION_ACCESS_ENABLE;	//允许读取此区域中的指令
	MPU_Initure.IsShareable=MPU_ACCESS_NOT_SHAREABLE;
	MPU_Initure.IsCacheable=MPU_ACCESS_NOT_CACHEABLE;
	MPU_Initure.IsBufferable=MPU_ACCESS_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_Initure);

	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);			//开启MPU
}

uint8_t NAND_ModeSet(uint8_t mode)
{
    *(__IO uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_FEATURE;//发送设置特性命令
    *(__IO uint8_t*)(NAND_ADDRESS|NAND_ADDR)=0X01;		//地址为0X01,设置mode
 	*(__IO uint8_t*)NAND_ADDRESS=mode;					//P1参数,设置mode
	*(__IO uint8_t*)NAND_ADDRESS=0;
	*(__IO uint8_t*)NAND_ADDRESS=0;
	*(__IO uint8_t*)NAND_ADDRESS=0;
	return 0;
}
#if 0
#include <string.h>
uint8_t buf[2048];
void NAND_check(void)
{
	NAND_AddressTypeDef addr = {0};
	NAND_IDTypeDef id;

    HAL_NAND_Read_ID(&NAND_Handler, &id);
    kprintf("HAL_id = 0x%02x%02x%02x%02x\r\n", id.Device_Id,id.Fourth_Id,id.Maker_Id,id.Third_Id);

	addr.Block = 0;
	addr.Page = 0;
	addr.Plane = 0;
	kprintf("3.HAL_NAND_Erase_Block = %d\r\n", HAL_NAND_Erase_Block(&NAND_Handler, &addr));

	memset(buf,0,2048);


    kprintf("2.HAL_NAND_Read_Page_8b = %d\r\n", HAL_NAND_Read_Page_8b(&NAND_Handler, &addr, buf, 1));
    for(int i = 0; i < 2048; i++)
    {
        if((i % 5) == 0)  kprintf("\r\n");
        kprintf("buf[%04d] = 0x%02X ", i, buf[i]);
    }
    kprintf("\r\n");


    // 4、初始化数据，把新的数据写入芯片第0页，并打印HAL_NAND_Write_Page_8b函数执行结果
    for(int i = 0; i < 2048; i++)
    {
        buf[i] = i & 0x00FF;
    }
    kprintf("4.HAL_NAND_Write_Page_8b = %d\r\n", HAL_NAND_Write_Page_8b(&NAND_Handler, &addr, buf, 1));

    // 5、重新读取芯片第0页数据，并打印HAL_NAND_Read_Page_8b函数执行结果
    memset(buf, 0, 2048);
    kprintf("5.HAL_NAND_Read_Page_8b = %d\r\n", HAL_NAND_Read_Page_8b(&NAND_Handler, &addr, buf, 1));
    for(int i = 0; i < 2048; i++)
    {
        if((i % 5) == 0)  kprintf("\r\n");
        kprintf("buf[%04d] = 0x%02X ", i, buf[i]);
    }
}

#include "lx_stm32_nand_custom_driver.h"
struct fx_lx_nand_driver_instance  fx_lx_nand_drivers =
{
    .name = "NAND CUSTOM DRIVER",  .id = 0xABCDEF, .nand_driver_initialize = _lx_nand_flash_initialize_driver
};
long nand_test(void)
{
	UINT status;
	static struct fx_lx_nand_driver_instance *current_driver = NULL;
	current_driver = &fx_lx_nand_drivers;
	if(current_driver->initialized == FX_FALSE)
    {
		status = lx_nand_flash_open(&current_driver->flash_instance, current_driver->name, current_driver->nand_driver_initialize);
		if (status == LX_SUCCESS)
		{
			current_driver->initialized = FX_TRUE;
			memset(buf, 0xe2, 2048);
			status = lx_nand_flash_sector_write(&current_driver->flash_instance, 0, buf);
			if(status == LX_SUCCESS)
			{
				memset(buf, 0, 2048);
				status = lx_nand_flash_sector_read(&current_driver->flash_instance, 0, buf);
				if (status == LX_SUCCESS)
				{
						for(int i = 0; i < 2048; i++)
						{
							if((i % 5) == 0)  kprintf("\r\n");
							kprintf("buf[%04d] = 0x%02X ", i, buf[i]);
						}
				}
				else
					kprintf("nand read:%d\r\n", status);
			}
			else
				kprintf("nand write:%d\r\n", status);
		}
		else
			kprintf("nand init:%d\r\n", status);
	}
	return 0;
}
MSH_CMD_EXPORT(nand_test, nand_test TEST);
#endif
/**
* @brief  Reads NAND flash IDs.
* @param  Instance  NAND Instance
* @param  pNAND_ID   Pointer to NAND ID structure
* @retval BSP status
*/
int32_t BSP_FMC_NAND_ReadID(NAND_HandleTypeDef *hnand, NAND_IDTypeDef *pNAND_ID)
{
	int32_t ret = HAL_OK;

	if(HAL_NAND_Read_ID(hnand, pNAND_ID) != HAL_OK)
	{
	  ret = HAL_ERROR;
	}

  return ret;
}

/**
* @brief  Reads current status of the NAND memory.
* @param  Instance  NAND Instance
* @retval NAND memory status: whether busy or not
*/
int32_t BSP_FMC_NAND_GetStatus(NAND_HandleTypeDef *hnand)
{
	int32_t ret = HAL_OK;
	uint32_t status;

	status = HAL_NAND_Read_Status(hnand);

	switch (status)
	{
	case NAND_BUSY:
	  ret = HAL_BUSY;
	  break;

	case NAND_ERROR:
	  ret = HAL_ERROR;
	  break;

	case NAND_READY:
	  ret = HAL_OK;
	  break;
	}

  return ret;
}

int32_t BSP_FMC_NAND_EraseChip(NAND_HandleTypeDef *hnand)
{
	int32_t ret = HAL_OK;
	uint32_t i;
	NAND_AddressTypeDef Address = {0};

	for (i = 0; i < hnand->Config.BlockNbr; i++)
	{
	  Address.Block = i;

	  if (HAL_NAND_Erase_Block(hnand, &Address) != HAL_OK)
	  {
		ret = HAL_ERROR;
	  }
	}

  return ret;
}

/**
* @brief  Erases the specified block of the NAND device.
* @param  Instance  NAND Instance
* @param  pAddress  pointer to BSP_NAND address structure
* @retval BSP status
*/

int32_t BSP_FMC_NAND_Erase_Block(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress)
{
	int32_t ret = HAL_OK;

	NAND_AddressTypeDef Address = {0};

	Address.Block = pAddress->Block;
	Address.Page = pAddress->Page;

	if (HAL_NAND_Erase_Block(hnand, &Address) != HAL_OK)
	{
	  ret = HAL_ERROR;
	}

  return ret;
}

/**
* @brief  Reads an amount of data from the NAND device.
* @param  Instance       NAND Instance
* @param  pBuffer        pointer to destination read buffer
* @param  pAddress       pointer to BSP_NAND address structure
* @param  NumPageToRead  number of pages to read from block
* @retval BSP status
*/
int32_t BSP_FMC_NAND_Read(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress, uint8_t *pBuffer, uint32_t NumPageToRead)
{
	int32_t ret = HAL_OK;
	NAND_AddressTypeDef Address = {0};

	Address.Block = pAddress->Block;
	Address.Page = pAddress->Page;

	if (HAL_NAND_Read_Page_8b(hnand, &Address, pBuffer, NumPageToRead) != HAL_OK)
	{
	  ret = HAL_ERROR;
	}
	return ret;
}

/**
* @brief  Write Page(s) to NAND memory block
* @param  Instance         NAND Instance
* @param  pAddress         pointer to BSP_NAND address structure
* @param  pBuffer          pointer to source buffer to write.
* @param  NumPageToWrite   number of pages to write to block
* @retval BSP status
*/
int32_t BSP_FMC_NAND_Write(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress, uint8_t *pBuffer, uint32_t NumPageToWrite)
{
	int32_t ret = HAL_OK;
	NAND_AddressTypeDef Address = {0};

	Address.Block = pAddress->Block;
	Address.Page = pAddress->Page;

	if (HAL_NAND_Write_Page_8b(hnand, &Address, pBuffer, NumPageToWrite) != HAL_OK)
	{
	  ret = HAL_ERROR;
	}

	return ret;
}

/**
* @brief  Read Spare area(s) from NAND device
* @param  Instance             NAND Instance
* @param  pAddress             pointer to BSP_NAND address structure
* @param  pBuffer              pointer to source buffer to write. pBuffer should be 16bits aligned.
* @param  NumSpareAreaToRead   Number of spare area to read
* @retval HAL status
*/
int32_t  BSP_FMC_NAND_Read_SpareArea(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress, uint8_t *pBuffer, uint32_t NumSpareAreaToRead)
{
	int32_t ret = HAL_OK;
	NAND_AddressTypeDef Address = {0};

	Address.Block = pAddress->Block;
	Address.Page = pAddress->Page;

	if (HAL_NAND_Read_SpareArea_8b(hnand, &Address, pBuffer, NumSpareAreaToRead) != HAL_OK)
	{
	  ret = HAL_ERROR;
	}

	return ret;
}

/**
* @brief  Write Spare area(s) to NAND device
* @param  Instance              NAND Instance
* @param  pAddress              pointer to BSP_NAND address structure
* @param  pBuffer               pointer to source buffer to write
* @param  NumSpareAreaTowrite   number of spare areas to write to block
* @retval HAL status
*/
int32_t  BSP_FMC_NAND_Write_SpareArea(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress, uint8_t *pBuffer, uint32_t NumSpareAreaTowrite)
{
	int32_t ret = HAL_OK;
	NAND_AddressTypeDef Address = {0};

	Address.Block = pAddress->Block;
	Address.Page = pAddress->Page;

	if (HAL_NAND_Write_SpareArea_8b(hnand, &Address, pBuffer, NumSpareAreaTowrite) != HAL_OK)
	{
	  ret = HAL_ERROR;
	}

	return ret;
}

