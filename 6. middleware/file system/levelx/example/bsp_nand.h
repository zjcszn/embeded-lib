#ifndef __BSP_NAND_H__
#define __BSP_NAND_H__
#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/
	
#include "stm32h7xx.h"

#define NAND_MAX_PAGE_SIZE			2048		//定义NAND FLASH的最大的PAGE大小（不包括SPARE区），默认4096字节
#define NAND_ECC_SECTOR_SIZE		512			//执行ECC计算的单元大小，默认512字节

typedef struct
{
  uint16_t Page;   /*!< NAND memory Page address  */

  uint16_t Block;  /*!< NAND memory Block address */

} BSP_NAND_AddressTypeDef_t;
extern NAND_HandleTypeDef NAND_Handler;    //NAND FLASH句柄

#define NAND_RB  	 (((GPIOD->IDR) >> 6) & 0x1U)//NAND Flash的闲/忙引脚

#define NAND_ADDRESS			0X80000000	//nand flash的访问地址,接NCE3,地址为:0X8000 0000
#define NAND_CMD				1<<16		//发送命令
#define NAND_ADDR				1<<17		//发送地址

//NAND FLASH命令
#define NAND_READID         	0X90    	//读ID指令
#define NAND_FEATURE			0XEF    	//设置特性指令
#define NAND_RESET          	0XFF    	//复位NAND
#define NAND_READSTA        	0X70   	 	//读状态
#define NAND_AREA_A         	0X00
#define NAND_AREA_TRUE1     	0X30
#define NAND_WRITE0        	 	0X80
#define NAND_WRITE_TURE1    	0X10
#define NAND_ERASE0        	 	0X60
#define NAND_ERASE1         	0XD0
#define NAND_MOVEDATA_CMD0  	0X00
#define NAND_MOVEDATA_CMD1  	0X35
#define NAND_MOVEDATA_CMD2  	0X85
#define NAND_MOVEDATA_CMD3  	0X10

//NAND FLASH状态
#define NSTA_READY       	   	0X40		//nand已经准备好
#define NSTA_ERROR				0X01		//nand错误
#define NSTA_TIMEOUT        	0X02		//超时
#define NSTA_ECC1BITERR       	0X03		//ECC 1bit错误
#define NSTA_ECC2BITERR       	0X04		//ECC 2bit以上错误


//NAND FLASH型号和对应的ID号
#define MT29F4G08ABADA			0XDC909556	//MT29F4G08ABADA
#define MT29F16G08ABABA			0X48002689	//MT29F16G08ABABA
#define W29N01GVSIAA			0XF1809500	//W29N01GVSIAA
#define W29N01HVSINA			0XF1009500	//W29N01HVSINA

#define BSP_NAND_MANUFACTURER_CODE  0xEF
#define BSP_NAND_DEVICE_CODE        0xF1
#define BSP_NAND_THIRD_ID           0x00
#define BSP_NAND_FOURTH_ID          0x95

//MPU相关设置
#define NAND_REGION_NUMBER      MPU_REGION_NUMBER3	    //NAND FLASH使用region0
#define NAND_ADDRESS_START      0X80000000              //NAND FLASH区的首地址
#define NAND_REGION_SIZE        MPU_REGION_SIZE_256MB   //NAND FLASH区大小

uint8_t NAND_Init(void);
uint8_t NAND_ModeSet(uint8_t mode);
void NAND_MPU_Config(void);
void NAND_check(void);
int32_t BSP_FMC_NAND_ReadID(NAND_HandleTypeDef *hnand, NAND_IDTypeDef *pNAND_ID);
int32_t BSP_FMC_NAND_GetStatus(NAND_HandleTypeDef *hnand);
int32_t BSP_FMC_NAND_EraseChip(NAND_HandleTypeDef *hnand);
int32_t BSP_FMC_NAND_Erase_Block(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress);
int32_t BSP_FMC_NAND_Read(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress, uint8_t *pBuffer, uint32_t NumPageToRead);
int32_t BSP_FMC_NAND_Write(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress, uint8_t *pBuffer, uint32_t NumPageToWrite);
int32_t  BSP_FMC_NAND_Read_SpareArea(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress, uint8_t *pBuffer, uint32_t NumSpareAreaToRead);
int32_t  BSP_FMC_NAND_Write_SpareArea(NAND_HandleTypeDef *hnand, BSP_NAND_AddressTypeDef_t *pAddress, uint8_t *pBuffer, uint32_t NumSpareAreaTowrite);

#if 0
#include "lx_api.h"
struct fx_lx_nand_driver_instance
{
    LX_NAND_FLASH flash_instance;

    CHAR name[32];

    UINT id;

    UINT (*nand_driver_initialize)(LX_NAND_FLASH *);

    UINT initialized;

};
#endif
#ifdef __cpluscplus
}
#endif /*__cplusplus*/
#endif /*__BSP_NAND_H__*/
