/*********************************************************************
*                                                                    *
*   Copyright (c) 2010 Shanghai FuDan MicroElectronic Inc, Ltd.      *
*   All rights reserved. Licensed Software Material.                 *
*                                                                    *
*   Unauthorized use, duplication, or distribution is strictly       *
*   prohibited by law.                                               *
*                                                                    *
**********************************************************************
*********************************************************************/

/*********************************************************************/
/* 	Uart通讯相关函数											     */
/* 	主要功能:														 */
/* 		1.实现Uart相关通讯											 */
/* 	编制:赵清泉														 */
/* 	编制时间:2012年5月24日											 */
/* 																	 */
/*********************************************************************/
#ifndef _UART_H_
#define _UART_H_

#include "DEFINE.h"

extern void UART_Com_Para_Init(UART_Com_Para_Def* UART_Com_Para);
extern void Uart_Send_Msg( char *msg);
extern void Uart_Send_Hex(unsigned char *input_byte,u16 input_len);
extern void Uart_Rece_Msg(unsigned char rece_len,char *msg);
extern Bool Scan_User_Reply(void);
#endif

