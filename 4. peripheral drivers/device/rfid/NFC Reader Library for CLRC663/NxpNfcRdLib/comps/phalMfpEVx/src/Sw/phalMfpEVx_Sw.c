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

#include <ph_Status.h>
#include <phhalHw.h>
#include <phalMfpEVx.h>
#include <phpalMifare.h>

#ifdef NXPBUILD__PH_CRYPTOSYM
#include <phCryptoSym.h>
#endif /* NXPBUILD__PH_CRYPTOSYM */
#ifdef NXPBUILD__PH_CRYPTORNG
#include <phCryptoRng.h>
#endif /* NXPBUILD__PH_CRYPTORNG */

#include <phKeyStore.h>
#include <ph_RefDefs.h>
#include <phTMIUtils.h>
#include <phalVca.h>

#ifdef NXPBUILD__PHAL_MFPEVX_SW

#include "../phalMfpEVx_Int.h"
#include "phalMfpEVx_Sw.h"



phStatus_t phalMfpEVx_Sw_Init(phalMfpEVx_Sw_DataParams_t * pDataParams, uint16_t wSizeOfDataParams, void * pPalMifareDataParams,
    void * pKeyStoreDataParams, void * pCryptoDataParamsEnc, void * pCryptoDataParamsMac, void * pCryptoRngDataParams,
    void * pCryptoDiversifyDataParams, void * pTMIDataParams, void * pVCADataParams)
{
    phStatus_t PH_MEMLOC_REM wStatus = 0;

    /* data param check */
    if(sizeof(phalMfpEVx_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFPEVX);
    }
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFPEVX);
    PH_ASSERT_NULL_PARAM(pPalMifareDataParams, PH_COMP_AL_MFPEVX);


    PH_ASSERT_NULL_PARAM(pTMIDataParams, PH_COMP_AL_MFPEVX);

#ifdef NXPBUILD__PHAL_VCA
    PH_ASSERT_NULL_PARAM(pVCADataParams, PH_COMP_AL_MFPEVX);
#endif /* NXPBUILD__PHAL_VCA */

    /* init private data */
    pDataParams->wId = PH_COMP_AL_MFPEVX | PHAL_MFPEVX_SW_ID;
    pDataParams->pPalMifareDataParams = pPalMifareDataParams;
    pDataParams->pKeyStoreDataParams = pKeyStoreDataParams;
    pDataParams->pCryptoDataParamsEnc = pCryptoDataParamsEnc;
    pDataParams->pCryptoDataParamsMac = pCryptoDataParamsMac;
    pDataParams->pCryptoRngDataParams = pCryptoRngDataParams;
    pDataParams->pCryptoDiversifyDataParams = pCryptoDiversifyDataParams;
    pDataParams->pTMIDataParams = pTMIDataParams;
    pDataParams->bWrappedMode = 0x00;       /* Use native mode by default */
    pDataParams->bExtendedLenApdu = 0x00;       /* Use short length APDU by default */
    pDataParams->pVCADataParams = pVCADataParams;
    pDataParams->bAuthMode = (uint8_t) PHAL_MFPEVX_NOTAUTHENTICATED;
    pDataParams->bSMMode = (uint8_t) PHAL_MFPEVX_SECURE_MESSAGE_EV0;

    (void) memset(pDataParams->bSesAuthENCKey, 0x00, 16);
    (void) memset(pDataParams->bSesAuthMACKey, 0x00, 16);
    (void) memset(pDataParams->bIv, 0x00, 16);

    /* clear the secure messaging state */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Sw_ResetSecMsgState(pDataParams));

    return PH_ERR_SUCCESS;
}





/***************************************************************************************************************************************/
/* Mifare Plus EVx Software command for personalization.                                                                               */
/***************************************************************************************************************************************/
phStatus_t phalMfpEVx_Sw_WritePerso(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bLayer4Comm, uint16_t wBlockNr,
    uint8_t bNumBlocks, uint8_t * pValue)
{
    return phalMfpEVx_Int_WritePerso(pDataParams->pPalMifareDataParams, bLayer4Comm, pDataParams->bWrappedMode, pDataParams->bExtendedLenApdu,
        wBlockNr, bNumBlocks, pValue);
}

phStatus_t phalMfpEVx_Sw_CommitPerso(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bLayer4Comm)
{
    return phalMfpEVx_Int_CommitPerso(pDataParams->pPalMifareDataParams, bOption, bLayer4Comm, pDataParams->bWrappedMode,
        pDataParams->bExtendedLenApdu);
}





/***************************************************************************************************************************************/
/* Mifare Plus EVx Software command for authentication.                                                                                */
/***************************************************************************************************************************************/
phStatus_t phalMfpEVx_Sw_AuthenticateMfc(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bBlockNo, uint8_t bKeyType, uint16_t wKeyNo,
    uint16_t wKeyVer, uint8_t * pUid, uint8_t bUidLength)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aKey[PH_KEYSTORE_MAX_KEY_SIZE];
    uint8_t *   PH_MEMLOC_REM pKey = NULL;
    uint16_t    PH_MEMLOC_REM bKeystoreKeyType = 0;

    /* Verify the uid length. */
    if((bUidLength != PHAL_MFPEVX_UID_LENGTH_4B) &&
        (bUidLength != PHAL_MFPEVX_UID_LENGTH_7B) &&
        (bUidLength != PHAL_MFPEVX_UID_LENGTH_10B))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFPEVX);
    }

    /* Return an error if keystore is not initialized. */
    if(pDataParams->pKeyStoreDataParams == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_KEY, PH_COMP_AL_MFPEVX);
    }

    /* Check the keytype for KEYA or KEYB. */
    if((bKeyType != PHHAL_HW_MFC_KEYA) && (bKeyType != PHHAL_HW_MFC_KEYB))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFPEVX);
    }

    /* Retrieve KeyA & KeyB value from keystore. */
    PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVer,
        PH_KEYSTORE_KEY_TYPE_AES256_SIZE,
        aKey,
        &bKeystoreKeyType));

    /* Check the key type available in the keystore. */
    if(bKeystoreKeyType != PH_KEYSTORE_KEY_TYPE_MIFARE)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_KEY, PH_COMP_AL_MFPEVX);
    }

    /* Evaluate which key to use. */
    if(bKeyType == PHHAL_HW_MFC_KEYA)
    {
        /* Use KeyA */
        pKey = aKey;
    }
    else
    {
        /* Use KeyB */
        pKey = &aKey[PHHAL_HW_MFC_KEY_LENGTH];
    }

    /* Authenticate in MFC mode. */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_MfcAuthenticate(
        pDataParams->pPalMifareDataParams,
        bBlockNo,
        bKeyType,
        pKey,
        &pUid[bUidLength - 4]));

    /* Update the Auth Mode to MIFARE Authenticated. */
    pDataParams->bAuthMode = (uint8_t) PHAL_MFPEVX_SL1_MIFARE_AUTHENTICATED;

    return PH_ERR_SUCCESS;
}






/***************************************************************************************************************************************/
/* Mifare Plus EVx Software command for special operations.                                                                            */
/***************************************************************************************************************************************/
phStatus_t phalMfpEVx_Sw_GetVersion(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t * pResponse, uint8_t * pVerLen)
{
    uint16_t    PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aCmdBuf[PHAL_MFPEVX_VERSION_COMMAND_LENGTH];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;
    uint8_t*    PH_MEMLOC_REM pRxBuffer = NULL;
    uint16_t    PH_MEMLOC_REM wRxBufLen = 0;
    uint8_t     PH_MEMLOC_REM aVersion[PHAL_MFPEVX_VERSION_INFO_LENGTH];
    uint8_t     PH_MEMLOC_REM bVerLen = 0;

    /* Frame the command. */
    aCmdBuf[bCmdLen++] = PHAL_MFPEVX_CMD_GET_VERSION;


    /* Exchange the first part of the command. */
    /* Check if ISO 7816-4 wapping is required. */
    if(0U != pDataParams->bWrappedMode)
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_Send7816Apdu(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            (uint16_t) (bCmdLen - 1U /* Excluding the command code. */),
            pDataParams->bExtendedLenApdu,
            aCmdBuf,
            bCmdLen,
            &pRxBuffer,
            &wRxBufLen));
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            aCmdBuf,
            bCmdLen,
            &pRxBuffer,
            &wRxBufLen));
    }

    /* Check if additional frame byte is available in the response. */
    if((wRxBufLen != (1U /* Status */ + PHAL_MFPEVX_VERSION_PART1_LENGTH)) && (pRxBuffer[0] != PHAL_MFPEVX_RESP_ADDITIONAL_FRAME))
    {
        return phalMfpEVx_Int_ComputeErrorResponse(wRxBufLen, pRxBuffer[0], PH_ON);
    }

    /* Copy the Version A into version buffer and update the version buffer length .*/
    (void) memcpy(&aVersion[bVerLen], &pRxBuffer[1], PHAL_MFPEVX_VERSION_PART1_LENGTH);
    bVerLen = PHAL_MFPEVX_VERSION_PART1_LENGTH;

    /* Update the additional command frame code to command buffer. */
    aCmdBuf[0] = PHAL_MFPEVX_RESP_ADDITIONAL_FRAME;
    bCmdLen = 1;

    /* Exchange the second part of the command. */
    /* Check if ISO 7816-4 wapping is required. */
    if(0U != pDataParams->bWrappedMode)
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_Send7816Apdu(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            (uint16_t) (bCmdLen - 1U /* Excluding the command code. */),
            pDataParams->bExtendedLenApdu,
            aCmdBuf,
            bCmdLen,
            &pRxBuffer,
            &wRxBufLen));
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            aCmdBuf,
            bCmdLen,
            &pRxBuffer,
            &wRxBufLen));
    }

    /* Check if additional frame byte is available in the response. */
    if((wRxBufLen != (1U /* Status */ + PHAL_MFPEVX_VERSION_PART2_LENGTH)) && (pRxBuffer[0] != PHAL_MFPEVX_RESP_ADDITIONAL_FRAME))
    {
        return phalMfpEVx_Int_ComputeErrorResponse(wRxBufLen, pRxBuffer[0], PH_ON);
    }

    /* Copy the Version B into version buffer and update the version buffer length .*/
    (void) memcpy(&aVersion[bVerLen], &pRxBuffer[1], PHAL_MFPEVX_VERSION_PART2_LENGTH);
    bVerLen = bVerLen + PHAL_MFPEVX_VERSION_PART2_LENGTH;

    /* Update the additional command frame code to command buffer. */
    aCmdBuf[0] = PHAL_MFPEVX_RESP_ADDITIONAL_FRAME;
    bCmdLen = 1;

    /* Exchange the third part of the command. */
    /* Check if ISO 7816-4 wapping is required. */
    if(0U != pDataParams->bWrappedMode)
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_Send7816Apdu(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            (uint16_t) (bCmdLen - 1 /* Excluding the command code. */),
            pDataParams->bExtendedLenApdu,
            aCmdBuf,
            bCmdLen,
            &pRxBuffer,
            &wRxBufLen));
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            aCmdBuf,
            bCmdLen,
            &pRxBuffer,
            &wRxBufLen));
    }

    /* Check if authenticated in MFP mode. */
    if((pDataParams->bAuthMode == PHAL_MFPEVX_SL1_MFP_AUTHENTICATED) || (pDataParams->bAuthMode == PHAL_MFPEVX_SL3_MFP_AUTHENTICATED))
    {
    }
    else
    {
        /* Check if ISO14443 L4 success byte is available in the response. */
        if((wRxBufLen != (1U /* Status */ + PHAL_MFPEVX_VERSION_PART3_LENGTH_04B)) &&
            (wRxBufLen != (1U /* Status */ + PHAL_MFPEVX_VERSION_PART3_LENGTH_07B)) &&
            (wRxBufLen != (1U /* Status */ + PHAL_MFPEVX_VERSION_PART3_LENGTH_10B)) &&
            (pRxBuffer[0] != PHAL_MFPEVX_RESP_ACK_ISO4))
        {
            return phalMfpEVx_Int_ComputeErrorResponse(wRxBufLen, pRxBuffer[0], PH_ON);
        }

        /* Copy the VersionC bytes into version buffer and update the version buffer length .*/
        (void) memcpy(&aVersion[bVerLen], &pRxBuffer[1], (wRxBufLen - 1U /* Status */));
        bVerLen = (uint8_t) (bVerLen + (wRxBufLen - 1U /* Status code excluded. */)) /* Version C length. */;
    }

    /* Copy the local version buffer to the reference parameter. */
    (void) memcpy(pResponse, aVersion, bVerLen);
    *pVerLen = bVerLen;

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Sw_ReadSign(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bLayer4Comm, uint8_t bAddr, uint8_t ** pSignature)
{

    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM bCmdBuff[1 /* command code */ + 2 /* R_Ctr */ + PHAL_MFPEVX_SIZE_TI + 1 /* Address */ + PHAL_MFPEVX_SIG_LENGTH_ENC];
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
/* build command frame */
    bCmdBuff[wCmdLen++] = PHAL_MFPEVX_CMD_READ_SIG;

    /* Req spec(ver 0.06 says),
     * 1. Cmd.Read_Sig shall return the NXPOriginalitySignature as written during wafer test in plain if not authenticated
     * 2. Cmd.Read_Sig shall require MACed command if authenticated.
     */
     /* Exchange command/response after Layer 4 activation */
    if(0U != bLayer4Comm)
    {
        switch(pDataParams->bAuthMode)
        {

            case PHAL_MFPEVX_NOTAUTHENTICATED:
            case PHAL_MFPEVX_NOT_AUTHENTICATED_L3:
            case PHAL_MFPEVX_NOT_AUTHENTICATED_L4:
            case PHAL_MFPEVX_SL1_MIFARE_AUTHENTICATED:
                /* PICC not Authenticated */
                bCmdBuff[wCmdLen++] = bAddr;
                break;

            default:
                break;
        }

        /* Check if ISO 7816-4 wrapping is required */
        if(0U != pDataParams->bWrappedMode)
        {
            /* buffer the header */
            PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_Send7816Apdu(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_DEFAULT,
                wCmdLen - 1U,   /* Excluding the command code */
                pDataParams->bExtendedLenApdu,
                bCmdBuff,
                wCmdLen,        /* Command code is included as part of length. */
                &pResponse,
                &wRxLength));
        }
        else
        {
            /* buffer the header */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_DEFAULT,
                bCmdBuff,
                wCmdLen,
                &pResponse,
                &wRxLength));
        }
    }
    else
    {
        bCmdBuff[wCmdLen++] = bAddr;
        /* command exchange in layer 3 */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            bCmdBuff,
            wCmdLen,
            &pResponse,
            &wRxLength));

    }

    PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_ComputeErrorResponse(wRxLength, pResponse[0], bLayer4Comm));

    /* Check received length for 64 (Encrypted Signature) + 8 (MAC) bytes */
    switch(pDataParams->bAuthMode)
    {

        case PHAL_MFPEVX_NOTAUTHENTICATED:
        case PHAL_MFPEVX_NOT_AUTHENTICATED_L3:
        case PHAL_MFPEVX_NOT_AUTHENTICATED_L4:
        case PHAL_MFPEVX_SL1_MIFARE_AUTHENTICATED:
            /* Check for 56 bytes of Signature data + 1 byte status code */
            if(wRxLength != (PHAL_MFPEVX_SIG_LENGTH + 1U))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_LENGTH_ERROR, PH_COMP_AL_MFPEVX);
            }
            else
            {
                /* Increment receive buffer to point to first byte.
                * then Assign the decrypted response to signature
                * as the pResponse[0] = status code(90).
                */
                ++pResponse;
                *pSignature = pResponse;
            }
            break;

        default:
            break;
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Sw_ResetAuth(phalMfpEVx_Sw_DataParams_t * pDataParams)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;

    /* Perform ResetAuth. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_ResetAuth(pDataParams->pPalMifareDataParams, pDataParams->bWrappedMode, pDataParams->bExtendedLenApdu));

    /* Reset the crypto layer */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_ResetSecMsgState(pDataParams));

#ifdef NXPBUILD__PHAL_VCA
    /* Update the authentication state if VCA PC feature is required by the application. */
    if(pDataParams->pVCADataParams != NULL)
    {
        /* Reset VCA session */
        PH_CHECK_SUCCESS_FCT(wStatus, phalVca_SetSessionKeyUtility((phalVca_Sw_DataParams_t *) pDataParams->pVCADataParams,
            pDataParams->bSesAuthENCKey,
            pDataParams->bAuthMode));
    }
#endif /* NXPBUILD__PHAL_VCA */

    /* return exchange status code */
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Sw_PersonalizeUid(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bUidType)
{
    return phalMfpEVx_Int_PersonalizeUid(pDataParams->pPalMifareDataParams, bUidType);
}

phStatus_t phalMfpEVx_Sw_SetConfigSL1(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t bOption)
{
    return phalMfpEVx_Int_SetConfigSL1(pDataParams->pPalMifareDataParams, bOption);
}

phStatus_t phalMfpEVx_Sw_ReadSL1TMBlock(phalMfpEVx_Sw_DataParams_t * pDataParams, uint16_t wBlockNr, uint8_t * pBlocks)
{
    return phalMfpEVx_Int_ReadSL1TMBlock(pDataParams->pPalMifareDataParams, wBlockNr, pBlocks);
}

phStatus_t phalMfpEVx_Sw_VCSupportLastISOL3(phalMfpEVx_Sw_DataParams_t * pDataParams, uint8_t * pIid, uint8_t * pPcdCapL3, uint8_t * pInfo)
{
    return phalMfpEVx_Int_VCSupportLastISOL3(pDataParams->pPalMifareDataParams, pIid, pPcdCapL3, pInfo);
}






/***************************************************************************************************************************************/
/* Mifare Plus EVx Software command for utility operations.                                                                            */
/***************************************************************************************************************************************/
phStatus_t phalMfpEVx_Sw_ResetSecMsgState(phalMfpEVx_Sw_DataParams_t * pDataParams)
{
    phStatus_t wStatus = 0;

    pDataParams->wRCtr = 0;
    pDataParams->wWCtr = 0;
    pDataParams->bNumUnprocessedReadMacBytes = 0;
    pDataParams->bFirstRead = 1;
    pDataParams->bSMMode = (uint8_t) PHAL_MFPEVX_SECURE_MESSAGE_EV0;

    /* State machine should be handled in a way where L3 activation or L4 activation shouldnot be lost */
    if((pDataParams->bAuthMode == PHAL_MFPEVX_SL3_MFP_AUTHENTICATED) ||
        (pDataParams->bAuthMode == PHAL_MFPEVX_SL1_MFP_AUTHENTICATED) ||
        (pDataParams->bAuthMode == PHAL_MFPEVX_NOT_AUTHENTICATED_L4))
    {
        pDataParams->bAuthMode = PHAL_MFPEVX_NOT_AUTHENTICATED_L4;
    }
    else if((pDataParams->bAuthMode == PHAL_MFPEVX_NOT_AUTHENTICATED_L3) ||
        (pDataParams->bAuthMode == PHAL_MFPEVX_SL1_MIFARE_AUTHENTICATED))
    {
        pDataParams->bAuthMode = PHAL_MFPEVX_NOT_AUTHENTICATED_L3;
    }
    else
    {
        pDataParams->bAuthMode = PHAL_MFPEVX_NOTAUTHENTICATED;
    }

    (void) memset(pDataParams->bIv, 0x00, (size_t) sizeof(pDataParams->bIv));
    (void) memset(pDataParams->bSesAuthENCKey, 0x00, (size_t) sizeof(pDataParams->bSesAuthENCKey));
    (void) memset(pDataParams->bSesAuthMACKey, 0x00, (size_t) sizeof(pDataParams->bSesAuthMACKey));

    (void) memset(pDataParams->bTi, 0x00, PHAL_MFPEVX_SIZE_TI);

    wStatus = phTMIUtils_ActivateTMICollection((phTMIUtils_t *) pDataParams->pTMIDataParams, PH_TMIUTILS_RESET_TMI);

#ifdef NXPBUILD__PHAL_VCA
    /* Update the authentication state if VCA PC feature is required by the application. */
    if(pDataParams->pVCADataParams != NULL)
    {
        wStatus = phalVca_SetSessionKeyUtility(pDataParams->pVCADataParams, pDataParams->bSesAuthMACKey,
            PHAL_MFPEVX_NOTAUTHENTICATED);
    }
#endif /* NXPBUILD__PHAL_VCA */

    return PH_ADD_COMPCODE(wStatus, PH_COMP_AL_MFPEVX);
}

phStatus_t phalMfpEVx_Sw_SetConfig(phalMfpEVx_Sw_DataParams_t * pDataParams, uint16_t wOption, uint16_t wValue)
{
    switch(wOption)
    {
        case PHAL_MFPEVX_WRAPPED_MODE:
            pDataParams->bWrappedMode = (uint8_t) wValue;
            break;

        case PHAL_MFPEVX_EXTENDED_APDU:
            pDataParams->bExtendedLenApdu = (uint8_t) wValue;
            break;

        case PHAL_MFPEVX_AUTH_MODE:
            pDataParams->bAuthMode = (uint8_t) wValue;
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_AL_MFPEVX);
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Sw_GetConfig(phalMfpEVx_Sw_DataParams_t *pDataParams, uint16_t wOption, uint16_t * pValue)
{
    switch(wOption)
    {
        case PHAL_MFPEVX_WRAPPED_MODE:
            *pValue = (uint16_t) pDataParams->bWrappedMode;
            break;

        case PHAL_MFPEVX_EXTENDED_APDU:
            *pValue = (uint16_t) pDataParams->bExtendedLenApdu;
            break;

        case PHAL_MFPEVX_AUTH_MODE:
            *pValue = (uint16_t) pDataParams->bAuthMode;
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_AL_MFPEVX);
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Sw_SetVCAParams(phalMfpEVx_Sw_DataParams_t * pDataParams, void * pAlVCADataParams)
{
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pAlVCADataParams);

    pDataParams->pVCADataParams = pAlVCADataParams;

    return PH_ERR_SUCCESS;
}






#endif /* NXPBUILD__PHAL_MFPEVX_SW */
