/*----------------------------------------------------------------------------*/
/* Copyright 2009-2020,2022,2023 NXP                                          */
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
* Software MIFARE(R) Ultralight contactless IC Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#include <ph_Status.h>
#include <phalMful.h>
#include <phpalMifare.h>
#include <phKeyStore.h>
#ifdef NXPBUILD__PH_CRYPTOSYM
#include <phCryptoSym.h>
#endif /* NXPBUILD__PH_CRYPTOSYM */
#ifdef NXPBUILD__PH_CRYPTORNG
#include <phCryptoRng.h>
#endif /* NXPBUILD__PH_CRYPTORNG */
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PHAL_MFUL_SW

#include "phalMful_Sw.h"
#include "../phalMful_Int.h"

/*
* Private constants
*/
#ifdef NXPBUILD__PH_CRYPTOSYM
static const uint8_t PH_MEMLOC_CONST_ROM phalMful_Sw_FirstIv[PHAL_MFUL_AES_BLOCK_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif /* NXPBUILD__PH_CRYPTOSYM */
phStatus_t phalMful_Sw_Init(
                            phalMful_Sw_DataParams_t * pDataParams,
                            uint16_t wSizeOfDataParams,
                            void * pPalMifareDataParams,
                            void * pKeyStoreDataParams,
                            void * pCryptoDataParams,
                            void * pCryptoRngDataParams
                            )
{
    if (sizeof(phalMful_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFUL);
    }
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pPalMifareDataParams);

    /* init private data */
    pDataParams->wId                    = PH_COMP_AL_MFUL | PHAL_MFUL_SW_ID;
    pDataParams->pPalMifareDataParams   = pPalMifareDataParams;
    pDataParams->pKeyStoreDataParams    = pKeyStoreDataParams;
    pDataParams->pCryptoDataParams      = pCryptoDataParams;
    pDataParams->pCryptoRngDataParams   = pCryptoRngDataParams;
    pDataParams->bAuthMode              = PHAL_MFUL_NOT_AUTHENTICATED;
    pDataParams->bCMACReq               = PH_OFF;
    pDataParams->wCmdCtr                = 0x00;
    pDataParams->bAdditionalInfo        = 0x00;
    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Sw_UlcAuthenticate(phalMful_Sw_DataParams_t * pDataParams, uint16_t wKeyNumber, uint16_t wKeyVersion)
{
#ifdef NXPBUILD__PH_CRYPTOSYM
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bFrame[PHAL_MFUL_DES_BLOCK_SIZE+1];
    uint8_t     PH_MEMLOC_REM bRndA[PHAL_MFUL_DES_BLOCK_SIZE];
    uint8_t     PH_MEMLOC_REM bRndB[PHAL_MFUL_DES_BLOCK_SIZE+1];
    uint8_t     PH_MEMLOC_REM bKey[PHAL_MFUL_DES_KEY_LENGTH];
    uint16_t    PH_MEMLOC_REM wKeyType;
    uint8_t *   PH_MEMLOC_REM pRxBuffer;
    uint16_t    PH_MEMLOC_REM wRxLength;

    /* Parameter check */
    if ((pDataParams->pKeyStoreDataParams == NULL) ||
        (pDataParams->pCryptoDataParams == NULL) ||
        (pDataParams->pCryptoRngDataParams) == NULL)
    {
        return PH_ADD_COMPCODE(PH_ERR_USE_CONDITION, PH_COMP_AL_MFUL);
    }

    /* retrieve key from keystore */
    statusTmp = phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNumber,
        wKeyVersion,
        PHAL_MFUL_DES_KEY_LENGTH,
        bKey,
        &wKeyType);

    /* check return code and map to key error */
    if ((statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        return PH_ADD_COMPCODE(PH_ERR_KEY, PH_COMP_AL_MFUL);
    }

    /* check for correct key type */
    if (wKeyType != PH_KEYSTORE_KEY_TYPE_2K3DES)
    {
        return PH_ADD_COMPCODE(PH_ERR_KEY, PH_COMP_AL_MFUL);
    }

    /* build the authentication request */
    bFrame[0] = PHAL_MFUL_CMD_AUTH;
    bFrame[1] = 0x00;

    /* transmit the request */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        bFrame,
        2,
        &pRxBuffer,
        &wRxLength
        ));

    /* check the format of the received data */
    if ((wRxLength != PHAL_MFUL_DES_BLOCK_SIZE+1) ||
        (pRxBuffer[0] != PHAL_MFUL_PREAMBLE_TX))
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* load the key into the crypto */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadKeyDirect(
        pDataParams->pCryptoDataParams,
        bKey,
        PH_CRYPTOSYM_KEY_TYPE_2K3DES));

    /* Load first IV*/
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        phalMful_Sw_FirstIv,
        PHAL_MFUL_DES_BLOCK_SIZE));

    /* decrypt RndB */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_Decrypt(
        pDataParams->pCryptoDataParams,
        PH_EXCHANGE_BUFFER_FIRST | PH_CRYPTOSYM_CIPHER_MODE_CBC,
        &pRxBuffer[1],
        PHAL_MFUL_DES_BLOCK_SIZE,
        bRndB));

    /* generate RndA  */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoRng_Rnd(
        pDataParams->pCryptoRngDataParams,
        PHAL_MFUL_DES_BLOCK_SIZE,
        bRndA));

    /* buffer the preamble */
    bFrame[0] = PHAL_MFUL_PREAMBLE_TX;
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bFrame,
        1,
        &pRxBuffer,
        &wRxLength));

    /* encrypt RndA */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_Encrypt(
        pDataParams->pCryptoDataParams,
        PH_EXCHANGE_BUFFER_CONT | PH_CRYPTOSYM_CIPHER_MODE_CBC,
        bRndA,
        PHAL_MFUL_DES_BLOCK_SIZE,
        bFrame));

    /* buffer the first part of the cryptogram (Enc(RndA)) */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_CONT,
        bFrame,
        PHAL_MFUL_DES_BLOCK_SIZE,
        &pRxBuffer,
        &wRxLength));

    /* rotate RndB to the left to generate RndB' */
    bRndB[PHAL_MFUL_DES_BLOCK_SIZE] = bRndB[0];

    /* encrypt RndB' */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_Encrypt(
        pDataParams->pCryptoDataParams,
        PH_EXCHANGE_BUFFER_CONT | PH_CRYPTOSYM_CIPHER_MODE_CBC,
        &bRndB[1],
        PHAL_MFUL_DES_BLOCK_SIZE,
        bFrame));

    /* append the second part of the cryptogram (Enc(RndB')) and send the whole stream. */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        bFrame,
        PHAL_MFUL_DES_BLOCK_SIZE,
        &pRxBuffer,
        &wRxLength));

    /* check the format of the received data */
    if ((wRxLength != PHAL_MFUL_DES_BLOCK_SIZE+1) ||
        (pRxBuffer[0] != PHAL_MFUL_PREAMBLE_RX))
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* decrypt RndA' */
    PH_CHECK_SUCCESS_FCT(statusTmp, phCryptoSym_Decrypt(
        pDataParams->pCryptoDataParams,
        PH_EXCHANGE_BUFFER_LAST | PH_CRYPTOSYM_CIPHER_MODE_CBC,
        &pRxBuffer[1],
        PHAL_MFUL_DES_BLOCK_SIZE,
        &bFrame[1]));

    /* rotate RndA' to the right to retrieve RndA */
    bFrame[0] = bFrame[PHAL_MFUL_DES_BLOCK_SIZE];

    /* compare the received RndA with our RndA */
    if (memcmp(bRndA, bFrame, PHAL_MFUL_DES_BLOCK_SIZE) != 0)
    {
        return PH_ADD_COMPCODE(PH_ERR_AUTH_ERROR, PH_COMP_AL_MFUL);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFUL);
#endif /* NXPBUILD__PH_CRYPTOSYM */
#ifndef NXPBUILD__PH_CRYPTOSYM
    /* satisfy compiler */
    if (pDataParams || wKeyNumber || wKeyVersion)
    {
        ;/*Do Nothing*/
    }
    return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AL_MFUL);
#endif /* NXPBUILD__PH_CRYPTOSYM */
}

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalMful_Sw_AuthenticateAES(phalMful_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVer, uint8_t bKeyNoCard)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aRndA[PH_CRYPTOSYM_AES_BLOCK_SIZE];
    uint8_t     PH_MEMLOC_REM aRndB[PH_CRYPTOSYM_AES_BLOCK_SIZE + 1];
    uint8_t     PH_MEMLOC_REM aSesAuthMACKey[PH_CRYPTOSYM_AES_BLOCK_SIZE];
    uint8_t     PH_MEMLOC_REM aKey[PH_CRYPTOSYM_AES128_KEY_SIZE];
    uint16_t    PH_MEMLOC_REM wKeyType = 0;
    uint8_t     PH_MEMLOC_REM aCmdBuff[35];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint8_t     PH_MEMLOC_REM aSV[32];
    uint8_t *   PH_MEMLOC_REM pRecv = NULL;
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM bMacLen = 0;

    /* Validate Card Key Number */
    if((bKeyNoCard > PHAL_MFUL_ORIGINALITY_KEY))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFUL);
    }

    /* Get Key out of the key store object */
    PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVer,
        PH_CRYPTOSYM_AES128_KEY_SIZE,
        aKey,
        &wKeyType));

    /* Invalid key type at wKeyNo and wKeyVer */
    if (wKeyType != PH_CRYPTOSYM_KEY_TYPE_AES128)
    {
        return PH_ADD_COMPCODE(PH_ERR_KEY, PH_COMP_AL_MFUL);
    }

    /* load key */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadKeyDirect(
        pDataParams->pCryptoDataParams,
        aKey,
        wKeyType));

    /* Send the cmd and receive the encrypted RndB */
    aCmdBuff[wCmdLen++] = PHAL_MFUL_CMD_AUTH;
    aCmdBuff[wCmdLen++] = bKeyNoCard; /* key number card */

    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen));

    /* Verify the status. */
    if ((wRxlen != (PH_CRYPTOSYM_AES_BLOCK_SIZE + 1)) || (pRecv[0] != PHAL_MFUL_PREAMBLE_TX))
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* Store the unencrypted RndB */
    (void)memcpy(aRndB, &pRecv[1], PH_CRYPTOSYM_AES_BLOCK_SIZE);

    /* Load Iv.*/
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        phalMful_Sw_FirstIv,
        PH_CRYPTOSYM_AES_BLOCK_SIZE));

    /* Decrypt the RndB received from PICC */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Decrypt(
        pDataParams->pCryptoDataParams,
        (uint16_t) (PH_CRYPTOSYM_CIPHER_MODE_CBC | PH_EXCHANGE_DEFAULT),
        aRndB,
        PH_CRYPTOSYM_AES_BLOCK_SIZE,
        aRndB));

    /* Generate RndA */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoRng_Seed(pDataParams->pCryptoRngDataParams, aRndB, PH_CRYPTOSYM_AES_BLOCK_SIZE));
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoRng_Rnd(pDataParams->pCryptoRngDataParams, PH_CRYPTOSYM_AES_BLOCK_SIZE, aRndA));

    /* Concat RndA and RndB' */
    aCmdBuff[0] = PHAL_MFUL_PREAMBLE_TX;
    (void)memcpy(&aCmdBuff[1], aRndA, PH_CRYPTOSYM_AES_BLOCK_SIZE);
    (void)memcpy(&aCmdBuff[PH_CRYPTOSYM_AES_BLOCK_SIZE + 1], &aRndB[1], (PH_CRYPTOSYM_AES_BLOCK_SIZE - 1));
    aCmdBuff[2 * PH_CRYPTOSYM_AES_BLOCK_SIZE] = aRndB[0]; /* RndB left shifted by 8 bits */

    /* Load Iv */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        phalMful_Sw_FirstIv,
        PH_CRYPTOSYM_AES_BLOCK_SIZE));

    /* Encrypt RndA + RndB' */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Encrypt(
        pDataParams->pCryptoDataParams,
        (uint16_t) (PH_CRYPTOSYM_CIPHER_MODE_CBC | PH_EXCHANGE_DEFAULT),
        &aCmdBuff[1],
        (uint16_t) (2 * PH_CRYPTOSYM_AES_BLOCK_SIZE),
        &aCmdBuff[1]));

    /* Update command length */
    wCmdLen = (uint16_t) (2 * PH_CRYPTOSYM_AES_BLOCK_SIZE + 1);

    /* Get the encrypted RndA' */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
        pDataParams->pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen));

    /* Verify the status. */
    if ((wRxlen != (PH_CRYPTOSYM_AES_BLOCK_SIZE + 1)) || (pRecv[0] != 0x00))
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* Load IV */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        phalMful_Sw_FirstIv,
        PH_CRYPTOSYM_AES_BLOCK_SIZE));

    /* Decrypt RndA' */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_Decrypt(
        pDataParams->pCryptoDataParams,
        (uint16_t) (PH_CRYPTOSYM_CIPHER_MODE_CBC | PH_EXCHANGE_DEFAULT),
        &pRecv[1],
        (uint16_t) (wRxlen - 1),
        &aCmdBuff[1]));

    aCmdBuff[0] = aCmdBuff[PH_CRYPTOSYM_AES128_KEY_SIZE];

    /* Compare RndA and buff */
    if (memcmp(aCmdBuff, aRndA, PH_CRYPTOSYM_AES128_KEY_SIZE) != 0)
    {
        /* Authentication failed */
        return PH_ADD_COMPCODE(PH_ERR_AUTH_ERROR, PH_COMP_AL_MFUL);
    }

    /* Generate the session key SV
     *  SV = 0x5A||0xA5||0x00||0x01||0x00||0x80||RndA[15:14]|| (RndA[13::8] XOR RndB[15::10])||RndB[9::0]||RndA[7::0]
     */
    aSV[0] = 0x5A;
    aSV[1] = 0xA5;
    aSV[2] = 0x00;
    aSV[3] = 0x01;
    aSV[4] = 0x00;
    aSV[5] = 0x80;
    aSV[6] = aRndA[0];
    aSV[7] = aRndA[1];

    aSV[8]  = aRndA[2] ^ aRndB[0];
    aSV[9]  = aRndA[3] ^ aRndB[1];
    aSV[10] = aRndA[4] ^ aRndB[2];
    aSV[11] = aRndA[5] ^ aRndB[3];
    aSV[12] = aRndA[6] ^ aRndB[4];
    aSV[13] = aRndA[7] ^ aRndB[5];

    (void)memcpy(&aSV[14], &aRndB[6], 10);
    (void)memcpy(&aSV[24], &aRndA[8], 8);

    /* Load IV */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        phalMful_Sw_FirstIv,
        PH_CRYPTOSYM_AES_BLOCK_SIZE));

   /* Calculate MAC for SV  */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_CalculateMac(
        pDataParams->pCryptoDataParams,
        (uint16_t) (PH_CRYPTOSYM_MAC_MODE_CMAC | PH_EXCHANGE_DEFAULT),
        aSV,
        32,
        aSesAuthMACKey,
        &bMacLen));

    /* Load the MAC session key */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadKeyDirect(
        pDataParams->pCryptoDataParams,
        aSesAuthMACKey,
        PH_CRYPTOSYM_KEY_TYPE_AES128));

    /* Session key is generated */
    pDataParams->wCmdCtr = 0x0000;

    /* Authentication Mode Set to AES */
    pDataParams->bAuthMode = PHAL_MFUL_CMD_AUTH;

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFUL);
}
#endif /* NXPBUILD__PH_CRYPTOSYM */

phStatus_t phalMful_Sw_Read(
                            phalMful_Sw_DataParams_t * pDataParams,
                            uint8_t bAddress,
                            uint8_t * pData
                            )
{
    return phalMful_Int_Read(pDataParams, bAddress, pData);
}

phStatus_t phalMful_Sw_Write(
                             phalMful_Sw_DataParams_t * pDataParams,
                             uint8_t bAddress,
                             uint8_t * pData
                             )
{
    return phalMful_Int_Write(pDataParams, bAddress, pData);
}

phStatus_t phalMful_Sw_FastWrite(
                                 phalMful_Sw_DataParams_t * pDataParams,
                                 uint8_t * pData
                                 )
{
    return phalMful_Int_FastWrite(pDataParams->pPalMifareDataParams, pData);
}

phStatus_t phalMful_Sw_CompatibilityWrite(
    phalMful_Sw_DataParams_t * pDataParams,
    uint8_t bAddress,
    uint8_t * pData
    )
{
    return phalMful_Int_CompatibilityWrite(pDataParams->pPalMifareDataParams, bAddress, pData);
}

phStatus_t phalMful_Sw_IncrCnt(
                               phalMful_Sw_DataParams_t * pDataParams,
                               uint8_t bCntNum,
                               uint8_t * pCnt
                               )
{
    return phalMful_Int_IncrCnt(pDataParams, bCntNum, pCnt);
}

phStatus_t phalMful_Sw_ReadCnt(
                               phalMful_Sw_DataParams_t * pDataParams,
                               uint8_t bCntNum,
                               uint8_t * pCntValue
                               )
{
    return phalMful_Int_ReadCnt(pDataParams, bCntNum, pCntValue);
}

phStatus_t phalMful_Sw_PwdAuth(
                               phalMful_Sw_DataParams_t * pDataParams,
                               uint8_t * pPwd,
                               uint8_t * pPack
                               )
{
    return phalMful_Int_PwdAuth(pDataParams->pPalMifareDataParams, pPwd, pPack);
}

phStatus_t phalMful_Sw_GetVersion(
                                  phalMful_Sw_DataParams_t * pDataParams,
                                  uint8_t * pPwd
                                  )
{
    return phalMful_Int_GetVersion(pDataParams, pPwd);
}

phStatus_t phalMful_Sw_FastRead(
                                phalMful_Sw_DataParams_t * pDataParams,
                                uint8_t  bStartAddr,
                                uint8_t bEndAddr,
                                uint8_t ** ppData,
                                uint16_t * pNumBytes
                                )
{
    return phalMful_Int_FastRead(pDataParams, bStartAddr, bEndAddr, ppData, pNumBytes);
}

phStatus_t phalMful_Sw_SectorSelect(
                                    phalMful_Sw_DataParams_t * pDataParams,
                                    uint8_t bSecNo
                                    )
{
    return phalMful_Int_SectorSelect(pDataParams->pPalMifareDataParams, bSecNo);
}

phStatus_t phalMful_Sw_ReadSign(phalMful_Sw_DataParams_t * pDataParams, uint8_t bAddr, uint8_t ** pSignature,uint16_t* pDataLen)
{
    return phalMful_Int_ReadSign(pDataParams, bAddr, pSignature, pDataLen );
}

phStatus_t phalMful_Sw_ChkTearingEvent(
                                       phalMful_Sw_DataParams_t * pDataParams,
                                       uint8_t bCntNum,
                                       uint8_t * pValidFlag
                                       )
{
    return phalMful_Int_ChkTearingEvent(pDataParams->pPalMifareDataParams, bCntNum, pValidFlag);
}

phStatus_t phalMful_Sw_WriteSign(
                                 phalMful_Sw_DataParams_t * pDataParams,
                                 uint8_t bAddress,
                                 uint8_t * pSignature
                                 )
{
    return phalMful_Int_WriteSign(pDataParams, bAddress, pSignature);
}

phStatus_t phalMful_Sw_LockSign(
                                phalMful_Sw_DataParams_t * pDataParams,
                                uint8_t bLockMode
                                )
{
    return phalMful_Int_LockSign(pDataParams, bLockMode);
}

phStatus_t phalMful_Sw_VirtualCardSelect(
                                         phalMful_Sw_DataParams_t * pDataParams,
                                         uint8_t * pVCIID,
                                         uint8_t bVCIIDLen,
                                         uint8_t * pVCTID
                                         )
{
    return phalMful_Int_VirtualCardSelect (pDataParams->pPalMifareDataParams, pVCIID, bVCIIDLen, pVCTID);
}

phStatus_t phalMful_Sw_ReadTTStatus(phalMful_Sw_DataParams_t * pDataParams, uint8_t bAddr, uint8_t * pData)
{
    return phalMful_Int_ReadTTStatus(pDataParams->pPalMifareDataParams, bAddr, pData);
}

#ifdef NXPBUILD__PH_CRYPTOSYM
phStatus_t phalMful_Sw_GetConfig(phalMful_Sw_DataParams_t *pDataParams, uint16_t wConfig, uint16_t * pValue)
{
    switch (wConfig)
    {
        case PHAL_MFUL_CMAC_STATUS:
            *pValue = pDataParams->bCMACReq;
            break;

        case PHAL_MFUL_ADDITIONAL_INFO:
            *pValue = pDataParams->bAdditionalInfo;
            break;

        default:
            return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_AL_MFUL);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFUL);
}

phStatus_t phalMful_Sw_SetConfig(phalMful_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t wValue)
{
    switch (wConfig)
    {
        case PHAL_MFUL_CMAC_STATUS:
            pDataParams->bCMACReq = (uint8_t) wValue;
            break;

            case PHAL_MFUL_ADDITIONAL_INFO:
            pDataParams->bAdditionalInfo = (uint8_t)wValue;
            break;

        default:
            return PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_AL_MFUL);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFUL);
}

phStatus_t phalMful_Sw_CalculateSunCMAC(phalMful_Sw_DataParams_t * pDataParams, uint16_t wKeyNo,
    uint16_t wKeyVer, uint8_t * pInData, uint16_t wInDataLen, uint8_t * pRespMac)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aMac[PH_CRYPTOSYM_AES_BLOCK_SIZE];
    uint8_t     PH_MEMLOC_REM bMacLen = 0;
    uint8_t     PH_MEMLOC_REM aIV[PH_CRYPTOSYM_AES_BLOCK_SIZE];
    uint8_t     PH_MEMLOC_REM aKey[PH_CRYPTOSYM_AES128_KEY_SIZE] ;
    uint16_t    PH_MEMLOC_REM wKeyType;

    /* Now Get the Keys from SW Key Store */
    PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVer,
        PH_CRYPTOSYM_AES128_KEY_SIZE,
        aKey,
        &wKeyType));

    /* Invalid key type at wKeyNo and wKeyVer */
    if (wKeyType != PH_CRYPTOSYM_KEY_TYPE_AES128)
    {
        return PH_ADD_COMPCODE(PH_ERR_KEY, PH_COMP_AL_MFUL);
    }

    /* load key */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadKeyDirect(
        pDataParams->pCryptoDataParams,
        aKey,
        wKeyType));

    /* Load zero to IV */
    (void)memset(aIV, 0x00, PH_CRYPTOSYM_AES_BLOCK_SIZE);

    /* Load the IV. */
    PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadIv(
        pDataParams->pCryptoDataParams,
        aIV,
        PH_CRYPTOSYM_AES_BLOCK_SIZE));

    /* Compute the MAC. */
        PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_CalculateMac(
        pDataParams->pCryptoDataParams,
        (PH_CRYPTOSYM_MAC_MODE_CMAC | PH_EXCHANGE_DEFAULT),
        pInData,
        wInDataLen,
        aMac,
        &bMacLen));

    /* Truncate the MAC generated */
    phalMful_Int_TruncateMac(aMac);

    /* Copy the Truncated CMAC into the return buffer */
    (void)memcpy(pRespMac, aMac, 8);

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_MFUL);
}
#endif /* NXPBUILD__PH_CRYPTOSYM */
#endif /* NXPBUILD__PHAL_MFUL_SW */
