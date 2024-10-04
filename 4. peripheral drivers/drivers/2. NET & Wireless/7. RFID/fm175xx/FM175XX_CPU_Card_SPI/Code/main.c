/*************************************************************/
//2014.07.10修改版
/*************************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "pin.h"
#include "uart.h"
#include "spi.h"
#include "FM175XX.h"
#include "main.h"
#include "type_a.h"
#include "type_b.h"
#include "mifare_card.h"
#include "ultralight.h"
#include "des.h"
#include "cpu_card.h"
#include "cpu_app.h"
#include <string.h>
void main()
{
	Io_Init();
	Timer_Uart_Init();
	SPI_Init();
	Uart_Send_Msg("FM17520,FM17522,FM17550 CPU Card Reader\r\n");
	Uart_Send_Msg("Version 3.0 2015.4.18\r\n");
	if(FM175XX_HardReset()!=OK)
		{
		Uart_Send_Msg("-> FM175XX Reset ERROR\r\n");//FM175XX复位失败
		while(1);
		}
	Uart_Send_Msg("-> FM175XX Reset OK\r\n");//FM175XX复位成功
	Pcd_ConfigISOType(0);
	Set_Rf(3);   //turn on radio
	while(1)
	{
	if (TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK)
		{
		Uart_Send_Msg("---------------------\r\n");
		Uart_Send_Msg("-> Card Activate OK\r\n");
		Uart_Send_Msg("<- ATQA = ");Uart_Send_Hex(PICC_ATQA,2);Uart_Send_Msg("\r\n");
		Uart_Send_Msg("<- UID = ");Uart_Send_Hex(PICC_UID,4);Uart_Send_Msg("\r\n");
		Uart_Send_Msg("<- SAK = ");Uart_Send_Hex(PICC_SAK,1);Uart_Send_Msg("\r\n");
		if(CPU_APP()!=OK)
			{
			Set_Rf(0);
			Set_Rf(3);
			}
		}
   }
}

/*****************************/
/*函数介绍：	管脚初始化程序
/*输入参数：	无
/*输出参数：	无
/*返回值：		无
/*占用资源：	无
/*****************************/
void Io_Init()
{
	P0=0xFF;
	P1=0xFF;
	P2=0xFF;
	P3=0xFF;
	NPD=1;
	LED_2=0;
	LED_3=0;
	return;
}
/*************************/
/*函数介绍：	延时函数
/*输入参数：	delay_time
/*输出参数：	无
/*返回值：		无
/*占用资源：	无
/*************************/
void Delay_100us(unsigned int delay_time)		//0.1ms*delay_time
{
	unsigned int data i;
	for(i=0;i<delay_time;i++)
	{
		_nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 
		_nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
		_nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
		_nop_(); _nop_(); _nop_(); _nop_();	_nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
		_nop_(); _nop_(); _nop_(); _nop_();	_nop_(); _nop_(); _nop_(); _nop_();	_nop_(); _nop_();
	}
}




