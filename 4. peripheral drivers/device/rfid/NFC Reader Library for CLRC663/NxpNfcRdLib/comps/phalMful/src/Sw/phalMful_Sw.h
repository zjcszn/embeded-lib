/*----------------------------------------------------------------------------*/
/* Copyright 2009-2020,2022 NXP                                               */
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
* Software MIFARE(R) Ultralight contactless IC Application Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#ifndef PHALMFUL_SW_H
#define PHALMFUL_SW_H

#include <ph_Status.h>

phStatus_t phalMful_Sw_UlcAuthenticate(phalMful_Sw_DataParams_t * pDataParams, uint16_t wKeyNumber, uint16_t wKeyVersion);

phStatus_t phalMful_Sw_Read(
                            phalMful_Sw_DataParams_t * pDataParams,
                            uint8_t bAddress,
                            uint8_t * pData
                            );

phStatus_t phalMful_Sw_Write(
                             phalMful_Sw_DataParams_t * pDataParams,
                             uint8_t bAddress,
                             uint8_t * pData
                             );

phStatus_t phalMful_Sw_FastWrite(
                                 phalMful_Sw_DataParams_t * pDataParams,
                                 uint8_t * pData
                                 );

phStatus_t phalMful_Sw_CompatibilityWrite(
    phalMful_Sw_DataParams_t * pDataParams,
    uint8_t bAddress,
    uint8_t * pData
    );

phStatus_t phalMful_Sw_IncrCnt(
                               phalMful_Sw_DataParams_t * pDataParams,
                               uint8_t bCntNum,
                               uint8_t * pCnt
                               );

phStatus_t phalMful_Sw_ReadCnt(
                               phalMful_Sw_DataParams_t * pDataParams,
                               uint8_t bCntNum,
                               uint8_t * pCntValue
                               );

phStatus_t phalMful_Sw_PwdAuth(
                               phalMful_Sw_DataParams_t * pDataParams,
                               uint8_t * pPwd,
                               uint8_t * pPack
                               );

phStatus_t phalMful_Sw_GetVersion(
                                  phalMful_Sw_DataParams_t * pDataParams,
                                  uint8_t * pPwd
                                  );

phStatus_t phalMful_Sw_FastRead(
                                phalMful_Sw_DataParams_t * pDataParams,
                                uint8_t  bStartAddr,
                                uint8_t bEndAddr,
                                uint8_t ** ppData,
                                uint16_t * pNumBytes
                                );

phStatus_t phalMful_Sw_SectorSelect(
                                    phalMful_Sw_DataParams_t * pDataParams,
                                    uint8_t bSecNo
                                    );

phStatus_t phalMful_Sw_ReadSign(
                                phalMful_Sw_DataParams_t * pDataParams,
                                uint8_t bAddr,
                                uint8_t ** pSignature,
                                uint16_t* pDataLen
                                );

phStatus_t phalMful_Sw_ChkTearingEvent(
                                       phalMful_Sw_DataParams_t * pDataParams,
                                       uint8_t bCntNum,
                                       uint8_t * pValidFlag
                                       );

phStatus_t phalMful_Sw_WriteSign(
                                 phalMful_Sw_DataParams_t * pDataParams,
                                 uint8_t bAddress,
                                 uint8_t * pSignature
                                 );

phStatus_t phalMful_Sw_LockSign(
                                phalMful_Sw_DataParams_t * pDataParams,
                                uint8_t bLockMode
                                );

phStatus_t phalMful_Sw_VirtualCardSelect(
                                         phalMful_Sw_DataParams_t * pDataParams,
                                         uint8_t * pVCIID,
                                         uint8_t bVCIIDLen,
                                         uint8_t * pVCTID
                                         );

phStatus_t phalMful_Sw_ReadTTStatus(
                                    phalMful_Sw_DataParams_t * pDataParams,
                                    uint8_t bAddr,
                                    uint8_t * pData);

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalMful_Sw_AuthenticateAES(phalMful_Sw_DataParams_t * pDataParams,
                                        uint16_t wKeyNo,
                                        uint16_t wKeyVer,
                                        uint8_t bKeyNoCard);

phStatus_t phalMful_Sw_GetConfig(phalMful_Sw_DataParams_t * pDataParams,
                                uint16_t wConfig,
                                uint16_t * pValue);

phStatus_t phalMful_Sw_SetConfig(phalMful_Sw_DataParams_t *pDataParams,
                                 uint16_t wConfig,
                                 uint16_t wValue);

phStatus_t phalMful_Sw_CalculateSunCMAC(phalMful_Sw_DataParams_t * pDataParams,
                                        uint16_t wKeyNo,
                                        uint16_t wKeyVer,
                                        uint8_t * pInData,
                                        uint16_t wInDataLen,
                                        uint8_t * pRespMac);
#endif /* NXPBUILD__PH_CRYPTOSYM */
#endif /* PHALMFUL_SW_H */
