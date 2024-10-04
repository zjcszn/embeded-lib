/*----------------------------------------------------------------------------*/
/* Copyright 2013 - 2018, 2022 NXP                                            */
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
* Generic MIFARE Plus(R) Application Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 4433 $ (v07.09.00)
* $Date: 2018-02-12 19:35:06 +0530 (Mon, 12 Feb 2018) $
*
* History:
*  Kumar GVS: Generated 15. Apr 2013
*
*/

#include <ph_Status.h>
#include <phpalMifare.h>
#include <phalMfpEVx.h>
#include "phalMfpEVx_Int.h"
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PHAL_MFPEVX

phStatus_t phalMfpEVx_Int_ComputeErrorResponse(uint16_t wNumBytesReceived, uint16_t wStatus, uint8_t bLayer4Comm)
{
    phStatus_t  PH_MEMLOC_REM status;

    /* Invalid error response */
    if(wNumBytesReceived == 0U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFPEVX);
    }

    /* validate received response */
    if(wNumBytesReceived == 1U)
    {
        if(0U != bLayer4Comm)
        {
            switch(wStatus)
            {
                case PHAL_MFPEVX_RESP_ACK_ISO4:
                case PHAL_MFPEVX_ISO7816_RESP_SUCCESS:
                    status = PH_ERR_SUCCESS;
                    break;

                case PHAL_MFPEVX_RESP_ERR_AUTH:

                    status = PHAL_MFPEVX_ERR_AUTH;
                    break;

                case PHAL_MFPEVX_RESP_ERR_CMD_OVERFLOW:

                    status = PHAL_MFPEVX_ERR_CMD_OVERFLOW;
                    break;

                case PHAL_MFPEVX_RESP_ERR_MAC_PCD:

                    status = PHAL_MFPEVX_ERR_MAC_PCD;
                    break;

                case PHAL_MFPEVX_RESP_ERR_BNR:

                    status = PHAL_MFPEVX_ERR_BNR;
                    break;

                case PHAL_MFPEVX_RESP_ERR_CMD_INVALID:

                    status = PHAL_MFPEVX_ERR_CMD_INVALID;
                    break;

                case PHAL_MFPEVX_RESP_ERR_FORMAT:

                    status = PHAL_MFPEVX_ERR_FORMAT;
                    break;

                case PHAL_MFPEVX_RESP_ERR_GEN_FAILURE:

                    status = PHAL_MFPEVX_ERR_GEN_FAILURE;
                    break;

                case PHAL_MFPEVX_RESP_ERR_EXT:
                    status = PHAL_MFPEVX_ERR_EXT;
                    break;

                case PHAL_MFPEVX_RESP_ERR_TM:

                    status = PHAL_MFPEVX_ERR_TM;
                    break;

                case PHAL_MFPEVX_RESP_ERR_NOT_SUP:

                    status = PHAL_MFPEVX_ERR_NOT_SUP;
                    break;

                case PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_LENGTH:
                    status = PHAL_MFPEVX_ISO7816_ERR_WRONG_LENGTH;
                    break;

                case PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_PARAMS:
                    status = PHAL_MFPEVX_ISO7816_ERR_WRONG_LENGTH;
                    break;

                case PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_LC:
                    status = PHAL_MFPEVX_ISO7816_ERR_WRONG_LC;
                    break;

                case PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_LE:
                    status = PHAL_MFPEVX_ISO7816_ERR_WRONG_LE;
                    break;

                case PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_CLA:
                    status = PHAL_MFPEVX_ISO7816_ERR_WRONG_CLA;
                    break;

                default:

                    status = PH_ERR_PROTOCOL_ERROR;
                    break;
            }

            return PH_ADD_COMPCODE(status, PH_COMP_AL_MFPEVX);
        }
        else
        {
            switch(wStatus)
            {
                case PHAL_MFPEVX_RESP_ACK_ISO3:
                case PHAL_MFPEVX_RESP_ACK_ISO4:
                    status = PH_ERR_SUCCESS;
                    break;
                    /* Mapping of NAK codes: */
                case PHAL_MFPEVX_RESP_NACK0:
                    status = PHPAL_MIFARE_ERR_NAK0;
                    break;
                case PHAL_MFPEVX_RESP_NACK1:
                    status = PHPAL_MIFARE_ERR_NAK1;
                    break;
                case PHAL_MFPEVX_RESP_NACK4:
                    status = PHPAL_MIFARE_ERR_NAK4;
                    break;
                case PHAL_MFPEVX_RESP_NACK5:
                    status = PHPAL_MIFARE_ERR_NAK5;
                    break;
                default:
                    status = PH_ERR_PROTOCOL_ERROR;
                    break;
            }

            return PH_ADD_COMPCODE(status, PH_COMP_PAL_MIFARE); /* For compatibility reasons with SW stack we use here PAL MIFARE */
        }

    }

    /* validate received response with wNumBytesReceived > 1 */
    if(wStatus != PHAL_MFPEVX_RESP_ACK_ISO4)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFPEVX);
    }

    /* proper error response */
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_ComputeErrorResponseMfc(uint16_t wNumBytesReceived, uint8_t bStatus)
{
    phStatus_t  PH_MEMLOC_REM status;

    /* Validate received response */
    if(wNumBytesReceived == 1U)
    {
        switch(bStatus)
        {
            case PHAL_MFPEVX_RESP_ACK_ISO3:
                status = PH_ERR_SUCCESS;
                break;

                /* Mapping of NAK codes: */
            case PHAL_MFPEVX_RESP_NACK0:
                status = PHPAL_MIFARE_ERR_NAK0;
                break;

            case PHAL_MFPEVX_RESP_NACK1:
                status = PHPAL_MIFARE_ERR_NAK1;
                break;

            case PHAL_MFPEVX_RESP_NACK4:
                status = PHPAL_MIFARE_ERR_NAK4;
                break;

            case PHAL_MFPEVX_RESP_NACK5:
                status = PHPAL_MIFARE_ERR_NAK5;
                break;

            default:
                status = PH_ERR_PROTOCOL_ERROR;
                break;
        }

        return PH_ADD_COMPCODE(status, PH_COMP_PAL_MIFARE);
    }

    /* Proper error response */
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_WritePerso(void * pPalMifareDataParams, uint8_t bLayer4Comm, uint8_t bWrappedMode, uint8_t bExtendedLenApdu,
    uint16_t wBlockNr, uint8_t bNumBlocks, uint8_t * pValue)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aCmdBuff[3];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Reset the command buffer and its lenght. */
    bCmdLen = 0;
    (void) memset(aCmdBuff, 0x00, sizeof(aCmdBuff));

    /* Frame WritePerso command. */
    aCmdBuff[bCmdLen++] = PHAL_MFPEVX_CMD_WRITEPERSO;
    aCmdBuff[bCmdLen++] = (uint8_t) (wBlockNr & 0x00FFU);           /* LSB */
    aCmdBuff[bCmdLen++] = (uint8_t) ((wBlockNr & 0xFF00) >> 8U);        /* MSB */

    /* Exchange the command and value information to PAL layer. */
    if(0U != bLayer4Comm)
    {
        /* Check if ISO 7816-4 wapping is required */
        if(0U != bWrappedMode)
        {
            /* Buffer the command information to PAL exchange buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_Send7816Apdu(
                pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_FIRST,
                (uint16_t) ((bCmdLen - 1U /* Excluding the command code */) + (bNumBlocks * PHAL_MFPEVX_DATA_BLOCK_SIZE)),
                bExtendedLenApdu,
                aCmdBuff,
                (uint16_t) bCmdLen,  /* Command code is included as part of length. */
                NULL,
                NULL));

            /* Buffer and exchange the block information to PAL layer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_Send7816Apdu(
                pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_LAST,
                0x00,       /* Lc is zero because the length is updated in the first call. */
                bExtendedLenApdu,
                pValue,
                (uint16_t) (bNumBlocks * PHAL_MFPEVX_DATA_BLOCK_SIZE),
                &pResponse,
                &wRespLen));
        }
        else
        {
            /* Buffer the command information to PAL exchange buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_FIRST,
                aCmdBuff,
                bCmdLen,
                &pResponse,
                &wRespLen));

            /* Buffer and exchange the block information to PAL layer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_LAST,
                pValue,
                (uint16_t) (bNumBlocks * PHAL_MFPEVX_DATA_BLOCK_SIZE),
                &pResponse,
                &wRespLen));
        }
    }
    else
    {
        /* Buffer and exchange the block information to PAL layer. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_FIRST,
            aCmdBuff,
            bCmdLen,
            &pResponse,
            &wRespLen));

        /* Buffer and exchange the block information to PAL layer. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_LAST,
            pValue,
            (uint16_t) (bNumBlocks * PHAL_MFPEVX_DATA_BLOCK_SIZE),
            &pResponse,
            &wRespLen));
    }

    /* Verify the status code received from PICC and compute its equivalent error code. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_ComputeErrorResponse(wRespLen, pResponse[0], bLayer4Comm));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_CommitPerso(void * pPalMifareDataParams, uint8_t bOption, uint8_t bLayer4Comm, uint8_t bWrappedMode,
    uint8_t bExtendedLenApdu)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aCmdBuff[2];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Reset the command buffer and its lenght. */
    bCmdLen = 0;
    (void) memset(aCmdBuff, 0x00, sizeof(aCmdBuff));

    /* Frame CommitPerso command. */
    aCmdBuff[bCmdLen++] = PHAL_MFPEVX_CMD_COMMITPERSO;

    /* Exchange the command information to PAL layer. */
    if(bOption != 0U)
    {
        /* Add the option information to comamnd buffer. */
        aCmdBuff[bCmdLen++] = bOption;
    }

    /* exchange command/response */
    if(0U != bLayer4Comm)
    {
        /* Check if ISO 7816-4 wapping is required */
        if(0U != bWrappedMode)
        {
            /* Buffer and exchange the command information to PAL layer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_Send7816Apdu(
                pPalMifareDataParams,
                PH_EXCHANGE_DEFAULT,
                (bCmdLen - 1U /* Excluding the command code */),
                bExtendedLenApdu,
                aCmdBuff,
                bCmdLen,        /* Command code is included as part of length. */
                &pResponse,
                &wRespLen));
        }
        else
        {
            /* Buffer and exchange the command information to PAL layer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pPalMifareDataParams,
                PH_EXCHANGE_DEFAULT,
                aCmdBuff,
                bCmdLen,
                &pResponse,
                &wRespLen));
        }
    }
    else
    {
        /* Buffer and exchange the command information to PAL layer. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
            pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            aCmdBuff,
            bCmdLen,
            &pResponse,
            &wRespLen));
    }

    /* Verify the status code received from PICC and compute its equivalent error code. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_ComputeErrorResponse(wRespLen, pResponse[0], bLayer4Comm));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_ResetAuth(void * pPalMifareDataParams, uint8_t bWrappedMode, uint8_t bExtendedLenApdu)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aCmd[1 /* command code */];
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;

    /* command code */
    aCmd[0] = PHAL_MFPEVX_CMD_RAUTH;

    /* Check if ISO 7816-4 wapping is required. */
    if(0U != bWrappedMode)
    {
        /* command exchange */
        PH_CHECK_SUCCESS_FCT(statusTmp, phalMfpEVx_Int_Send7816Apdu(
            pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            0x00, /* Data is not available. Only command is sent */
            bExtendedLenApdu,
            aCmd,
            1,
            &pResponse,
            &wRxLength));
    }
    else
    {
        /* command exchange */
        PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_DEFAULT,
            aCmd,
            1,
            &pResponse,
            &wRxLength));
    }

    /* check response */
    PH_CHECK_SUCCESS_FCT(statusTmp, phalMfpEVx_Int_ComputeErrorResponse(wRxLength, pResponse[0], 1));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_PersonalizeUid(void * pPalMifareDataParams, uint8_t bUidType)
{
    uint8_t     PH_MEMLOC_REM aCmdBuff[2];
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Frame PersonalizeUID command.*/
    aCmdBuff[0] = PHAL_MFPEVX_CMD_PERSOUID;
    aCmdBuff[1] = bUidType;

    /* Exchange the command information to PICC. */
    return phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCmdBuff,
        2,
        &pResponse,
        &wRespLen);
}

phStatus_t phalMfpEVx_Int_SetConfigSL1(void * pPalMifareDataParams, uint8_t bOption)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM bCmdBuff[3];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Frame the command. */
    bCmdBuff[bCmdLen++] = PHAL_MFPEVX_CMD_SET_CONFIG_SL1;
    bCmdBuff[bCmdLen++] = bOption;

    /* Exchange the command information to PICC */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        bCmdBuff,
        bCmdLen,
        &pResponse,
        &wRespLen));

    PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_ComputeErrorResponse(wRespLen, pResponse[0], PH_OFF));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_ReadSL1TMBlock(void * pPalMifareDataParams, uint16_t wBlockNr, uint8_t * pBlocks)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aCmdBuff[4];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Frame the command buffer. */
    aCmdBuff[bCmdLen++] = PHAL_MFPEVX_CMD_READ_PNU;
    aCmdBuff[bCmdLen++] = (uint8_t) (wBlockNr & 0xFFU); /* LSB */
    aCmdBuff[bCmdLen++] = (uint8_t) (wBlockNr >> 8U);   /* MSB */
    aCmdBuff[bCmdLen++] = 0x01;   /* EXT */

    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCmdBuff,
        bCmdLen,
        &pResponse,
        &wRespLen));

    /* Evaluate the response for any errors. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_ComputeErrorResponseMfc(wRespLen, pResponse[0]));

    /* Check the received length. */
    if(wRespLen != (1U /* Status Code (0x90) */ + PHAL_MFPEVX_DATA_BLOCK_SIZE))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFPEVX);
    }

    /* Copy the received data to the return parameter. */
    (void) memcpy(pBlocks, &pResponse[1], (wRespLen - 1U));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_VCSupportLastISOL3(void * pPalMifareDataParams, uint8_t * pIid, uint8_t * pPcdCapL3, uint8_t * pInfo)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aCmdBuff[21];
    uint8_t     PH_MEMLOC_REM bCmdLen = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Reset the command buffer and its length. */
    bCmdLen = 0;
    (void) memset(aCmdBuff, 0x00, sizeof(aCmdBuff));

    /* Frame the command buffer. */
    aCmdBuff[bCmdLen++] = PHAL_MFPEVX_CMD_VCLAST_ISOL3;
    (void) memcpy(&aCmdBuff[bCmdLen], pIid, 16);
    bCmdLen += (uint8_t) 16U;

    (void) memcpy(&aCmdBuff[bCmdLen], pPcdCapL3, 4);
    bCmdLen += (uint8_t) 4U;

    /* Perform L3 exchange */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCmdBuff,
        bCmdLen,
        &pResponse,
        &wRespLen));

    /* Copy the one byte INFO byte data obtained from card */
    if(wRespLen == 0x01U)
    {
        *pInfo = *pResponse;
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_CreateValueBlock(uint8_t * pValue, uint8_t bAddrData, uint8_t * pBlock)
{
    pBlock[0] = (uint8_t) pValue[0];
    pBlock[1] = (uint8_t) pValue[1];
    pBlock[2] = (uint8_t) pValue[2];
    pBlock[3] = (uint8_t) pValue[3];
    pBlock[4] = (uint8_t) ~(uint8_t) pValue[0];
    pBlock[5] = (uint8_t) ~(uint8_t) pValue[1];
    pBlock[6] = (uint8_t) ~(uint8_t) pValue[2];
    pBlock[7] = (uint8_t) ~(uint8_t) pValue[3];
    pBlock[8] = (uint8_t) pValue[0];
    pBlock[9] = (uint8_t) pValue[1];
    pBlock[10] = (uint8_t) pValue[2];
    pBlock[11] = (uint8_t) pValue[3];
    pBlock[12] = (uint8_t) bAddrData;
    pBlock[13] = (uint8_t) ~(uint8_t) bAddrData;
    pBlock[14] = (uint8_t) bAddrData;

    pBlock[15] = (uint8_t) ~(uint8_t) bAddrData;
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_CheckValueBlockFormat(uint8_t * pBlock)
{
    /* check format of value block */
    if((pBlock[0] != pBlock[8]) ||
        (pBlock[1] != pBlock[9]) ||
        (pBlock[2] != pBlock[10]) ||
        (pBlock[3] != pBlock[11]) ||
        (pBlock[4] != (pBlock[0] ^ 0xFFU)) ||
        (pBlock[5] != (pBlock[1] ^ 0xFFU)) ||
        (pBlock[6] != (pBlock[2] ^ 0xFFU)) ||
        (pBlock[7] != (pBlock[3] ^ 0xFFU)) ||
        (pBlock[12] != pBlock[14]) ||
        (pBlock[13] != pBlock[15]) ||
        (pBlock[12] != (pBlock[13] ^ 0xFFU)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFPEVX);
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phalMfpEVx_Int_Send7816Apdu(void * pPalMifareDataParams, uint16_t wOptions, uint16_t wLc, uint8_t bExtendedLenApdu, uint8_t * pData,
    uint16_t wDataLen, uint8_t ** ppRxBuffer, uint16_t * pRxBufLen)
{
    phStatus_t  PH_MEMLOC_REM wStatus = PH_ERR_SUCCESS;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;
    uint8_t     PH_MEMLOC_REM bLc[3] = { 0x00, 0x00, 0x00 };
    uint8_t     PH_MEMLOC_REM bLe[3] = { 0x00, 0x00, 0x00 };
    uint8_t     PH_MEMLOC_REM bLcLen = 0;
    uint8_t     PH_MEMLOC_REM bBackup = 0;
    uint8_t     PH_MEMLOC_REM bHasCustomBits = 0;
    uint8_t     PH_MEMLOC_REM bISO7816Header[4] = { PHAL_MFPEVX_WRAPPEDAPDU_CLA, 0x00, PHAL_MFPEVX_WRAPPEDAPDU_P1, PHAL_MFPEVX_WRAPPEDAPDU_P2 };

    static uint8_t      PH_MEMLOC_REM bLeLen;

    /* Set the flag if Custom bits are available. */
    bHasCustomBits = (uint8_t) ((wOptions & PH_EXCHANGE_CUSTOM_BITS_MASK) ? PH_ON : PH_OFF);

    /* Mask out the Custom Bits */
    wOptions = (wOptions & (~PH_EXCHANGE_CUSTOM_BITS_MASK));

    if((wOptions == PH_EXCHANGE_BUFFER_FIRST) || (wOptions == PH_EXCHANGE_DEFAULT))
    {
        bLeLen = 1;

        /* Update the command code to the 7816 header */
        bISO7816Header[1] = pData[0];

        /* Add the ISO 7816 header to layer 4 buffer. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_FIRST,
            &bISO7816Header[0],
            PHAL_MFPEVX_WRAP_HDR_LEN,
            NULL,
            NULL));

        /* Add Lc if available */
        if(0U != wLc)
        {
            /* Update Lc bytes according to Extended APDU option. */
            if(0U != bExtendedLenApdu)
            {
                bLc[bLcLen++] = 0x00;
                bLc[bLcLen++] = (uint8_t) ((wLc & 0xFF00U) >> 8U);

                /* Le length is updated to two if Lc is presend and the APDU is extended. */
                bLeLen = 2;
            }

            bLc[bLcLen++] = (uint8_t) (wLc & 0x00FFU);

            /* Add the Lc to layer 4 buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT,
                &bLc[0],
                bLcLen,
                NULL,
                NULL));

            /* Add the data to layer 4 buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT,
                &pData[1], /* Exclude the command code because it is added to INS. */
                (wDataLen - 1U), /* Subtracted - 1 because command code is not available as part of data. */
                NULL,
                NULL));
        }
        else
        {
            /* Update Le count */
            if(0U != bExtendedLenApdu)
            {
                bLeLen = 3;
            }
        }
    }

    if(wOptions == PH_EXCHANGE_BUFFER_CONT)
    {
        /* Add the data to layer 4 buffer. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_CONT,
            pData,
            wDataLen,
            NULL,
            NULL));
    }

    if((wOptions == PH_EXCHANGE_BUFFER_LAST) || (wOptions == PH_EXCHANGE_DEFAULT))
    {
        if(wOptions == PH_EXCHANGE_BUFFER_LAST)
        {
            /* Add the data to layer 4 buffer. */
            PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
                pPalMifareDataParams,
                PH_EXCHANGE_BUFFER_CONT,
                pData,
                wDataLen,
                NULL,
                NULL));
        }

        /* Add Le to L4 buffer and exchange the command. */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            PH_EXCHANGE_BUFFER_LAST,
            &bLe[0],
            bLeLen,
            &pResponse,
            &wRespLen));

        /* Check if it not a response additional frame. */
        if(pResponse[wRespLen - 1U] != PHAL_MFPEVX_RESP_ADDITIONAL_FRAME)
        {
            /* Do not verify the response if Custom Bits are set.
             * The Custom bits will be passed by Sam Non X component for retaining the PICC status code.
             */
            if(0U == bHasCustomBits)
            {
                /* Check for success response */
                PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_ComputeErrorResponse(1, pResponse[wRespLen - 1], PH_ON));
            }
        }

        /* Create memory for updating the response of ISO 14443 format. */
        *ppRxBuffer = pResponse;

        /* This operation is performed to bring back the ISO14443-4 response data format. */

        /* Copy the second byte of response (SW2) to RxBuffer */
        bBackup = pResponse[wRespLen - 1U];

        /* Right shift the response buffer by 1 byte. */
        (void) memmove(&pResponse[1], &pResponse[0], wRespLen - 1U);

        /* Copy the backup data to first position of response buffer. */
        pResponse[0] = bBackup;

        /* Update the response buffer length excluding SW1. */
        *pRxBufLen = wRespLen - 1U;
    }

    if(wOptions == PH_EXCHANGE_RXCHAINING)
    {
        /* Exchange the command */
        PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL4(
            pPalMifareDataParams,
            wOptions,
            pData,
            wDataLen,
            &pResponse,
            &wRespLen));

        if(wRespLen != 0U)
        {
            /* Check if it not a response additional frame. */
            if(pResponse[wRespLen - 1U] != PHAL_MFPEVX_RESP_ADDITIONAL_FRAME)
            {
                /* Check for success response */
                PH_CHECK_SUCCESS_FCT(wStatus, phalMfpEVx_Int_ComputeErrorResponse(1, pResponse[wRespLen - 1], PH_ON));
            }

            /* Create memory for updating the response of ISO 14443 format. */
            *ppRxBuffer = pResponse;

            /* This operation is performed to bring back the ISO14443-4 response data format. */

            /* Copy the second byte of response (SW2) to RxBuffer */
            bBackup = pResponse[wRespLen - 1U];

            /* Right shift the response buffer by 1 byte. */
            (void) memmove(&pResponse[1], &pResponse[0], wRespLen - 1U);

            /* Copy the backup data to first position of response buffer. */
            pResponse[0] = bBackup;

            /* Update the response buffer length excluding SW1. */
            *pRxBufLen = wRespLen - 1U;
        }
    }

    return wStatus;
}
#endif /* NXPBUILD__PHAL_MFPEVX */
