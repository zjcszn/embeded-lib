/*************************************************************/
//2014.03.06修改版
/*************************************************************/
#include "fm175xx.h"
#include "type_b.h"
#include "cpu_card.h"
#include "main.h"
#include <string.h>

struct PICC_B_STR PICC_B;

/*****************************************************************************************/
/*名称：TypeB_Halt																		 */
/*功能：设置TYPE B卡片进入停止状态														 */
/*输入：card_sn：卡片的PUPI																 */
/*输出：																				 */
/*	   	OK：操作成功																	 */
/*		ERROR：操作失败																	 */
/*****************************************************************************************/
unsigned char TypeB_Halt(unsigned char *card_sn,unsigned char *rece_len,unsigned char *rece_buff)
{
	unsigned char result,send_buff[5];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0] = 0x50;
		memcpy(	send_buff+1, card_sn,4);
					
		result = Pcd_Comm(Transceive,send_buff,5,rece_buff,&rece_bitlen);
		if (result==OK)
			*rece_len=rece_bitlen/8;
		else
			*rece_len=0;
		return result;
}
/*****************************************************************************************/
/*名称：TypeB_WUP																		 */
/*功能：TYPE B卡片唤醒																	 */
/*输入：N/A																				 */
/*输出：																				 */
/*		rece_len:卡片应答的数据长度；buff：卡片应答的数据指针							 */
/*		card_sn:卡片的PUPI字节															 */
/*	   	OK：操作成功																	 */
/*		ERROR：操作失败																	 */
/*****************************************************************************************/
unsigned char TypeB_WUP(unsigned char *rece_len,unsigned char *buff, struct PICC_B_STR *type_b_card)
{						
	unsigned char result,send_buff[3];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0]=0x05;//APf
		send_buff[1]=0x00;//AFI (00:for all cards)
		send_buff[2]=0x08;//PARAM(WUP,Number of slots =0)
		
		result = Pcd_Comm(Transceive,send_buff,3,buff,&rece_bitlen);
		if (result==OK)
			{
			*rece_len=rece_bitlen/8;
			memcpy(type_b_card->PUPI,buff+1,4);
			memcpy(type_b_card->APPLICATION_DATA,buff+6,4);
			memcpy(type_b_card->PROTOCOL_INF,buff+10,3);
			}
		else
			*rece_len=0;		
		return (result);
}
/*****************************************************************************************/
/*名称：TypeB_Request																	 */
/*功能：TYPE B卡片选卡																	 */
/*输入：																				 */
/*输出：																				 */
/*	   	OK：操作成功																	 */
/*		ERROR：操作失败																	 */
/*****************************************************************************************/
unsigned char TypeB_Request(unsigned char *rece_len,unsigned char *buff,struct PICC_B_STR *type_b_card)
{			
	unsigned char result,send_buff[3];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0]=0x05;	//APf
		send_buff[1]=0x00;	//AFI (00:for all cards)
		send_buff[2]=0x00;	//PARAM(REQB,Number of slots =0)
	
		result = Pcd_Comm(Transceive,send_buff,3,buff,&rece_bitlen);
		if (result==OK)
			{	
			*rece_len=rece_bitlen/8;
			memcpy(type_b_card->PUPI,buff+1,4);
			memcpy(type_b_card->APPLICATION_DATA,buff+6,4);
			memcpy(type_b_card->PROTOCOL_INF,buff+10,3);
			}
		else
			*rece_len=0;
		return result;
}		
/*****************************************************************************************/
/*名称：TypeB_Select																	 */
/*功能：TYPE B卡片选卡																	 */
/*输入：card_sn：卡片PUPI字节（4字节）													 */
/*输出：																				 */
/*		rece_len：应答数据的长度														 */
/*		buff：应答数据的指针															 */
/*	   	OK：操作成功																	 */
/*		ERROR：操作失败																	 */
/*****************************************************************************************/
unsigned char TypeB_Select(unsigned char *card_sn,unsigned char *rece_len,unsigned char *buff)
{
	unsigned char result,send_buff[9];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0] = 0x1d;
				
		memcpy(send_buff+ 1,card_sn,4);
			
		send_buff[5] = 0x00;  //Param1
		send_buff[6] = 0x08;  //01--24，08--256
		send_buff[7] = 0x01;  //COMPATIBLE WITH 14443-4
		send_buff[8] = 0x01;  //CID：01 
        		
		result = Pcd_Comm(Transceive,send_buff,9,buff,&rece_bitlen);
		if (result==OK)
			*rece_len=rece_bitlen/8;
		else
			*rece_len=0;
		return result;
}	
/*****************************************************************************************/
/*名称：TypeB_GetUID																	 */
/*功能：身份证专用指令																	 */
/*输入：N/A																				 */
/*输出：rece_len：返回数据的长度														 */
/*		buff：返回数据的指针															 */
/*	   	OK：操作成功																	 */
/*		ERROR：操作失败																	 */
/*****************************************************************************************/
unsigned char TypeB_GetUID(unsigned char *rece_len,unsigned char *buff)
{
	unsigned char result,send_buff[5];
	unsigned int rece_bitlen;
		Pcd_SetTimer(5);
		send_buff[0]=0x00;
		send_buff[1]=0x36;
		send_buff[2]=0x00;
		send_buff[3]=0x00;
		send_buff[4]=0x08;
		
		result = Pcd_Comm(Transceive,send_buff,5,buff,&rece_bitlen);
		if (result==OK)
			*rece_len=rece_bitlen/8;
		else
			*rece_len=0;
		return (result);
}	
