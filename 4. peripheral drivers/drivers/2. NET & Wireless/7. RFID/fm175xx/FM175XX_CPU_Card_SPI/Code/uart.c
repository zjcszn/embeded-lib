/*************************************************************/
//2014.03.06修改版
/*************************************************************/
//串口收发程序

#include "STC15F2K60S2.h"
#include "intrins.h"
#include "pin.h"
#include "uart.h"
#include <string.h>
#define UART_BAUD 115200	 //115200bps
#define ETU_WIDTH 97 //103us
#define SYS_CLK 33177600 //33Mhz

// unsigned char data timer_L,timer_H;

// unsigned char xdata uart_buffer[255];	//串口缓冲区
 unsigned char data uart_data_length;		//串口数据长度
 unsigned char data uart_point;		//数据指针
 unsigned char data uart_verify;			//串口数据校验
 bit data uart_receive_finish_flag;		//串口完成接收一帧有效数据标志位

 bit data uart_send_finish_flag;			//串口完成发送一帧数据标志位
 bit data uart_receive_error_flag;		//串口接收出错标志
 bit data uart_send_error_flag;			//串口发送出错标志

/*********************************************/
/*函数名：	    timer_uart_init
/*功能：	    时钟与串口初始化
				初始化串口0为19200BPS
				1个停止位，8个数据位，无校验位
				接收中断允许
/*输入参数：	无
/*返回值：	    无
/*********************************************/
void Timer_Uart_Init()
{
	SCON=0x50;				    				//SM0:0 SM1:1 SM2:0 REN:1 TB8:0 RB8:0 TI:0 RI:0
	PCON=0x00;									//SMOD=0  
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设定定时器1为16位自动重装方式

	TL1=(65536-((long)SYS_CLK/4/(long)UART_BAUD));	  
	TH1=(65536-((long)SYS_CLK/4/(long)UART_BAUD))>>8;

	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1

	INT1=1;//配置中断脚为高电平
	IT1=1;//仅触发下降沿
	EX1=1;//中断使能

	ES=1;					    				//开串口0中断
	EA=1;					    				//开总中断
	return;
}

/********************************/
/*函数名：	    uart_int
/*功能：	    串口中断程序
/*输入参数：	无
/*返回值：	    无
/*******************************/
void Uart_Int() interrupt 4
{

	if(RI==1)                                           		//串口接收中断
	{	
//		if (SBUF != '#')
//		{
//			uart_buffer[uart_point]=SBUF;
//			uart_point++;
//		}
//		else
//		{
//			uart_receive_finish_flag=1;
//		}
		RI=0;
	}

	if(TI==1)                                           		//串口发送中断
	{
		TI=0;
		uart_send_finish_flag=1;
	}
	return;
}




//void Timer0_Int(void) interrupt 1
//{
// 	TH0=timer_H;
//	TL0=timer_L;
//	return;
//}

void Uart_Send_Msg(unsigned char *msg)
{ 
	unsigned int data i;
	unsigned int data msg_len;
	msg_len= strlen(msg);
//	REN=0;
 	for (i=0;i<msg_len;i++)
	{
		SBUF=msg[i];//
		while(uart_send_finish_flag==0);
			uart_send_finish_flag=0;
	}
//	REN=1;
	return;
}

//void Uart_Rece_Msg(unsigned char recr_len)
//{
//	uart_point=0;
//	while(uart_point!=recr_len);
//		uart_data_length=uart_point;
//		uart_point=0;
//	return;
//}

void Uart_Send_Hex(unsigned char *input_byte,unsigned int input_len)
{
unsigned char data temp_byte;
unsigned int data i;

	for(i=0;i<input_len;i++)
	{
		temp_byte=(*(input_byte+i) & 0xF0)>>4;
		if (temp_byte<0x0A)
			SBUF=0x30+temp_byte;
		else
			SBUF=0x37+temp_byte;

		while(uart_send_finish_flag==0);
			uart_send_finish_flag=0;
	
		temp_byte=(*(input_byte+i) & 0x0F);
		if (temp_byte<0x0A)
			SBUF=0x30+temp_byte;
		else
		    SBUF=0x37+temp_byte;
		while(uart_send_finish_flag==0);
			uart_send_finish_flag=0;
	}
	return;
}


//void Hex_to_BCD(unsigned long hex_input,unsigned char *BCD_output)
//{
//		BCD_output[0] = hex_input / (long)100000;
//		hex_input = hex_input - (BCD_output[0]* (long)100000);
//		BCD_output[1] = hex_input / (long)10000;
//		hex_input = hex_input - (BCD_output[1]* (long)10000);
//		BCD_output[2] = hex_input / (long)1000;
//		hex_input = hex_input -(BCD_output[2] *(long) 1000);
//		BCD_output[3] = hex_input / 100;
//		hex_input = hex_input - (BCD_output[3] * 100);
//		BCD_output[4] = hex_input / 10;
//		hex_input = hex_input - (BCD_output[4] * 10);
//		BCD_output[5] = hex_input / 1;
//		return;
//}