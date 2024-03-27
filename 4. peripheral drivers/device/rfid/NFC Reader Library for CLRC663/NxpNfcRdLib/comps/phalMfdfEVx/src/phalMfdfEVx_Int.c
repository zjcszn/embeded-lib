/*----------------------------------------------------------------------------*/
/* Copyright 2014-2020,2023 NXP                                               */
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

#ifdef NXPBUILD__PHAL_MFDFEVX

#include <string.h>
#include <phpalMifare.h>
#include <phalMfdfEVx.h>
#include "phalMfdfEVx_Int.h"
#include <ph_RefDefs.h>

phStatus_t phalMfdfEVx_Int_ComputeErrorResponse(void * pDataParams, uint16_t wStatus)
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCmdCode = 0;

    PH_CHECK_SUCCESS_FCT(status, phalMfdfEVx_Int_GetCmdCode(pDataParams, &bCmdCode));

    switch (wStatus)
    {
        case PHAL_MFDFEVX_RESP_OPERATION_OK:
        case PHAL_MFDFEVX_ISO7816_SUCCESS:
        case PHAL_MFDFEVX_RESP_OK:
            status = PH_ERR_SUCCESS;
            break;

        case PHAL_MFDFEVX_RESP_ERR_CMD_INVALID:
            status = PHAL_MFDFEVX_ERR_CMD_INVALID;
            break;

        case PHAL_MFDFEVX_RESP_NO_CHANGES:
            if(bCmdCode == PHAL_MFDFEVX_CMD_AUTH_PDC)
                status = PHAL_MFDFEVX_ERR_FORMAT;
            else
                status = PHAL_MFDFEVX_NO_CHANGES;
            break;

        case PHAL_MFDFEVX_RESP_ERR_NOT_SUP:
            status = PHAL_MFDFEVX_ERR_NOT_SUPPORTED;
            break;

        case PHAL_MFDFEVX_RESP_ERR_OUT_OF_EEPROM_ERROR:
            status = PHAL_MFDFEVX_ERR_OUT_OF_EEPROM_ERROR;
            break;

        case PHAL_MFDFEVX_RESP_ERR_INTEGRITY_ERROR:
            status = PHAL_MFDFEVX_ERR_PICC_CRYPTO;
            break;

        case PHAL_MFDFEVX_RESP_ERR_PARAMETER_ERROR:
            status = PHAL_MFDFEVX_ERR_PARAMETER_ERROR;
            break;

        case PHAL_MFDFEVX_RESP_NO_SUCH_KEY:
            status = PHAL_MFDFEVX_ERR_NO_SUCH_KEY;
            break;

        case PHAL_MFDFEVX_RESP_ERR_LENGTH_ERROR:
            status = PH_ERR_LENGTH_ERROR;
            break;

        case PHAL_MFDFEVX_RESP_PERMISSION_DENIED:
            status = PHAL_MFDFEVX_ERR_PERMISSION_DENIED;
            break;

        case PHAL_MFDFEVX_RESP_APPLICATION_NOT_FOUND:
            status = PHAL_MFDFEVX_ERR_APPLICATION_NOT_FOUND;
            break;

        case PHAL_MFDFEVX_RESP_ERR_AUTHENTICATION_ERROR:
        case PHAL_MFDFEVX_RESP_ERR_AUTH_ERROR:
            status = PH_ERR_AUTH_ERROR;
            break;

        case PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME:
            status = PH_ERR_SUCCESS_CHAINING;
            break;

        case PHAL_MFDFEVX_RESP_ERR_BOUNDARY_ERROR:
            status = PHAL_MFDFEVX_ERR_BOUNDARY_ERROR;
            break;

        case PHAL_MFDFEVX_RESP_ERR_COMMAND_ABORTED:
            status = PHAL_MFDFEVX_ERR_COMMAND_ABORTED;
            break;

        case PHAL_MFDFEVX_RESP_ERR_COUNT:
            status = PHAL_MFDFEVX_ERR_COUNT;
            break;

        case PHAL_MFDFEVX_RESP_ERR_DUPLICATE:
            status = PHAL_MFDFEVX_ERR_DUPLICATE;
            break;

        case PHAL_MFDFEVX_RESP_ERR_FILE_NOT_FOUND:
            status = PHAL_MFDFEVX_ERR_FILE_NOT_FOUND;
            break;

        case PHAL_MFDFEVX_RESP_ERR_APPL_INTEGRITY_ERROR:
        case PHAL_MFDFEVX_RESP_ERR_PICC_INTEGRITY:
        case PHAL_MFDFEVX_RESP_ERR_EEPROM:
        case PHAL_MFDFEVX_RESP_ERR_FILE_INTEGRITY:
        case PHAL_MFDFEVX_RESP_ERR_PIC_DISABLED:
        case PHAL_MFDFEVX_RESP_ILLEGAL_COMMAND_CODE:
            status = PHAL_MFDFEVX_ERR_DF_GEN_ERROR;
            PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_SetConfig(
                pDataParams,
                PHAL_MFDFEVX_ADDITIONAL_INFO,
                wStatus));
            break;

        case PHAL_MFDFEVX_ISO7816_ERR_WRONG_LENGTH:
        case PHAL_MFDFEVX_ISO7816_ERR_WRONG_LE:
        case PHAL_MFDFEVX_ISO7816_ERR_FILE_NOT_FOUND:
        case PHAL_MFDFEVX_ISO7816_ERR_WRONG_PARAMS:
        case PHAL_MFDFEVX_ISO7816_ERR_WRONG_LC:
        case PHAL_MFDFEVX_ISO7816_ERR_NO_PRECISE_DIAGNOSTICS:
        case PHAL_MFDFEVX_ISO7816_ERR_EOF_REACHED:
        case PHAL_MFDFEVX_ISO7816_ERR_FILE_ACCESS:
        case PHAL_MFDFEVX_ISO7816_ERR_FILE_EMPTY:
        case PHAL_MFDFEVX_ISO7816_ERR_MEMORY_FAILURE:
        case PHAL_MFDFEVX_ISO7816_ERR_INCORRECT_PARAMS:
        case PHAL_MFDFEVX_ISO7816_ERR_WRONG_CLA:
        case PHAL_MFDFEVX_ISO7816_ERR_UNSUPPORTED_INS:
        case PHAL_MFDFEVX_ISO7816_ERR_LIMITED_FUNCTIONALITY_INS:
            status = PHAL_MFDFEVX_ERR_DF_7816_GEN_ERROR;
            PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_SetConfig(
                pDataParams,
                PHAL_MFDFEVX_ADDITIONAL_INFO,
                wStatus));
            break;

        case PHAL_MFDFEVX_RESP_CHAINING:
            status = PH_ERR_SUCCESS_CHAINING;
            break;

        case PHAL_MFDFEVX_RESP_OPERATION_OK_LIM:
            status = PHAL_MFDFEVX_ERR_OPERATION_OK_LIM;
            break;

        case PHAL_MFDFEVX_RESP_ERR_CMD_OVERFLOW:
            status = PHAL_MFDFEVX_ERR_CMD_OVERFLOW;
            break;

        case PHAL_MFDFEVX_RESP_ERR_CMD_GEN_FAILURE:
            status = PHAL_MFDFEVX_ERR_GEN_FAILURE;
            break;

        case PHAL_MFDFEVX_RESP_ERR_BNR:
            status = PHAL_MFDFEVX_ERR_BNR;
            break;

        default:
            status = PH_ERR_PROTOCOL_ERROR;
    }
    return PH_ADD_COMPCODE(status, PH_COMP_AL_MFDFEVX);
}

/* Should take care of wrapping and unwrapping if ISO 7816 Wrapped mode
 * Does not handle any chaining or CMAC generation/verification or encryption/decryption
 */
phStatus_t phalMfdfEVx_ExchangeCmd(void * pDataParams, void * pPalMifareDataParams, uint8_t bWrappedMode, uint8_t * pSendBuff, uint16_t wCmdLen,
    uint8_t ** ppResponse, uint16_t * pRxlen)
{
    uint16_t    PH_MEMLOC_REM wFrameLen = 0;
    uint8_t     PH_MEMLOC_REM * pRecv = NULL;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wStatusWord;
    uint8_t     PH_MEMLOC_REM pApdu[5] = { PHAL_MFDFEVX_WRAPPEDAPDU_CLA, 0x00, PHAL_MFDFEVX_WRAPPEDAPDU_P1, PHAL_MFDFEVX_WRAPPEDAPDU_P2, 0x00 };

    if (0U != (bWrappedMode))
    {
        wFrameLen = PHAL_MFDFEVX_MAXWRAPPEDAPDU_SIZE;
    }
    else
    {
        wFrameLen = PHAL_MFDFEVX_MAXDFAPDU_SIZE;
    }

    if (wCmdLen > wFrameLen)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_MFDFEVX);
    }

    /* Send the data to PICC */
    if (0U != (bWrappedMode))
    {
        pApdu[1] = pSendBuff[0]; /* DESFire cmd code in INS */
        pApdu[4] = (uint8_t)(wCmdLen) - 0x01u;

        PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            (pApdu[4] == 0x00U) ? PH_EXCHANGE_DEFAULT : PH_EXCHANGE_BUFFER_FIRST,
            pApdu,
            PHAL_MFDFEVX_WRAP_HDR_LEN,
            &pRecv,
            pRxlen));

        if (pApdu[4] != 0x00U)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp,  phpalMifare_ExchangeL4(
                pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT,
                &pSendBuff[1],
                wCmdLen - 1u,
                &pRecv,
                pRxlen));

            /* Le byte */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_LAST,
                &pApdu[2], /* Le is always zero in wrapped mode. */
                0x01,
                &pRecv,
                pRxlen));
        }
    }
    else
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            pSendBuff,
            wCmdLen,
            &pRecv,
            pRxlen));
    }
    if (0U != (bWrappedMode))
    {
        *ppResponse = pRecv;
        wStatusWord = pRecv[(*pRxlen) - 2];
        wStatusWord <<= 8U;
        wStatusWord |= pRecv[(*pRxlen) - 1];
        if((pRecv[(*pRxlen) - 2]) == 0x91U)
        {
            (*pRxlen) -= 2u;
            return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, (uint8_t) wStatusWord);
        }
        else
        {
            (*pRxlen) -= 2u;
            return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, wStatusWord);
        }
    }
    else
    {
        *ppResponse = pRecv + 1U;
        wStatusWord = pRecv[0];
        (*pRxlen) -= 1u;
        return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, (uint8_t) wStatusWord);
    }
}

phStatus_t phalMfdfEVx_Int_Send7816Apdu(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t bIns, uint8_t bP1, uint8_t bP2,
    uint32_t bLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint32_t bLe, uint8_t ** ppDataOut, uint16_t *pDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp = PH_ERR_SUCCESS;
    uint8_t     PH_MEMLOC_REM bCmdBuff[20];
    uint16_t    PH_MEMLOC_REM wCmdLen = 0;

    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Int_SetCmdCode(pDataParams, bIns));

    bCmdBuff[wCmdLen++] = 0x00; /* Class is always 0x00 */
    bCmdBuff[wCmdLen++] = bIns;
    bCmdBuff[wCmdLen++] = bP1;
    bCmdBuff[wCmdLen++] = bP2;

    switch(bIns)
    {
        case PHAL_MFDFEVX_CMD_ISO7816_SELECT_FILE:
            /* Both Lc and Le are required */
            statusTmp = phalMfdfEVx_Int_ISOSelectFile(pDataParams, pPalMifareDataParams, bOption, bCmdBuff, wCmdLen,
                (uint8_t)bLc, bExtendedLenApdu, pDataIn, (uint8_t)bLe, ppDataOut, pDataLen);
            break;

        case PHAL_MFDFEVX_CMD_ISO7816_UPDATE_BINARY:
            statusTmp = phalMfdfEVx_Int_ISOUpdateBinary(pDataParams, pPalMifareDataParams, bOption, bCmdBuff, wCmdLen,
                bLc, bExtendedLenApdu, pDataIn, ppDataOut, pDataLen);
            break;

        case PHAL_MFDFEVX_CMD_ISO7816_APPEND_RECORD:
            /* Le is not required since no response data expected */
            statusTmp = phalMfdfEVx_Int_ISOAppendRecord(pDataParams, pPalMifareDataParams, bOption, bCmdBuff, wCmdLen,
                bLc, bExtendedLenApdu, pDataIn, ppDataOut, pDataLen);
            break;

        case PHAL_MFDFEVX_CMD_ISO7816_GET_CHALLENGE:
            statusTmp = phalMfdfEVx_Int_ISOGetChallenge(pDataParams, pPalMifareDataParams, bOption, bCmdBuff, wCmdLen,
                (uint8_t)bLe, bExtendedLenApdu, ppDataOut, pDataLen);
            break;

        case PHAL_MFDFEVX_CMD_ISO7816_EXT_AUTHENTICATE:
            /* Le is not required since no response data expected. bLc however will be 1 byte since it is framed as a part of
             * input buffer and sent it from AL. All other commands are sent as type uint32_t from AL, but here bLc is sent in uint8_t
             */
            statusTmp = phalMfdfEVx_Int_ISOExternalAuthenticate(pDataParams, pPalMifareDataParams, bOption, bCmdBuff, wCmdLen,
                (uint8_t)bLc, bExtendedLenApdu, pDataIn, ppDataOut, pDataLen);
            break;

        case PHAL_MFDFEVX_CMD_ISO7816_INT_AUTHENTICATE:
            statusTmp = phalMfdfEVx_Int_ISOInternalAuthenticate(pDataParams, pPalMifareDataParams, bOption, bCmdBuff, wCmdLen,
                (uint8_t)bLc, bExtendedLenApdu, pDataIn, bLe, ppDataOut, pDataLen);
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AL_MFDFEVX);
    }
    return statusTmp;
}

phStatus_t phalMfdfEVx_Int_ISOSelectFile(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint8_t bLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint8_t bLe, uint8_t ** ppDataOut, uint16_t *pDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    phStatus_t  PH_MEMLOC_REM statusTmp1;
    uint16_t    PH_MEMLOC_REM wRxlen;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint16_t    wVal = 0;

    /* Check whether Lc is passed */
    if (0U != (bOption & 0x01U))
    {
        /* Check whether Length Lc is represented in short APDU or extended APDU */
        if(bExtendedLenApdu == 0x01U)
        {
            /* Extended APDU */
            bCmdBuff[wCmdLen++] = 0x00; /* first byte will be 0x00 if Ext apdu present. Next 2 byte contains actual data */
            bCmdBuff[wCmdLen++] = 0x00; /* As of now this field will be set to 0x00 since maximum data that can be sent is 16 bytes. In case if data to be sent exceeds 255 bytes, this byte shall be used  */
            bCmdBuff[wCmdLen++] = bLc;  /* Actual data to be sent */
        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = bLc;
        }
    }
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    if ((0U != ((bOption & 0x01U))) && (bLc > 0U))
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_CONT,
            pDataIn,
            bLc,
            &pRecv,
            &wRxlen
            ));
    }
    wCmdLen = 0;

     /* As per ISO/IEC:7816-4(2005), Section 5, An extended Le field consists of either three bytes
     * (one byte set to '00' followed by two bytes with any
     * value) if the Lc field is absent, or two bytes (with any value) if an extended Lc field is present.
     */
    if (0U != (bOption & 0x02U))
    {
        /* Check whether Length is represented in extended APDU format and Lc is present.
         * If true, then Le should represented in 2 bytes else Le should be represented in 3 bytes
         */
        if(bExtendedLenApdu == 0x01U)
        {
            /* Extended APDU */
            if(0U != (bOption & 0x01U))
            {
                bCmdBuff[wCmdLen++] = 0x00;
                bCmdBuff[wCmdLen++] = bLe;
            }
            else
            {
                bCmdBuff[wCmdLen++] = 0x00;
                bCmdBuff[wCmdLen++] = 0x00;
                bCmdBuff[wCmdLen++] = bLe;
            }
        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = bLe;
        }
    }
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    statusTmp = pRecv[wRxlen - 2u];
    statusTmp <<= 8U;
    statusTmp |= pRecv[wRxlen - 1u];

    statusTmp = phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, statusTmp);

    if((statusTmp & PH_ERR_MASK) == PHAL_MFDFEVX_ERR_DF_7816_GEN_ERROR)
    {
        PH_CHECK_SUCCESS_FCT( statusTmp1, phalMfdfEVx_GetConfig(pDataParams, PHAL_MFDFEVX_ADDITIONAL_INFO, &wVal));
    }

    /*  Check for Success and for LIMITED FUNCTIONALITY error. In both cases, FCI would be returned */
    if((statusTmp == PH_ERR_SUCCESS) || (wVal == PHAL_MFDFEVX_ISO7816_ERR_LIMITED_FUNCTIONALITY_INS))
    {
        if (pDataLen != NULL)
        {
            *pDataLen = wRxlen -2u;
        }
        if (ppDataOut != NULL)
        {
            *ppDataOut = pRecv;
        }
    }
    else
    {
        return statusTmp;
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Int_ISOUpdateBinary(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint32_t dwLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint8_t ** ppDataOut, uint16_t *pDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wRxlen;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint8_t     PH_MEMLOC_REM *bDataLenPtr;
    /* Initialise LcPtr */
    bDataLenPtr = (uint8_t *)&dwLc;
    /* Check whether Lc has to be passed */
    if (0U != (bOption & 0x01U))
    {
        /* Check whether Length Lc is represented in short APDU or extended APDU */
        if(bExtendedLenApdu == 0x01U)
        {
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
            bCmdBuff[wCmdLen++] = 0x00;
            bCmdBuff[wCmdLen++] = *(bDataLenPtr + 1U);
            bCmdBuff[wCmdLen++] = *(bDataLenPtr);
            /* Note: Need to handle the case where data to write is more than 0xFFFF(i.e 65535 bytes).
            */
        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = *bDataLenPtr;
        }
    }
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    if ((0U != ((bOption & 0x01U))) && (dwLc > 0U))
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_CONT,
            pDataIn,
            (uint16_t)dwLc,
            &pRecv,
            &wRxlen
            ));
    }
    wCmdLen = 0;
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    statusTmp = pRecv[wRxlen - 2u];
    statusTmp <<= 8U;
    statusTmp |= pRecv[wRxlen - 1u];
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, statusTmp));
    if (pDataLen != NULL)
    {
        *pDataLen = wRxlen -2u;
    }
    if (ppDataOut != NULL)
    {
        *ppDataOut = pRecv;
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Int_ISOAppendRecord(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint32_t dwLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint8_t ** ppDataOut, uint16_t *pDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wRxlen;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint8_t     PH_MEMLOC_REM *bDataLenPtr;
    /* Initialise LcPtr */
    bDataLenPtr = (uint8_t *)&dwLc;
    if (0U != (bOption & 0x01U))
    {
        /* Check whether Length Lc is represented in short APDU or extended APDU */
        if(bExtendedLenApdu == 0x01U)
        {
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
            bCmdBuff[wCmdLen++] = 0x00;
            bCmdBuff[wCmdLen++] = *(bDataLenPtr + 1U);
            bCmdBuff[wCmdLen++] = *(bDataLenPtr);
            /* Note: Need to handle the case where data to write is more than 0xFFFF(i.e 65535 bytes).*/
        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = *bDataLenPtr;
        }
    }
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    if ((0U != ((bOption & 0x01U))) && (dwLc > 0U))
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_CONT,
            pDataIn,
            (uint16_t)dwLc,
            &pRecv,
            &wRxlen
            ));
    }
    wCmdLen = 0;
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    statusTmp = pRecv[wRxlen - 2u];
    statusTmp <<= 8U;
    statusTmp |= pRecv[wRxlen - 1u];
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, statusTmp));
    if (pDataLen != NULL)
    {
        *pDataLen = wRxlen -2u;
    }
    if (ppDataOut != NULL)
    {
        *ppDataOut = pRecv;
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Int_ISOGetChallenge(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint8_t bLe, uint8_t bExtendedLenApdu, uint8_t ** ppDataOut, uint16_t *pDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wRxlen;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;

    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    wCmdLen = 0;
    if (0U != (bOption & 0x02U))
   {
        /* Check whether Length Le is represented in short APDU(1B) or extended APDU(3B) */
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
             bCmdBuff[wCmdLen++] = 0x00; /* This byte is set to 0x00 since maximum bytes received will be either 0x08 or 0x10 as per ref arch ver 0.14*/
             bCmdBuff[wCmdLen++] = bLe;

        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = bLe;
        }
    }
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    statusTmp = pRecv[wRxlen - 2u];
    statusTmp <<= 8U;
    statusTmp |= pRecv[wRxlen - 1u];
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, statusTmp));
    if (pDataLen != NULL)
    {
        *pDataLen = wRxlen -2u;
    }
    if (ppDataOut != NULL)
    {
        *ppDataOut = pRecv;
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Int_ISOExternalAuthenticate(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint8_t bLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint8_t ** ppDataOut, uint16_t *pDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wRxlen;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;

    /* Check if Lc is passed */
    if (0U != (bOption & 0x01U))
    {
        /* Check whether Length Lc is represented in short APDU or extended APDU */
        if(bExtendedLenApdu == 0x01U)
        {
            /* Extended APDU */
            bCmdBuff[wCmdLen++] = 0x00; /* first byte will be 0x00 if Ext apdu present. Next 2 byte contains actual data */
            bCmdBuff[wCmdLen++] = 0x00; /* As of now this field will be set to 0x00 since maximum data that can be sent is 16 bytes or 32 bytes. In case if data to be sent exceeds 255 bytes, this byte shall be used  */
            bCmdBuff[wCmdLen++] = bLc;  /* Actual data to be sent */
        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = bLc;
        }
    }
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    if ((0U != ((bOption & 0x01U))) && (bLc > 0U))
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_CONT,
            pDataIn,
            (uint16_t)bLc,
            &pRecv,
            &wRxlen
            ));
    }
    wCmdLen = 0;
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    statusTmp = pRecv[wRxlen - 2u];
    statusTmp <<= 8U;
    statusTmp |= pRecv[wRxlen - 1u];
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, statusTmp));
    if (pDataLen != NULL)
    {
        *pDataLen = wRxlen -2u;
    }
    if (ppDataOut != NULL)
    {
        *ppDataOut = pRecv;
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Int_ISOInternalAuthenticate(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint8_t bLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint32_t dwLe, uint8_t ** ppDataOut, uint16_t *pDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wRxlen;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint8_t     PH_MEMLOC_REM *bLePtr;

    /* Initialize LcPtr and LePtr */
    bLePtr = (uint8_t *)&dwLe;

    /* Handle for 3 bytes Lc */
    if (0U != (bOption & 0x01U))
    {
        /* Check whether Length Lc is represented in short APDU or extended APDU */
        if(bExtendedLenApdu == 0x01U)
        {
            bCmdBuff[wCmdLen++] = 0x00; /* first byte will be 0x00 if Ext apdu present. Next 2 byte contains actual data */
            bCmdBuff[wCmdLen++] = 0x00; /* As of now this field will be set to 0x00 since maximum data that can be sent is 8 bytes or 16 bytes. In case if data to be sent exceeds 255 bytes, this byte shall be used  */
            bCmdBuff[wCmdLen++] = bLc;  /* Actual data to be sent */
        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = bLc;
        }
    }
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    if ((0U != ((bOption & 0x01U))) && (bLc > 0U))
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_CONT,
            pDataIn,
            (uint16_t)bLc,
            &pRecv,
            &wRxlen
            ));
    }
    wCmdLen = 0;
    /* As per ISO/IEC:7816-4(2005), Section 5, An extended Le field consists of either three bytes
     * (one byte set to '00' followed by two bytes with any
     * value) if the Lc field is absent, or two bytes (with any value) if an extended Lc field is present.
     */
    if (0U != (bOption & 0x02U))
    {
        /* Check whether Length is represented in extended APDU format and Lc is present.
         * If true, then Le should represented in 2 bytes else Le should be represented in 3 bytes
         */
         if(bExtendedLenApdu == 0x01U)
        {
            /* Check if Lc is present */
            if(0U != (bOption & 0x01U))
        {
            bCmdBuff[wCmdLen++] = *(bLePtr + 1U);
            bCmdBuff[wCmdLen++] = *bLePtr;
        }
            else
        {
                bCmdBuff[wCmdLen++] = 0x00;
                bCmdBuff[wCmdLen++] = *(bLePtr + 1U);
                bCmdBuff[wCmdLen++] = *(bLePtr);
            }
        }
        else
        {
            /* Short APDU */
            bCmdBuff[wCmdLen++] = *(bLePtr);
        }
    }
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        bCmdBuff,
        wCmdLen,
        &pRecv,
        &wRxlen
        ));
    statusTmp = pRecv[wRxlen - 2u];
    statusTmp <<= 8U;
    statusTmp |= pRecv[wRxlen - 1u];
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, statusTmp));
    if (pDataLen != NULL)
    {
        *pDataLen = wRxlen -2u;
    }
    if (ppDataOut != NULL)
    {
        *ppDataOut = pRecv;
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Int_GetCmdCode(void * pDataParams, uint8_t * pCmdCode)
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;

    switch(PH_GET_COMPID(pDataParams))
    {
        case PHAL_MFDFEVX_SW_ID:
            *pCmdCode = ((phalMfdfEVx_Sw_DataParams_t *)pDataParams)->bCmdCode;
            break;



        default:
            wStatus = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    return wStatus;
}

phStatus_t phalMfdfEVx_Int_SetCmdCode(void * pDataParams, uint8_t bCmdCode)
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;

    switch(PH_GET_COMPID(pDataParams))
    {
        case PHAL_MFDFEVX_SW_ID:
            ((phalMfdfEVx_Sw_DataParams_t *)pDataParams)->bCmdCode = bCmdCode;
            break;

        default:
            wStatus = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    return wStatus;
}
#endif /* NXPBUILD__PHAL_MFDFEVX */
