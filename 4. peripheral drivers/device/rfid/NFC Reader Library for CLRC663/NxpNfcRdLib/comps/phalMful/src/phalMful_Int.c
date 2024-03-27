/*----------------------------------------------------------------------------*/
/* Copyright 2009-2020,2022 NXP                                               */
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
* Internal functions of Software implementation of MIFARE Ultralight contactless IC application layer.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#include <ph_Status.h>
#include <phpalMifare.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PHAL_MFUL

#ifdef NXPBUILD__PH_CRYPTOSYM
#include <phCryptoSym.h>
#endif /* NXPBUILD__PH_CRYPTOSYM */

#include <phalMful.h>
#include "phalMful_Int.h"

phStatus_t phalMful_Int_Read(
                             void * pDataParams,
                             uint8_t bAddress,
                             uint8_t * pData
                             )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Process CMAC and exchange the command. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_Exchange(
        pDataParams,
        PHAL_MFUL_CMD_READ,
        &bAddress,
        1,
        &pResponse,
        &wRespLen));

    /* Check received length. */
    if (wRespLen != PHAL_MFUL_READ_BLOCK_LENGTH)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* Copy received data block. */
    (void)memcpy(pData, pResponse, wRespLen);

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_Write(
                              void * pDataParams,
                              uint8_t bAddress,
                              uint8_t * pData
                              )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aDataBuff[5];
    uint8_t     PH_MEMLOC_REM bDataBufLen = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Frame data information. */
    aDataBuff[bDataBufLen++] = bAddress;
    (void)memcpy(&aDataBuff[bDataBufLen], pData, PHAL_MFUL_WRITE_BLOCK_LENGTH);
    bDataBufLen += PHAL_MFUL_WRITE_BLOCK_LENGTH;

    /* Process CMAC and exchange the command. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_Exchange(
        pDataParams,
        PHAL_MFUL_CMD_WRITE,
        aDataBuff,
        bDataBufLen,
        &pResponse,
        &wRespLen));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_FastWrite(
                                 void * pPalMifareDataParams,
                                 uint8_t * pData
                                 )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCommand[3];
    uint8_t *   PH_MEMLOC_REM pRxBuffer = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;

    /* build command frame */
    bCommand[0] = PHAL_MFUL_CMD_FAST_WRITE;
    bCommand[1] = 0xF0; /* Start Address */
    bCommand[2] = 0xFF; /* End Address */

    /* buffer the command frame */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        bCommand,
        3,
        &pRxBuffer,
        &wRxLength
        ));

    /* transmit the data */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        pData,
        (16u * PHAL_MFUL_WRITE_BLOCK_LENGTH),
        &pRxBuffer,
        &wRxLength
        ));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_CompatibilityWrite(
    void * pPalMifareDataParams,
    uint8_t bAddress,
    uint8_t * pData
    )
{
    phStatus_t  PH_MEMLOC_REM status;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCommand[2];
    uint8_t *   PH_MEMLOC_REM pRxBuffer = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;

    /* build command frame */
    bCommand[0] = PHAL_MFUL_CMD_COMPWRITE;
    bCommand[1] = bAddress;

    /* send the first part */
    status = phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        bCommand,
        2,
        &pRxBuffer,
        &wRxLength);

    /* Either ACK (newer UL cards) or TO (older UL cards) is expected */
    if ((status & PH_ERR_MASK) != PH_ERR_IO_TIMEOUT)
    {
        PH_CHECK_SUCCESS(status);
    }

    /* buffer the data */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        pData,
        PHAL_MFUL_COMPWRITE_BLOCK_LENGTH,
        &pRxBuffer,
        &wRxLength));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_IncrCnt(
                                void * pDataParams,
                                uint8_t bCntNum,
                                uint8_t * pCnt
                                )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aDataBuff[5];
    uint8_t     PH_MEMLOC_REM bDataBufLen = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Frame data information. */
    aDataBuff[bDataBufLen++] = bCntNum;
    (void)memcpy(&aDataBuff[bDataBufLen], pCnt, PHAL_MFUL_COUNTER_WR_VALUE_LENGTH);
    bDataBufLen += PHAL_MFUL_COUNTER_WR_VALUE_LENGTH;

    /* Process CMAC and exchange the command. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_Exchange(
        pDataParams,
        PHAL_MFUL_CMD_INCR_CNT,
        aDataBuff,
        bDataBufLen,
        &pResponse,
        &wRespLen));

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_ReadCnt(
                                void * pDataParams,
                                uint8_t bCntNum,
                                uint8_t * pCntValue
                                )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Process CMAC and exchange the command. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_Exchange(
        pDataParams,
        PHAL_MFUL_CMD_READ_CNT,
        &bCntNum,
        1,
        &pResponse,
        &wRespLen));

    /* Check received length. */
    if (wRespLen != PHAL_MFUL_COUNTER_RD_VALUE_LENGTH)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* Copy received data block. */
    (void)memcpy(pCntValue, pResponse, wRespLen);

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_PwdAuth(
                                void * pPalMifareDataParams,
                                uint8_t * pPwd,
                                uint8_t * pPack
                                )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCommand;
    uint8_t *   PH_MEMLOC_REM pRxBuffer = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;

    /* build command frame */
    bCommand = PHAL_MFUL_CMD_PWD_AUTH;

    /* transmit the command frame */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        &bCommand,
        1,
        &pRxBuffer,
        &wRxLength
        ));

    /* transmit the data */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        pPwd,
        PHAL_MFUL_WRITE_BLOCK_LENGTH,
        &pRxBuffer,
        &wRxLength
        ));

    /* check received length */
    if (wRxLength != PHAL_MFUL_PACK_LENGTH)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* copy received data block */
    (void)memcpy(pPack, pRxBuffer, wRxLength);

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_GetVersion(
                                   void * pDataParams,
                                   uint8_t * pVersion
                                   )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Process CMAC and exchange the command. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_Exchange(
        pDataParams,
        PHAL_MFUL_CMD_GET_VER,
        NULL,
        0,
        &pResponse,
        &wRespLen));

    /* Check received length. */
    if (wRespLen != PHAL_MFUL_VERSION_LENGTH)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* Copy received data block. */
    (void)memcpy(pVersion, pResponse, wRespLen);

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_FastRead(
                                 void * pDataParams,
                                 uint8_t  bStartAddr,
                                 uint8_t bEndAddr,
                                 uint8_t ** ppData,
                                 uint16_t * pNumBytes
                                 )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aDataBuff[5];
    uint8_t     PH_MEMLOC_REM bDataBufLen = 0;

    /* Frame data information. */
    aDataBuff[bDataBufLen++] = bStartAddr;
    aDataBuff[bDataBufLen++] = bEndAddr;

    /* Process CMAC and exchange the command. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_Exchange(
        pDataParams,
        PHAL_MFUL_CMD_FAST_READ,
        aDataBuff,
        bDataBufLen,
        ppData,
        pNumBytes));

    /* check received length */
    if (*pNumBytes != ((uint16_t) ((bEndAddr - bStartAddr + 1) * 4)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_SectorSelect(
                                     void * pPalMifareDataParams,
                                     uint8_t bSecNo
                                     )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCommand1[2];
    uint8_t     PH_MEMLOC_REM bCommand2[4];
    uint8_t *   PH_MEMLOC_REM pRxBuffer = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;

    /* build command frame packet 1 */
    bCommand1[0] = PHAL_MFUL_CMD_SECTOR_SELECT;
    bCommand1[1] = 0xFF;

    /* transmit the command frame packet 1 */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        bCommand1,
        2,
        &pRxBuffer,
        &wRxLength
        ));

    /* Build command frame packet 2 */
    bCommand2[0] = bSecNo;
    bCommand2[1] = 0x00;
    bCommand2[2] = 0x00;
    bCommand2[3] = 0x00;

    /* transmit the command frame packet 2 */
    statusTmp = phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        bCommand2,
        4,
        &pRxBuffer,
        &wRxLength
        );

    /* No response expected for packet 2 */
    if((statusTmp & PH_ERR_MASK) != PH_ERR_IO_TIMEOUT)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_ReadSign(
                                 void * pDataParams,
                                 uint8_t bAddr,
                                 uint8_t ** pSignature,
                                 uint16_t* pDataLen
                                 )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;

    /* Process CMAC and exchange the command. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_Exchange(
        pDataParams,
        PHAL_MFUL_CMD_READ_SIG,
        &bAddr,
        1,
        pSignature,
        pDataLen));

    /* check received length */
    if((*pDataLen != PHAL_MFUL_SIG_LENGTH )
        && (*pDataLen != PHAL_MFUL_SIG_LENGTH_48)
        )
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

#ifdef NXPBUILD__PH_CRYPTOSYM
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_SetConfig(pDataParams, PHAL_MFUL_ADDITIONAL_INFO, *pDataLen));
#endif /* NXPBUILD__PH_CRYPTOSYM */

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_ChkTearingEvent(
                                        void * pPalMifareDataParams,
                                        uint8_t bCntNum,
                                        uint8_t * pValidFlag
                                        )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bCommand[2];
    uint8_t *   PH_MEMLOC_REM pRxBuffer = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;

    /* build command frame */
    bCommand[0] = PHAL_MFUL_CMD_CHK_TRG_EVT;
    bCommand[1] = bCntNum;

    /* transmit the command frame */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        bCommand,
        2,
        &pRxBuffer,
        &wRxLength
        ));


    /* check received length */
    if (wRxLength != 1U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* copy received data block */
    (void)memcpy(pValidFlag, pRxBuffer, wRxLength);


    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_WriteSign(
                                  void * pDataParams,
                                  uint8_t bAddress,
                                  uint8_t * pSignature
                                  )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t     PH_MEMLOC_REM aDataBuff[5];
    uint8_t     PH_MEMLOC_REM bDataBufLen = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Frame data information. */
    aDataBuff[bDataBufLen++] = bAddress;
    (void)memcpy(&aDataBuff[bDataBufLen], pSignature, PHAL_MFUL_WRITE_BLOCK_LENGTH);
    bDataBufLen += PHAL_MFUL_WRITE_BLOCK_LENGTH;

    /* Process CMAC and exchange the command. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_Exchange(
        pDataParams,
        PHAL_MFUL_CMD_WRITE_SIGN,
        aDataBuff,
        bDataBufLen,
        &pResponse,
        &wRespLen));

     return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_LockSign(
                                 void * pDataParams,
                                 uint8_t bLockMode
                                 )
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
    uint8_t *   PH_MEMLOC_REM pResponse = NULL;
    uint16_t    PH_MEMLOC_REM wRespLen = 0;

    /* Process CMAC and exchange the command. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_Exchange(
        pDataParams,
        PHAL_MFUL_CMD_LOCK_SIGN,
        &bLockMode,
        1,
        &pResponse,
        &wRespLen));

     return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_VirtualCardSelect(
                                          void * pPalMifareDataParams,
                                          uint8_t * pVCIID,
                                          uint8_t bVCIIDLen,
                                          uint8_t * pVCTID
                                          )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aCommand[1];
    uint8_t *   PH_MEMLOC_REM pRxBuffer = NULL;
    uint16_t    PH_MEMLOC_REM wRxLength = 0;

    /*Frame the command buffer. */
    aCommand[0] = PHAL_MFUL_CMD_VCSL;

    /* Transmit the command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        aCommand,
        1,
        &pRxBuffer,
        &wRxLength
        ));

    /* Transmit the IID data. */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        pVCIID,
        bVCIIDLen,
        &pRxBuffer,
        &wRxLength
        ));

    /* check received length */
    if (wRxLength != 1U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* copy received data block */
    (void)memcpy(pVCTID, pRxBuffer, wRxLength);


    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_ReadTTStatus(void * pPalMifareDataParams, uint8_t bAddr, uint8_t * pData)
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM aCommand[2];
    uint8_t *   PH_MEMLOC_REM pRxBuffer;
    uint16_t    PH_MEMLOC_REM wRxLength;

    /*Frame the command buffer. */
    aCommand[0] = PHAL_MFUL_CMD_READ_TT_STATUS;
    aCommand[1] = bAddr;

    /* Transmit the command */
    PH_CHECK_SUCCESS_FCT(statusTmp, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_DEFAULT,
        aCommand,
        0x02,
        &pRxBuffer,
        &wRxLength
        ));

    /* check received length */
    if (wRxLength != 5)
    {
        return PH_ADD_COMPCODE(PH_ERR_PROTOCOL_ERROR, PH_COMP_AL_MFUL);
    }

    /* copy received data block */
    memcpy(pData, pRxBuffer, wRxLength);

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_Exchange (void * pDataParams, uint8_t bCmd, uint8_t * pData, uint8_t bDataLen,
    uint8_t ** ppResponse, uint16_t * pRespLen)
{
    phStatus_t  PH_MEMLOC_REM wStatus = 0;
#ifdef NXPBUILD__PH_CRYPTOSYM
    uint8_t     PH_MEMLOC_REM aMac[PH_CRYPTOSYM_AES_BLOCK_SIZE];
#else
#define aMac NULL
#endif /* NXPBUILD__PH_CRYPTOSYM */
    uint8_t     PH_MEMLOC_REM bMacLen = 0;
    void *      PH_MEMLOC_REM pPalMifareDataParams = NULL;

#ifdef NXPBUILD__PH_CRYPTOSYM
    uint8_t     PH_MEMLOC_REM bCMACVal = 0;
    uint8_t     PH_MEMLOC_REM bAUTHMODE = 0;
    uint8_t     PH_MEMLOC_REM aInBuff[256];
    uint8_t     PH_MEMLOC_REM bInBufLen = 0;
    uint8_t     PH_MEMLOC_REM aIV[PH_CRYPTOSYM_AES_BLOCK_SIZE];
    /* Set the buffers Buffer to zero. */
    (void)memset(aIV, 0x00, sizeof(aIV));
#endif /* NXPBUILD__PH_CRYPTOSYM */

    /* Get the PAL Dataparams. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_GetPalDataParams(pDataParams, &pPalMifareDataParams));

#ifdef NXPBUILD__PH_CRYPTOSYM
    /* Get the CMAC Value Enabled or Disabled. */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_GetCMAC(pDataParams, &bCMACVal));

    /* Get the Auth Mode */
    PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_GetAUTHMODE(pDataParams, &bAUTHMODE));

    /* Check if CMAC is enabled. */
    if(bCMACVal)
    {
        /* Check if its Authenticated. */
        if(bAUTHMODE == PHAL_MFUL_CMD_AUTH)
        {
            /* Compute CMAC for Software mode */
            if(PH_GET_COMPID(pDataParams) == PHAL_MFUL_SW_ID)
            {
                /* Frame the MAC input buffer.
                    * wCmdCtr || Cmd || CmdData
                    */
                aInBuff[bInBufLen++] = (uint8_t) (((phalMful_Sw_DataParams_t *) pDataParams)->wCmdCtr);
                aInBuff[bInBufLen++] = (uint8_t) (((phalMful_Sw_DataParams_t *) pDataParams)->wCmdCtr >> 8);
                aInBuff[bInBufLen++] = bCmd;
                (void)memcpy(&aInBuff[bInBufLen], pData, bDataLen);
                bInBufLen += bDataLen;

                /* Load the IV. */
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadIv(
                    ((phalMful_Sw_DataParams_t *) pDataParams)->pCryptoDataParams,
                    aIV,
                    PH_CRYPTOSYM_AES_BLOCK_SIZE));

                /* Compute the MAC. */
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_CalculateMac(
                    ((phalMful_Sw_DataParams_t *) pDataParams)->pCryptoDataParams,
                    (uint16_t) (PH_CRYPTOSYM_MAC_MODE_CMAC | PH_EXCHANGE_DEFAULT),
                    aInBuff,
                    bInBufLen,
                    aMac,
                    &bMacLen));

                /* Truncate the MAC generated */
                PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_TruncateMac(aMac));
                bMacLen = 8; /* Truncated len. */

                /* Reset Input buffer length. */
                bInBufLen = 0;
            }
            else
            {
                /* Nothing to do. */
            }
        }
    }
    else
    {
        /* Nothing to do. */
    }
#endif /* NXPBUILD__PH_CRYPTOSYM */

    /* Buffer the command information. */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_FIRST,
        &bCmd,
        1,
        NULL,
        NULL));

    /* Buffer the data information. */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_CONT,
        pData,
        bDataLen,
        NULL,
        NULL));

    /* Buffer the mac information and exchange the bufferred information. */
    PH_CHECK_SUCCESS_FCT(wStatus, phpalMifare_ExchangeL3(
        pPalMifareDataParams,
        PH_EXCHANGE_BUFFER_LAST,
        aMac,
        bMacLen,
        ppResponse,
        pRespLen));

#ifdef NXPBUILD__PH_CRYPTOSYM
    /* Check if CMAC is enabled. */
    if(bCMACVal)
    {
        /* Check if its Authenticated. */
        if(bAUTHMODE == PHAL_MFUL_CMD_AUTH)
        {
            /* Verify CMAC for Software mode only.*/
            if(PH_GET_COMPID(pDataParams) == PHAL_MFUL_SW_ID)
            {
                /* Increment the command counter. */
                ((phalMful_Sw_DataParams_t *) pDataParams)->wCmdCtr++;

                /* Frame the MAC input buffer.
                 * wCmdCtr || CmdData
                 */
                aInBuff[bInBufLen++] = (uint8_t) (((phalMful_Sw_DataParams_t *) pDataParams)->wCmdCtr);
                aInBuff[bInBufLen++] = (uint8_t) (((phalMful_Sw_DataParams_t *) pDataParams)->wCmdCtr >> 8);
                (void)memcpy(&aInBuff[bInBufLen], *ppResponse, (*pRespLen - 8U));
                bInBufLen += (uint8_t) (*pRespLen - 8U);

                /* Load the IV. */
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_LoadIv(
                    ((phalMful_Sw_DataParams_t *) pDataParams)->pCryptoDataParams,
                    aIV,
                    PH_CRYPTOSYM_AES_BLOCK_SIZE));

                /* Compute the MAC. */
                PH_CHECK_SUCCESS_FCT(wStatus, phCryptoSym_CalculateMac(
                    ((phalMful_Sw_DataParams_t *) pDataParams)->pCryptoDataParams,
                    (uint16_t) (PH_CRYPTOSYM_MAC_MODE_CMAC | PH_EXCHANGE_DEFAULT),
                    aInBuff,
                    bInBufLen,
                    aMac,
                    &bMacLen));

                /* Truncate the MAC generated */
                PH_CHECK_SUCCESS_FCT(wStatus, phalMful_Int_TruncateMac(aMac));
                bMacLen = 8; /* Truncated len. */

                /* Compare the MAC. */
                if (memcmp(&ppResponse[0][*pRespLen - 8], aMac, 8) != 0)
                {
                    return PH_ADD_COMPCODE(PH_ERR_INTEGRITY_ERROR, PH_COMP_AL_MFUL);
                }

                /* Increment the command counter. */
                ((phalMful_Sw_DataParams_t *) pDataParams)->wCmdCtr++;

                /* Update the length to remove the mac from response. */
                *pRespLen -= 8;
            }
            else
            {
                /* Nothing to do. */
            }
        }
    }
    else
    {
        /* Nothing to do. */
    }
#endif /* NXPBUILD__PH_CRYPTOSYM */

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_GetPalDataParams(void * pDataParams, void** pPalMifareDataParams)
{
    switch(PH_GET_COMPID(pDataParams))
    {
        case PHAL_MFUL_SW_ID:
            *pPalMifareDataParams = ((phalMful_Sw_DataParams_t *) pDataParams)->pPalMifareDataParams;
            break;
        default:
            *pPalMifareDataParams = NULL;
            break;
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalMful_Int_GetCMAC(void * pDataParams, uint8_t* pCMACVal)
{
    switch(PH_GET_COMPID(pDataParams))
    {
        case PHAL_MFUL_SW_ID:
            *pCMACVal = ((phalMful_Sw_DataParams_t *) pDataParams)->bCMACReq;
            break;
        default:
            *pCMACVal = 0;
            break;
    }

    return PH_ERR_SUCCESS;
}
phStatus_t phalMful_Int_GetAUTHMODE(void * pDataParams, uint8_t* pAUTHMODE)
{
    switch(PH_GET_COMPID(pDataParams))
    {
        case PHAL_MFUL_SW_ID:
            *pAUTHMODE = ((phalMful_Sw_DataParams_t *) pDataParams)->bAuthMode;
            break;
        default:
            *pAUTHMODE = 0;
            break;
    }

    return PH_ERR_SUCCESS;
}
phStatus_t phalMful_Int_TruncateMac(uint8_t * pMac)
{
    uint8_t PH_MEMLOC_REM bIndex;
    uint8_t PH_MEMLOC_REM bIndex2;

    for (bIndex = 1, bIndex2 = 0; bIndex < 16; bIndex += 2, bIndex2++)
    {
        pMac[bIndex2] = pMac[bIndex];
    }

    return PH_ERR_SUCCESS;
}
#endif /* NXPBUILD__PHAL_MFUL */
