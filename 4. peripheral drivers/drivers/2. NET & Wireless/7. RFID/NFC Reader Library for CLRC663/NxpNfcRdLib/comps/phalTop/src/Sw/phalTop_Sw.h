/*----------------------------------------------------------------------------*/
/* Copyright 2016, 2017, 2022 NXP                                             */
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
* Software Tag Operation Application Layer Component of
* Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#ifndef PHALTOP_SW_H
#define PHALTOP_SW_H

#ifdef NXPBUILD__PHAL_TOP_SW

typedef phStatus_t (*pphalTop_Sw_CheckNdef) (phalTop_Sw_DataParams_t * pDataParams, uint8_t * pTagState );
typedef phStatus_t (*pphalTop_Sw_FormatNdef) (phalTop_Sw_DataParams_t * pDataParams);
typedef phStatus_t (*pphalTop_Sw_EraseNdef) (phalTop_Sw_DataParams_t * pDataParams);
typedef phStatus_t (*pphalTop_Sw_ReadNdef) (phalTop_Sw_DataParams_t * pDataParams,  uint8_t * pData,  uint32_t * pLength);
typedef phStatus_t (*pphalTop_Sw_WriteNdef) (phalTop_Sw_DataParams_t * pDataParams,  uint8_t * pData,  uint32_t  dwLength);
typedef phStatus_t (*pphalTop_Sw_Int_SetReadOnly) (phalTop_Sw_DataParams_t * pDataParams);
typedef phStatus_t (*pphalTop_Sw_LockBlock) (phalTop_Sw_DataParams_t * pDataParams, uint16_t wBlockNum);

phStatus_t phalTop_Sw_CheckNdef(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint8_t * pTagState
                                );

phStatus_t phalTop_Sw_FormatNdef(
                                 phalTop_Sw_DataParams_t * pDataParams
                                 );

phStatus_t phalTop_Sw_EraseNdef(
                                phalTop_Sw_DataParams_t * pDataParams
                                );

phStatus_t phalTop_Sw_Reset(
                            phalTop_Sw_DataParams_t * pDataParams
                            );

phStatus_t phalTop_Sw_SetConfig(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint16_t wConfig,
                                uint32_t dwValue
                                );

phStatus_t phalTop_Sw_GetConfig(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint16_t wConfig,
                                uint32_t  * dwValue
                                );

phStatus_t phalTop_Sw_ReadNdef(
                               phalTop_Sw_DataParams_t * pDataParams,
                               uint8_t * pData,
                               uint32_t * pLength
                               );

phStatus_t phalTop_Sw_WriteNdef(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint8_t * pData,
                                uint32_t dwLength
                                );

phStatus_t phalTop_Sw_LockBlock(
                                 phalTop_Sw_DataParams_t * pDataParams,
                                 uint16_t wBlockNum
                                 );

#ifdef __DEBUG
phStatus_t phalTop_Sw_SetPtr(
                             phalTop_Sw_DataParams_t * pDataParams,
                             void * pT1T,
                             void * pT2T,
                             void * pT3T,
                             void * pT4T,
                             void * pT5T,
                             void * pMfcTop
                             );
#endif /* __DEBUG */

#endif /* NXPBUILD__PHAL_TOP_SW */

#endif /* PHALTOP_SW_H */
