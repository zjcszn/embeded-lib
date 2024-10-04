/*----------------------------------------------------------------------------*/
/* Copyright 2020,2022,2023 NXP                                               */
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
* Software MIFARE Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6155 $ (v07.09.00)
* $Date: 2020-07-20 22:32:37 +0530 (Mon, 20 Jul 2020) $
*
* History:
*  CHu: Generated 31. August 2009
*
*/

#include <phhalHw.h>
#include <phalVca.h>
#include <phpalMifare.h>
#ifdef NXPBUILD__PH_CRYPTOSYM
#include <phCryptoSym.h>
#endif /* NXPBUILD__PH_CRYPTOSYM */
#ifdef NXPBUILD__PH_CRYPTORNG
#include <phCryptoRng.h>
#endif /* NXPBUILD__PH_CRYPTORNG */
#include <phKeyStore.h>
#include <ph_RefDefs.h>
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
#include <phalMfdfEVx.h>
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */
#ifdef NXPBUILD__PHAL_MFPEVX_SW
#include <phalMfpEVx.h>
#endif /* NXPBUILD__PHAL_MFPEVX_SW */

#ifdef NXPBUILD__PHAL_VCA_SW

#include "../phalVca_Int.h"
#include "phalVca_Sw_Int.h"
#include "phalVca_Sw.h"

#ifdef NXPBUILD__PH_CRYPTOSYM
/* Private constants */
static const uint8_t PH_MEMLOC_CONST_ROM phalVca_Sw_FirstIv[PH_CRYPTOSYM_AES_BLOCK_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif /* NXPBUILD__PH_CRYPTOSYM */

phStatus_t phalVca_Sw_Init(phalVca_Sw_DataParams_t * pDataParams, uint16_t wSizeOfDataParams,  void * pPalMifareDataParams, void * pKeyStoreDataParams,
    void * pCryptoDataParams, void * pCryptoRngDataParams, phalVca_Sw_IidTableEntry_t * pIidTableStorage, uint16_t wNumIidTableStorageEntries,
    phalVca_Sw_CardTableEntry_t * pCardTableStorage, uint16_t wNumCardTableStorageEntries)
{
    if (sizeof(phalVca_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_VCA);
    }

    PH_ASSERT_NULL_DATA_PARAM (pDataParams, PH_COMP_AL_VCA);
    PH_ASSERT_NULL_PARAM (pPalMifareDataParams, PH_COMP_AL_VCA);

#ifdef NXPBUILD__PH_CRYPTOSYM
    PH_ASSERT_NULL_PARAM (pKeyStoreDataParams, PH_COMP_AL_VCA);
    PH_ASSERT_NULL_PARAM (pCryptoDataParams, PH_COMP_AL_VCA);
    PH_ASSERT_NULL_PARAM (pCryptoRngDataParams, PH_COMP_AL_VCA);
#endif /* NXPBUILD__PH_CRYPTOSYM */

    if (0U != (wNumIidTableStorageEntries)) PH_ASSERT_NULL_PARAM (pIidTableStorage, PH_COMP_AL_VCA);
    if (0U != (wNumCardTableStorageEntries)) PH_ASSERT_NULL_PARAM (pCardTableStorage, PH_COMP_AL_VCA);

    /* init private data */
    pDataParams->wId                    = PH_COMP_AL_VCA | PHAL_VCA_SW_ID;
    pDataParams->pPalMifareDataParams   = pPalMifareDataParams;
    pDataParams->pKeyStoreDataParams    = pKeyStoreDataParams;
    pDataParams->pCryptoDataParams      = pCryptoDataParams;
    pDataParams->wCurrentCardTablePos   = 0;
    pDataParams->pCardTable             = pCardTableStorage;
    pDataParams->wNumCardTableEntries   = wNumCardTableStorageEntries;
    pDataParams->pIidTable              = pIidTableStorage;
    pDataParams->wCurrentIidTablePos    = 0;
    pDataParams->wNumIidTableEntries    = wNumIidTableStorageEntries;
    pDataParams->pCryptoRngDataParams   = pCryptoRngDataParams;
    pDataParams->wAdditionalInfo        = PH_ERR_SUCCESS;
    pDataParams->eVCState               = VC_NOT_SELECTED;
    pDataParams->ePCState               = PC_NO_PCHK_IN_PROGRESS;
    pDataParams->bWrappedMode           = 0;
    pDataParams->bExtendedLenApdu       = 0;
    pDataParams->bOption                = 1;
    pDataParams->bLowerBoundThreshold   = 0;

    return PH_ERR_SUCCESS;
}




phStatus_t phalVca_Sw_StartCardSelection (phalVca_Sw_DataParams_t * pDataParams)
{
    uint16_t PH_MEMLOC_REM wIndex;

    for (wIndex=0; wIndex < pDataParams->wNumCardTableEntries; ++wIndex)
    {
        pDataParams->pCardTable[wIndex].bValid = PHAL_VCA_CARD_TABLE_ENTRY_INVALID;
    }

    pDataParams->wCurrentIidIndex       = 0;
    pDataParams->wCurrentCardTablePos   = 0;
    pDataParams->wCurrentIidTablePos    = 0;

    return PH_ERR_SUCCESS;
}

phStatus_t phalVca_Sw_FinalizeCardSelection (phalVca_Sw_DataParams_t * pDataParams, uint16_t * pNumValidIids)
{
    uint16_t PH_MEMLOC_REM wDummyValidIids = 0;
    uint16_t PH_MEMLOC_REM wValidIids = 0;
    uint16_t PH_MEMLOC_REM wIndex;

    for (wIndex = 0; wIndex < pDataParams->wNumCardTableEntries; ++wIndex)
    {
        if (pDataParams->pCardTable[wIndex].bValid == PHAL_VCA_CARD_TABLE_ENTRY_INVALID)
        {
            ++wDummyValidIids;
        }
        else
        {
            ++wValidIids;
        }
    }

    *pNumValidIids = wValidIids;

    return PH_ERR_SUCCESS;
}

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_SelectVc(phalVca_Sw_DataParams_t * pDataParams, uint16_t wValidIidIndex, uint16_t wKeyNumber, uint16_t wKeyVersion)
{
    phStatus_t  PH_MEMLOC_REM statusTmp, status;
    uint8_t     PH_MEMLOC_REM aTxBuffer[PHAL_VCA_IID_SIZE + 1];
    uint8_t *   PH_MEMLOC_REM pResponse;
    uint16_t    PH_MEMLOC_REM wRxLength;
    uint8_t     PH_MEMLOC_REM aKey[PH_CRYPTOSYM_AES128_KEY_SIZE];
    uint16_t    PH_MEMLOC_REM wKeyType;
    uint8_t     PH_MEMLOC_REM bMacLength;

    /* Prepare command header */
    aTxBuffer[0] = PHAL_VCA_CMD_SVC;
    aTxBuffer[1] = PHAL_VCA_CMD_SVC;

    /* Resolve Iid index */
    status = phalVca_Sw_Int_ResolveValidIndex(
        pDataParams,
        wValidIidIndex,
        &wValidIidIndex);

    /* for the case of an overflow we generate random data */
    /* Prepare MAC data */
    if ((status  & PH_ERR_MASK) == PH_ERR_INVALID_PARAMETER)
    {
        wValidIidIndex = 0;
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoRng_Rnd(pDataParams->pCryptoRngDataParams, PHAL_VCA_IID_SIZE-1, &aTxBuffer[2]));
    }
    else
    {
        PH_CHECK_SUCCESS(status);
        (void)memcpy(&aTxBuffer[2], &pDataParams->pCardTable[wValidIidIndex].pCardData[1], PHAL_VCA_IID_SIZE-1);
    }

    /* Get MAC Key */
    PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNumber,
        wKeyVersion,
        sizeof(aKey),
        aKey,
        &wKeyType));

    /* Check key type */
    if (wKeyType != PH_CRYPTOSYM_KEY_TYPE_AES128)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
    }

    /* Load key */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadKeyDirect(
        pDataParams->pCryptoDataParams,
        aKey,
        PH_CRYPTOSYM_KEY_TYPE_AES128));

    /* Calculate MAC */
    /* CMAC with Padding */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_CalculateMac(
        pDataParams->pCryptoDataParams,
        PH_CRYPTOSYM_MAC_MODE_CMAC,
        &aTxBuffer[1],
        PHAL_VCA_IID_SIZE,
        &aTxBuffer[1],
        &bMacLength));

    /* Truncate MAC */
    phalVca_Sw_Int_TruncateMac(&aTxBuffer[1], &aTxBuffer[1]);

    /* command exchange */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aTxBuffer,
        1 + PHAL_VCA_TRUNCATED_MAC_SIZE,
        &pResponse,
        &wRxLength));

    /* check response */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Int_ComputeErrorResponse(wRxLength, pResponse[0]));

    /* check response length */
    if (wRxLength != 1 /* STATUS */)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_VCA);
    }

    return PH_ERR_SUCCESS;
}
#endif /* NXPBUILD__PH_CRYPTOSYM */

phStatus_t phalVca_Sw_DeselectVc(phalVca_Sw_DataParams_t * pDataParams)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aTxBuffer[1];
    uint8_t *   PH_MEMLOC_REM pResponse;
    uint16_t    PH_MEMLOC_REM wRxLength;

    /* command code */
    aTxBuffer[0] = PHAL_VCA_CMD_DVC;

    /* command exchange */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aTxBuffer,
        1,
        &pResponse,
        &wRxLength));

    /* check response */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Int_ComputeErrorResponse(wRxLength, pResponse[0]));

    /* check response length */
    if (wRxLength != 1 /* STATUS */)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_VCA);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalVca_Sw_VcSupport(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pIid, uint16_t wKeyEncNumber, uint16_t wKeyEncVersion,
    uint16_t wKeyMacNumber, uint16_t wKeyMacVersion)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aTxBuffer[1];
    uint8_t *   PH_MEMLOC_REM pResponse;
    uint16_t    PH_MEMLOC_REM wRxLength;

    /* Check available space in key duos list */
    if (pDataParams->wCurrentIidTablePos >= pDataParams->wNumIidTableEntries )
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_VCA);
    }

    /* Add keys and iids to the iid table list */
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wIidIndex = pDataParams->wCurrentIidIndex;
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wKeyEncNumber = wKeyEncNumber;
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wKeyEncVersion = wKeyEncVersion;
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wKeyMacNumber = wKeyMacNumber;
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wKeyMacVersion = wKeyMacVersion;
    pDataParams->wCurrentIidTablePos++;
    pDataParams->wCurrentIidIndex++;

    /* command code */
    aTxBuffer[0] = PHAL_VCA_CMD_VCS;

    /* buffer the command frame */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        aTxBuffer,
        1,
        &pResponse,
        &wRxLength));

    /* Append IID and exchange the command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        pIid,
        PHAL_VCA_IID_SIZE,
        &pResponse,
        &wRxLength));

    /* check response */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Int_ComputeErrorResponse(wRxLength, pResponse[0]));

    /* check response length */
    if (wRxLength != 1 /* STATUS */)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_VCA);
    }
    return PH_ERR_SUCCESS;
}

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_VcSupportLast(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pIid, uint8_t bLenCap, uint8_t * pPcdCapabilities,
    uint16_t wKeyEncNumber, uint16_t wKeyEncVersion, uint16_t wKeyMacNumber, uint16_t wKeyMacVersion)
{
    phStatus_t                      PH_MEMLOC_REM statusTmp;
    uint8_t                         PH_MEMLOC_REM aTmpBuf[PH_CRYPTOSYM_AES_BLOCK_SIZE * 2U];
    uint8_t                         PH_MEMLOC_REM aTxBuffer[1];
    uint8_t *                       PH_MEMLOC_REM pResponse = NULL;
    uint16_t                        PH_MEMLOC_REM wRxLength = 0;
    uint8_t                         PH_MEMLOC_REM aKey[PH_CRYPTOSYM_AES128_KEY_SIZE];
    uint16_t                        PH_MEMLOC_REM wKeyType;
    uint8_t                         PH_MEMLOC_REM bMac[PH_CRYPTOSYM_AES_BLOCK_SIZE];
    uint8_t                         PH_MEMLOC_REM bMacLength;
    phalVca_Sw_CardTableEntry_t     PH_MEMLOC_REM pDummyCardData;
    phalVca_Sw_CardTableEntry_t *   PH_MEMLOC_REM pCardDataStorage = NULL;
    uint16_t                        PH_MEMLOC_REM wIndex;

    /* Build the command frame */
    aTxBuffer[0] = PHAL_VCA_CMD_VCSL;

    /* Copy PCD Caps */
    (void)memset(aTmpBuf, 0x00, 4);
    if (0U != (bLenCap))
    {
        if (bLenCap > 3U)
        {
            bLenCap = 3;
        }
        aTmpBuf[0] = bLenCap;
        (void)memcpy(&aTmpBuf[1], pPcdCapabilities, bLenCap);
    }

    /* Check available space in key duos list */
    if (pDataParams->wCurrentIidTablePos >= pDataParams->wNumIidTableEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_VCA);
    }

    /* Check available space in card table */
    if (pDataParams->wCurrentCardTablePos >= pDataParams->wNumCardTableEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_VCA);
    }

    /* Add keys and iids to the iid table list */
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wIidIndex = pDataParams->wCurrentIidIndex;
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wKeyEncNumber = wKeyEncNumber;
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wKeyEncVersion = wKeyEncVersion;
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wKeyMacNumber = wKeyMacNumber;
    pDataParams->pIidTable[pDataParams->wCurrentIidTablePos].wKeyMacVersion = wKeyMacVersion;
    pDataParams->wCurrentIidTablePos++;
    pDataParams->wCurrentIidIndex++;

    /* Generate RNDQ */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoRng_Rnd(pDataParams->pCryptoRngDataParams, 12, &aTmpBuf[PHAL_VCA_POS_RNDQ]));

    /* buffer command frame */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        aTxBuffer,
        1,
        &pResponse,
        &wRxLength));

    /* buffer installation identifier */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_CONT,
        pIid,
        PHAL_VCA_IID_SIZE,
        &pResponse,
        &wRxLength));

    /* buffer RNDQ identifier */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_CONT,
        &aTmpBuf[PHAL_VCA_POS_RNDQ],
        12,
        &pResponse,
        &wRxLength));

    /* append PCDCaps and transmit the command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        &aTmpBuf[0],
        (uint16_t)(1U + (uint16_t)bLenCap),
        &pResponse,
        &wRxLength));

    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Int_ComputeErrorResponse(wRxLength, pResponse[0]));

    /* check response length */
    if (wRxLength != 1U /*Status */ + 16U /* Cryptogram */ + 8U /*MAC */)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_VCA);
    }

    /* Prepare MAC data */
    aTmpBuf[0] = pResponse[0];
    (void)memcpy(&aTmpBuf[PHAL_VCA_POS_PAYLOAD], &pResponse[1], 16);

    /* Iterate over all available key pairs and try to find a match */
    for (wIndex = pDataParams->wCurrentIidTablePos; wIndex > 0U; --wIndex)
    {
        /* Retrieve MAC key */
        PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(
            pDataParams->pKeyStoreDataParams,
            pDataParams->pIidTable[(wIndex-1u)].wKeyMacNumber,
            pDataParams->pIidTable[(wIndex-1u)].wKeyMacVersion,
            sizeof(aKey),
            aKey,
            &wKeyType));

        /* Check key type */
        if (wKeyType != PH_CRYPTOSYM_KEY_TYPE_AES128)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
        }

        /* Load the key */
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadKeyDirect(
            pDataParams->pCryptoDataParams,
            aKey,
            PH_CRYPTOSYM_KEY_TYPE_AES128));

        /* Set the correct MAC calculation mode */
        /* CMAC with Padding */
        /* Calculate the MAC */
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_CalculateMac(
            pDataParams->pCryptoDataParams,
            PH_CRYPTOSYM_MAC_MODE_CMAC,
            aTmpBuf,
            32,
            bMac,
            &bMacLength));

        /* Truncate the MAC */
        phalVca_Sw_Int_TruncateMac(bMac, bMac);

        /* Compare the MACs */
        if (memcmp(&pResponse[17], bMac, PHAL_VCA_TRUNCATED_MAC_SIZE) == 0)
        {
            pCardDataStorage = &pDataParams->pCardTable[pDataParams->wCurrentCardTablePos];
        }
        else
        {
            pCardDataStorage = &pDummyCardData;
        }

        /* In any case, we need to decrypt */
        pCardDataStorage->bValid = PHAL_VCA_CARD_TABLE_ENTRY_VALID;

        /* Retrieve ENC key */
        PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(
            pDataParams->pKeyStoreDataParams,
            pDataParams->pIidTable[(wIndex-1u)].wKeyEncNumber,
            pDataParams->pIidTable[(wIndex-1u)].wKeyEncVersion,
            sizeof(aKey),
            aKey,
            &wKeyType));

        /* Check key type */
        if (wKeyType != PH_CRYPTOSYM_KEY_TYPE_AES128)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
        }

        /* Load the key */
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadKeyDirect(
            pDataParams->pCryptoDataParams,
            aKey,
            PH_CRYPTOSYM_KEY_TYPE_AES128));

        /* Load first IV*/
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
            pDataParams->pCryptoDataParams,
            phalVca_Sw_FirstIv,
            PH_CRYPTOSYM_AES_BLOCK_SIZE));

        /* Perform decryption */
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_Decrypt(
            pDataParams->pCryptoDataParams,
            PH_CRYPTOSYM_CIPHER_MODE_CBC,
            &pResponse[1],
            PHAL_VCA_IID_SIZE,
            pCardDataStorage->pCardData
            ));

        /* Copy the found IID Index */
        pCardDataStorage->wIidIndex = pDataParams->pIidTable[(wIndex-1u)].wIidIndex;
    }

    pDataParams->wCurrentIidTablePos = 0;
    ++pDataParams->wCurrentCardTablePos;

    return PH_ERR_SUCCESS;
}
#endif /* NXPBUILD__PH_CRYPTOSYM */

phStatus_t phalVca_Sw_GetIidInfo(phalVca_Sw_DataParams_t * pDataParams, uint16_t wValidIidIndex, uint16_t * pIidIndex, uint8_t * pVcUidSize,
    uint8_t * pVcUid, uint8_t * pInfo, uint8_t * pPdCapabilities)
{
    phStatus_t PH_MEMLOC_REM statusTmp;

    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Sw_Int_ResolveValidIndex(
        pDataParams,
        wValidIidIndex,
        &wValidIidIndex));

    *pIidIndex = pDataParams->pCardTable[wValidIidIndex].wIidIndex;
    *pInfo = pDataParams->pCardTable[wValidIidIndex].pCardData[0];
    pPdCapabilities[0] = pDataParams->pCardTable[wValidIidIndex].pCardData[1];
    pPdCapabilities[1] = pDataParams->pCardTable[wValidIidIndex].pCardData[2];

    if (0U != (*pInfo & 0x80U))
    {
        *pVcUidSize = 4;
        (void)memcpy(pVcUid, &pDataParams->pCardTable[wValidIidIndex].pCardData[3], 4);
    }
    else
    {
        *pVcUidSize = 7;
        (void)memcpy(pVcUid, &pDataParams->pCardTable[wValidIidIndex].pCardData[3], 7);
    }

    return PH_ERR_SUCCESS;
}

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_IsoSelect(phalVca_Sw_DataParams_t * pDataParams, uint8_t   bSelectionControl, uint8_t bOption, uint8_t bDFnameLen,
    uint8_t * pDFname, uint8_t * pFCI, uint16_t * pwFCILen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bData[20];
    uint8_t     PH_MEMLOC_REM bLc = 0x00;
    uint8_t     PH_MEMLOC_REM bCmdBuff[PHAL_VCA_CMD_SIZE];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint8_t *   PH_MEMLOC_REM pRecv;
    uint16_t    PH_MEMLOC_REM wRxlen;

    if ((bDFnameLen == 0U) || (bDFnameLen > 16U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
    }
    if ((bOption != 0x00U) || (bSelectionControl != 0x04U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
    }

    /*
     * ReSet the Error code stored in pDataParams->wAdditionalInfo to default value as
     * PH_ERR_SUCCESS. This is to flush any error code stored as a part of previous cmd execution.
     */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Sw_SetConfig(pDataParams,PHAL_VCA_ADDITIONAL_INFO,PH_ERR_SUCCESS));

    /* Selection by DF Name */
    (void)memcpy(bData, pDFname, bDFnameLen);
    bLc = bDFnameLen;

    /* command formation */
    bCmdBuff[wCmdLen++] = 0x00;                 /* Class is always 0x00 */
    bCmdBuff[wCmdLen++] = PHAL_VCA_CMD_ISOSVC;  /* INS */
    bCmdBuff[wCmdLen++] = bSelectionControl;    /* P1 */
    bCmdBuff[wCmdLen++] = bOption;              /* P2 */


    /*
     * As per ISO-7816 Standard(2005), section 5.1,
     * A short Lc field consists of one byte not set to '00'.
     * From '01' to 'FF', the byte encodes Nc from one to 255.
     *
     * An extended Lc field consists of three bytes: one byte
     * set to '00' followed by two bytes not set to '0000'.From
     * '0001' to 'FFFF', the two bytes encode Nc from one to
     * 65535.
     */
    if(pDataParams->bExtendedLenApdu)
    {
        /* Extended Length APDU (3 bytes as Lc to be passed) */
        bCmdBuff[wCmdLen++] = 0x00;    /* first byte will be 0x00 if Ext apdu present. Next 2 byte contains actual data */
        bCmdBuff[wCmdLen++] = 0x00;    /* As of now this field will be set to 0x00 since maximum data that can be sent is 16 bytes. In case if data to be sent exceeds 255 bytes, this byte shall be used. */
        bCmdBuff[wCmdLen++] = bLc;     /* Actual data to be sent */
    }
    else
    {
       /* Short APDU( 1 byte of Lc to be passed)  */
       bCmdBuff[wCmdLen++] = bLc;  /* IID length; In this case it is DF Name */
    }

    /*  command exchange with (CLA INS P1 P2 Lc)*/
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));

    /* Data exchange */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_CONT,
        bData,
        bDFnameLen,
        &pRecv,
        &wRxlen
        ));

    wCmdLen = 0;

    /* As per ISO/IEC:7816-4(2005), Section 5, An extended Le field consists of either three bytes
     * (one byte set to '00' followed by two bytes with any
     * value) if the Lc field is absent, or two bytes (with any value) if an extended Lc field is present.
     */
    if(pDataParams->bExtendedLenApdu)
    {
        /*
         * Extended Length APDU (2 bytes as Le to be passed since Lc is present, Refer to iso7816-4 page no 13)
         * Hardcoding the Le value to 0x00 since Spec says this command will always have Le = 0x00
         */
        bCmdBuff[wCmdLen++] = 0x00;
        bCmdBuff[wCmdLen++] = 0x00;

    }
    else
    {
        /* Short Length APDU (1 byte as Le to be passed) */
        bCmdBuff[wCmdLen++] = 0x00;
    }
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
    /* Reset the EV2 authentication since the PICC moves its state from card specific state to VCA state. Henc
     * Any card authentication maintained will be reset by card. The same has to be done at reader side
     */
    if((pDataParams->eVCState == VC_DF_AUTH_D40) ||
       (pDataParams->eVCState == VC_DF_AUTH_ISO) ||
       (pDataParams->eVCState == VC_DF_AUTH_EV2) ||
       (pDataParams->eVCState == VC_DF_AUTH_AES))
    {
        if(pDataParams->pAlDataParams != NULL)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_ResetAuthentication(pDataParams->pAlDataParams));
        }
    }
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */
#ifdef NXPBUILD__PHAL_MFPEVX_SW
    /* Reset the MPFEV1 authentication since the PICC moves its state from card specific state to VCA state. Hence
     * Any card authentication maintained will be reset by card. The same has to be done at reader side.
     */
    if((pDataParams->eVCState == VC_MFP_AUTH_AES_SL1) ||
       (pDataParams->eVCState == VC_MFP_AUTH_AES_SL3))
    {
        if(pDataParams->pAlDataParams != NULL)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phalMfpEVx_ResetSecMsgState(pDataParams->pAlDataParams));
        }
    }
#endif /* NXPBUILD__PHAL_MFPEVX_SW */
    /* NOTE: As per Ref Arch Ver 0.15, the Le is given as 1,3(mandatory) and Lc is 1,3(mandatory). This is in contrast to
     * the ISO 7816-4 standard where it says Le can be 3 bytes when Lc is not present.
     * But as per ref arch Lc is always present which means that extended Le shall be always 2 bytes.
     */

    /* Command exchange with Le*/
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
    pDataParams->pPalMifareDataParams,
    PH_EXCHANGE_BUFFER_LAST,
    bCmdBuff,
    wCmdLen,
    &pRecv,
    &wRxlen
    ));

    statusTmp = pRecv[wRxlen - 2];
    statusTmp <<= 8;
    statusTmp |= pRecv[wRxlen - 1];

    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Int_ComputeErrorResponse_Extended(pDataParams, statusTmp));
    if (wRxlen != 0U)
    {
        /* After receiving the response, we need to update the VCState to VC_DF_NOT_AUTH */
        pDataParams->eVCState = VC_DF_NOT_AUTH;

        if (wRxlen == 0x02)
        {
            /* Since no MAC is returned, this indicates AuthVCMandatory flag is not set and IsoSelect is success
             * as per the ref arch(v 15) page no 283 */
            pDataParams->eVCState = VC_PROXIMITYCHECK;
            *pwFCILen = wRxlen - 2;
            return PH_ERR_SUCCESS;
        }
        else if(wRxlen == (PHAL_VCA_AUTH_RND_LEN + 6))
        {
            *pwFCILen = wRxlen - 2 ;    /* FCI Length should be 36 bytes excluding 2 bytes of status data */
            /* As per reference architecture, Handling the two response cases:
             * Case-1: [if TargetVC != NULL AND TargetVC.AuthVCMandatory == true] and
             * Case-2: [if TargetVC == NULL] in a single if-case since there is no way to get 'AuthVCMandatory' and 'targetted IID Supported' flag values */

            /* 4 Bytes Header + 16 Bytes RndChal + 16 Bytes Payload + 2 Bytes SW1||SW2 (Total 38 Bytes returned) */
            if ((pRecv[0]== 0x6F) && (pRecv[1]== 0x22) && (pRecv[2]== 0x85) && (pRecv[3]== 0x20))
            {
                if(memcpy(pFCI, &pRecv[0], 36) == NULL) /* Copy the entire 36 bytes of data (RndChal + Payload) */
                {
                     return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AL_VCA);
                }
            }
            else
            {   /* Case-3: [if TargetVC != NULL AND TargetVC.AuthVCMandatory == false AND (IID is DESFire application DF name)]
                 * FCI[36] bytes shall be stored in file ID 31 of the DF */
                memcpy(pFCI, &pRecv[0], 36);
            }
        }
        /* Case-3: [if TargetVC != NULL AND TargetVC.AuthVCMandatory == false AND (IID is DESFire application DF name)]
         * FCI[1...MaxFileSize] bytes shall be stored in file ID 31 of the DF + 2 bytes SW1||SW2 */
        else
        {
            /* FCI contains contents of FileID 31 of the DF */
            memcpy(pFCI, &pRecv[0], wRxlen-2);
            *pwFCILen = wRxlen - 2;
        }
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalVca_Sw_IsoExternalAuthenticate(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pInData, uint16_t wKeyNumber, uint16_t wKeyVersion)
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[PHAL_VCA_CMD_SIZE];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint8_t *   PH_MEMLOC_REM pRecv;
    uint16_t    PH_MEMLOC_REM wRxlen;
    uint8_t     PH_MEMLOC_REM aKey[PH_CRYPTOSYM_AES128_KEY_SIZE];
    uint16_t    PH_MEMLOC_REM wKeyType;
    uint8_t     PH_MEMLOC_REM bMac[PH_CRYPTOSYM_AES_BLOCK_SIZE];
    uint8_t     PH_MEMLOC_REM bMacLength;
    uint8_t     PH_MEMLOC_REM bIv[16];
    uint8_t     PH_MEMLOC_REM bIv_bak[PH_CRYPTOSYM_AES128_KEY_SIZE];
    /*
     * ReSet the Error code stored in pDataParams->wAdditionalInfo to default value as
     * PH_ERR_SUCCESS. This is to flush any error code stored as a part of previous cmd execution.
     */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Sw_SetConfig(pDataParams, PHAL_VCA_ADDITIONAL_INFO, PH_ERR_SUCCESS));

    /* Get Iso AUTH Key */
    PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNumber,
        wKeyVersion,
        sizeof(aKey),
        aKey,
        &wKeyType));

    /* Check key type */
    if (wKeyType != PH_CRYPTOSYM_KEY_TYPE_AES128)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
    }

    /* Load Iso AUTH Key */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadKeyDirect(
        pDataParams->pCryptoDataParams,
        aKey,
        PH_CRYPTOSYM_KEY_TYPE_AES128));

    /* Set the bIv_bak buffer to 0s */
    (void)memset(bIv_bak, 0x00, PH_CRYPTOSYM_AES128_KEY_SIZE);

    /* Take the back up of existing IV here */
    phhalVca_Sw_Int_BackupIV(pDataParams, bIv_bak);

    /* Reset IV for the first crypto operation */
    (void)memset(bIv, 0x00, PH_CRYPTOSYM_AES128_KEY_SIZE);

    /* Load zero Iv.*/
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        bIv,
        16));
    /* CMAC with Padding */
    /* mac calculation: CMAC(RndChal || VCData) */
    status = phCryptoSym_CalculateMac(
        pDataParams->pCryptoDataParams,
        PH_CRYPTOSYM_MAC_MODE_CMAC,
        pInData,
        PHAL_VCA_AUTH_RND_LEN,
        bMac,
        &bMacLength);
    if(status != PH_ERR_SUCCESS)
    {
        /* Load back up Iv */
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
            pDataParams->pCryptoDataParams,
            bIv_bak,
            PH_CRYPTOSYM_AES128_KEY_SIZE));
        return status;
    }
    /* Load the original IV back */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        bIv_bak,
        PH_CRYPTOSYM_AES128_KEY_SIZE));

    /* Perform MAC truncation: [0, 2, 4, 6, 8, 10, 12, 14] of the input Mac */
    phalVca_Sw_Int_TruncateMac(bMac, bMac);

    /* Prepare "IsoExternlAuthenticate" command */
    bCmdBuff[wCmdLen++] = 0x00;                     /* Class is always 0x00 */
    bCmdBuff[wCmdLen++] = PHAL_VCA_CMD_ISOEXT_AUTH; /* INS */
    bCmdBuff[wCmdLen++] = 0x00;                     /* P1 */
    bCmdBuff[wCmdLen++] = 0x00;                     /* P2 */

    /*
     * As per ISO-7816 Standard(2005), section 5.1,
     * A short Lc field consists of one byte not set to '00'.
     * From '01' to 'FF', the byte encodes Nc from one to 255.
     *
     * An extended Lc field consists of three bytes: one byte
     * set to '00' followed by two bytes not set to '0000'.From
     * '0001' to 'FFFF', the two bytes encode Nc from one to
     * 65535.
     */
    if(0U != (pDataParams->bExtendedLenApdu))
    {
        /* Extended Length APDU (3 bytes as Lc to be passed) */
        bCmdBuff[wCmdLen++] = 0x00;
        bCmdBuff[wCmdLen++] = 0x00;
        bCmdBuff[wCmdLen++] = 0x08;  /* As per ref arch ver 0.15, Response to Challenge  will be 8 bytes always.*/

    }
    else
    {
        /* Short Length APDU (1 byte as Lc to be passed) */
        bCmdBuff[wCmdLen++] = 0x08;     /* As per ref arch ver 0.15, Response to Challenge  will be 8 bytes always.*/
    }

    /* command exchange CLA INS P1 P2 Lc */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));

    /* command exchange Data(MAC) */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        bMac,
        PHAL_VCA_TRUNCATED_MAC_SIZE,
        &pRecv,
        &wRxlen
        ));

    statusTmp = pRecv[wRxlen - 2u];
    statusTmp <<= 8U;
    statusTmp |= pRecv[wRxlen - 1u];

    statusTmp = phalVca_Int_ComputeErrorResponse_Extended(pDataParams, statusTmp);

    if(statusTmp == PH_ERR_SUCCESS)
    {
        /* MAC is verified properly and is a success, so the VC State needs to be updated to proximity check */
        pDataParams->eVCState = VC_PROXIMITYCHECK;
    }
    else
    {
        /* MAC is verified properly and is not a success, so the VC State needs to be updated to proximity failed */
        pDataParams->eVCState = VC_PROXIMITYFAILED;
        return statusTmp;
    }
    return PH_ERR_SUCCESS;
}
#endif /* NXPBUILD__PH_CRYPTOSYM */




#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_ProximityCheck(phalVca_Sw_DataParams_t * pDataParams, uint8_t bGenerateRndC, uint8_t * pRndC, uint8_t bPps1,
    uint8_t bNumSteps, uint16_t wKeyNumber, uint16_t wKeyVersion, uint8_t * pUsedRndRC)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;

    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Sw_Cmd_PrepareProximityCheck(pDataParams));

    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Sw_Cmd_ProximityCheck(pDataParams, bGenerateRndC, pRndC, bNumSteps, pUsedRndRC));

    return (phalVca_Sw_Cmd_VerifyProximityCheck(pDataParams, pUsedRndRC, bPps1, wKeyNumber, wKeyVersion));
}

phStatus_t phalVca_Sw_ProximityCheckNew(phalVca_Sw_DataParams_t * pDataParams, uint8_t bGenerateRndC, uint8_t * pPrndC, uint8_t bNumSteps,
    uint16_t wKeyNumber, uint16_t wKeyVersion, uint8_t * pOption, uint8_t * pPubRespTime, uint8_t * pResponse, uint16_t * pRespLen,
    uint8_t * pCumRndRC)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;

    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Sw_Cmd_PrepareProximityCheckNew(pDataParams, pOption, pPubRespTime, pResponse, pRespLen));

    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Sw_Cmd_ProximityCheckNew(pDataParams, bGenerateRndC, pPrndC, bNumSteps, pPubRespTime, pCumRndRC));

    return (phalVca_Sw_Cmd_VerifyProximityCheckNew(pDataParams, *pOption, pPubRespTime, pResponse, *pRespLen, wKeyNumber, wKeyVersion, pCumRndRC));
}

phStatus_t phalVca_Sw_Cmd_PrepareProximityCheck(phalVca_Sw_DataParams_t * pDataParams)
{
    /* send "Prepare Proximity Check" command */
    return(phalVca_Int_PrepareProximityCheck(pDataParams->pPalMifareDataParams));
}

phStatus_t phalVca_Sw_Cmd_PrepareProximityCheckNew(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pOption,  uint8_t * pPubRespTime, uint8_t * pResponse,
    uint16_t * pRespLen)
{
    phStatus_t PH_MEMLOC_REM statusTmp;

    /* Before sending the PPC command, set the PC state as PC_PPC_IN_PROGRESS */
    pDataParams->ePCState = PC_PPC_IN_PROGRESS;

    /* send "Prepare Proximity Check" command */
    statusTmp = phalVca_Int_PrepareProximityCheckNew(pDataParams, pOption, pPubRespTime, pResponse, pRespLen);

    if(statusTmp == PH_ERR_SUCCESS)
    {
        /* PPC command is a success, set the PC state as PC_PCHK_PREPARED */
        pDataParams->ePCState = PC_PCHK_PREPARED;
    }
    else
    {
        /* PPC command is a failure, set the PC state back to PC_NO_PCHK_IN_PROGRESS */
        pDataParams->ePCState = PC_NO_PCHK_IN_PROGRESS;
    }

    return statusTmp;
}

phStatus_t phalVca_Sw_Cmd_ProximityCheck(phalVca_Sw_DataParams_t * pDataParams, uint8_t bGenerateRndC, uint8_t * pRndC, uint8_t bNumSteps,
    uint8_t * pUsedRndRC)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bRndC[PHAL_VCA_PC_RND_LEN];

    /* parameter checking */
    if (((bGenerateRndC == 0U) && (pRndC == NULL)) || (pUsedRndRC == NULL))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
    }


    /* check whether to generate RndC or not */
    if (0U != (bGenerateRndC))
    {
        pRndC = bRndC;
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoRng_Rnd(
            pDataParams->pCryptoRngDataParams,
            PHAL_VCA_PC_RND_LEN,
            pRndC
            ));
    }

    /* send "Proximity Check" command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Int_ProximityCheck(
        pDataParams->pPalMifareDataParams,
        bNumSteps,
        pRndC,
        pUsedRndRC));

    return PH_ERR_SUCCESS;
}

phStatus_t phalVca_Sw_Cmd_ProximityCheckNew(phalVca_Sw_DataParams_t * pDataParams, uint8_t bGenerateRndC, uint8_t * pPrndC, uint8_t bNumSteps,
    uint8_t * pPubRespTime, uint8_t * pCumRndRC)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bRndC[PHAL_VCA_PC_RND_LEN_NEW];

    /* parameter checking */
    if (((bGenerateRndC == 0U) && (pPrndC == NULL)) || (pCumRndRC == NULL))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
    }

    /* check whether to generate RndC or not */
    if (0U != (bGenerateRndC))
    {
        pPrndC = bRndC;
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoRng_Rnd(
            pDataParams->pCryptoRngDataParams,
            PHAL_VCA_PC_RND_LEN_NEW,
            pPrndC
            ));
    }
    /* Before sending the PC command, set the PC state as PC_PCHK_IN_PROGRESS */
    pDataParams->ePCState = PC_PCHK_IN_PROGRESS;

    /* send "Proximity Check" command */
    statusTmp = phalVca_Int_ProximityCheckNew(
        pDataParams,
        bNumSteps,
        pPubRespTime,
        pPrndC,
        pCumRndRC);

    if(statusTmp == PH_ERR_SUCCESS)
    {
        /* PC command is a success, set the PC state as PC_WAITING_PC_VERIFICATION */
        pDataParams->ePCState = PC_WAITING_PC_VERIFICATION;

        /* Update the state to ProximityCheck state in case of VC not authenticated. */
        if((pDataParams->eVCState != VC_DF_AUTH_EV2) && (pDataParams->eVCState != VC_DF_AUTH_AES) &&
            (pDataParams->eVCState != VC_MFP_AUTH_AES_SL1) && (pDataParams->eVCState != VC_MFP_AUTH_AES_SL3))
            pDataParams->eVCState = VC_PROXIMITYCHECK;
    }
    else
    {
        /* PC command is a failure, set the PC state back to PC_NO_PCHK_IN_PROGRESS */
        pDataParams->ePCState = PC_NO_PCHK_IN_PROGRESS;
        return statusTmp;
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalVca_Sw_Cmd_VerifyProximityCheck(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pRndRC, uint8_t bPps1, uint16_t wKeyNumber,
    uint16_t wKeyVersion)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;
    uint8_t     PH_MEMLOC_REM aTmpBuf[2U + PHAL_VCA_PC_RND_LEN * 2U];
    uint8_t     PH_MEMLOC_REM aKey[PH_CRYPTOSYM_AES128_KEY_SIZE];
    uint16_t    PH_MEMLOC_REM wKeyType;
    uint8_t     PH_MEMLOC_REM bMac[PH_CRYPTOSYM_AES_BLOCK_SIZE] = {0};
    uint8_t     PH_MEMLOC_REM bMacLength;

    /* prepare "Verify Proximity Check" command */
    aTmpBuf[0] = PHAL_VCA_CMD_VPC;
    aTmpBuf[1] = bPps1;
    (void)memcpy(&aTmpBuf[2], pRndRC, PHAL_VCA_PC_RND_LEN * 2U);

    /* Get Proximity Check Key */
    PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNumber,
        wKeyVersion,
        (uint8_t)(sizeof(aKey)),
        aKey,
        &wKeyType));

    /* Check key type */
    if (wKeyType != PH_CRYPTOSYM_KEY_TYPE_AES128)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
    }

    /* Load Proximity Check Key */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadKeyDirect(
        pDataParams->pCryptoDataParams,
        aKey,
        PH_CRYPTOSYM_KEY_TYPE_AES128));

    /* CMAC with Padding */
    /* mac calculation: CMAC(CMD || PPS1 || (RndRC1 || ... || RndRC14)) */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_CalculateMac(
        pDataParams->pCryptoDataParams,
        PH_CRYPTOSYM_MAC_MODE_CMAC,
        aTmpBuf,
        (2u + (PHAL_VCA_PC_RND_LEN * 2U)),
        bMac,
        &bMacLength));

    /* perform MAC truncation */
    phalVca_Sw_Int_TruncateMac(bMac, bMac);

    /* buffer the command code */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        aTmpBuf,
        1,
        &pResponse,
        &wRxLength));

    /* append the MAC and exchange frame */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        bMac,
        PHAL_VCA_TRUNCATED_MAC_SIZE,
        &pResponse,
        &wRxLength));

    /* check response */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Int_ComputeErrorResponse(wRxLength, pResponse[0]));

    /* check response length */
    if (wRxLength != 1U /* Status */ + 8U /* MAC */)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_VCA);
    }

    /* Calculate MAC */
    aTmpBuf[0] = pResponse[0];
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_CalculateMac(
        pDataParams->pCryptoDataParams,
        PH_CRYPTOSYM_MAC_MODE_CMAC,
        aTmpBuf,
        (2u + (PHAL_VCA_PC_RND_LEN * 2U)),
        bMac,
        &bMacLength));

    /* perform MAC truncation */
    phalVca_Sw_Int_TruncateMac(bMac, bMac);

    /* Compare MAC */
    if (memcmp(bMac, &pResponse[1], PHAL_VCA_TRUNCATED_MAC_SIZE) != 0)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_VCA_ERR_AUTH, PH_COMP_AL_VCA);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalVca_Sw_Cmd_VerifyProximityCheckNew(phalVca_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pPubRespTime, uint8_t * pResponse,
    uint16_t wRespLen, uint16_t wKeyNumber, uint16_t wKeyVersion, uint8_t * pRndCmdResp)
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t *   PH_MEMLOC_REM pResponseTmp;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;
    uint8_t     PH_MEMLOC_REM bCmd[9];
    uint8_t     PH_MEMLOC_REM aTmpBuf[15 /* Cmd / Rsp + Option + PubRspTime + PPS + ActBitRate */ + PHAL_VCA_PC_RND_LEN_NEW * 2];
    uint8_t     PH_MEMLOC_REM aTmpBufLen = 0;
    uint8_t     PH_MEMLOC_REM aKey[PH_CRYPTOSYM_AES128_KEY_SIZE];
    uint16_t    PH_MEMLOC_REM wKeyType;
    uint8_t     PH_MEMLOC_REM bMac[PH_CRYPTOSYM_AES_BLOCK_SIZE] = {0};
    uint8_t     PH_MEMLOC_REM bMacLength;
    uint8_t     PH_MEMLOC_REM bIv_bak[PH_CRYPTOSYM_AES128_KEY_SIZE];
    uint8_t     PH_MEMLOC_REM Key_bak[PH_CRYPTOSYM_MAX_BLOCK_SIZE];
    uint8_t     PH_MEMLOC_REM bRespCode;

    /* prepare "Verify Proximity Check" command */
    aTmpBuf[aTmpBufLen++] = PHAL_VCA_CMD_VPC;
    aTmpBuf[aTmpBufLen++] = bOption;
    aTmpBuf[aTmpBufLen++] = pPubRespTime[0];
    aTmpBuf[aTmpBufLen++] = pPubRespTime[1];

    /* Reset the aKey buffer */
    (void)memset(aKey, 0x00, PH_CRYPTOSYM_AES128_KEY_SIZE);

    /* bPps1 is optional, present if bit[0] of bOption is set */
    if(0U != (bOption & 0x01U))
    {
        aTmpBuf[aTmpBufLen++] = *pResponse;
    }

    /* ActBitRate is optional, present if bit[1] of bOption is set */
    if(bOption & 0x02)
    {
        (void)memcpy(&aTmpBuf[aTmpBufLen], pResponse, wRespLen);
        aTmpBufLen += (uint8_t) wRespLen;
    }

    (void)memcpy(&aTmpBuf[aTmpBufLen], pRndCmdResp, PHAL_VCA_PC_RND_LEN_NEW * 2U);

    switch(pDataParams->eVCState)
    {
        /* VCState = No VC Selected or Proximity Failed, use DummyMACKey(Random AES128 Key) for MACing */
        case VC_NOT_SELECTED:
        case VC_PROXIMITYFAILED:
            /* Generating a random DummyMACKey of 16 bytes length */
            PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoRng_Rnd(pDataParams->pCryptoRngDataParams,
                PH_CRYPTOSYM_AES128_KEY_SIZE, aKey));
            break;

        /* VCState = Proximity Check/Not Authenticated/D40 auth/ISO auth/EV1 auth without AES key,
         * use VCProximityKey(CardKeyNo = 0x21) for MACing
         */
        case VC_PROXIMITYCHECK:
        case VC_DF_NOT_AUTH:
        case VC_DF_AUTH_D40:
        case VC_DF_AUTH_ISO:
            /* Get VC ProximityCheckKey (CardKeyNo = 0x21) */
            PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(pDataParams->pKeyStoreDataParams,
                wKeyNumber, wKeyVersion, sizeof(aKey), aKey, &wKeyType));

            /* Check key type */
            if (wKeyType != PH_CRYPTOSYM_KEY_TYPE_AES128)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
            }
            break;

        /* VCState = Authenticated(AES or EV2) with AES keys, use Card Type Specific SessionAuthMACKey */
        case VC_DF_AUTH_EV2:
        case VC_DF_AUTH_AES:
        case VC_MFP_AUTH_AES_SL1:
        case VC_MFP_AUTH_AES_SL3:
            (void)memcpy(aKey, pDataParams->bSessionAuthMACKey, 16);
            break;
        default:
            break;
    }

    /* backup the current key */
    phhalVca_Sw_Int_BackupKey(pDataParams, Key_bak);

    /* Load the Key obtained from the switch cases above */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadKeyDirect(
        pDataParams->pCryptoDataParams,
        aKey,
        PH_CRYPTOSYM_KEY_TYPE_AES128));

    /* Store the IV to be used for encryption later */
    phhalVca_Sw_Int_BackupIV(pDataParams, bIv_bak);

    /* Reset IV for the first crypto operation */
    phhalVca_Sw_Int_ResetIV(pDataParams);

    /* Load Iv.*/
    status = phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
		phhalVca_Sw_Int_GetIV(pDataParams),
        PH_CRYPTOSYM_AES128_KEY_SIZE);
    if(status != PH_ERR_SUCCESS)
    {
        return status;
    }

    /* CMAC with Padding */
    /* mac calculation: CMAC(CMD || PPS1 || (RndRC1 || ... || RndRC16)) */
    status = phCryptoSym_CalculateMac(
        pDataParams->pCryptoDataParams,
        PH_CRYPTOSYM_MAC_MODE_CMAC,
        aTmpBuf,
        (aTmpBufLen + (PHAL_VCA_PC_RND_LEN_NEW * 2U)),
        bMac,
        &bMacLength);
    if(status != PH_ERR_SUCCESS)
    {
        /* Load back up Iv */
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
            pDataParams->pCryptoDataParams,
            bIv_bak,
            PH_CRYPTOSYM_AES128_KEY_SIZE));
        return status;
    }

    /* perform MAC truncation */
    phalVca_Sw_Int_TruncateMac(bMac, bMac);

    /* Before sending the VPC command, set the PC state as PC_VPC_IN_PROGRESS */
    pDataParams->ePCState = PC_VPC_IN_PROGRESS;

    /* Framing the buffer again */
    bCmd[0] = PHAL_VCA_CMD_VPC;

    /* Append the MAC before exchanging the frame */
    (void)memcpy(&bCmd[1], bMac,PHAL_VCA_TRUNCATED_MAC_SIZE);

    /* Check for ISO Wrapped Mode */
    if(0U != (pDataParams->bWrappedMode))
    {
        status = phalVca_Int_SendISOWrappedCmd(
            pDataParams,
            bCmd,
            PHAL_VCA_TRUNCATED_MAC_SIZE,    /* Lc Value */
            &pResponseTmp,
            &wRxLength
            );
        if(wRxLength >= 2U)
        {
            /* Store the SW2 of response code. This is required for response MAC calculation */
            bRespCode = pResponseTmp[wRxLength - 1u];
            status = phalVca_Int_ComputeErrorResponse_Extended(pDataParams, bRespCode);
            if(status != PH_ERR_SUCCESS)
            {
                /* Load back up Iv */
                PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
                    pDataParams->pCryptoDataParams,
                    bIv_bak,
                    PH_CRYPTOSYM_AES128_KEY_SIZE));
                return status;
            }
        }
        else
        {
            /* Load back up Iv */
            PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
                pDataParams->pCryptoDataParams,
                bIv_bak,
                PH_CRYPTOSYM_AES128_KEY_SIZE));
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_VCA);
        }
    }
    else
    {
        /* exchange frame in native mode */
        status = phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            bCmd,
            1 + PHAL_VCA_TRUNCATED_MAC_SIZE,
            &pResponseTmp,
            &wRxLength);
        if(status != PH_ERR_SUCCESS)
        {
            /* Load back up Iv */
            PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
                pDataParams->pCryptoDataParams,
                bIv_bak,
                PH_CRYPTOSYM_AES128_KEY_SIZE));
            return status;
        }

        /* Store the response code. This is required for response MAC calculation */
        bRespCode = pResponseTmp[0];

        /* Computing the error response on SW1 */
        status = phalVca_Int_ComputeErrorResponse_Extended(pDataParams, pResponseTmp[0]);
        if(status != PH_ERR_SUCCESS)
        {
            /* Load back up Iv */
            PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
                pDataParams->pCryptoDataParams,
                bIv_bak,
                PH_CRYPTOSYM_AES128_KEY_SIZE));
            return status;
        }
    }
    /* VPC command is a success or a failure, set the PC state back to PC_NO_PCHK_IN_PROGRESS */
    pDataParams->ePCState = PC_NO_PCHK_IN_PROGRESS;

    /* Reset IV for the first crypto operation */
    phhalVca_Sw_Int_ResetIV(pDataParams);

     /* Load Iv.*/
    status = phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
		phhalVca_Sw_Int_GetIV(pDataParams),
        PH_CRYPTOSYM_AES128_KEY_SIZE);
    if(status != PH_ERR_SUCCESS)
    {
        return status;
    }

    /* Calculate MAC this time with status code pResponse[0] for native response to match with PICC calculations */
    aTmpBuf[0] = bRespCode;

    /* Computing the MAC again for alignment with the computation of PICC */
    statusTmp = phCryptoSym_CalculateMac(
        pDataParams->pCryptoDataParams,
        PH_CRYPTOSYM_MAC_MODE_CMAC,
        aTmpBuf,
        (aTmpBufLen + PHAL_VCA_PC_RND_LEN_NEW * 2U),
        bMac,
        &bMacLength);

    /* perform MAC truncation */
    phalVca_Sw_Int_TruncateMac(bMac, bMac);

    /* Load back up Iv */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        bIv_bak,
        PH_CRYPTOSYM_AES128_KEY_SIZE));

    /* restore backup key */
    phhalVca_Sw_Int_RestoreKey(pDataParams, Key_bak);

    /* Compare MAC */
    if (memcmp(bMac, (((pDataParams->bWrappedMode) != 0U) ? &pResponseTmp[0] : &pResponseTmp[1]), PHAL_VCA_TRUNCATED_MAC_SIZE) != 0)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_VCA_ERR_AUTH, PH_COMP_AL_VCA);
    }

    return PH_ERR_SUCCESS;
}
#endif /* NXPBUILD__PH_CRYPTOSYM */

phStatus_t phalVca_Sw_Cmd_VerifyProximityCheckUtility(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pCmdMac, uint8_t * pCmdResp)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aTmpBuf[1];
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;
    /* Validating the Input MAC */
    if(pCmdMac == NULL)
    {
         return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AL_VCA);
    }
    /* Framing the buffer(Cmd code and Input MAC) and sending to PICC */
    aTmpBuf[0] = PHAL_VCA_CMD_VPC;

    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        aTmpBuf,
        1,
        &pResponse,
        &wRxLength));

    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        pCmdMac,
        8,
        &pResponse,
        &wRxLength));

    /* check response */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalVca_Int_ComputeErrorResponse_Extended(pDataParams, pResponse[0]));
    /* Copy the response MAC to pCmdResp */
    if(memcpy(pCmdResp, &pResponse[1], 8) == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AL_VCA);
    }

    return statusTmp;
}




phStatus_t phalVca_Sw_SetConfig(phalVca_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t wValue)
{
    switch (wConfig)
    {
        case PHAL_VCA_ADDITIONAL_INFO:
            pDataParams->wAdditionalInfo = wValue;
            break;
        case PHAL_VCA_WRAPPED_MODE:
            pDataParams->bWrappedMode = (uint8_t)wValue;
            break;
        case PHAL_VCA_TIMING_MODE:
            pDataParams->bOption = (uint8_t)wValue;
            break;
        case PHAL_VCA_PC_LOWER_THRESHOLD:
            pDataParams->bLowerBoundThreshold = (uint8_t)wValue;
            break;
        case PHAL_VCA_PC_EXTENDED_APDU:
            pDataParams->bExtendedLenApdu = (uint8_t)wValue;
            break;
        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_AL_VCA);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalVca_Sw_GetConfig(phalVca_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t * pValue)
{
    switch (wConfig)
    {
        case PHAL_VCA_ADDITIONAL_INFO:
            *pValue = pDataParams->wAdditionalInfo;
            break;
        case PHAL_VCA_WRAPPED_MODE:
            *pValue = pDataParams->bWrappedMode;
            break;
        case PHAL_VCA_TIMING_MODE:
            *pValue = pDataParams->bOption;
            break;
        case PHAL_VCA_PC_LOWER_THRESHOLD:
            *pValue = pDataParams->bLowerBoundThreshold;
            break;
        case PHAL_VCA_PC_EXTENDED_APDU:
            *pValue = pDataParams->bExtendedLenApdu;
            break;
        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_AL_VCA);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalVca_Sw_SetSessionKeyUtility(phalVca_Sw_DataParams_t * pDataParams, uint8_t * pSessionKey, uint8_t bAuthMode)
{
    /*
     * Handle all the cases as mentioned below before performing PC operations
     * case-1: VCState = Authenticated, Session keys from DesfireEV2 should be used for MAC calculations
     * case-2: VCState = Proximity Check or Not Authenticated, VC Proximity Key should be used for MAC calculations
     * case-3: VCState = No VC Selected, DummyMACKey of type AES 128 should be used for MAC calculations.
     */
    /* Handling Case-1. First need to get the card auth type(AES, DES, ISO, EV2) and set the VC state based on the Auth Mode */
    switch(bAuthMode)
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
    case PHAL_MFDFEVX_AUTHENTICATE:
        pDataParams->eVCState = VC_DF_AUTH_D40;
        break;

    case PHAL_MFDFEVX_AUTHENTICATEISO:
        pDataParams->eVCState = VC_DF_AUTH_ISO;
        break;

    case PHAL_MFDFEVX_AUTHENTICATEAES:
        pDataParams->eVCState = VC_DF_AUTH_AES;
        /* Set the session key (since session key is AES) to the VCA structure parameter */
        (void)memcpy(pDataParams->bSessionAuthMACKey, pSessionKey, sizeof(pDataParams->bSessionAuthMACKey));
        break;

    case PHAL_MFDFEVX_AUTHENTICATEEV2:
        pDataParams->eVCState = VC_DF_AUTH_EV2;
        /* Set the session key (since session key is AES) to the VCA structure parameter */
        (void)memcpy(pDataParams->bSessionAuthMACKey, pSessionKey, sizeof(pDataParams->bSessionAuthMACKey));
        break;

    case PHAL_MFDFEVX_NOT_AUTHENTICATED:
        pDataParams->eVCState = VC_DF_NOT_AUTH;
        break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */
#ifdef NXPBUILD__PHAL_MFPEVX_SW
    case PHAL_MFPEVX_SL1_MFP_AUTHENTICATED:
        pDataParams->eVCState = VC_MFP_AUTH_AES_SL1;

        /* Set the session key (since session key is AES) to the VCA structure parameter */
        (void)memcpy(pDataParams->bSessionAuthMACKey, pSessionKey, sizeof(pDataParams->bSessionAuthMACKey));
        break;

    case PHAL_MFPEVX_SL3_MFP_AUTHENTICATED:
        pDataParams->eVCState = VC_MFP_AUTH_AES_SL3;

        /* Set the session key (since session key is AES) to the VCA structure parameter */
        (void)memcpy(pDataParams->bSessionAuthMACKey, pSessionKey, sizeof(pDataParams->bSessionAuthMACKey));
        break;

    case PHAL_MFPEVX_NOTAUTHENTICATED:
    case PHAL_MFPEVX_NOT_AUTHENTICATED_L3:
    case PHAL_MFPEVX_NOT_AUTHENTICATED_L4:
        pDataParams->eVCState = VC_DF_NOT_AUTH;
        break;
#endif /* NXPBUILD__PHAL_MFPEVX_SW */
    default:
        pDataParams->eVCState = VC_NOT_SELECTED;
        break;
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalVca_Sw_SetApplicationType(phalVca_Sw_DataParams_t * pDataParams, void * pAlDataParams)
{
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pAlDataParams);

    pDataParams->pAlDataParams = pAlDataParams;

    return PH_ERR_SUCCESS;
}

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalVca_Sw_DecryptResponse(phalVca_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVersion, uint8_t * pInData,
    uint8_t * pRandChal, uint8_t * pVCData)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aKey[PH_CRYPTOSYM_AES128_KEY_SIZE];
    uint8_t     PH_MEMLOC_REM bInDataLen = (uint8_t) strlen((const char *) pInData);
    uint16_t    PH_MEMLOC_REM wKeyType;
    uint8_t     PH_MEMLOC_REM bIv[16];
    uint8_t     PH_MEMLOC_REM bIv_bak[PH_CRYPTOSYM_AES128_KEY_SIZE];
    uint8_t     PH_MEMLOC_REM bIndex;

    /* Set the buffer to 0: MISRA warning */
    (void)memset(bIv_bak, 0x00, PH_CRYPTOSYM_AES128_KEY_SIZE);

    /* Check for pointer to Input data */
    if((pInData == NULL) && (bInDataLen != PHAL_VCA_AUTH_RND_LEN))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AL_VCA);
    }
    /* The inout data contains 4 bytes header + 32 bytes of Data(16B challenge & 16B payload */
    /* Store 16 bytes of the Input data (Response of IsoSelect) to Random challenge(RndChal) */
    if(memcpy(pRandChal, &pInData[4], 16) == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AL_VCA);
    }

    /* Get Select ENC Key */
    PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVersion,
        (uint8_t)(sizeof(aKey)),
        aKey,
        &wKeyType));

    /* Check key type */
    if (wKeyType != PH_CRYPTOSYM_KEY_TYPE_AES128)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
    }

    /* Load Select ENC Key */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadKeyDirect(
        pDataParams->pCryptoDataParams,
        aKey,
        PH_CRYPTOSYM_KEY_TYPE_AES128));

    /* Store the IV to be used for Crypto operations later */
    phhalVca_Sw_Int_BackupIV(pDataParams, bIv_bak);

    /* Reset IV for the first crypto operation */
    (void)memset(bIv, 0x00, PH_CRYPTOSYM_AES128_KEY_SIZE);

    /* Load zero Iv.*/
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        bIv,
        16));

    /* Decryption of the response data received from IsoSelect */
    statusTmp = phCryptoSym_Decrypt(
        pDataParams->pCryptoDataParams,
        PH_CRYPTOSYM_CIPHER_MODE_CBC,
        &pInData[20],
        16,
        pVCData);
    if(statusTmp != PH_ERR_SUCCESS)
    {
        /* Load back up Iv */
        PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
            pDataParams->pCryptoDataParams,
            bIv_bak,
            PH_CRYPTOSYM_AES128_KEY_SIZE));
        return statusTmp;
    }
    /* Decrypted VCData is not plain text. It needs to be XORed with RndChal to obtain plain VCData */
    for(bIndex =0; bIndex < 16U; bIndex++)
    {
        pVCData[bIndex] = (pVCData[bIndex] ^ pRandChal[bIndex]);
    }
    /* Store the decrypted VCData to pCardData paramter of VCA structure for futher access to info, PDCap.1 and VCUID */
    if(memcpy(pDataParams->pCardTable->pCardData, pVCData, 16) == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AL_VCA);
    }

    return PH_ERR_SUCCESS;
}
#endif /* NXPBUILD__PH_CRYPTOSYM */

phStatus_t phalVca_Sw_Int_ResolveValidIndex(phalVca_Sw_DataParams_t * pDataParams, uint16_t wIidIndex, uint16_t * pValidIndex)
{
    uint8_t     PH_MEMLOC_REM bCurrentValidIndex = 0;
    uint8_t     PH_MEMLOC_REM bDummyValidIndex = 0;
    uint8_t     PH_MEMLOC_REM bDummyFoundIndex = 0;
    uint8_t     PH_MEMLOC_REM bIndex;
    uint16_t    PH_MEMLOC_REM bFoundIndex = pDataParams->wCurrentCardTablePos;

    for (bIndex = 0; bIndex < pDataParams->wCurrentCardTablePos; ++bIndex)
    {
        if (pDataParams->pCardTable[bIndex].bValid == 0xFF)
        {
            if (bCurrentValidIndex == wIidIndex)
            {
                bDummyFoundIndex = bIndex;
            }
            else
            {
                bDummyFoundIndex = bIndex;
            }
            bDummyValidIndex++;
        }
        else
        {
            if (bCurrentValidIndex == wIidIndex)
            {
                bFoundIndex = bIndex;
            }
            else
            {
                bDummyFoundIndex = bIndex;
            }
            bCurrentValidIndex++;
        }
    }

    /* Check if the index is valid */
    if (bFoundIndex >= pDataParams->wCurrentCardTablePos)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_VCA);
    }

    *pValidIndex = bFoundIndex;

    PH_UNUSED_VARIABLE(bDummyFoundIndex);
    return PH_ERR_SUCCESS;
}

#ifdef NXPBUILD__PH_CRYPTOSYM
void phalVca_Sw_Int_TruncateMac(uint8_t * pMac, uint8_t * pTruncatedMac)
{
    uint8_t PH_MEMLOC_REM bIndex;

    /* truncated MAC = [1, 3, 5, 7, 9, 11, 13, 15] of the input Mac */
    for (bIndex = 0; bIndex < PHAL_VCA_TRUNCATED_MAC_SIZE; ++bIndex)
    {
        pTruncatedMac[bIndex] = pMac[1 + (bIndex << 1)];
    }
}

void phhalVca_Sw_Int_BackupIV(phalVca_Sw_DataParams_t * pDataParams, uint8_t *pIv_Bak)
{
    uint8_t * pIV_Tmp = NULL;

    switch(PH_GET_COMPID(pDataParams->pCryptoDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            pIV_Tmp = ((phCryptoSym_Sw_DataParams_t *) pDataParams->pCryptoDataParams)->pIV;
            if(pIV_Tmp != NULL)
            {
                (void) memcpy(pIv_Bak, pIV_Tmp, PH_CRYPTOSYM_AES128_KEY_SIZE);
            }
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */


        default:
            break;
    }
}

void phhalVca_Sw_Int_ResetIV(phalVca_Sw_DataParams_t * pDataParams)
{
    uint8_t * pIV_Tmp = NULL;

    switch(PH_GET_COMPID(pDataParams->pCryptoDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            pIV_Tmp = ((phCryptoSym_Sw_DataParams_t *) pDataParams->pCryptoDataParams)->pIV;

            (void) memset(pIV_Tmp, 0x00, PH_CRYPTOSYM_AES128_KEY_SIZE);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */


        default:
            break;
    }
}

uint8_t * phhalVca_Sw_Int_GetIV(phalVca_Sw_DataParams_t * pDataParams)
{
    uint8_t * pIV_Tmp = NULL;

    switch(PH_GET_COMPID(pDataParams->pCryptoDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            pIV_Tmp = ((phCryptoSym_Sw_DataParams_t *) pDataParams->pCryptoDataParams)->pIV;
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */


        default:
            break;
    }

    return pIV_Tmp;
}

void phhalVca_Sw_Int_BackupKey(phalVca_Sw_DataParams_t * pDataParams, uint8_t *pKey_Bak)
{
    uint8_t * pKey_Tmp = NULL;

    switch(PH_GET_COMPID(pDataParams->pCryptoDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            pKey_Tmp = ((phCryptoSym_Sw_DataParams_t *) pDataParams->pCryptoDataParams)->pKey;
            if(pKey_Tmp != NULL)
            {
                (void) memcpy(pKey_Bak, pKey_Tmp, PH_CRYPTOSYM_MAX_BLOCK_SIZE);
            }
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */


        default:
            break;
    }
}

void phhalVca_Sw_Int_RestoreKey(phalVca_Sw_DataParams_t * pDataParams, uint8_t *pKey)
{
    uint8_t * pKey_Tmp = NULL;

    switch(PH_GET_COMPID(pDataParams->pCryptoDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            pKey_Tmp = ((phCryptoSym_Sw_DataParams_t *) pDataParams->pCryptoDataParams)->pKey;
            if(pKey_Tmp != NULL)
            {
                (void) memcpy(pKey_Tmp, pKey, PH_CRYPTOSYM_MAX_BLOCK_SIZE);
            }
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */


        default:
            break;
    }
}
#endif /* NXPBUILD__PH_CRYPTOSYM */

#endif /* NXPBUILD__PHAL_VCA_SW */
