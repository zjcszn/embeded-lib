/*----------------------------------------------------------------------------*/
/* Copyright 2019-2023 NXP                                                    */
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
* Instruction Manager routines for Pn5190 HAL.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <ph_RefDefs.h>
#include <ph_Status.h>
#include <phhalHw.h>

#ifdef NXPBUILD__PHHAL_HW_PN5190
#ifndef _WIN32
#include <phOsal.h>
#include <phDriver.h>
#include "BoardSelection.h"
#else

#define PH_DRIVER_SET_LOW  0
#define PH_DRIVER_SET_HIGH 1

#define RESET_POWERDOWN_LEVEL  PH_DRIVER_SET_LOW
#define RESET_POWERUP_LEVEL    PH_DRIVER_SET_HIGH

#ifndef true
#define true 1
#endif


#ifndef false
#define false 0
#endif


#endif /* _WIN32 */

#include <phTools.h>
#include "phhalHw_Pn5190.h"
#include <phhalHw_Pn5190_Instr.h>
#include "phhalHw_Pn5190_Int.h"
#include "phhalHw_Pn5190_InstrMngr.h"
#include "phhalHw_Pn5190_Wait.h"

/* 1 Byte Command Code + 2 Bytes of Length*/
#define CMD_HEADER_LONG         3U

#define SEND                    0x01U
#define QUEUE                   0x02U
#define QUEUE_N_SEND            0x03U

static phStatus_t phhalHw_Pn5190_InstMngr_Exchange(phhalHw_Pn5190_DataParams_t *pDataParams,
            phhalHw_InstMngr_CmdParams_t *pCmdParams,uint8_t bExchAction);

static phStatus_t phhalHw_Pn5190_InstMngr_SendBuffdData(phhalHw_Pn5190_DataParams_t *pDataParams);

static uint16_t   phhalHw_Pn5190_InstMngr_PrepSnglTlv(phhalHw_Pn5190_DataParams_t *pDataParams,
            phhalHw_InstMngr_CmdParams_t *pCmdParams, uint16_t wTxBuff_Offset, uint8_t *pTxBuffer);

static phStatus_t phhalHw_Pn5190_InstMngr_SendSnglTlv(phhalHw_Pn5190_DataParams_t *pDataParams,
            uint16_t wSnglTLVLenght);

static uint8_t    phhalHw_Pn5190_InstMngr_CheckBuffSpace(phhalHw_Pn5190_DataParams_t *pDataParams,
            uint16_t wCurrSpaceNeeded);

static phStatus_t phhalHw_Pn5190_InstMngr_Procss_Cmds(phhalHw_Pn5190_DataParams_t *pDataParams,
            phhalHw_InstMngr_CmdParams_t *pCmdParams,uint16_t wOptions);

static phStatus_t phhalHw_Pn5190_InstMngr_PrepTlv_RFData(phhalHw_Pn5190_DataParams_t *pDataParams,
            phhalHw_InstMngr_CmdParams_t *pCmdParams,uint8_t * pTxBuffer, uint8_t bExchAction);

static phStatus_t phhalHw_Pn5190_InstMngr_Transceive(phhalHw_Pn5190_DataParams_t *pDataParams,
            phhalHw_InstMngr_CmdParams_t *pCmdParams, uint16_t wTlv_Length);


phStatus_t phhalHw_Pn5190_InstMngr_HandleCmd(phhalHw_Pn5190_DataParams_t *pDataParams,
        phhalHw_InstMngr_CmdParams_t *pCmdParams,uint16_t wOptions)
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint16_t    PH_MEMLOC_REM wCurrTotalTxLength = 0U;
    uint8_t     PH_MEMLOC_REM bSpaceAvailable = false;

#ifdef DEBUG
    if((pCmdParams->bCmd != PHHAL_HW_PN5190_INSTR_FIELD_OFF) &&
        (pCmdParams->bCmd != PHHAL_HW_PN5190_INSTR_EXCHANGE_RF_DATA) &&
        (pCmdParams->bCmd != PHHAL_HW_PN5190_INSTR_TRANSMIT_RF_DATA) &&
        (pCmdParams->bCmd != PHHAL_HW_PN5190_INSTR_RETRIEVE_RF_DATA) &&
        (pCmdParams->bCmd != PHHAL_HW_PN5190_INSTR_RETRIEVE_RF_FELICA_EMD_DATA) &&
        (pCmdParams->bCmd != PHHAL_HW_PN5190_INSTR_GET_DIE_ID) &&
        (pCmdParams->bCmd != PHHAL_HW_PN5190_INSTR_GET_VERSION) &&
        (pCmdParams->bCmd != PHHAL_HW_PN5190_INSTR_GET_CRC_USER_AREA_EEPROM) &&
        (pCmdParams->bCmd != PHHAL_HW_PN5190_INSTR_CONFIGURE_MUL_TESTBUS_DIGITAL))
    {
        if (pCmdParams->wTxDataLength == 0U)
        {
            return PH_ERR_INVALID_PARAMETER;
        }
    }
#endif

    wCurrTotalTxLength = pCmdParams->wTxDataLength + pCmdParams->wAddnDataLen + CMD_HEADER_LONG + 1;

    bSpaceAvailable = phhalHw_Pn5190_InstMngr_CheckBuffSpace(pDataParams,wCurrTotalTxLength);

    if(bSpaceAvailable == false)
    {
        pDataParams->wTxBufLen = 0U;
        pDataParams->wTxBufStartPos = 0U;

        status =  PH_ERR_BUFFER_OVERFLOW;
    }
    else
    {
        status = phhalHw_Pn5190_InstMngr_Procss_Cmds(pDataParams, pCmdParams, wOptions);
    }

    return status;
}


static phStatus_t phhalHw_Pn5190_InstMngr_Procss_Cmds(phhalHw_Pn5190_DataParams_t *pDataParams,
    phhalHw_InstMngr_CmdParams_t *pCmdParams, uint16_t wOptions)
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint8_t    PH_MEMLOC_REM bExchAction = QUEUE_N_SEND;

    if( (pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_EXCHANGE_RF_DATA) ||
        (pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_TRANSMIT_RF_DATA) )
    {
        pDataParams->bNonRF_Cmd = 0U;

        if(wOptions & PH_EXCHANGE_BUFFERED_BIT)
        {
            bExchAction = QUEUE;
        }
    }
    else
    {
        pDataParams->bNonRF_Cmd = 1U;
    }
    status = phhalHw_Pn5190_InstMngr_Exchange(pDataParams, pCmdParams, bExchAction);

    return status;
}


static phStatus_t phhalHw_Pn5190_InstMngr_Exchange(phhalHw_Pn5190_DataParams_t *pDataParams,
        phhalHw_InstMngr_CmdParams_t *pCmdParams, uint8_t bExchAction)
{

    phStatus_t  PH_MEMLOC_REM  status = PH_ERR_SUCCESS;
    uint16_t    PH_MEMLOC_REM  wSnglTLVLenght = 0U;
    uint16_t  PH_MEMLOC_REM  wTxBufferLen = 0U;
    uint16_t  PH_MEMLOC_REM  wTxBufferSize = 0U;
    uint8_t   PH_MEMLOC_REM  * pTxBuffer = NULL;

    if(bExchAction & QUEUE)
    {
        if(pDataParams->bNonRF_Cmd)
        {
            /*RF_Exchange is in Progress, prep TLV in different Buff */
            wSnglTLVLenght = phhalHw_Pn5190_InstMngr_PrepSnglTlv(pDataParams,pCmdParams,0,
                     &pDataParams->pInstrBuffer[1]);
        }
        else
        {
            (void)phhalHw_Pn5190_Int_GetTxBuffer(pDataParams, &pTxBuffer, &wTxBufferLen, &wTxBufferSize);

            phhalHw_Pn5190_InstMngr_PrepTlv_RFData(pDataParams,pCmdParams,pTxBuffer,bExchAction);
        }
    }

    if (bExchAction & SEND)
    {
        status = phhalHw_Pn5190_InstMngr_Transceive(pDataParams, pCmdParams, wSnglTLVLenght);
    }

    return status;
}


static phStatus_t phhalHw_Pn5190_InstMngr_PrepTlv_RFData(phhalHw_Pn5190_DataParams_t *pDataParams,
        phhalHw_InstMngr_CmdParams_t *pCmdParams,uint8_t * pTxBuffer, uint8_t bExchAction)
{
    uint8_t   PH_MEMLOC_REM  bTxBuffOffset = 0U;
    uint8_t   PH_MEMLOC_REM  bCmd = pCmdParams->bCmd;
    uint8_t   PH_MEMLOC_REM * pData = pCmdParams->pTxDataBuff;
    uint16_t  PH_MEMLOC_REM  wDataLength = pCmdParams->wTxDataLength;

    if (pDataParams->wTxBufStartPos == 0U )
    {
        pTxBuffer[pDataParams->wTxBufStartPos++] = bCmd; /* Reserved 0th Location of buffer for Command code */
    }
    else
    {
        pDataParams->pTxBuffer[0] = bCmd;
    }

    bTxBuffOffset = 2;

    /* Copy Additional data required by the TLV Command that needs to be sent to PN5190. */
    if(pCmdParams->wAddnDataLen)
    {
        (void)memcpy(&pTxBuffer[pDataParams->wTxBufStartPos + bTxBuffOffset],
                    pCmdParams->pAddnData, pCmdParams->wAddnDataLen);

        pDataParams->wTxFwDataLen =  pCmdParams->wAddnDataLen;
    }

    if (pCmdParams->pTxDataBuff != NULL)
    {
        (void)memcpy(&pTxBuffer[pDataParams->wTxBufStartPos + pDataParams->wTxBufLen
                                + pDataParams->wTxFwDataLen + bTxBuffOffset],
                    pData, wDataLength);

        pDataParams->wTxBufLen += wDataLength;
    }

    if(SEND & bExchAction)
    {
         pDataParams->wTxBufLen += pDataParams->wTxFwDataLen;
         /*In the last we are updating the length*/
         pTxBuffer[pDataParams->wTxBufStartPos++] = ((pDataParams->wTxBufLen & 0xff00U) >> 8);

         pTxBuffer[pDataParams->wTxBufStartPos++] = (pDataParams->wTxBufLen & 0x00ffU);

         /* this is the last packet to add for the RF_EXchange the*/
         pDataParams->wTxBufLen += pDataParams->wTxBufStartPos;

         pDataParams->wTxFwDataLen = 0;
    }

    return PH_ERR_SUCCESS;
}


static uint16_t  phhalHw_Pn5190_InstMngr_PrepSnglTlv(phhalHw_Pn5190_DataParams_t *pDataParams,
        phhalHw_InstMngr_CmdParams_t *pCmdParams, uint16_t wTxBuff_Offset, uint8_t *pTxBuffer)
{
    uint16_t    PH_MEMLOC_REM  wDataLength = 0U;
    uint16_t    PH_MEMLOC_REM  wTxIndex = wTxBuff_Offset;

    pTxBuffer[wTxIndex++] = pCmdParams->bCmd;

    /*2-Bytes for length*/
    wTxIndex ++;
    wTxIndex ++;

    if (pCmdParams->pTxDataBuff != NULL)
    {
        /* Copy additional data passed from Instruction Layer. */
        if(pCmdParams->wAddnDataLen)
        {
            (void)memcpy(&pTxBuffer[wTxIndex],
                    pCmdParams->pAddnData, pCmdParams->wAddnDataLen);

            wTxIndex +=  pCmdParams->wAddnDataLen;
            wDataLength += pCmdParams->wAddnDataLen;
        }
        (void)memcpy(&pTxBuffer[wTxIndex],
                pCmdParams->pTxDataBuff, pCmdParams->wTxDataLength);
    }
    wTxIndex +=  pCmdParams->wTxDataLength;
    wDataLength += pCmdParams->wTxDataLength;

    pTxBuffer[wTxBuff_Offset + 1] = (uint8_t)((wDataLength & 0xff00U) >> 8);

    pTxBuffer[wTxBuff_Offset + 2] = (uint8_t) (wDataLength & 0x00ffU);

    return wTxIndex;
}


static phStatus_t  phhalHw_Pn5190_InstMngr_Transceive(phhalHw_Pn5190_DataParams_t *pDataParams,
        phhalHw_InstMngr_CmdParams_t *pCmdParams, uint16_t wTlv_Length)
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    phOsal_EventBits_t PH_MEMLOC_REM dwReceivedEvents;
    uint8_t     PH_MEMLOC_REM bAbortCMDExe = 0;

    if (pDataParams->bNonRF_Cmd != 0x00U)
    {
        status = phhalHw_Pn5190_InstMngr_SendSnglTlv(pDataParams,wTlv_Length);
    }
    else
    {
        status = phhalHw_Pn5190_InstMngr_SendBuffdData(pDataParams);

        /*Reset the TxBuffer*/
        pDataParams->wTxBufLen = 0U;
        pDataParams->wTxBufStartPos = 0U;
    }

    if (status == PH_ERR_SUCCESS)
    {
        /* Now wait for the IRQ */
        status = phhalHw_Pn5190_WaitIrq(pDataParams, &dwReceivedEvents);

#ifndef _WIN32
        if (((status & PH_ERR_MASK) == PH_ERR_ABORTED) || ((status & PH_ERR_MASK) == PH_OSAL_IO_TIMEOUT))
        {
            if (pDataParams->wTargetMode != PH_OFF)
            {
                pDataParams->bCmdAborted = PH_ON;
            }
            PH_CHECK_SUCCESS_FCT(status, phhalHw_Pn5190_Int_UserAbort(pDataParams, &bAbortCMDExe));

            PH_CHECK_SUCCESS_FCT(status, phhalHw_Pn5190_Wait_IrqAbortHandling(pDataParams, pCmdParams, bAbortCMDExe));

            return PH_ERR_ABORTED;
        }
        else
#endif
        {
            PH_CHECK_SUCCESS(status);

            phOsal_EventClear(&pDataParams->HwEventObj.EventHandle, E_OS_EVENT_OPT_NONE, E_PH_OSAL_EVT_RF, NULL);

            status = phhalHw_Pn5190_InstMngr_ReadRsp(pDataParams, pCmdParams);
        }
    }

    return status;
}

/*
 * phhalHw_Pn5190_InstMngr_SendBuffdData is helper function to transmits the
 * buffered data from Tx_Buffer using BAL interface.
 */
static phStatus_t phhalHw_Pn5190_InstMngr_SendBuffdData(phhalHw_Pn5190_DataParams_t *pDataParams)
{
    uint16_t    PH_MEMLOC_REM wBuffrdDataLength = 0U;
    uint16_t    PH_MEMLOC_REM wBufferSize = 0U;
    uint8_t     PH_MEMLOC_REM * pTxBuffer = NULL;

    (void)phhalHw_Pn5190_Int_GetTxBuffer(pDataParams, &pTxBuffer, &wBuffrdDataLength, &wBufferSize);

    pTxBuffer--;
    *pTxBuffer = PHHAL_HW_PN5190_INT_SPI_WRITE;

    return phhalHw_Pn5190_Send(pDataParams, pTxBuffer, (wBuffrdDataLength + 1), PH_ON);
}


static phStatus_t  phhalHw_Pn5190_InstMngr_SendSnglTlv(phhalHw_Pn5190_DataParams_t *pDataParams,
        uint16_t wSnglTLVLenght)
{
    pDataParams->pInstrBuffer[0] = PHHAL_HW_PN5190_INT_SPI_WRITE;

    return phhalHw_Pn5190_Send(pDataParams, &pDataParams->pInstrBuffer[0], wSnglTLVLenght + 1, PH_ON);
}

/*
 *  phhalHw_Pn5190_InstMngr_ReadRsp processes the data read by HAL IsrCallback.
 *  if response is not complete, it reads remaining number of byte from the Front End.
 */
phStatus_t  phhalHw_Pn5190_InstMngr_ReadRsp(phhalHw_Pn5190_DataParams_t *pDataParams,
        phhalHw_InstMngr_CmdParams_t *pCmdParams)
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM statusTmp = PH_ERR_SUCCESS;
    uint16_t    PH_MEMLOC_REM wRespLength = 0U;
    uint16_t    PH_MEMLOC_REM wRemRespLength = 0U;
    uint16_t    PH_MEMLOC_REM wCopyRespLength = 0U;
    int16_t     PH_MEMLOC_REM wDataLenToRead = 0U;
    uint8_t     PH_MEMLOC_REM bIndex = 0U;
    uint8_t     PH_MEMLOC_REM * pTmpRxBuffer = NULL;
    uint8_t     PH_MEMLOC_REM bHeaderLen = 0U;
    uint8_t     PH_MEMLOC_REM bBackUp = 0U;
    uint8_t     PH_MEMLOC_REM * pIsrBuffPtr = NULL;

    /*
     * TLV Definition:
     * 0th location - FF (Direction Byte)
     * 1th location - Type Field
     * 2th location - RspLength MSB
     * 3nd location - RspLength LSB
     * 4th location - RspStatus (Value 0)
     * 5th location - Value 1 */
    bIndex = 2;
    pIsrBuffPtr = pDataParams->sIrqResp.pIsrBuffPtr;

    /* Response length is of 2 byte @bIndex  1 and 2 */
    wRespLength = pIsrBuffPtr[bIndex++];
    wRespLength <<= 8;
    wRespLength |= pIsrBuffPtr[bIndex++];

    if(wRespLength > pDataParams->wRxBufSize)
    {
        return PH_ERR_BUFFER_OVERFLOW;
    }

    /* Calculate the no of bytes to be read */
    bHeaderLen = TYPE_FIELD_LEN + LENGTH_FIELD_LEN + DIRECTION_BYTE_LEN;
    wDataLenToRead = wRespLength - (pDataParams->sIrqResp.bIsrBytesRead - bHeaderLen);

    /* Get Response Status byte. */
    status = (uint16_t)pIsrBuffPtr[bIndex++];

    if (wRespLength > STATUS_FIELD_LEN)
    {
        do
        {
            if( pCmdParams == NULL )
            {
                status = PH_ERR_INTERNAL_ERROR;
                break;
            }

            wRemRespLength = (wDataLenToRead > 0) ? wDataLenToRead : 0x00U;
            wCopyRespLength = (((wDataLenToRead > 0) ? (pDataParams->sIrqResp.bIsrBytesRead - bHeaderLen) : wRespLength) - STATUS_FIELD_LEN);

            if ((pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_RETRIEVE_RF_DATA) ||
                (pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_RETRIEVE_RF_FELICA_EMD_DATA) ||
                (pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_CTS_RETRIEVE_LOG) ||
                (pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_READ_E2PROM) ||
                (pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_EPC_GEN2_INVENTORY) ||
                (pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_GET_DIE_ID) ||
                (pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_GET_VERSION) ||
                (pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_GET_RF_CONFIGURATION))
            {
                pTmpRxBuffer = * pCmdParams->ppRxBuffer;

                if (wCopyRespLength != 0)
                {
                    (void)memcpy(pTmpRxBuffer, &pIsrBuffPtr[bIndex], wCopyRespLength);
                    pTmpRxBuffer += wCopyRespLength;
                }

                if (wRemRespLength != 0)
                {
                    --pTmpRxBuffer;
                    bBackUp = *pTmpRxBuffer;

                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Read(pDataParams,
                            pTmpRxBuffer, (wRemRespLength + DIRECTION_BYTE_LEN)));

                    *pTmpRxBuffer = bBackUp;
                }

                pTmpRxBuffer = (*pCmdParams->ppRxBuffer) - 1;
            }
            else
            {
                pTmpRxBuffer = pIsrBuffPtr;

                if (wRemRespLength != 0)
                {
                    --pTmpRxBuffer;
                    bBackUp = *(&pTmpRxBuffer[pDataParams->sIrqResp.bIsrBytesRead]);

                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Read(pDataParams,
                        &pTmpRxBuffer[pDataParams->sIrqResp.bIsrBytesRead], wRemRespLength + DIRECTION_BYTE_LEN));

                    *(&pTmpRxBuffer[pDataParams->sIrqResp.bIsrBytesRead]) = bBackUp;
                }
                pTmpRxBuffer = &pIsrBuffPtr[bHeaderLen];
            }
            /* pre-incrementing the Buffer pointer to remove the SPI read header */
            *pCmdParams->ppRxBuffer = ++pTmpRxBuffer;
            *pCmdParams->pRxLength = wRespLength - STATUS_FIELD_LEN;

        }while(0);
    }
    return status;
}


phStatus_t phhalHw_Pn5190_Send(phhalHw_Pn5190_DataParams_t *pDataParams,
        uint8_t * pTxBuffer, uint16_t wTxLength, uint8_t bCheckIRQ)
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_IO_TIMEOUT;
    uint16_t    PH_MEMLOC_REM wResponseLen = 0U;
    phOsal_EventBits_t PH_MEMLOC_REM dwReceivedEvents;

#ifndef _WIN32
    if (bCheckIRQ == PH_ON)
    {
        /* Wait for the IRQ to be Low to De-assert the NSS. */
        do
        {
            if (phDriver_IRQPinRead(PHDRIVER_PIN_IRQ) == 0)
            {
                break;
            }
            else
            {
                /*Below is for making application Fail Safe.*/
                /*In normal working condition, when NxpNfcRdLib is about to send cmd, */
                /*IRQ should not be high. if this happens, the Application should call */
                /*phhalHw_Pn5190_AsyncAbort and then should reset the Pn5190. */
                status = phOsal_EventPend(
                    (volatile phOsal_Event_t *)(&pDataParams->HwEventObj.EventHandle),
                    E_OS_EVENT_OPT_PEND_SET_ANY | E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT,
                    PHHAL_HW_PN5190_IRQ_CHECK_WAIT_TIME,
                    (E_PH_OSAL_EVT_ABORT),
                    &dwReceivedEvents);

                if (status == PH_ERR_SUCCESS)
                {
                    if ((E_PH_OSAL_EVT_RF & dwReceivedEvents) || (E_PH_OSAL_EVT_SIG & dwReceivedEvents))
                    {
                        return PH_ERR_INTERNAL_ERROR;
                    }
                    else
                    {
                        return PH_ERR_ABORTED;
                    }
                }
            }
        }while((status & PH_ERR_MASK) ==  PH_ERR_IO_TIMEOUT);
    }
#endif


    /* Enable chip select connected to reader IC (NSS low). */
    phhalHw_Pn5190_Int_WriteSSEL(
        pDataParams->pBalDataParams,
        PH_DRIVER_SET_LOW);

    /* Send current queue */
    status = phbalReg_Exchange(
        pDataParams->pBalDataParams,
        PH_EXCHANGE_DEFAULT,
        pTxBuffer,
        wTxLength,
        0,
        0,
        &wResponseLen);

    /* Disable chip select connected to reader IC (NSS high). */
    phhalHw_Pn5190_Int_WriteSSEL(
        pDataParams->pBalDataParams,
        PH_DRIVER_SET_HIGH);

    return status;
}


/**
 * phhalHw_Pn5190_InstMngr_Read API does the actual reading of data from Front End using BAL
 */
phStatus_t phhalHw_Pn5190_Read(phhalHw_Pn5190_DataParams_t *pDataParams,uint8_t * pRxBuffer,
        uint16_t wRxLength)
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint16_t    PH_MEMLOC_REM wResponseLen = 0U;

    /* Enable chip select connected to reader IC (NSS low). */
    phhalHw_Pn5190_Int_WriteSSEL(
        pDataParams->pBalDataParams,
        PH_DRIVER_SET_LOW);

    /* Read back response */
    status =  phbalReg_Exchange(
        pDataParams->pBalDataParams,
        PH_EXCHANGE_DEFAULT,
        NULL,
        0U,
        wRxLength,
        pRxBuffer,
        &wResponseLen);

    /* Disable chip select connected to reader IC (NSS high). */
    phhalHw_Pn5190_Int_WriteSSEL(
        pDataParams->pBalDataParams,
        PH_DRIVER_SET_HIGH);

    if(wResponseLen != wRxLength)
    {
        status = PH_ERR_INTERNAL_ERROR;
    }

    return status;
}

static uint8_t  phhalHw_Pn5190_InstMngr_CheckBuffSpace(phhalHw_Pn5190_DataParams_t *pDataParams,
        uint16_t wCurrSpaceNeeded)
{
    uint16_t    PH_MEMLOC_REM    wPrevBuffrdDataLen = 0U;
    uint16_t    PH_MEMLOC_REM    wCurrBufferSize = 0U;
    uint8_t     PH_MEMLOC_REM    * ppTmpBuffer = NULL;
    uint8_t     PH_MEMLOC_REM    bSpaceAvailable = 0U;

    /*Check if sufficient space is there in the Buffer*/
    (void)phhalHw_Pn5190_Int_GetTxBuffer(pDataParams, &ppTmpBuffer, &wPrevBuffrdDataLen, &wCurrBufferSize);

    if (wCurrBufferSize < (wCurrSpaceNeeded + wPrevBuffrdDataLen + pDataParams->wTxBufStartPos))
    {
        bSpaceAvailable = false;
    }
    else
    {
        bSpaceAvailable = true;
    }
    return bSpaceAvailable;
}


#endif /* NXPBUILD__PHHAL_HW_PN5190 */

