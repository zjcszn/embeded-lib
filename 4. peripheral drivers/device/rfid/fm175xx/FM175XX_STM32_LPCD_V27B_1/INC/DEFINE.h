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
/* 	ARM所用变量宏定义													 */
/* 	主要功能:														 */
/* 		1.程序中产量定义											 */
/* 	编制:赵清泉														 */
/* 	编制时间:2012年5月16日											 */
/* 																	 */
/*********************************************************************/

#ifndef _DEFINE_H_
#define _DEFINE_H_

#include "stm32f10x_map.h"

#define UART1_EN_SEL       0x00
#define UART3_EN_SEL  	    0x01
#define	I2C_EN_SEL		     	0x00
#define SPI1_EN_SEL					0x00
#define SPI2_EN_SEL        0x01
#define USB_EN_SEL         0x00
#define UART1_BaudRate     115200
#define UART3_BaudRate		115200
#ifndef true
	#define true				TRUE	//真
#endif
#ifndef false
	#define false				FALSE	//假
#endif
#ifndef True
	#define True				TRUE
	#define False				FALSE
#endif

#ifndef uchar
	#define uchar 			u8
	#define uint				u16
#endif

#define BIT0               0x01
#define BIT1               0x02
#define BIT2               0x04
#define BIT3               0x08
#define BIT4               0x10
#define BIT5               0x20
#define BIT6               0x40
#define BIT7               0x80

typedef struct
{
	u16 Send_Len;
	u16 Send_Index;
	Bool SendStatus;
	u16 Recv_Len;
	u16 Recv_Index; 
	Bool RecvStatus;			
	char *buff;
} UART_Com_Para_Def;

//******************* ARM 管脚定义区 **************************

#define PORT_IRQ					GPIOC
#define PIN_IRQ						GPIO_Pin_12			//PC12 IRQ引脚（Pin23）

#define PORT_NRST					GPIOC
#define PIN_NRST					GPIO_Pin_15			//PB13 NPD引脚（Pin6）

//FM175XX A0~A1	 
#define PORT_ADDR					GPIOC
#define PIN_EA						GPIO_Pin_11			//PA0 EA引脚（Pin32）
#define PIN_I2C						GPIO_Pin_10			//PA1 I2C引脚（Pin1）

//FM175XX SPI
#define PORT_SPI					GPIOB
#define PIN_NSS						GPIO_Pin_12			//PB12 NSS引脚（Pin24）
#define PIN_SCK						GPIO_Pin_13			//PB13 SCK引脚（Pin29）
#define PIN_MISO					GPIO_Pin_14			//PB14 MISO引脚（Pin31）
#define PIN_MOSI					GPIO_Pin_15			//PB15 MOSI引脚（Pin30）


//LED控制：PD2
#define PORT_LED              GPIOD		   
#define PIN_LED0              GPIO_Pin_2
#define LED_ARM_WORKING				PIN_LED0
#define LED_ARM_WORKING_OFF		(PORT_LED->BSRR = LED_ARM_WORKING)		//LED0
#define LED_ARM_WORKING_ON		(PORT_LED->BRR = LED_ARM_WORKING)			//LED0

//-----------------------------------------------------------------------------

#endif
