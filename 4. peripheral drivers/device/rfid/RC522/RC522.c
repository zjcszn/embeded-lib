#include "stm32f1xx_hal.h"
#include "RC522.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"
#include "tim.h"

#define osDelay HAL_Delay


#define RS522_RST(N) HAL_GPIO_WritePin(RC522_RST_GPIO_Port, RC522_RST_Pin, N==1?GPIO_PIN_SET:GPIO_PIN_RESET)
#define RS522_NSS(N) HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, N==1?GPIO_PIN_SET:GPIO_PIN_RESET)

extern SPI_HandleTypeDef hspi2;


void MFRC_Init(void)
{
    RS522_NSS(1);
    RS522_RST(1);
}

static uint8_t ret;  //这些函数是HAL与标准库不同的地方【读写函数】
uint8_t SPI2_RW_Byte(uint8_t byte)
{
    HAL_SPI_TransmitReceive(&hspi2, &byte, &ret, 1, 10);//把byte 写入，并读出一个值，把它存入ret
    return   ret;//入口是byte 的地址，读取时用的也是ret地址，一次只写入一个值10
}

void MFRC_WriteReg(uint8_t addr, uint8_t data)
{
    uint8_t AddrByte;
    AddrByte = (addr << 1 ) & 0x7E; //求出地址字节
    RS522_NSS(0);                   //NSS拉低
    SPI2_RW_Byte(AddrByte);         //写地址字节
    SPI2_RW_Byte(data);             //写数据
    RS522_NSS(1);                   //NSS拉高
}

uint8_t MFRC_ReadReg(uint8_t addr)
{
    uint8_t AddrByte, data;
    AddrByte = ((addr << 1 ) & 0x7E ) | 0x80;   //求出地址字节
    RS522_NSS(0);                               //NSS拉低
    SPI2_RW_Byte(AddrByte);                     //写地址字节
    data = SPI2_RW_Byte(0x00);                  //读数据
    RS522_NSS(1);                               //NSS拉高
    return data;
}

void MFRC_SetBitMask(uint8_t addr, uint8_t mask)
{
    uint8_t temp;
    temp = MFRC_ReadReg(addr);                  //先读回寄存器的值
    MFRC_WriteReg(addr, temp | mask);           //处理过的数据再写入寄存器
}

void MFRC_ClrBitMask(uint8_t addr, uint8_t mask)
{
    uint8_t temp;
    temp = MFRC_ReadReg(addr);                  //先读回寄存器的值
    MFRC_WriteReg(addr, temp & ~mask);          //处理过的数据再写入寄存器
}

void MFRC_CalulateCRC(uint8_t *pInData, uint8_t len, uint8_t *pOutData)
{
    //0xc1 1        2           pInData[2]
    uint8_t temp;
    uint32_t i;
    MFRC_ClrBitMask(MFRC_DivIrqReg, 0x04);                  //使能CRC中断
    MFRC_WriteReg(MFRC_CommandReg, MFRC_IDLE);              //取消当前命令的执行
    MFRC_SetBitMask(MFRC_FIFOLevelReg, 0x80);               //清除FIFO及其标志位
    for(i = 0; i < len; i++)                                //将待CRC计算的数据写入FIFO
    {
        MFRC_WriteReg(MFRC_FIFODataReg, *(pInData + i));
    }
    MFRC_WriteReg(MFRC_CommandReg, MFRC_CALCCRC);           //执行CRC计算
    i = 100000;
    do
    {
        temp = MFRC_ReadReg(MFRC_DivIrqReg);                //读取DivIrqReg寄存器的值
        i--;
    }
    while((i != 0) && !(temp & 0x04));                      //等待CRC计算完成
    pOutData[0] = MFRC_ReadReg(MFRC_CRCResultRegL);         //读取CRC计算结果
    pOutData[1] = MFRC_ReadReg(MFRC_CRCResultRegM);
}

char MFRC_CmdFrame(uint8_t cmd, uint8_t *pInData, uint8_t InLenByte, uint8_t *pOutData, uint16_t *pOutLenBit)
{
    uint8_t lastBits;
    uint8_t n;
    uint32_t i;
    char status = MFRC_ERR;
    uint8_t irqEn   = 0x00;
    uint8_t waitFor = 0x00;

    /*根据命令设置标志位*/
    switch(cmd)
    {
        case MFRC_AUTHENT:                  //Mifare认证
            irqEn = 0x12;
            waitFor = 0x10;                 //idleIRq中断标志
            break;
        case MFRC_TRANSCEIVE:               //发送并接收数据
            irqEn = 0x77;
            waitFor = 0x30;                 //RxIRq和idleIRq中断标志
            break;
    }

    /*发送命令帧前准备*/
    MFRC_WriteReg(MFRC_ComIEnReg, irqEn | 0x80);    //开中断
    MFRC_ClrBitMask(MFRC_ComIrqReg, 0x80);          //清除中断标志位SET1
    MFRC_WriteReg(MFRC_CommandReg, MFRC_IDLE);      //取消当前命令的执行
    MFRC_SetBitMask(MFRC_FIFOLevelReg, 0x80);       //清除FIFO缓冲区及其标志位

    /*发送命令帧*/
    for(i = 0; i < InLenByte; i++)                  //写入命令参数
    {
        MFRC_WriteReg(MFRC_FIFODataReg, pInData[i]);
    }
    MFRC_WriteReg(MFRC_CommandReg, cmd);            //执行命令
    if(cmd == MFRC_TRANSCEIVE)
    {
        MFRC_SetBitMask(MFRC_BitFramingReg, 0x80);  //启动发送
    }
    i = 300000;                                     //根据时钟频率调整,操作M1卡最大等待时间25ms
    do
    {
        n = MFRC_ReadReg(MFRC_ComIrqReg);
        i--;
    }
    while((i != 0) && !(n & 0x01) && !(n & waitFor));     //等待命令完成
    MFRC_ClrBitMask(MFRC_BitFramingReg, 0x80);          //停止发送

    /*处理接收的数据*/
    if(i != 0)
    {
        if(!(MFRC_ReadReg(MFRC_ErrorReg) & 0x1B))
        {
            status = MFRC_OK;
            if(n & irqEn & 0x01)
            {
                status = MFRC_NOTAGERR;
            }
            if(cmd == MFRC_TRANSCEIVE)
            {
                n = MFRC_ReadReg(MFRC_FIFOLevelReg);
                lastBits = MFRC_ReadReg(MFRC_ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if(n == 0)
                {
                    n = 1;
                }
                if(n > MFRC_MAXRLEN)
                {
                    n = MFRC_MAXRLEN;
                }
                for(i = 0; i < n; i++)
                {
                    pOutData[i] = MFRC_ReadReg(MFRC_FIFODataReg);
                }
            }
        }
        else
        {
            status = MFRC_ERR;
        }
    }

    MFRC_SetBitMask(MFRC_ControlReg, 0x80);               //停止定时器运行
    MFRC_WriteReg(MFRC_CommandReg, MFRC_IDLE);            //取消当前命令的执行

    return status;
}

void PCD_Reset(void)
{
    /*硬复位*/
    RS522_RST(1);//用到复位引脚
    osDelay(2);
    RS522_RST(0);
    osDelay(2);
    RS522_RST(1);
    osDelay(2);

    /*软复位*/
    MFRC_WriteReg(MFRC_CommandReg, MFRC_RESETPHASE);
    osDelay(2);

    /*复位后的初始化配置*/
    MFRC_WriteReg(MFRC_ModeReg, 0x3D);              //CRC初始值0x6363
    MFRC_WriteReg(MFRC_TReloadRegL, 30);            //定时器重装值
    MFRC_WriteReg(MFRC_TReloadRegH, 0);
    MFRC_WriteReg(MFRC_TModeReg, 0x8D);             //定时器设置
    MFRC_WriteReg(MFRC_TPrescalerReg, 0x3E);        //定时器预分频值
    MFRC_WriteReg(MFRC_TxAutoReg, 0x40);            //100%ASK

    PCD_AntennaOff();                               //关天线
    osDelay(2);
    PCD_AntennaOn();                                //开天线
    
    printf("初始化完成\n");
}

void PCD_AntennaOn(void)
{
    uint8_t temp;
    temp = MFRC_ReadReg(MFRC_TxControlReg);
    if (!(temp & 0x03))
    {
        MFRC_SetBitMask(MFRC_TxControlReg, 0x03);
    }
}

void PCD_AntennaOff(void)
{
    MFRC_ClrBitMask(MFRC_TxControlReg, 0x03);
}

void PCD_Init(void)
{
    MFRC_Init();      //MFRC管脚配置
    PCD_Reset();      //PCD复位  并初始化配置
    PCD_AntennaOff(); //关闭天线
    PCD_AntennaOn();   //开启天线
}

char PCD_Request(uint8_t RequestMode, uint8_t *pCardType)
{
    int status;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    MFRC_ClrBitMask(MFRC_Status2Reg, 0x08);//关内部温度传感器
    MFRC_WriteReg(MFRC_BitFramingReg, 0x07); //存储模式，发送模式，是否启动发送等
    MFRC_SetBitMask(MFRC_TxControlReg, 0x03);//配置调制信号13.56MHZ

    CmdFrameBuf[0] = RequestMode;

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 1, CmdFrameBuf, &unLen);

    if((status == PCD_OK) && (unLen == 0x10))
    {
        *pCardType = CmdFrameBuf[0];
        *(pCardType + 1) = CmdFrameBuf[1];
    }

    return status;
}

char PCD_Anticoll(uint8_t *pSnr)
{
    char status;
    uint8_t i, snr_check = 0;
    uint16_t  unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    MFRC_ClrBitMask(MFRC_Status2Reg, 0x08);
    MFRC_WriteReg(MFRC_BitFramingReg, 0x00);
    MFRC_ClrBitMask(MFRC_CollReg, 0x80);

    CmdFrameBuf[0] = PICC_ANTICOLL1;
    CmdFrameBuf[1] = 0x20;

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 2, CmdFrameBuf, &unLen);

    if(status == PCD_OK)
    {
        for(i = 0; i < 4; i++)
        {
            *(pSnr + i)  = CmdFrameBuf[i];
            snr_check ^= CmdFrameBuf[i];
        }
        if(snr_check != CmdFrameBuf[i])
        {
            status = PCD_ERR;
        }
    }

    MFRC_SetBitMask(MFRC_CollReg, 0x80);
    return status;
}

char PCD_Select(uint8_t *pSnr)
{
    char status;
    uint8_t i;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_ANTICOLL1;
    CmdFrameBuf[1] = 0x70;
    CmdFrameBuf[6] = 0;
    for(i = 0; i < 4; i++)
    {
        CmdFrameBuf[i + 2] = *(pSnr + i);
        CmdFrameBuf[6]  ^= *(pSnr + i);
    }
    MFRC_CalulateCRC(CmdFrameBuf, 7, &CmdFrameBuf[7]);

    MFRC_ClrBitMask(MFRC_Status2Reg, 0x08);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 9, CmdFrameBuf, &unLen);

    if((status == PCD_OK) && (unLen == 0x18))
    {
        status = PCD_OK;
    }
    else
    {
        status = PCD_ERR;
    }
    return status;
}

char PCD_AuthState(uint8_t AuthMode, uint8_t BlockAddr, uint8_t *pKey, uint8_t *pSnr)
{
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];
    CmdFrameBuf[0] = AuthMode;
    CmdFrameBuf[1] = BlockAddr;
    for(i = 0; i < 6; i++)
    {
        CmdFrameBuf[i + 2] = *(pKey + i);
    }
    for(i = 0; i < 4; i++)
    {
        CmdFrameBuf[i + 8] = *(pSnr + i);
    }

    status = MFRC_CmdFrame(MFRC_AUTHENT, CmdFrameBuf, 12, CmdFrameBuf, &unLen);
    if((status != PCD_OK) || (!(MFRC_ReadReg(MFRC_Status2Reg) & 0x08)))
    {
        status = PCD_ERR;
    }

    return status;
}

char PCD_WriteBlock(uint8_t BlockAddr, uint8_t *pData)
{
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_WRITE;
    CmdFrameBuf[1] = BlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

    if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
    {
        status = PCD_ERR;
    }

    if(status == PCD_OK)
    {
        for(i = 0; i < 16; i++)
        {
            CmdFrameBuf[i] = *(pData + i);
        }
        MFRC_CalulateCRC(CmdFrameBuf, 16, &CmdFrameBuf[16]);

        status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 18, CmdFrameBuf, &unLen);

        if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
        {
            status = PCD_ERR;
        }
    }

    return status;
}

char PCD_ReadBlock(uint8_t BlockAddr, uint8_t *pData)
{
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_READ;
    CmdFrameBuf[1] = BlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);
    if((status == PCD_OK) && (unLen == 0x90))
    {
        for(i = 0; i < 16; i++)
        {
            *(pData + i) = CmdFrameBuf[i];
        }
    }
    else
    {
        status = PCD_ERR;
    }

    return status;
}

char PCD_Value(uint8_t mode, uint8_t BlockAddr, uint8_t *pValue)
{
    //0XC1        1           Increment[4]={0x03, 0x01, 0x01, 0x01};
    char status;
    uint16_t unLen;
    uint8_t i, CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = mode;
    CmdFrameBuf[1] = BlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

    if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
    {
        status = PCD_ERR;
    }

    if(status == PCD_OK)
    {
        for(i = 0; i < 16; i++)
        {
            CmdFrameBuf[i] = *(pValue + i);
        }
        MFRC_CalulateCRC(CmdFrameBuf, 4, &CmdFrameBuf[4]);
        unLen = 0;
        status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 6, CmdFrameBuf, &unLen);
        if(status != PCD_ERR)
        {
            status = PCD_OK;
        }
    }

    if(status == PCD_OK)
    {
        CmdFrameBuf[0] = PICC_TRANSFER;
        CmdFrameBuf[1] = BlockAddr;
        MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

        status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

        if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
        {
            status = PCD_ERR;
        }
    }
    return status;
}

char PCD_BakValue(uint8_t sourceBlockAddr, uint8_t goalBlockAddr)
{
    char status;
    uint16_t  unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_RESTORE;
    CmdFrameBuf[1] = sourceBlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);
    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);
    if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
    {
        status = PCD_ERR;
    }

    if(status == PCD_OK)
    {
        CmdFrameBuf[0] = 0;
        CmdFrameBuf[1] = 0;
        CmdFrameBuf[2] = 0;
        CmdFrameBuf[3] = 0;
        MFRC_CalulateCRC(CmdFrameBuf, 4, &CmdFrameBuf[4]);
        status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 6, CmdFrameBuf, &unLen);
        if(status != PCD_ERR)
        {
            status = PCD_OK;
        }
    }

    if(status != PCD_OK)
    {
        return PCD_ERR;
    }

    CmdFrameBuf[0] = PICC_TRANSFER;
    CmdFrameBuf[1] = goalBlockAddr;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);
    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);
    if((status != PCD_OK) || (unLen != 4) || ((CmdFrameBuf[0] & 0x0F) != 0x0A))
    {
        status = PCD_ERR;
    }

    return status;
}

char PCD_Halt(void)
{
    char status;
    uint16_t unLen;
    uint8_t CmdFrameBuf[MFRC_MAXRLEN];

    CmdFrameBuf[0] = PICC_HALT;
    CmdFrameBuf[1] = 0;
    MFRC_CalulateCRC(CmdFrameBuf, 2, &CmdFrameBuf[2]);

    status = MFRC_CmdFrame(MFRC_TRANSCEIVE, CmdFrameBuf, 4, CmdFrameBuf, &unLen);

    return status;
}


uint8_t readUid[5]; //卡号
uint8_t CT[3]; 		//卡类型
uint8_t DATA[16];	//存放数据

uint8_t KEY_A[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t KEY_B[6]= {0xff,0xff,0xff,0xff,0xff,0xff};


uint8_t status;
uint8_t addr=0x00; // 0扇区 0块

void Cardcompare()
{
	    uint8_t i;

		status = PCD_Request(0x52, CT);						//找到卡返回0
		if(!status)  //寻卡成功
		{
				status = PCD_ERR;
				status = PCD_Anticoll(readUid);//防冲撞
		}
		
		if(!status) //防冲撞成功
		{
			status = PCD_ERR;
            printf("卡的类型为：%x%x%x\r\n",CT[0],CT[1],CT[2]); //读取卡的类型 //读取卡的类型
			printf("卡号：%x-%x-%x-%x\r\n",readUid[0],readUid[1],readUid[2],readUid[3]);
			HAL_Delay(1000);
            status=PCD_Select(readUid);
		}	
		
		if(!status) //选卡成功
		{
				status = PCD_ERR;
				// 验证A密钥 块地址 密码 SN 
				status = PCD_AuthState(0x60, addr, KEY_A, readUid);
				if(status == PCD_OK)//验证A成功
				{
						printf("A密钥验证成功\r\n");
						HAL_Delay(1000);
				}
				else
				{
						printf("A密钥验证失败\r\n");
						HAL_Delay(1000);
				}
								
				// 验证B密钥 块地址 密码 SN  
				status = PCD_AuthState(0x61, addr, KEY_B, readUid);
				if(status == PCD_OK)//验证B成功
				{
						printf("B密钥验证成功\r\n");
				}
				else
				{
						printf("B密钥验证失败\r\n");					
				}
				HAL_Delay(1000);
		}	
		
		if(status == PCD_OK)//验证密码成功，接着读取0块
		{
            status = PCD_ERR;
            status = PCD_ReadBlock(addr, DATA); 
            if(status == PCD_OK)//读卡成功
            {		
                printf("0扇区0块DATA:");
                for(i = 0; i < 16; i++)
                {
                    printf("%02x", DATA[i]);
                }
                printf("\r\n");	
            }
            else
            {
                printf("读卡失败\r\n");
            }
                    HAL_Delay(1000);
		}
		
}

