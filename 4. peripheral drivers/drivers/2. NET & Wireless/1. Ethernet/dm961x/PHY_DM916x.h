/*
*********************************************************************************************************
*
*	模块名称 : DM9161/9162 FOR CMSIS Driver
*	文件名称 : PHY_DM916x.h
*	版    本 : V1.0
*	说    明 : DM9161/9162驱动。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2019-08-14   Eric2013     首版
*
*	Copyright (C), 2019-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
/* ---------------------------------------------------------------------------
 * Copyright (C) 2013-2016 ARM Limited. All rights reserved.
 *  
 * $Date:        28. June 2016
 * $Revision:    V6.2
 *  
 * Project:      Ethernet Physical Layer Transceiver (PHY)
 *               Definitions for KSZ8081RNA
 * --------------------------------------------------------------------------*/

#ifndef __PHY_DM916X_H
#define __PHY_DM916X_H

#include "Driver_ETH_PHY.h"

/* Register Map */
#define REG_BMCR              0x00U     /* Basic Control                      */
#define REG_BMSR              0x01U     /* Basic Status                       */
#define REG_PHYIDR1           0x02U     /* PHY Identifier 1                   */
#define REG_PHYIDR2           0x03U     /* PHY Identifier 2                   */
#define REG_ANAR              0x04U     /* Auto-Negotiation Advertisement     */
#define REG_ANLPAR            0x05U     /* Auto-Neg. Link Partner Ability     */
#define REG_ANER              0x06U     /* Auto-Neg. Expansion                */

/* PHY Extended Registers for DM9161/9162 */
#define REG_DSCR        0x10     /* Specified Congfiguration Register            */
#define REG_DSCSR       0x11     /* Specified Congfiguration and Status Register */
#define REG_10BTCSR     0x12     /* 10Base-T Status/Control Register     		 */
#define REG_PWDOR       0x13     /* Power Down Control Register       			 */
#define REG_CONGFIG   	0x14     /* Specified Congfig Register      			 */
#define REG_INTERRUPT   0x15     /* Specified interrupt Register             	 */
#define REG_SRECR       0x16     /* Specified Receive Error Counter  			 */
#define REG_DISCR       0x17     /* Specified Disconnect Counter Register        */
#define REG_RLSR       	0x18     /* Hardware reset latch state Register			 */
#define REG_PSCR       	0x1D     /* Power Saving control register                */

/* Basic Control Register Bitmasks */
#define BMCR_RESET            0x8000U   /* Software Reset                     */
#define BMCR_LOOPBACK         0x4000U   /* Loopback mode                      */
#define BMCR_SPEED_SELECT     0x2000U   /* Speed Select (1=100Mb/s)           */
#define BMCR_ANEG_EN          0x1000U   /* Auto Negotiation Enable            */
#define BMCR_POWER_DOWN       0x0800U   /* Power Down                         */
#define BMCR_ISOLATE          0x0400U   /* Isolate Media interface            */
#define BMCR_RESTART_ANEG     0x0200U   /* Restart Auto Negotiation           */
#define BMCR_DUPLEX_MODE      0x0100U   /* Duplex Mode (1=Full duplex)        */
#define BMCR_COLLISION_TEST   0x0080U   /* Collision Test                     */

/* Basic Status Register Bitmasks */
#define BMSR_100B_T4          0x8000U   /* 100BASE-T4 Capable                 */
#define BMSR_100B_TX_FD       0x4000U   /* 100BASE-TX Full Duplex Capable     */
#define BMSR_100B_TX_HD       0x2000U   /* 100BASE-TX Half Duplex Capable     */
#define BMSR_10B_T_FD         0x1000U   /* 10BASE-T Full Duplex Capable       */
#define BMSR_10B_T_HD         0x0800U   /* 10BASE-T Half Duplex Capable       */
#define BMSR_NO_PREAMBLE      0x0040U   /* Preamble suppression               */
#define BMSR_ANEG_COMPLETE    0x0020U   /* Auto Negotiation Complete          */
#define BMSR_REMOTE_FAULT     0x0010U   /* Remote Fault                       */
#define BMSR_ANEG_ABILITY     0x0008U   /* Auto Negotiation Ability           */
#define BMSR_LINK_STAT        0x0004U   /* Link Status (1=link is up)         */
#define BMSR_JABBER_DETECT    0x0002U   /* Jabber Detect                      */
#define BMSR_EXT_CAPAB        0x0001U   /* Extended Capability                */

//#define PHY_ID_DM9161     0x0181B8B0  /* DM9161 PHY Identifier            */
//#define PHY_ID_DM9162     0x0181B8A0  /* DM9162 PHY Identifier            */
/* PHY Identifier Registers */
#define PHY_ID1              0x0181     /* DM916x Device Identifier MSB    */
#define PHY_ID2              0xB800     /* DM916x Device Identifier LSB    */

/* Auto-Negotiation Advertisement Register Bitmasks */
#define ANAR_NEXT_PAGE        0x8000U   /* Next page capable                  */
#define ANAR_REMOTE_FAULT     0x2000U   /* Remote fault supported             */
#define ANAR_PAUSE            0x0C00U   /* Pause                              */
#define ANAR_100B_T4          0x0200U   /* 100Base-T4 capable                 */
#define ANAR_100B_TX_FD       0x0100U   /* 100MBps full-duplex capable        */
#define ANAR_100B_TX_HD       0x0080U   /* 100MBps half-duplex capable        */
#define ANAR_10B_TX_FD        0x0040U   /* 10MBps full-duplex capable         */
#define ANAR_10B_TX_HD        0x0020U   /* 10MBps half-duplex capable         */
#define ANAR_SELECTOR_FIELD   0x001FU   /* Selector field (0x01==IEEE 802.3)  */

/* Auto-Negotiation Link Partner Ability Register Bitmasks */
#define ANLPAR_NEXT_PAGE      0x8000U   /* Next page capable                  */
#define ANLPAR_ACKNOWLEDGE    0x4000U   /* Acknowledge from partner           */
#define ANLPAR_REMOTE_FAULT   0x2000U   /* Remote fault detected              */
#define ANLPAR_PAUSE          0x0C00U   /* Pause                              */
#define ANLPAR_100B_TX_FD     0x0100U   /* 100MBps full-duplex capable        */
#define ANLPAR_100B_TX_HD     0x0080U   /* 100MBps half-duplex capable        */
#define ANLPAR_10B_TX_FD      0x0040U   /* 10MBps full-duplex capable         */
#define ANLPAR_10B_TX_HD      0x0020U   /* 10MBps half-duplex capable         */
#define ANLPAR_SELECTOR_FIELD 0x001FU   /* Selector field (0x01==IEEE 802.3)  */

/* Auto-Negotiation Expansion Register Bitmasks */
#define ANER_PDF              0x0010U   /* Parallel Detection Fault           */
#define ANER_LPAR_NEXT_PAGE   0x0008U   /* Link Partner Next Page Able        */
#define ANER_NEXT_PAGE        0x0004U   /* Next Page Able                     */
#define ANER_PAGE_RECEIVED    0x0002U   /* Page Received                      */
#define ANER_LPAR_ANEG        0x0001U   /* Link Partner Auto-Negotiation Able */


/* Specified Congfiguration and Status Register */
#define DSCSR_100M_FD         0x8000U   /* 100Base-TX Full-duplex bitmask    */
#define DSCSR_100M_HD         0x4000U   /* 100Base-TX Half-duplex bitmask    */
#define DSCSR_10M_FD          0x2000U   /* 10Base-TX Full-duplex bitmask     */
#define DSCSR_10M_HD          0x1000U   /* 10Base-TX Half-duplex bitmask     */

/* PHY Driver State Flags */
#define PHY_INIT              0x01U     /* Driver initialized                */
#define PHY_POWER             0x02U     /* Driver power is on                */

/* PHY Driver Control Structure */
typedef struct phy_ctrl {
  ARM_ETH_PHY_Read_t  reg_rd;           /* PHY register read function        */
  ARM_ETH_PHY_Write_t reg_wr;           /* PHY register write function       */
  uint16_t            bmcr;             /* BMCR register value               */
  uint8_t             flags;            /* Control flags                     */
  uint8_t             rsvd;             /* Reserved                          */
} PHY_CTRL;
#endif /* __PHY_KSZ8081RNA_H */
