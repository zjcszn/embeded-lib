/*----------------------------------------------------------------------------*/
/* Copyright 2020 NXP                                                         */
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
* Software Virtual Card Architecture Application Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 5951 $ (v07.09.00)
* $Date: 2020-01-31 17:48:03 +0530 (Fri, 31 Jan 2020) $
*
* History:
*  CHu: Generated 31. August 2009
*
*/

#ifndef PHALVCA_SW_H
#define PHALVCA_SW_H

#include <ph_Status.h>

phStatus_t phalVca_Sw_StartCardSelection (phalVca_Sw_DataParams_t * pDataParams);

phStatus_t phalVca_Sw_FinalizeCardSelection (phalVca_Sw_DataParams_t * pDataParams, uint16_t * pNumValidIids);

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_SelectVc(phalVca_Sw_DataParams_t * pDataParams, uint16_t wValidIidIndex, uint16_t wKeyNumber, uint16_t wKeyVersion);
#endif /* NXPBUILD__PH_CRYPTOSYM */

phStatus_t phalVca_Sw_DeselectVc(phalVca_Sw_DataParams_t * pDataParams);

phStatus_t phalVca_Sw_VcSupport(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pIid, uint16_t wKeyEncNumber, uint16_t wKeyEncVersion,
    uint16_t wKeyMacNumber, uint16_t wKeyMacVersion);

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_VcSupportLast(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pIid, uint8_t bLenCap, uint8_t * pPcdCapabilities,
    uint16_t wKeyEncNumber, uint16_t wKeyEncVersion, uint16_t wKeyMacNumber, uint16_t wKeyMacVersion);
#endif /* NXPBUILD__PH_CRYPTOSYM */

phStatus_t phalVca_Sw_GetIidInfo(phalVca_Sw_DataParams_t * pDataParams, uint16_t wValidIidIndex, uint16_t * pIidIndex, uint8_t * pVcUidSize,
    uint8_t * pVcUid, uint8_t * pInfo, uint8_t * pPdCapabilities);

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_IsoSelect(phalVca_Sw_DataParams_t * pDataParams, uint8_t bSelectionControl, uint8_t bOption, uint8_t bDFnameLen,
    uint8_t * pDFname, uint8_t * pFCI, uint16_t * pwFCILen);

phStatus_t phalVca_Sw_IsoExternalAuthenticate(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pInData, uint16_t wKeyNumber, uint16_t wKeyVersion);
#endif /* NXPBUILD__PH_CRYPTOSYM */




#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_ProximityCheck(phalVca_Sw_DataParams_t * pDataParams, uint8_t bGenerateRndC, uint8_t * pRndC, uint8_t bPps1, uint8_t bNumSteps,
    uint16_t wKeyNumber, uint16_t wKeyVersion, uint8_t * pUsedRndC);

phStatus_t phalVca_Sw_ProximityCheckNew(phalVca_Sw_DataParams_t * pDataParams, uint8_t bGenerateRndC, uint8_t * pPrndC, uint8_t bNumSteps,
    uint16_t wKeyNumber, uint16_t wKeyVersion, uint8_t * pOption, uint8_t * pPubRespTime, uint8_t * pResponse, uint16_t * pRespLen,
    uint8_t * pCumRndRC);

phStatus_t phalVca_Sw_Cmd_PrepareProximityCheck(phalVca_Sw_DataParams_t * pDataParams);

phStatus_t phalVca_Sw_Cmd_PrepareProximityCheckNew(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pOption,  uint8_t * pPubRespTime, uint8_t * pResponse,
    uint16_t * pRespLen);

phStatus_t phalVca_Sw_Cmd_ProximityCheck(phalVca_Sw_DataParams_t * pDataParams, uint8_t bGenerateRndC, uint8_t * pRndC, uint8_t bNumSteps,
    uint8_t * pUsedRndRC);

phStatus_t phalVca_Sw_Cmd_ProximityCheckNew(phalVca_Sw_DataParams_t * pDataParams, uint8_t bGenerateRndC, uint8_t * pPrndC, uint8_t bNumSteps,
    uint8_t * pPubRespTime, uint8_t * pCumRndRC);

phStatus_t phalVca_Sw_Cmd_VerifyProximityCheck(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pRndRC, uint8_t bPps1, uint16_t wKeyNumber,
    uint16_t wKeyVersion);

phStatus_t phalVca_Sw_Cmd_VerifyProximityCheckNew(phalVca_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pPubRespTime, uint8_t * pResponse,
    uint16_t wRespLen, uint16_t wKeyNumber, uint16_t wKeyVersion, uint8_t * pRndCmdResp);
#endif /* NXPBUILD__PH_CRYPTOSYM */

phStatus_t phalVca_Sw_Cmd_VerifyProximityCheckUtility(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pCmdMac, uint8_t * pCmdResp);




phStatus_t phalVca_Sw_SetConfig(phalVca_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t wValue);

phStatus_t phalVca_Sw_GetConfig(phalVca_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t * pValue);

phStatus_t phalVca_Sw_SetSessionKeyUtility(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pSessionKey, uint8_t bAuthMode);

phStatus_t phalVca_Sw_SetApplicationType(phalVca_Sw_DataParams_t * pDataParams, void * pAlDataParams);

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_DecryptResponse(phalVca_Sw_DataParams_t * pDataParams, uint16_t  wKeyNo, uint16_t  wKeyVersion, uint8_t * pInData,
    uint8_t * pRandChal, uint8_t * pVCData);
#endif /* NXPBUILD__PH_CRYPTOSYM */

#endif /* PHALVCA_SW_H */
