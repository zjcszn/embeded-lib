/*----------------------------------------------------------------------------*/
/* Copyright 2013-2023 NXP                                                    */
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
* Internal functions for Type 4 Tag Operation component of Reader Library
* Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <ph_TypeDefs.h>
#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phNxpNfcRdLib_Config.h>

#include <phalTop.h>

#ifdef NXPBUILD__PHAL_TOP_SW
#ifdef NXPBUILD__PHAL_TOP_T4T_SW

#include "phalTop_Sw_Int_T4T.h"

/*
 * ISO7816-4 commands
 * */
static phStatus_t phalIso7816_Int_ComputeErrorResponse(
        phalTop_T4T_t * pDataParams,
        uint16_t wStatusIn
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;

    switch(wStatusIn)
    {
    case PHAL_ISO7816_RESP_OPERATION_OK:
    case PHAL_ISO7816_SUCCESS:
        wStatus = PH_ERR_SUCCESS;
        break;

    case PHAL_ISO7816_RESP_COMMAND_ABORTED:
        wStatus = PHAL_ISO7816_ERR_COMMAND_ABORTED;
        break;

    case PHAL_ISO7816_RESP_OUT_OF_EEPROM_ERROR:
        wStatus = PHAL_ISO7816_ERR_OUT_OF_EEPROM;
        break;

    case PHAL_ISO7816_RESP_INTEGRITY_ERROR:
        wStatus = PHAL_ISO7816_ERR_PICC_CRYPTO;
        break;

    case PHAL_ISO7816_RESP_PARAMETER_ERROR:
        wStatus = PHAL_ISO7816_ERR_PARAMETER_ERROR;
        break;

    case PHAL_ISO7816_RESP_PERMISSION_DENIED:
        wStatus = PHAL_ISO7816_ERR_PERMISSION_DENIED;
        break;

    case PHAL_ISO7816_RESP_DUPLICATE:
        wStatus = PHAL_ISO7816_ERR_DUPLICATE;
        break;

    case PHAL_ISO7816_RESP_MEMORY_ERROR:
    case PHAL_ISO7816_RESP_ILLEGAL_COMMAND_CODE:
        wStatus = PHAL_ISO7816_ERR_DF_GEN_ERROR;
        pDataParams->wAdditionalInfo = wStatusIn;
        break;

    case PHAL_ISO7816_ERR_WRONG_LENGTH:
    case PHAL_ISO7816_ERR_WRONG_LE:
    case PHAL_ISO7816_ERR_FILE_NOT_FOUND:
    case PHAL_ISO7816_ERR_WRONG_PARAMS:
    case PHAL_ISO7816_ERR_WRONG_LC:
    case PHAL_ISO7816_ERR_NO_PRECISE_DIAGNOSTICS:
    case PHAL_ISO7816_ERR_EOF_REACHED:
    case PHAL_ISO7816_ERR_FILE_ACCESS:
    case PHAL_ISO7816_ERR_FILE_EMPTY:
    case PHAL_ISO7816_ERR_MEMORY_FAILURE:
    case PHAL_ISO7816_ERR_INCORRECT_PARAMS:
    case PHAL_ISO7816_ERR_WRONG_CLA:
    case PHAL_ISO7816_ERR_UNSUPPORTED_INS:
    case PHAL_ISO7816_ERR_LIMITED_FUNCTIONALITY_INS:
        wStatus = PHAL_ISO7816_ERR_DF_7816_GEN_ERROR;
        pDataParams->wAdditionalInfo = wStatusIn;
        break;

    default:
        wStatus = PH_ERR_PROTOCOL_ERROR;
        break;
    }
    return PH_ADD_COMPCODE(wStatus, PH_COMP_AL_TOP);
}

static phStatus_t phalIso7816_Sw_Int_ValidateResponse(
        phalTop_T4T_t * pDataParams,
        uint8_t bOption,
        uint16_t wStatus,
        uint16_t wPiccRetCode
        )
{
    uint16_t    PH_MEMLOC_REM wPICCStatusCode = 0;

    /* Evaluate the response. */
    if(wStatus == PH_ERR_SUCCESS)
    {
        /* Frame PICC Status Code. */
        wPICCStatusCode = (uint16_t) ((bOption == PHAL_ISO7816_STANDARD_CMD) ? wPiccRetCode : (wPiccRetCode & 0x00FF));

        /* Validate the PICC Status. */
        PH_CHECK_SUCCESS_FCT(wStatus, phalIso7816_Int_ComputeErrorResponse(pDataParams, wPICCStatusCode));
    }
    else
    {
        if((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING)
        {
            wStatus = PH_ADD_COMPCODE(PH_ERR_SUCCESS_CHAINING, PH_COMP_AL_TOP);
        }

        PH_CHECK_SUCCESS(wStatus);
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

static phStatus_t phalIso7816_Sw_Int_ISOSelectFile(
        phalTop_T4T_t * pDataParams,
        uint8_t * pData,
        uint16_t wDataLen,
        uint8_t * pLe,
        uint16_t wLe_Len,
        uint8_t ** ppResponse,
        uint16_t * pRspLen
        )
{
    phStatus_t  PH_MEMLOC_REM wStatusTmp = PH_ERR_SUCCESS;
    uint8_t     PH_MEMLOC_REM *pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRspLen = 0U;

    /* Buffer Command Data. */
    PH_CHECK_SUCCESS_FCT(wStatusTmp, phpalMifare_ExchangeL4(
        pDataParams->pAlT4TDataParams,
        PH_EXCHANGE_BUFFER_CONT,
        pData,
        wDataLen,
        &pResponse,
        &wRspLen));

    /* Buffer LE and Exchange the command to PICC.  */
    PH_CHECK_SUCCESS_FCT(wStatusTmp, phpalMifare_ExchangeL4(
        pDataParams->pAlT4TDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        pLe,
        wLe_Len,
        &pResponse,
        &wRspLen));

    /* Combine Sw1 and Sw2 status codes. */
    wStatusTmp = (uint16_t)((pResponse[wRspLen - 2U] << 8U) | pResponse[wRspLen - 1U]);

    /* Evaluate the Status. */
    wStatusTmp = phalIso7816_Int_ComputeErrorResponse(pDataParams, wStatusTmp);

    /*  Check for Success, FCI would be returned */
    if(wStatusTmp == PH_ERR_SUCCESS)
    {
        if(pRspLen != NULL)
        {
            *pRspLen = wRspLen - 2U;
        }
        if(ppResponse != NULL)
        {
            *ppResponse = pResponse;
        }
    }
    else
    {
        return wStatusTmp;
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

static phStatus_t phalIso7816_Sw_Int_ISOReadBinary(
        phalTop_T4T_t * pDataParams,
        uint16_t wBuffOption,
        uint8_t * pLe,
        uint16_t wLe_Len,
        uint8_t ** ppResponse,
        uint16_t * pRspLen
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM wStatus_Rsp = PH_ERR_SUCCESS;
    uint8_t     PH_MEMLOC_REM *pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRspLen = 0U;
    uint16_t    PH_MEMLOC_REM wOptions_Tmp = 0U;

    if((wBuffOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_RXCHAINING)
    {
        wLe_Len = 0U;
        wOptions_Tmp = PH_EXCHANGE_RXCHAINING;
    }
    else
    {
        wOptions_Tmp = PH_EXCHANGE_BUFFER_LAST;
    }

    /* Buffer LE and Exchange the command to PICC.  */
    wStatus = phpalMifare_ExchangeL4(
        pDataParams->pAlT4TDataParams,
        wOptions_Tmp,
        pLe,
        wLe_Len,
        &pResponse,
        &wRspLen);

    if(wRspLen >= 0x02U)
    {
        /* Combine Sw1 and Sw2 status codes. */
        wStatus_Rsp = (uint16_t)((pResponse[wRspLen - 2U] << 8U) | pResponse[wRspLen - 1U]);
    }

    /* Evaluate the Status. */
    wStatus = phalIso7816_Sw_Int_ValidateResponse(pDataParams, PHAL_ISO7816_STANDARD_CMD, wStatus, wStatus_Rsp);

    /* Reset Authentication state. */
    if(((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS) || ((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING))
    {
        /* Copy the response to parameter. */
        *ppResponse = pResponse;
        *pRspLen = wRspLen;

        /* Decrement Status code. */
        if((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
        {
            *pRspLen = *pRspLen - 2U;
        }
    }

    return wStatus;
}

static phStatus_t phalIso7816_Sw_Int_ISOReadBinary_ODO(
        phalTop_T4T_t * pDataParams,
        uint8_t bOption,
        uint16_t wBuffOption,
        uint8_t * pData,
        uint16_t wDataLen,
        uint8_t * pLe,
        uint16_t wLe_Len,
        uint8_t ** ppResponse,
        uint16_t * pRspLen
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM wStatus_Rsp = PH_ERR_SUCCESS;
    uint8_t     PH_MEMLOC_REM *pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRspLen = 0U;
    uint16_t    PH_MEMLOC_REM wOptions_Tmp = 0U;
    uint8_t     PH_MEMLOC_REM bRespLenOffset = 0U;

    /* Buffer Command Data. */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
        pDataParams->pAlT4TDataParams,
        PH_EXCHANGE_BUFFER_CONT,
        pData,
        wDataLen,
        &pResponse,
        &wRspLen));

    if((wBuffOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_RXCHAINING)
    {
        wLe_Len = 0U;
        wOptions_Tmp = PH_EXCHANGE_RXCHAINING;
    }
    else
    {
        wOptions_Tmp = PH_EXCHANGE_BUFFER_LAST;
    }

    /* Buffer LE and Exchange the command to PICC.  */
    wStatus = phpalMifare_ExchangeL4(
        pDataParams->pAlT4TDataParams,
        wOptions_Tmp,
        pLe,
        wLe_Len,
        &pResponse,
        &wRspLen);

    if(wRspLen >= 0x02U)
    {
        /* Combine Sw1 and Sw2 status codes. */
        wStatus_Rsp = (uint16_t)((pResponse[wRspLen - 2U] << 8U) | pResponse[wRspLen - 1U]);
    }

    /* Evaluate the Status. */
    wStatus = phalIso7816_Sw_Int_ValidateResponse(pDataParams, PHAL_ISO7816_STANDARD_CMD, wStatus, wStatus_Rsp);

    /* Reset Authentication state. */
    if(((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS) || ((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING))
    {
        /* Removing ODO format */
        if(bOption & 0xF0U)
        {
            /* Removal Static no, Ld and actual received length */
            if(pResponse[1U] <= PHAL_ISO7816_BER_TLV_L_00_7F)
            {
                pResponse = pResponse + 2U;
                wRspLen = wRspLen - 2U;
            }
            else
            {
                if((pResponse[1U] == PHAL_ISO7816_BER_TLV_L_81) || (pResponse[1U] == PHAL_ISO7816_BER_TLV_L_82))
                {
                    bRespLenOffset = (uint8_t)(pResponse[1U] & 0x03) + 1U /* Ld */;
                }

                pResponse = pResponse + (1U /* ODO Static no */ + bRespLenOffset /* Actual Length */);
                wRspLen = wRspLen - (1U /* ODO Static no */ + bRespLenOffset /* Actual Length */);
            }
        }

        /* Copy the response to parameter. */
        *ppResponse = pResponse;
        *pRspLen = wRspLen;

        /* Decrement Status code. */
        if((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
        {
            *pRspLen = *pRspLen - 2U;
        }
    }

    return wStatus;
}

static phStatus_t phalIso7816_Sw_Int_ISOUpdateBinary(
        phalTop_T4T_t * pDataParams,
        uint8_t * pData,
        uint16_t wDataLen
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM wStatus_Rsp = PH_ERR_SUCCESS;
    uint8_t     PH_MEMLOC_REM *pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRspLen = 0U;

    /* Buffer LE and Exchange the command to PICC.  */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
        pDataParams->pAlT4TDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        pData,
        wDataLen,
        &pResponse,
        &wRspLen));

    /* Combine Sw1 and Sw2 status codes. */
    wStatus_Rsp = (uint16_t)((pResponse[wRspLen - 2U] << 8U) | pResponse[wRspLen - 1U]);

    /* Evaluate the Status. */
    wStatus = phalIso7816_Int_ComputeErrorResponse(pDataParams, wStatus_Rsp);

    return wStatus;
}

static phStatus_t phalIso7816_Sw_Int_SendApdu(
        phalTop_T4T_t * pDataParams,
        uint8_t bOption,
        uint16_t wBufOption,
        uint8_t bExtendedLenApdu,
        uint8_t bIns,
        uint8_t bP1,
        uint8_t bP2,
        uint8_t * pData,
        uint16_t wDataLen,
        uint32_t dwExpBytes,
        uint8_t ** ppResponse,
        uint16_t * pRspLen
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;
    uint8_t     PH_MEMLOC_REM aLe[3] = { 0x00U, 0x00U, 0x00U };
    uint8_t     PH_MEMLOC_REM bCmdBuff[20U] = { 0U };
    uint8_t     PH_MEMLOC_REM bCmdLen = 0U;
    uint8_t     PH_MEMLOC_REM *pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wCompleteDataLen = 0U;
    uint16_t    PH_MEMLOC_REM wRspLen = 0U;
    uint16_t    PH_MEMLOC_REM wLe_Len = 0U;

    /* Set the command code to DataParams. */
    pDataParams->bCmdCode =  bIns;

    /* Frame Standard ISO7816 - 4 Header. */
    bCmdBuff[bCmdLen++] = PHAL_ISO7816_GENERIC_CLA;
    bCmdBuff[bCmdLen++] = bIns;
    bCmdBuff[bCmdLen++] = bP1;
    bCmdBuff[bCmdLen++] = bP2;

    /* Check whether Lc needs to be exchanged. */
    if(0U != (bOption & PHAL_ISO7816_EXCHANGE_LC_ONLY))
    {
        /* Check whether Length Lc is represented in short APDU or extended APDU */
        if(bExtendedLenApdu == PH_ON)
        {
            /* First byte will be 0x00 if Ext apdu present. Next 2 byte contains actual data. */
            bCmdBuff[bCmdLen++] = 0x00U;

            /* As of now this field will be set to 0x00 since maximum data that can be sent is 16 bytes.
             * In case if data to be sent exceeds 255 bytes, this byte shall be used.
             */
            bCmdBuff[bCmdLen++] = (uint8_t)((wDataLen & 0xFF00U) >> 8U);
        }

        /* Short Length APDU. */
        bCmdBuff[bCmdLen++] = (uint8_t) (wDataLen & 0x00FFU);

        if(0U != (bOption & (PH_EXCHANGE_ODO_DDO >> 12U)))
        {
            wCompleteDataLen = wDataLen + 5U /* 0x54 + 0x03 + 3 byte Offset */
                    + (((wDataLen & 0xFF00U) != 0) ? 4U : 3U) /* 0x53 + 0x81/0x82 + Len(1 or 2bytes) */;

            if((wDataLen) <= PHAL_ISO7816_BER_TLV_L_00_7F)
            {
                wCompleteDataLen = wCompleteDataLen - 1U; /* 1st byte itself length byte i.e 0x53 + Len(1byte) */
            }

            bCmdBuff[bCmdLen - 2U] = (uint8_t)((wCompleteDataLen & 0xFF00U) >> 8U);
            bCmdBuff[bCmdLen - 1U] = (uint8_t)(wCompleteDataLen & 0x00FFU);
        }
    }

    /* Buffer ISO7816-4 Command Header. */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
        pDataParams->pAlT4TDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCmdBuff,
        bCmdLen,
        &pResponse,
        &wRspLen));

    if(0U != (bOption & (PH_EXCHANGE_ODO_DDO >> 12U)))
    {
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pDataParams->pAlT4TDataParams,
            PH_EXCHANGE_BUFFER_CONT,
            *ppResponse,
            *pRspLen,
            &pResponse,
            &wRspLen));

        bCmdLen = 0;
        bCmdBuff[bCmdLen++] = 0x53U;
        if(wDataLen & 0xFF00U)
        {
            bCmdBuff[bCmdLen++] = PHAL_ISO7816_BER_TLV_L_82;
            bCmdBuff[bCmdLen++] = (uint8_t)(wDataLen >> 8U);
            bCmdBuff[bCmdLen++] = (uint8_t)(wDataLen);
        }
        else
        {
            if(wDataLen > PHAL_ISO7816_BER_TLV_L_00_7F)
            {
                bCmdBuff[bCmdLen++] = PHAL_ISO7816_BER_TLV_L_81;
            }

            bCmdBuff[bCmdLen++] = (uint8_t)(wDataLen);
        }

        /* Buffer ISO7816-4 Command Header. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pDataParams->pAlT4TDataParams,
            PH_EXCHANGE_BUFFER_CONT,
            bCmdBuff,
            bCmdLen,
            &pResponse,
            &wRspLen));
    }

    /* Check whether Le needs to be exchanged. */
    if(0U != (bOption & PHAL_ISO7816_EXCHANGE_LE_ONLY))
    {
        /* As per ISO/IEC:7816-4(2005), Section 5, An extended LE field consists of either three bytes
         * (one byte set to '00' followed by two bytes with any value) if the LC field is absent, or
         * two bytes (with any value) if an extended LC field is present.
         */

        /* Check whether Length is represented in extended APDU format and LC is present.
         * If true, then Le should represented in 2 bytes else LE should be represented in 3 bytes
         */
        if(bExtendedLenApdu == PH_ON)
        {
            if(!(bOption & PHAL_ISO7816_EXCHANGE_LC_ONLY))
            {
                aLe[wLe_Len++] = (uint8_t)((dwExpBytes & 0x00FF0000U) >> 16U);
            }

            aLe[wLe_Len++] = (uint8_t)((dwExpBytes & 0x0000FF00U) >> 8U);
        }

        /* Short APDU */
        aLe[wLe_Len++] = (uint8_t)(dwExpBytes & 0x000000FFU);
    }

    /* Exchange the command based on the INS. */
    switch(bIns)
    {
        case PHAL_T4T_CMD_ISO7816_SELECT_FILE:
            wStatus = phalIso7816_Sw_Int_ISOSelectFile(pDataParams, pData, wDataLen, aLe, wLe_Len, ppResponse, pRspLen);
            break;

        case PHAL_T4T_CMD_ISO7816_READ_BINARY:
            wStatus = phalIso7816_Sw_Int_ISOReadBinary(pDataParams, wBufOption, aLe, wLe_Len, ppResponse, pRspLen);
            break;

        case PHAL_T4T_CMD_ISO7816_READ_BINARY_ODO:
            wStatus = phalIso7816_Sw_Int_ISOReadBinary_ODO(pDataParams, bOption, wBufOption, pData, wDataLen, aLe, wLe_Len, ppResponse, pRspLen);
            break;

        case PHAL_T4T_CMD_ISO7816_UPDATE_BINARY:
        case PHAL_T4T_CMD_ISO7816_UPDATE_BINARY_ODO_DDO:
            wStatus = phalIso7816_Sw_Int_ISOUpdateBinary(pDataParams, pData, wDataLen);
            break;

        /*
             * This case cannot be achieved as the INS that are implemented are the supported ones only.
             * This case is kept for completeness of the switch statement and to avoid error while
             * implementing new command support in future.
             */
        default:
            wStatus = PH_ADD_COMPCODE(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AL_TOP);
            break;
    }

    return wStatus;
}

static phStatus_t phalIsol7816_Sw_IsoSelectFile(
        phalTop_T4T_t * pDataParams,
        uint8_t bOption,
        uint8_t bSelector,
        uint8_t * pFid,
        uint8_t * pDFname,
        uint8_t bDFnameLen,
        uint8_t bExtendedLenApdu,
        uint8_t ** ppFCI,
        uint16_t * pFCILen
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;
    uint8_t     PH_MEMLOC_REM aData[24U];
    uint16_t    PH_MEMLOC_REM wLc = 0U;
    uint16_t    PH_MEMLOC_REM wLe = 0U;
    uint8_t     PH_MEMLOC_REM aFileId[3U] = { '\0' };
    uint8_t     PH_MEMLOC_REM aPiccDfName[7U] = { 0xD2U, 0x76U, 0x00U, 0x00U, 0x85U, 0x01U, 0x00U };
    uint16_t    PH_MEMLOC_REM wVal = 0U;

    if (bDFnameLen > 16U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    if ((bOption != PHAL_ISO7816_FCI_RETURNED) && (bOption != PHAL_ISO7816_FCI_NOT_RETURNED))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* Validate APDU Format. */
    PH_CHECK_SUCCESS(PHAL_ISO7816_VALIDATE_APDU_FORMAT(bExtendedLenApdu));

    switch(bSelector)
    {
        case PHAL_ISO7816_SELECTOR_0:   /* Select MF, DF or EF, by file identifier */
        case PHAL_ISO7816_SELECTOR_1:   /* Select child DF */
        case PHAL_ISO7816_SELECTOR_2:   /* Select EF under the current DF, by file identifier */
        case PHAL_ISO7816_SELECTOR_3:   /* Select parent DF of the current DF */
                                        /* Selection by EF Id*/
                                        /* Send MSB first to card */
            aFileId[1U] = aData[0U] = pFid[1U];
            aFileId[0U] = aData[1U] = pFid[0U];
            aFileId[2U] = 0x00U;
            wLc = 2;
            break;

        case PHAL_ISO7816_SELECTOR_4:   /* Select by DF name, see Cmd.ISOSelect for VC selection. */
            (void) memcpy(aData, pDFname, bDFnameLen);
            wLc = bDFnameLen;
            break;

        default:
            return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    wStatus = phalIso7816_Sw_Int_SendApdu(
        pDataParams,
        (uint8_t)((bOption == PHAL_ISO7816_FCI_NOT_RETURNED) ?
            PHAL_ISO7816_EXCHANGE_LC_ONLY : PHAL_ISO7816_EXCHANGE_LC_LE_BOTH),
        PH_EXCHANGE_DEFAULT,
        bExtendedLenApdu,
        PHAL_T4T_CMD_ISO7816_SELECT_FILE,
        bSelector,
        bOption,
        aData,
        wLc,
        wLe,
        ppFCI,
        pFCILen);

    if((wStatus & PH_ERR_MASK) == PHAL_ISO7816_ERR_DF_7816_GEN_ERROR)
    {
        wVal = pDataParams->wAdditionalInfo;
    }

    if((wStatus == PH_ERR_SUCCESS) || (wVal == PHAL_ISO7816_ERR_LIMITED_FUNCTIONALITY_INS))
    {
        /* ISO wrapped mode is on */
        pDataParams->bWrappedMode = 1U;

        /* Once the selection Success, update the File Id to master data structure if the selection is done through AID */
        if((bSelector == 0x00U) || (bSelector == 0x01U) || (bSelector == 0x02U))
        {
            (void) memcpy(pDataParams->aAid, aFileId, sizeof(aFileId));
        }
        else if((bSelector == 0x04U))
        {
            /* Update the file ID to all zeros if DF Name is of PICC. */
            if(memcmp(pDFname, aPiccDfName, sizeof(aPiccDfName)) == 0U)
            {
                aFileId[0U] = 0x00U;
                aFileId[1U] = 0x00U;
                aFileId[2U] = 0x00U;
            }
            else
            {
                aFileId[0U] = 0xFFU;
                aFileId[1U] = 0xFFU;
                aFileId[2U] = 0xFFU;
            }

            (void) memcpy(pDataParams->aAid, aFileId, sizeof(aFileId));
        }
    }
    else
    {
        return wStatus;
    }

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

static phStatus_t phalIso7816_Sw_Int_CardExchange(
        phalTop_T4T_t * pDataParams,
        uint16_t wBufferOption,
        uint16_t wTotDataLen,
        uint8_t bExchangeLE,
        uint8_t * pData,
        uint16_t wDataLen,
        uint8_t ** ppResponse,
        uint16_t * pRespLen
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0U;
    phStatus_t  PH_MEMLOC_REM wPICCStatus = 0U;
    uint16_t    PH_MEMLOC_REM wLc = 0U;
    uint16_t    PH_MEMLOC_REM wRspLen = 0U;
    uint8_t     PH_MEMLOC_REM bISO7816HeaderLen = 4U;
    uint8_t     PH_MEMLOC_REM bLcLen = 0U;
    uint8_t     PH_MEMLOC_REM bCheckStatus = 0U;
    uint8_t     PH_MEMLOC_REM bLeLen = 0U;
    uint8_t     PH_MEMLOC_REM * pResponse = NULL;
    uint8_t     PH_MEMLOC_REM aLc[3] = { 0x00U, 0x00U, 0x00U };
    uint8_t     PH_MEMLOC_REM aLe[3] = { 0x00U, 0x00U, 0x00U };
    uint8_t     PH_MEMLOC_REM aISO7816Header[8U] = {PHAL_ISO7816_WRAPPEDAPDU_CLA, 0x00U,
            PHAL_ISO7816_WRAPPEDAPDU_P1, PHAL_ISO7816_WRAPPEDAPDU_P2 };

    if((wBufferOption == PH_EXCHANGE_BUFFER_FIRST) || (wBufferOption == PH_EXCHANGE_DEFAULT))
    {
        bLeLen = 1U;

        /* Set the LC information. */
        wLc = (uint16_t)(wTotDataLen - 1U /* Excluding the command code. */);

        /* Update the command code to Iso7816 header */
        aISO7816Header[1U] = pData[0U];

        /* Add the ISO 7816 header to layer 4 buffer. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pDataParams->pAlT4TDataParams,
            PH_EXCHANGE_BUFFER_FIRST,
            &aISO7816Header[0U],
            bISO7816HeaderLen,
            NULL,
            NULL));

        /* Add Lc if available */
        if(wLc)
        {
            /* Update Lc bytes according to Extended APDU option. */
            if(pDataParams->bShortLenApdu == PH_OFF)
            {
                aLc[bLcLen++] = 0x00U;
                aLc[bLcLen++] = (uint8_t)((wLc & 0xFF00U) >> 8U);

                /* Le length is updated to two if Lc is presend and the APDU is extended. */
                bLeLen = 2U;
            }

            aLc[bLcLen++] = (uint8_t)(wLc & 0x00FFU);

            /* Add the Lc to layer 4 buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pDataParams->pAlT4TDataParams,
                PH_EXCHANGE_BUFFER_CONT,
                &aLc[0U],
                bLcLen,
                NULL,
                NULL));

            /* Add the data to layer 4 buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pDataParams->pAlT4TDataParams,
                PH_EXCHANGE_BUFFER_CONT,
                &pData[1U],  /* Exclude the command code because it is added to INS. */
                (uint16_t)(wDataLen - 1U),
                NULL,
                NULL));
        }
        else
        {
            /* Update Le count */
            if(pDataParams->bShortLenApdu == PH_OFF)
            {
                bLeLen = 3U;
            }
        }
    }

    if(wBufferOption == PH_EXCHANGE_BUFFER_CONT)
    {
        /* Add the data to layer 4 buffer. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pDataParams->pAlT4TDataParams,
            PH_EXCHANGE_BUFFER_CONT,
            pData,
            wDataLen,
            NULL,
            NULL));
    }

    if((wBufferOption == PH_EXCHANGE_BUFFER_LAST) || (wBufferOption == PH_EXCHANGE_DEFAULT))
    {
        if(wBufferOption == PH_EXCHANGE_BUFFER_LAST)
        {
            /* Add the data to layer 4 buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pDataParams->pAlT4TDataParams,
                PH_EXCHANGE_BUFFER_CONT,
                pData,
                wDataLen,
                NULL,
                NULL));
        }

        /* Add Le to L4 buffer and exchange the command. */
        wStatus = phpalMifare_ExchangeL4(
            pDataParams->pAlT4TDataParams,
            PH_EXCHANGE_BUFFER_LAST,
            &aLe[0U],
            (uint8_t)(bExchangeLE ? bLeLen : 0U),
            &pResponse,
            &wRspLen);

        /* Validate the Status. */
        if(((wStatus & PH_ERR_MASK) != PH_ERR_SUCCESS) && ((wStatus & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING))
        {
            return wStatus;
        }

        /* Should the status needs to be verified. */
        bCheckStatus = (uint8_t)((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS);

        /* Combine Sw1 and Sw2 status codes. */
        if(bCheckStatus)
        {
            wPICCStatus = (uint16_t)((pResponse[wRspLen - 2U] << 8U) | pResponse[wRspLen - 1U]);
        }

        /* Evaluate the Status. */
        wStatus = phalIso7816_Sw_Int_ValidateResponse(pDataParams, PHAL_ISO7816_PRODUCT_CMD, wStatus, wPICCStatus);

        /* Create memory for updating the response of ISO 14443 format. */
        *ppResponse = pResponse;

        /* Update the response buffer length excluding SW1SW2. */
        *pRespLen = (uint16_t)(wRspLen - (bCheckStatus ? 2U : 0U));
    }

    return wStatus;
}

static phStatus_t phalIso7816_Sw_IsoReadBinary(
        phalTop_T4T_t * pDataParams,
        uint32_t dwBufOption,
        uint32_t dwOffset,
        uint8_t bSfid,
        uint32_t dwBytesToRead,
        uint8_t bExtendedLenApdu,
        uint8_t ** ppResponse,
        uint16_t * pRspLen
        )
{
    uint8_t     PH_MEMLOC_REM bP1 = 0U;
    uint8_t     PH_MEMLOC_REM bP2 = 0U;
    uint8_t     PH_MEMLOC_REM bOption = 0U;
    uint8_t     PH_MEMLOC_REM bIns = 0U;
    uint8_t     PH_MEMLOC_BUF aData[5U] = { 0x54U, 0x03U, 0x00U, 0x00U, 0x00U };

    /* Validate Parameters. */
    if((dwBufOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_DEFAULT)
    {
        if (!((dwBufOption & 0xFFFF0000) == PH_EXCHANGE_ODO))
        {
            /* Short File Identifier from 00 - 1F.
             * OR Encode first part Offset of 32767 (MSB of 0xFFFF)
             */
            bP1 = bSfid;

            /* Encode second part Offset of 32767 (LSB of 0xFFFF) */
            bP2 = (uint8_t)dwOffset;;

            bOption = PHAL_ISO7816_EXCHANGE_LE_ONLY;
            bIns = PHAL_T4T_CMD_ISO7816_READ_BINARY;
        }
        else
        {
            bP1 = 0x00U;
            bP2 = 0x00U;
            aData[2U] = (uint8_t)((dwOffset & 0x00FF0000) >> 16U);
            aData[3U] = (uint8_t)((dwOffset & 0x0000FF00) >> 8U);
            aData[4U] = (uint8_t)((dwOffset & 0x000000FF));
            bOption = (uint8_t)(PHAL_ISO7816_EXCHANGE_LC_LE_BOTH | (PH_EXCHANGE_ODO >> 12U));
            bIns = PHAL_T4T_CMD_ISO7816_READ_BINARY_ODO;
        }

    }
    else if((dwBufOption & PH_EXCHANGE_MODE_MASK) == PH_EXCHANGE_RXCHAINING)
    {
        /* Do Nothing. Code is handled internally. */
    }
    else
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* Validate APDU Format. */
    PH_CHECK_SUCCESS(PHAL_ISO7816_VALIDATE_APDU_FORMAT(bExtendedLenApdu));

    return phalIso7816_Sw_Int_SendApdu(
        pDataParams,
        bOption,
        (uint8_t)dwBufOption,
        bExtendedLenApdu,
        bIns,
        bP1,
        bP2,
        aData,
        sizeof(aData),
        dwBytesToRead,
        ppResponse,
        pRspLen);
}

static phStatus_t phalIso7816_Sw_IsoUpdateBinary(
        phalTop_T4T_t * pDataParams,
        uint32_t dwBufOption,
        uint32_t dwOffset,
        uint8_t bSfid,
        uint8_t bExtendedLenApdu,
        uint8_t * pData,
        uint16_t wDataLen
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;
    uint8_t     PH_MEMLOC_REM bOption = 0U;
    uint8_t     PH_MEMLOC_REM bIns = 0U;
    uint8_t     PH_MEMLOC_REM bP1 = 0U;
    uint8_t     PH_MEMLOC_REM bP2 = 0U;
    uint8_t     PH_MEMLOC_BUF aData[5U];
    uint8_t     PH_MEMLOC_BUF * pCmd;
    uint16_t    PH_MEMLOC_BUF wCmdLength = 0U;

    pCmd = (uint8_t *)&aData[0U];

    if (!((dwBufOption & 0xFFFF0000) == PH_EXCHANGE_ODO_DDO))
    {
        /* Short File Identifier from 00 - 1F.
         * OR Encode first part Offset of 32767 (MSB of 0xFFFF)
         */
        bP1 = bSfid;

        /* Encode second part Offset of 32767 (LSB of 0xFFFF) */
        bP2 = (uint8_t)dwOffset;;

        bOption = PHAL_ISO7816_EXCHANGE_LC_ONLY;
        bIns = PHAL_T4T_CMD_ISO7816_UPDATE_BINARY;
    }
    else
    {
        bP1 = 0x00U;
        bP2 = 0x00U;
        aData[(uint8_t)wCmdLength++] = 0x54U;
        aData[(uint8_t)wCmdLength++] = 0x03U;
        aData[(uint8_t)wCmdLength++] = (uint8_t)((dwOffset & 0x00FF0000) >> 16U);
        aData[(uint8_t)wCmdLength++] = (uint8_t)((dwOffset & 0x0000FF00) >> 8U);
        aData[(uint8_t)wCmdLength++] = (uint8_t)((dwOffset & 0x000000FF));
        bOption = (uint8_t)(PHAL_ISO7816_EXCHANGE_LC_ONLY | (PH_EXCHANGE_ODO_DDO >> 12U));
        bIns = PHAL_T4T_CMD_ISO7816_UPDATE_BINARY_ODO_DDO;
    }

    /* Validate APDU Format. */
    PH_CHECK_SUCCESS(PHAL_ISO7816_VALIDATE_APDU_FORMAT(bExtendedLenApdu));

    wStatus = phalIso7816_Sw_Int_SendApdu(
        pDataParams,
        bOption,
        (uint8_t)dwBufOption,
        bExtendedLenApdu,
        bIns,
        bP1,
        bP2,
        pData,
        wDataLen,
        0U,
        &pCmd,
        &wCmdLength);

    return wStatus;
}

static phStatus_t phalIso7816_Sw_CreateApplication(
        phalTop_T4T_t * pDataParams,
        uint8_t bOption,
        uint8_t * pAid,
        uint8_t bKeySettings1,
        uint8_t bKeySettings2,
        uint8_t bKeySettings3,
        uint8_t * pKeySetValues,
        uint8_t bKeySetValuesLen,
        uint8_t * pISOFileId,
        uint8_t * pISODFName,
        uint8_t bISODFNameLen
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0U;
    uint8_t     PH_MEMLOC_REM bCmdBuf[25U] = { 0U };
    uint8_t     PH_MEMLOC_REM bCmdLen = 0U;
    uint8_t     PH_MEMLOC_REM * pResp = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0U;

    if(((bOption & PHAL_ISO7816_ISO_DF_NAME_AVAILABLE) && (bISODFNameLen > 16U))
        || (bOption > PHAL_ISO7816_ISO_FILE_ID_DF_NAME_AVAILABLE))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* Frame the command information. */
    bCmdBuf[bCmdLen++] = PHAL_ISO7816_CMD_CREATE_APPLICATION;

    /* Add Application ID to command buffer. */
    (void)memcpy(&bCmdBuf[bCmdLen], pAid, 0x03U);
    bCmdLen += 0x03U;

    /* Add Key Settings to command buffer. */
    bCmdBuf[bCmdLen++] = bKeySettings1;
    bCmdBuf[bCmdLen++] = bKeySettings2;

    /* Add Key Settings 3 and Key Set values to command buffer. */
    if(bKeySettings2 & PHAL_ISO7816_KEYSETT3_PRESENT)
    {
        bCmdBuf[bCmdLen++] = bKeySettings3;
        if(bKeySettings3 & PHAL_ISO7816_KEYSETVALUES_PRESENT)
        {
            (void)memcpy(&bCmdBuf[bCmdLen], pKeySetValues, bKeySetValuesLen);
            bCmdLen += bKeySetValuesLen;
        }
    }

    /* Add ISO File ID to command buffer. */
    if(bOption & PHAL_ISO7816_ISO_FILE_ID_AVAILABLE)
    {
        bCmdBuf[bCmdLen++] = pISOFileId[0U];
        bCmdBuf[bCmdLen++] = pISOFileId[1U];
    }

    /* Add ISO DFName to command buffer. */
    if(bOption & PHAL_ISO7816_ISO_DF_NAME_AVAILABLE)
    {
        (void)memcpy(&bCmdBuf[bCmdLen], pISODFName, bISODFNameLen);
        bCmdLen += bISODFNameLen;
    }

    /* Enable Short APDU */
    pDataParams->bShortLenApdu = PH_ON;

    PH_CHECK_SUCCESS_FCT(wStatus, phalIso7816_Sw_Int_CardExchange(
        pDataParams,
        PH_EXCHANGE_DEFAULT,
        bCmdLen,
        PH_ON,
        bCmdBuf,
        bCmdLen,
        &pResp,
        &wRespLen));

    return PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP);
}

static phStatus_t phalIso7816_Sw_CreateStdDataFile(
        phalTop_T4T_t * pDataParams,
        uint8_t bOption,
        uint8_t bFileNo,
        uint8_t * pISOFileId,
        uint8_t bFileOption,
        uint8_t * pAccessRights,
        uint8_t * pFileSize
        )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0U;
    uint8_t     PH_MEMLOC_REM bCmdBuf[25U] = { 0U };
    uint8_t     PH_MEMLOC_REM bCmdLen = 0U;
    uint8_t     PH_MEMLOC_REM * pResp = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0U;

    if(bOption > PHAL_ISO7816_ISO_FILE_ID_AVAILABLE)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* Frame the command information. */
    bCmdBuf[bCmdLen++] = PHAL_T4T_CMD_CREATE_STANDARD_DATA_FILE;
    bCmdBuf[bCmdLen++] = bFileNo;

    /* Add ISO File ID to command buffer. */
    if(bOption == PHAL_ISO7816_ISO_FILE_ID_AVAILABLE)
    {
        (void)memcpy(&bCmdBuf[bCmdLen], pISOFileId, 2U);
        bCmdLen += 2U;
    }

    /* Add File Options to command buffer. */
    bCmdBuf[bCmdLen++] = bFileOption;

    /* Add Access Rights to command buffer. */
    (void)memcpy(&bCmdBuf[bCmdLen], pAccessRights, 2U);
    bCmdLen += 2U;

    /* Add File Size to command buffer. */
    (void)memcpy(&bCmdBuf[bCmdLen], pFileSize, 3U);
    bCmdLen += 3U;

    pDataParams->bShortLenApdu = PH_ON;

    PH_CHECK_SUCCESS_FCT(wStatus, phalIso7816_Sw_Int_CardExchange(
        pDataParams,
        PH_EXCHANGE_DEFAULT,
        bCmdLen,
        PH_ON,
        bCmdBuf,
        bCmdLen,
        &pResp,
        &wRespLen));

    return PH_ADD_COMPCODE(wStatus, PH_COMP_AL_TOP);
}

phStatus_t phalTop_Sw_Int_T4T_ClearState(
        phalTop_Sw_DataParams_t * pDataParams,
        phalTop_T4T_t * pT4T
        )
{
    uint8_t  PH_MEMLOC_BUF aFidMApp[2U] = {0x00U, 0x3FU};
    uint8_t  PH_MEMLOC_REM * pFCI = NULL;
    uint16_t PH_MEMLOC_REM wFCILen = 0U;

    /* Workaround for MIFARE DESFire contactless IC tags to enable selection of NDEF application
     * again, when CheckNdef is called multiple times. By default selection of
     * same application again will return error. To avoid this, Master
     * application is selection is done.
     *
     * NOTE: As per NFC specification calling CheckNdef multiple times is not
     * needed under any use case. So during normal use cases this selection of
     * master application file will not be executed. */
    if(pT4T->bCurrentSelectedFile != 0U)
    {
        /* Select the Master application (file ID 0x3F00) */
        (void)phalIsol7816_Sw_IsoSelectFile(
            pT4T,
            PHAL_ISO7816_FCI_NOT_RETURNED,
            PHAL_ISO7816_SELECTOR_0,
            aFidMApp,
            NULL,
            0x00U,
            0x00U,
            &pFCI,
            &wFCILen);
    }

    /* Reset parameters */
    pT4T->wCCLEN = 0U;
    pDataParams->bVno = 0U;
    pT4T->wMLe = 0U;
    pT4T->wMLc = 0U;

    pT4T->aNdefFileID[0U] = 0U;
    pT4T->aNdefFileID[1U] = 0U;
    pT4T->dwMaxFileSize = 0U;
    pT4T->bRa = 0U;
    pT4T->bWa = 0U;
    pDataParams->dwNdefLength = 0U;

    pDataParams->bTagState = 0U;
    pT4T->bCurrentSelectedFile = 0U;
    pT4T->wAdditionalInfo = 0U;
    pT4T->bShortLenApdu = PH_ON;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T4T_CheckNdef(
        phalTop_Sw_DataParams_t * pDataParams,
        uint8_t * pTagState
        )
{
    phStatus_t    PH_MEMLOC_REM status;
    uint32_t      PH_MEMLOC_REM wBytesRead = 0U;
    uint8_t       PH_MEMLOC_REM * pRxBuffer = NULL;
    uint8_t       PH_MEMLOC_BUF aFidCc[2] = {0x03U, 0xE1U};
    uint8_t       PH_MEMLOC_BUF aNdefAppName[7U] = {0xD2U, 0x76U, 0x00U, 0x00U, 0x85U, 0x01U, 0x01U};
    uint8_t       PH_MEMLOC_REM * pFCI = NULL;
    uint8_t       PH_MEMLOC_REM aFileCtrlTlvV[8U] = { 0x00U };
    uint16_t      PH_MEMLOC_REM wReceivedCCLen = 0U;
    uint16_t      PH_MEMLOC_REM wFCILen = 0U;
    phalTop_T4T_t PH_MEMLOC_REM * pT4T = &pDataParams->ualTop.salTop_T4T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT4T->pAlT4TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Reset tag state */
    *pTagState = PHAL_TOP_STATE_NONE;

    /* Set Wrapped mode */
    pT4T->bWrappedMode = 1U;

    /* Clear values from previous detection, if any */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T4T_ClearState(pDataParams, pT4T));

    /* Select the NDEF Tag Application */
    status = phalIsol7816_Sw_IsoSelectFile(
        pT4T,
        PHAL_ISO7816_FCI_RETURNED,
        PHAL_ISO7816_SELECTOR_4,
        NULL,
        aNdefAppName,
        sizeof(aNdefAppName),
        0x00U,
        &pFCI,
        &wFCILen);

    if((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_NON_NDEF_TAG, PH_COMP_AL_TOP);
    }

    pT4T->bCurrentSelectedFile = PHAL_TOP_T4T_SELECTED_NDEF_APP;

    /* Select the Capability Container (CC) file */
    PH_CHECK_SUCCESS_FCT(status, phalIsol7816_Sw_IsoSelectFile(
        pT4T,
        PHAL_ISO7816_FCI_NOT_RETURNED,
        PHAL_ISO7816_SELECTOR_0,
        aFidCc,
        NULL,
        0x00U,
        0x00U,
        &pFCI,
        &wFCILen));

    pT4T->bCurrentSelectedFile = PHAL_TOP_T4T_SELECTED_CC_FILE;

    /* Read the CC file */
    PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoReadBinary(
        pT4T,
        PH_EXCHANGE_DEFAULT,
        0x00U,
        0x00U,
        (uint32_t)PHAL_TOP_T4T_CC_FILE_MIN_LEN,
        0x00U,
        &pRxBuffer,
        (uint16_t *)&wBytesRead));

    /* Validate CC length (CCLEN) */
    wReceivedCCLen = (((uint16_t)pRxBuffer[PHAL_TOP_T4T_CCLEN_OFFSET] << 8U) |
        pRxBuffer[PHAL_TOP_T4T_CCLEN_OFFSET + 1U]);

    if((wReceivedCCLen < PHAL_TOP_T4T_CC_FILE_MIN_LEN) || (wReceivedCCLen > PHAL_TOP_T4T_CC_FILE_MAX_LEN))
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
    }

    /* Update version number */
    pDataParams->bVno = pRxBuffer[PHAL_TOP_T4T_VERSION_OFFSET];

    /* Check for supported version number */
    if(!(((uint8_t)(pDataParams->bVno & 0xF0U) == (uint8_t)(PHAL_TOP_T4T_NDEF_SUPPORTED_VNO & 0xF0U)) ||
       ((uint8_t)(pDataParams->bVno & 0xF0U) == (uint8_t)(PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20 & 0xF0U))))
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_UNSUPPORTED_VERSION, PH_COMP_AL_TOP);
    }

    /* Update MLe and MLc */
    pT4T->wMLe = (uint16_t)((uint16_t)pRxBuffer[PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_OFFSET] << 8U) |
        pRxBuffer[PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_OFFSET + 1U];
    pT4T->wMLc = (uint16_t)((uint16_t)pRxBuffer[PHAL_TOP_T4T_MAX_CMD_LENGTH_OFFSET] << 8U) |
        pRxBuffer[PHAL_TOP_T4T_MAX_CMD_LENGTH_OFFSET + 1U];

    /* Validate MLe and MLc */
    if((pT4T->wMLe < PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_MIN) || (pT4T->wMLc < PHAL_TOP_T4T_MAX_CMD_LENGTH_MIN))
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
    }

    /* Validate File Control TLV T-Field and L-Field */
    if((pDataParams->bVno & 0xF0) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20)
    {
        if((pRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET] != PHAL_TOP_T4T_NDEF_TLV_T) ||
            (pRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 1U] != PHAL_TOP_T4T_NDEF_TLV_L))
        {
            /* NDEF TLV not present */
            return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
        }
    }
    else
    {
        if((pRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET] != PHAL_TOP_T4T_ENDEF_TLV_T) ||
            (pRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 1U] != PHAL_TOP_T4T_ENDEF_TLV_L))
        {
            /* NDEF TLV not present */
            return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
        }
    }

    memcpy(aFileCtrlTlvV, &(pRxBuffer[PHAL_TOP_T4T_NDEFTLV_V_FILE_ID_OFFSET]), PHAL_TOP_T4T_NDEF_FILE_CTRL_TLV_L);

    /* Read CC bytes (NDEF TLV V Field) */
    if(wReceivedCCLen > PHAL_TOP_T4T_CC_FILE_MIN_LEN)
    {
        /* Read the CC file (NDEF TLV Bytes V Field) */
        PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoReadBinary(
            pT4T,
            PH_EXCHANGE_DEFAULT,
            (uint8_t)(PHAL_TOP_T4T_NDEFTLV_OFFSET + 2U),
            0x00,
            (uint32_t)(pRxBuffer[PHAL_TOP_T4T_NDEFTLV_OFFSET + 1U]),
            0x00,
            &pRxBuffer,
            (uint16_t *)&wBytesRead));

        memcpy(aFileCtrlTlvV, pRxBuffer, PHAL_TOP_T4T_ENDEF_FILE_CTRL_TLV_L);
    }

    if((pDataParams->bVno & 0xF0) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20)
    {
        /* Update NDEF READ and WRITE access MV 2.0 */
        pT4T->bRa = aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_RA_MV20_BUF_OFFSET];
        pT4T->bWa = aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_WA_MV20_BUF_OFFSET];
    }
    else
    {
        /* Update NDEF READ and WRITE access MV 3.0 */
        pT4T->bRa = aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_RA_MV30_BUF_OFFSET];
        pT4T->bWa = aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_WA_MV30_BUF_OFFSET];
    }

    /* Validate read access */
    if(pT4T->bRa == PHAL_TOP_T4T_NDEF_FILE_READ_ACCESS)
    {
        if((pDataParams->bVno & 0xF0) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20)
        {
            /* Max NDEF file size */
            pT4T->dwMaxFileSize = ((uint16_t)aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_BUF_OFFSET] << 8U) |
                aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_BUF_OFFSET + 1U];
        }
        else
        {
            pT4T->dwMaxFileSize = (((uint32_t)aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_BUF_OFFSET] << 24U) |
                ((uint32_t)aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_BUF_OFFSET + 1U] << 16U) |
                ((uint32_t)aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_BUF_OFFSET + 2U] << 8U) |
                (uint32_t)aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_BUF_OFFSET + 3U]);
        }

        /* Validate max file size */
        if((((pT4T->dwMaxFileSize < PHAL_TOP_T4T_NDEF_FILE_SIZE_MIN_MV20) ||
            (pT4T->dwMaxFileSize > PHAL_TOP_T4T_NDEF_FILE_SIZE_MAX_MV20)) &&
                ((pDataParams->bVno & 0xF0) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20)) ||
            (((pT4T->dwMaxFileSize < PHAL_TOP_T4T_NDEF_FILE_SIZE_MIN_MV30) ||
            (pT4T->dwMaxFileSize > PHAL_TOP_T4T_NDEF_FILE_SIZE_MAX_MV30)) &&
                ((pDataParams->bVno & 0xF0) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO)))
        {
            return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
        }

        /* Read NDEF File ID */
        pT4T->aNdefFileID[0] = aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_FILE_ID_BUF_OFFSET + 1U];
        pT4T->aNdefFileID[1] = aFileCtrlTlvV[PHAL_TOP_T4T_NDEFTLV_V_FILE_ID_BUF_OFFSET];

        /* Validate NDEF file ID */
        if(!(((((uint16_t)pT4T->aNdefFileID[1] << 8U) | pT4T->aNdefFileID[0]) == 0x0000U) ||
            ((((uint16_t)pT4T->aNdefFileID[1] << 8U) | pT4T->aNdefFileID[0]) == 0xE102U) ||
            ((((uint16_t)pT4T->aNdefFileID[1] << 8U) | pT4T->aNdefFileID[0]) == 0xE103U) ||
            ((((uint16_t)pT4T->aNdefFileID[1] << 8U) | pT4T->aNdefFileID[0]) == 0x3F00U) ||
            ((((uint16_t)pT4T->aNdefFileID[1] << 8U) | pT4T->aNdefFileID[0]) == 0x3FFFU) ||
            ((((uint16_t)pT4T->aNdefFileID[1] << 8U) | pT4T->aNdefFileID[0]) == 0xFFFFU)))
        {
            /* Select the NDEF file  */
            PH_CHECK_SUCCESS_FCT(status, phalIsol7816_Sw_IsoSelectFile(
                pT4T,
                PHAL_ISO7816_FCI_NOT_RETURNED,
                PHAL_ISO7816_SELECTOR_0,
                pT4T->aNdefFileID,
                NULL,
                0x00U,
                0x00U,
                &pFCI,
                &wFCILen));

            pT4T->bCurrentSelectedFile = PHAL_TOP_T4T_SELECTED_NDEF_FILE;

            /* Read the Length of the NDEF File */
            PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoReadBinary(
                pT4T,
                PH_EXCHANGE_DEFAULT,
                0x00U,
                0x00U,
                (((pDataParams->bVno & 0xF0U) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20) ?
                	(uint32_t)PHAL_TOP_T4T_NLEN_SIZE : (uint32_t)PHAL_TOP_T4T_ENLEN_SIZE),
                0x00U,
                &pRxBuffer,
                (uint16_t *)&wBytesRead));

            if ((pDataParams->bVno & 0xF0U) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20)
            {
                /* Update NDEF message length */
                pDataParams->dwNdefLength = ((uint16_t)pRxBuffer[0U] << 8U) | pRxBuffer[1U];
            }
            else
            {
                /* Update NDEF message length */
                pDataParams->dwNdefLength = (((uint32_t)pRxBuffer[0U] << 24U) | ((uint32_t)pRxBuffer[1] << 16U) |
                    ((uint32_t)pRxBuffer[2U] << 8U) | (uint32_t)pRxBuffer[3U]);
            }

            /* Validate NDEF length */
            if((pDataParams->dwNdefLength > 0x0000U) && (pDataParams->dwNdefLength <= (pT4T->dwMaxFileSize - 2U)))
            {
                if ((pT4T->bWa > PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS) &&
                    (pT4T->bWa < PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS_PROP_START))
                {
                    /* RFU */
                    return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
                }
                /* Limited WRITE access, granted based on proprietary methods option is not supported.
                 * Therefore, from 0x80 to 0xFE are considered as no WRITE access granted like 0xFF */
                else if(pT4T->bWa >= PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS_PROP_START)
                {
                    pDataParams->bTagState = PHAL_TOP_STATE_READONLY;
                }
                else
                {
                    pDataParams->bTagState = PHAL_TOP_STATE_READWRITE;
                }
            }
            else if((pDataParams->dwNdefLength == 0x0000U))
            {
                /* Check write access */
                if(pT4T->bWa == PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS)
                {
                    pDataParams->bTagState = PHAL_TOP_STATE_INITIALIZED;
                }
                else
                {
                    return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
                }
            }
            else
            {
                /* NDEF length / file size not properly configured */
                return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
            }
        }
        else
        {
            /* Wrong NDEF file ID */
            return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
        }
    }
    else
    {
        /* Proprietary read options; Not supported */
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
    }

    /* Update max. NDEF size */
    pDataParams->dwMaxNdefLength = (pT4T->dwMaxFileSize - 2U);

    /* Update state in out parameter */
    *pTagState = pDataParams->bTagState;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T4T_FormatNdef(phalTop_Sw_DataParams_t * pDataParams)
{
    phStatus_t    PH_MEMLOC_REM status;
    uint8_t       PH_MEMLOC_BUF aFidMApp[2U] = {0x00U, 0x3FU};
    uint8_t       PH_MEMLOC_BUF aCcFileLen[3U] = {PHAL_TOP_T4T_CCLEN_MV30, 0x00U, 0x00U};
    uint8_t       PH_MEMLOC_BUF aNdefFileLen[3U] = {0x00U, 0x04U, 0x00U};
    uint8_t       PH_MEMLOC_BUF aAccessRights[2U] = {0xEEU, 0xEEU };
    uint8_t       PH_MEMLOC_BUF aAid[3U] = {0x01U, 0x00U, 0x00U };
    uint8_t       PH_MEMLOC_BUF aFidApp[2U] = {0x02U, 0xE1U };
    uint8_t       PH_MEMLOC_BUF aFidCc[2U] = {0x03U, 0xE1U };
    uint8_t       PH_MEMLOC_REM * pFCI = NULL;
    uint16_t      PH_MEMLOC_REM wFCILen = 0U;
    /* NDEF application name */
    uint8_t       PH_MEMLOC_REM aNdefAppName[7U] = {0xD2U, 0x76U, 0x00U, 0x00U, 0x85U, 0x01U, 0x01U };
    /* Default CC File as per Mapping version 3.0 */
    uint8_t       PH_MEMLOC_REM aCcData[17U] = {
                    0x00, PHAL_TOP_T4T_CCLEN_MV30,      /* Offset 0 and 1: CC Length */
                    PHAL_TOP_T4T_NDEF_SUPPORTED_VNO,    /* Offset 2: Mapping Version 3.0 */
                    0x00U, 0x3BU,                       /* Offset 3 and 4: MLe (59 bytes); maximum R-APDU data size */
                    0x00U, 0x34U,                       /* Offset 5 and 6: MLc (52 bytes); maximum C-APDU data size */
                    PHAL_TOP_T4T_ENDEF_FILE_CTRL_TLV_T, /* Offset 7: T-field of the ENDEF-File_Ctrl_TLV */
                    PHAL_TOP_T4T_ENDEF_FILE_CTRL_TLV_L, /* Offset 8: L-Field of the ENDEF-File_Ctrl_TLV */
                    0xE1U, 0x04U,                       /* Offset 9 and 10: File identifier */
                    0x00U, 0x00U, 0x04U, 0x00U,         /* Offset 11 to 14: NDEF File size 1KB */
                    PHAL_TOP_T4T_NDEF_FILE_READ_ACCESS, /* Offset 15: NDEF File READ access */
                    PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS /* Offset 16: NDEF File WRITE access */
                    };

    /* Default NDEF file */
    uint8_t    PH_MEMLOC_REM aNdefData[7U] = {0x00U, 0x00U, 0x00U, 0x03U, 0xD0U, 0x00U, 0x00U };
    phalTop_T4T_t PH_MEMLOC_REM * pT4T = &pDataParams->ualTop.salTop_T4T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT4T->pAlT4TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Check for NDEF presence */
    if(pDataParams->bTagState != PHAL_TOP_STATE_NONE)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_FORMATTED_TAG, PH_COMP_AL_TOP);
    }

    /* Mapping Version number, default 3.0 */
    if ((pDataParams->bVno & 0xF0U) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20)
    {
        aCcData[PHAL_TOP_T4T_VERSION_OFFSET] = PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20;  /* Mapping Version 2.0 */
    }
    else
    {
        pDataParams->bVno = PHAL_TOP_T4T_NDEF_SUPPORTED_VNO; /* Mapping Version 3.0 */
    }

    /* Validate and update MLe */
    if(pT4T->wMLe >= PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_MIN)
    {
        aCcData[PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_OFFSET] = (uint8_t)((pT4T->wMLe & 0xFF00U) >> 8U);
        aCcData[PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_OFFSET + 1U] = (uint8_t)(pT4T->wMLe & 0x00FFU);
    }
    else
    {
        pT4T->wMLe = ((uint16_t)aCcData[PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_OFFSET] << 8U) |
            aCcData[PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_OFFSET + 1U];
    }

    /* Validate and update MLc */
    if(pT4T->wMLc >= PHAL_TOP_T4T_MAX_CMD_LENGTH_MIN)
    {
        aCcData[PHAL_TOP_T4T_MAX_CMD_LENGTH_OFFSET] = (uint8_t)((pT4T->wMLc & 0xFF00U) >> 8U);
        aCcData[PHAL_TOP_T4T_MAX_CMD_LENGTH_OFFSET + 1U] = (uint8_t)(pT4T->wMLc & 0x00FFU);
    }
    else
    {
        pT4T->wMLc = ((uint16_t)aCcData[PHAL_TOP_T4T_MAX_CMD_LENGTH_OFFSET] << 8U) |
            aCcData[PHAL_TOP_T4T_MAX_CMD_LENGTH_OFFSET + 1U];
    }

    /* Validate and update NDEF file ID */
    if(!(((((uint16_t)pT4T->aNdefFileID[1U] << 8U) | pT4T->aNdefFileID[0U]) == 0x0000U) ||
        ((((uint16_t)pT4T->aNdefFileID[1U] << 8U) | pT4T->aNdefFileID[0U]) == 0x3F00U) ||
        ((((uint16_t)pT4T->aNdefFileID[1U] << 8U) | pT4T->aNdefFileID[0U]) == 0x3FFFU) ||
        ((((uint16_t)pT4T->aNdefFileID[1U] << 8U) | pT4T->aNdefFileID[0U]) == 0xE102U) ||
        ((((uint16_t)pT4T->aNdefFileID[1U] << 8U) | pT4T->aNdefFileID[0U]) == 0xE103U) ||
        ((((uint16_t)pT4T->aNdefFileID[1U] << 8U) | pT4T->aNdefFileID[0U]) == 0xFFFFU)))
    {
        aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_ID_OFFSET + 1U] = pT4T->aNdefFileID[0U];
        aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_ID_OFFSET] = pT4T->aNdefFileID[1U];
    }
    else
    {
        pT4T->aNdefFileID[0U] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_ID_OFFSET + 1U];
        pT4T->aNdefFileID[1U] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_ID_OFFSET];
    }

    if ((pDataParams->bVno & 0xF0U) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO)
    {
        /* Validate and update file size */
        if(!((pT4T->dwMaxFileSize < PHAL_TOP_T4T_NDEF_FILE_SIZE_MIN_MV30) ||
            (pT4T->dwMaxFileSize > PHAL_TOP_T4T_NDEF_FILE_SIZE_MAX_MV30)))
        {
            aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET] = (uint8_t)((pT4T->dwMaxFileSize & 0xFF000000U) >> 24U);
            aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 1U] = (uint8_t)((pT4T->dwMaxFileSize & 0x00FF0000U) >> 16U);
            aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 2U] = (uint8_t)((pT4T->dwMaxFileSize & 0x0000FF00U) >> 8U);
            aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 3U] = (uint8_t)(pT4T->dwMaxFileSize & 0x000000FFU);
        }
        else
        {
            pT4T->dwMaxFileSize = (((uint32_t)aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET] << 24U) |
                ((uint32_t)aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 1U] << 16U) |
                ((uint32_t)aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 2U] << 8U) |
                (uint32_t)aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 3U]);
        }

        /* Get NDEF file size */
        aNdefFileLen[0U] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 3U];
        aNdefFileLen[1U] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 2U];
        aNdefFileLen[2U] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 1U];
    }
    else
    {
        /* Update CC length */
        aCcData[PHAL_TOP_T4T_CCLEN_OFFSET + 1U] = PHAL_TOP_T4T_CCLEN_MV20;

        /* T-Field and L-Field of the NDEF File Control TLV */
        aCcData[PHAL_TOP_T4T_NDEFTLV_OFFSET] = PHAL_TOP_T4T_NDEF_FILE_CTRL_TLV_T;
        aCcData[PHAL_TOP_T4T_NDEFTLV_OFFSET + 1U] = PHAL_TOP_T4T_NDEF_FILE_CTRL_TLV_L;

        /* Validate and update file size */
        if(!((pT4T->dwMaxFileSize < PHAL_TOP_T4T_NDEF_FILE_SIZE_MIN_MV20) ||
            (pT4T->dwMaxFileSize > PHAL_TOP_T4T_NDEF_FILE_SIZE_MAX_MV20)))
        {
            aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET] = (uint8_t)((pT4T->dwMaxFileSize & 0xFF00U) >> 8U);
            aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 1U] = (uint8_t)(pT4T->dwMaxFileSize & 0x00FFU);
        }
        else
        {
            aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 2U];
            aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 1U] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 3U];
            pT4T->dwMaxFileSize = ((uint16_t)aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET] << 8U) |
                aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 1U];
        }

        /* By default, tag is formated in READ-WRITE state */
        aCcData[PHAL_TOP_T4T_NDEFTLV_V_RA_MV20_OFFSET] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_RA_MV30_OFFSET];
        aCcData[PHAL_TOP_T4T_NDEFTLV_V_WA_MV20_OFFSET] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_WA_MV30_OFFSET];
        pT4T->bRa = aCcData[PHAL_TOP_T4T_NDEFTLV_V_RA_MV20_OFFSET];
        pT4T->bWa = aCcData[PHAL_TOP_T4T_NDEFTLV_V_WA_MV20_OFFSET];

        aCcFileLen[0] = PHAL_TOP_T4T_CCLEN_MV20;

        /* Get NDEF file size */
        aNdefFileLen[0] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET + 1U];
        aNdefFileLen[1] = aCcData[PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET];
    }

    /* Update CC length */
    pT4T->wCCLEN = aCcData[PHAL_TOP_T4T_CCLEN_OFFSET + 1U];

    /* Select the Master application (file ID 0x3F00) for MIFARE DESFire */
    status = phalIsol7816_Sw_IsoSelectFile(
        pT4T,
        PHAL_ISO7816_FCI_NOT_RETURNED,
        PHAL_ISO7816_SELECTOR_0,
        aFidMApp,
        NULL,
        0x00U,
        0x00U,
        &pFCI,
        &wFCILen);

    /* Check for success (for MIFARE DESFire)*/
    if((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
    }

    /* Select the NDEF Tag Application */
    status = phalIsol7816_Sw_IsoSelectFile(
        pT4T,
        PHAL_ISO7816_FCI_RETURNED,
        PHAL_ISO7816_SELECTOR_4,
        NULL,
        aNdefAppName,
        sizeof(aNdefAppName),
        0x00U, /* Extended APDU */
        &pFCI,
        &wFCILen);

    if((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        /* Create NDEF tag application */
        PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_CreateApplication(
            pT4T,
            0x03U,
            aAid,
            0x0FU,
            0x21U,
            0x00,
            NULL,
            0x00U,
            aFidApp,
            aNdefAppName,
            0x07U));

        /* Select the NDEF Tag Application */
        PH_CHECK_SUCCESS_FCT(status, phalIsol7816_Sw_IsoSelectFile(
            pT4T,
            PHAL_ISO7816_FCI_RETURNED,
            PHAL_ISO7816_SELECTOR_4,
            NULL,
            aNdefAppName,
            sizeof(aNdefAppName),
            0x00U,
            &pFCI,
            &wFCILen));
    }

    /* Select the Capability Container (CC) file */
    status = phalIsol7816_Sw_IsoSelectFile(
        pT4T,
        PHAL_ISO7816_FCI_NOT_RETURNED,
        PHAL_ISO7816_SELECTOR_0,
        aFidCc,
        NULL,
        0x00U,
        0x00U,
        &pFCI,
        &wFCILen);

    if((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        /* Create CC file */
        PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_CreateStdDataFile(
            pT4T,
            0x01U,
            0x01U,
            aFidCc,
            PHAL_ISO7816_COMMUNICATION_PLAIN,
            aAccessRights,
            aCcFileLen));

        /* Select the Capability Container (CC) file */
        PH_CHECK_SUCCESS_FCT(status, phalIsol7816_Sw_IsoSelectFile(
            pT4T,
            PHAL_ISO7816_FCI_NOT_RETURNED,
            PHAL_ISO7816_SELECTOR_0,
            aFidCc,
            NULL,
            0x00U,
            0x00U,
            &pFCI,
            &wFCILen));
    }

    /* Write data to CC file */
    PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoUpdateBinary(
        pT4T,
        PH_EXCHANGE_DEFAULT,
        0x00U,
        0x00U,
        0x00U,
        aCcData,
        pT4T->wCCLEN));

    /* Select the NDEF file */
    status = phalIsol7816_Sw_IsoSelectFile(
        pT4T,
        PHAL_ISO7816_FCI_NOT_RETURNED,
        PHAL_ISO7816_SELECTOR_0,
        pT4T->aNdefFileID,
        NULL,
        0x00U,
        0x00U,
        &pFCI,
        &wFCILen);

    if((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        /* Create NDEF file */
        PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_CreateStdDataFile(
            pT4T,
            0x01U,
            0x02U,
            pT4T->aNdefFileID,
            PHAL_ISO7816_COMMUNICATION_PLAIN,
            aAccessRights,
            aNdefFileLen));

        /* Select the NDEF file */
        PH_CHECK_SUCCESS_FCT(status, phalIsol7816_Sw_IsoSelectFile(
            pT4T,
            PHAL_ISO7816_FCI_NOT_RETURNED,
            PHAL_ISO7816_SELECTOR_0,
            pT4T->aNdefFileID,
            NULL,
            0x00U,
            0x00U,
            &pFCI,
            &wFCILen));
    }

    /* Write Empty NDEF message to NDEF file */
    PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoUpdateBinary(
        pT4T,
        PH_EXCHANGE_DEFAULT,
        0x00U,
        0x00U,
        0x00U,
        (((pDataParams->bVno & 0xF0U) != PHAL_TOP_T4T_NDEF_SUPPORTED_VNO) ? &aNdefData[2U] : &aNdefData[0]),
        (uint16_t)(((pDataParams->bVno & 0xF0U) != PHAL_TOP_T4T_NDEF_SUPPORTED_VNO) ? 0x05U : 0x07U)));

    /* Select the Master application (file ID 0x3F00) */
    /* This is being performed again to go to the master application back to perform different operations */
    PH_CHECK_SUCCESS_FCT(status, phalIsol7816_Sw_IsoSelectFile(
        pT4T,
        PHAL_ISO7816_FCI_NOT_RETURNED,
        PHAL_ISO7816_SELECTOR_0,
        aFidMApp,
        NULL,
        0x00U,
        0x00U,
        &pFCI,
        &wFCILen));

    /* Update state parameters */
    pDataParams->dwNdefLength = 0x03U;
    pDataParams->bTagState = PHAL_TOP_STATE_READWRITE;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T4T_ReadNdef(
            phalTop_Sw_DataParams_t * pDataParams,
            uint8_t * pData,
            uint32_t * pLength
            )
{
    phStatus_t    PH_MEMLOC_REM   status;
    uint16_t      PH_MEMLOC_REM   wBytesRead = 0U;
    uint32_t      PH_MEMLOC_REM   dwNdefLen = 0U;
    uint32_t      PH_MEMLOC_REM   dwOffset = 0U;
    uint8_t       PH_MEMLOC_REM   bP1 = 0U;
    uint32_t      PH_MEMLOC_REM   dwP2 = 0U;
    uint8_t       PH_MEMLOC_REM   * pRxBuffer = NULL;
    uint8_t       PH_MEMLOC_REM   bNdefLenSize = 0U;
    uint8_t       PH_MEMLOC_REM   bReadFinished = PH_OFF;
    uint8_t       PH_MEMLOC_REM   bODO_En_HdrExcludeLen = PH_OFF;
    uint32_t      PH_MEMLOC_COUNT dwCount = 0U;
    uint16_t      PH_MEMLOC_REM   wReadLength = 0U;
    phalTop_T4T_t PH_MEMLOC_REM * pT4T = &pDataParams->ualTop.salTop_T4T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT4T->pAlT4TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Reset NDEF length */
    *pLength = 0U;

    /* Check if tag is in valid state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_NONE)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_INVALID_STATE, PH_COMP_AL_TOP);
    }

    /* Check for NDEF length > 0 (in initialized state NDEF length is 0) */
    if(pDataParams->bTagState == PHAL_TOP_STATE_INITIALIZED)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_EMPTY_NDEF, PH_COMP_AL_TOP);
    }

    /* Buffer allocated by user is less than NDEF message size on Tag. */
    if (pDataParams->dwNdefLength > PH_NXPNFCRDLIB_CONFIG_MAX_NDEF_DATA)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_TOP);
    }

    /* NDEF length to be read */
    dwNdefLen = pDataParams->dwNdefLength;

    /* NDEF or ENDEF length size */
    bNdefLenSize = (((pDataParams->bVno & 0xF0) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20) ?
        PHAL_TOP_T4T_NLEN_SIZE : PHAL_TOP_T4T_ENLEN_SIZE);

    /* Max read length in single command */
    wReadLength = pT4T->wMLe;

    if(((pDataParams->bVno & 0xF0U) > PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20) && (pDataParams->dwNdefLength > 0x7FFFU))
    {
        /*
         * Used to indicate ODO enable and ODO Header length to be excluded from Received data to get actual NDEF data length.
         * As per ISO/IEC 7816-4, E.2 BER-TLV length fields,
         * ODO Header length to be excluded - 3 or 4 bytes : 53 + 81 or 82 + 1 or 2 bytes length.
         * */
        if (wReadLength <= 0xFFU)
        {
            /* Max 3 Bytes: 0x53, 0x81(applicable in case of Read length greater than 0x7F) and 1 Length byte */
            bODO_En_HdrExcludeLen = 3U;
        }
        else
        {
            bODO_En_HdrExcludeLen = 4U; /* 4 Bytes: 0x53, 0x82 and 2 Length bytes */
        }
    }

    do
    {
        /* NDEF message starts from offset 0x02 or 0x04 based on NDEF message type */
        dwOffset = dwCount + bNdefLenSize;

        if (bODO_En_HdrExcludeLen == PH_OFF)
        {
            /* P1 and P2 represents offset */
            bP1 = (uint8_t)((dwOffset & 0xFF00U) >> 8U);
            dwP2 = (uint8_t)(dwOffset & 0x00FFU);
        }

       if((dwNdefLen + bODO_En_HdrExcludeLen) <= wReadLength)
       {
           wReadLength = dwNdefLen; /* Read Remaining data */

           if(bODO_En_HdrExcludeLen != PH_OFF)
           {
               /* Add Static no and Ld byte size to Read length as per ISO/IEC 7816-4, E.2 BER-TLV length fields */
               if(wReadLength <= PHAL_ISO7816_BER_TLV_L_00_7F)
               {
                   wReadLength = wReadLength + 2U; /* 2 Bytes: 0x53 and 1 Length byte */
               }
               else
               {
                   /* 3 Bytes: 0x53, 0x81 and 1 Length byte
                    * (or) 4 Bytes: 0x53, 0x82 and 2 Length bytes
                    * */
                   wReadLength = wReadLength + bODO_En_HdrExcludeLen;
               }
           }
       }

        PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoReadBinary(
            pT4T,
            (bODO_En_HdrExcludeLen == PH_OFF) ? ((uint32_t)(PH_EXCHANGE_DEFAULT)) :
                ((uint32_t)(PH_EXCHANGE_DEFAULT | PH_EXCHANGE_ODO)),
            (bODO_En_HdrExcludeLen == PH_OFF) ? (dwP2) : (dwOffset),
            bP1,
            (uint32_t)wReadLength,
            ((pT4T->bShortLenApdu == PH_ON) ? 0x00U : 0x01U),
            &pRxBuffer,
            &wBytesRead));

        if(wBytesRead != 0U)
        {
            (void)memcpy(&pData[dwCount], pRxBuffer, wBytesRead);
            dwNdefLen -= wBytesRead;
            dwCount += wBytesRead;
        }

        if((dwCount >= pDataParams->dwNdefLength) || (wBytesRead == 0U))
        {
            bReadFinished = PH_ON;
        }
    }while(bReadFinished != PH_ON);

    /* Return Read NDEF length */
    *pLength = dwCount;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T4T_WriteNdef(
            phalTop_Sw_DataParams_t * pDataParams,
            uint8_t * pData,
            uint32_t dwLength
            )
{
    phStatus_t    PH_MEMLOC_REM   status;
    uint32_t      PH_MEMLOC_REM   dwNdefLen = 0U;
    uint32_t      PH_MEMLOC_COUNT dwCount;
    uint32_t      PH_MEMLOC_REM   dwOffset;
    uint8_t       PH_MEMLOC_BUF   aNDEFLength[4U] = {0x00U, 0x00U, 0x00U, 0x00U };
    uint16_t      PH_MEMLOC_REM   wWriteLength = 0U;
    uint8_t       PH_MEMLOC_REM   bP1 = 0U;
    uint32_t      PH_MEMLOC_REM   dwP2;
    uint8_t       PH_MEMLOC_REM   bNdefLenSize = 0U;
    uint8_t       PH_MEMLOC_REM   bWriteFinished = PH_OFF;
    phalTop_T4T_t PH_MEMLOC_REM * pT4T = &pDataParams->ualTop.salTop_T4T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT4T->pAlT4TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Check if tag is in valid state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_NONE)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_INVALID_STATE, PH_COMP_AL_TOP);
    }

    /* Check if tag is read-only */
    if(pDataParams->bTagState == PHAL_TOP_STATE_READONLY)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_READONLY_TAG, PH_COMP_AL_TOP);
    }

    /* Validate input parameters */
    if((dwLength == 0U) ||
        ((dwLength > (pT4T->dwMaxFileSize - PHAL_TOP_T4T_NLEN_SIZE)) &&
            ((pDataParams->bVno & 0xF0U) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20)) ||
        ((dwLength > (pT4T->dwMaxFileSize - PHAL_TOP_T4T_ENLEN_SIZE)) &&
            ((pDataParams->bVno & 0xF0U) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* NDEF or ENDEF length size */
    bNdefLenSize = (((pDataParams->bVno & 0xF0U) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20) ?
        PHAL_TOP_T4T_NLEN_SIZE : PHAL_TOP_T4T_ENLEN_SIZE);

    /* Write 0 in the NLEN field */
    PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoUpdateBinary(
        pT4T,
        PH_EXCHANGE_DEFAULT,
        0x00U,
        0x00U,
        0x00U,
        aNDEFLength,
        (uint16_t)bNdefLenSize));

    /* NDEF length to be written */
    dwNdefLen = dwLength;

    /* Max write length in single command */
    wWriteLength = pT4T->wMLc;

    if(((pDataParams->bVno & 0xF0U) > PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20) &&
        (dwNdefLen > PHAL_TOP_T4T_NDEF_FILE_SIZE_MAX_MV20))
    {
        /* Write NDEF with ODO and DDO */
        pT4T->bShortLenApdu = PH_OFF;
    }

    dwCount = 0U;
    do
    {
        /* Write the NDEF message from 2nd or 4th Byte in memory based on NDEF message type */
        dwOffset = dwCount + bNdefLenSize;

        if(pT4T->bShortLenApdu == PH_ON)
        {
            bP1 = (uint8_t)((dwOffset & 0xFF00U) >> 8U);
            dwP2 = (uint8_t)(dwOffset & 0x00FFU);
        }

        if(dwNdefLen <= wWriteLength)
        {
            bWriteFinished = PH_ON;
            wWriteLength = dwNdefLen;
        }

        PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoUpdateBinary(
            pT4T,
            ((pT4T->bShortLenApdu == PH_ON) ? ((uint32_t)(PH_EXCHANGE_DEFAULT)) :
                ((uint32_t)(PH_EXCHANGE_DEFAULT | PH_EXCHANGE_ODO_DDO))),
            (pT4T->bShortLenApdu == PH_ON) ? (dwP2) : (dwOffset),
            bP1,
            ((pT4T->bShortLenApdu == PH_ON) ? 0x00U : 0x01U),
            &pData[dwCount],
            wWriteLength));

        dwNdefLen -= wWriteLength;
        dwCount += wWriteLength;
    }while(bWriteFinished != PH_ON);

    if((pDataParams->bVno & 0xF0U) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20)
    {
        /* Update NDEF length in NDEF file */
        aNDEFLength[0U] = (uint8_t)((dwLength & 0xFF00U) >> 8U);
        aNDEFLength[1U] = (uint8_t)(dwLength & 0x00FFU);
    }
    else
    {
        /* Update NDEF length in NDEF file */
        aNDEFLength[0U] = (uint8_t)((uint32_t)(dwLength & 0xFF000000U) >> 24U);
        aNDEFLength[1U] = (uint8_t)((uint32_t)(dwLength & 0x00FF0000U) >> 16U);
        aNDEFLength[2U] = (uint8_t)((uint32_t)(dwLength & 0x0000FF00U) >> 8U);
        aNDEFLength[3U] = (uint8_t)((uint32_t)(dwLength & 0x000000FFU));
    }

    /* Write the length of NDEF message in the NLEN field */
    PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoUpdateBinary(
        pT4T,
        PH_EXCHANGE_DEFAULT,
        0x00U,
        0x00U,
        0x00U,
        aNDEFLength,
        (uint16_t)bNdefLenSize));

    pDataParams->dwNdefLength = dwLength;
    pDataParams->bTagState = PHAL_TOP_STATE_READWRITE;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T4T_EraseNdef(phalTop_Sw_DataParams_t * pDataParams)
{
    phStatus_t    PH_MEMLOC_REM status;
    uint8_t       PH_MEMLOC_BUF aNDEFLength[4U] = {0x00U, 0x00U, 0x00U, 0x00U };
    phalTop_T4T_t PH_MEMLOC_REM * pT4T = &pDataParams->ualTop.salTop_T4T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT4T->pAlT4TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Check if tag is in valid state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_NONE)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_INVALID_STATE, PH_COMP_AL_TOP);
    }

    /* Check if tag is read-only */
    if(pDataParams->bTagState == PHAL_TOP_STATE_READONLY)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_READONLY_TAG, PH_COMP_AL_TOP);
    }

    /* Check if tag is already in initialized state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_INITIALIZED)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_EMPTY_NDEF, PH_COMP_AL_TOP);
    }

    /* Set NDEF length as 0 (i.e. initialized state) */
    PH_CHECK_SUCCESS_FCT(status, phalIso7816_Sw_IsoUpdateBinary(
        pT4T,
        PH_EXCHANGE_DEFAULT,
        0x00U,
        0x00U,
        0x00U,
        aNDEFLength,
        (uint16_t)(((pDataParams->bVno & 0xF0) == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20) ?
            PHAL_TOP_T4T_NLEN_SIZE : PHAL_TOP_T4T_ENLEN_SIZE)));

    /* Update state variables */
    pDataParams->bTagState = PHAL_TOP_STATE_INITIALIZED;
    pDataParams->dwNdefLength = 0U;

    return PH_ERR_SUCCESS;
}

#endif /* NXPBUILD__PHAL_TOP_T4T_SW */
#endif /* NXPBUILD__PHAL_TOP_SW */
