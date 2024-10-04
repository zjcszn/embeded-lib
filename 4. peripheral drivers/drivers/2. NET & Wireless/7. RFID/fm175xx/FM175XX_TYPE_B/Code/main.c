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
#include "type_b.h"
#include "type_b_app.h"
#include <string.h>
void main()
{
	Io_Init();
	Timer_Uart_Init();
	SPI_Init();
	Uart_Send_Msg("FM17550 Card TYPE B Reader\r\n");
	Uart_Send_Msg("Version 2.0 2014.7.10\r\n");
	FM175XX_SoftReset();
	Uart_Send_Msg("-> Reset OK\r\n");
	
	Pcd_ConfigISOType(1);//选择TYPE B模式
	while(1)
	{

		TYPE_B_APP();
		
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




