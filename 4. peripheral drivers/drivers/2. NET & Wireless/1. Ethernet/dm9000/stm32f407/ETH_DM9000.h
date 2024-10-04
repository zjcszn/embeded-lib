/*
*********************************************************************************************************
*
*	模块名称 : CMSIS-Driver
*	文件名称 : ETH_DM9000.c
*	版    本 : V1.0
*	说    明 : CMSIS-Driver的DM9000底层驱动实现【安富莱原创】
*
*	修改记录 :
*		版本号    日期        作者     说明
*		V1.0    2020-11-21   Eric2013 正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#ifndef _ETH_DM9000_H_
#define _ETH_DM9000_H_

#include <stdint.h>
#include "Driver_ETH_MAC.h"

/* ETH Driver state flags */
#define ETH_INIT                     (0x01UL)   /* Driver initialized */
#define ETH_POWER                    (0x02UL)   /* Driver powered     */

/* ETH Driver Control Information */
typedef struct {
  uint32_t Us_Cyc;
  uint32_t Tx_Len;
  uint16_t Tx_PacketTag;
  uint8_t  Flags;
  uint8_t  Rsvd;
  ARM_ETH_MAC_SignalEvent_t cb_event;   
} ETH_CTRL;

extern uint32_t SystemCoreClock;

#endif 

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
