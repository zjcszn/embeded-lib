/*----------------------------------------------------------------------------*/
/* Copyright 2020, 2022 NXP                                                   */
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
* Internal functions of Software implementation of Virtual Card Architecture.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 5951 $ (v07.09.00)
* $Date: 2020-01-31 17:48:03 +0530 (Fri, 31 Jan 2020) $
*
* History:
*  CHu: Generated 31. August 2009
*
*/

#ifndef PHALVCA_SW_INT_H
#define PHALVCA_SW_INT_H

#include <ph_Status.h>

#define PHAL_VCA_CARD_TABLE_ENTRY_VALID     0x00U   /**< Value for a valid card table entry. */
#define PHAL_VCA_CARD_TABLE_ENTRY_INVALID   0xFFU   /**< Value for an invalid card table entry. */

#define PHAL_VCA_TRUNCATED_MAC_SIZE         8U      /**< Size of the truncated MAC. */

#define PHAL_VCA_POS_PCDCAP                 1U
#define PHAL_VCA_POS_STATUS                 0U
#define PHAL_VCA_POS_PAYLOAD                4U
#define PHAL_VCA_POS_RNDQ                   20U

phStatus_t phalVca_Sw_Int_ResolveValidIndex(phalVca_Sw_DataParams_t * pDataParams, uint16_t wIidIndex, uint16_t * pValidIndex);

#ifdef NXPBUILD__PH_CRYPTOSYM
void phalVca_Sw_Int_TruncateMac(uint8_t * pMac, uint8_t * pTruncatedMac);

void phhalVca_Sw_Int_ResetIV(phalVca_Sw_DataParams_t * pDataParams);

void phhalVca_Sw_Int_BackupIV(phalVca_Sw_DataParams_t * pDataParams, uint8_t *pIv_Bak);

uint8_t * phhalVca_Sw_Int_GetIV(phalVca_Sw_DataParams_t * pDataParams);

void phhalVca_Sw_Int_BackupKey(phalVca_Sw_DataParams_t * pDataParams, uint8_t *pKey_Bak);

void phhalVca_Sw_Int_RestoreKey(phalVca_Sw_DataParams_t * pDataParams, uint8_t *pKey);
#endif /* NXPBUILD__PH_CRYPTOSYM */


#endif /* PHALVCA_SW_INT_H */
