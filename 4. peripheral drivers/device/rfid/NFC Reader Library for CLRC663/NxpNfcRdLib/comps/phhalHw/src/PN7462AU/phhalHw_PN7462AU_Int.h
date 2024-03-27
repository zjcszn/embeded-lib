/*----------------------------------------------------------------------------*/
/* Copyright 2013, 2021, 2022 NXP                                             */
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

/**
 *
 * \file phhalHw_PN7462AU.h :  This file contains wrapper Api's which connects
 *                              the Reader library interfaces with the phhalRf interfaces.
 *
 * Project:  PN7462AU
 *
 * $Date$
 * $Author$
 * $Revision$ (v07.09.00)
 *
 */

#ifndef PHHALHW_PN7462AU_INT_H_
#define PHHALHW_PN7462AU_INT_H_

/*********************************************************************************************************************/
/*   INCLUDES                                                                                                        */
/*********************************************************************************************************************/
#include <ph_Status.h>
#include <ph_Datatypes.h>

/*********************************************************************************************************************/
/*   DEFINES                                                                                                         */
/*********************************************************************************************************************/

/**
* ISO 18000p3m3 Sub-carrier type
*/
#define PHHAL_HW_PN7462AU_I18000P3M3_M_MANCHESTER_2     0x02U
#define PHHAL_HW_PN7462AU_I18000P3M3_M_MANCHESTER_4     0x03U

/**
* ISO 18000p3m3 CRC-PRESET Values
*/
#define PHHAL_HW_PN7462AU_I18000P3M3_SELECT_PRESET      0x03U
#define PHHAL_HW_PN7462AU_I18000P3M3_NONSELECT_PRESET   0x04U

/**
* ISO 18000p3m3 Timeout constants
*/
#define PHHAL_HW_PN7462AU_I18000P3M3_SW_T1_MAX_US      78U
#define PHHAL_HW_PN7462AU_I18000P3M3_SW_T2_MIN_US      151U
#define PHHAL_HW_PN7462AU_I18000P3M3_SW_T3_MIN_US      25U
#define PHHAL_HW_PN7462AU_I18000P3M3_SW_T4_MIN_US      100U
#ifndef NXPBUILD__PHPAL_PR_ARTF32855
#define PHHAL_HW_PN7462AU_I18000P3M3_SW_DELTA_MIN_US   10U
#endif /* NXPBUILD__PHPAL_PR_ARTF32855*/

/**
* ISO 18000p3m3 Command Codes
*/
#define PHHAL_HW_PN7462AU_I18000P3M3_CMD_NEXT_SLOT      0x00U
#define PHHAL_HW_PN7462AU_I18000P3M3_CMD_ACK            0x01U
#define PHHAL_HW_PN7462AU_I18000P3M3_CMD_REQ_RN         0xC1U

phStatus_t phhalHw_PN7462AU_18000p3m3_Sw_Exchange(
    phhalHw_PN7462AU_DataParams_t * pDataParams,
    uint16_t wOption,
    uint8_t * pTxBuffer,
    uint16_t wTxLength,
    uint8_t bTxLastBits,
    uint8_t ** ppRxBuffer,
    uint16_t * pRxLength,
    uint8_t * pRxLastBits
    );

phStatus_t phhalHw_PN7462AU_18000p3m3_Sw_Select(
    phhalHw_PN7462AU_DataParams_t * pDataParams,
    uint8_t * pSelCmd,
    uint8_t bSelCmdLen,
    uint8_t bNumValidBitsinLastByte
    );

phStatus_t phhalHw_PN7462AU_18000p3m3_Sw_BeginRound(
    phhalHw_PN7462AU_DataParams_t * pDataParams,
    uint8_t * pBeginRndCmd
    );

phStatus_t phhalHw_PN7462AU_18000p3m3_Sw_NextSlot(
    phhalHw_PN7462AU_DataParams_t * pDataParams
    );

phStatus_t phhalHw_PN7462AU_18000p3m3_Sw_Ack(
    phhalHw_PN7462AU_DataParams_t * pDataParams,
    uint8_t ** ppRxBuffer,
    uint16_t *pRxLength,
    uint8_t *pRxLastBits
    );

phStatus_t phhalHw_PN7462AU_lI18000p3m3ReqRn(
    phhalHw_PN7462AU_DataParams_t * pDataParams,
    uint8_t ** ppRxBuffer
    );

/* Returns the TxBuffer pointer, length and size */
phStatus_t phhalHw_PN7462AU_GetTxBuffer(
    phhalHw_PN7462AU_DataParams_t * pDataParams,      /**< [In] Pointer to this layer's parameter structure. */
    uint8_t ** pTxBuffer,                             /**< [Out] Pointer to HAL TX buffer.*/
    uint16_t * pTxBufferLen,                          /**< [Out] Length of the buffer.*/
    uint16_t * pTxBufferSize                          /**< [Out] Size of the buffer.*/
);

phStatus_t phhalHw_PN7462AU_GetFdt(
    phhalHw_PN7462AU_DataParams_t * pDataParams,
    uint32_t * pTime
    );

#endif /* PHHALHW_PN7462AU_INT_H_ */
