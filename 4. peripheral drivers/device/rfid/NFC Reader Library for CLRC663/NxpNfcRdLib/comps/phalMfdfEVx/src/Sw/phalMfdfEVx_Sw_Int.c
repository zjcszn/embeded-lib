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
#include <phpalMifare.h>
#include <phpalI14443p4.h>
#include <ph_RefDefs.h>
#include <ph_TypeDefs.h>
#include <phTools.h>
#include <string.h>
#ifdef NXPBUILD__PH_CRYPTOSYM
#include <phCryptoSym.h>
#endif /* NXPBUILD__PH_CRYPTOSYM */
#include <phKeyStore.h>
#include <phTMIUtils.h>
#include <phalVca.h>

#ifdef NXPBUILD__PHAL_MFDFEVX_SW

#include "../phalMfdfEVx_Int.h"
#include "phalMfdfEVx_Sw_Int.h"

phStatus_t phalMfdfEVx_Sw_Int_CardExchange(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wBufferOption, uint16_t wTotDataLen,
    uint8_t bExchangeLE, uint8_t * pData, uint16_t wDataLen, uint8_t ** ppResponse, uint16_t * pRespLen, uint8_t * pPiccRetCode)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint16_t    PH_MEMLOC_REM wLc = 0;
    uint8_t *   PH_MEMLOC_REM pResponse;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    uint8_t         PH_MEMLOC_REM aLc[1] = { 0x00 };
    uint8_t         PH_MEMLOC_REM aLe[1] = { 0x00 };
    uint8_t         PH_MEMLOC_REM bLcLen = 0;
    uint8_t         PH_MEMLOC_REM aISO7816Header[8] = { PHAL_MFDFEVX_WRAPPEDAPDU_CLA, 0x00, PHAL_MFDFEVX_WRAPPEDAPDU_P1, PHAL_MFDFEVX_WRAPPEDAPDU_P2 };
    uint8_t         PH_MEMLOC_REM bISO7816HeaderLen = 4;
    uint8_t         PH_MEMLOC_REM bIsIsoChainnedCmd = PH_OFF;
    static uint8_t  PH_MEMLOC_REM bLeLen;

    /* Exchange the command in Iso7816 wrapped formmat. ----------------------------------------------------------------- */
    if (0U != (pDataParams->bWrappedMode))
    {
        if ((wBufferOption == PH_EXCHANGE_BUFFER_FIRST) || (wBufferOption == PH_EXCHANGE_DEFAULT))
        {
            /* Set the flag for data operation commands. */
            bIsIsoChainnedCmd = (uint8_t) (((pData[0] == PHAL_MFDFEVX_CMD_READ_DATA_ISO) || (pData[0] == PHAL_MFDFEVX_CMD_READ_RECORDS_ISO) ||
                (pData[0] == PHAL_MFDFEVX_CMD_WRITE_DATA_ISO) || (pData[0] == PHAL_MFDFEVX_CMD_WRITE_RECORD_ISO) ||
                (pData[0] == PHAL_MFDFEVX_CMD_UPDATE_RECORD_ISO)) ? PH_ON : PH_OFF);

            bLeLen = 1;

            /* Set the LC information. */
            wLc = (uint16_t) (wTotDataLen - 1 /* Excluding the command code. */);

            /* Update the command code to Iso7816 header */
            aISO7816Header[1] = pData[0];

            /* Add the ISO 7816 header to layer 4 buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_FIRST,
                &aISO7816Header[0],
                bISO7816HeaderLen,
                NULL,
                NULL));

            /* Add Lc if available */
            if (wLc)
            {
                /* Update Lc bytes */
                aLc[bLcLen++] = (uint8_t) (wLc & 0x00FF);

                /* Add the Lc to layer 4 buffer. */
                PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_CONT,
                    &aLc[0],
                    bLcLen,
                    NULL,
                    NULL));

                /* Add the data to layer 4 buffer. */
                PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_CONT,
                    &pData[1],  /* Exclude the command code because it is added to INS. */
                    (uint16_t) (wDataLen - 1),
                    NULL,
                    NULL));
            }
            else
            {
                ;
            }
        }

        if (wBufferOption == PH_EXCHANGE_BUFFER_CONT)
        {
            /* Add the data to layer 4 buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT,
                pData,
                wDataLen,
                NULL,
                NULL));
        }

        if ((wBufferOption == PH_EXCHANGE_BUFFER_LAST) || (wBufferOption == PH_EXCHANGE_DEFAULT))
        {
            if (wBufferOption == PH_EXCHANGE_BUFFER_LAST)
            {
                /* Add the data to layer 4 buffer. */
                PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_CONT,
                    pData,
                    wDataLen,
                    NULL,
                    NULL));
            }

            /* Add Le to L4 buffer and exchange the command. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_LAST,
                &aLe[0],
                (uint8_t) (bExchangeLE ? bLeLen : 0),
                &pResponse,
                &wRespLen));

            /* Evaluate the response. */
            PH_CHECK_SUCCESS_FCT(wStatus, phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, pResponse[wRespLen - 1U]));

            /* Create memory for updating the response of ISO 14443 format. */
            *ppResponse = pResponse;

            /* Update the response buffer length excluding SW1SW2. */
            *pRespLen = wRespLen - 2U;

            /* Copy the second byte of response (SW2) to RxBuffer */
            *pPiccRetCode = pResponse[wRespLen - 1U];
        }

        if (wBufferOption == PH_EXCHANGE_RXCHAINING)
        {
            /* Exchange the command */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                wBufferOption,
                pData,
                wDataLen,
                &pResponse,
                &wRespLen));

            /* Evaluate the response. */
            PH_CHECK_SUCCESS_FCT(wStatus, phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, pResponse[wRespLen - 1]));

            /* Create memory for updating the response of ISO 14443 format. */
            *ppResponse = pResponse;

            /* Update the response buffer length excluding SW1SW2. */
            *pRespLen = wRespLen - 2U;

            /* Copy the second byte of response (SW2) to RxBuffer */
            *pPiccRetCode = pResponse[wRespLen - 1U];
        }
    }

    /* Exchange the command in Native formmat. -------------------------------------------------------------------------- */
    else
    {
        /* Exchange the data to the card in Native format. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            wBufferOption,
            pData,
            wDataLen,
            &pResponse,
            &wRespLen));

        /* Evaluate the response. */
        if ((wBufferOption == PH_EXCHANGE_BUFFER_LAST) || (wBufferOption == PH_EXCHANGE_DEFAULT))
        {
            PH_CHECK_SUCCESS_FCT(wStatus, phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, pResponse[0]));

            /* Create memory for updating the response of ISO 14443 format. */
            *ppResponse = &pResponse[1];

            /* Update the response buffer length excluding SW1. */
            *pRespLen = wRespLen - 1U;

            /* Copy the second byte of response (SW2) to RxBuffer */
            *pPiccRetCode = pResponse[0];
        }
    }

     PH_UNUSED_VARIABLE(bIsIsoChainnedCmd);
    return wStatus;
}

phStatus_t phalMfdfEVx_Sw_Int_GetData(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pSendBuff, uint16_t wCmdLen,
    uint8_t ** pResponse, uint16_t * pRxlen)
{
    uint16_t   PH_MEMLOC_REM wOption;
    uint8_t    PH_MEMLOC_REM * pRecv = NULL;
    phStatus_t PH_MEMLOC_REM statusTmp = 0;
    uint8_t    PH_MEMLOC_REM bStatusByte = 0xFF;
    uint8_t    PH_MEMLOC_REM bCmdBuff[10];
    uint8_t    PH_MEMLOC_REM bBackupByte = 0;
    uint16_t   PH_MEMLOC_REM wNextPos = 0;
    uint16_t   PH_MEMLOC_REM wRxBufferSize = 0;
    uint8_t    PH_MEMLOC_REM bBackupBytes[3];
    uint8_t    PH_MEMLOC_REM pApdu[5] = { PHAL_MFDFEVX_WRAPPEDAPDU_CLA, 0x00, PHAL_MFDFEVX_WRAPPEDAPDU_P1, PHAL_MFDFEVX_WRAPPEDAPDU_P2, 0x00 };
    uint8_t    PH_MEMLOC_REM bBackUpByte;
    uint8_t    PH_MEMLOC_REM bBackUpByte1;
    uint16_t   PH_MEMLOC_REM wBackUpLen;
    uint8_t    PH_MEMLOC_REM bIvLen = 0;
    uint16_t   PH_MEMLOC_REM wTmp = 0;

    /* Status and two other bytes to be backed up before getting new frame of data */
    (void)memset(bBackupBytes, 0x00, 3);

    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
        pDataParams->pHalDataParams,
        PHHAL_HW_CONFIG_RXBUFFER_STARTPOS,
        &wTmp
    ));

    wOption = PH_EXCHANGE_DEFAULT;
    if (0U != (pDataParams->bWrappedMode))
    {
        if (wCmdLen > PHAL_MFDFEVX_MAXWRAPPEDAPDU_SIZE)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_MFDFEVX);
        }

        pApdu[1] = pSendBuff[0];  /* DESFire command code. */
        /* Encode APDU Length*/
        pApdu[4] = (uint8_t) wCmdLen - 1u; /* Set APDU Length. */

        statusTmp = phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            (pApdu[4] == 0x00U) ? PH_EXCHANGE_DEFAULT : PH_EXCHANGE_BUFFER_FIRST,
            pApdu,
            PHAL_MFDFEVX_WRAP_HDR_LEN,
            &pRecv,
            pRxlen
        );
        if ((pApdu[4] != 0x00U) && (statusTmp == PH_ERR_SUCCESS))
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT,
                &pSendBuff[1],
                wCmdLen - 1u,
                &pRecv,
                pRxlen
            ));

            statusTmp = phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_LAST,
                &pApdu[2],
                0x01,
                &pRecv,
                pRxlen
            );
        }
        /* To handle the case where the card returns only status 91 and returns
        AF in the next frame */
        if ((statusTmp & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING)
        {
            if (((pDataParams->bWrappedMode) && (*pRxlen == 2U)) ||
                ((!(pDataParams->bWrappedMode)) && (*pRxlen == 1U)))
            {
                /* AF should always be accompanied by data. Otherwise
                it is a protocol error */
                return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
            }
            /* One more status byte to read from DesFire */
            bBackUpByte = pRecv[0];
            bBackUpByte1 = pRecv[1];
            wBackUpLen = *pRxlen;

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_RXCHAINING,
                &pApdu[2],
                0x01,
                &pRecv,
                pRxlen
            ));

            /* Received length can be one or two Ex: 0x91 0xAF */
            if (*pRxlen == 2U)
            {
                pRecv[wBackUpLen] = pRecv[0];
                pRecv[wBackUpLen + 1U] = pRecv[1];
                bStatusByte = pRecv[1];
            }
            else if (*pRxlen == 1U)
            {
                bStatusByte = pRecv[0];
                pRecv[wBackUpLen] = bStatusByte;
            }
            else
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
            }

            *pRxlen = wBackUpLen + *pRxlen;

            /* Set back the backed up bytes */
            pRecv[0] = bBackUpByte;
            pRecv[1] = bBackUpByte1;
        }
        else
        {
            if (statusTmp != PH_ERR_SUCCESS)
            {
                return statusTmp;
            }
        }
    }
    else
    {
        /* Normal mode */
        if (wCmdLen > PHAL_MFDFEVX_MAXDFAPDU_SIZE)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_MFDFEVX);
        }

        /* Send this on L4 */
        PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            wOption,
            pSendBuff,
            wCmdLen,
            &pRecv,
            pRxlen
        ));
    }

    /* Storing the original pointer */
    *pResponse = pRecv;

    /* Status is 0xAF or 0x00? */
    if (*pRxlen > 0x0000U)
    {
        if (0U != (pDataParams->bWrappedMode))
        {
            bStatusByte = (*pResponse)[(*pRxlen) - 1];
        }
        else
        {
            bStatusByte = (*pResponse)[wTmp];
        }
    }

    if (bStatusByte == PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME)
    {
        if (((pDataParams->bWrappedMode) && (*pRxlen == 2U)) ||
            ((!(pDataParams->bWrappedMode)) && (*pRxlen == 1U)))
        {
            /* AF should always be accompanied by data. Otherwise
            it is a protocol error */
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }

        if (0U != (pDataParams->bWrappedMode))
        {
            /* Next position will ensure overwriting on the
            SW1SW2 received from previous command */
            wNextPos = (*pRxlen) - 2U;
            (void)memcpy(bBackupBytes, &(*pResponse)[wNextPos - 3u], 3);
        }
        else
        {
            /* Backup the last byte */
            bBackupByte = (*pResponse)[(*pRxlen - 1)];
            (void)memcpy(bBackupBytes, &(*pResponse)[(*pRxlen - 3)], 3);
            wNextPos = (*pRxlen) - 1U;
        }

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
            pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_RXBUFFER_BUFSIZE,
            &wRxBufferSize
        ));

        if (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES)
        {
            bIvLen = 16;
        }
        else if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
            (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATE))
        {
            bIvLen = 8;
        }
        else
        {
            bIvLen = 0;
        }
    }

    while (bStatusByte == PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME)
    {
        if (((pDataParams->bWrappedMode) && (*pRxlen == 2U)) ||
            ((!(pDataParams->bWrappedMode)) && (*pRxlen == 1U)))
        {
            /* AF should always be accompanied by data. Otherwise
            it is a protocol error */
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }
        if ((wNextPos + PHAL_MFDFEVX_MAX_FRAME_SIZE) >= wRxBufferSize)
        {
            /* Return 0xAF and let the caller recall the function with
            option = PH_EXCHANGE_RXCHAINING */
            /* Return the data accumulated till now and its length */
            if (0U != (pDataParams->bWrappedMode))
            {
                (*pRxlen) -= 2u;
            }
            else
            {
                (*pRxlen) -= 1u;
                (*pResponse)++;
            }
            return PH_ADD_COMPCODE_FIXED(PH_ERR_SUCCESS_CHAINING, PH_COMP_AL_MFDFEVX);
        }
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_RXBUFFER_STARTPOS,
            wNextPos
        ));

        bCmdBuff[0] = PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME;
        wCmdLen = 1;
        if ( 0U != (pDataParams->bWrappedMode) )
        {
            pApdu[1] = bCmdBuff[0];  /* DESFire command code. */
            /* Encode APDU Length*/
            pApdu[4] = (uint8_t) wCmdLen - 1u; /* Set APDU Length. */

            statusTmp = phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                (pApdu[4] == 0x00U) ? PH_EXCHANGE_DEFAULT : PH_EXCHANGE_BUFFER_FIRST,
                pApdu,
                PHAL_MFDFEVX_WRAP_HDR_LEN,
                &pRecv,
                pRxlen
            );
            if ((pApdu[4] != 0x00U) && (statusTmp == PH_ERR_SUCCESS))
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_CONT,
                    bCmdBuff,
                    wCmdLen,
                    &pRecv,
                    pRxlen
                ));

                bCmdBuff[0] = 0x00; /* Le */
                statusTmp = phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_LAST,
                    bCmdBuff,
                    0x01,
                    &pRecv,
                    pRxlen
                );
            }
            /* To handle the case where the card returns only status 91 and returns
            AF in the next frame */
            if ((statusTmp & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING)
            {
                /* One or two more status bytes to read from DesFire */
                bBackUpByte = pRecv[0];
                bBackUpByte1 = pRecv[1];
                wBackUpLen = *pRxlen;

                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_RXCHAINING,
                    &pApdu[2],
                    0x01,
                    &pRecv,
                    pRxlen
                ));

                /* Received length can be one or two Ex: 0x91 0xAF */
                if (*pRxlen == 2U)
                {
                    pRecv[wBackUpLen] = pRecv[0];
                    pRecv[wBackUpLen + 1U] = pRecv[1];
                    bStatusByte = pRecv[1];
                }
                else if (*pRxlen == 1U)
                {
                    bStatusByte = pRecv[0];
                    pRecv[wBackUpLen] = bStatusByte;
                }
                else
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
                }

                *pRxlen = wBackUpLen + *pRxlen;

                /* Set back the backed up bytes */
                pRecv[0] = bBackUpByte;
                pRecv[1] = bBackUpByte1;
            }
            else
            {
                if (statusTmp != PH_ERR_SUCCESS)
                {
                    return statusTmp;
                }
            }
        }
        else
        {
            /* Send this on L4 */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                wOption,
                bCmdBuff,
                wCmdLen,
                &pRecv,
                pRxlen
            ));
        }

        /* Update wNextPos */
        if (0U != (pDataParams->bWrappedMode))
        {
            bStatusByte = (*pResponse)[(*pRxlen) - 1];

            /* Putback the backed up bytes */
            (void)memcpy(&(*pResponse)[wNextPos - 3u], bBackupBytes, 3);

            wNextPos = (*pRxlen) - 2U;
            (void)memcpy(bBackupBytes, &(*pResponse)[wNextPos - 3u], 3);
        }
        else
        {
            bStatusByte = (*pResponse)[wNextPos];

            /* Put back the previously backedup byte */
            (*pResponse)[wNextPos] = bBackupByte;

            /* Putback the backed up bytes */
            (void)memcpy(&(*pResponse)[wNextPos - 2u], bBackupBytes, 3);

            wNextPos = (*pRxlen) - 1U;
            bBackupByte = (*pResponse)[wNextPos];

            /* Backup 3 bytes. The nxt frame will overwrite these */
            (void)memcpy(bBackupBytes, &(*pResponse)[wNextPos - 2u], 3);
        }
    }
    if (0U != (pDataParams->bWrappedMode))
    {
        (*pRxlen) -= 2u;
    }
    else
    {
        (*pRxlen) -= 1u;
        (*pResponse)++;
    }

    /* satisfy compiler */
    PH_UNUSED_VARIABLE (bIvLen);

    return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, bStatusByte);
}

phStatus_t phalMfdfEVx_Sw_Int_ISOGetData(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pSendBuff, uint16_t wCmdLen,
    uint8_t ** pResponse, uint16_t * pRxlen)
{
    uint16_t   PH_MEMLOC_REM wOption;
    uint8_t    PH_MEMLOC_REM * pRecv = NULL;
    phStatus_t PH_MEMLOC_REM statusTmp = 0;
    phStatus_t PH_MEMLOC_REM status = 0;
    uint8_t    PH_MEMLOC_REM bCmdBuff[10];
    uint8_t    PH_MEMLOC_REM bApduLen = 4;  /* Initializing with 4 since Length of the Data(Lc) starts from 4th element of pApdu[] */
    uint16_t   PH_MEMLOC_REM wNextPos = 0;
    uint16_t   PH_MEMLOC_REM wRxBufferSize = 0;
    uint8_t    PH_MEMLOC_REM bBackupBytes[3];
    uint8_t    PH_MEMLOC_REM pApdu[7] = { PHAL_MFDFEVX_WRAPPEDAPDU_CLA, 0x00, PHAL_MFDFEVX_WRAPPEDAPDU_P1, PHAL_MFDFEVX_WRAPPEDAPDU_P2, 0x00,/* Extended Length Apdu */ 0x00, 0x00 };
    uint8_t    PH_MEMLOC_REM bLe[2] = { 0x00, 0x00 };
    uint8_t    PH_MEMLOC_REM bExtendedLenApdu = 0;
    uint16_t    PH_MEMLOC_REM wFSD = 0;
    uint16_t    PH_MEMLOC_REM wFSC = 0;
    uint16_t   PH_MEMLOC_REM wTmp = 0;
    uint16_t   pValue;

    /* Status and two other bytes to be backed up before getting new frame of data */
    (void)memset(bBackupBytes, 0x00, 3);

    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
        pDataParams->pHalDataParams,
        PHHAL_HW_CONFIG_RXBUFFER_STARTPOS,
        &wTmp
    ));

    wOption = PH_EXCHANGE_DEFAULT;
    /*
       0xAF is just an indication that this is a call
       to the function to get remaining data
    */
    if (pSendBuff[0] != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME)
    {
        if (0U != (pDataParams->bWrappedMode))
        {
            /* Check for permissible CmdBuff size */
            if (wCmdLen > PHAL_MFDFEVX_MAXWRAPPEDAPDU_SIZE)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_MFDFEVX);
            }

            pApdu[1] = pSendBuff[0];  /* DESFire command code. */
            switch (pApdu[1])
            {
                case PHAL_MFDFEVX_CMD_READ_DATA:
                case PHAL_MFDFEVX_CMD_READ_DATA_ISO:
                case PHAL_MFDFEVX_CMD_READ_RECORDS:
                case PHAL_MFDFEVX_CMD_READ_RECORDS_ISO:
                    /* Get the format value(Whether to use short APDU or extended APDU */
                    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_GetConfig(pDataParams,
                        PHAL_MFDFEVX_SHORT_LENGTH_APDU,
                        &pValue));
                    /* To Note: Short APDU will be used,
                     *  1. when user force the 'length' to be sent as short APDU in case of BIG ISO.
                     *  2. When the data to read is not BIG ISO(less than 256 bytes).
                     */
                    if(0U != (pValue & 0x0001U))
                    {
                        /* Encode 'Length' in Short APDU format */
                        pApdu[bApduLen++] = (uint8_t) wCmdLen - 1u; /* Set APDU Length. */
                    }
                    else
                    {
                        /* Encode 'Length' in extended Length format */
                        bExtendedLenApdu = 0x01;
                        pApdu[bApduLen++] = 0x00;
                        pApdu[bApduLen++] = 0x00;
                        pApdu[bApduLen++] = (uint8_t) wCmdLen - 1u; /* Set APDU Length. */
                    }
                    break;

                default:
                    /* Rest other commands, retain existing implementation which is Short APDU */
                    pApdu[bApduLen++] = (uint8_t) wCmdLen - 1u; /* Set APDU Length. */
                    break;
            }
            status = phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                (pApdu[bApduLen - 1] == 0x00) ? PH_EXCHANGE_DEFAULT : PH_EXCHANGE_BUFFER_FIRST,
                pApdu,
                bApduLen,
                &pRecv,
                pRxlen
            );
            /* Check if pApdu[4] is valid in case of Short APDU or
             * Check if pAdpu[6] is valid in case of Extended APDU
             */
            if (((pApdu[4] != 0x00U) && (status == PH_ERR_SUCCESS)) ||
                ((bExtendedLenApdu && (pApdu[6] != 0x00U)) && (status == PH_ERR_SUCCESS)))
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_CONT,
                    &pSendBuff[1],
                    wCmdLen - 1u,
                    &pRecv,
                    pRxlen
                ));

                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_LAST,
                    bLe,
                    bExtendedLenApdu ? 0x02 : 0x01,
                    &pRecv,
                    pRxlen
                ));
            }
        }
        else
        {
            /* Normal mode */
            if (wCmdLen > PHAL_MFDFEVX_MAXDFAPDU_SIZE)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_MFDFEVX);
            }

            /* Send this on L4 */
            status = phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                wOption,
                pSendBuff,
                wCmdLen,
                &pRecv,
                pRxlen
            );
        }
    }
    else
    {
        /* Send this on L4 */
        status = phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_RXCHAINING,
            pSendBuff,
            wCmdLen,
            &pRecv,
            pRxlen
        );
    }

    if ((status != PH_ERR_SUCCESS) && ((status & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING))
    {
        return status;
    }

    /* Storing the original pointer */
    *pResponse = pRecv;

    if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING)
    {
        if (0U != (pDataParams->bWrappedMode))
        {
            /* Next position will ensure overwriting on the
            SW1SW2 received from previous command */
            wNextPos = (*pRxlen) - 2U;
            (void)memcpy(bBackupBytes, &(*pResponse)[wNextPos - 3u], 3);
        }
        else
        {
            /* Backup the last byte */
            (void)memcpy(bBackupBytes, &(*pResponse)[(*pRxlen - 3)], 3);
            wNextPos = *pRxlen;
        }

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
            pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_RXBUFFER_BUFSIZE,
            &wRxBufferSize
        ));
    }

    while ((status & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING)
    {
        /* Get the Frame length */
        PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_GetFrameLength(
            pDataParams,
            &wFSD,
            &wFSC
        ));

        if (wFSD > wFSC)
        {
            if ((wNextPos + wFSC) >= wRxBufferSize)
            {
                /* Return Chaining and let the caller recall the function with
                option = PH_EXCHANGE_RXCHAINING */
                /* Return the data accumulated till now and its length */
                if ((pSendBuff[0] != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME) && (!pDataParams->bWrappedMode))
                {
                    (*pRxlen) -= 1u;
                    (*pResponse)++;
                }
                return PH_ADD_COMPCODE_FIXED(PH_ERR_SUCCESS_CHAINING, PH_COMP_AL_MFDFEVX);
            }
        }
        else
        {
            if ((wNextPos + wFSD) >= wRxBufferSize)
            {
                /* Return Chaining and let the caller recall the function with
                option = PH_EXCHANGE_RXCHAINING */
                /* Return the data accumulated till now and its length */
                if (0U != (pDataParams->bWrappedMode))
                {
                    (*pRxlen) -= 2u;
                }

                return PH_ADD_COMPCODE_FIXED(PH_ERR_SUCCESS_CHAINING, PH_COMP_AL_MFDFEVX);
            }
        }

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
            pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_RXBUFFER_STARTPOS,
            wNextPos
        ));

        /* Send this on L4 */
        status = phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_RXCHAINING,
            bCmdBuff,
            wCmdLen,
            &pRecv,
            pRxlen
        );

        /* Update wNextPos */
        if (0U != (pDataParams->bWrappedMode))
        {
            /* Putback the backed up bytes */
            (void)memcpy(&(*pResponse)[wNextPos - 3u], bBackupBytes, 3);

            /* Update the Buffer Position */
            wNextPos = (*pRxlen) - 2U;
        }
        else
        {
            /* Putback the backed up bytes */
            (void)memcpy(&(*pResponse)[wNextPos - 3u], bBackupBytes, 3);

            /* Update the Buffer Position */
            wNextPos = *pRxlen;
        }
        /* Backup 3 bytes. The nxt frame will overwrite these */
        (void)memcpy(bBackupBytes, &(*pResponse)[wNextPos - 3u], 3);
    }

    /* Status is 0x00? */
    if (0U != (pDataParams->bWrappedMode))
    {
        statusTmp = pRecv[(*pRxlen) - 1];
        (*pRxlen) -= 2u;
    }
    else
    {
        if (pSendBuff[0] != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME)
        {
            statusTmp = pRecv[wTmp];
            (*pRxlen) -= 1u;
            (*pResponse)++;
        }
        else
        {
            statusTmp = status;
        }
    }

    return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, statusTmp);
}

phStatus_t phalMfdfEVx_Sw_Int_ReadData_Plain(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * bCmdBuff,
    uint16_t wCmdLen, uint8_t ** ppRxdata, uint16_t * pRxdataLen)
{
    uint16_t    PH_MEMLOC_REM status = 0;
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM * pRecv = NULL;
    uint8_t     PH_MEMLOC_REM bWorkBuffer[32];
    uint16_t    PH_MEMLOC_REM wWorkBufferLen = 0;

    (void)memset(bWorkBuffer, 0x00, 32);

    /* Copy the bCmdBuff data to the bWorkBuff */
    (void)memcpy(bWorkBuffer, bCmdBuff, wCmdLen);
    wWorkBufferLen = wCmdLen;


    if (0U != (bOption & PHAL_MFDFEVX_ISO_CHAINING_MODE))
    {
        status = phalMfdfEVx_Sw_Int_ISOGetData(
            pDataParams,
            bWorkBuffer,
            wWorkBufferLen,
            &pRecv,
            &wRxlen
        );
    }
    else
    {
        /* Send the command */
        status = phalMfdfEVx_Sw_Int_GetData(
            pDataParams,
            bWorkBuffer,
            wWorkBufferLen,
            &pRecv,
            &wRxlen
        );
    }

    if (((status & PH_ERR_MASK) != PH_ERR_SUCCESS) &&
        ((status & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING))
    {
        return status;
    }

    if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) &&
        (bCmdBuff[0] != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME))
    {
        /* Increment the command counter. This increments irrespective of
        * Plain mode or MAC mode. Ensuring here that it is incremented
        * only for the case where the user has called this for the first
        * time i.e., without PH_EXCHANGE_RXCHAINING option
        */
        pDataParams->wCmdCtr++;
    }


    *ppRxdata = pRecv;
    *pRxdataLen = wRxlen;

    if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_SUCCESS_CHAINING, PH_COMP_AL_MFDFEVX);;
    }

    return PH_ERR_SUCCESS;
}


phStatus_t phalMfdfEVx_Sw_Int_Write_New(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t * pCmdBuff,
    uint16_t wCmdLen, uint8_t * pData, uint16_t wDataLen)
{
    uint16_t    PH_MEMLOC_REM wStatus = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;
    uint8_t     PH_MEMLOC_REM bPiccRetCode = 0;
    uint8_t     PH_MEMLOC_REM aEncBuffer[256];
    uint16_t    PH_MEMLOC_REM wEncBufLen = 0;
    uint8_t     PH_MEMLOC_REM aMac[16];
    uint8_t     PH_MEMLOC_REM bMacLen = 0;


    /* Buffer Command Information to PAL. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfdfEVx_Sw_Int_CardExchange(
        pDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        (uint16_t) (wCmdLen + ((bCommOption == PHAL_MFDFEVX_COMMUNICATION_ENC) ? wEncBufLen : wDataLen) + bMacLen),
        PH_OFF,
        pCmdBuff,
        wCmdLen,
        NULL,
        NULL,
        NULL));

    /* Buffer Encrypted / Plain MFCLicenseMAC Information to PAL. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfdfEVx_Sw_Int_CardExchange(
        pDataParams,
        PH_EXCHANGE_BUFFER_CONT,
        0,
        PH_OFF,
        (bCommOption == PHAL_MFDFEVX_COMMUNICATION_ENC) ? aEncBuffer : pData,
        (uint16_t) (bCommOption == PHAL_MFDFEVX_COMMUNICATION_ENC) ? wEncBufLen : wDataLen,
        NULL,
        NULL,
        NULL));

    /* Buffer and Exchange Secure Messaging MAC to PICC. */
    wStatus = phalMfdfEVx_Sw_Int_CardExchange(
        pDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        0,
        PH_ON,
        aMac,
        bMacLen,
        &pResponse,
        &wRespLen,
        &bPiccRetCode);

    /* Verify the status. */
    if ((wStatus & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        /* Reset authentication status */
        if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
            (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES) ||
            (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2))
        {
            phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
        }

        return wStatus;
    }


    return PH_ERR_SUCCESS;
}

phStatus_t phalMfdfEVx_Sw_Int_Write_Plain(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bIns, uint8_t * bCmdBuff,
    uint16_t wCmdLen, uint8_t bCommOption, uint8_t * pData, uint16_t wDataLen)
{
    phStatus_t  PH_MEMLOC_REM statusTmp = 0;
    uint16_t    PH_MEMLOC_REM status = 0;
    uint16_t    PH_MEMLOC_REM wRxlen = 0;
    uint8_t     PH_MEMLOC_REM bWorkBuffer[32];
    uint16_t    PH_MEMLOC_REM wFrameLen = 0;
    uint16_t    PH_MEMLOC_REM wTotalLen = 0;
    uint16_t    PH_MEMLOC_REM wTmp = 0;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint8_t     PH_MEMLOC_REM pApdu[5] = { PHAL_MFDFEVX_WRAPPEDAPDU_CLA, 0x00, PHAL_MFDFEVX_WRAPPEDAPDU_P1, PHAL_MFDFEVX_WRAPPEDAPDU_P2, 0x00 };
    uint8_t     PH_MEMLOC_REM bCMacCard[8];
    uint8_t     PH_MEMLOC_REM bMacLen = 0;
    uint8_t     PH_MEMLOC_REM bIvLen = 0;
    uint16_t    PH_MEMLOC_REM wWorkBufferLen = 0;
    uint16_t    PH_MEMLOC_REM wFSD = 0;
    uint16_t    PH_MEMLOC_REM wFSC = 0;
    uint16_t    PH_MEMLOC_REM wApduLen = 0;
    uint8_t     PH_MEMLOC_REM bAppId[3] = { 0x00, 0x00, 0x00 };

    (void)memset(bWorkBuffer, 0x00, 32);

    if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES))
    {

        /* If communication mode is set to plain
         * or bCommOption equals PHAL_MFDFEVX_MAC_DATA_INCOMPLETE, then MAC is only
         * calculated to update the init vector but is not sent with the data
         */
        if (((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_PLAIN) ||
            ((bCommOption & PHAL_MFDFEVX_MAC_DATA_INCOMPLETE) == PHAL_MFDFEVX_MAC_DATA_INCOMPLETE))
        {
            bMacLen = 0;
        }
        else
        {
            bMacLen = 8;
        }
    }
    else if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2) &&
        ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_MACD))
    {
        if ((bCmdBuff[0] != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME))
        {
            (void)memset(pDataParams->bIv, 0x00, bIvLen);
            pDataParams->bNoUnprocBytes = 0;
            /* Calculate MAC on Cmd || wCmdCtr || TI || CmdHeader || CmdData */
            bWorkBuffer[wWorkBufferLen++] = bCmdBuff[0];
            bWorkBuffer[wWorkBufferLen++] = (uint8_t) (pDataParams->wCmdCtr);
            bWorkBuffer[wWorkBufferLen++] = (uint8_t) (pDataParams->wCmdCtr >> 8U);
            (void)memcpy(&bWorkBuffer[wWorkBufferLen], pDataParams->bTi, PHAL_MFDFEVX_SIZE_TI);
            wWorkBufferLen += PHAL_MFDFEVX_SIZE_TI;
        }
        else
        {
            (void)memcpy(bWorkBuffer, pDataParams->pUnprocByteBuff, pDataParams->bNoUnprocBytes);
            wWorkBufferLen = pDataParams->bNoUnprocBytes;
            pDataParams->bNoUnprocBytes = 0;
        }
        /* Check for presence of command header */
        if (wCmdLen > 1U)
        {
            /* Calculate the total length of data for MAC calculation */
            wTmp = ((wCmdLen - 1u) + (wWorkBufferLen));
            /* Since bWorkbuffer can accomodate 32 bytes, check for buffer overflow */
            if (wTmp > 32U)
            {
                (void)memcpy(&bWorkBuffer[wWorkBufferLen], &bCmdBuff[1], (32 - wWorkBufferLen));
                /* Copy the remaining bCmdBuff into bWorkBuffer */
                (void)memcpy(bWorkBuffer, &bCmdBuff[(32 - wWorkBufferLen) + 1U], (wTmp - 32u));
                wWorkBufferLen = (wTmp - 32u);
            }
            else
            {
                (void)memcpy(&bWorkBuffer[wWorkBufferLen], &bCmdBuff[1], (wCmdLen - 1u));
                wWorkBufferLen += (wCmdLen - 1u);
            }
        }
        if ((bCommOption & PHAL_MFDFEVX_MAC_DATA_INCOMPLETE) == PHAL_MFDFEVX_MAC_DATA_INCOMPLETE)
        {
            bMacLen = 0;
        }
    }
    else if (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATE)
    {
        if ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_MACD)
        {
            if (bCmdBuff[0] != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME)
            {
                pDataParams->bNoUnprocBytes = 0;
            }
        }
    }
    else
    {
        /* Else statement due to else if above. */
        bMacLen = 0;
    }

    /* Update wTotalLen = datalen + CMAClen*/
    wTotalLen = wDataLen + bMacLen;

    if (bIns != PHAL_MFDFEVX_ISO_CHAINING_MODE)
    {
        if (0U != (pDataParams->bWrappedMode))
        {
            wFrameLen = PHAL_MFDFEVX_MAXWRAPPEDAPDU_SIZE;
        }
        else
        {
            wFrameLen = PHAL_MFDFEVX_MAXDFAPDU_SIZE;
        }
    }
    else
    {
        /* Get the Frame length */
        PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_GetFrameLength(
            pDataParams,
            &wFSD,
            &wFSC
        ));

        if (0U != (pDataParams->bWrappedMode))
        {
            wFrameLen = wFSC - 9u;
        }
        else
        {
            wFrameLen = wFSC - 4u;
        }
    }

    if (wTotalLen == 0x0000U)
    {
        /* Single frame cmd without any data. Just send it */
        status = phalMfdfEVx_ExchangeCmd(
            pDataParams,
            pDataParams->pPalMifareDataParams,
            pDataParams->bWrappedMode,
            bCmdBuff,
            wCmdLen,
            &pRecv,
            &wRxlen
        );
        if (((status & PH_ERR_MASK) != PH_ERR_SUCCESS) &&
            ((status & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING))
        {
            /* Reset authentication status */
            if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
                (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES) ||
                (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2))
            {
                phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
            }

            return PH_ADD_COMPCODE(status, PH_COMP_AL_MFDFEVX);
        }
        if (wRxlen > 32U)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }
        (void)memcpy(bWorkBuffer, pRecv, wRxlen);
    }
    else
    {
        if (bIns != PHAL_MFDFEVX_ISO_CHAINING_MODE)
        {
            if (0U != (pDataParams->bWrappedMode))
            {
                wFrameLen = PHAL_MFDFEVX_MAXWRAPPEDAPDU_SIZE;
            }
            else
            {
                wFrameLen = PHAL_MFDFEVX_MAXDFAPDU_SIZE;
            }
        }
        else
        {
            /* Get the Frame length */
            PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_GetFrameLength(
                pDataParams,
                &wFSD,
                &wFSC
            ));

            if (0U != (pDataParams->bWrappedMode))
            {
                wFrameLen = wFSC - 9u;
            }
            else
            {
                wFrameLen = wFSC - 4u;
            }
        }

        wTmp = wTotalLen;

        if (wTmp <= (wFrameLen - wCmdLen))
        {
            wApduLen = ((wCmdLen == 0x01U) && (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE)) ? 0 : PHAL_MFDFEVX_WRAP_HDR_LEN;
            wCmdLen = ((wCmdLen == 0x01U) && (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE)) ? 0 : wCmdLen;

            /* Send in one shot */
            if (0U != (pDataParams->bWrappedMode))
            {
                pApdu[1] = bCmdBuff[0]; /* DESFire cmd code in INS */

                if(wCmdLen > 0U)
                {
                    pApdu[4] = (uint8_t) (wCmdLen + wTotalLen) - 0x01u;
                }

                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_FIRST |
                    (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (0U != ((bCommOption & PHAL_MFDFEVX_MAC_DATA_INCOMPLETE)))) ? PH_EXCHANGE_TXCHAINING : 0),
                    pApdu,
                    wApduLen,
                    &pRecv,
                    &wRxlen));

                if (wCmdLen > 0U)
                {
                    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                        pDataParams->pPalMifareDataParams,
                        PH_EXCHANGE_BUFFER_CONT |
                        (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (0U != (bCommOption & PHAL_MFDFEVX_MAC_DATA_INCOMPLETE))) ? PH_EXCHANGE_TXCHAINING : 0),
                        &bCmdBuff[1],
                        wCmdLen - 1u,
                        &pRecv,
                        &wRxlen));
                }

                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_CONT |
                    (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (0U != ((bCommOption & PHAL_MFDFEVX_MAC_DATA_INCOMPLETE)))) ? PH_EXCHANGE_TXCHAINING : 0),
                    pData,
                    wDataLen,
                    &pRecv,
                    &wRxlen));

                /* Le byte */
                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_LAST | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (0U != ((bCommOption & PHAL_MFDFEVX_MAC_DATA_INCOMPLETE)))) ? PH_EXCHANGE_TXCHAINING : 0),
                    &pApdu[2],
                    (uint16_t) ((pDataParams->dwPayLoadLen > 0xFEU) && (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE)) ? 0x02U : 0x01U,
                    &pRecv,
                    &wRxlen));
            }
            else
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_FIRST |
                    (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (0U != ((bCommOption & PHAL_MFDFEVX_MAC_DATA_INCOMPLETE)))) ? PH_EXCHANGE_TXCHAINING : 0),
                    bCmdBuff,
                    wCmdLen,
                    &pRecv,
                    &wRxlen));


                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    (((bMacLen == 0x00U) ? PH_EXCHANGE_BUFFER_LAST : PH_EXCHANGE_BUFFER_CONT)) |
                    (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (0U != ((bCommOption & PHAL_MFDFEVX_MAC_DATA_INCOMPLETE)))) ? PH_EXCHANGE_TXCHAINING : 0),
                    pData,
                    wDataLen,
                    &pRecv,
                    &wRxlen));

            }
            if (0U != (pDataParams->bWrappedMode))
            {
                status = pRecv[wRxlen - 1u];
                wRxlen -= 2u;
            }
            else
            {
                status = pRecv[0];
                pRecv++; /* Increment pointer to point only to data */
                wRxlen -= 1u;
            }

            if ((status != PH_ERR_SUCCESS) &&
                ((status & PH_ERR_MASK) != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME))
            {

                /* Reset authentication status */
                if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
                    (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES) ||
                    (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2))
                {
                    phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
                }

                return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, (uint8_t) status);
            }

            (void)memcpy(bWorkBuffer, pRecv, wRxlen);
        }
        else
        {
        }
    }

    if ((status == PH_ERR_SUCCESS) && ((bCommOption & PHAL_MFDFEVX_AUTHENTICATE_RESET) == PHAL_MFDFEVX_AUTHENTICATE_RESET))
    {
        /* Reset authentication status */
        if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
            (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES) ||
            (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2))
        {
            phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
        }
        return PH_ERR_SUCCESS;
    }

    /* Verify the MAC. MAC is not received if in 0x0A MAC'd mode */
    if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
        (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES))
    {
        if ((status & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING)  /* if no chaining ongoing */
        {
            /*
             * In case of delete app, check whether the command is called at APP level or PICC level.
             * 1. At APP level, the MAC is not returned.
             * 2. At PICC level, 8 bytes MAC is returned.
             * So to check whether its in APP level or PICC level. To do this, check for pDataParams->pAid. If its 0x00, then its PICC level
             * else its in APP level.
             */
            if (PHAL_MFDFEVX_CMD_DELETE_APPLN == bCmdBuff[0])
            {
                /* if PICC level selected */
                if (memcmp(pDataParams->pAid, bAppId, 3) == 0x00)
                {
                    /* If NO Mac is returned */
                    if (wRxlen < 8U)
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
                    }
                }
                /* if APP level selected */
                else
                {
                    /* Before returning status code, reset auth and set app ID to Master APP */
                    phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);

                    if ((memset(pDataParams->pAid, 0x00, 3)) == NULL)
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AL_MFDFEVX);
                    }
                    /* return error if Mac is returned */
                    if (wRxlen >= 8U)
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
                    }
                    else
                    {
                        return PH_ERR_SUCCESS;
                    }
                }
            }
            else
            {
                if (wRxlen < 8U) /* If no CMAC received */
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
                }
            }
            if ((statusTmp & PH_ERR_MASK) == PH_ERR_SUCCESS)
            {
                /* copy CMAC received from card*/
                (void)memcpy(bCMacCard, &bWorkBuffer[wRxlen - 8u], 8);
                wRxlen -= 8u;
                /* Copy the status byte at the end */
                bWorkBuffer[wRxlen] = (uint8_t) status;
            }
        }
    }
    else if (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEEV2)
    {
        /*
        * In case of delete app, check whether the command is called at APP level or PICC level.
        * 1. At APP level, the MAC is not returned.
        * 2. At PICC level, 8 bytes MAC is returned.
        * So to check whether its in APP level or PICC level. To do this, check for pDataParams->pAid. If its 0x00, then its PICC level
        * else its in APP level.
        */
        if (PHAL_MFDFEVX_CMD_DELETE_APPLN == bCmdBuff[0])
        {
            /* If PICC level is selected */
            if (memcmp(pDataParams->pAid, bAppId, 3) == 0x00)
            {
                /* If NO Mac is returned */
                if (wRxlen < 8U)
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
                }
            }
            /* if APP level selected */
            else
            {
                /* Before returning status code, reset auth and set app ID to Master APP */
                phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);

                if ((memset(pDataParams->pAid, 0x00, 3)) == NULL)
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AL_MFDFEVX);
                }
                /* return error if Mac is returned */
                if (wRxlen >= 8U)
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
                }
                else
                {
                    /* Increment the command counter. */
                    pDataParams->wCmdCtr++;
                    return PH_ERR_SUCCESS;
                }
            }
        }
        if ((status & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING)  /* if no chaining ongoing */
        {

            if ((statusTmp & PH_ERR_MASK) == PH_ERR_SUCCESS)
            {
                /* Increment the command counter.
                *  This increments irrespective of Plain mode or MAC mode.
                */
                pDataParams->wCmdCtr++;
            }

            if ((bCommOption & 0xF0U) == PHAL_MFDFEVX_COMMUNICATION_MACD)
            {
                if (wRxlen < 8U) /* If no CMAC received */
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
                }
            }

        }

    }
    else
    {
        /* Should not get more bytes than the status bytes in case
        of no authentication */
        if (wRxlen > 0U)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }
    }

    return PH_ERR_SUCCESS;
}

void phalMfdfEVx_Sw_Int_ResetAuthStatus(phalMfdfEVx_Sw_DataParams_t * pDataParams)
{
    phStatus_t PH_MEMLOC_REM statusTmp;
    (void)memset(pDataParams->bSesAuthENCKey, 0x00, (size_t)sizeof(pDataParams->bSesAuthENCKey));
    (void)memset(pDataParams->bSesAuthMACKey, 0x00, (size_t)sizeof(pDataParams->bSesAuthMACKey));
    pDataParams->bKeyNo = 0xFF;
    (void)memset(pDataParams->bIv, 0x00, (size_t)sizeof(pDataParams->bIv));
    pDataParams->bAuthMode = PHAL_MFDFEVX_NOT_AUTHENTICATED;
    pDataParams->bCryptoMethod = 0xFF;
    pDataParams->wCmdCtr = 0;
    (void)memset(pDataParams->bTi, 0x00, PHAL_MFDFEVX_SIZE_TI);
    pDataParams->bNoUnprocBytes = 0;
    pDataParams->bLastBlockIndex = 0;
    statusTmp = phTMIUtils_ActivateTMICollection((phTMIUtils_t *) pDataParams->pTMIDataParams, PH_TMIUTILS_RESET_TMI);

    pDataParams->bCmdCode = PHAL_MFDFEVX_CMD_INVALID;

    /* Update the authentication state if VCA PC feature is required by the application. */
    if (pDataParams->pVCADataParams != NULL)
    {
        statusTmp = phalVca_SetSessionKeyUtility((phalVca_Sw_DataParams_t *) pDataParams->pVCADataParams,
            pDataParams->bSesAuthENCKey,
            pDataParams->bAuthMode);
    }

   /* satisfy compiler */
    PH_UNUSED_VARIABLE (statusTmp);
}

phStatus_t phalMfdfEVx_Sw_Int_SendDataToPICC(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bIns, uint8_t bCommOption,
    uint8_t * pCmd, uint16_t wCmdLen, uint8_t * pData, uint16_t wDataLen, uint8_t * bLastChunk, uint16_t wLastChunkLen,
    uint8_t * pResp, uint16_t * pRespLen)
{
    /* Utility function to send data to PICC if more then wFrameLen*/
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bStatusByte;
    uint16_t    PH_MEMLOC_REM wIndexDataLen = 0;
    uint16_t    PH_MEMLOC_REM wFrameLen = 0;
    uint8_t     PH_MEMLOC_REM pApdu[7] = { PHAL_MFDFEVX_WRAPPEDAPDU_CLA, 0x00, PHAL_MFDFEVX_WRAPPEDAPDU_P1, PHAL_MFDFEVX_WRAPPEDAPDU_P2, 0x00, 0x00, 0x00 };
    uint8_t     PH_MEMLOC_REM * pRecv = NULL;
    uint8_t     PH_MEMLOC_REM bExtended7816 = 0;
    uint8_t     PH_MEMLOC_REM bCommOptionTmp;
    uint16_t    PH_MEMLOC_REM wFSD = 0;
    uint16_t    PH_MEMLOC_REM wFSC = 0;
    uint16_t    PH_MEMLOC_REM wDataToBeSent = 0;
    uint16_t    PH_MEMLOC_REM wCopyDataLen = 0;
    uint16_t    PH_MEMLOC_REM wTmpDataLen = 0;
    uint16_t    PH_MEMLOC_REM wCopyLastChunkLen = 0;
    uint16_t    PH_MEMLOC_REM wTmpLastChunkLen = 0;
    uint16_t    PH_MEMLOC_REM wIndexLastChunkLen = 0;
    uint16_t    PH_MEMLOC_REM wApduHeaderLen = 0;
    uint16_t    PH_MEMLOC_REM wLeFieldLen = 0;
    uint16_t    PH_MEMLOC_REM wIndexCmdLen = 1;
    uint16_t    PH_MEMLOC_REM wTmpData;
    uint16_t    PH_MEMLOC_REM wTemLen = 0;

    if (bIns != PHAL_MFDFEVX_ISO_CHAINING_MODE)
    {
        if (0U != (pDataParams->bWrappedMode))
        {
            wFrameLen = PHAL_MFDFEVX_MAXWRAPPEDAPDU_SIZE;
            wApduHeaderLen = PHAL_MFDFEVX_WRAP_HDR_LEN;
        }
        else
        {
            wFrameLen = PHAL_MFDFEVX_MAXDFAPDU_SIZE;
        }
    }
    else
    {
        /* Get the Frame length */
        PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_GetFrameLength(
            pDataParams,
            &wFSD,
            &wFSC
        ));

        if (0U != (pDataParams->bWrappedMode))
        {
            bExtended7816 = (uint8_t) (pDataParams->dwPayLoadLen > 0xFEU) ? 1 : 0;
            /* if Lc is more then 0xFF, length of Lc should be 3 bytes */
            wApduHeaderLen = PHAL_MFDFEVX_WRAP_HDR_LEN + (bExtended7816 ? 2 : 0);
        }
        wFrameLen = wFSC - 4u;
    }

    /* If Ins mode and wrapped mode are enable Le is sent with the last packet. So update Le when only last packet is sent */
    wLeFieldLen = ((bIns != PHAL_MFDFEVX_ISO_CHAINING_MODE) && (pDataParams->bWrappedMode == 1U)) ? 1 : 0;

    /* Send the data to PICC */
    wTmpDataLen = wDataLen;
    wTmpLastChunkLen = wLastChunkLen;

    do
    {
        /* In case of wrapped mode, cmd byte is added as a part of Apdu Header. */
        wIndexCmdLen = wApduHeaderLen ? 1 : 0;

        /* this If condition is added to suppress QAC warning */
        wTemLen = wCmdLen;
        if (wCmdLen > 0U)
        {
            wTemLen = wCmdLen - wIndexCmdLen;
        }
        /* If Ins mode and wrapped mode are enable Le is sent with the last packet */
        if (pDataParams->bWrappedMode && (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && ((wTmpDataLen + wTmpLastChunkLen + wTemLen) <= wFrameLen))
        {
            wLeFieldLen = 1 + bExtended7816;
        }
        if (wTmpDataLen > 0U)
        {
            wCopyDataLen = (wTmpDataLen < (wFrameLen - (wTemLen + wApduHeaderLen + wLeFieldLen))) ?
                wTmpDataLen : (wFrameLen - (wTemLen + wApduHeaderLen + wLeFieldLen));
        }

        if (wTmpLastChunkLen > 0U)
        {
            wTmpData = wTemLen + wCopyDataLen + wApduHeaderLen + wLeFieldLen;
            wCopyLastChunkLen = (wTmpLastChunkLen < (wFrameLen - wTmpData)) ? wTmpLastChunkLen : (wFrameLen - wTmpData);
        }

        /* remaining data to be sent */
        /* this If condition is added to suppress QAC warning */
        wDataToBeSent = (wTmpDataLen - wCopyDataLen) + (wTmpLastChunkLen - wCopyLastChunkLen);

        wCmdLen = ((wCmdLen == 0x01U) && (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE)) ? 0 : wCmdLen;

        bCommOptionTmp = (uint8_t) (((!wDataToBeSent) && (!bCommOption)) ? 0 : 1);

        if (0U != (pDataParams->bWrappedMode))
        {
            pApdu[1] = pCmd[0]; /* DESFire cmd code in INS */

            /* in case of ISO chaining mode, total length of data should be sent with the first frame*/
            if (bExtended7816 && (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE))
            {
                pDataParams->dwPayLoadLen = pDataParams->dwPayLoadLen + wTemLen;
                pApdu[4] = (uint8_t) ((pDataParams->dwPayLoadLen >> 16U) & 0xFFU);
                pApdu[5] = (uint8_t) ((pDataParams->dwPayLoadLen >> 8U) & 0xFFU);
                pApdu[6] = (uint8_t) (pDataParams->dwPayLoadLen & 0xFFU);
            }
            else
            {
                pApdu[4] = (uint8_t) (wTemLen + ((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) ? pDataParams->dwPayLoadLen : (wCopyDataLen + wCopyLastChunkLen)));
            }

            wApduHeaderLen = ((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (wTemLen == 0U)) ? 0 : wApduHeaderLen;

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_FIRST | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U)) ? PH_EXCHANGE_TXCHAINING : 0),
                pApdu,
                wApduHeaderLen,
                &pRecv,
                pRespLen));

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pCmd[wIndexCmdLen],
                wTemLen,
                &pRecv,
                pRespLen));

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pData[wIndexDataLen],
                wCopyDataLen,
                &pRecv,
                pRespLen));

            /* send last chunk */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U)) ? PH_EXCHANGE_TXCHAINING : 0),
                &bLastChunk[wIndexLastChunkLen],
                wCopyLastChunkLen,
                &pRecv,
                pRespLen));

            wLeFieldLen = ((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U)) ? 0 : wLeFieldLen;

            /* Le byte */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_LAST | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pApdu[2],
                wLeFieldLen,
                &pRecv,
                pRespLen));
        }
        else
        {
            /* send cmd */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_FIRST | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U)) ? PH_EXCHANGE_TXCHAINING : 0),
                pCmd,
                wCmdLen,
                &pRecv,
                pRespLen));

            if(0U != (wCopyDataLen))
            {
                /*  send data */
                PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                    pDataParams->pPalMifareDataParams,
                    PH_EXCHANGE_BUFFER_CONT | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U)) ? PH_EXCHANGE_TXCHAINING : 0),
                    &pData[wIndexDataLen],
                    wCopyDataLen,
                    &pRecv,
                    pRespLen));
            }

            /* send last chunk */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_LAST | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U)) ? PH_EXCHANGE_TXCHAINING : 0),
                &bLastChunk[wIndexLastChunkLen],
                wCopyLastChunkLen,
                &pRecv,
                pRespLen));
        }

        /* copy number of data sent */
        wIndexDataLen += wCopyDataLen;
        wIndexLastChunkLen += wCopyLastChunkLen;

        /* copy the remaining data to be sent */
        /* this If condition is added to suppress QAC warning */
        if (wTmpDataLen > 0U)
        {
            wTmpDataLen = wTmpDataLen - wCopyDataLen;
        }

        /* this If condition is added to suppress QAC warning */
        if (wTmpLastChunkLen > 0U)
        {
            wTmpLastChunkLen = wTmpLastChunkLen - wCopyLastChunkLen;
        }

        wCopyDataLen = 0;
        wCopyLastChunkLen = 0;

        /* in case of 14443-4 chaining R-block that indicates a positive acknowledge */
        if ((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) && (bCommOptionTmp != 0U))
        {
            bStatusByte = (uint8_t) ((pRecv[0] & 0xF0U) == 0xA0U) ? PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME : PH_ERR_PROTOCOL_ERROR;
        }
        else
        {
            /* validate the response byte */
            if (0U != (pDataParams->bWrappedMode))
            {
                (void)memcpy(pResp, pRecv, (*pRespLen) - 2);
                bStatusByte = pRecv[(*pRespLen) - 1];
                (*pRespLen) -= 2u;
            }
            else
            {
                (void)memcpy(pResp, &pRecv[1], (*pRespLen) - 1);
                bStatusByte = pRecv[0];
                (*pRespLen) -= 1u;
            }
        }

        if ((bStatusByte != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME) &&
            (bStatusByte != PH_ERR_SUCCESS))
        {
            /* Reset authentication status */
            if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
                (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES))
            {
                phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
            }

            return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, bStatusByte);
        }

        /* Success returned even before writing all data? protocol error */
        if ((bStatusByte == PH_ERR_SUCCESS) && (bCommOptionTmp != 0U))
        {
            /* Reset authentication status */
            if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
                (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES))
            {
                phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
            }

            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }

        if (bStatusByte != 0x00U)
        {
            pCmd[0] = PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME;
            wCmdLen = (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) ? 0 : 1;
            wApduHeaderLen = (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE) ? 0 : wApduHeaderLen;
        }

    }while(0U != wDataToBeSent);

    return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, bStatusByte);
}

phStatus_t phalMfdfEVx_Sw_Int_SendDataAndAddDataToPICC(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bIns, uint8_t * pCmd,
    uint16_t wCmdLen, uint8_t * pData, uint16_t wDataLen, uint8_t * pAddData, uint16_t wAddDataLen, uint8_t * pResp,
    uint16_t * pRespLen)
{
    /* Utility function to send encrypted data to PICC as and when it is available from SAM */
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bStatusByte = 0;
    uint16_t    PH_MEMLOC_REM wIndex = 0;
    uint16_t    PH_MEMLOC_REM wDataLeft;
    uint16_t    PH_MEMLOC_REM wFrameLen = 0;
    uint8_t     PH_MEMLOC_REM pApdu[7] = { PHAL_MFDFEVX_WRAPPEDAPDU_CLA, 0x00, PHAL_MFDFEVX_WRAPPEDAPDU_P1, PHAL_MFDFEVX_WRAPPEDAPDU_P2, 0x00, 0x00, 0x00 };
    uint8_t     PH_MEMLOC_REM pLe[2] = { 0x00, 0x00 };
    uint8_t     PH_MEMLOC_REM * pRecv = NULL;
    uint16_t    PH_MEMLOC_REM wFSD = 0;
    uint16_t    PH_MEMLOC_REM wFSC = 0;
    uint16_t    PH_MEMLOC_REM wrappedApduHeaderLen = PHAL_MFDFEVX_WRAP_HDR_LEN;
    uint16_t    PH_MEMLOC_REM wrappedApduTrailerLen = 1;
    uint8_t     PH_MEMLOC_REM bIsExtended7816 = 0;
    uint16_t    PH_MEMLOC_REM wHeaderIdx = 0;
    uint16_t    PH_MEMLOC_REM wCmdIdx = 0;
    uint16_t    PH_MEMLOC_REM wAddDataIdx = 0;
    uint16_t    PH_MEMLOC_REM wTrailerIdx = 0;
    uint16_t    PH_MEMLOC_REM wWrappedApduHeaderLenLeft = 0;
    uint16_t    PH_MEMLOC_REM wWrappedApduTrailerLenLeft = 0;
    uint16_t    PH_MEMLOC_REM wCmdLenLeft = 0;
    uint16_t    PH_MEMLOC_REM wAddDataLenLeft = 0;
    uint16_t    PH_MEMLOC_REM wAddDataLenTotal = 0;
    uint16_t    PH_MEMLOC_REM wCmdLenTotal = 0;
    uint16_t    PH_MEMLOC_REM wDataLenTotal = 0;
    uint16_t    PH_MEMLOC_REM wMoreDataToTransmit = 0;

    if (bIns != PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED)
    {
        if (0U != (pDataParams->bWrappedMode))
        {
            wFrameLen = PHAL_MFDFEVX_MAXDFAPDU_SIZE;
        }
        else
        {
            wFrameLen = PHAL_MFDFEVX_MAXDFAPDU_SIZE;
        }
    }
    else
    {
        /* Get the Frame length */
        PH_CHECK_SUCCESS_FCT(statusTmp, phalMfdfEVx_Sw_Int_GetFrameLength(
            pDataParams,
            &wFSD,
            &wFSC
        ));

        if (0U != (pDataParams->bWrappedMode))
        {
            bIsExtended7816 = (uint8_t) ((wCmdLen + wDataLen + wAddDataLen - 0x01u) > 255U);
            wrappedApduHeaderLen += (bIsExtended7816 ? 2 : 0);
            wrappedApduTrailerLen += (bIsExtended7816 ? 1 : 0);
        }
        wFrameLen = wFSC - 4u;
    }

    wWrappedApduHeaderLenLeft = wrappedApduHeaderLen;
    wCmdLenLeft = wCmdLenTotal = (pDataParams->bWrappedMode) ? ((wCmdLen > 0) ? wCmdLen - 1 : 0) : wCmdLen; /* subtract instruction byte */
    wDataLeft = wDataLenTotal = wDataLen;
    wAddDataLenLeft = wAddDataLenTotal = wAddDataLen;
    wWrappedApduTrailerLenLeft = wrappedApduTrailerLen;

    do
    {
        if (0U != (pDataParams->bWrappedMode))
        {
            if (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED)
            {
                if (wWrappedApduHeaderLenLeft > 0U)
                {
                    wrappedApduHeaderLen = (wFrameLen / wWrappedApduHeaderLenLeft) ? wWrappedApduHeaderLenLeft : (wFrameLen % wWrappedApduHeaderLenLeft);
                }
                else
                {
                    wrappedApduHeaderLen = 0;
                }
                if (wCmdLenLeft > 0U)
                {
                    wCmdLen = ((wFrameLen - wrappedApduHeaderLen) / wCmdLenLeft) ? wCmdLenLeft : ((wFrameLen - wrappedApduHeaderLen) % wCmdLenLeft);
                }
                else
                {
                    wCmdLen = 0;
                }
                if (wDataLeft > 0U)
                {
                    wDataLen = ((wFrameLen - wrappedApduHeaderLen - wCmdLen) / wDataLeft) ? wDataLeft : ((wFrameLen - wrappedApduHeaderLen - wCmdLen) % wDataLeft);
                }
                else
                {
                    wDataLen = 0;
                }
                if (wAddDataLenLeft > 0U)
                {
                    wAddDataLen = ((wFrameLen - wrappedApduHeaderLen - wCmdLen - wDataLen) / wAddDataLenLeft)
                    ? wAddDataLenLeft
                    : ((wFrameLen - wrappedApduHeaderLen - wCmdLen - wDataLen) % wAddDataLenLeft);
                }
                else
                {
                    wAddDataLen = 0;
                }
                if (wWrappedApduTrailerLenLeft > 0U)
                {
                    wrappedApduTrailerLen = (0u != ((wFrameLen - wrappedApduHeaderLen - wCmdLen - wDataLen - wAddDataLen) / wWrappedApduTrailerLenLeft))
                    ? wWrappedApduTrailerLenLeft
                    : ((wFrameLen - wrappedApduHeaderLen - wCmdLen - wDataLen - wAddDataLen) % wWrappedApduTrailerLenLeft);
            }
            }
            else
            {
                if (wWrappedApduHeaderLenLeft > 0U)
                {
                    wrappedApduHeaderLen = (0u !=(wFrameLen / wWrappedApduHeaderLenLeft)) ? wWrappedApduHeaderLenLeft : (wFrameLen % wWrappedApduHeaderLenLeft);
                }
                else
                {
                    wrappedApduHeaderLen = 0;
                }
                if (wWrappedApduTrailerLenLeft > 0U)
                {
                    wrappedApduTrailerLen = (0u !=((wFrameLen - wrappedApduHeaderLen)) / wWrappedApduTrailerLenLeft)
                    ? wWrappedApduTrailerLenLeft
                    : ((wFrameLen - wrappedApduHeaderLen) % wWrappedApduTrailerLenLeft);
                }
                else
                {
                    wrappedApduTrailerLen = 0;
                }
                if (wCmdLenLeft > 0U)
                {
                    wCmdLen = (0u != ((wFrameLen - wrappedApduHeaderLen - wrappedApduTrailerLen) / wCmdLenLeft))
                    ? wCmdLenLeft
                    : ((wFrameLen - wrappedApduHeaderLen - wrappedApduTrailerLen) % wCmdLenLeft);
                }
                else
                {
                    wCmdLen = 0;
                }
                if (wDataLeft > 0U)
                {
                    wDataLen = (0u != ((wFrameLen - wrappedApduHeaderLen - wrappedApduTrailerLen - wCmdLen) / wDataLeft))
                    ? wDataLeft
                    : ((wFrameLen - wrappedApduHeaderLen - wrappedApduTrailerLen - wCmdLen) % wDataLeft);
                }
                else
                {
                    wDataLen = 0;
                }
                if (wAddDataLenLeft > 0U)
                {
                    wAddDataLen = (0u != ((wFrameLen - wrappedApduHeaderLen - wrappedApduTrailerLen - wCmdLen - wDataLen) / wAddDataLenLeft))
                    ? wAddDataLenLeft
                    : ((wFrameLen - wrappedApduHeaderLen - wrappedApduTrailerLen - wCmdLen - wDataLen) % wAddDataLenLeft);
                }
                else
                {
                    wAddDataLen = 0;
            }
            }

            pApdu[1] = pCmd[0]; /* DESFire cmd code in INS */
            if (bIsExtended7816 && (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED))
            {
                pApdu[6] = (uint8_t) (wCmdLenTotal + wDataLenTotal + wAddDataLenTotal);
                pApdu[5] = (uint8_t) ((wCmdLenTotal + wDataLenTotal + wAddDataLenTotal) >> 8U);
                /*pApdu[4] = 0; */
            }
            else
            {
                pApdu[4] = (uint8_t) (wCmdLen + ((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) ? (wDataLenTotal + wAddDataLenTotal) : (wDataLen + wAddDataLen)));
            }

            wMoreDataToTransmit = (wWrappedApduHeaderLenLeft - wrappedApduHeaderLen) +
                (wCmdLenLeft - wCmdLen) +
                (wDataLeft - wDataLen) +
                (wAddDataLenLeft - wAddDataLen) +
                (wWrappedApduTrailerLenLeft - wrappedApduTrailerLen);

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_FIRST | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) && (0u != wMoreDataToTransmit)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pApdu[wHeaderIdx],
                wrappedApduHeaderLen,
                &pRecv,
                pRespLen));

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) && (0u != wMoreDataToTransmit)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pCmd[1U + wCmdIdx],
                wCmdLen,
                &pRecv,
                pRespLen));

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) && (0u !=wMoreDataToTransmit)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pData[wIndex],
                wDataLen,
                &pRecv,
                pRespLen));

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) && (0u != wMoreDataToTransmit)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pAddData[wAddDataIdx],
                wAddDataLen,
                &pRecv,
                pRespLen));

            /* Le byte */
            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_LAST | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) && (0u !=wMoreDataToTransmit)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pLe[wTrailerIdx],
                wrappedApduTrailerLen,
                &pRecv,
                pRespLen));

            if (bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED)
            {
                wHeaderIdx += wrappedApduHeaderLen;
                wWrappedApduHeaderLenLeft -= wrappedApduHeaderLen;
                wCmdIdx += wCmdLen;
                wCmdLenLeft -= wCmdLen;
                wTrailerIdx += wrappedApduTrailerLen;
                wWrappedApduTrailerLenLeft -= wrappedApduTrailerLen;
            }
        }
        else
        {
            if (wCmdLenLeft > 0U)
            {
                wCmdLen = ((wFrameLen) / wCmdLenLeft) ? wCmdLenLeft : ((wFrameLen) % wCmdLenLeft);
            }
            else
            {
                wCmdLen = 0;
            }
            if (wDataLeft > 0U)
            {
                wDataLen = ((wFrameLen - wCmdLen) / wDataLeft) ? wDataLeft : ((wFrameLen - wCmdLen) % wDataLeft);
            }
            else
            {
                wDataLen = 0;
            }
            if (wAddDataLenLeft > 0U)
            {
                wAddDataLen = ((wFrameLen - wCmdLen - wDataLen) / wAddDataLenLeft) ? wAddDataLenLeft : ((wFrameLen - wCmdLen - wDataLen) % wAddDataLenLeft);
            }
            else
            {
                wAddDataLen = 0;
            }

            wMoreDataToTransmit = (wCmdLenLeft - wCmdLen) +
                (wDataLeft - wDataLen) +
                (wAddDataLenLeft - wAddDataLen);

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_FIRST | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) && (0u != wMoreDataToTransmit)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pCmd[wCmdIdx],
                wCmdLen,
                &pRecv,
                pRespLen));

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) && (0u != wMoreDataToTransmit)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pData[wIndex],
                wDataLen,
                &pRecv,
                pRespLen));

            PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
                pDataParams->pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_LAST | (((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) && (0u != wMoreDataToTransmit)) ? PH_EXCHANGE_TXCHAINING : 0),
                &pAddData[wAddDataIdx],
                wAddDataLen,
                &pRecv,
                pRespLen));

            wCmdIdx += wCmdLen;
            wCmdLenLeft -= wCmdLen;
        }

        wIndex += wDataLen;
        wDataLeft -= wDataLen;
        wAddDataIdx += wAddDataLen;
        wAddDataLenLeft -= wAddDataLen;

        /* in case of BIGISO, iso chaining is expected, and therefore R(ACK) block*/
        if ((bIns == PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED) && (0u != wMoreDataToTransmit))
        {
            /* in case of ACK */
            if ((pRecv[0] & 0xF0U) == 0xA0U)
            {
                continue;
            }
        }

        if (0U != (pDataParams->bWrappedMode))
        {
            if ((*pRespLen) < 2U)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
            }
            (void)memcpy(pResp, pRecv, (*pRespLen) - 2);
            bStatusByte = pRecv[(*pRespLen) - 1];
            (*pRespLen) -= 2u;
        }
        else
        {
            if ((*pRespLen) < 1U)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
            }
            (void)memcpy(pResp, &pRecv[1], (*pRespLen) - 1);
            bStatusByte = pRecv[0];
            (*pRespLen) -= 1u;
        }

        if ((bStatusByte != PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME) &&
            (bStatusByte != PH_ERR_SUCCESS))
        {

            /* Reset authentication status */
            if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
                (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES))
            {
                phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
            }

            return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, bStatusByte);
        }

        /* Success returned even before writing all data? protocol error */
        if ((bStatusByte == PH_ERR_SUCCESS) && (wDataLeft != 0U))
        {
            /* Reset authentication status */
            if ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEISO) ||
                (pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATEAES))
            {
                phalMfdfEVx_Sw_Int_ResetAuthStatus(pDataParams);
            }

            return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFDFEVX);
        }

        if (bStatusByte != 0x00U)
        {
            pCmd[0] = PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME;
            wCmdIdx = 0;
            wCmdLenLeft = (pDataParams->bWrappedMode) ? 0 : 1;
        }
    } while (0U != wMoreDataToTransmit);

    return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, bStatusByte);
}

phStatus_t phalMfdfEVx_Sw_Int_IsoRead(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wOption, uint8_t * bCmdBuff,
    uint16_t wCmdLen, uint8_t ** ppRxBuffer, uint32_t * pBytesRead)
{
    phStatus_t  PH_MEMLOC_REM status;
    uint16_t    PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wRxBufferSize;
    uint32_t    PH_MEMLOC_REM wNextPos;
    uint32_t    PH_MEMLOC_REM wRxlen;
    uint8_t     PH_MEMLOC_REM *pRecv = NULL;
    uint8_t     PH_MEMLOC_REM bBackupBytes[3];

    status = phpalMifare_ExchangeL4(
        pDataParams->pPalMifareDataParams,
        (wOption & PH_EXCHANGE_MODE_MASK),
        bCmdBuff,
        wCmdLen,
        ppRxBuffer,
        (uint16_t *) pBytesRead
    );

    /* First put everything on the reader Rx buffer upto buffer size - 60 */
    wRxlen = *pBytesRead;
    pRecv = *ppRxBuffer;

    if ((status != PH_ERR_SUCCESS) && ((status & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING))
    {
        /* Authentication should be reset */
        return status;
    }

    while ((status & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(
            pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_RXBUFFER_BUFSIZE,
            &wRxBufferSize
        ));

        wNextPos = *pBytesRead;
        (void)memcpy(bBackupBytes, &pRecv[wNextPos - 3u], 3);

        if ((wNextPos + PHAL_MFDFEVX_MAX_FRAME_SIZE) >= wRxBufferSize)
        {
            /* Calculate partical cmac if authenticated and return PH_ERR_SUCCESS_CHAINING */
            break;
        }

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
            pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_RXBUFFER_STARTPOS,
            (uint16_t) wNextPos
        ));
        status = phpalMifare_ExchangeL4(
            pDataParams->pPalMifareDataParams,
            PH_EXCHANGE_RXCHAINING,
            bCmdBuff,
            wCmdLen,
            ppRxBuffer,
            (uint16_t *) pBytesRead
        );

        /* Put back the backed up bytes */
        (void)memcpy(&pRecv[wNextPos - 3u], bBackupBytes, 3);

        if ((status != PH_ERR_SUCCESS) &&
            ((status & PH_ERR_MASK) != PH_ERR_SUCCESS_CHAINING))
        {
            return status;
        }
        wRxlen = *pBytesRead;
    }

    /* The data is now in *ppRxBuffer, length = wRxlen */
       /* satisfy compiler */
        PH_UNUSED_VARIABLE (wRxlen);

    if (status == PH_ERR_SUCCESS)
    {
        statusTmp = (*ppRxBuffer)[*pBytesRead - 2]; /* SW1 */
        statusTmp = statusTmp << 8U; /* Shift SW1 to MSB */
        statusTmp |= (*ppRxBuffer)[*pBytesRead - 1]; /* SW2 */

        if ((pDataParams->bAuthMode == PHAL_MFDFEVX_NOT_AUTHENTICATED) ||
            ((pDataParams->bAuthMode == PHAL_MFDFEVX_AUTHENTICATE) && ((wOption & PH_EXCHANGE_CUSTOM_BITS_MASK) == PHAL_MFDFEVX_COMMUNICATION_PLAIN)))
        {
            *pBytesRead -= 2;
            return phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, statusTmp);
        }
        statusTmp = phalMfdfEVx_Int_ComputeErrorResponse(pDataParams, statusTmp);

        if (statusTmp != PH_ERR_SUCCESS)
        {
            return statusTmp;
        }
    }
    else
    {

    }
    return PH_ADD_COMPCODE((status & PH_ERR_MASK), PH_COMP_AL_MFDFEVX);
}


phStatus_t phalMfdfEVx_Sw_Int_GetFrameLength(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t * pFSD, uint16_t * pFSC)
{
    uint16_t    PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wTmp = 0;

    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_GetConfig(
        pDataParams->pPalMifareDataParams,
        PHPAL_I14443P4_CONFIG_FSI,
        &wTmp
    ));

    /* Get FSD */
    switch ((uint8_t) (wTmp >> 8U))
    {
        case 0:
            *pFSD = 16;
            break;
        case 1:
            *pFSD = 24;
            break;
        case 2:
            *pFSD = 32;
            break;
        case 3:
            *pFSD = 40;
            break;
        case 4:
            *pFSD = 48;
            break;
        case 5:
            *pFSD = 64;
            break;
        case 6:
            *pFSD = 96;
            break;
        case 7:
            *pFSD = 128;
            break;
        case 8:
            *pFSD = 256;
            break;
        default:
            break;
    }

    /* Get FSC */
    switch ((uint8_t) (wTmp))
    {
        case 0:
            *pFSC = 16;
            break;
        case 1:
            *pFSC = 24;
            break;
        case 2:
            *pFSC = 32;
            break;
        case 3:
            *pFSC = 40;
            break;
        case 4:
            *pFSC = 48;
            break;
        case 5:
            *pFSC = 64;
            break;
        case 6:
            *pFSC = 96;
            break;
        case 7:
            *pFSC = 128;
            break;
        case 8:
            *pFSC = 256;
            break;
        default:
            break;
    }

    return PH_ERR_SUCCESS;
}


#endif /* NXPBUILD__PHAL_MFDFEVX_SW */
