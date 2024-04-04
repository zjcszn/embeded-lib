/**********************************************************************/
/*                                                                    */
/*   Copyright (c) 2010 Shanghai FuDan MicroElectronic Inc, Ltd.      */
/*   All rights reserved. Licensed Software Material.                 */
/*                                                                    */
/*   Unauthorized use, duplication, or distribution is strictly       */
/*   prohibited by law.                                               */
/*                                                                    */
/**********************************************************************/
#include "DEVICE_CFG.h"
#include <string.h>

void Uart_Send_Msg( char *msg)
{ 
	UART_Com3_Para.Send_Len = strlen(msg);
	if(UART_Com3_Para.Send_Len == 0)
		return;
	UART_Com3_Para.buff = msg;
	UART_Com3_Para.Send_Index = 0;
	UART_Com3_Para.SendStatus = False;   
	USART3->DR =(u16)UART_Com3_Para.buff [0];//开始发送
	while(UART_Com3_Para.SendStatus == False);
	return;
}

void Uart_Rece_Msg(unsigned char rece_len,char *msg)
{
	UART_Com3_Para.Recv_Index = 0;
	UART_Com3_Para.Recv_Len = rece_len;
	UART_Com3_Para.buff = msg;
	UART_Com3_Para.RecvStatus = False;
	while(UART_Com3_Para.RecvStatus == False);
	return;
}

Bool Scan_User_Reply(void)
{
	char user_reply;
	
	while(1)
	{		
	Uart_Rece_Msg(1,&user_reply);	
	if((user_reply == 'N')||(user_reply == 'n'))
	{
		Uart_Send_Msg("<- N\r\n");
		return False;
	}
	if((user_reply == 'Y')||(user_reply == 'y'))
	{
		Uart_Send_Msg("<- Y\r\n");
		return True;
	}
	}
}

void Uart_Send_Hex(unsigned char *input_byte,unsigned int input_len)
{
char buff[2];
unsigned int i;	
	
	UART_Com3_Para.buff = buff;
	for(i=0;i<input_len;i++)
	{
		UART_Com3_Para.buff[0] =(*(input_byte+i) & 0xF0)>>4;
		if (UART_Com3_Para.buff[0]<0x0A)
			UART_Com3_Para.buff[0] +=0x30;
		else
			UART_Com3_Para.buff[0] +=0x37;
		
		UART_Com3_Para.buff[1]=(*(input_byte+i) & 0x0F);
		if (UART_Com3_Para.buff[1]<0x0A)
			UART_Com3_Para.buff[1] +=0x30;
		else
			UART_Com3_Para.buff[1] +=0x37;
		
		UART_Com3_Para.SendStatus = False; 
		UART_Com3_Para.Send_Len = 2;
		UART_Com3_Para.Send_Index = 0;
		USART3->DR =(u16)UART_Com3_Para.buff [0];//开始发送
		while( UART_Com3_Para.SendStatus==0);		
	}
	return;
}

void UART_Com_Para_Init(UART_Com_Para_Def* UART_Com_Para)
{	
		UART_Com_Para->Send_Len = 0;
		UART_Com_Para->Send_Index = 0;
		UART_Com_Para->Recv_Len = 0;
		UART_Com_Para->Recv_Index = 0; 
		UART_Com_Para->RecvStatus = False;			
		UART_Com_Para->SendStatus= False;
}

