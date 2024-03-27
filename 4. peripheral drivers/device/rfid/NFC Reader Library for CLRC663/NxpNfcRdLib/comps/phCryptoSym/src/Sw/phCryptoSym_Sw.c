/*----------------------------------------------------------------------------*/
/* Copyright 2009-2019,2022,2023 NXP                                          */
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
* Software specific Crypto-Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6645 $ (v07.09.00)
* $Date: 2022-06-06 15:15:13 +0530 (Mon, 06 Jun 2022) $
*
* History:
*  SLe: Generated 01.12.2009
*
*/

#include <ph_Status.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PH_KEYSTORE
#include <phKeyStore.h>
#endif /* #ifdef NXPBUILD__PH_KEYSTORE */

#include <phCryptoSym.h>

#ifdef NXPBUILD__PH_CRYPTOSYM_SW

#include "phCryptoSym_Sw.h"

#ifdef PH_CRYPTOSYM_SW_AES
#include "phCryptoSym_Sw_Aes.h"
#endif /* PH_CRYPTOSYM_SW_AES*/

#ifdef PH_CRYPTOSYM_SW_DES
#include "phCryptoSym_Sw_Des.h"
#endif /* PH_CRYPTOSYM_SW_DES */


#include "phCryptoSym_Sw_Int.h"

phStatus_t phCryptoSym_Sw_Init(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wSizeOfDataParams, void * pKeyStoreDataParams)
{
    phStatus_t wStatus = 0;
    if(sizeof(phCryptoSym_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
    }
    PH_ASSERT_NULL (pDataParams);

    /* Init. private data */
    pDataParams->wId = PH_COMP_CRYPTOSYM | PH_CRYPTOSYM_SW_ID;
    pDataParams->pKeyStoreDataParams = pKeyStoreDataParams;


    /* Invalidate keys */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_InvalidateKey(pDataParams));

    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_Sw_InvalidateKey(phCryptoSym_Sw_DataParams_t * pDataParams)
{
    /* Reset all the key storage */
    (void) memset(pDataParams->pKey, 0x00, (size_t) sizeof(pDataParams->pKey));
    (void) memset(pDataParams->pIV, 0x00, (size_t) sizeof(pDataParams->pIV));

    pDataParams->wKeyType = PH_CRYPTOSYM_KEY_TYPE_INVALID;
    pDataParams->wKeepIV = PH_CRYPTOSYM_VALUE_KEEP_IV_OFF;
    pDataParams->wAddInfo = 0x00;

#ifndef PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION
    pDataParams->bCMACSubKeysInitialized = PH_OFF;
    (void) memset(pDataParams->pCMACSubKey1, 0x00, (size_t) sizeof(pDataParams->pCMACSubKey1));
    (void) memset(pDataParams->pCMACSubKey2, 0x00, (size_t) sizeof(pDataParams->pCMACSubKey2));
#endif /* PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION */

    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_Sw_Encrypt(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, const uint8_t * pPlainBuff, uint16_t wBuffLen,
    uint8_t * pEncBuff)
{
    phStatus_t wStatus = 0;
    uint16_t wBlockSize = 0;
    uint16_t wIndex_Buff = 0;
    uint8_t bIndex_BlockSize = 0;
    uint8_t * pIv = NULL;


#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
    uint8_t PH_CRYTOSYM_SW_FAST_RAM pHelperBuffer[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE];
#endif /* PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE */


    /* Get the block size of the currently loaded key */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_GetConfig(pDataParams, PH_CRYPTOSYM_CONFIG_BLOCK_SIZE, &wBlockSize));

    /* Check that the input buffer length is a multiple of the block size; */
    if (0U != (wBuffLen % wBlockSize))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    /* Set the IV to the iv specified in the private data params */
    pIv = pDataParams->pIV;

    /*Iterate over all blocks and perform the encryption*/
    wIndex_Buff = 0;
    while(wIndex_Buff < wBuffLen)
    {
        /* Is the output array the same as the input array? Else we need to recopy the plaintext upfronjt */
        if(pPlainBuff != pEncBuff)
        {
            (void) memcpy(&pEncBuff[wIndex_Buff], &pPlainBuff[wIndex_Buff], wBlockSize);
        }

        /* In case of CBC mode, we need to perform the XOR with the previous cipher block */
        switch((uint8_t) (wOption))
        {
            case PH_CRYPTOSYM_CIPHER_MODE_CBC:
            case PH_CRYPTOSYM_CIPHER_MODE_CBC_DF4:
                for(bIndex_BlockSize = 0; bIndex_BlockSize < wBlockSize; bIndex_BlockSize++)
                {
                    pEncBuff[bIndex_BlockSize + wIndex_Buff] ^= pIv[bIndex_BlockSize];
                }
                break;

            case PH_CRYPTOSYM_CIPHER_MODE_ECB:
                /* Nothing to do here */
                break;

            default:
                /* Add additional Modes of operation in here! */
                return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
        }

#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
        (void) memcpy(pHelperBuffer, &pEncBuff[wIndex_Buff], wBlockSize);

        if((uint8_t) wOption == PH_CRYPTOSYM_CIPHER_MODE_CBC_DF4)
        {
            PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_DecryptBlock(pDataParams, pHelperBuffer));
        }
        else
        {
            PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_EncryptBlock(pDataParams, pHelperBuffer));
        }

        (void) memcpy(&pEncBuff[wIndex_Buff], pHelperBuffer, wBlockSize);
#else
        if((uint8_t) wOption == PH_CRYPTOSYM_CIPHER_MODE_CBC_DF4)
        {
            PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_DecryptBlock(pDataParams, &pEncBuff[wIndex_Buff]));
        }
        else
        {
            PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_EncryptBlock(pDataParams, &pEncBuff[wIndex_Buff]));
        }

#endif /* PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE */

        /* do the loop dependent post processing of the data according to the used mode of operation */
        switch((uint8_t) (wOption))
        {
            case PH_CRYPTOSYM_CIPHER_MODE_CBC:
            case PH_CRYPTOSYM_CIPHER_MODE_CBC_DF4:
                /* we should set the IV now to the old ciphertext... */
                pIv = &pEncBuff[wIndex_Buff];
                break;

            case PH_CRYPTOSYM_CIPHER_MODE_ECB:
                /* Nothing to do here */
                break;

            default:
                /* Add additional Modes of operation in here! */
                return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
        }

        /* update the loop counter */
        wIndex_Buff = wBlockSize + wIndex_Buff;
    } /* end of loop over all data blocks */

    /* do the final update of the IV according to the keep IV setting. */
    if((pDataParams->wKeepIV == PH_CRYPTOSYM_VALUE_KEEP_IV_ON) || (0U != (wOption & PH_EXCHANGE_BUFFERED_BIT)))
    {
        (void) memcpy(pDataParams->pIV, pIv, wBlockSize);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_Sw_Decrypt(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, const uint8_t * pEncBuff, uint16_t wBuffLen,
    uint8_t * pPlainBuff)
{
    phStatus_t wStatus = 0;
    uint16_t wBlockSize = 0;
    uint16_t wIndex_Buff = 0;
    uint8_t bIndex_BlockSize = 0;
    uint8_t pIv[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE]; /* AES block size is the maximum available block size */
    uint8_t pNextIv[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE]; /* AES block size is the maximum available block size */


#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
    uint8_t PH_CRYTOSYM_SW_FAST_RAM pHelperBuffer[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE];
#endif /* PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE */

    /* Clear buffer for QAC */
    (void) memset(pNextIv, 0, PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE);


    /* Get the block size of the currently loaded key */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_GetConfig(pDataParams, PH_CRYPTOSYM_CONFIG_BLOCK_SIZE, &wBlockSize));

    /* Get IV */
    (void) memcpy(pIv, pDataParams->pIV, wBlockSize);

    /* Check that the input buffer length is a multiple of the block size; */
    if (0U != (wBuffLen % wBlockSize))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    /*Iterate over all blocks and perform the decryption*/
    wIndex_Buff = 0;
    while(wIndex_Buff < wBuffLen)
    {
        /* Is the output array the same as the input array? Else we need to recopy the encrypted data into the plaintext data buffer first. */
        if(pEncBuff != pPlainBuff)
        {
            (void) memcpy(&pPlainBuff[wIndex_Buff], &pEncBuff[wIndex_Buff], wBlockSize);
        }

        /* perform pre processing of the data according to the selected mode of operation. */
        switch((uint8_t) (wOption))
        {
            case PH_CRYPTOSYM_CIPHER_MODE_CBC:
            case PH_CRYPTOSYM_CIPHER_MODE_CBC_DF4:
                /* In case of CBC mode, we need to remember the previous encrytped block */
                (void) memcpy(pNextIv, &pEncBuff[wIndex_Buff], wBlockSize);
                break;

            case PH_CRYPTOSYM_CIPHER_MODE_ECB:
                /* Nothing to do! */
                break;

            default:
                /* Add additional Modes of operation in here! */
                return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
        }

#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
        (void) memcpy(pHelperBuffer, &pPlainBuff[wIndex_Buff], wBlockSize);
        PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_DecryptBlock(pDataParams, pHelperBuffer));
        (void) memcpy(&pPlainBuff[wIndex_Buff], pHelperBuffer, wBlockSize);
#else
        PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_DecryptBlock(pDataParams, &pPlainBuff[wIndex_Buff]));

#endif /* PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE */

        /* perform post processing of the data according to the selected mode of operation. */
        switch((uint8_t) (wOption))
        {
            case PH_CRYPTOSYM_CIPHER_MODE_CBC:
            case PH_CRYPTOSYM_CIPHER_MODE_CBC_DF4:
                /* In case of CBC mode, we need to perform the XOR with the previous cipher block */
                for(bIndex_BlockSize = 0; bIndex_BlockSize < wBlockSize; ++bIndex_BlockSize)
                {
                    pPlainBuff[bIndex_BlockSize + wIndex_Buff] ^= pIv[bIndex_BlockSize];
                }

                /* Get IV */
                (void) memcpy(pIv, pNextIv, wBlockSize);
                break;

            case PH_CRYPTOSYM_CIPHER_MODE_ECB:
                /* Nothing to do here */
                break;

            default:
                /* Add additional Modes of operation in here! */
                return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
        }

        /* update the loop counter */
        wIndex_Buff = wBlockSize + wIndex_Buff;
    }

    if((pDataParams->wKeepIV == PH_CRYPTOSYM_VALUE_KEEP_IV_ON) || (0U != (wOption & PH_EXCHANGE_BUFFERED_BIT)))
    {
        /* Update IV */
        (void) memcpy(pDataParams->pIV, pIv, wBlockSize);
    }

    return PH_ERR_SUCCESS;
}


phStatus_t phCryptoSym_Sw_CalculateMac(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, const uint8_t * pData, uint16_t wDataLen,
    uint8_t * pMac, uint8_t * pMacLen)
{
    phStatus_t wStatus = 0;
    uint16_t wBlockSize = 0;
    uint16_t wIndex_Buff = 0;
    uint8_t bIndex_BlockSize = 0;
    uint8_t bPaddingLen = 0;
    uint8_t bLastBlock[16];
    uint8_t * pIv = NULL;

#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
    uint8_t PH_CRYTOSYM_SW_FAST_RAM pHelperBuffer[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE];
#endif /* PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE */

#ifndef PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION
    uint8_t * pSubKey1 = pDataParams->pCMACSubKey1;
    uint8_t * pSubKey2 = pDataParams->pCMACSubKey2;
#else
    uint8_t pSubKey1[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE];
    uint8_t pSubKey2[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE];

    (void) memset(pSubKey1, 0x00, (size_t) sizeof(pSubKey1));
    (void) memset(pSubKey2, 0x00, (size_t) sizeof(pSubKey2));
#endif /* PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION */

    /* Clear MAC length */
    *pMacLen = 0;

    /* Clear the last block array */
    (void) memset(bLastBlock, 0, (size_t) sizeof(bLastBlock));


    /* Get the block size of the currently loaded key */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_GetConfig(pDataParams, PH_CRYPTOSYM_CONFIG_BLOCK_SIZE, &wBlockSize));

    /* In case of a first block and in case of KEEP_IV is not set, the IV has to be cleared. */
    if((0U != (wOption & PH_EXCHANGE_LEAVE_BUFFER_BIT)) ||
        (pDataParams->wKeepIV == PH_CRYPTOSYM_VALUE_KEEP_IV_ON))
    {
        /* better leave the IV */
    }
    else
    {
        (void) memset(pDataParams->pIV, 0x00, wBlockSize);
    }

    /* Now we may start with  MAC calculation */

    /*Let's find out whether we should complete the MAC or if this is just an intermediate MAC calculation */
    if (0U != (wOption & PH_EXCHANGE_BUFFERED_BIT))
    {
        /* This is just an intermediate MAC */

        /* In this case we do not allow incomplete blocks. */
        if (0U != (wDataLen % wBlockSize))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
        }
    }
    else
    {
        switch((uint8_t) (wOption))
        {
            case PH_CRYPTOSYM_MAC_MODE_CMAC:
#ifndef PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION
                /* Do we need to compute the sub keys? */
                if(pDataParams->bCMACSubKeysInitialized == 0x00U)
                {
                    /* start with sub key generation */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_CMAC_GenerateK1K2(pDataParams, pSubKey1, pSubKey2));

                }
#else
                /* Always perform with sub key generation */
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_CMAC_GenerateK1K2(pDataParams, pSubKey1, pSubKey2));
#endif /* PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION */
                break;

            case PH_CRYPTOSYM_MAC_MODE_CBCMAC:
                /* Nothing to do! */
                break;

            default:
                /* Add additional Modes of operation in here! */
                return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
        }

        /* Get number of bytes in last block */
        bPaddingLen = (uint8_t) (wDataLen % wBlockSize);

        /* do we have incomplete blocks? */
        if((0U != bPaddingLen) || (wDataLen == 0x0000U))
        {
            /* Update wDataLen, last block is in other array */
            wDataLen = (uint16_t) (wDataLen - bPaddingLen);

            (void) memcpy(bLastBlock, &pData[wDataLen], bPaddingLen);

            /* Apply padding byte*/
            bLastBlock[bPaddingLen] = 0x80;
            /* pad with zeros not necessary, memset done upfront*/

            if((uint8_t) wOption == PH_CRYPTOSYM_MAC_MODE_CMAC)
            {
                /* XOR with K2, as we have an icomplete block */
                for(bIndex_BlockSize = 0; bIndex_BlockSize < wBlockSize; bIndex_BlockSize++)
                {
                    bLastBlock[bIndex_BlockSize] ^= pSubKey2[bIndex_BlockSize];
                }
            }
        }
        else
        {
            /* Update wDataLen, last block is in other array */
            wDataLen = wDataLen - wBlockSize;

            /* Copy whole block into bLastBlock */
            (void) memcpy(bLastBlock, &pData[wDataLen], wBlockSize);

            if((uint8_t) wOption == PH_CRYPTOSYM_MAC_MODE_CMAC)
            {
                /* XOR with K1, as we have a complete block */
                for(bIndex_BlockSize = 0; bIndex_BlockSize < wBlockSize; bIndex_BlockSize++)
                {
                    bLastBlock[bIndex_BlockSize] ^= pSubKey1[bIndex_BlockSize];
                }
            }
        }
    }

    /* Set the IV to the iv specified in the private data params */
    pIv = pDataParams->pIV;

    /*Iterate over all blocks and perform the CBC encryption*/
    wIndex_Buff = 0;
    while(wIndex_Buff < wDataLen)
    {
        /* perform the XOR with the previous cipher block */
        for(bIndex_BlockSize = 0; bIndex_BlockSize < wBlockSize; bIndex_BlockSize++)
        {
            /* Note: after one round pIv == pMac */
#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
            pHelperBuffer[bIndex_BlockSize] = pIv[bIndex_BlockSize] ^ pData[wIndex_Buff + bIndex_BlockSize];
#else
            pMac[bIndex_BlockSize] = pIv[bIndex_BlockSize] ^ pData[wIndex_Buff + bIndex_BlockSize];
#endif
        }

#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
        PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_EncryptBlock(pDataParams, pHelperBuffer));
        (void) memcpy(pMac, pHelperBuffer, wBlockSize);
#else
        PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_EncryptBlock(pDataParams, pMac));
#endif

        /* set pIv to last cipher block*/
        pIv = pMac;

        /* update the loop counter */
        wIndex_Buff = wBlockSize + wIndex_Buff;
    } /* end of loop over all data blocks */

    /* If we have a complete MAC, lets encrypt the last block */
    if(0U == (wOption & PH_EXCHANGE_BUFFERED_BIT))
    {
        /* Encrypt last block. */
        /* perform the XOR with the previous cipher block */
        for(bIndex_BlockSize = 0; bIndex_BlockSize < wBlockSize; bIndex_BlockSize++)
        {
            /* Note: after one round pIv == pMac */
#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
            pHelperBuffer[bIndex_BlockSize] = pIv[bIndex_BlockSize] ^ bLastBlock[bIndex_BlockSize];
#else
            pMac[bIndex_BlockSize] = pIv[bIndex_BlockSize] ^ bLastBlock[bIndex_BlockSize];
#endif
        }
#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
        PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_EncryptBlock(pDataParams, pHelperBuffer));
        (void) memcpy(pMac, pHelperBuffer, wBlockSize);
#else
        PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_EncryptBlock(pDataParams, pMac));
#endif

    }

    /* do the final update of the IV according to the settings */
    if((pDataParams->wKeepIV == PH_CRYPTOSYM_VALUE_KEEP_IV_ON) || (0U != (wOption & PH_EXCHANGE_BUFFERED_BIT)))
    {
        (void) memcpy(pDataParams->pIV, pMac, wBlockSize);
    }
    else
    {
        /* Clear the IV for security reasons */
        (void) memset(pDataParams->pIV, 0, wBlockSize);
    }

#ifdef PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION
    /* Clear key arrays */
    (void) memset(pSubKey1, 0x00, (size_t) sizeof(pSubKey1));
    (void) memset(pSubKey2, 0x00, (size_t) sizeof(pSubKey2));
#endif

    *pMacLen = (uint8_t) wBlockSize;
    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_Sw_LoadIv(phCryptoSym_Sw_DataParams_t * pDataParams, const uint8_t * pIV, uint8_t bIVLen)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint16_t    PH_MEMLOC_REM wBlockSize = 0;

    /* Get the block size of the currently loaded key */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_GetConfig(pDataParams, PH_CRYPTOSYM_CONFIG_BLOCK_SIZE, &wBlockSize));

    /* Check block-size */
    if(bIVLen != wBlockSize)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    /* Update IV */
    (void) memcpy(pDataParams->pIV, pIV, wBlockSize);

    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_Sw_LoadKey(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVer, uint16_t wKeyType)
{
#ifdef NXPBUILD__PH_KEYSTORE
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_BUF aKey[PH_CRYPTOSYM_AES256_KEY_SIZE];
    uint16_t    PH_MEMLOC_REM wStored_KeyType = 0;

    /* Not possible without keystore */
    if(pDataParams->pKeyStoreDataParams == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_CRYPTOSYM);
    }

    /* Retrieve key settings */
    PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVer,
        (uint8_t) sizeof(aKey),
        aKey,
        &wStored_KeyType));

    /* KeyType should match */
    if(wKeyType != wStored_KeyType)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    /* Finally load the key */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_LoadKeyDirect(pDataParams, aKey, wKeyType));

    /* For security reasons */
    (void) memset(aKey, 0x00, (size_t) sizeof(aKey));

    return PH_ERR_SUCCESS;
#else
    /* satisfy compiler */
    if(pDataParams || wKeyNo || wKeyVer || wKeyType)
    {
        ; /* Do Nothing */
    }
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_CRYPTOSYM);
#endif /* NXPBUILD__PH_KEYSTORE */
}

phStatus_t phCryptoSym_Sw_LoadKeyDirect(phCryptoSym_Sw_DataParams_t * pDataParams, const uint8_t * pKey, uint16_t wKeyType)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;

    /* Clear existing key */
    (void) memset(pDataParams->pKey, 0x00, (size_t) sizeof(pDataParams->pKey));


    {
#ifndef PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION
        /* Disable the CMAC calculated Flag */
        pDataParams->bCMACSubKeysInitialized = PH_OFF;
#endif /* PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION */

        switch(wKeyType)
        {
#ifdef PH_CRYPTOSYM_SW_AES
            case PH_CRYPTOSYM_KEY_TYPE_AES128:
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Aes_KeyExpansion(
                    pDataParams,
                    pKey,
                    PH_CRYPTOSYM_AES128_KEY_SIZE >> 2U,
                    (PH_CRYPTOSYM_SW_NUM_AES_ROUNDS_128 + 1U) << 2U));
                break;

            case PH_CRYPTOSYM_KEY_TYPE_AES192:
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Aes_KeyExpansion(
                    pDataParams,
                    pKey,
                    PH_CRYPTOSYM_AES192_KEY_SIZE >> 2U,
                    (PH_CRYPTOSYM_SW_NUM_AES_ROUNDS_192 + 1U) << 2U));
                break;

            case PH_CRYPTOSYM_KEY_TYPE_AES256:
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Aes_KeyExpansion(
                    pDataParams,
                    pKey,
                    PH_CRYPTOSYM_AES256_KEY_SIZE >> 2U,
                    (PH_CRYPTOSYM_SW_NUM_AES_ROUNDS_256 + 1U) << 2U));
                break;
#endif /* PH_CRYPTOSYM_SW_AES */

#ifdef PH_CRYPTOSYM_SW_DES
            case PH_CRYPTOSYM_KEY_TYPE_DES:
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Des_KeyInit(pDataParams, pKey, 1));
                break;

            case PH_CRYPTOSYM_KEY_TYPE_2K3DES:
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Des_KeyInit(pDataParams, pKey, 2));
                break;

            case PH_CRYPTOSYM_KEY_TYPE_3K3DES:
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Des_KeyInit(pDataParams, pKey, 3));
                break;
#endif /* PH_CRYPTOSYM_SW_DES */

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
        }
    }
    /* Update global KeyType */
    pDataParams->wKeyType = wKeyType;

    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_Sw_DiversifyKey(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, uint16_t wKeyNo, uint16_t wKeyVer,
    uint8_t * pDivInput, uint8_t bDivInputLen, uint8_t * pDiversifiedKey, uint8_t * pDivKeyLen)
{
#ifdef NXPBUILD__PH_KEYSTORE
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aKey[PH_CRYPTOSYM_AES256_KEY_SIZE];
    uint16_t    PH_MEMLOC_REM wKeyType = 0;

    /* Not possible without keystore */
    if(pDataParams->pKeyStoreDataParams == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_CRYPTOSYM);
    }

    /* Parameter check */
    if(((wOption & PH_CRYPTOSYM_DIV_MODE_MASK) != PH_CRYPTOSYM_DIV_MODE_DESFIRE) &&
        ((wOption & PH_CRYPTOSYM_DIV_MODE_MASK) != PH_CRYPTOSYM_DIV_MODE_MIFARE_PLUS) &&
        ((wOption & PH_CRYPTOSYM_DIV_MODE_MASK) != PH_CRYPTOSYM_DIV_MODE_MIFARE_ULTRALIGHT))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    /* Retrieve key from keystore */
    PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVer,
        sizeof(aKey),
        aKey,
        &wKeyType));

    /* Perform diversification */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_DiversifyDirectKey(
        pDataParams,
        wOption,
        aKey,
        wKeyType,
        pDivInput,
        bDivInputLen,
        pDiversifiedKey,
        pDivKeyLen));

    /* For security reasons */
    (void) memset(aKey, 0x00, sizeof(aKey));

    return PH_ERR_SUCCESS;
#else
    /* satisfy compiler */
    if(pDataParams || wKeyNo || wKeyVer || pDiversifiedKey || bDivInputLen || pDivInput || wOption)
    {
        ; /* Do Nothing */
    }
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_CRYPTOSYM);
#endif  /* NXPBUILD__PH_KEYSTORE */
}

phStatus_t phCryptoSym_Sw_DiversifyDirectKey(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wOption, uint8_t * pKey, uint16_t wKeyType,
    uint8_t * pDivInput, uint8_t bDivInputLen, uint8_t * pDiversifiedKey, uint8_t * pDivKeyLen)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_BUF abDivData[PH_CRYPTOSYM_SW_KDIV_MFP_DIVLENGTH_AES_MAX + 1];
    uint8_t     PH_MEMLOC_BUF abMac[16];
    uint8_t     PH_MEMLOC_REM bTmpLen = 0;
    uint16_t    PH_MEMLOC_REM wKeyVer = 0;

    /* Load the Key to diversify */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_LoadKeyDirect(pDataParams, pKey, wKeyType));

    /* Check diversification method */
    switch(wOption & PH_CRYPTOSYM_DIV_MODE_MASK)
    {
        case PH_CRYPTOSYM_DIV_MODE_DESFIRE:
            switch(wKeyType)
            {
#ifdef PH_CRYPTOSYM_SW_AES
                case PH_CRYPTOSYM_KEY_TYPE_AES128:
                case PH_CRYPTOSYM_KEY_TYPE_AES192:
                    /* Parameter check */
                    if(bDivInputLen != PH_CRYPTOSYM_AES128_KEY_SIZE)
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
                    }

                    /* Use the DivData as IV */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_LoadIv(pDataParams, pDivInput, bDivInputLen));

                    /* Retrieve KeySize */
                    bDivInputLen = (uint8_t) phCryptoSym_GetKeySize(wKeyType);

                    /* Copy Key to temporary diversification data */
                    (void) memcpy(abDivData, pKey, bDivInputLen);

                    /* Apply padding if neccessary */
                    if((bDivInputLen % PH_CRYPTOSYM_AES_BLOCK_SIZE) != 0U)
                    {
                        (void) memset(&abDivData[PH_CRYPTOSYM_AES192_KEY_SIZE], 0x00, 8);
                        bTmpLen = PH_CRYPTOSYM_AES192_KEY_SIZE + 8U;
                    }
                    else
                    {
                        bTmpLen = bDivInputLen;
                    }

                    /* Perform Encryption */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Encrypt(
                        pDataParams,
                        PH_CRYPTOSYM_CIPHER_MODE_CBC | PH_EXCHANGE_BUFFER_CONT,
                        abDivData,
                        (uint16_t) bTmpLen,
                        abDivData));

                    /* Copy diversified Key to output buffer */
                    (void) memcpy(pDiversifiedKey, abDivData, bDivInputLen);
                    *pDivKeyLen = bDivInputLen;

                    pDataParams->wAddInfo = bDivInputLen;
                    break;
#endif /* PH_CRYPTOSYM_SW_AES */

#ifdef PH_CRYPTOSYM_SW_DES
                case PH_CRYPTOSYM_KEY_TYPE_DES:
                case PH_CRYPTOSYM_KEY_TYPE_2K3DES:
                    /* Parameter check */
                    if(bDivInputLen != PH_CRYPTOSYM_DES_KEY_SIZE)
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
                    }

                    /* Use the DivData as IV */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_LoadIv(pDataParams, pDivInput, bDivInputLen));

                    /* Retrieve KeyVersion */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Des_DecodeVersion(pKey, &wKeyVer));

                    /* Half-Key Diversification (DES 56 Bit Key Type ) */
                    if(0U != (wOption & PH_CRYPTOSYM_DIV_OPTION_2K3DES_HALF))
                    {
                        /* Perform Encryption */
                        PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Encrypt(
                            pDataParams,
                            PH_CRYPTOSYM_CIPHER_MODE_CBC | PH_EXCHANGE_BUFFER_CONT,
                            pKey,
                            PH_CRYPTOSYM_DES_KEY_SIZE,
                            pDiversifiedKey));

                        /* half key diversification -> copy first part to second part */
                        (void) memcpy(&pDiversifiedKey[PH_CRYPTOSYM_DES_KEY_SIZE], &pDiversifiedKey[0], PH_CRYPTOSYM_DES_KEY_SIZE);
                        *pDivKeyLen = (uint8_t) PH_CRYPTOSYM_DES_KEY_SIZE;

                        pDataParams->wAddInfo = PH_CRYPTOSYM_DES_KEY_SIZE;
                    }

                    /* Full-Key Diversification (DES2K3 - 112 Bit Key Type) */
                    else
                    {
                        /* Perform Encryption */
                        PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Encrypt(
                            pDataParams,
                            PH_CRYPTOSYM_CIPHER_MODE_CBC | PH_EXCHANGE_BUFFER_CONT,
                            pKey,
                            PH_CRYPTOSYM_2K3DES_KEY_SIZE,
                            pDiversifiedKey));
                        *pDivKeyLen = (uint8_t) PH_CRYPTOSYM_2K3DES_KEY_SIZE;

                        pDataParams->wAddInfo = PH_CRYPTOSYM_2K3DES_KEY_SIZE;
                    }

                    /* Re-Encode KeyVersion */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Des_EncodeVersion(pDiversifiedKey, wKeyVer, wKeyType, pDiversifiedKey));
                    break;

                case PH_CRYPTOSYM_KEY_TYPE_3K3DES:
                    /* Parameter check */
                    if(bDivInputLen != PH_CRYPTOSYM_DES_KEY_SIZE)
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
                    }

                    /* Use the DivData as IV */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_LoadIv(pDataParams, pDivInput, bDivInputLen));

                    /* Retrieve KeyVersion */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Des_DecodeVersion(pKey, &wKeyVer));

                    /* Perform Encryption */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Encrypt(
                        pDataParams,
                        PH_CRYPTOSYM_CIPHER_MODE_CBC | PH_EXCHANGE_BUFFER_CONT,
                        pKey,
                        PH_CRYPTOSYM_3K3DES_KEY_SIZE,
                        pDiversifiedKey));

                    /* Re-Encode KeyVersion */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Des_EncodeVersion(pDiversifiedKey, wKeyVer, wKeyType, pDiversifiedKey));
                    *pDivKeyLen = (uint8_t) PH_CRYPTOSYM_3K3DES_KEY_SIZE;

                    pDataParams->wAddInfo = PH_CRYPTOSYM_3K3DES_KEY_SIZE;
                    break;
#endif /* PH_CRYPTOSYM_SW_DES */

                default:
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
            }
            break;

        case PH_CRYPTOSYM_DIV_MODE_MIFARE_PLUS:
            /* Parameter check */
            if(bDivInputLen > PH_CRYPTOSYM_SW_KDIV_MFP_DIVLENGTH_AES_MAX)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
            }

            /* Copy div. input */
            (void) memcpy(&abDivData[1], pDivInput, bDivInputLen);

            /* Increment div. input length */
            ++bDivInputLen;

            switch(wKeyType)
            {
#ifdef PH_CRYPTOSYM_SW_AES
                case PH_CRYPTOSYM_KEY_TYPE_AES128:
                    /* Set div. header */
                    abDivData[0] = PH_CRYPTOSYM_SW_KDIV_MFP_AES128_CONST;

                    /* Perform MAC calculation */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Diversify_CMAC(
                        pDataParams,
                        abDivData,
                        (uint16_t) bDivInputLen,
                        pDiversifiedKey,
                        &bTmpLen));
                    *pDivKeyLen = bTmpLen;

                    pDataParams->wAddInfo = bTmpLen;
                    break;

                case PH_CRYPTOSYM_KEY_TYPE_AES192:
                    /* Set div. header for DiversifiedKeyA */
                    abDivData[0] = PH_CRYPTOSYM_SW_KDIV_MFP_AES192_CONST_1;

                    /* Perform MAC calculation (KeyA) */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Diversify_CMAC(
                        pDataParams,
                        abDivData,
                        (uint16_t) bDivInputLen,
                        pDiversifiedKey,
                        &bTmpLen));

                    /* Set div. header for DiversifiedKeyB */
                    abDivData[0] = PH_CRYPTOSYM_SW_KDIV_MFP_AES192_CONST_2;

                    /* Perform MAC calculation (KeyB) */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Diversify_CMAC(
                        pDataParams,
                        abDivData,
                        (uint16_t) bDivInputLen,
                        abMac,
                        &bTmpLen));

                    /* perform DiversifiedKeyA[8-15] ^ DiversifiedKeyB[0-7] */
                    for(bTmpLen = 0; bTmpLen < 8U; ++bTmpLen)
                    {
                        pDiversifiedKey[8U + bTmpLen] ^= abMac[bTmpLen];
                    }

                    /* copy DiversifiedKeyB[8-15] to DiversifiedKey[16-23] */
                    (void) memcpy(&pDiversifiedKey[16], &abMac[8], 8);
                    *pDivKeyLen = 24;

                    pDataParams->wAddInfo = 24;
                    break;
#endif /* PH_CRYPTOSYM_SW_AES */

#ifdef PH_CRYPTOSYM_SW_DES
                case PH_CRYPTOSYM_KEY_TYPE_DES:
                case PH_CRYPTOSYM_KEY_TYPE_2K3DES:
                    /* Set div. header for DiversifiedKeyA */
                    abDivData[0] = PH_CRYPTOSYM_SW_KDIV_MFP_3DES_CONST_1;

                    /* Perform MAC calculation (KeyA) */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Diversify_CMAC(
                        pDataParams,
                        abDivData,
                        (uint16_t) bDivInputLen,
                        &pDiversifiedKey[0],
                        &bTmpLen));

                    /* Set div. header for DiversifiedKeyB */
                    abDivData[0] = PH_CRYPTOSYM_SW_KDIV_MFP_3DES_CONST_2;

                    /* Perform MAC calculation (KeyB) */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Diversify_CMAC(
                        pDataParams,
                        abDivData,
                        (uint16_t) bDivInputLen,
                        &pDiversifiedKey[8],
                        &bTmpLen));
                    *pDivKeyLen = 16;

                    pDataParams->wAddInfo = 16;
                    break;

                case PH_CRYPTOSYM_KEY_TYPE_3K3DES:
                    /* Set div. header for DiversifiedKeyA */
                    abDivData[0] = PH_CRYPTOSYM_SW_KDIV_MFP_3KEY3DES_CONST_1;

                    /* Perform MAC calculation (KeyA) */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Diversify_CMAC(
                        pDataParams,
                        abDivData,
                        (uint16_t) bDivInputLen,
                        &pDiversifiedKey[0],
                        &bTmpLen));

                    /* Set div. header for DiversifiedKeyB */
                    abDivData[0] = PH_CRYPTOSYM_SW_KDIV_MFP_3KEY3DES_CONST_2;

                    /* Perform MAC calculation (KeyB) */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Diversify_CMAC(
                        pDataParams,
                        abDivData,
                        (uint16_t) bDivInputLen,
                        &pDiversifiedKey[8],
                        &bTmpLen));

                    /* Set div. header for DiversifiedKeyC */
                    abDivData[0] = PH_CRYPTOSYM_SW_KDIV_MFP_3KEY3DES_CONST_3;

                    /* Perform MAC calculation (KeyC) */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Diversify_CMAC(
                        pDataParams,
                        abDivData,
                        (uint16_t) bDivInputLen,
                        &pDiversifiedKey[16],
                        &bTmpLen));
                    *pDivKeyLen = 24;

                    pDataParams->wAddInfo = 24;
                    break;
#endif /* PH_CRYPTOSYM_SW_DES */

                default:
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
            }
            break;


        case PH_CRYPTOSYM_DIV_MODE_MIFARE_ULTRALIGHT:
            /* Parameter check */
            if(bDivInputLen > PH_CRYPTOSYM_SW_KDIV_MFP_DIVLENGTH_AES_MAX)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
            }

            /* Copy div. input */
            (void) memcpy(&abDivData[1], pDivInput, bDivInputLen);

            /* Increment div. input length */
            ++bDivInputLen;

            switch(wKeyType)
            {
#ifdef PH_CRYPTOSYM_SW_AES
                case PH_CRYPTOSYM_KEY_TYPE_AES128:
                    /* Set div. header */
                    abDivData[0] = PH_CRYPTOSYM_SW_KDIV_MFUL_AES128_CONST;

                    /* Perform MAC calculation */
                    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Sw_Diversify_CMAC(
                        pDataParams,
                        abDivData,
                        (uint16_t) bDivInputLen,
                        pDiversifiedKey,
                        &bTmpLen));
                    *pDivKeyLen = bTmpLen;

                    pDataParams->wAddInfo = bTmpLen;
                    break;
#endif /* PH_CRYPTOSYM_SW_AES */

                default:
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
            }
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_Sw_SetConfig(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t wValue)
{
    switch(wConfig)
    {
        case PH_CRYPTOSYM_CONFIG_KEEP_IV:
            /* parameter check */
            if((wValue != PH_CRYPTOSYM_VALUE_KEEP_IV_OFF) &&
                (wValue != PH_CRYPTOSYM_VALUE_KEEP_IV_ON))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
            }
            pDataParams->wKeepIV = wValue;
            break;


        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_Sw_GetConfig(phCryptoSym_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t * pValue)
{
    switch(wConfig)
    {
        case PH_CRYPTOSYM_CONFIG_KEY_SIZE:
            switch(pDataParams->wKeyType)
            {
#ifdef PH_CRYPTOSYM_SW_AES
                case PH_CRYPTOSYM_KEY_TYPE_AES128:
                    *pValue = PH_CRYPTOSYM_AES128_KEY_SIZE;
                    break;

                case PH_CRYPTOSYM_KEY_TYPE_AES192:
                    *pValue = PH_CRYPTOSYM_AES192_KEY_SIZE;
                    break;

                case PH_CRYPTOSYM_KEY_TYPE_AES256:
                    *pValue = PH_CRYPTOSYM_AES256_KEY_SIZE;
                    break;
#endif /* PH_CRYPTOSYM_SW_AES */

#ifdef PH_CRYPTOSYM_SW_DES
                case PH_CRYPTOSYM_KEY_TYPE_DES:
                    *pValue = PH_CRYPTOSYM_DES_KEY_SIZE;
                    break;

                case PH_CRYPTOSYM_KEY_TYPE_2K3DES:
                    *pValue = PH_CRYPTOSYM_2K3DES_KEY_SIZE;
                    break;

                case PH_CRYPTOSYM_KEY_TYPE_3K3DES:
                    *pValue = PH_CRYPTOSYM_3K3DES_KEY_SIZE;
                    break;
#endif /* PH_CRYPTOSYM_SW_DES */

                default:
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
            }
            break;

        case PH_CRYPTOSYM_CONFIG_BLOCK_SIZE:
            switch(pDataParams->wKeyType)
            {
#ifdef PH_CRYPTOSYM_SW_AES
                case PH_CRYPTOSYM_KEY_TYPE_AES128:
                case PH_CRYPTOSYM_KEY_TYPE_AES192:
                case PH_CRYPTOSYM_KEY_TYPE_AES256:
                    *pValue = PH_CRYPTOSYM_AES_BLOCK_SIZE;
                    break;
#endif /* PH_CRYPTOSYM_SW_AES */

#ifdef PH_CRYPTOSYM_SW_DES
                case PH_CRYPTOSYM_KEY_TYPE_DES:
                case PH_CRYPTOSYM_KEY_TYPE_2K3DES:
                case PH_CRYPTOSYM_KEY_TYPE_3K3DES:
                    *pValue = PH_CRYPTOSYM_DES_BLOCK_SIZE;
                    break;
#endif /* PH_CRYPTOSYM_SW_DES */

                default:
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
            }
            break;

        case PH_CRYPTOSYM_CONFIG_KEY_TYPE:
            *pValue = pDataParams->wKeyType;
            break;

        case PH_CRYPTOSYM_CONFIG_KEEP_IV:
            *pValue = pDataParams->wKeepIV;
            break;


        case PH_CRYPTOSYM_CONFIG_ADDITIONAL_INFO:
            *pValue = pDataParams->wAddInfo;
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    return PH_ERR_SUCCESS;
}

#endif /* NXPBUILD__PH_CRYPTOSYM_SW */
