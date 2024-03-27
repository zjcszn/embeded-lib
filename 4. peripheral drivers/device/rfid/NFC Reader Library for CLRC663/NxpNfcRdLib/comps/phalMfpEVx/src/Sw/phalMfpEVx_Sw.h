/*----------------------------------------------------------------------------*/
/* Copyright 2013 - 2019, 2022 NXP                                            */
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
* Software MIFARE Plus EVx contactless IC (Ev1, and future versions) contactless IC Application Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 5464 $ (v07.09.00)
* $Date: 2019-01-10 19:08:57 +0530 (Thu, 10 Jan 2019) $
*
* History:
*  Kumar GVS: Generated 15. Apr 2013
*
*/

#ifndef PHALMFPEVX_SW_H
#define PHALMFPEVX_SW_H

#include <ph_Status.h>

phStatus_t phalMfpEVx_Sw_WritePerso(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bLayer4Comm, uint16_t wBlockNr, uint8_t bNumBlocks,
    uint8_t * pValue);

phStatus_t phalMfpEVx_Sw_CommitPerso(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bLayer4Comm);





phStatus_t phalMfpEVx_Sw_AuthenticateMfc(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bBlockNo, uint8_t bKeyType, uint16_t wKeyNumber,
    uint16_t wKeyVersion, uint8_t * pUid, uint8_t bUidLength);






phStatus_t phalMfpEVx_Sw_GetVersion(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t * pResponse, uint8_t * pVerLen);

phStatus_t phalMfpEVx_Sw_ReadSign(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bLayer4Comm, uint8_t bAddr, uint8_t ** pSignature);

phStatus_t phalMfpEVx_Sw_ResetAuth(phalMfpEVx_Sw_DataParams_t * pDataParams);

phStatus_t phalMfpEVx_Sw_PersonalizeUid(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bUidType);

phStatus_t phalMfpEVx_Sw_SetConfigSL1(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bOption);

phStatus_t phalMfpEVx_Sw_ReadSL1TMBlock(phalMfpEVx_Sw_DataParams_t * pDataParams, uint16_t wBlockNr, uint8_t * pBlocks);

phStatus_t phalMfpEVx_Sw_VCSupportLastISOL3(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t * pIid, uint8_t * pPcdCapL3,
    uint8_t * pInfo);






phStatus_t phalMfpEVx_Sw_ResetSecMsgState(phalMfpEVx_Sw_DataParams_t * pDataParams);

phStatus_t phalMfpEVx_Sw_SetConfig(phalMfpEVx_Sw_DataParams_t *pDataParams, uint16_t wOption, uint16_t wValue);

phStatus_t phalMfpEVx_Sw_GetConfig(phalMfpEVx_Sw_DataParams_t * pDataParams, uint16_t wOption, uint16_t * pValue);

phStatus_t phalMfpEVx_Sw_SetVCAParams(phalMfpEVx_Sw_DataParams_t * pDataParams, void * pAlVCADataParams);


#endif /* PHALMFPEVX_SW_H */
