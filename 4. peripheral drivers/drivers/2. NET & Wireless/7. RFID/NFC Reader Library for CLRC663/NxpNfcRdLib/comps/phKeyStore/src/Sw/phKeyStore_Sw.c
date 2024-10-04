/*----------------------------------------------------------------------------*/
/* Copyright 2006-2013,2021-2023 NXP                                          */
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
* Software KeyStore Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6671 $ (v07.09.00)
* $Date: 2022-08-07 19:39:47 +0530 (Sun, 07 Aug 2022) $
*
* History:
*  CHu: Generated 29. May 2009
*
*/

#include <ph_Status.h>
#include <phKeyStore.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PH_KEYSTORE_SW

#include "phKeyStore_Sw.h"
#include "phKeyStore_Sw_Int.h"

phStatus_t phKeyStore_Sw_Init(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wSizeOfDataParams, phKeyStore_Sw_KeyEntry_t * pKeyEntries,
    uint16_t wNoOfKeyEntries, phKeyStore_Sw_KeyVersionPair_t * pKeyVersionPairs, uint16_t wNoOfVersionPairs, phKeyStore_Sw_KUCEntry_t * pKUCEntries,
    uint16_t wNoOfKUCEntries)
{
    uint16_t wEntryIndex;
    uint16_t wPos;
    phStatus_t wStatus;
    phKeyStore_Sw_KeyVersionPair_t * pKeyVersion;

    if(sizeof(phKeyStore_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
    }
    PH_ASSERT_NULL(pDataParams);
    PH_ASSERT_NULL(pKeyEntries);
    PH_ASSERT_NULL(pKeyVersionPairs);
    PH_ASSERT_NULL(pKUCEntries);

    /* Init private data */
    pDataParams->wId = PH_COMP_KEYSTORE | PH_KEYSTORE_SW_ID;
    pDataParams->pKeyEntries = pKeyEntries;
    pDataParams->pKeyVersionPairs = pKeyVersionPairs;
    pDataParams->wNoOfKeyEntries = wNoOfKeyEntries;
    pDataParams->wNoOfVersions = wNoOfVersionPairs;
    pDataParams->pKUCEntries = pKUCEntries;
    pDataParams->wNoOfKUCEntries = wNoOfKUCEntries;

    for(wEntryIndex = 0; wEntryIndex < pDataParams->wNoOfKeyEntries; wEntryIndex++)
    {
        pDataParams->pKeyEntries[wEntryIndex].wKeyType = PH_KEYSTORE_INVALID_ID;
        pDataParams->pKeyEntries[wEntryIndex].wRefNoKUC = PH_KEYSTORE_INVALID_ID;

        for(wPos = 0; wPos < pDataParams->wNoOfVersions; wPos++)
        {
            PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_Sw_GetKeyValuePtrPos(pDataParams, wEntryIndex, wPos, &pKeyVersion));
            pKeyVersion->wVersion = PH_KEYSTORE_DEFAULT_ID;


        }
    }

    for(wEntryIndex = 0; wEntryIndex < pDataParams->wNoOfKUCEntries; wEntryIndex++)
    {
        pDataParams->pKUCEntries[wEntryIndex].dwLimit = 0xFFFFFFFFU;
        pDataParams->pKUCEntries[wEntryIndex].dwCurVal = 0;
    }

    return PH_ERR_SUCCESS;
}





/* Common Interfaces ------------------------------------------------------------------------------------------------------------------- */
phStatus_t phKeyStore_Sw_FormatKeyEntry(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wNewKeyType)
{
    phStatus_t wStatus;
    uint8_t    bPos;
    phKeyStore_Sw_KeyVersionPair_t * pKeyPair;
    /* Overflow checks */
    if(wKeyNo >= pDataParams->wNoOfKeyEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    switch(wNewKeyType)
    {
        case PH_KEYSTORE_KEY_TYPE_AES128:
        case PH_KEYSTORE_KEY_TYPE_DES:
        case PH_KEYSTORE_KEY_TYPE_2K3DES:
        case PH_KEYSTORE_KEY_TYPE_MIFARE:
        case PH_KEYSTORE_KEY_TYPE_AES192:
        case PH_KEYSTORE_KEY_TYPE_3K3DES:
        case PH_KEYSTORE_KEY_TYPE_AES256:
            break;


        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    pDataParams->pKeyEntries[wKeyNo].wKeyType = wNewKeyType;

    /* Reset CEK to master Key */
    pDataParams->pKeyEntries[wKeyNo].wRefNoKUC = PH_KEYSTORE_INVALID_ID;

    /* Reset all keys to 0x00*/
    for(bPos = 0; bPos < pDataParams->wNoOfVersions; ++bPos)
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_Sw_GetKeyValuePtrPos(pDataParams, wKeyNo, bPos, &pKeyPair));
        pKeyPair->wVersion = 0;

        (void) memset(pKeyPair->pKey, 0x00, PH_KEYSTORE_MAX_KEY_SIZE);

    }

    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_SetKUC(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wRefNoKUC)
{
    /* Overflow checks */
    if(wKeyNo >= pDataParams->wNoOfKeyEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }
    /* Check for a valid KUC entry */
    if(wRefNoKUC >= pDataParams->wNoOfKUCEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }
    pDataParams->pKeyEntries[wKeyNo].wRefNoKUC = wRefNoKUC;

    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_GetKUC(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wRefNoKUC, uint32_t * pdwLimit,
    uint32_t * pdwCurVal)
{
    /* Overflow checks */
    if(wRefNoKUC >= pDataParams->wNoOfKUCEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    *pdwLimit = pDataParams->pKUCEntries[wRefNoKUC].dwLimit;
    *pdwCurVal = pDataParams->pKUCEntries[wRefNoKUC].dwCurVal;

    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_ChangeKUC(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wRefNoKUC, uint32_t dwLimit)
{
    /* Overflow checks */
    if(wRefNoKUC >= pDataParams->wNoOfKUCEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }
    pDataParams->pKUCEntries[wRefNoKUC].dwLimit = dwLimit;

    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_SetConfig(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t wValue)
{
    /* satisfy compiler */
    if(pDataParams || wConfig || wValue)
        {
        ;/*do nothing*/
        }
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_KEYSTORE);
}

phStatus_t phKeyStore_Sw_SetConfigStr(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint8_t *pBuffer,
    uint16_t wBuffLen)
{
    /* satisfy compiler */
    if(pDataParams || wConfig || pBuffer || wBuffLen)
        {
        ;/*do nothing*/
        }
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_KEYSTORE);
}

phStatus_t phKeyStore_Sw_GetConfig(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t * pValue)
{
    /* satisfy compiler */
    if(pDataParams || wConfig || pValue)
        {
        ;/*do nothing*/
        }
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_KEYSTORE);
}

phStatus_t phKeyStore_Sw_GetConfigStr(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint8_t ** ppBuffer,
    uint16_t * pBuffLen)
{
    /* satisfy compiler */
    if(pDataParams || wConfig || ppBuffer || pBuffLen)
        {
        ;/*do nothing*/
        }
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_KEYSTORE);
}





/* Interfaces for Symmetric Keys ------------------------------------------------------------------------------------------------------- */
phStatus_t phKeyStore_Sw_SetKey(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVer,
    uint16_t wKeyType, uint8_t * pNewKey, uint16_t wNewKeyVer)
{
    phStatus_t wStatus;
    phKeyStore_Sw_KeyVersionPair_t * pKeyVer;
    PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_Sw_GetKeyValuePtrVersion(pDataParams, wKeyNo, wKeyVer, &pKeyVer));

    /* Check that Key type matches with current Key Type format */
    if(pDataParams->pKeyEntries[wKeyNo].wKeyType != wKeyType)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    /* copy the key and version */
    (void) memcpy(pKeyVer->pKey, pNewKey, phKeyStore_GetKeySize(wKeyType));
    pKeyVer->wVersion = wNewKeyVer;

    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_SetKeyAtPos(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wPos, uint16_t wKeyType,
    uint8_t * pNewKey, uint16_t wNewKeyVer)
{
    phStatus_t wStatus;
    phKeyStore_Sw_KeyVersionPair_t * pKeyVer;
    PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_Sw_GetKeyValuePtrPos(pDataParams, wKeyNo, wPos, &pKeyVer));

    /* Check that Key type matches with current Key Type format */
    if(pDataParams->pKeyEntries[wKeyNo].wKeyType != wKeyType)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    /* copy the key and version */
    (void) memcpy(pKeyVer->pKey, pNewKey, phKeyStore_GetKeySize(wKeyType));
    pKeyVer->wVersion = wNewKeyVer;

    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_SetFullKeyEntry(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wNoOfKeys, uint16_t wKeyNo,
    uint16_t wNewRefNoKUC, uint16_t wNewKeyType, uint8_t * pNewKeys, uint16_t * pNewKeyVerList)
{
    phStatus_t wStatus;
    uint8_t    bPos;
    uint8_t bKeyLen;
    phKeyStore_Sw_KeyVersionPair_t * pKeyVer;

    /* Overflow checks */
    if(wKeyNo >= pDataParams->wNoOfKeyEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }
    /* Check for a valid KUC entry */
    if(wNewRefNoKUC >= pDataParams->wNoOfKUCEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }
    /* Overflow checks */
    if(wNoOfKeys > pDataParams->wNoOfVersions)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    switch(wNewKeyType)
    {
        case PH_KEYSTORE_KEY_TYPE_AES128:
        case PH_KEYSTORE_KEY_TYPE_2K3DES:
        case PH_KEYSTORE_KEY_TYPE_AES192:
        case PH_KEYSTORE_KEY_TYPE_3K3DES:
        case PH_KEYSTORE_KEY_TYPE_AES256:
        case PH_KEYSTORE_KEY_TYPE_DES:
        case PH_KEYSTORE_KEY_TYPE_MIFARE:
            bKeyLen = (uint8_t) phKeyStore_GetKeySize(wNewKeyType);
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    pDataParams->pKeyEntries[wKeyNo].wKeyType = wNewKeyType;

    /* Reset KUC to master Key */
    pDataParams->pKeyEntries[wKeyNo].wRefNoKUC = wNewRefNoKUC;

    /* Reset all keys to 0x00*/
    for(bPos = 0; bPos < wNoOfKeys; bPos++)
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_Sw_GetKeyValuePtrPos(pDataParams, wKeyNo, bPos, &pKeyVer));
        pKeyVer->wVersion = pNewKeyVerList[bPos];
        (void) memcpy(pKeyVer->pKey, &pNewKeys[bPos * bKeyLen], bKeyLen);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_GetKeyEntry(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVerBufSize,
    uint16_t * wKeyVer, uint16_t * wKeyVerLen, uint16_t * pKeyType)
{
    phStatus_t wStatus;
    uint16_t bPos;
    phKeyStore_Sw_KeyVersionPair_t * pKeyVer;

    /* Check for overflow */
    if(wKeyVerBufSize < (sizeof(uint16_t) * pDataParams->wNoOfVersions))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_KEYSTORE);
    }

    for(bPos = 0; bPos < pDataParams->wNoOfVersions; bPos++)
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_Sw_GetKeyValuePtrPos(pDataParams, wKeyNo, bPos, &pKeyVer));
        wKeyVer[bPos] = pKeyVer->wVersion;
    }
    *wKeyVerLen = pDataParams->wNoOfVersions;
    *pKeyType = pDataParams->pKeyEntries[wKeyNo].wKeyType;

    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_GetKey(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVer, uint8_t bKeyBufSize,
    uint8_t * pKey, uint16_t * pKeyType)
{
    phStatus_t wStatus;
    uint16_t wKeySize;
    phKeyStore_Sw_KeyVersionPair_t * pKeyVer;
    PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_Sw_GetKeyValuePtrVersion(pDataParams, wKeyNo, wKeyVer, &pKeyVer));

    /* Check for Counter overflow */
    PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_Sw_CheckUpdateKUC(pDataParams, pDataParams->pKeyEntries[wKeyNo].wRefNoKUC));

    /* check buffer size */
    wKeySize = phKeyStore_GetKeySize(pDataParams->pKeyEntries[wKeyNo].wKeyType);
    if(bKeyBufSize < wKeySize)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_KEYSTORE);
    }
    /* copy the key */
    (void) memcpy(pKey, pKeyVer->pKey, wKeySize);

    *pKeyType = pDataParams->pKeyEntries[wKeyNo].wKeyType;
    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_GetKeyValuePtrVersion(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVer,
    phKeyStore_Sw_KeyVersionPair_t ** pKeyVer)
{
    uint16_t bPos;
    *pKeyVer = NULL;
    /* Overflow checks */
    if(wKeyNo >= pDataParams->wNoOfKeyEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    for(bPos = 0; bPos < pDataParams->wNoOfVersions; bPos++)
    {
        *pKeyVer = &pDataParams->pKeyVersionPairs[wKeyNo * pDataParams->wNoOfVersions + bPos];
        if((*pKeyVer)->wVersion == wKeyVer)
        {
            break;
        }
    }
    /* No entry found */
    if(bPos == pDataParams->wNoOfVersions)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phKeyStore_Sw_CheckUpdateKUC(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wKeyUsageCtrNumber)
{
    if(wKeyUsageCtrNumber != PH_KEYSTORE_INVALID_ID)
    {
        /* Check for a valid KUC entry */
        if(wKeyUsageCtrNumber >= pDataParams->wNoOfKUCEntries)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
        }

        /* Now let's check the limit*/
        if(pDataParams->pKUCEntries[wKeyUsageCtrNumber].dwCurVal >= pDataParams->pKUCEntries[wKeyUsageCtrNumber].dwLimit)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_KEY, PH_COMP_KEYSTORE);
        }
        pDataParams->pKUCEntries[wKeyUsageCtrNumber].dwCurVal++;
    }
    return PH_ERR_SUCCESS;
}






phStatus_t phKeyStore_Sw_GetKeyValuePtrPos(phKeyStore_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wPos,
    phKeyStore_Sw_KeyVersionPair_t ** pKeyVersion)
{
    *pKeyVersion = NULL;
    /* Overflow checks */
    if(wKeyNo >= pDataParams->wNoOfKeyEntries)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    /* Overflow checks */
    if(wPos >= pDataParams->wNoOfVersions)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_KEYSTORE);
    }

    *pKeyVersion = &pDataParams->pKeyVersionPairs[wKeyNo * pDataParams->wNoOfVersions + wPos];

    return PH_ERR_SUCCESS;
}

#endif /* NXPBUILD__PH_KEYSTORE_SW */
