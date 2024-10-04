/*----------------------------------------------------------------------------*/
/* Copyright 2009-2020, 2022 NXP                                              */
/*                                                                            */
/* NXP Confidential. This software is owned or controlled by NXP and may only */
/* be used strictly in accordance with the applicable license terms.          */
/* By expressly accepting such terms or by downloading, installing,           */
/* activating and/or otherwise using the software, you are agreeing that you  */
/* have read, and that you agree to comply with and are bound by, such        */
/* license terms. If you do not agree to be bound by the applicable license   */
/* terms, then you may not retain, install, activate or otherwise use the     */
/* software.                                                                  */
/*----------------------------------------------------------------------------*/

/** \file
* Internal definitions for Software ISO14443-3A Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
* History:
*  RSn: Generated 21. October 2009
*
*/

#ifndef PHPALSLI15693_SW_INT_H
#define PHPALSLI15693_SW_INT_H

#include <ph_Status.h>

/** \addtogroup ph_Private
* @{
*/

/** \name Constats
*/
/*@{*/
/** IC Manufacturer Code of NXP. */
#define PHPAL_SLI15693_SW_NXP_MFG_CODE              0x04U
/** Time extension in microseconds for all commands.  */
#define PHPAL_SLI15693_SW_EXT_TIME_US               60U
/** Waiting time T1 in us (ISO/IEC15693, 9.1.4.) */
#define PHPAL_SLI15693_SW_T1_WAITING_TIME           324U
/** Waiting time T2 in us (ISO/IEC15693, 9.1.4.) */
#define PHPAL_SLI15693_SW_T2_WAITING_TIME           310U
/** UID length in bits when complete. */
#define PHPAL_SLI15693_SW_UID_COMPLETE              64U
/** Default flags (DATA_RATE and INVENTORY BIT is set). */
#define PHPAL_SLI15693_SW_FLAGS_DEFAULT             0x06U
/** Slot timeout */
#define PHPAL_SLI15693_SW_TIMEOUT_SLOT_US           PHPAL_SLI15693_SW_T1_WAITING_TIME + PHPAL_SLI15693_SW_T2_WAITING_TIME
/** Nominal Response Time for High Data Rate */
#define PHPAL_SLI15693_SW_HIGH_NRT_US               3928U
/** Nominal Response Time for Fast High Data Rate */
#define PHPAL_SLI15693_SW_FASTHIGH_NRT_US           1964U
/** Nominal Response Time for Low Data Rate */
#define PHPAL_SLI15693_SW_LOW_NRT_US                15709U
/** Nominal Response Time for Fast Low Data Rate */
#define PHPAL_SLI15693_SW_FASTLOW_NRT_US            7855U
/** Start of Frame Time for High Data Rate */
#define PHPAL_SLI15693_SW_HIGH_SOF_US               152U
/** Start of Frame  Time for Fast High Data Rate */
#define PHPAL_SLI15693_SW_FASTHIGH_SOF_US           76U
/** Start of Frame Time for Low Data Rate */
#define PHPAL_SLI15693_SW_LOW_SOF_US                605U
/** Start of Frame  Time for Fast Low Data Rate */
#define PHPAL_SLI15693_SW_FASTLOW_SOF_US            303U
/** Default retry count. */
#define PHPAL_SLI15693_SW_RETRYCOUNT_DEFAULT        0x02U
/** Max retry count. */
#define PHPAL_SLI15693_SW_RETRYCOUNT_MAX            0x05U

/*@}*/

/** \name Mandatory commands
*/
/*@{*/
/** Anticollision inventory cmd code */
#define PHPAL_SLI15693_SW_CMD_INVENTORY        0x01U
/** Stay Quiet cmd code */
#define PHPAL_SLI15693_SW_CMD_STAY_QUIET       0x02U
/*@}*/

/** \name Optional commands
*/
/*@{*/
/** Select cmd code */
#define PHPAL_SLI15693_SW_CMD_SELECT            0x25U
/** Reset to ready cmd code */
#define PHPAL_SLI15693_SW_CMD_RESET_TO_READY    0x26U
/*@}*/

/** \name Custom commands
*/
/*@{*/
/** Inventory read cmd code */
#define PHPAL_SLI15693_SW_CMD_INVENTORY_READ            0xA0U
/** Fast inventory read cmd code */
#define PHPAL_SLI15693_SW_CMD_FAST_INVENTORY_READ       0xA1U
/** Inventory page read cmd code */
#define PHPAL_SLI15693_SW_CMD_INVENTORY_PAGE_READ       0xB0U
/** Fast inventory page read cmd code */
#define PHPAL_SLI15693_SW_CMD_FAST_INVENTORY_PAGE_READ  0xB1U
/** Stay quiet persistent cmd code */
#define PHPAL_SLI15693_SW_CMD_STAY_QUIET_PERSISTENT     0xBCU
/** ICODE command code for ExtendedGetSystemInformation command. */
#define PHPAL_SLI15693_SW_CMD_EXTENDED_GET_SYSTEM_INFORMATION  0x3BU
/** Begin Code for Custom command  as per ISO 15693 */
#define PHPAL_SLI15693_SW_CUSTOM_CMD_CODE_BEGIN     0x9FU
/** End Code for Custom command  as per ISO 15693 */
#define PHPAL_SLI15693_SW_CUSTOM_CMD_CODE_END       0xE0U
/* Macro to represent the command's MaskBitLen as extended. */
#define PHPAL_SLI15693_SW_FLAG_INVENTORY_READ_EXTENSION         0x80U
/* Maximum blocks supported by I15693 specification. */
#define PHPAL_SLI15693_SW_MAX_BLOCKS                            256U
/*@}*/

/**
* \brief Perform all kinds of Inventory commands.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phpalSli15693_Sw_InventoryEx(
                                        phpalSli15693_Sw_DataParams_t* pDataParams,     /**< [In] Pointer to this layers parameter structure. */
                                        /** [In] defines which way of (Fast)Inventory(Page)Read is done.\n
                                        * \c bCmd bits are defined as follows:\n
                                        * \li b1 = 0 -> InventoryRead
                                        * \li b1 = 1 -> InventoryPageRead
                                        * \li b2 = 0 -> normal
                                        * \li b2 = 1 -> FAST */
                                        uint8_t bCmd,
                                        uint8_t bFlags,             /**< [In] Request flags byte. */
                                        uint8_t bAfi,               /**< [In] Application Family Identifier. */
                                        uint8_t * pMask,            /**< [In] UID mask, holding known UID bits. */
                                        uint8_t bMaskBitLength,     /**< [In] Number of UID bits within pMask. */
                                        uint8_t bPage_Block_No,           /**< [In] Block number of first block to read. */
                                        uint16_t wNoOfPages_Blocks,       /**< [In] Number of blocks to read */
                                        uint8_t * pUid,             /**< [Out] Received Uid */
                                        uint8_t * pUidLength,       /**< [Out]  Number of received UID bytes. */
                                        uint8_t * pData,            /**< [Out]  Received data. */
                                        uint16_t * pDataLength          /**< [Out]  Number of received data bytes. */
                                        );


/**
* \brief Perform Inventory Read Extended mode
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_PROTOCOL_ERROR Invalid response received.
* \retval Other Depending on implementation and underlaying component.
*/
phStatus_t phpalSli15693_Sw_InventoryExt(
                                        phpalSli15693_Sw_DataParams_t* pDataParams,     /**< [In] Pointer to this layers parameter structure. */
                                        /** [In] defines which way of (Fast)Inventory(Page)Read is done.\n
                                        * \c bOption bits are defined as follows:\n
                                        * \li b1 = 0 -> InventoryRead
                                        * \li b1 = 1 -> InventoryPageRead
                                        * \li b2 = 0 -> normal
                                        * \li b2 = 1 -> FAST */
                                        uint8_t bCmd,
                                        uint8_t bFlags,             /**< [In] Request flags byte. */
                                        uint8_t bAfi,               /**< [In] Application Family Identifier. */
                                        uint8_t bExtended,          /**< [In] Extended mode */
                                        uint8_t bExtendedOptions,   /**< [In] Features of the extended mode */
                                        uint8_t * pCID,             /**< [In] Two byte CID -> if marked in extended options */
                                        uint8_t * pMask,            /**< [In] UID mask, holding known UID bits. */
                                        uint8_t bMaskBitLength,     /**< [In] Number of UID bits within pMask. */
                                        uint8_t bPage_Block_No,           /**< [In] Block number of first block to read. */
                                        uint16_t wNoOfPages_Blocks,       /**< [In] Number of blocks to read */
                                        uint8_t * pCIDOut,          /**< [Out] Received CID. */
                                        uint8_t * pUid,             /**< [Out] Received Uid */
                                        uint8_t * pUidLength,       /**< [Out]  Number of received UID bytes. */
                                        uint8_t * pData,            /**< [Out]  Received data. */
                                        uint16_t * pDataLength          /**< [Out]  Number of received data bytes. */
                                        );

/** @}
* end of ph_Private
*/

#endif /* PHPALSLI15693_SW_INT_H */
