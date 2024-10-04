/*----------------------------------------------------------------------------*/
/* Copyright 2009 - 2019, 2022 NXP                                            */
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
* Software specific Symmetric Cryptography Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6642 $ (v07.09.00)
* $Date: 2022-06-06 11:32:31 +0530 (Mon, 06 Jun 2022) $
*
* History:
*  SLe: Generated 01.12.2009
*
*/

#ifndef PHCRYPTOSYM_SW_H
#define PHCRYPTOSYM_SW_H

#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phCryptoSym.h>

#define PH_CRYPTOSYM_SW_NUM_AES_ROUNDS_128 10U  /**< AES rounds for 128 bit key. */
#define PH_CRYPTOSYM_SW_NUM_AES_ROUNDS_192 12U  /**< AES rounds for 192 bit key. */
#define PH_CRYPTOSYM_SW_NUM_AES_ROUNDS_256 14U  /**< AES rounds for 256 bit key. */

phStatus_t phCryptoSym_Sw_InvalidateKey(phCryptoSym_Sw_DataParams_t * pDataParams);

phStatus_t phCryptoSym_Sw_Encrypt(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, const uint8_t * pPlainBuff, uint16_t wBuffLen,
    uint8_t * pEncBuff);

phStatus_t phCryptoSym_Sw_Decrypt(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, const uint8_t * pEncBuff, uint16_t wBuffLen,
    uint8_t * pPlainBuff);


phStatus_t phCryptoSym_Sw_CalculateMac(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, const uint8_t * pData, uint16_t  wDataLen,
    uint8_t * pMac, uint8_t * pMacLen);

phStatus_t phCryptoSym_Sw_LoadIv(phCryptoSym_Sw_DataParams_t * pDataParams, const uint8_t * pIV, uint8_t bIVLen);

phStatus_t phCryptoSym_Sw_LoadKey(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVer, uint16_t wKeyType);

phStatus_t phCryptoSym_Sw_LoadKeyDirect(phCryptoSym_Sw_DataParams_t * pDataParams, const uint8_t * pKey, uint16_t wKeyType);

phStatus_t phCryptoSym_Sw_DiversifyKey(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, uint16_t wKeyNo, uint16_t wKeyVer,
    uint8_t * pDivInput, uint8_t bDivInputLen, uint8_t * pDiversifiedKey, uint8_t * pDivKeyLen);

phStatus_t phCryptoSym_Sw_DiversifyDirectKey(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, uint8_t * pKey, uint16_t wKeyType,
    uint8_t * pDivInput, uint8_t bDivInputLen, uint8_t * pDiversifiedKey, uint8_t * pDivKeyLen);

phStatus_t phCryptoSym_Sw_SetConfig(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t wValue);

phStatus_t phCryptoSym_Sw_GetConfig(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t * pValue);

#endif /* PHCRYPTOSYM_SW_H */
