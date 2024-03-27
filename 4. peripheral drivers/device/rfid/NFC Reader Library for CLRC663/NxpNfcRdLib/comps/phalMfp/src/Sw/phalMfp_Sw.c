/*----------------------------------------------------------------------------*/
/* Copyright 2009-2021 NXP                                                    */
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
* Software MIFARE Plus contactless IC Application Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#include <ph_Status.h>
#include <phhalHw.h>
#include <phalMfp.h>
#include <phpalMifare.h>
#ifdef NXPBUILD__PH_CRYPTOSYM
#include <phCryptoSym.h>
#endif /* NXPBUILD__PH_CRYPTOSYM */
#ifdef NXPBUILD__PH_CRYPTORNG
#include <phCryptoRng.h>
#endif /* NXPBUILD__PH_CRYPTORNG */
#include <phKeyStore.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PHAL_MFP_SW

#include "../phalMfp_Int.h"
#include "phalMfp_Sw.h"
#include "phalMfp_Sw_Int.h"

phStatus_t phalMfp_Sw_Init(
                           phalMfp_Sw_DataParams_t * pDataParams,
                           uint16_t wSizeOfDataParams,
                           void * pPalMifareDataParams,
                           void * pKeyStoreDataParams,
                           void * pCryptoDataParamsEnc,
                           void * pCryptoDataParamsMac,
                           void * pCryptoRngDataParams,
                           void * pCryptoDiversifyDataParams
                           )
{
    phStatus_t PH_MEMLOC_REM statusTmp;

    /* data param check */
    if (sizeof(phalMfp_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFP);
    }
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pPalMifareDataParams);

    /* init private data */
    pDataParams->wId                    = PH_COMP_AL_MFP | PHAL_MFP_SW_ID;
    pDataParams->pPalMifareDataParams   = pPalMifareDataParams;
    pDataParams->pKeyStoreDataParams    = pKeyStoreDataParams;
    pDataParams->pCryptoDataParamsEnc   = pCryptoDataParamsEnc;
    pDataParams->pCryptoDataParamsMac   = pCryptoDataParamsMac;
    pDataParams->pCryptoRngDataParams   = pCryptoRngDataParams;
    pDataParams->pCryptoDiversifyDataParams  = pCryptoDiversifyDataParams;
    (void)memset(pDataParams->bKeyModifier, 0x00, PHAL_MFP_SIZE_KEYMODIFIER);

    /* clear the secure messaging state */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfp_Sw_ResetSecMsgState(pDataParams));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfp_Sw_WritePerso(
                                 phalMfp_Sw_DataParams_t * pDataParams,
                                 uint8_t bLayer4Comm,
                                 uint16_t wBlockNr,
                                 uint8_t * pValue
                                 )
{
    return phalMfp_Int_WritePerso(pDataParams->pPalMifareDataParams, bLayer4Comm, wBlockNr, pValue);
}

phStatus_t phalMfp_Sw_CommitPerso(
                                  phalMfp_Sw_DataParams_t * pDataParams,
                                  uint8_t bLayer4Comm
                                  )
{
    return phalMfp_Int_CommitPerso(pDataParams->pPalMifareDataParams, bLayer4Comm);
}

phStatus_t phalMfp_Sw_AuthenticateClassicSL2(
    phalMfp_Sw_DataParams_t * pDataParams,
    uint8_t bBlockNo,
    uint8_t bKeyType,
    uint16_t wKeyNo,
    uint16_t wKeyVersion,
    uint8_t * pUid,
    uint8_t bUidLength
    )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aKey[PHHAL_HW_MFC_KEY_LENGTH * 2U];
    uint8_t *   PH_MEMLOC_REM pKey = NULL;
    uint16_t    PH_MEMLOC_REM bKeystoreKeyType;
    uint8_t     PH_MEMLOC_REM bValue;

    /* Parameter check */
    if (pDataParams->pKeyStoreDataParams == NULL)
    {
        return PH_ADD_COMPCODE(PH_ERR_USE_CONDITION, PH_COMP_AL_MFP);
    }

    /* Bail out if we haven't got a keystore */
    if (pDataParams->pKeyStoreDataParams == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_KEY, PH_COMP_HAL);
    }

    /* retrieve KeyA & KeyB from keystore */
    PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVersion,
        sizeof(aKey),
        aKey,
        &bKeystoreKeyType));

    /* check key type */
    if (bKeystoreKeyType != PH_KEYSTORE_KEY_TYPE_MIFARE)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_KEY, PH_COMP_HAL);
    }

    /* Evaluate which key to use */
    if ((bKeyType & 0x7FU) == PHHAL_HW_MFC_KEYA)
    {
        /* Use KeyA */
        pKey = aKey;
    }
    else if ((bKeyType & 0x7FU) == PHHAL_HW_MFC_KEYB)
    {
        /* Use KeyB */
        pKey = &aKey[PHHAL_HW_MFC_KEY_LENGTH];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Apply the key modifier if requested */
    for (bValue = 0; bValue < PHHAL_HW_MFC_KEY_LENGTH; bValue++)
    {
        pKey[bValue] ^= pDataParams->bKeyModifier[bValue];
    }

    return phpalMifare_MfcAuthenticate(
        pDataParams->pPalMifareDataParams,
        bBlockNo,
        bKeyType,
        pKey,
        &pUid[bUidLength - 4U]);
}

phStatus_t phalMfp_Sw_MultiBlockRead(
                                     phalMfp_Sw_DataParams_t * pDataParams,
                                     uint8_t bBlockNr,
                                     uint8_t bNumBlocks,
                                     uint8_t * pBlocks
                                     )
{
    return phalMfp_Int_MultiBlockRead(pDataParams->pPalMifareDataParams, bBlockNr, bNumBlocks, pBlocks);
}

phStatus_t phalMfp_Sw_MultiBlockWrite(
                                      phalMfp_Sw_DataParams_t * pDataParams,
                                      uint8_t bBlockNr,
                                      uint8_t bNumBlocks,
                                      uint8_t * pBlocks
                                      )
{
    return phalMfp_Int_MultiBlockWrite(pDataParams->pPalMifareDataParams, bBlockNr, bNumBlocks, pBlocks);
}

phStatus_t phalMfp_Sw_ResetAuth(
                                phalMfp_Sw_DataParams_t * pDataParams
                                )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aCmd[1 /* command code */];
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;

    /* command code */
    aCmd[0] = PHAL_MFP_CMD_RAUTH;

    /* command exchange */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCmd,
        1,
        &pResponse,
        &wRxLength));

    /* check response */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfp_Int_ComputeErrorResponse(wRxLength, pResponse[0], 1));

    /* Reset the crypto layer */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfp_ResetSecMsgState(pDataParams));

    /* return exchange status code */
    return PH_ERR_SUCCESS;
}
phStatus_t phalMfp_Sw_ResetSecMsgState(
                                       phalMfp_Sw_DataParams_t * pDataParams
                                       )
{
    pDataParams->wRCtr                          = 0;
    pDataParams->wWCtr                          = 0;
    pDataParams->bNumUnprocessedReadMacBytes    = 0;
    pDataParams->bFirstRead                     = 1;
    (void)memset(pDataParams->bTi, 0x00, PHAL_MFP_SIZE_TI);

    return PH_ERR_SUCCESS;
}


phStatus_t phalMfp_Sw_Int_ComputeIv(
                                    uint8_t bIsResponse,
                                    uint8_t * pTi,
                                    uint16_t wRCtr,
                                    uint16_t wWCtr,
                                    uint8_t * pIv
                                    )
{
    uint8_t PH_MEMLOC_REM bIndex = 0;
    uint8_t PH_MEMLOC_REM bRCtrMsb = (uint8_t)(wRCtr >> 8U);
    uint8_t PH_MEMLOC_REM bRCtrLsb = (uint8_t)(wRCtr & 0x00ffU);
    uint8_t PH_MEMLOC_REM bWCtrMsb = (uint8_t)(wWCtr >> 8U);
    uint8_t PH_MEMLOC_REM bWCtrLsb = (uint8_t)(wWCtr & 0x00ffU);

    /* parameter checking */
    if ((pTi == NULL) || (pIv == NULL))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AL_MFP);
    }

    /* IV = [Transaction Identifier[0-1] | Transaction Identifier[2-3] | wRCtr | wWCtr | wRCtr | wWCtr | wRCtr | wWCtr] */

#if PHAL_MFP_TAPEOUT_VERSION >= 30
    if (bIsResponse == 0U)
    {
        pIv[bIndex++] = pTi[0];
        pIv[bIndex++] = pTi[1];
        pIv[bIndex++] = pTi[2];
        pIv[bIndex++] = pTi[3];
    }
#else

    pIv[bIndex++] = pTi[0];
    pIv[bIndex++] = pTi[1];
    pIv[bIndex++] = pTi[2];
    pIv[bIndex++] = pTi[3];

#endif

    pIv[bIndex++] = bRCtrLsb;
    pIv[bIndex++] = bRCtrMsb;
    pIv[bIndex++] = bWCtrLsb;
    pIv[bIndex++] = bWCtrMsb;

    pIv[bIndex++] = bRCtrLsb;
    pIv[bIndex++] = bRCtrMsb;
    pIv[bIndex++] = bWCtrLsb;
    pIv[bIndex++] = bWCtrMsb;

    pIv[bIndex++] = bRCtrLsb;
    pIv[bIndex++] = bRCtrMsb;
    pIv[bIndex++] = bWCtrLsb;
    pIv[bIndex++] = bWCtrMsb;

#if PHAL_MFP_TAPEOUT_VERSION >= 30
    if (bIsResponse != 0U)
    {
        pIv[bIndex++] = pTi[0];
        pIv[bIndex++] = pTi[1];
        pIv[bIndex++] = pTi[2];
        pIv[bIndex++] = pTi[3];
    }
#else
    /* satisfy compiler*/
    if (0U != (bIsResponse));
#endif

    return PH_ERR_SUCCESS;
}



void phalMfp_Sw_Int_TruncateMac(
                                uint8_t * pMac,
                                uint8_t * pTruncatedMac
                                )
{
#if PHAL_MFP_TAPEOUT_VERSION >= 20

    uint8_t PH_MEMLOC_REM bIndex;

    /* truncated MAC = [1, 3, 5, 7, 9, 11, 13, 15] of the input Mac */
    for (bIndex = 0; bIndex < PHAL_MFP_TRUNCATED_MAC_SIZE; ++bIndex)
    {
        pTruncatedMac[bIndex] = pMac[1U + (bIndex << 1U)];
    }

#else

    /* truncated MAC = 8 MSB of the input Mac */
    (void)memcpy(pTruncatedMac, pMac, PHAL_MFP_TRUNCATED_MAC_SIZE);

#endif
}

phStatus_t phalMfp_Sw_Cmd_PrepareProximityCheck(
    phalMfp_Sw_DataParams_t * pDataParams
    )
{
    return(phalMfp_Int_PrepareProximityCheck(pDataParams->pPalMifareDataParams));
}

#endif /* NXPBUILD__PHAL_MFP_SW */
