/*********************************************************************
*                                                                    *
*   Copyright (c) 2010 Shanghai FuDan MicroElectronic Inc, Ltd.      *
*   All rights reserved. Licensed Software Material.                 *
*                                                                    *
*   Unauthorized use, duplication, or distribution is strictly       *
*   prohibited by law.                                               *
*                                                                    *
*********************************************************************/

/********************************************************************/
/* 	主要功能:						        																		*/
/* 	    TYPE A 和 TYPE B卡片Polling操作								        			*/
/*      支持身份证UID读取														      					*/
/* 	编制:宋耀海 																										*/
/* 	编制时间:2017年9月7日																						*/
/* 																																	*/
/********************************************************************/
#include "string.h"
#include "DEVICE_CFG.h"
#include "FM175XX_REG.h"
#include "READER_API.h"
#include "FM175XX.h"
//#include "MIFARE.h"
#include "UART.h"
struct type_a_struct Type_A; 
struct type_b_struct Type_B;

void SetTxCRC(unsigned char mode)
{
	if(mode)
		ModifyReg(JREG_TXMODE,BIT7,SET);
	else
		ModifyReg(JREG_TXMODE,BIT7,RESET);
	return;
}

void SetRxCRC(unsigned char mode)
{
	if(mode)
		ModifyReg(JREG_RXMODE,BIT7,SET);
	else
		ModifyReg(JREG_RXMODE,BIT7,RESET);
	return;
}


unsigned char SetTimeOut(unsigned int microseconds)
{
	unsigned long  timereload;
	unsigned int  prescaler;

	prescaler = 0;
	timereload = 0;
	while(prescaler < 0xfff)
	{
		timereload = ((microseconds*(long)13560)-1)/(prescaler * 2 + 1);
		
		if( timereload < 0xffff)
			break;
		prescaler++;
	}
		timereload = timereload & 0xFFFF;
	  	ModifyReg(JREG_TMODE,0x0F,RESET);
		ModifyReg(JREG_TMODE,prescaler >> 8,SET);
		SetReg(JREG_TPRESCALER,prescaler & 0xFF);					
		SetReg(JREG_TRELOADHI,timereload >> 8);
		SetReg(JREG_TRELOADLO,timereload & 0xFF);
    return FM175XX_SUCCESS;
}

/******************************************************************************/
//函数名：Command_Execute
//入口参数：
//			unsigned char* sendbuf:发送数据缓冲区	unsigned char sendlen:发送数据长度
//			unsigned char* recvbuf:接收数据缓冲区	unsigned char* recvlen:接收到的数据长度
//出口参数
/******************************************************************************/
unsigned char Command_Execute(command_struct *comm_status)
{
	unsigned char reg_data,send_length,receive_length,send_finish;
	unsigned char irq;
	unsigned char result;
	send_length = comm_status->nBytesToSend;
	receive_length = 0;
	send_finish = 0;//发送完成标志
	comm_status->nBitsReceived = 0;
	comm_status->nBytesReceived = 0;
	comm_status->CollPos = 0;
	comm_status->Error = 0;
	
	SetReg(JREG_COMMAND,CMD_IDLE);
	Clear_FIFO();	
	SetReg(JREG_WATERLEVEL,0x20);//waterlevel = 32字节
	SetReg(JREG_DIVIEN,0x00);
	SetReg(JREG_COMMIEN,0x80);	
	SetReg(JREG_COMMIRQ,0x7F);	
	SetReg(JREG_TMODE,0x80);			
	
	if((comm_status->Cmd == CMD_TRANSCEIVE)||(comm_status->Cmd == CMD_TRANSMIT))
		{		
		SetReg(JREG_COMMAND,comm_status->Cmd);			
		}
		
	if(comm_status->Cmd == CMD_AUTHENT)
		{	
		SPI_Write_FIFO(send_length,comm_status->pSendBuf);
		send_length = 0;
		SetReg(JREG_BITFRAMING,BIT7 | comm_status->nBitsToSend);
		SetReg(JREG_COMMAND,comm_status->Cmd);	
		}				
		
	while(1)
		{			
				GetReg(JREG_COMMIRQ,&irq);
				if(irq & BIT0)//TimerIRq = 1
				{
					SetReg(JREG_COMMIRQ,BIT0);	
					result = FM175XX_TIMER_ERR;		
					break;
				}
				
				if(irq & BIT1)//ERRIRq = 1
				{			
					GetReg(JREG_ERROR,&reg_data);	
					comm_status->Error = reg_data;
					
					if(comm_status->Error & BIT3)
					{
						GetReg(JREG_COLL,&reg_data);	
						comm_status->CollPos = reg_data & 0x1F;//读取冲突位置
						result = FM175XX_COLL_ERR;
						break;	
					}
				
					result = FM175XX_COMM_ERR;			
					SetReg(JREG_COMMIRQ,BIT1);			
					break;	
				}
				
				if(irq & BIT2)//LoAlterIRq = 1
				{
					if(send_length > 0)
						{
								
						if(send_length > 32)
						{			
							SPI_Write_FIFO(32,comm_status->pSendBuf);	
							comm_status->pSendBuf = comm_status->pSendBuf + 32;
							send_length = send_length - 32;
						}
						else
						{					
							SPI_Write_FIFO(send_length,comm_status->pSendBuf);							
							send_length = 0;						
						}					
						SetReg(JREG_BITFRAMING,BIT7 | comm_status->nBitsToSend);
						}
					SetReg(JREG_COMMIRQ,BIT2);
				}
				
				if(irq & BIT3)//HiAlterIRq = 1
				{
					if(send_finish == 1)//确认发送完成
					{
					SPI_Read_FIFO(32,comm_status->pReceiveBuf + comm_status->nBytesReceived);
					comm_status->nBytesReceived = comm_status->nBytesReceived + 32;
					}
					SetReg(JREG_COMMIRQ,BIT3);
				}
				
				if((irq & BIT4)&&(comm_status->Cmd == CMD_AUTHENT))//IdleTRq = 1
				{
					SetReg(JREG_COMMIRQ,BIT4);			
					result = FM175XX_SUCCESS;
					break;	
				}				
				
				if((irq & BIT5)&&((comm_status->Cmd == CMD_TRANSCEIVE)||(comm_status->Cmd == CMD_RECEIVE)))//RxIRq = 1
				{					
					GetReg(JREG_CONTROL, &reg_data);
					comm_status->nBitsReceived = reg_data & 0x07;//接收位长度				
					GetReg(JREG_FIFOLEVEL,&reg_data);
					receive_length = reg_data & 0x7F;//接收字节长度														
					SPI_Read_FIFO(receive_length, comm_status->pReceiveBuf + comm_status->nBytesReceived);
					comm_status->nBytesReceived = comm_status->nBytesReceived + receive_length; 
					if((comm_status->nBytesToReceive != comm_status->nBytesReceived)&&(comm_status->nBytesToReceive != 0))
							{
							result = FM175XX_COMM_ERR;//接收到的数据长度错误
							break;
							}					
					SetReg(JREG_COMMIRQ,BIT5);			
					result = FM175XX_SUCCESS;					
					break;						
				}
				
				if(irq & BIT6)//TxIRq = 1
				{
					SetReg(JREG_COMMIRQ,BIT6);
					if(comm_status->Cmd == CMD_TRANSMIT)
						{
						result = FM175XX_SUCCESS;
						break;					
						}	
					if(comm_status->Cmd == CMD_TRANSCEIVE)
						send_finish = 1;//发送完成
				}
				
		}
	
	ModifyReg(JREG_BITFRAMING,BIT7,RESET);//CLR Start Send	
	SetReg(JREG_COMMAND,CMD_IDLE);
	return result;
}

void FM175XX_Initial_ReaderA(void)
{
	
	SetReg(JREG_TXMODE,0x00);
	SetReg(JREG_RXMODE,0x00);	
	SetReg(JREG_MODWIDTH,MODWIDTH);				//MODWIDTH 106kbps
	SetReg(JREG_GSN,(GSNON<<4)|MODGSNON);	
	SetReg(JREG_CWGSP,GSP);
	SetReg(JREG_CONTROL,0x10);		//Initiator = 1
	SetReg(JREG_RFCFG,RXGAIN<<4);
	SetReg(JREG_RXTRESHOLD,COLLLEVEL|(MINLEVEL<<4));
	ModifyReg(JREG_TXAUTO,BIT6,SET);//Force 100ASK = 1
//	Mifare_Clear_Crypto();//循环读卡操作时，需要在读卡前清除前次操作的认证标志
	return;
}


void FM175XX_Initial_ReaderB(void)
{
	ModifyReg(JREG_STATUS2,BIT3,RESET);
	SetReg(JREG_TXMODE,BIT7|BIT1|BIT0);//BIT1~0 = 11 :ISO/IEC 14443B
	SetReg(JREG_RXMODE,BIT7|BIT1|BIT0);//BIT1~0 = 11 :ISO/IEC 14443B
	SetReg(JREG_TXAUTO,0);
	SetReg(JREG_MODWIDTH,0x26);
	SetReg(JREG_RXTRESHOLD,0x55);
	SetReg(JREG_GSN,0xF8);  
	SetReg(JREG_CWGSP,0x3F);
	SetReg(JREG_MODGSP,0x20);
	SetReg(JREG_CONTROL,0x10);
	SetReg(JREG_RFCFG,0x48);//接收器放大倍数
	return;
}


unsigned char ReaderA_Halt(void)
{
	unsigned char result;
	unsigned char outbuf[2],inbuf[1];
	command_struct command_stauts;
	command_stauts.pSendBuf = outbuf;
	command_stauts.pReceiveBuf = inbuf;	

	SetTxCRC(SET);
	SetRxCRC(SET);
	command_stauts.pSendBuf[0] = 0x50;
  	command_stauts.pSendBuf[1] = 0x00;
  	command_stauts.nBytesToSend = 2;
	command_stauts.nBitsToSend = 0;
	command_stauts.nBytesToReceive = 0;
  	SetTimeOut(5);//5ms	
	command_stauts.Cmd = CMD_TRANSMIT;	
  result = Command_Execute(&command_stauts);
  return result;
}

unsigned char ReaderA_Wakeup(void)
{
	unsigned char result;
	unsigned char outbuf[1],inbuf[2];
	command_struct command_stauts;
	command_stauts.pSendBuf = outbuf;
	command_stauts.pReceiveBuf = inbuf;	
 	SetTxCRC(RESET);
	SetRxCRC(RESET);
	command_stauts.pSendBuf[0] = RF_CMD_WUPA;   //
	command_stauts.nBytesToSend = 1;//
	command_stauts.nBitsToSend = 7;
	command_stauts.nBytesToReceive = 2;
	SetTimeOut(1);//1ms	
	command_stauts.Cmd = CMD_TRANSCEIVE;
	result = Command_Execute(&command_stauts);
	if((result == FM175XX_SUCCESS)&&(command_stauts.nBytesReceived == 2))
	{
		memcpy(Type_A.ATQA,command_stauts.pReceiveBuf,2);		
	}
	else
		result =  FM175XX_COMM_ERR;
	return result;
}

unsigned char ReaderA_Request(void)
{
	unsigned char result;
	unsigned char outbuf[1],inbuf[2];
	command_struct command_stauts;
	command_stauts.pSendBuf = outbuf;
	command_stauts.pReceiveBuf = inbuf;
 	SetTxCRC(RESET);
	SetRxCRC(RESET);
	command_stauts.pSendBuf[0] = RF_CMD_REQA;   //
	command_stauts.nBytesToSend = 1;//
	command_stauts.nBitsToSend = 7;
	command_stauts.nBytesToReceive = 2;
	SetTimeOut(1);//1ms	
	command_stauts.Cmd = CMD_TRANSCEIVE;
	result = Command_Execute(&command_stauts);
	if((result == FM175XX_SUCCESS)&&(command_stauts.nBytesReceived == 2))
	{
		memcpy(Type_A.ATQA, command_stauts.pReceiveBuf,2);		
	}
	else
		result =  FM175XX_COMM_ERR;
	return result;
}

//*************************************
//函数  名：ReaderA_AntiColl
//入口参数：size:防冲突等级，包括0、1、2
//出口参数：unsigned char:0:OK  others：错误
//*************************************
unsigned char ReaderA_AntiColl(unsigned char cascade_level)
{
	unsigned char result;
	unsigned char outbuf[2],inbuf[5];
	command_struct command_stauts;
	command_stauts.pSendBuf = outbuf;
	command_stauts.pReceiveBuf = inbuf;	
	SetTxCRC(RESET);
	SetRxCRC(RESET);
	if(cascade_level >2)
		return FM175XX_PARAM_ERR;
	command_stauts.pSendBuf[0] = RF_CMD_ANTICOL[cascade_level];   //防冲突等级
	command_stauts.pSendBuf[1] = 0x20;
	command_stauts.nBytesToSend = 2;						//发送长度：2
	command_stauts.nBitsToSend = 0;
	command_stauts.nBytesToReceive = 5;
	SetTimeOut(1);//1ms	
	command_stauts.Cmd = CMD_TRANSCEIVE;
	result = Command_Execute(&command_stauts);
	ModifyReg(JREG_COLL,BIT7,SET);
	if((result == FM175XX_SUCCESS)&&(command_stauts.nBytesReceived == 5))
	{
		memcpy(Type_A.UID +(cascade_level*4),command_stauts.pReceiveBuf,4);
		memcpy(Type_A.BCC + cascade_level,command_stauts.pReceiveBuf + 4,1);
		if((Type_A.UID[cascade_level*4] ^  Type_A.UID[cascade_level*4 + 1] ^ Type_A.UID[cascade_level*4 + 2]^ Type_A.UID[cascade_level*4 + 3] ^ Type_A.BCC[cascade_level]) !=0)
			result = FM175XX_COMM_ERR;
	}
	return result;
}

//*************************************
//函数  名：ReaderA_Select
//入口参数：size:防冲突等级，包括0、1、2
//出口参数：unsigned char:0:OK  others：错误
//*************************************
unsigned char ReaderA_Select(unsigned char cascade_level)
{
	unsigned char result;
	unsigned char outbuf[7],inbuf[1];
	command_struct command_stauts;
	command_stauts.pSendBuf = outbuf;
	command_stauts.pReceiveBuf = inbuf;	
	SetTxCRC(SET);
	SetRxCRC(SET);
	if(cascade_level > 2)							 //最多三重防冲突
		return FM175XX_PARAM_ERR;
	*(command_stauts.pSendBuf+0) = RF_CMD_ANTICOL[cascade_level];   //防冲突等级
	*(command_stauts.pSendBuf+1) = 0x70;   					//
	*(command_stauts.pSendBuf+2) = Type_A.UID[4*cascade_level];
	*(command_stauts.pSendBuf+3) = Type_A.UID[4*cascade_level+1];
	*(command_stauts.pSendBuf+4) = Type_A.UID[4*cascade_level+2];
	*(command_stauts.pSendBuf+5) = Type_A.UID[4*cascade_level+3];
	*(command_stauts.pSendBuf+6) = Type_A.BCC[cascade_level];
	command_stauts.nBytesToSend = 7;
	command_stauts.nBitsToSend = 0;
	command_stauts.nBytesToReceive = 1;
	SetTimeOut(1);//1ms	
	command_stauts.Cmd = CMD_TRANSCEIVE;
	result = Command_Execute(&command_stauts);
	if((result == FM175XX_SUCCESS)&&(command_stauts.nBytesReceived == 1))
		Type_A.SAK [cascade_level] = *(command_stauts.pReceiveBuf);
	else
		result = FM175XX_COMM_ERR;
	return result;
}



void SetRf(unsigned char mode)
{
	if(mode == 0)	
		{
		ModifyReg(JREG_TXCONTROL,BIT0|BIT1,RESET);
		}
	if(mode == 1)	
		{
		ModifyReg(JREG_TXCONTROL,BIT0,SET);
		ModifyReg(JREG_TXCONTROL,BIT1,RESET);	
		}
	if(mode == 2)	
		{
		ModifyReg(JREG_TXCONTROL,BIT0,RESET);
		ModifyReg(JREG_TXCONTROL,BIT1,SET);	
		}
	if(mode == 3)	
		{
		ModifyReg(JREG_TXCONTROL,BIT0|BIT1,SET);
		}
		mDelay(10);
}

unsigned char ReaderA_CardActivate(void)
{
unsigned char  result,cascade_level;	
//		result = ReaderA_Wakeup();//
//		if (result != FM175XX_SUCCESS)
//			return FM175XX_COMM_ERR;
				
			if 	((Type_A.ATQA[0]&0xC0)==0x00)	//1重UID		
				Type_A.CASCADE_LEVEL = 1;
			
			if 	((Type_A.ATQA[0]&0xC0)==0x40)	//2重UID			
				Type_A.CASCADE_LEVEL = 2;		

			if 	((Type_A.ATQA[0]&0xC0)==0x80)	//3重UID
				Type_A.CASCADE_LEVEL = 3;				
			
			for (cascade_level = 0;cascade_level < Type_A.CASCADE_LEVEL;cascade_level++)
				{
					result = ReaderA_AntiColl(cascade_level);//
					if (result != FM175XX_SUCCESS)
						return FM175XX_COMM_ERR;
					
					result=ReaderA_Select(cascade_level);//
					if (result != FM175XX_SUCCESS)
						return FM175XX_COMM_ERR;
				}						
			
		return result;
}

unsigned char ReaderB_Wakeup(void)
{
	unsigned char result;
	unsigned char outbuf[3],inbuf[12];
	command_struct command_stauts;
	command_stauts.pSendBuf = outbuf;
	command_stauts.pReceiveBuf = inbuf;	

 	SetTxCRC(SET);
	SetRxCRC(SET);
	command_stauts.pSendBuf[0] = 0x05;  //APf
	command_stauts.pSendBuf[1] = 0x00;   //AFI (00:for all cards)
	command_stauts.pSendBuf[2] = 0x08;   //PARAM(REQB,Number of slots =0)
	command_stauts.nBytesToSend = 3;//
	command_stauts.nBitsToSend = 0;
	command_stauts.nBytesToReceive = 12;
	SetTimeOut(1);//1ms	
	command_stauts.Cmd = CMD_TRANSCEIVE;
	result = Command_Execute(&command_stauts);
	if((result == FM175XX_SUCCESS)&&(command_stauts.nBytesReceived == 12))
	{
		memcpy(Type_B.ATQB, command_stauts.pReceiveBuf,12);	
		memcpy(Type_B.PUPI,Type_B.ATQB + 1,4);
		memcpy(Type_B.APPLICATION_DATA,Type_B.ATQB + 6,4);
		memcpy(Type_B.PROTOCOL_INF,Type_B.ATQB + 10,3);
	}
	else
		result = FM175XX_COMM_ERR;
	return result;
}

unsigned char ReaderB_Request(void)
{
	unsigned char result;
	unsigned char outbuf[3],inbuf[12];
	command_struct command_stauts;
	command_stauts.pSendBuf = outbuf;
	command_stauts.pReceiveBuf = inbuf;
 	SetTxCRC(SET);
	SetRxCRC(SET);
	command_stauts.pSendBuf[0] = 0x05;  //APf
	command_stauts.pSendBuf[1] = 0x00;   //AFI (00:for all cards)
	command_stauts.pSendBuf[2] = 0x00;   //PARAM(REQB,Number of slots =0)
	command_stauts.nBytesToSend = 3;//
	command_stauts.nBitsToSend = 0;
	command_stauts.nBytesToReceive = 12;
	SetTimeOut(1);//1ms	
	command_stauts.Cmd = CMD_TRANSCEIVE;
	result = Command_Execute(&command_stauts);
	if((result == FM175XX_SUCCESS)&&(command_stauts.nBytesReceived == 12))
	{
		memcpy(Type_B.ATQB, command_stauts.pReceiveBuf,12);	
		memcpy(Type_B.PUPI,Type_B.ATQB + 1,4);
		memcpy(Type_B.APPLICATION_DATA,Type_B.ATQB + 6,4);
		memcpy(Type_B.PROTOCOL_INF,Type_B.ATQB + 10,3);
	}
	else
		result = FM175XX_COMM_ERR;
	return result;
}

unsigned char ReaderB_Attrib(void)
{
	unsigned char result;
	unsigned char outbuf[9],inbuf[1];
	command_struct command_stauts;
	command_stauts.pSendBuf = outbuf;
	command_stauts.pReceiveBuf = inbuf;
 	SetTxCRC(SET);
	SetRxCRC(SET);
	command_stauts.pSendBuf[0] = 0x1D;  //
	command_stauts.pSendBuf[1] = Type_B.PUPI[0];   //
	command_stauts.pSendBuf[2] = Type_B.PUPI[1];   //
	command_stauts.pSendBuf[3] = Type_B.PUPI[2];   //
	command_stauts.pSendBuf[4] = Type_B.PUPI[3];   //	
	command_stauts.pSendBuf[5] = 0x00;  //Param1
	command_stauts.pSendBuf[6] = 0x08;  //01--24,08--256
	command_stauts.pSendBuf[7] = 0x01;  //COMPATIBLE WITH 14443-4
	command_stauts.pSendBuf[8] = 0x01;  //CID:01 
	command_stauts.nBytesToSend = 9;//
	command_stauts.nBitsToSend = 0;
	command_stauts.nBytesToReceive = 1;
	SetTimeOut(1);//1ms	
	command_stauts.Cmd = CMD_TRANSCEIVE;
	result = Command_Execute(&command_stauts);
	if(result == FM175XX_SUCCESS)
	{
		Type_B.LEN_ATTRIB = command_stauts.nBytesReceived;
		memcpy(Type_B.ATTRIB, command_stauts.pReceiveBuf,Type_B.LEN_ATTRIB);		
	}
	else
		result = FM175XX_COMM_ERR;
	return result;
}

unsigned char ReaderB_GetUID(void)
{
	unsigned char result;
	unsigned char outbuf[5],inbuf[10];
	command_struct command_stauts;
	command_stauts.pSendBuf = outbuf;
	command_stauts.pReceiveBuf = inbuf;
 	SetTxCRC(SET);
	SetRxCRC(SET);
	command_stauts.pSendBuf[0] = 0x00;  //
	command_stauts.pSendBuf[1] = 0x36;   //
	command_stauts.pSendBuf[2] = 0x00;   //
	command_stauts.pSendBuf[3] = 0x00;   //
	command_stauts.pSendBuf[4] = 0x08;   //	
	
	command_stauts.nBytesToSend = 5;//
	command_stauts.nBitsToSend = 0;
	command_stauts.nBytesToReceive = 10;
	SetTimeOut(1);//1ms	
	command_stauts.Cmd = CMD_TRANSCEIVE;
	result = Command_Execute(&command_stauts);
	if(result == FM175XX_SUCCESS)
	{
		//Type_B.LEN_ATTRIB = command_stauts.nBytesReceived;
		memcpy(Type_B.UID, command_stauts.pReceiveBuf,8);		
	
	}
	else
		result = FM175XX_COMM_ERR;
	return result;
}

unsigned char FM175XX_Polling(unsigned char *polling_card)
{
unsigned char result;
	*polling_card = 0;
	FM175XX_Initial_ReaderA();
	result = ReaderA_Wakeup();//
		if (result == FM175XX_SUCCESS)
		{
			ReaderA_CardActivate();
			*polling_card |= BIT0;
		}
	FM175XX_Initial_ReaderB();
	result = ReaderB_Wakeup();//
		if (result == FM175XX_SUCCESS)
		{
			ReaderB_Attrib();
			ReaderB_GetUID();
			*polling_card |= BIT1;
		}
if (*polling_card != 0)		
	return SUCCESS;	
else
	return ERROR;
}

