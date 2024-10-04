/*----------------------------------------------------------------------------*/
/* Copyright 2013 - 2017, 2022 NXP                                            */
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
* Internal functions of Software implementation for MIFARE Plus EVx (Ev1, and future versions) application layer.
* $Author: NXP99556 $
* $Revision: 3340 $ (v07.09.00)
* $Date: 2017-04-05 16:07:29 +0530 (Wed, 05 Apr 2017) $
*
* History:
*  Kumar GVS: Generated 15. Apr 2013
*
*/

#ifndef PHALMFPEVX_SW_INT_H
#define PHALMFPEVX_SW_INT_H

#include <ph_Status.h>
#include <phalMfpEVx.h>

#define PHAL_MFPEVX_TAPEOUT_VERSION            30U

phStatus_t phalMfpEVx_Sw_Int_AuthenticateGeneral(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bLayer4Comm, uint8_t bFirstAuth,
    uint16_t wBlockNr, uint16_t wKeyNo, uint16_t wKeyVer, uint8_t bDivInputLen, uint8_t * pDivInput, uint8_t bLenPcdCap2,
    uint8_t * pPcdCap2In, uint8_t * pPcdCap2Out, uint8_t * pPdCap2);

phStatus_t phalMfpEVx_Sw_Int_ComputeIv(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bIsResponse, uint8_t * pTi, uint16_t wRCtr,
    uint16_t wWCtr, uint8_t * pIv);

phStatus_t phalMfpEVx_Sw_Int_KDF_EV0(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t * pRndA, uint8_t * pRndB);

phStatus_t phalMfpEVx_Sw_Int_KDF_EV1(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t * pRndA, uint8_t * pRndB);

phStatus_t phalMfpEVx_Sw_Int_WriteExtMfc(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bCmdCode, uint8_t bBlockNo, uint8_t * pData,
    uint16_t wDataLen, uint8_t * pTMC, uint8_t * pTMV);

phStatus_t phalMfpEVx_Sw_Int_ReadExtMfc(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bBlockNo, uint8_t * pData);

phStatus_t phalMfpEVx_Sw_Int_WriteExt(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bCmdCode, uint16_t wSrcBnr, uint16_t wDstBnr,
    uint8_t * pData, uint16_t wDataLen, uint8_t bEncrypted, uint8_t * pTMC, uint8_t * pTMV);

phStatus_t phalMfpEVx_Sw_Int_ReadExt(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bEncrypted, uint8_t bReadMaced, uint8_t bMacOnCmd,
    uint16_t wBlockNr, uint8_t bNumBlocks, uint8_t * pBlocks);

void phalMfpEVx_Sw_Int_TruncateMac(uint8_t * pMac, uint8_t * pTruncatedMac);

uint8_t * phalMfpEVx_Sw_Int_GetIV(void * pCryptoDataParams);

#endif /* PHALMFPEVX_SW_INT_H */
