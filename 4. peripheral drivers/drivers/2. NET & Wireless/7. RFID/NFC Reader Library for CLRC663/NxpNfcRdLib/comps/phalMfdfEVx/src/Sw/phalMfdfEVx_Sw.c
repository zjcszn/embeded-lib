/*----------------------------------------------------------------------------*/
/* Copyright 2014-2020,2022,2023 NXP                                          */
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
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <ph_TypeDefs.h>
#include <string.h>
#include <phTools.h>
#include <phKeyStore.h>

#ifdef NXPBUILD__PH_CRYPTOSYM
#include <phCryptoSym.h>
#endif /* NXPBUILD__PH_CRYPTOSYM */

#ifdef NXPBUILD__PH_CRYPTORNG
#include <phCryptoRng.h>
#endif /* NXPBUILD__PH_CRYPTORNG */

#include <phTMIUtils.h>
#include <phalVca.h>

#ifdef NXPBUILD__PHAL_MFDFEVX_SW

#include "../phalMfdfEVx_Int.h"
#include "phalMfdfEVx_Sw.h"
#include "phalMfdfEVx_Sw_Int.h"

/* APP level keys are invalid between 0x0D to 0x21. */
 #define IS_INVALID_APP_KEY(keyNo) ((((keyNo) & 0x7fU) > 0x0DU)     && (((keyNo) & 0x7fU) < 0x21U))

/* VC keys are invalid after 0x23. */
 #define IS_INVALID_VC_KEY(keyNo)   (((keyNo) & 0x7fU) > 0x23U)


phStatus_t phalMfdfEVx_Sw_Init(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wSizeOfDataParams, void * pPalMifareDataParams,
    void * pKeyStoreDataParams, void * pCryptoDataParamsEnc, void * pCryptoDataParamsMac, void * pCryptoRngDataParams,
    void * pTMIDataParams, void * pVCADataParams, void * pHalDataParams)
{
    /* data param check */
    if (sizeof(phalMfdfEVx_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pPalMifareDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pTMIDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pVCADataParams, PH_COMP_AL_MFDFEVX);

    /* init private data */
    pDataParams->wId = PH_COMP_AL_MFDFEVX | PHAL_MFDFEVX_SW_ID;
    pDataParams->pPalMifareDataParams = pPalMifareDataParams;
    pDataParams->pKeyStoreDataParams = pKeyStoreDataParams;
    pDataParams->pCryptoDataParamsEnc = pCryptoDataParamsEnc;
    pDataParams->pCryptoDataParamsMac = pCryptoDataParamsMac;
    pDataParams->pCryptoRngDataParams = pCryptoRngDataParams;
    pDataParams->pTMIDataParams = pTMIDataParams;
    pDataParams->pVCADataParams = pVCADataParams;
    pDataParams->pHalDataParams = pHalDataParams;
    /* 2 Byte CRC initial value in Authenticate mode. */
    pDataParams->wCrc = PH_TOOLS_CRC16_PRESET_ISO14443A;

    /* 4 Byte CRC initial value in 0x1A, 0xAA mode. */
    pDataParams->dwCrc = PH_TOOLS_CRC32_PRESET_DF8;

    (void)memset(pDataParams->bSesAuthENCKey, 0x00, 24);
    pDataParams->bKeyNo = 0xFF; /* Set to invalid */
    (void)memset(pDataParams->bIv, 0x00, 16);
    (void)memset(pDataParams->pAid, 0x00, 3);
    pDataParams->bAuthMode = PHAL_MFDFEVX_NOT_AUTHENTICATED; /* Set to invalid */
    pDataParams->bWrappedMode = 0x00; /* Set to false */
    pDataParams->bCryptoMethod = 0xFF; /* No crypto just after init */
    pDataParams->wAdditionalInfo = 0x0000;
    pDataParams->bShortLenApdu = 0x00; /* By default, extended length APDU format is used for BIG ISO Read */
    pDataParams->dwPayLoadLen = 0;
    pDataParams->wCmdCtr = 0;
    (void)memset(pDataParams->bTi, 0x00, PHAL_MFDFEVX_SIZE_TI);
    (void)memset(pDataParams->bSesAuthMACKey, 0x00, 16);
    (void)memset(pDataParams->pUnprocByteBuff, 0x00, PHAL_MFDFEVX_SIZE_MAC);
    pDataParams->bNoUnprocBytes = 0;
    (void)memset(pDataParams->bLastBlockBuffer, 0x00, 16);
    pDataParams->bLastBlockIndex = 0;
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_INVALID;

    return PH_ERR_SUCCESS;
}








/* MIFARE DESFire EVx Memory and Configuration mamangement commands. ------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_FreeMem(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pMemInfo)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_FREE_MEM;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_FREE_MEM;
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        bCmdBuff,
        1,
        &pRecv,
        &wRxlen
        ));

    if (wRxlen != 3U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }

    (void)memcpy(pMemInfo, pRecv, wRxlen);

    return PH_ERR_SUCCESS;
}


phStatus_t phalMfdfEVx_Sw_GetVersion(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pVerInfo)
{
    phStatus_t  PH_MEMLOC_REM statusTmp = 0;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_GET_VERSION;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_GET_VERSION;

    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        bCmdBuff,
        1,
        &pRecv,
        &wRxlen
        ));

    /* If received Data length is not equal to 28B(In case of 7BUID) or 30B(In case of 10B UID), 27B(In case of 4B UID)
    * then its a Protocol Error
    */
    if ((wRxlen != PHAL_MFDFEVX_DEF_VERSION_LENGTH) &&
        (wRxlen != PHAL_MFDFEVX_10B_VERSION_LENGTH) &&
        (wRxlen != PHAL_MFDFEVX_4B_VERSION_LENGTH))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }

    (void)memcpy(pVerInfo, pRecv, wRxlen);

    /* Do a Set Config of ADDITIONAL_INFO to set  the length(wLength) of the Version string */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_SetConfig((phalMfdfEVx_Sw_DataParams_t *)pDataParams, PHAL_MFDFEVX_ADDITIONAL_INFO,
        wRxlen));

    return PH_ERR_SUCCESS;
}


phStatus_t phalMfdfEVx_Sw_GetKeySettings(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pKeySettings,
    uint8_t * bRespLen)
{
    /**
    * This command can be issued without valid authentication
    */
    uint16_t    PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_GET_KEY_SETTINGS;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_GET_KEY_SETTINGS;

    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        bCmdBuff,
        1,
        &pRecv,
        &wRxlen
    ));

    if ((wRxlen != 0x02U) && (wRxlen != 0x06U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }
    (void)memcpy(pKeySettings, pRecv, wRxlen);
    *bRespLen = (uint8_t) wRxlen;

    return PH_ERR_SUCCESS;
}





/* MIFARE DESFire EVx Application mamangement commands. -------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_CreateApplication(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pAid,
    uint8_t bKeySettings1, uint8_t bKeySettings2, uint8_t bKeySettings3, uint8_t * pKeySetValues, uint8_t * pISOFileId,
    uint8_t * pISODFName, uint8_t bISODFNameLen)
{
    /*
    If (bKeySettings2 & 0x03)== 00 [DES, 3DES] then pDataParams->bAuthMode can be either
    0x0A or 0x1A.
    If (bKeySettings2 & 0x03)== 01 [3K3DES] then pDataParams->bAuthMode can only be 0x1A.
    If (bKeySettings2 & 0x03)== 10 [AES] then pDataParams->bAuthMode can be 0xAA or AuthEVx.
    */
    uint8_t     PH_MEMLOC_REM bCmdBuff[33];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if ((bISODFNameLen > 16U) || (bOption > 0x03U) || (bOption == 0x02U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CREATE_APPLN;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_CREATE_APPLN;

    (void)memcpy(&bCmdBuff[wCmdLen], pAid, 3);
    wCmdLen += 3U;

    bCmdBuff[wCmdLen++] = bKeySettings1;
    bCmdBuff[wCmdLen++] = bKeySettings2;

    if (0U != (bKeySettings2 & PHAL_MFDFEVX_KEYSETT3_PRESENT))
    {
        bCmdBuff[wCmdLen++] = bKeySettings3;
        if ((bKeySettings3 & PHAL_MFDFEVX_KEYSETVALUES_PRESENT) && (pKeySetValues != NULL))
        {
            /* KeySet Values */
            (void)memcpy(&bCmdBuff[wCmdLen], pKeySetValues, 4);
            wCmdLen += 4U;
        }
    }
    if (0U != (bOption & 0x01U))
    {
        /* pISOFileId is present */
        bCmdBuff[wCmdLen++] = pISOFileId[0];
        bCmdBuff[wCmdLen++] = pISOFileId[1];
    }
    if (0U != (bOption & 0x02U))
    {
        /* pISODFName is present */
        (void)memcpy(&bCmdBuff[wCmdLen], pISODFName, bISODFNameLen);
        wCmdLen = wCmdLen + bISODFNameLen;
    }

    return phalMfdfEVx_Sw_Int_Write_Plain(
        pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        wCmdLen,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}

phStatus_t phalMfdfEVx_Sw_DeleteApplication(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pAid, uint8_t * pDAMMAC, uint8_t bDAMMAC_Len)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[20];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_DELETE_APPLN;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_DELETE_APPLN;
    (void)memcpy(&bCmdBuff[wCmdLen], pAid, PHAL_MFDFEVX_DFAPPID_SIZE);
    wCmdLen += PHAL_MFDFEVX_DFAPPID_SIZE;

    /* Append the DAMMAC */
    if(bDAMMAC_Len)
    {
        (void)memcpy(&bCmdBuff[wCmdLen], pDAMMAC, bDAMMAC_Len);
        wCmdLen += bDAMMAC_Len;
    }

    return phalMfdfEVx_Sw_Int_Write_Plain(
        pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        wCmdLen,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}

phStatus_t phalMfdfEVx_Sw_CreateDelegatedApplication(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pAid,
    uint8_t * pDamParams, uint8_t bKeySettings1, uint8_t bKeySettings2, uint8_t bKeySettings3, uint8_t * bKeySetValues,
    uint8_t * pISOFileId, uint8_t * pISODFName, uint8_t bISODFNameLen, uint8_t * pEncK, uint8_t * pDAMMAC)
{
    phStatus_t  PH_MEMLOC_REM status;
    uint8_t     PH_MEMLOC_REM bCmdBuff[43];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if ((bISODFNameLen > 16U) || (bOption > 0x03U) || (bOption == 0x02U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CREATE_DELEGATED_APPLN;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_CREATE_DELEGATED_APPLN;

    /* Copy AID */
    (void)memcpy(&bCmdBuff[wCmdLen], pAid, 3);
    wCmdLen += 3U;

    /* Copy DAMSlotNo || DAMSlotVersion || QuotaLimit
     *  Size of DAMSlotNo is changed to 2 byte
     */
    (void)memcpy(&bCmdBuff[wCmdLen], pDamParams, 5);
    wCmdLen += 5U;

    bCmdBuff[wCmdLen++] = bKeySettings1;
    bCmdBuff[wCmdLen++] = bKeySettings2;

    if (0U != (bKeySettings2 & PHAL_MFDFEVX_KEYSETT3_PRESENT))
    {
        bCmdBuff[wCmdLen++] = bKeySettings3;
        if ((bKeySettings3 & PHAL_MFDFEVX_KEYSETVALUES_PRESENT) && (bKeySetValues != NULL))
        {
            /* KeySet Values */
            (void)memcpy(&bCmdBuff[wCmdLen], bKeySetValues, 4);
            wCmdLen += 4U;
        }
    }
    if (0U != (bOption & 0x01U))
    {
        /* pISOFileId is present */
        bCmdBuff[wCmdLen++] = pISOFileId[0];
        bCmdBuff[wCmdLen++] = pISOFileId[1];
    }
    if (0U != (bOption & 0x02U))
    {
        /* pISODFName is present */
        (void)memcpy(&bCmdBuff[wCmdLen], pISODFName, bISODFNameLen);
        wCmdLen += bISODFNameLen;
    }

    /* PHAL_MFDFEVX_MAC_DATA_INCOMPLETE should only be ORed for MACD communication mode.
    * For plain this will not be required.
    * First call to phalMfdfEVx_Sw_Int_Write_Plain should
    * get PHAL_MFDFEVX_COMMUNICATION_PLAIN as bCommOption only if D40, else
    * (PHAL_MFDFEVX_COMMUNICATION_MACD | PHAL_MFDFEVX_MAC_DATA_INCOMPLETE)
    */
    status = phalMfdfEVx_Sw_Int_Write_Plain(
        pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        wCmdLen,
        PHAL_MFDFEVX_MAC_DATA_INCOMPLETE | ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN),
        NULL,
        0x0000
        );

    if ((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        return PH_ADD_COMPCODE((status & PH_ERR_MASK), PH_COMP_AL_MFDFEVX);
    }

    bCmdBuff[0] = PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME;

    /* Copy the EncKey */
    (void)memcpy(&bCmdBuff[1], pEncK, 32);
    /* Copy the DAMMAC */
    (void)memcpy(&bCmdBuff[33], pDAMMAC, 8);

    return phalMfdfEVx_Sw_Int_Write_Plain(
        pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        1,
        ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN),
        &bCmdBuff[1],
        0x0028
        );
}

phStatus_t phalMfdfEVx_Sw_SelectApplication(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pAppId,
    uint8_t * pAppId2)
{
    uint16_t  PH_MEMLOC_REM statusTmp;
    uint8_t   PH_MEMLOC_REM bCmdBuff[8];
    uint16_t  PH_MEMLOC_REM wRxlen = 0;
    uint16_t  PH_MEMLOC_REM wCmdLen = 0;
    uint8_t * PH_MEMLOC_REM pRecv = NULL;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_SELECT_APPLN;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_SELECT_APPLN;
    (void)memcpy(&bCmdBuff[1], pAppId, PHAL_MFDFEVX_DFAPPID_SIZE);
    wCmdLen = PHAL_MFDFEVX_DFAPPID_SIZE + 1U;
    if (bOption == 0x01U)
    {
        /* Second application Id also provided */
        (void)memcpy(&bCmdBuff[4], pAppId2, PHAL_MFDFEVX_DFAPPID_SIZE);
        wCmdLen += PHAL_MFDFEVX_DFAPPID_SIZE;
    }

    /* Reset Authentication Status here */
    phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);

    /* Send the command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_ExchangeCmd(
        pDataParams,
        pDataParams->pPalMifareDataParams,
        pDataParams->bWrappedMode,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));

    /* Store the currently selected application Id */
    (void)memcpy(pDataParams->pAid, pAppId, 3);

    /* Select command never returns CMAC */
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_GetApplicationIDs(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t ** pAidBuff,
    uint8_t * pNumAIDs)
{
    /*
    * A PICC can store any number of applications limited by PICC memory
    */
    phStatus_t  PH_MEMLOC_REM status;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;

    /* Initialization */
    *pNumAIDs = 0;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_GET_APPLN_IDS;

    if ((bOption & 0x0FU) == PH_EXCHANGE_RXCHAINING)
    {
        bCmdBuff[0] = PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME;
    }
    else if ((bOption & 0x0FU) == PH_EXCHANGE_DEFAULT)
    {
        /* form the command */
        bCmdBuff[0] = PHAL_MFDFEVX_CMD_GET_APPLN_IDS;
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    /*
    * If EV2 authenticated, it is actually MAC mode in which this command
    * is sent. This is similar concept as in EV1 but the generated MAC
    * is attached with the command. The ReadData_Plain handles this.
    */
    status = phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        bCmdBuff,
        1,
        &pRecv,
        &wRxlen
        );

    if (((status & PH_ERR_MASK) == PH_ERR_SUCCESS) ||
        ((status & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING))
    {
        /* Length should be a multiple of 3. Else return error */
        if (0U != (wRxlen % 3))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }
        *pAidBuff = pRecv;

        /* Update pNumAids and return  */
        *pNumAIDs = (uint8_t)(wRxlen / 3);
    }
    return PH_ADD_COMPCODE(status, PH_COMP_AL_MFDFEVX);
}


phStatus_t phalMfdfEVx_Sw_GetDelegatedInfo(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pDAMSlot, uint8_t * pDamSlotVer,
    uint8_t * pQuotaLimit, uint8_t * pFreeBlocks, uint8_t * pAid)
{
    uint16_t    PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_GET_DELEGATED_INFO;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_GET_DELEGATED_INFO;
    (void)memcpy(&bCmdBuff[1], pDAMSlot, 2);

    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        bCmdBuff,
        3,
        &pRecv,
        &wRxlen
        ));

    /* Received data length should be equal to 6 bytes + 2 bytes of FreeBlocks */
    if (wRxlen != 0x08U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }

    (void)memcpy(pDamSlotVer, &pRecv[0], 1);
    (void)memcpy(pQuotaLimit, &pRecv[1], 2);
    (void)memcpy(pFreeBlocks, &pRecv[3], 2);
    (void)memcpy(pAid, &pRecv[5], 3);

    return PH_ERR_SUCCESS;

}




/* MIFARE DESFire EVx File mamangement commands. --------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_CreateStdDataFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bFileNo,
    uint8_t * pISOFileId, uint8_t bFileOption, uint8_t * pAccessRights, uint8_t * pFileSize)
{
    /*
    If (bKeySettings2 & 0x03)== 00 [DES, 3DES] then pDataParams->bAuthMode can be either
    0x0A or 0x1A.
    If (bKeySettings2 & 0x03)== 01 [3K3DES] then pDataParams->bAuthMode can only be 0x1A.
    If (bKeySettings2 & 0x03)== 10 [AES] then pDataParams->bAuthMode can only be 0xAA.
    */
    uint8_t     PH_MEMLOC_REM bCmdBuff[16];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if (((bFileNo & 0x7fU) > 0x1fU) || (bOption > 0x01U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    if (((bFileOption & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_PLAIN >> 4U)) &&
        ((bFileOption & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_ENC >> 4U)) &&
        ((bFileOption & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_MACD >> 4U)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CREATE_STD_DATAFILE;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_CREATE_STD_DATAFILE;

    /* File Number */
    bCmdBuff[wCmdLen++] = bFileNo;

    /* Copy ISO Filed ID if present */
    if (bOption == 0x01U)
    {
        bCmdBuff[wCmdLen++] = pISOFileId[0];
        bCmdBuff[wCmdLen++] = pISOFileId[1];
    }

    /* Copy communication settings. */
    bCmdBuff[wCmdLen++] = bFileOption;

    /* Copy Access rights */
    bCmdBuff[wCmdLen++] = pAccessRights[0];
    bCmdBuff[wCmdLen++] = pAccessRights[1];

    /* Copy File size supplied by the user */
    (void)memcpy(&bCmdBuff[wCmdLen], pFileSize, 3);
    wCmdLen += 3U;

    /* COMMUNICATION IS PLAIN */
    return phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        wCmdLen,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}

phStatus_t phalMfdfEVx_Sw_CreateBackupDataFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bFileNo,
    uint8_t * pISOFileId, uint8_t bFileOption, uint8_t * pAccessRights, uint8_t * pFileSize)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[16];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if (((bFileNo & 0x7fU) > 0x1fU) || (bOption > 0x01U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if (((bFileOption & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_PLAIN >> 4U)) &&
        ((bFileOption & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_ENC >> 4U)) &&
        ((bFileOption & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_MACD >> 4U)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CREATE_BKUP_DATAFILE;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_CREATE_BKUP_DATAFILE;

    /* File Number */
    bCmdBuff[wCmdLen++] = bFileNo;

    /* Copy ISO Filed ID if present */
    if (bOption == 0x01U)
    {
        bCmdBuff[wCmdLen++] = pISOFileId[0];
        bCmdBuff[wCmdLen++] = pISOFileId[1];
    }

    /* Copy communication settings. */
    bCmdBuff[wCmdLen++] = bFileOption;

    /* Copy Access rights */
    bCmdBuff[wCmdLen++] = pAccessRights[0];
    bCmdBuff[wCmdLen++] = pAccessRights[1];

    /* Copy File size supplied by the user */
    (void)memcpy(&bCmdBuff[wCmdLen], pFileSize, 3);
    wCmdLen += 3U;

    /* COMMUNICATION IS PLAIN */
    return phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        wCmdLen,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}

phStatus_t phalMfdfEVx_Sw_CreateValueFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bFileNo, uint8_t bCommSett,
    uint8_t * pAccessRights, uint8_t * pLowerLmit, uint8_t * pUpperLmit, uint8_t * pValue, uint8_t bLimitedCredit)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[32];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if ((bFileNo & 0x7fU) > 0x1fU)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    if (((bCommSett & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_PLAIN >> 4U)) &&
        ((bCommSett & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_MACD >> 4U)) &&
        ((bCommSett & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_ENC >> 4U)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CREATE_VALUE_FILE;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_CREATE_VALUE_FILE;
    bCmdBuff[wCmdLen++] = bFileNo;

    /* Copy communication settings */
    bCmdBuff[wCmdLen++] = bCommSett;

    (void)memcpy(bCmdBuff+wCmdLen, pAccessRights, 2);
    wCmdLen = wCmdLen + 2U;

    (void)memcpy(bCmdBuff+wCmdLen, pLowerLmit, 4);
    wCmdLen = wCmdLen + 4U;

    (void)memcpy(bCmdBuff+wCmdLen, pUpperLmit, 4);
    wCmdLen = wCmdLen + 4U;

    (void)memcpy(bCmdBuff+wCmdLen, pValue, 4);
    wCmdLen = wCmdLen + 4U;

    bCmdBuff[wCmdLen++] = bLimitedCredit;

    /* COMMUNICATION IS PLAIN */
    return phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        wCmdLen,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}

phStatus_t phalMfdfEVx_Sw_CreateLinearRecordFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t  bFileNo,
    uint8_t  *pIsoFileId, uint8_t bCommSett, uint8_t * pAccessRights, uint8_t * pRecordSize, uint8_t * pMaxNoOfRec)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[32];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if (((bFileNo & 0x7fU) > 0x1fU) || (bOption > 0x01U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if (((bCommSett & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_PLAIN >> 4U)) &&
        ((bCommSett & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_MACD >> 4U)) &&
        ((bCommSett & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_ENC >> 4U)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CREATE_LINEAR_RECFILE;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_CREATE_LINEAR_RECFILE;

    /* Copy the value information supplied by the user */
    bCmdBuff[wCmdLen++] = bFileNo;

    if (bOption == 0x01U)
    {
        bCmdBuff[wCmdLen++] = pIsoFileId[0];
        bCmdBuff[wCmdLen++] = pIsoFileId[1];
    }

    /* Copy communication settings */
    bCmdBuff[wCmdLen++] = bCommSett;

    bCmdBuff[wCmdLen++] = pAccessRights[0];
    bCmdBuff[wCmdLen++] = pAccessRights[1];

    bCmdBuff[wCmdLen++] = pRecordSize[0];
    bCmdBuff[wCmdLen++] = pRecordSize[1];
    bCmdBuff[wCmdLen++] = pRecordSize[2];

    bCmdBuff[wCmdLen++] = pMaxNoOfRec[0];
    bCmdBuff[wCmdLen++] = pMaxNoOfRec[1];
    bCmdBuff[wCmdLen++] = pMaxNoOfRec[2];

    /* COMMUNICATION IS PLAIN */
    return phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        wCmdLen,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}

phStatus_t phalMfdfEVx_Sw_CreateCyclicRecordFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t  bFileNo,
    uint8_t  *pIsoFileId, uint8_t bCommSett, uint8_t * pAccessRights, uint8_t * pRecordSize,
    uint8_t * pMaxNoOfRec)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[32];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if (((bFileNo & 0x7fU) > 0x1fU) || (bOption > 0x01U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if (((bCommSett & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_PLAIN >> 4U)) &&
        ((bCommSett & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_MACD >> 4U)) &&
        ((bCommSett & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_ENC >> 4U)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CREATE_CYCLIC_RECFILE;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_CREATE_CYCLIC_RECFILE;

    /* Copy the value information supplied by the user */
    bCmdBuff[wCmdLen++] = bFileNo;

    if (bOption == 0x01U)
    {
        bCmdBuff[wCmdLen++] = pIsoFileId[0];
        bCmdBuff[wCmdLen++] = pIsoFileId[1];
    }

    /* Copy communication settings */
    bCmdBuff[wCmdLen++] = bCommSett;

    bCmdBuff[wCmdLen++] = pAccessRights[0];
    bCmdBuff[wCmdLen++] = pAccessRights[1];

    bCmdBuff[wCmdLen++] = pRecordSize[0];
    bCmdBuff[wCmdLen++] = pRecordSize[1];
    bCmdBuff[wCmdLen++] = pRecordSize[2];

    bCmdBuff[wCmdLen++] = pMaxNoOfRec[0];
    bCmdBuff[wCmdLen++] = pMaxNoOfRec[1];
    bCmdBuff[wCmdLen++] = pMaxNoOfRec[2];

    /* COMMUNICATION IS PLAIN */
    return phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        wCmdLen,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}


phStatus_t phalMfdfEVx_Sw_DeleteFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bFileNo)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];

#ifdef RDR_LIB_PARAM_CHECK
    if ((bFileNo & 0x7fU) > 0x1fU)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_DELETE_FILE;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_DELETE_FILE;
    bCmdBuff[1] = bFileNo;

    /* COMMUNICATION IS PLAIN */
    return phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        2,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}

phStatus_t phalMfdfEVx_Sw_GetFileIDs(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pFid, uint8_t * bNumFID)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM * pRecv = NULL;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_GET_FILE_IDS;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_GET_FILE_IDS;

    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        bCmdBuff,
        1,
        &pRecv,
        &wRxlen
        ));

    (void)memcpy(pFid, pRecv, wRxlen);
    *bNumFID = (uint8_t)wRxlen;

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_GetISOFileIDs(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pFidBuffer, uint8_t * pNumFID)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM * pRecv = NULL;

    /* Initialization */
    *pNumFID = 0;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_GET_ISO_FILE_IDS;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_GET_ISO_FILE_IDS;

    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        bCmdBuff,
        1,
        &pRecv,
        &wRxlen
        ));

    /* Length should be multiple of 2 */
    if ((wRxlen != 0U) && (wRxlen % 2U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }
    (void)memcpy(pFidBuffer, pRecv, wRxlen);

    /* Update pNumAids and return  */
    *pNumFID = (uint8_t)(wRxlen / 2U);

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_GetFileSettings(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bFileNo, uint8_t * pFSBuffer,
    uint8_t * pBufferLen)
{
    uint16_t    PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_GET_FILE_SETTINGS;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_GET_FILE_SETTINGS;
    bCmdBuff[1] = bFileNo;

    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        bCmdBuff,
        2,
        &pRecv,
        &wRxlen
        ));

    /*
    * File type can be obtained by reading the zeroth index of the receive buffer
    * For Std data file, pRecv[0] = 0x00
    * For Backup data file, pRecv[0] = 0x01
    * For Value file, pRecv[0] = 0x02
    * For Linear Record file, pRecv[0] = 0x03
    * For Cyclic Record file, pRecv[0] = 0x04
    * For Transaction MAC file, pRecv[0] = 0x05
    * For TransactionMAC file wRxLen = 6 if TMCLimit is not available else 10 if TMCLimit is available.
    * For Standard file or Backup file mandatory 7 bytes and optional No. of AAR with AAR are received
    * For Value file mandatory 17 bytes and optional No. of AAR with AAR are received
    * For Linear/Cyclic Record file mandatory 13 bytes and optional No. of AAR with AAR are received
    */

    if ((pRecv[0] == 0x00U) || (pRecv[0] == 0x01U))
    {
        if ((pRecv[1] & PHAL_MFDFEVX_FILE_OPTION_SDM_MIRRORING_ENABLED) ==
            PHAL_MFDFEVX_FILE_OPTION_SDM_MIRRORING_ENABLED)
        {
            /* Do Nothing */
        }
        else
        {
            if ((wRxlen < 7U) || ((wRxlen > 7U) && (wRxlen != (8U + (pRecv[7] * 2U)))))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
            }
        }
    }
    else if (pRecv[0] == 0x02U)
    {
        if ((wRxlen < 17U) || ((wRxlen > 17U) && (wRxlen != (18U + (pRecv[17] * 2U)))))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }
    }
    else if ((pRecv[0] == 0x03U) || (pRecv[0] == 0x04U))
    {
        if ((wRxlen < 13U) || ((wRxlen > 13U) && (wRxlen != (14U + (pRecv[13] * 2U)))))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }
    }
    else if (pRecv[0] == 0x05U)
    {
        if (wRxlen < 0x06U)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }

    (void)memcpy(pFSBuffer, pRecv, wRxlen);

    /* Update pBufferLen and return  */
    *pBufferLen = (uint8_t)wRxlen;

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_GetFileCounters(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bFileNo,
    uint8_t * pFileCounters, uint8_t * pRxLen)
{
    uint16_t    PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[20];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_GET_FILE_COUNTERS;

    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_GET_FILE_COUNTERS;
    bCmdBuff[wCmdLen++] = bFileNo;

    if (bOption == PHAL_MFDFEVX_COMMUNICATION_PLAIN)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
            pDataParams,
            PHAL_MFDFEVX_COMMUNICATION_PLAIN,
            bCmdBuff,
            wCmdLen,
            &pRecv,
            &wRxlen
            ));
    }
    else
    {

#ifndef NXPBUILD__PHAL_MFDFEVX_NDA
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
#endif /* NXPBUILD__PHAL_MFDFEVX_NDA */
    }

    *pRxLen = (uint8_t)wRxlen;
    (void)memcpy(pFileCounters, pRecv, wRxlen);

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_ChangeFileSettings(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption,
    uint8_t bFileNo, uint8_t bFileOption, uint8_t * pAccessRights, uint8_t bAddInfoLen, uint8_t * pAddInfo)
{
    uint8_t     PH_MEMLOC_REM aCmdBuff[2];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;
    uint8_t     PH_MEMLOC_REM aDataBuff[50];
    uint8_t     PH_MEMLOC_REM bDataLen = 0;
    uint8_t     PH_MEMLOC_REM bAddARsLen = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if ((bFileNo & 0x3fU) > 0x1fU)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    if (((bFileOption & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_PLAIN >> 4U)) &&
        ((bFileOption & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_ENC >> 4U)) &&
        ((bFileOption & 0x03U) != (PHAL_MFDFEVX_COMMUNICATION_MACD >> 4U)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Frame the command. */
    aCmdBuff[bCmdLen++] = PHAL_MFDFEVX_CMD_CHANGE_FILE_SETTINGS;
    aCmdBuff[bCmdLen++] = bFileNo;

    /* Frame the data. */
    aDataBuff[bDataLen++] = bFileOption;
    aDataBuff[bDataLen++] = pAccessRights[0];
    aDataBuff[bDataLen++] = pAccessRights[1];

    /* Exchange the informatio as is to PICC. */
    if (0U != (bOption & PHAL_MFDFEVX_EXCHANGE_ADD_INFO_BUFFER_COMPLETE))
    {
        (void)memcpy(&aDataBuff[bDataLen], pAddInfo, bAddInfoLen);
        bDataLen += bAddInfoLen;
    }
    else
    {
        if (0U != (bFileOption & PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT))
        {
            /* Compute the Additional ACCESS Rights length. */
            bAddARsLen = (uint8_t) ((bFileOption & PHAL_MFDFEVX_FILE_OPTION_TMCLIMIT_PRESENT) ? (bAddInfoLen - 4) : bAddInfoLen);

            aDataBuff[bDataLen++] = bAddARsLen;
            (void)memcpy(&aDataBuff[bDataLen], pAddInfo, (bAddARsLen * 2U));
            bDataLen += (bAddARsLen * 2U);
        }

        /* TMCLimit buffer in command buffer if Bit5 of File Option is SET. */
        if (0U != (bFileOption & PHAL_MFDFEVX_FILE_OPTION_TMCLIMIT_PRESENT))
        {
            (void)memcpy(&aDataBuff[bDataLen], &pAddInfo[bAddARsLen], 4U);
            bDataLen += 4U;
        }
    }

    if(((bOption & 0x30U) == PHAL_MFDFEVX_COMMUNICATION_PLAIN) ||
        ((bOption & 0x30U) == PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        /* Update the payload len. */
        pDataParams->dwPayLoadLen = bDataLen;

        /* COMMUNICATION IS PLAIN */
        return phalMfdfEVx_Sw_Int_Write_Plain(pDataParams, PHAL_MFDFEVX_ISO_CHAINING_MODE, aCmdBuff, bCmdLen,
            PHAL_MFDFEVX_COMMUNICATION_PLAIN, aDataBuff, bDataLen );
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
}




/* MIFARE DESFire EVx Data mamangement commands. --------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_ReadData(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bIns,
    uint8_t bFileNo, uint8_t * pOffset, uint8_t * pLength, uint8_t ** ppRxdata, uint16_t * pRxdataLen)
{
    /* The signature of this is changed. We include
    * the bIns as third parameter that will differentiate
    * between application chaining and ISO chaining modes
    */
    phStatus_t  PH_MEMLOC_REM status = 0;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[16];
    uint8_t     PH_MEMLOC_REM bTmp = 0;
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    uint32_t    PH_MEMLOC_REM dwDataLen = 0;
    uint8_t     PH_MEMLOC_REM bShortLengthApduValue = 0;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = (uint8_t) (bIns ? PHAL_MFDFEVX_CMD_READ_DATA_ISO :
        PHAL_MFDFEVX_CMD_READ_DATA);

#ifdef RDR_LIB_PARAM_CHECK
    /* bit[1] of bIns will also be used. Hence bIns should be checked for above 0x03*/
    if (((bFileNo & 0x7fU) > 0x1fU) || (bIns > 0x03U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif
    if ((bOption & 0x0FU) == PH_EXCHANGE_RXCHAINING)
    {
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME;
    }
    else if ((bOption & 0x0FU) == PH_EXCHANGE_DEFAULT)
    {
        /* copy data length */
        dwDataLen = pLength[2];
        dwDataLen = (dwDataLen << 8U) | pLength[1];
        dwDataLen = (dwDataLen << 8U) | pLength[0];
        /* Set the format of data to be sent as short APDU when,
        * 1. bit[1] of bIns is set. This means user is force sending the data in short APDU format in case of BIGISO read.
        * 2. In case data to be read is not BIGISO(Less than 256 bytes).
        */
        if ((bIns & 0x02U) || ((dwDataLen <= 0xFFU) && (dwDataLen != 0U)))
        {
            /* Setting parameter 'bShortLenApdu' in EVx data Structure for Short Length APDUs */
            bShortLengthApduValue = 0x01;
            PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_SetConfig(pDataParams,
                PHAL_MFDFEVX_SHORT_LENGTH_APDU,
                bShortLengthApduValue));
            /* Reset Bit[1] of 'bIns' for subsequent operations */
            bIns &= 0xFDU;
        }
        if (bIns == 0x00U)
        {
            bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_READ_DATA;
        }
        else
        {
            bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_READ_DATA_ISO;
        }

        bCmdBuff[wCmdLen++] = bFileNo;
        (void)memcpy(&bCmdBuff[wCmdLen], pOffset, 3);
        wCmdLen += 3U;
        (void)memcpy(&bCmdBuff[wCmdLen], pLength, 3);
        wCmdLen += 3U;

        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
            PH_TMIUTILS_TMI_STATUS,
            &dwTMIStatus));

        /* Check TMI Collection Status */
        if (dwTMIStatus == PH_ON)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
                (uint8_t)((dwDataLen == 0U) ? (PH_TMIUTILS_READ_INS | PH_TMIUTILS_ZEROPAD_CMDBUFF) : PH_TMIUTILS_ZEROPAD_CMDBUFF),
                bCmdBuff,
                wCmdLen,
                NULL,
                0,
                PHAL_MFDFEVX_BLOCK_SIZE
                ));
        }
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    if ((bOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_ENC)
    {
    }
    else if (((bOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_MACD) ||
        ((bOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_PLAIN))
    {

        if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATE) && ((bOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_MACD))
        {
            bOption = bOption | PHAL_MFDFEVX_COMMUNICATION_MAC_ON_RC;
        }

        status = phalMfdfEVx_Sw_Int_ReadData_Plain(
            pDataParams,
            bOption | ((bIns == 0x00U) ? PHAL_MFDFEVX_DEFAULT_MODE : PHAL_MFDFEVX_ISO_CHAINING_MODE),
            bCmdBuff,
            wCmdLen,
            ppRxdata,
            pRxdataLen
            );
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    /* Restoring the Extended Length APDU mode */
    bShortLengthApduValue = 0x00;
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_SetConfig(pDataParams,
        PHAL_MFDFEVX_SHORT_LENGTH_APDU,
        bShortLengthApduValue));
    if ((status == PH_ERR_SUCCESS) && (bCmdBuff[0] != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME))
    {
        if ((dwDataLen != *pRxdataLen) && (dwDataLen != 0U))
        {
            /* Reset authentication status */
            if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
                (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES) ||
                (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2))
            {
                phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
            }
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }
    }
    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        if (dwDataLen == 0U)
        {
            bTmp = PH_TMIUTILS_READ_INS;
        }
        if (status == PH_ERR_SUCCESS)
        {
            bTmp |= PH_TMIUTILS_ZEROPAD_DATABUFF;
        }

        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            bTmp,
            NULL,
            0,
            *ppRxdata,
            *pRxdataLen,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));

        if ((status == PH_ERR_SUCCESS) && (dwDataLen == 0U))
        {
            /* Reset wOffsetInTMI to 0 */
            PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_SetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
                PH_TMIUTILS_TMI_OFFSET_LENGTH,
                0
                ));
        }
    }
    return status;
}

phStatus_t phalMfdfEVx_Sw_WriteData(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bIns,
    uint8_t bFileNo, uint8_t * pOffset, uint8_t * pData, uint8_t * pDataLen)
{
    /* The signature of this is changed. We include
    * the bIns as third parameter that will differentiate
    * between application chaining and ISO chaining modes
    */
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCommOptionTemp = bCommOption;
    uint8_t     PH_MEMLOC_REM bLoopData = 1;
    uint8_t     PH_MEMLOC_REM bCmdBuff[16];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint16_t    PH_MEMLOC_REM wDataLenTemp;
    uint32_t    PH_MEMLOC_REM dwDataLen;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    uint32_t    PH_MEMLOC_REM dwDataWritten = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if (((bFileNo & 0x7fU) > 0x1fU) || (bIns > 0x01U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if ((bCommOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_ENC) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = (uint8_t) (bIns ? PHAL_MFDFEVX_CMD_WRITE_DATA_ISO :
        PHAL_MFDFEVX_CMD_WRITE_DATA);

    /* form the command depending on bIns */
    if (bIns == 0x00U)
    {
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_WRITE_DATA;
    }
    else
    {
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_WRITE_DATA_ISO;
    }
    bCmdBuff[wCmdLen++] = bFileNo;
    (void)memcpy(&bCmdBuff[wCmdLen], pOffset, 3);
    wCmdLen += 3U;
    (void)memcpy(&bCmdBuff[wCmdLen], pDataLen, 3);
    wCmdLen += 3U;

    /* copy data length */
    dwDataLen = pDataLen[2];
    dwDataLen = (dwDataLen << 8U) | pDataLen[1];
    dwDataLen = (dwDataLen << 8U) | pDataLen[0];

    /* to handle 2 MB of data update maximum of data bytes that can be sent in a single loop */
    if (dwDataLen > PHAL_MFDFEVX_MAX_WRITE_SIZE)
    {
        wDataLenTemp = (uint16_t)PHAL_MFDFEVX_MAX_WRITE_SIZE;
        bLoopData = (uint8_t)(dwDataLen / PHAL_MFDFEVX_MAX_WRITE_SIZE);
        if(0U != (dwDataLen % PHAL_MFDFEVX_MAX_WRITE_SIZE))
        {
            bLoopData++;
        }
        bCommOptionTemp = PHAL_MFDFEVX_MAC_DATA_INCOMPLETE | bCommOption;
    }
    else
    {
        wDataLenTemp = (uint16_t)dwDataLen;
    }

    /* update the total number of data to be written
    * when INS and wrapped mode is enable, we need to send the total number of bytes with Le */
    pDataParams->dwPayLoadLen = dwDataLen;

    if (pDataParams->bAuthMode == PHAL_MFDFEVX_NOT_AUTHENTICATED)
    {
        bCommOption = PHAL_MFDFEVX_COMMUNICATION_PLAIN;
    }

    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            (PH_TMIUTILS_ZEROPAD_CMDBUFF | PH_TMIUTILS_ZEROPAD_DATABUFF),
            bCmdBuff,
            wCmdLen,
            pData,
            dwDataLen,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));
    }

    do
    {
        if (bLoopData == 1U)
        {
            bCommOptionTemp = bCommOption;
            wDataLenTemp = (uint16_t)(dwDataLen - dwDataWritten);
        }
        if ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_ENC)
        {
        }
        else
        {
            statusTmp = phalMfdfEVx_Sw_Int_Write_Plain(
                pDataParams,
                ((bIns == 0x00U) ? PHAL_MFDFEVX_DEFAULT_MODE : PHAL_MFDFEVX_ISO_CHAINING_MODE),
                bCmdBuff,
                wCmdLen,
                bCommOptionTemp,
                &pData[dwDataWritten],
                wDataLenTemp
                );
        }

        if ((statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING)
        {
            return statusTmp;
        }

        bLoopData--;
        dwDataWritten += wDataLenTemp;

        bCmdBuff[0] = 0xAF;
        wCmdLen = 1;

    } while (bLoopData > 0U);

    return statusTmp;

}

phStatus_t phalMfdfEVx_Sw_GetValue(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bFileNo,
    uint8_t * pValue)
{

    /* If not authenticated, send the data and get the value in plain.
    Else use the mode dictated by the caller of this API
    */
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if ((bFileNo & 0x7fU) > 0x1fU)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if ((bCommOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_ENC) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_GET_VALUE;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_GET_VALUE;
    bCmdBuff[1] = bFileNo;

    if ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_ENC)
    {
    }
    else
    {
        /* For EV2, bCommOption == MACD means the cmd+MAC is sent to card
        * and a MAC on response is received.
        * Hope this is taken care of in readdata_plain
        */

        if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATE) && ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_MACD))
        {
            bCommOption = bCommOption | PHAL_MFDFEVX_COMMUNICATION_MAC_ON_RC;
        }

        PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
            pDataParams,
            bCommOption,
            bCmdBuff,
            2,
            &pRecv,
            &wRxlen
            ));
    }

    if (wRxlen != 4U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }
    (void)memcpy(pValue, pRecv, wRxlen);

    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            PH_TMIUTILS_ZEROPAD_DATABUFF,
            bCmdBuff,
            2,
            pValue,
            4,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_Credit(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bFileNo,
    uint8_t * pValue)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    phStatus_t status = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if ((bFileNo & 0x7fU) > 0x1fU)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if ((bCommOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_ENC) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CREDIT;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_CREDIT;
    bCmdBuff[wCmdLen++] = bFileNo;

    (void)memcpy(&bCmdBuff[wCmdLen], pValue, 4);
    wCmdLen += 4U;

    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            PH_TMIUTILS_ZEROPAD_CMDBUFF,
            bCmdBuff,
            wCmdLen,
            NULL,
            0,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));
    }

    if ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_ENC)
    {
    }
    else
    {
        /* COMMUNICATION IS PLAIN */
        /* Need to differentiate between plain and MACD mode
        * for AuthEVX mode
        */
        status = phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
            PHAL_MFDFEVX_DEFAULT_MODE,
            bCmdBuff,
            0x0002,
            bCommOption,
            &bCmdBuff[2],
            0x0004
            );
    }
    return status;
}

phStatus_t phalMfdfEVx_Sw_Debit(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bFileNo,
    uint8_t * pValue)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    phStatus_t status = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if ((bFileNo & 0x7fU) > 0x1fU)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if ((bCommOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_ENC) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_DEBIT;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_DEBIT;
    bCmdBuff[wCmdLen++] = bFileNo;

    (void)memcpy(&bCmdBuff[wCmdLen], pValue, 4);
    wCmdLen += 4U;

    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            PH_TMIUTILS_ZEROPAD_CMDBUFF,
            bCmdBuff,
            wCmdLen,
            NULL,
            0,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));
    }

    if ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_ENC)
    {
    }
    else
    {
        /* COMMUNICATION IS PLAIN */
        status = phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
            PHAL_MFDFEVX_DEFAULT_MODE,
            bCmdBuff,
            0x0002,
            bCommOption,
            &bCmdBuff[2],
            0x0004
            );
    }
    return status;
}

phStatus_t phalMfdfEVx_Sw_LimitedCredit(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bFileNo,
    uint8_t * pValue)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    phStatus_t status = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if ((bFileNo & 0x7fU) > 0x1fU)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if ((bCommOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_ENC) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_LIMITED_CREDIT;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_LIMITED_CREDIT;
    bCmdBuff[wCmdLen++] = bFileNo;

    (void)memcpy(&bCmdBuff[wCmdLen], pValue, 4);
    wCmdLen += 4U;

    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            PH_TMIUTILS_ZEROPAD_CMDBUFF,
            bCmdBuff,
            wCmdLen,
            NULL,
            0,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));
    }

    if ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_ENC)
    {
    }
    else
    {
        /* COMMUNICATION IS PLAIN */
        status = phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
            PHAL_MFDFEVX_DEFAULT_MODE,
            bCmdBuff,
            0x0002,
            bCommOption,
            &bCmdBuff[2],
            0x0004
            );
    }
    return status;
}

phStatus_t phalMfdfEVx_Sw_WriteRecord(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bIns, uint8_t bFileNo, uint8_t * pOffset,
    uint8_t * pData, uint8_t * pDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint8_t     PH_MEMLOC_REM bCommOptionTemp = bCommOption;
    uint8_t     PH_MEMLOC_REM bLoopData = 1;
    uint16_t    PH_MEMLOC_REM wDataLenTemp;
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint32_t    PH_MEMLOC_REM dwDataLen;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    uint32_t    PH_MEMLOC_REM dwDataWritten = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if (((bFileNo & 0x7fU) > 0x1fU) || (bIns > 0x01U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if ((bCommOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_ENC) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = (uint8_t) (bIns ? PHAL_MFDFEVX_CMD_WRITE_RECORD_ISO :
        PHAL_MFDFEVX_CMD_WRITE_RECORD);

    /* form the command depending on bIns */
    if (bIns == 0x00U)
    {
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_WRITE_RECORD;
    }
    else
    {
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_WRITE_RECORD_ISO;
    }
    bCmdBuff[wCmdLen++] = bFileNo;
    (void)memcpy(&bCmdBuff[wCmdLen], pOffset, 3);
    wCmdLen += 3U;
    (void)memcpy(&bCmdBuff[wCmdLen], pDataLen, 3);
    wCmdLen += 3U;

    /* Assuming here that the size can never go beyond FFFF. */
    dwDataLen = pDataLen[2];
    dwDataLen = (dwDataLen << 8U) | pDataLen[1];
    dwDataLen = (dwDataLen << 8U) | pDataLen[0];

    /* to handle 2 MB of data update maximum of data bytes that can be sent in a single loop */
    if (dwDataLen > PHAL_MFDFEVX_MAX_WRITE_SIZE)
    {
        wDataLenTemp = (uint16_t)PHAL_MFDFEVX_MAX_WRITE_SIZE;
        bLoopData = (uint8_t)(dwDataLen / PHAL_MFDFEVX_MAX_WRITE_SIZE);
        if  (0U != (dwDataLen % PHAL_MFDFEVX_MAX_WRITE_SIZE))
        {
            bLoopData++;
        }
        bCommOptionTemp = PHAL_MFDFEVX_MAC_DATA_INCOMPLETE | bCommOption;
    }
    else
    {
        wDataLenTemp = (uint16_t)dwDataLen;
    }


    /* update the total number of data to be written
    * when INS and wrapped mode is enable, we need to send the total number of bytes with Le */
    pDataParams->dwPayLoadLen = dwDataLen;

    if (pDataParams->bAuthMode == PHAL_MFDFEVX_NOT_AUTHENTICATED)
    {
        bCommOption = PHAL_MFDFEVX_COMMUNICATION_PLAIN;
    }
    else
    {
    }

    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            (PH_TMIUTILS_ZEROPAD_CMDBUFF | PH_TMIUTILS_ZEROPAD_DATABUFF),
            bCmdBuff,
            wCmdLen,
            pData,
            dwDataLen,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));
    }

    do
    {
        if (bLoopData == 1U)
        {
            bCommOptionTemp = bCommOption;
            wDataLenTemp = (uint16_t)(dwDataLen - dwDataWritten);
        }

        if ((pDataParams->bAuthMode == PHAL_MFDFEVX_NOT_AUTHENTICATED) ||
            (bCommOption == PHAL_MFDFEVX_COMMUNICATION_MACD) ||
            (bCommOption == PHAL_MFDFEVX_COMMUNICATION_PLAIN))
        {
            statusTmp = phalMfdfEVx_Sw_Int_Write_Plain(
                pDataParams,
                ((bIns == 0x00U) ? PHAL_MFDFEVX_DEFAULT_MODE : PHAL_MFDFEVX_ISO_CHAINING_MODE),
                bCmdBuff,
                wCmdLen,
                bCommOptionTemp,
                &pData[dwDataWritten],
                wDataLenTemp
                );

        }

        if ((statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING)
        {
            return statusTmp;
        }

        bLoopData--;
        dwDataWritten += wDataLenTemp;

        bCmdBuff[0] = 0xAF;
        wCmdLen = 1;

    } while (bLoopData > 0U);

    return statusTmp;
}

phStatus_t phalMfdfEVx_Sw_ReadRecords(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bIns, uint8_t bFileNo, uint8_t * pRecNo,
    uint8_t * pRecCount, uint8_t * pRecSize, uint8_t ** ppRxdata, uint16_t * pRxdataLen)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint32_t    PH_MEMLOC_REM dwRecLen = 0;
    uint32_t    PH_MEMLOC_REM dwNumRec = 0;
    phStatus_t  PH_MEMLOC_REM status = 0;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bOption = 0;
    uint32_t    PH_MEMLOC_REM dwDataLen = 0;
    uint32_t    PH_MEMLOC_REM dwOffsetTMI = 0;
    uint32_t    PH_MEMLOC_REM dwTMIBufInd = 0;
    uint32_t    PH_MEMLOC_REM dwTotalRecLen = 0;
    uint32_t    PH_MEMLOC_REM dwNumRecCal = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if (((bFileNo & 0x7fU) > 0x1fU) || (bIns > 0x03U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif
    /* copy size of each record */
    dwRecLen = pRecSize[2];/* MSB */
    dwRecLen = (dwRecLen << 8U) | pRecSize[1];
    dwRecLen = (dwRecLen << 8U) | pRecSize[0];/* LSB */

    /* copy number of records to be read */
    dwNumRec = pRecCount[2];/* MSB */
    dwNumRec = (dwNumRec << 8U) | pRecCount[1];
    dwNumRec = (dwNumRec << 8U) | pRecCount[0];/* LSB */

    /* Set the format of data to be sent as short APDU when,
     * 1. bit[1] of bIns is set. This means user is forcing the data to be sent in short APDU format in case of BIGISO read.
     * 2. In case data to be read is not BIGISO(Less than 256 bytes).
     */
    if ((bIns & 0x02U) || (((dwNumRec * dwRecLen) <= 0xFFU) && (dwNumRec != 0U)))
    {
        /* Setting parameter 'bShortLenApdu' in EVx data Structure for Short Length APDUs */
        PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_SetConfig(pDataParams,
            PHAL_MFDFEVX_SHORT_LENGTH_APDU,
            0x0001));
        /* Reset Bit[1] of 'bIns' for subsequent operations */
        bIns &= 0xFDU;
    }
    if ((bCommOption & 0x0FU) ==  PH_EXCHANGE_RXCHAINING)
    {
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME;
    }
    else if ((bCommOption & 0x0FU) ==  PH_EXCHANGE_DEFAULT)
    {
        /* form the command depending upon bIns */
        if (bIns == 0x00U)
        {
            bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_READ_RECORDS;
        }
        else
        {
            bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_READ_RECORDS_ISO;
        }
        bCmdBuff[wCmdLen++] = bFileNo;

        /* Record No */
        (void)memcpy(&bCmdBuff[wCmdLen], pRecNo, 3);
        wCmdLen += 3U;

        /* Record Count */
        (void)memcpy(&bCmdBuff[wCmdLen], pRecCount, 3);
        wCmdLen += 3U;

        /* Total number of bytes to read */
        dwTotalRecLen = (uint32_t)dwRecLen * dwNumRec;

        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
            PH_TMIUTILS_TMI_STATUS,
            &dwTMIStatus));

        /* Check TMI Collection Status */
        if (dwTMIStatus == PH_ON)
        {
            /* Should should provide atleast wRecLen / wNumRec to update in TIM collection */
            if((0U == dwRecLen) && (0U == dwNumRec))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
            }
            PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
                (uint8_t)((dwNumRec == 0U) ? (PH_TMIUTILS_READ_INS | PH_TMIUTILS_ZEROPAD_CMDBUFF) : PH_TMIUTILS_ZEROPAD_CMDBUFF),
                bCmdBuff,
                wCmdLen,
                NULL,
                0,
                PHAL_MFDFEVX_BLOCK_SIZE
                ));
        }
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    if ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_ENC)
    {
    }
    else if (((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_PLAIN) ||
        ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_MACD))
    {

        if((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATE) && ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_MACD))
        {
            bCommOption = bCommOption | PHAL_MFDFEVX_COMMUNICATION_MAC_ON_RC;
        }

        status = phalMfdfEVx_Sw_Int_ReadData_Plain(
            pDataParams,
            bCommOption | ((bIns == 0x00U) ? PHAL_MFDFEVX_DEFAULT_MODE : PHAL_MFDFEVX_ISO_CHAINING_MODE),
            bCmdBuff,
            wCmdLen,
            ppRxdata,
            pRxdataLen
            );
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    /* Restoring the Extended Length APDU mode */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_SetConfig(pDataParams,
        PHAL_MFDFEVX_SHORT_LENGTH_APDU,
        0x0000));
    if (((status & PH_ERR_MASK) != PH_ERR_SUCCESS) &&
        ((status & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING))
    {
        return status;
    }

    if ((status == PH_ERR_SUCCESS) && (bCmdBuff[0] != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME))
    {
        /* Can check this case if user has not given num of records
        as 0x000000. If 0x000000, then all records are read */
        if ((dwTotalRecLen != *pRxdataLen) && (dwTotalRecLen != 0U))
        {
            /* Reset authentication status */
            if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
                (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES))
            {
                phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
            }
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }
    }

    /* if function called with PH_EXCHANGE_RXCHAINING */
    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        if ((dwNumRec == 0U) && (status == PH_ERR_SUCCESS))
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
                PH_TMIUTILS_TMI_OFFSET_LENGTH,
                &dwOffsetTMI
                ));
            PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
                PH_TMIUTILS_TMI_BUFFER_INDEX,
                &dwTMIBufInd
                ));

            /* calculate Rx length in case of chaining */
            dwDataLen = *pRxdataLen + dwTMIBufInd -(dwOffsetTMI + 11U);

            /* for Qmore compliance below code is added check is done before itself  */
            if(dwRecLen == 0U)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
            }

            /* if user update worng RecSize, we cant calculate recCnt */
            if(0U != (dwDataLen % dwRecLen))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
            }

            /* calculate number of records */
            dwNumRecCal = dwDataLen / dwRecLen ;

            /* update record count */
            PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_SetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
                PH_TMIUTILS_TMI_OFFSET_VALUE,
                dwNumRecCal
                ));

        }
        if (status == PH_ERR_SUCCESS)
        {
            bOption = PH_TMIUTILS_ZEROPAD_DATABUFF;
        }

        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            bOption,
            NULL,
            0,
            *ppRxdata,
            *pRxdataLen,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));

        if ((status == PH_ERR_SUCCESS) && (dwNumRec == 0U))
        {
            /* Reset wOffsetInTMI to 0 */
            PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_SetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
                PH_TMIUTILS_TMI_OFFSET_LENGTH,
                0
                ));
        }
    }
    return status;
}

phStatus_t phalMfdfEVx_Sw_UpdateRecord(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bIns, uint8_t bFileNo, uint8_t * pRecNo,
    uint8_t * pOffset, uint8_t * pData, uint8_t * pDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCommOptionTemp = bCommOption;
    uint8_t     PH_MEMLOC_REM bLoopData = 1;
    uint8_t     PH_MEMLOC_REM bCmdBuff[16];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint16_t    PH_MEMLOC_REM wDataLenTemp;
    uint32_t    PH_MEMLOC_REM dwDataLen;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    uint32_t    PH_MEMLOC_REM dwDataWritten = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if (((bFileNo & 0x7fU) > 0x1fU) || (bIns > 0x01U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if ((bCommOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_ENC) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = (uint8_t) (bIns ? PHAL_MFDFEVX_CMD_UPDATE_RECORD_ISO :
        PHAL_MFDFEVX_CMD_UPDATE_RECORD);

    /* form the command depending on bIns */
    if (bIns == 0x00U)
    {
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_UPDATE_RECORD;
    }
    else
    {
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_UPDATE_RECORD_ISO;
    }
    bCmdBuff[wCmdLen++] = bFileNo;
    (void)memcpy(&bCmdBuff[wCmdLen], pRecNo, 3);
    wCmdLen += 3U;
    (void)memcpy(&bCmdBuff[wCmdLen], pOffset, 3);
    wCmdLen += 3U;
    (void)memcpy(&bCmdBuff[wCmdLen], pDataLen, 3);
    wCmdLen += 3U;

    /* copy data length */
    dwDataLen = pDataLen[2];
    dwDataLen = (dwDataLen << 8U) | pDataLen[1];
    dwDataLen = (dwDataLen << 8U) | pDataLen[0];

    /* to handle 2 MB of data update maximum of data bytes that can be sent in a single loop */
    if (dwDataLen > PHAL_MFDFEVX_MAX_WRITE_SIZE)
    {
        wDataLenTemp = (uint16_t)PHAL_MFDFEVX_MAX_WRITE_SIZE;
        bLoopData = (uint8_t)(dwDataLen / PHAL_MFDFEVX_MAX_WRITE_SIZE);
        if  (0U != (dwDataLen % PHAL_MFDFEVX_MAX_WRITE_SIZE))
        {
            bLoopData++;
        }
        bCommOptionTemp = PHAL_MFDFEVX_MAC_DATA_INCOMPLETE | bCommOption;
    }
    else
    {
        wDataLenTemp = (uint16_t)dwDataLen;
    }

    /* update the total number of data to be written
    * when INS and wrapped mode is enable, we need to send the total number of bytes with Le */
    pDataParams->dwPayLoadLen = dwDataLen;

    if (pDataParams->bAuthMode == PHAL_MFDFEVX_NOT_AUTHENTICATED)
    {
        bCommOption = PHAL_MFDFEVX_COMMUNICATION_PLAIN;
    }

    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            (PH_TMIUTILS_ZEROPAD_CMDBUFF | PH_TMIUTILS_ZEROPAD_DATABUFF),
            bCmdBuff,
            wCmdLen,
            pData,
            dwDataLen,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));
    }

    do
    {
        if (bLoopData == 1U)
        {
            bCommOptionTemp = bCommOption;
            wDataLenTemp = (uint16_t)(dwDataLen - dwDataWritten);
        }

        if ((pDataParams->bAuthMode == PHAL_MFDFEVX_NOT_AUTHENTICATED) ||
            (bCommOption == PHAL_MFDFEVX_COMMUNICATION_MACD) ||
            (bCommOption == PHAL_MFDFEVX_COMMUNICATION_PLAIN))
        {
            statusTmp = phalMfdfEVx_Sw_Int_Write_Plain(
                pDataParams,
                ((bIns == 0x00U) ? PHAL_MFDFEVX_DEFAULT_MODE : PHAL_MFDFEVX_ISO_CHAINING_MODE),
                bCmdBuff,
                wCmdLen,
                bCommOptionTemp,
                &pData[dwDataWritten],
                wDataLenTemp
                );
        }
        /* If chaining send data remaining data */
        if ((statusTmp & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING)
        {
            return statusTmp;
        }

        bLoopData--;
        dwDataWritten += wDataLenTemp;

        bCmdBuff[0] = 0xAF;
        wCmdLen = 1;

    } while (bLoopData > 0U);

    return statusTmp;
}

phStatus_t phalMfdfEVx_Sw_ClearRecordFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bFileNo)
{
    uint8_t PH_MEMLOC_REM bCmdBuff[8];
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    phStatus_t  PH_MEMLOC_REM statusTmp;

#ifdef RDR_LIB_PARAM_CHECK
    if ((bFileNo & 0x7fU) > 0x1fU)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CLEAR_RECORD_FILE;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_CLEAR_RECORD_FILE;
    bCmdBuff[1] = bFileNo;

    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
            PH_TMIUTILS_ZEROPAD_CMDBUFF,
            bCmdBuff,
            2,
            NULL,
            0,
            PHAL_MFDFEVX_BLOCK_SIZE
            ));
    }

    return phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        2,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) ? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}




/* MIFARE DESFire EVx Transaction mamangement commands. -------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_CommitTransaction(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pTMC,
    uint8_t * pTMAC)
{
    uint16_t    PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint16_t    PH_MEMLOC_REM wRxlen = 0;

#ifdef RDR_LIB_PARAM_CHECK
    /* As per ref_arch 0.04 for Cmd.CommitTransaction: simplified to always
     * use CommMode.MAC, so communication mode of response does not depend
     * on File-Type.TransactionMAC anymore.
     */
    if ((bOption & 0x0FU) > 0x01U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_COMMIT_TXN;

    /* form the command */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_COMMIT_TXN;

    if (0U != (bOption & 0x0FU))
    {
        bCmdBuff[wCmdLen++] = (bOption & 0x0FU);
    }

    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));


    if (((bOption & 0x0FU) == 0x01U) && (wRxlen != 0x0CU))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }
    else
    {
        if(NULL != pRecv)
        {
            (void)memcpy(pTMC, pRecv, 4);
            (void)memcpy(pTMAC, &pRecv[4],8);
        }
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_AbortTransaction(phalMfdfEVx_Sw_DataParams_t * pDataParams)
{
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_ABORT_TXN;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_ABORT_TXN;

    /* COMMUNICATION IS PLAIN */
    return phalMfdfEVx_Sw_Int_Write_Plain(pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        bCmdBuff,
        1,
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)? PHAL_MFDFEVX_COMMUNICATION_MACD : PHAL_MFDFEVX_COMMUNICATION_PLAIN,
        NULL,
        0x0000
        );
}

phStatus_t phalMfdfEVx_Sw_CommitReaderID(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pTMRI, uint8_t * pEncTMRI)
{
    uint16_t    PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[24];
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;
    uint8_t     PH_MEMLOC_REM bOption = PHAL_MFDFEVX_COMMUNICATION_PLAIN;
    uint8_t     PH_MEMLOC_REM bEncTMRILocal[16];

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_COMMIT_READER_ID;

    /* form the command */
    bCmdBuff[0] = PHAL_MFDFEVX_CMD_COMMIT_READER_ID;
    (void)memcpy(&bCmdBuff[1], pTMRI, 16);

    /* For d40  PCD->PICC: cmd + cmdData + CMAC(4byte) (Kses, cmdData)
     *           PICC->PCD: RC

     * For Ev1 SM: PCD->PICC: cmd + cmdData ( CMAC (Kses, Cmd||cmdData) Mac is used as IV for next operation)
     *             PICC->PCD: RC + respData + CMAC (Kses, cmdData||RC)
     */
    if((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATE) ||
       (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
       (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES))
    {
        bOption = PHAL_MFDFEVX_COMMUNICATION_MAC_ON_CMD;
    }

    if(pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)
    {
        bOption = PHAL_MFDFEVX_COMMUNICATION_MACD;
    }

    statusTmp = phalMfdfEVx_Sw_Int_ReadData_Plain(
        pDataParams,
        bOption,
        bCmdBuff,
        17,
        &pRecv,
        &wRxlen
        );

    /* Force the buffer to NULL in case of failure. */
    if (statusTmp != PH_ERR_SUCCESS)
    {
        pEncTMRI = NULL;
        return statusTmp;
    }

    /*
     * If Not Authenticated, there should not be any response from PICC.
     * If Authenticated, PICC should response with 16 bytes of information.
     */
    if ((pDataParams->bAuthMode == PHAL_MFDFEVX_NOT_AUTHENTICATED) && (wRxlen != 0))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }
    else if (((bOption == PHAL_MFDFEVX_COMMUNICATION_MAC_ON_CMD) || (bOption == PHAL_MFDFEVX_COMMUNICATION_MACD)) && (wRxlen != 16))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }

    if (pDataParams->bAuthMode == PHAL_MFDFEVX_NOT_AUTHENTICATED)
    {
        pEncTMRI = NULL;
    }
    else
    {
        (void)memcpy(bEncTMRILocal, pRecv, wRxlen);
        (void)memcpy(pEncTMRI, pRecv, wRxlen);
    }

    /* Do a Set Config of ADDITIONAL_INFO to set  the length(wLength) of the recieved TMRI */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_SetConfig((phalMfdfEVx_Sw_DataParams_t *)pDataParams, PHAL_MFDFEVX_ADDITIONAL_INFO,
        wRxlen));


    PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS,
        &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        /*
        * If authenticated, Cmd.CommitReaderID shall update the Transaction MAC Input TMI as follows:
        * TMI = TMI || Cmd || TMRICur||EncTMRI||ZeroPadding
        */
        if (pDataParams->bAuthMode != PHAL_MFDFEVX_NOT_AUTHENTICATED)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
                PH_TMIUTILS_ZEROPAD_DATABUFF,
                bCmdBuff,
                17,
                pEncTMRI,
                wRxlen,
                PHAL_MFDFEVX_BLOCK_SIZE
                ));
            memcpy(pEncTMRI,bEncTMRILocal,16);
        }
        else
        {
            /* If Not authenticated, Cmd.CommitReaderID shall update the Transaction MAC Input TMI as follows:
            * TMI = TMI || Cmd || TMRICur||ZeroPadding
            */
            PH_CHECK_SUCCESS_FCT(statusTmp, phTMIUtils_CollectTMI((phTMIUtils_t *)pDataParams->pTMIDataParams,
                PH_TMIUTILS_ZEROPAD_CMDBUFF,
                bCmdBuff,
                17,
                NULL,
                0x00,
                PHAL_MFDFEVX_BLOCK_SIZE
                ));
            pEncTMRI = NULL;
        }
    }

    return PH_ERR_SUCCESS;
}




/* MIFARE DESFire EVx ISO7816-4 commands. ---------------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_IsoSelectFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bSelector,
    uint8_t * pFid, uint8_t * pDFname, uint8_t bDFnameLen, uint8_t  bExtendedLenApdu, uint8_t ** ppFCI, uint16_t * pwFCILen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bData[24];
    uint32_t     PH_MEMLOC_REM bLc = 0;
    uint32_t     PH_MEMLOC_REM bLe = 0;
    uint8_t     PH_MEMLOC_REM bFileId[3] = { '\0' };
    uint8_t     PH_MEMLOC_REM aPiccDfName[7] = { 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x00 };
    phStatus_t  PH_MEMLOC_REM status;
    uint16_t    wVal = 0;

#ifdef RDR_LIB_PARAM_CHECK
    if (bDFnameLen > 16U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    if ((bOption != 0x00U) && (bOption != 0x0CU))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif
    switch (bSelector)
    {
        case 0x00:  /* Select MF, DF or EF, by file identifier */
        case 0x01:  /* Select child DF */
        case 0x02:  /* Select EF under the current DF, by file identifier */
        case 0x03:  /* Select parent DF of the current DF */
        /* Selection by EF Id*/
        /* Send MSB first to card */
            bFileId[1] = bData[0] = pFid[1];
            bFileId[0] = bData[1] = pFid[0];
            bFileId[2] = 0x00;
            bLc = 2;
            break;

        case 0x04:  /* Select by DF name, see Cmd.ISOSelect for VC selection. */
        /* Selection by DF Name */
            (void)memcpy(bData, pDFname, bDFnameLen);
            bLc = bDFnameLen;
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    statusTmp = phalMfdfEVx_Int_Send7816Apdu(
        pDataParams,
        pDataParams->pPalMifareDataParams,
        (bOption == 0x0CU) ? 0x01U : 0x03U,             /* As per Table 40-P2 in ISO/IEC FDIS 7816-4 */
        PHAL_MFDFEVX_CMD_ISO7816_SELECT_FILE,
        bSelector,
        bOption,
        bLc,
        bExtendedLenApdu,
        bData,
        bLe,
        ppFCI,
        pwFCILen);

    if ((statusTmp & PH_ERR_MASK) == PHAL_MFDFEVX_ERR_DF_7816_GEN_ERROR)
    {
        status = phalMfdfEVx_GetConfig(pDataParams, PHAL_MFDFEVX_ADDITIONAL_INFO, &wVal);
    }

    if ((statusTmp == PH_ERR_SUCCESS) || (wVal == PHAL_MFDFEVX_ISO7816_ERR_LIMITED_FUNCTIONALITY_INS))
    {
        /* Reset Authentication should not be targeted for elementary file selection using file ID */
        if (bSelector != 0x02U)
        {
            /* Reset Authentication Status here */
            phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
        }
        /* ISO wrapped mode is on */
        pDataParams->bWrappedMode = 1;

        /* once the selection Success, update the File Id to master data structure if the selection is done through AID */
    if((bSelector ==  0x00U) || (bSelector == 0x01U) || (bSelector == 0x02U))
        {
        (void)memcpy(pDataParams->pAid, bFileId, sizeof(bFileId));
        }
    else if((bSelector ==  0x04U))
        {
            /* Update the file ID to all zeros if DF Name is of PICC. */
            if (memcmp(pDFname, aPiccDfName, 7) == 0)
            {
                bFileId[0] = 0x00;
                bFileId[1] = 0x00;
                bFileId[2] = 0x00;
            }
            else
            {
                bFileId[0] = 0xff;
                bFileId[1] = 0xff;
                bFileId[2] = 0xff;
            }

        (void)memcpy(pDataParams->pAid, bFileId, sizeof(bFileId));
        }
    }
    else
    {
        return statusTmp;
    }

    PH_UNUSED_VARIABLE(status);
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_IsoReadBinary(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wOption, uint8_t bOffset,
    uint8_t bSfid, uint32_t dwBytesToRead, uint8_t bExtendedLenApdu, uint8_t ** ppRxBuffer, uint32_t * pBytesRead)
{
    uint8_t     PH_MEMLOC_REM bP1 = 0;
    uint8_t     PH_MEMLOC_REM bP2 = 0;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wOffset;
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint8_t     PH_MEMLOC_REM *pLePtr = NULL;

    if ((wOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_DEFAULT)
    {
        if (0U != (bSfid & 0x80U))
        {
#ifdef RDR_LIB_PARAM_CHECK
            /* Short file id is supplied */
            if ((bSfid & 0x7FU) > 0x1FU)
            {
                /* Error condition */
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
            }
#endif
            bP1 = bSfid;
            bP2 = bOffset;
        }
        else
        {
            /* P1 and P2 code the offset */
            wOffset = bP1 = bSfid;
            wOffset <<= 8; /* Left shift */
            wOffset |= bOffset;
            bP2 = bOffset;
        }
        pLePtr = (uint8_t *)&dwBytesToRead;
        bCmdBuff[wCmdLen++] = 0x00; /* Class */
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_ISO7816_READ_BINARY; /* Ins */
        bCmdBuff[wCmdLen++] = bP1;
        bCmdBuff[wCmdLen++] = bP2;
        /* Check whether Length Le should be represented in Short APDU or extended length APDU */
        if (bExtendedLenApdu == 0x01U)
        {
            /*
            * An extended Le field consists of either three bytes (one * byte set to '00' followed by two bytes with any
            * value) if the Lc field is absent, or two bytes (with any * value) if an extended Lc field is present.
            * From '0001' to 'FFFF', the two bytes encode Ne from one
            * to 65 535.
            * If the two bytes are set to '0000', then Ne is 65 536.
            */
            bCmdBuff[wCmdLen++] = 0x00;
            bCmdBuff[wCmdLen++] = *(pLePtr + 1U);
            bCmdBuff[wCmdLen++] = *(pLePtr);
            /* Need to handle the case where the expected data to be read is more than 0xFFFF */
        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = *(pLePtr);
        }
    }
    else if ((wOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_RXCHAINING)
    {
        wCmdLen = 0;
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_ISO7816_READ_BINARY;

    return phalMfdfEVx_Sw_Int_IsoRead(
        pDataParams,
        wOption,
        bCmdBuff,
        wCmdLen,
        ppRxBuffer,
        pBytesRead
        );
}

phStatus_t phalMfdfEVx_Sw_IsoUpdateBinary(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOffset, uint8_t bSfid,
    uint8_t bExtendedLenApdu, uint8_t * pData, uint32_t dwDataLen)

{
    uint8_t     PH_MEMLOC_REM bP1 = 0;
    uint8_t     PH_MEMLOC_REM bP2 = 0;
    uint32_t    PH_MEMLOC_REM bLc = 0;
    uint16_t    PH_MEMLOC_REM wOffset;
    phStatus_t  PH_MEMLOC_REM status;

    if (0U != (bSfid & 0x80U))
    {
#ifdef RDR_LIB_PARAM_CHECK
        /* Short file id is supplied */
        if ((bSfid & 0x7FU) > 0x1FU)
        {
            /* Error condition */
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
        }
#endif
        bP1 = bSfid;
        bP2 = bOffset;
    }
    else
    {
        /* P1 and P2 code the offset */
        wOffset = bP1 = bSfid;
        wOffset <<= 8U; /* Left shift */
        wOffset |= bOffset;
        bP2 = bOffset;
    }

    bLc = dwDataLen;
    status = phalMfdfEVx_Int_Send7816Apdu(
        pDataParams,
        pDataParams->pPalMifareDataParams,
        0x01,
        PHAL_MFDFEVX_CMD_ISO7816_UPDATE_BINARY,
        bP1,
        bP2,
        bLc,
        bExtendedLenApdu,
        pData,
        0x00,
        NULL,
        NULL
        );

    if (status != PH_ERR_SUCCESS)
    {
        /* Reset authentication status */
        phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
    }

    return status;
}

phStatus_t phalMfdfEVx_Sw_IsoReadRecords(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wOption, uint8_t bRecNo,
    uint8_t bReadAllFromP1, uint8_t bSfid, uint32_t dwBytesToRead, uint8_t bExtendedLenApdu, uint8_t ** ppRxBuffer,
    uint32_t * pBytesRead)

{
    uint8_t     PH_MEMLOC_REM bP1 = 0;
    uint8_t     PH_MEMLOC_REM bP2 = 0;
    uint8_t     PH_MEMLOC_REM bCmdBuff[8];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint8_t     PH_MEMLOC_REM *pLePtr=NULL;

    if ((wOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_DEFAULT)
    {
        if (bSfid > 0x1FU)
        {
            /* Invalid Short File Id */
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
        }
        else
        {
            /* Valid Sfid */
            bP1 = bRecNo;
            bP2 = bSfid;
            bP2 <<= 3U; /* left shift by 3 bits to move SFID to bits 7 to 3 */
            if (0U != (bReadAllFromP1))
            {
                bP2 |= 0x05U; /* Last three bits of P2 = 101 */
            }
            else
            {
                bP2 |= 0x04U; /* Last three bits of P2 = 100. Read only record P1 */
            }
        }
        pLePtr = (uint8_t *) &dwBytesToRead;
        bCmdBuff[wCmdLen++] = 0x00; /* Class */
        bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_ISO7816_READ_RECORDS; /* Ins */
        bCmdBuff[wCmdLen++] = bP1;
        bCmdBuff[wCmdLen++] = bP2;

        /* Check whether Length Le should be represented in Short APDU or extended length APDU */
        if(bExtendedLenApdu == 0x01U)
        {
            /*
             * An extended Le field consists of either three bytes (one * byte set to '00' followed by two bytes with any
             * value) if the Lc field is absent, or two bytes (with any * value) if an extended Lc field is present.
             * From '0001' to 'FFFF', the two bytes encode Ne from one
             * to 65 535.
             * If the two bytes are set to '0000', then Ne is 65 536.
             */
             bCmdBuff[wCmdLen++] = 0x00;
             bCmdBuff[wCmdLen++] = *(pLePtr + 1U);
             bCmdBuff[wCmdLen++] = *(pLePtr);
             /* Need to handle the case where the expected data to be read is more than 0xFFFF */
        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = *(pLePtr);
        }
    }
    else if ((wOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_RXCHAINING)
    {
        wCmdLen = 0;
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_ISO7816_READ_RECORDS;

    return phalMfdfEVx_Sw_Int_IsoRead(
        pDataParams,
        wOption,
        bCmdBuff,
        wCmdLen,
        ppRxBuffer,
        pBytesRead
        );
}

phStatus_t phalMfdfEVx_Sw_IsoAppendRecord(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bSfid, uint8_t bExtendedLenApdu,
    uint8_t * pData, uint32_t dwDataLen)
{
    uint8_t     PH_MEMLOC_REM bP1 = 0;
    uint8_t     PH_MEMLOC_REM bP2 = 0;
    uint32_t    PH_MEMLOC_REM bLc = 0;
    phStatus_t  PH_MEMLOC_REM status;

#ifdef RDR_LIB_PARAM_CHECK
    if (bSfid > 0x1FU)
    {
        /* Invalid Short File Id */
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif
    bP2 = bSfid;
    bP2 <<= 3U; /* left shift by 3 bits to move SFID to bits 7 to 3 */
    /* Last three bits of P2 = 000 */

    bLc = dwDataLen;
    status = phalMfdfEVx_Int_Send7816Apdu(
        pDataParams,
        pDataParams->pPalMifareDataParams,
        0x01,
        PHAL_MFDFEVX_CMD_ISO7816_APPEND_RECORD,
        bP1,
        bP2,
        bLc,
        bExtendedLenApdu,
        pData,
        0x00,
        NULL,
        NULL
        );
    if (status != PH_ERR_SUCCESS)
    {
        /* Reset authentication status */
        phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
    }
    return status;
}

phStatus_t phalMfdfEVx_Sw_IsoUpdateRecord(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bIns, uint8_t bRecNo,
    uint8_t bSfid, uint8_t bRefCtrl, uint8_t * pData, uint8_t bDataLen)
{
    uint8_t     PH_MEMLOC_REM bP1 = 0;
    uint8_t     PH_MEMLOC_REM bP2 = 0;
    uint8_t     PH_MEMLOC_REM bLc = 0;
    phStatus_t  PH_MEMLOC_REM status;

#ifdef RDR_LIB_PARAM_CHECK
    if (bIns != 0xDCU)
    {
        if(bIns != 0xDDU)
        {
            /* Invalid Instruction */
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
        }
    }

    if (bSfid > 0x1FU)
    {
        /* Invalid Short File Id */
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif
    bP1 = bRecNo;
    bP2 = bSfid;
    bP2 <<= 3U; /* left shift by 3 bits to move SFID to bits 7 to 3 */
    bP2 |= bRefCtrl;

    bLc = bDataLen;
    status = phalMfdfEVx_Int_Send7816Apdu(
        pDataParams,
        pDataParams->pPalMifareDataParams,
        0x01,
        bIns,
        bP1,
        bP2,
        bLc,
        0x00,
        pData,
        0x00,
        NULL,
        NULL
        );
    if (status != PH_ERR_SUCCESS)
    {
        /* Reset authentication status */
        phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
    }
    return status;
}

phStatus_t phalMfdfEVx_Sw_IsoGetChallenge(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVer,
    uint8_t bExtendedLenApdu, uint32_t dwLe, uint8_t * pRPICC1)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint8_t     PH_MEMLOC_REM bData[24];
    uint16_t    PH_MEMLOC_REM wKeyType;

    PH_CHECK_SUCCESS_FCT(statusTmp, phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVer,
        sizeof(bData),
        bData,
        &wKeyType));

#ifdef RDR_LIB_PARAM_CHECK
    if ((wKeyType == PH_KEYSTORE_KEY_TYPE_AES128) || (wKeyType == PH_KEYSTORE_KEY_TYPE_3K3DES))
    {
        if (dwLe != 0x10U)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
        }
    }
    else if ((wKeyType == PH_KEYSTORE_KEY_TYPE_DES) || (wKeyType == PH_KEYSTORE_KEY_TYPE_2K3DES))
    {
        if (dwLe != 0x08U)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
        }
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
#endif
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Int_Send7816Apdu(
        pDataParams,
        pDataParams->pPalMifareDataParams,
        0x02,
        PHAL_MFDFEVX_CMD_ISO7816_GET_CHALLENGE,
        0x00,
        0x00,
        0x00,
        bExtendedLenApdu,
        NULL,
        dwLe,
        &pRecv,
        &wRxlen
        ));

    if (wRxlen != dwLe)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
    }

    (void)memcpy(pRPICC1, pRecv, wRxlen);

    /* Reset authentication status */
    phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);

    return PH_ERR_SUCCESS;
}





/* MIFARE DESFire EVx Originality Check functions. ------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_ReadSign(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bAddr, uint8_t ** pSignature)
{

    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdBuff[2];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_READ_SIG;

    /* build command frame */
    bCmdBuff[wCmdLen++] = PHAL_MFDFEVX_CMD_READ_SIG;
    bCmdBuff[wCmdLen++] = bAddr;

    /* Req spec(ver 0.14 says),
    * 1. Cmd.Read_Sig shall return the NXPOriginalitySignature as written during wafer test in plain if not authenticated
    * 2. Cmd.Read_Sig shall require MACed command if authenticated.
    */
    if (pDataParams->bAuthMode == PHAL_MFDFEVX_NOT_AUTHENTICATED)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_ReadData_Plain(
            pDataParams,
            PHAL_MFDFEVX_COMMUNICATION_PLAIN,
            bCmdBuff,
            wCmdLen,
            pSignature,
            &wRxLength
            ));
    }
    /* check received length :- 56 byte signature */
    if (wRxLength != PHAL_MFDFEVX_SIG_LENGTH)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_LENGTH_ERROR, PH_COMP_AL_MFDFEVX);
    }

    return PH_ERR_SUCCESS;

}




/* MIFARE DESFire EVx MIFARE Classic contactless IC functions. ---------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_CreateMFCMapping(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bComOption, uint8_t bFileNo,
    uint8_t bFileOption, uint8_t * pMFCBlockList, uint8_t bMFCBlocksLen, uint8_t bRestoreSource, uint8_t * pMFCLicense,
    uint8_t bMFCLicenseLen, uint8_t * pMFCLicenseMAC)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aCmdBuff[210];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;

    /* Validate the parameters. */
    if((bComOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN) &&
       (bComOption != PHAL_MFDFEVX_COMMUNICATION_ENC))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_CREATE_MFC_MAPPING;

    /* Frame the command information. */
    aCmdBuff[bCmdLen++] = PHAL_MFDFEVX_CMD_CREATE_MFC_MAPPING;
    aCmdBuff[bCmdLen++] = bFileNo;
    aCmdBuff[bCmdLen++] = bFileOption;
    aCmdBuff[bCmdLen++] = bMFCBlocksLen;

    /* Copy the MFCBlockList to command buffer. */
    (void)memcpy(&aCmdBuff[bCmdLen], pMFCBlockList, bMFCBlocksLen);
    bCmdLen += bMFCBlocksLen;

    /* Copy RestoreSource to command buffer. */
    if (0U != (bFileOption & 0x04U))
    {
        aCmdBuff[bCmdLen++] = bRestoreSource;
    }

    /* Copy the MFCLicense to command buffer. */
    (void)memcpy(&aCmdBuff[bCmdLen], pMFCLicense, bMFCLicenseLen);
    bCmdLen += bMFCLicenseLen;

    /* Exchange Cmd.CreateMFCMapping information to PICC. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfdfEVx_Sw_Int_Write_New(
        pDataParams,
        bComOption,
        aCmdBuff,
        bCmdLen,
        pMFCLicenseMAC,
        8));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_RestoreTransfer(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption,uint8_t bTargetFileNo, uint8_t bSourceFileNo)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aCmdBuff[15];
    uint32_t    PH_MEMLOC_REM dwTMIStatus = 0;

    if ((bCommOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_PLAIN_1) &&
        (bCommOption != PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_MFDFEVX);
    }

    /* Set the dataparams with command code. */
    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_RESTORE_TRANSFER;

    /* Frame the command information. */
    aCmdBuff[0] = PHAL_MFDFEVX_CMD_RESTORE_TRANSFER;
    aCmdBuff[1] = bTargetFileNo;
    aCmdBuff[2] = bSourceFileNo;

    /* Exchange Cmd.RestoreTransfer information to PICC. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfdfEVx_Sw_Int_Write_Plain(
        pDataParams,
        PHAL_MFDFEVX_DEFAULT_MODE,
        aCmdBuff,
        1,
        bCommOption,
        &aCmdBuff[1],
        2));

    /* Get the status of the TMI */
    PH_CHECK_SUCCESS_FCT(wStatus, phTMIUtils_GetConfig((phTMIUtils_t *)pDataParams->pTMIDataParams,
        PH_TMIUTILS_TMI_STATUS, &dwTMIStatus));

    /* Check TMI Collection Status */
    if (dwTMIStatus == PH_ON)
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phTMIUtils_CollectTMI((phTMIUtils_t *) pDataParams->pTMIDataParams,
            PH_TMIUTILS_ZEROPAD_CMDBUFF, aCmdBuff, 3, NULL, 0, PHAL_MFDFEVX_BLOCK_SIZE));
    }

    return PH_ERR_SUCCESS;
}





/* MIFARE DESFire EVx Miscellaneous functions. ----------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_GetConfig(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t * pValue)
{
    switch (wConfig)
    {
        case PHAL_MFDFEVX_ADDITIONAL_INFO:
            *pValue = pDataParams->wAdditionalInfo;
            break;

        case PHAL_MFDFEVX_WRAPPED_MODE:
            *pValue = (uint16_t) pDataParams->bWrappedMode;
            break;

        case PHAL_MFDFEVX_SHORT_LENGTH_APDU:
            *pValue = (uint16_t) pDataParams->bShortLenApdu;
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_SetConfig(phalMfdfEVx_Sw_DataParams_t *pDataParams, uint16_t wConfig, uint16_t wValue)
{
    switch (wConfig)
    {
        case PHAL_MFDFEVX_ADDITIONAL_INFO:
            pDataParams->wAdditionalInfo = wValue;
            break;

        case PHAL_MFDFEVX_WRAPPED_MODE:
            pDataParams->bWrappedMode = (uint8_t) wValue;
            break;

        case PHAL_MFDFEVX_SHORT_LENGTH_APDU:
            pDataParams->bShortLenApdu = (uint8_t) wValue;
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_AL_MFDFEVX);
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_ResetAuthentication(phalMfdfEVx_Sw_DataParams_t * pDataParams)
{
    phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);

    return PH_ERR_SUCCESS;
}


phStatus_t phalMfdfEVx_Sw_SetVCAParams(phalMfdfEVx_Sw_DataParams_t * pDataParams, void * pAlVCADataParams)
{
    PH_ASSERT_NULL(pDataParams);
    PH_ASSERT_NULL(pAlVCADataParams);

    pDataParams->pVCADataParams = pAlVCADataParams;

    return PH_ERR_SUCCESS;
}

#endif /* NXPBUILD__PHAL_MFDFEVX_SW */
