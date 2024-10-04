#include "bsp_icm42605.h"
#include "usart.h"
#include "stdio.h"

#define ICM42605DelayMs(_nms)  HAL_Delay(_nms)



/* 
	
	注意：如果修改了SPI或者GPIO引脚，注意修改对应 SPIx_SCK_AF、SPIx_MISO_AF、SPIx_MOSI_AF
	不定义在头文件中，避免重复定义

*/

#define SPIx							SPI6
#define SPIx_CLK_ENABLE()				__HAL_RCC_SPI6_CLK_ENABLE()

#define SPIx_FORCE_RESET()				__HAL_RCC_SPI6_FORCE_RESET()
#define SPIx_RELEASE_RESET()			__HAL_RCC_SPI6_RELEASE_RESET()

#define SPIx_SCK_CLK_ENABLE()			__HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MISO_CLK_ENABLE()			__HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MOSI_CLK_ENABLE()			__HAL_RCC_GPIOG_CLK_ENABLE()

#define SPIx_SCK_GPIO					GPIOA
#define SPIx_SCK_PIN					GPIO_PIN_5
#define SPIx_SCK_AF						GPIO_AF8_SPI6			

#define SPIx_MISO_GPIO					GPIOA
#define SPIx_MISO_PIN 					GPIO_PIN_6
#define SPIx_MISO_AF					GPIO_AF8_SPI6

#define SPIx_MOSI_GPIO					GPIOG
#define SPIx_MOSI_PIN 					GPIO_PIN_14
#define SPIx_MOSI_AF					GPIO_AF5_SPI6



/*********************************************************************************************************
											 变量
*********************************************************************************************************/
SPI_HandleTypeDef hspi = {0};

#define	SPI_BUFFER_SIZE		(4 * 1024)	
uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];  
uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];


static void bsp_InitSPIBus(void);
static void bsp_InitSPIGPIO(void);
static void bsp_InitSPIParam(void);
static uint8_t hal_Spi6_ReadWriteByte(uint8_t txdata);

static void Icm_Spi_ReadWriteNbytes(uint8_t* pBuffer, uint8_t len);
static uint8_t icm42605_read_reg(uint8_t reg);
static void icm42605_read_regs(uint8_t reg, uint8_t* buf, uint16_t len);
static uint8_t icm42605_write_reg(uint8_t reg, uint8_t value);


static float accSensitivity;   		/* 加速度的最小分辨率 */
static float gyroSensitivity;    	/* 陀螺仪的最小分辨率 */



static void bsp_InitSPIBus(void)
{
	bsp_InitSPIGPIO();
	bsp_InitSPIParam();
}


static void bsp_InitSPIGPIO(void)
{
	/* 配置 SPI总线GPIO : SCK MOSI MISO */
	{
		GPIO_InitTypeDef  GPIO_InitStruct;
			
		/* SPI和GPIP时钟 */
		SPIx_SCK_CLK_ENABLE();
		SPIx_MISO_CLK_ENABLE();
		SPIx_MOSI_CLK_ENABLE();
		SPIx_CLK_ENABLE();

		/* SPI SCK */
		GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = SPIx_SCK_AF;
		HAL_GPIO_Init(SPIx_SCK_GPIO, &GPIO_InitStruct);

		/* SPI MISO */
		GPIO_InitStruct.Pin 	  = SPIx_MISO_PIN;
		GPIO_InitStruct.Pull      = GPIO_NOPULL;
		GPIO_InitStruct.Alternate = SPIx_MISO_AF;
		HAL_GPIO_Init(SPIx_MISO_GPIO, &GPIO_InitStruct);

		/* SPI MOSI */
		GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
		GPIO_InitStruct.Pull      = GPIO_NOPULL;
		GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
		HAL_GPIO_Init(SPIx_MOSI_GPIO, &GPIO_InitStruct);
	}
}


/* 注意MCU主频400M */
static void bsp_InitSPIParam(void)
{	
	/* 设置SPI参数 */
	hspi.Instance               = SPIx;                   		/* 例化SPI */
	hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;    /* 设置波特率 */
	hspi.Init.Direction         = SPI_DIRECTION_2LINES;   		/* 全双工 */
	hspi.Init.CLKPolarity       = SPI_POLARITY_HIGH;           	/* 配置时钟极性 */
	hspi.Init.CLKPhase          = SPI_PHASE_2EDGE;              /* 配置时钟相位 */
	hspi.Init.DataSize          = SPI_DATASIZE_8BIT;      		/* 设置数据宽度 */
	hspi.Init.FirstBit          = SPI_FIRSTBIT_MSB;       		/* 数据传输先传高位 */
	hspi.Init.TIMode            = SPI_TIMODE_DISABLE;     		/* 禁止TI模式  */
	hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE; 	/* 禁止CRC */
	hspi.Init.CRCPolynomial     = 7;                       		/* 禁止CRC后，此位无效 */
	hspi.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;     		/* 禁止CRC后，此位无效 */
	hspi.Init.NSS               = SPI_NSS_SOFT;               	/* 使用软件方式管理片选引脚 */
	hspi.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_01DATA;  	/* 设置FIFO大小是一个数据项 */
	hspi.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;      	/* 禁止脉冲输出 */
	hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE; 	/* 禁止SPI后，SPI相关引脚保持当前状态 */  
	hspi.Init.Mode 			 	= SPI_MODE_MASTER;            		/* SPI工作在主控模式 */

	/* 复位配置 */
	if (HAL_SPI_DeInit(&hspi) != HAL_OK)
	{

	}	

	/* 初始化配置 */
	if (HAL_SPI_Init(&hspi) != HAL_OK)
	{

	}		
	
	printf("SPI Init OK!\r\n");
}

static uint8_t hal_Spi6_ReadWriteByte(uint8_t txdata)
{
    uint8_t rxdata = 0;
    HAL_SPI_TransmitReceive(&hspi, &txdata, &rxdata, 1, 0xff);
    return rxdata;
}



/* 初始化传感器CS */
void bsp_IcmSpixCsInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    ICM_RCC_SPIX_CS();

    /*Configure GPIO pins */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = ICM_PIN_SPIX_CS;
    HAL_GPIO_Init(ICM_PORT_SPIX_CS, &GPIO_InitStruct);
    HAL_GPIO_WritePin(ICM_PORT_SPIX_CS, ICM_PIN_SPIX_CS, GPIO_PIN_SET);
}


/* 传感器基本读写操作 */
static void Icm_Spi_ReadWriteNbytes(uint8_t* pBuffer, uint8_t len)
{
    uint8_t i = 0;

    for(i = 0; i < len; i ++)
    {
        *pBuffer = hal_Spi6_ReadWriteByte(*pBuffer);
        pBuffer++;
    }
}


/* 读单个寄存器 */
static uint8_t icm42605_read_reg(uint8_t reg)
{
    uint8_t regval = 0;
    reg=reg | 0x80;

    ICM_SPI_CS_LOW();

    Icm_Spi_ReadWriteNbytes(&reg, 1);
    Icm_Spi_ReadWriteNbytes(&regval, 1);

    ICM_SPI_CS_HIGH();

    return regval;
}

/* 连续读寄存器 */
static void icm42605_read_regs(uint8_t reg, uint8_t* buf, uint16_t len)
{
    reg=reg | 0x80;

    ICM_SPI_CS_LOW();

    Icm_Spi_ReadWriteNbytes(&reg, 1);
    Icm_Spi_ReadWriteNbytes(buf, len);

    ICM_SPI_CS_HIGH();
}

/* 写单个寄存器 */
static uint8_t icm42605_write_reg(uint8_t reg, uint8_t value)
{

    ICM_SPI_CS_LOW();

    Icm_Spi_ReadWriteNbytes(&reg, 1);
    Icm_Spi_ReadWriteNbytes(&value, 1);

    ICM_SPI_CS_HIGH();
    return 0;
}

/* 计算当前加速度分辨率 */
float bsp_Icm42605GetAres(uint8_t Ascale)
{
    switch(Ascale)
    {
    case AFS_2G:
        accSensitivity = 2000 / 32768.0f;
        break;
    case AFS_4G:
        accSensitivity = 4000 / 32768.0f;
        break;
    case AFS_8G:
        accSensitivity = 8000 / 32768.0f;
        break;
    case AFS_16G:
        accSensitivity = 16000 / 32768.0f;
        break;
    }

    return accSensitivity;
}

/* 计算当前陀螺仪分辨率 */
float bsp_Icm42605GetGres(uint8_t Gscale)
{
    switch(Gscale)
    {
    case GFS_15_125DPS:
        gyroSensitivity = 15.125f / 32768.0f;
        break;
    case GFS_31_25DPS:
        gyroSensitivity = 31.25f / 32768.0f;
        break;
    case GFS_62_5DPS:
        gyroSensitivity = 62.5f / 32768.0f;
        break;
    case GFS_125DPS:
        gyroSensitivity = 125.0f / 32768.0f;
        break;
    case GFS_250DPS:
        gyroSensitivity = 250.0f / 32768.0f;
        break;
    case GFS_500DPS:
        gyroSensitivity = 500.0f / 32768.0f;
        break;
    case GFS_1000DPS:
        gyroSensitivity = 1000.0f / 32768.0f;
        break;
    case GFS_2000DPS:
        gyroSensitivity = 2000.0f / 32768.0f;
        break;
    }
	
	gyroSensitivity = gyroSensitivity*1000;
	
	
    return gyroSensitivity;
}



uint8_t bsp_Icm42605RegCfg(void)
{
    uint8_t reg_val = 0;
    /* 读取 who am i 寄存器 */
    reg_val = icm42605_read_reg(ICM42605_WHO_AM_I);

    icm42605_write_reg(ICM42605_REG_BANK_SEL, 0); 		/* 设置bank 0区域寄存器 */
    icm42605_write_reg(ICM42605_REG_BANK_SEL, 0x01); 	/* 软复位传感器 */
	
    ICM42605DelayMs(100);
	/* ICM42605_ID == 0x42 */
    if(reg_val == ICM42605_ID)
    {
        icm42605_write_reg(ICM42605_REG_BANK_SEL, 1); 		/* 设置bank 1区域寄存器 */
        icm42605_write_reg(ICM42605_INTF_CONFIG4, 0x02); 	/* 设置为4线SPI通信 */

        icm42605_write_reg(ICM42605_REG_BANK_SEL, 0); 		/* 设置bank 0区域寄存器 */
        icm42605_write_reg(ICM42605_FIFO_CONFIG, 0x40); 	/* Stream-to-FIFO Mode(page61) */


        reg_val = icm42605_read_reg(ICM42605_INT_SOURCE0);
        icm42605_write_reg(ICM42605_INT_SOURCE0, 0x00);
        icm42605_write_reg(ICM42605_FIFO_CONFIG2, 0x00); 
        icm42605_write_reg(ICM42605_FIFO_CONFIG3, 0x02); 
        icm42605_write_reg(ICM42605_INT_SOURCE0, reg_val);
        icm42605_write_reg(ICM42605_FIFO_CONFIG1, 0x63); 	/* 使能FIFO */

        icm42605_write_reg(ICM42605_REG_BANK_SEL, 0x00);
        icm42605_write_reg(ICM42605_INT_CONFIG, 0x36);

        icm42605_write_reg(ICM42605_REG_BANK_SEL, 0x00);
        reg_val = icm42605_read_reg(ICM42605_INT_SOURCE0);
        reg_val |= (1 << 2);
        icm42605_write_reg(ICM42605_INT_SOURCE0, reg_val);
	
        bsp_Icm42605GetAres(AFS_8G);						/* 默认8g量程 */
        icm42605_write_reg(ICM42605_REG_BANK_SEL, 0x00);
        reg_val = icm42605_read_reg(ICM42605_ACCEL_CONFIG0);
        reg_val |= (AFS_8G << 5);  
        reg_val |= (AODR_200Hz);    
        icm42605_write_reg(ICM42605_ACCEL_CONFIG0, reg_val);

        bsp_Icm42605GetGres(GFS_1000DPS);					/* 默认1000dps量程 */
        icm42605_write_reg(ICM42605_REG_BANK_SEL, 0x00);
        reg_val = icm42605_read_reg(ICM42605_GYRO_CONFIG0);	
        reg_val |= (GFS_1000DPS << 5);  
        reg_val |= (GODR_200Hz);     
        icm42605_write_reg(ICM42605_GYRO_CONFIG0, reg_val);

        icm42605_write_reg(ICM42605_REG_BANK_SEL, 0x00);
        reg_val = icm42605_read_reg(ICM42605_PWR_MGMT0); 	/* 读取PWR—MGMT0当前寄存器的值(page72) */
        reg_val &= ~(1 << 5);		/* 使能温度测量 */
        reg_val |= ((3) << 2);		/* 设置GYRO_MODE  0:关闭 1:待机 2:预留 3:低噪声 */
        reg_val |= (3);				/* 设置ACCEL_MODE 0:关闭 1:关闭 2:低功耗 3:低噪声 */
        icm42605_write_reg(ICM42605_PWR_MGMT0, reg_val);
        ICM42605DelayMs(1); 		/* 操作完PWR—MGMT0寄存器后 200us内不能有任何读写寄存器的操作 */

        return 0;
    }
    return 0xff;
}

/* 初始化传感器，在bsp.c中调用 */
uint8_t bsp_Icm42605Init(void)
{
	bsp_InitSPIBus();			/* 初始化传感器使用的SPI6 */
	
	ICM42605DelayMs(100);
	
    bsp_IcmSpixCsInit();
    bsp_Icm42605RegCfg();
	return 0;
}



/* 获取芯片内部温度原始值 */
void bsp_IcmGetTemperature(int16_t* pTemp)
{
    uint8_t buffer[2] = {0};

    icm42605_read_regs(ICM42605_TEMP_DATA1, buffer, 2);

    *pTemp = (int16_t)(((int16_t)((buffer[0] << 8) | buffer[1])) / 132.48 + 25);

    printf("pTemp:%d\r\n",*pTemp);
}


/* 获取加速度原始值 */
void bsp_IcmGetRawAccelerometer(icm42605RawData_t* accData)
{
    uint8_t buffer[6] = {0};

    icm42605_read_regs(ICM42605_ACCEL_DATA_X1, buffer, 6);

    accData->x = ((uint16_t)buffer[0] << 8) | buffer[1];
    accData->y = ((uint16_t)buffer[2] << 8) | buffer[3];
    accData->z = ((uint16_t)buffer[4] << 8) | buffer[5];
}

/* 获取陀螺仪原始值 */
void bsp_IcmGetRawGyroscope(icm42605RawData_t* GyroData)
{
    uint8_t buffer[6] = {0};

    icm42605_read_regs(ICM42605_GYRO_DATA_X1, buffer, 6);

    GyroData->x = ((uint16_t)buffer[0] << 8) | buffer[1];
    GyroData->y = ((uint16_t)buffer[2] << 8) | buffer[3];
    GyroData->z = ((uint16_t)buffer[4] << 8) | buffer[5];
}

/* 获取6轴原始值 */
void bsp_IcmGetRawData(icm42605RawData_t* accRawData, icm42605RawData_t* GyroRawData)
{
    uint8_t buffer[12] = {0};

    icm42605_read_regs(ICM42605_ACCEL_DATA_X1, buffer, 12);

    accRawData->x  = ((uint16_t)buffer[0] << 8)  | buffer[1];
    accRawData->y  = ((uint16_t)buffer[2] << 8)  | buffer[3];
    accRawData->z  = ((uint16_t)buffer[4] << 8)  | buffer[5];
    GyroRawData->x = ((uint16_t)buffer[6] << 8)  | buffer[7];
    GyroRawData->y = ((uint16_t)buffer[8] << 8)  | buffer[9];
    GyroRawData->z = ((uint16_t)buffer[10] << 8) | buffer[11];
}



