#include "cpu_card.h"
#include "uart.h"
#include "main.h"
#include "des.h"
#include "type_a.h"
#include "fm175xx.h"
#include <string.h>
#define APDU_Debug 0

unsigned char CPU_Send_Buff[255],CPU_Rece_Buff[255],CPU_Send_Len,CPU_Rece_Len;

unsigned char APDU_Exchange(unsigned char send_len,unsigned char *send_buff,unsigned char *rece_len,unsigned char *rece_buff)
	{
	unsigned char data i,result,tpdu_rx_len;
	*rece_len=0;

	#if APDU_Debug
	Uart_Send_Msg("<- CPU_CARD.PCB = ");Uart_Send_Hex(&CPU_CARD.PCB,1);Uart_Send_Msg("\r\n");
	Uart_Send_Msg("<- CPU_CARD.CID = ");Uart_Send_Hex(&CPU_CARD.CID,1);Uart_Send_Msg("\r\n");
	Uart_Send_Msg("<- CPU_CARD.FSD = ");Uart_Send_Hex(&CPU_CARD.FSD,1);Uart_Send_Msg("\r\n");

	#endif 

	if(send_len>CPU_CARD.FSD)//发送长度不能超过卡片ATS中定义的最大接收帧长度
		{
		Uart_Send_Msg("<- APDU Send Length Overflow\r\n");
		return ERROR;
 		}

	result=	CPU_I_Block(send_len,send_buff,&(*rece_len),rece_buff);

	#if APDU_Debug
		Uart_Send_Msg("<- result = ");Uart_Send_Hex(&result,1);Uart_Send_Msg("\r\n");
		Uart_Send_Msg("<- rece_buff = ");Uart_Send_Hex(rece_buff,*rece_len);Uart_Send_Msg("\r\n");
	#endif

	if (result==OK)
		{
			while(1)
				{
					if(( CPU_CARD.PCB&0xF0)==0xF0)//WTX
					{
						result=CPU_WTX(CPU_CARD.WTXM,&tpdu_rx_len,rece_buff);
						if(result!=OK)
							return ERROR;
						CPU_CARD.PCB=*rece_buff;
						if(( CPU_CARD.PCB&0xF0)==0xF0)//WTX
							{
							if( CPU_CARD.PCB&0x08)
								CPU_CARD.WTXM=*(rece_buff+2);
							else
								CPU_CARD.WTXM=*(rece_buff+1);
							}
						*rece_len=tpdu_rx_len;
					}

					if((CPU_CARD.PCB&0xF0)==0x10)//R_Block
					{
						CPU_PCB_CONVER();
						result=CPU_R_Block(&tpdu_rx_len,rece_buff+(*rece_len));
						if(result!=OK)
							return ERROR;
						 CPU_CARD.PCB=*(rece_buff+(*rece_len));
						if( CPU_CARD.PCB&0x08)
								{
									for (i=0;i<tpdu_rx_len-2;i++)
										*(rece_buff+i+(*rece_len))=*(rece_buff+i+(*rece_len)+2);
									*rece_len=*rece_len+tpdu_rx_len-2;
								}
								else
								{
									for (i=0;i<tpdu_rx_len-1;i++)
										*(rece_buff+i+(*rece_len))=*(rece_buff+i+(*rece_len)+1);
									*rece_len=*rece_len+tpdu_rx_len-1;
								}
					}

					if(( CPU_CARD.PCB&0xF0)==0x00)//I_Block
					{
							if(result==OK)
							{
							CPU_PCB_CONVER();
							if( CPU_CARD.PCB&0x08)//cid存在
								{
								*rece_len=*rece_len-2;
								memmove(rece_buff,rece_buff+2,*rece_len);
								}
							else
								{
								*rece_len=*rece_len-1;
								memmove(rece_buff,rece_buff+1,*rece_len);
								}
							return OK;
							}
							else
							return ERROR;
					}
				}
				
		}

	return ERROR;
	}

unsigned char CPU_APP(void)
{
unsigned char result;

		result = CPU_Rats(0x02,0x50,&CPU_Rece_Len,CPU_Rece_Buff);//CPU卡片RATS操作
		if(result != OK)
			{
			Uart_Send_Msg("-> Card RATS ERROR\r\n");
		
			return ERROR;	
			}
		Uart_Send_Msg("-> Card RATS OK\r\n");
		Uart_Send_Msg("<- Ats = ");Uart_Send_Hex(CPU_Rece_Buff,CPU_Rece_Len);Uart_Send_Msg("\r\n");
			
		CPU_Send_Len=7;
		memcpy(CPU_Send_Buff,"\x00\xA4\x00\x00\x02\x3F\x00",CPU_Send_Len);//选择MF文件
		result = APDU_Exchange(CPU_Send_Len,CPU_Send_Buff,&CPU_Rece_Len,CPU_Rece_Buff);
		if(result !=OK)
			{
			Uart_Send_Msg("-> Select MF ERROR\r\n");
			return ERROR;
			}
		Uart_Send_Msg("-> Select MF OK\r\n");
		Uart_Send_Msg("<- Return = ");Uart_Send_Hex(CPU_Rece_Buff,CPU_Rece_Len);Uart_Send_Msg("\r\n");

		CPU_Send_Len=20;
		memcpy(CPU_Send_Buff,"\x00\xA4\x04\x00\x0E\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44\x46\x30\x31\x00",CPU_Send_Len);//选择MF文件
		result = APDU_Exchange(CPU_Send_Len,CPU_Send_Buff,&CPU_Rece_Len,CPU_Rece_Buff);
		if(result !=OK)
			{
			Uart_Send_Msg("-> Select File 1 ERROR\r\n");
			return ERROR;
			}
		Uart_Send_Msg("-> Select File 1 OK\r\n");
		Uart_Send_Msg("<- Return = ");Uart_Send_Hex(CPU_Rece_Buff,CPU_Rece_Len);Uart_Send_Msg("\r\n");

		CPU_Send_Len=14;
		memcpy(CPU_Send_Buff,"\x00\xA4\x04\x00\x08\xA0\x00\x00\x03\x33\x01\x01\x01\x00",CPU_Send_Len);//选择MF文件
		result = APDU_Exchange(CPU_Send_Len,CPU_Send_Buff,&CPU_Rece_Len,CPU_Rece_Buff);
		if(result !=OK)
			{
			Uart_Send_Msg("-> Select File 2 ERROR\r\n");
			return ERROR;
			}
		Uart_Send_Msg("-> Select File 2 OK\r\n");
		Uart_Send_Msg("<- Return = ");Uart_Send_Hex(CPU_Rece_Buff,CPU_Rece_Len);Uart_Send_Msg("\r\n");

		CPU_Send_Len=42;
		memcpy(CPU_Send_Buff,"\x80\xA8\x00\x00\x24\x83\x22\x28\x00\x00\x80\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\x56\x00\x00\x00\x00\x00\x01\x56\x13\x08\x28\x82\x12\x34\x56\x78\x00\x00",CPU_Send_Len);//选择MF文件
		result = APDU_Exchange(CPU_Send_Len,CPU_Send_Buff,&CPU_Rece_Len,CPU_Rece_Buff);
		if(result !=OK)
			{
			Uart_Send_Msg("-> Select File 3 ERROR\r\n");
			return ERROR;
			}
		Uart_Send_Msg("-> Select File 3 OK\r\n");
		Uart_Send_Msg("<- Return = ");Uart_Send_Hex(CPU_Rece_Buff,CPU_Rece_Len);Uart_Send_Msg("\r\n");

		result = CPU_Deselect();
			if(result !=OK)
			{
			Uart_Send_Msg("-> Deselect ERROR\r\n");
			return ERROR;
			}
		Uart_Send_Msg("-> Deselect OK\r\n");
		return OK;
}