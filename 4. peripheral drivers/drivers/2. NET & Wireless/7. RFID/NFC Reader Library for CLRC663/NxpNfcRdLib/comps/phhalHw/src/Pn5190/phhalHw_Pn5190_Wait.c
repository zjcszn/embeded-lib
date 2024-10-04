/*----------------------------------------------------------------------------*/
/* Copyright 2019-2020, 2023 NXP                                              */
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
* Wait Routines for Pn5190 specific HAL-Component of Reader Library Framework.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <ph_Status.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PHHAL_HW_PN5190

#ifndef _WIN32
#include <phOsal.h>
#include <phDriver.h>
#include <phNxpNfcRdLib_Config.h>
#endif
#include <phhalHw.h>
#include "phhalHw_Pn5190_Instr.h"
#include "phhalHw_Pn5190_Int.h"
#include "phhalHw_Pn5190_InstrMngr.h"
#include "phhalHw_Pn5190_Wait.h"


#define PHHAL_HW_PN5190_EXCHANGE_CMD_TIMEOUT      10000U

#ifndef _WIN32
#define PHHAL_HW_PN5190_ABORT_EVT_TIMEOUT         500U

static phStatus_t phhalHw_Pn5190_EventWait_AbortHandling(phhalHw_Pn5190_DataParams_t *pDataParams,uint32_t *dwEventsReceived,uint32_t dwEvtsTimeOut);
#endif

phStatus_t phhalHw_Pn5190_WaitIrq(
    phhalHw_Pn5190_DataParams_t * pDataParams,
    uint32_t * pReceivedEvents
    )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_INTERNAL_ERROR;
    uint16_t    PH_MEMLOC_REM wBuffrdDataLength = 0U;
    uint16_t    PH_MEMLOC_REM wBufferSize = 0U;
    uint8_t     PH_MEMLOC_REM *pTxBuffer = NULL;
    uint8_t     PH_MEMLOC_REM bTagType = 0;
    phOsal_Ticks_t PH_MEMLOC_REM dwTimeout;

    if (pDataParams->bNonRF_Cmd == 0x00U)
    {
        (void)phhalHw_Pn5190_Int_GetTxBuffer(pDataParams, &pTxBuffer, &wBuffrdDataLength, &wBufferSize);
        bTagType = *(&pTxBuffer[0]);
    }

    if (bTagType == PHHAL_HW_PN5190_INSTR_EXCHANGE_RF_DATA)
    {
        dwTimeout = PHHAL_HW_PN5190_EXCHANGE_CMD_TIMEOUT;
    }
    else
    {
        dwTimeout = PHOSAL_MAX_DELAY;
    }

    /* Wait for event */
    PH_CHECK_SUCCESS_FCT(status, phOsal_EventPend(
        (volatile phOsal_Event_t *)(&pDataParams->HwEventObj.EventHandle),
        E_OS_EVENT_OPT_PEND_SET_ANY | E_OS_EVENT_OPT_NONE,
        dwTimeout,
        (E_PH_OSAL_EVT_RF | E_PH_OSAL_EVT_ABORT),
        pReceivedEvents));
#ifndef _WIN32
    if ((E_PH_OSAL_EVT_ABORT & *pReceivedEvents) != 0U)
    {
        phOsal_EventClear(&pDataParams->HwEventObj.EventHandle, E_OS_EVENT_OPT_NONE, E_PH_OSAL_EVT_ABORT, NULL);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_ABORTED, PH_COMP_HAL);
    }
#endif
    return PH_ERR_SUCCESS;
}

phStatus_t phhalHw_Pn5190_WaitForEvent(
        phhalHw_Pn5190_DataParams_t * pDataParams,
        uint32_t dwEvtsWaitFor,
        uint32_t dwEvtsTimeOut,
        uint8_t bPayloadPresnt,
        uint32_t *dwEventsReceived,
        uint8_t ** ppEvtPayLoad
        )
{
    phStatus_t PH_MEMLOC_REM status = PH_EER_PN5190_UNEXPECTED_EVT;
    phStatus_t PH_MEMLOC_REM statustmp;
    phOsal_EventBits_t  PH_MEMLOC_REM tReceivedEvents = 0x0U;
    uint16_t  PH_MEMLOC_REM wEventLength = 0x0U;
    uint16_t  PH_MEMLOC_REM wRemEventLength = 0U;
    int16_t   PH_MEMLOC_REM wEventLenToRead = 0U;
    uint32_t  PH_MEMLOC_REM dwEventStatusReg = 0x0U;
    uint8_t   PH_MEMLOC_REM bBackup = 0x0U;
    uint8_t   PH_MEMLOC_REM bHeaderLen = 0U;
    uint8_t   PH_MEMLOC_REM bCMDAborted = 0;

    /* Wait for event */
    PH_CHECK_SUCCESS_FCT(statustmp, phOsal_EventPend(
        (volatile phOsal_Event_t *)(&pDataParams->HwEventObj.EventHandle),
        E_OS_EVENT_OPT_PEND_SET_ANY | E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT,
        dwEvtsTimeOut,
        (E_PH_OSAL_EVT_SIG | E_PH_OSAL_EVT_ABORT),
        &tReceivedEvents));

    if ((E_PH_OSAL_EVT_SIG & tReceivedEvents) != 0U)
    {
        /* Reading length form the ISR buffer, in which we have read first part of response */
        wEventLength = pDataParams->sIrqResp.pIsrEvtBuffPtr[2];
        wEventLength <<= 8;
        wEventLength |= pDataParams->sIrqResp.pIsrEvtBuffPtr[3];

        /* Calculate the no of bytes to be read */
        bHeaderLen = TYPE_FIELD_LEN + LENGTH_FIELD_LEN + DIRECTION_BYTE_LEN;
        wEventLenToRead = wEventLength - (pDataParams->sIrqResp.bIsrBytesRead - bHeaderLen);

        wRemEventLength = (wEventLenToRead > 0) ? wEventLenToRead : 0x00U;

        if (wRemEventLength != 0)
        {
            bBackup = pDataParams->sIrqResp.pIsrEvtBuffPtr[pDataParams->sIrqResp.bIsrBytesRead - 1];
            /* By using wEventLength, We are reading One byte extra, for spi header. Actually
             * we should have deducted one byte from it.
             */
            (void)phhalHw_Pn5190_Read(
                pDataParams,
                &pDataParams->sIrqResp.pIsrEvtBuffPtr[pDataParams->sIrqResp.bIsrBytesRead - 1],
                (wRemEventLength + DIRECTION_BYTE_LEN));

            pDataParams->sIrqResp.pIsrEvtBuffPtr[pDataParams->sIrqResp.bIsrBytesRead - 1] = bBackup;
        }

        dwEventStatusReg = (uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[4];
        dwEventStatusReg |= ((uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[5]) << 8U ;
        dwEventStatusReg |= ((uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[6]) << 16U;
        dwEventStatusReg |= ((uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[7]) << 24U;
    }

    if ((E_PH_OSAL_EVT_ABORT & tReceivedEvents) != 0U)
    {
#ifndef _WIN32
        /* SWITCH_MODE_NORMAL command will not be sent, in case IRQ is already high due to Event. */
        PH_CHECK_SUCCESS_FCT(statustmp, phhalHw_Pn5190_Int_UserAbort(pDataParams, &bCMDAborted));

        statustmp = phhalHw_Pn5190_EventWait_AbortHandling(pDataParams, dwEventsReceived, PHHAL_HW_PN5190_ABORT_EVT_TIMEOUT);

        if(statustmp != PH_ERR_SUCCESS)
        {
            /* Abort evt is not received. */
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR,PH_COMP_HAL);
        }
#endif
        return PH_ADD_COMPCODE_FIXED(PH_ERR_ABORTED,PH_COMP_HAL);
    }

    if(dwEventsReceived != NULL)
    {
        *dwEventsReceived = dwEventStatusReg;
    }
    if (dwEventStatusReg != PH_PN5190_EVT_CTS)
    {
        if (bPayloadPresnt != false)
        {
            *ppEvtPayLoad = &pDataParams->sIrqResp.pIsrEvtBuffPtr[ 4 + sizeof(dwEventStatusReg)];
        }
        else
        {
            *ppEvtPayLoad = NULL;
        }
    }

    if ((dwEventStatusReg & dwEvtsWaitFor) != 0U)
    {
        status = PH_ERR_SUCCESS;
    }

    return status;
}

#ifndef _WIN32
phStatus_t  phhalHw_Pn5190_Wait_IrqAbortHandling(phhalHw_Pn5190_DataParams_t *pDataParams, phhalHw_InstMngr_CmdParams_t *pCmdParams, uint8_t bAbortCMDExe)
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_INTERNAL_ERROR;
    uint32_t   PH_MEMLOC_REM dwEventReceived = 0x0U;
    phOsal_EventBits_t  PH_MEMLOC_REM tReceivedEvents = 0x0U;
    uint32_t   PH_MEMLOC_REM dwEventToWait = 0x0U;
#ifndef _WIN32
    uint8_t    PH_MEMLOC_REM aCmd[4] = {0};
#endif

    /* Wait for event, not clearing EVT here */
    PH_CHECK_SUCCESS_FCT(status, phOsal_EventPend(
        (volatile phOsal_Event_t *)(&pDataParams->HwEventObj.EventHandle),
        E_OS_EVENT_OPT_PEND_SET_ANY,
        PHOSAL_MAX_DELAY,
        (E_PH_OSAL_EVT_SIG | E_PH_OSAL_EVT_RF),
        &tReceivedEvents));

    if ((tReceivedEvents & E_PH_OSAL_EVT_RF) != 0U)
    {
        phOsal_EventClear(&pDataParams->HwEventObj.EventHandle, E_OS_EVENT_OPT_NONE, E_PH_OSAL_EVT_RF, NULL);
        /*Clear the FW res by reading*/
        phhalHw_Pn5190_InstMngr_ReadRsp(pDataParams, pCmdParams);
    }

    if (bAbortCMDExe == 0)
    {
        if ((tReceivedEvents & E_PH_OSAL_EVT_SIG) != 0U)
        {
            /* If SWITCH_MODE_NORMAL is not sent then PN5190 should not send Event Frame. */
            return PH_ERR_INTERNAL_ERROR;
        }
        else
        {
#ifndef _WIN32
            aCmd[0]  = PHHAL_HW_PN5190_INT_SPI_WRITE;
            aCmd[1]  = 0x20;
            if (pDataParams->bAbortCMD == PH_PN5190_SWITCH_MODE_NORMAL_2_1)
            {
                aCmd[2]  = 0x00;
            }
            else
            {
                aCmd[2]  = 0x02;
            }
            aCmd[3]  = 0x00;

            status = phhalHw_Pn5190_Send(pDataParams,&aCmd[0], 4, PH_ON);
#endif
        }
    }

    dwEventToWait = PH_PN5190_EVT_GENERAL_ERROR;

    if(pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_SWITCH_MODE_AUTOCOLL)
    {
        dwEventToWait |= PH_PN5190_EVT_AUTOCOLL;
    }
    else if(pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_SWITCH_MODE_LPCD)
    {
        dwEventToWait |= PH_PN5190_EVT_LPCD ;
    }
    else if(pCmdParams->bCmd == PHHAL_HW_PN5190_INSTR_SWITCH_MODE_STANDBY)
    {
        dwEventToWait |= PH_PN5190_EVT_STANDBY_PREV;
    }
    else
    {
        dwEventToWait |= PH_PN5190_EVT_IDLE;
    }

    status = phhalHw_Pn5190_Int_EventWait(
                    pDataParams,
                    dwEventToWait,
                    PHHAL_HW_PN5190_ABORT_EVT_TIMEOUT,
                    true,
                    &dwEventReceived);

    if (status == PH_ERR_SUCCESS)
    {
        if(dwEventReceived == PH_PN5190_EVT_AUTOCOLL)
        {
            if((pDataParams->dwEventStatus & 0x000000FF) != PH_ERR_PN5190_USER_CALNCELLED)
            {
                status = PH_ERR_INTERNAL_ERROR;
            }
        }
        else if(dwEventReceived == PH_PN5190_EVT_LPCD)
        {
            if(pDataParams->dwEventStatus != PHHAL_HW_LPCD_EVT_STATUS_HIF_ACTIVITY)
            {
                status = PH_ERR_INTERNAL_ERROR;
            }
        }
        else if(dwEventReceived == PH_PN5190_EVT_STANDBY_PREV)
        {
            if(pDataParams->dwEventStatus != PHHAL_HW_STANDBY_PREV_EVT_STATUS_SPI)
            {
                status = PH_ERR_INTERNAL_ERROR;
            }
        }
        else if(dwEventReceived == PH_PN5190_EVT_GENERAL_ERROR)
        {
            status = PH_ERR_INTERNAL_ERROR;
        }
        else
        {
            /* IDLE event should be received. */
        }
    }
    else
    {
        if ((status != PH_EER_PN5190_UNEXPECTED_EVT))
        {
            status = PH_ERR_INTERNAL_ERROR;
        }
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

static phStatus_t phhalHw_Pn5190_EventWait_AbortHandling(phhalHw_Pn5190_DataParams_t *pDataParams,uint32_t *dwEventsReceived,uint32_t dwEvtsTimeOut)
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_INTERNAL_ERROR;
    phOsal_EventBits_t  PH_MEMLOC_REM tReceivedEvents = 0x0U;
    uint16_t  PH_MEMLOC_REM wEventLength = 0x0U;
    uint16_t  PH_MEMLOC_REM wRemEventLength = 0U;
    int16_t   PH_MEMLOC_REM wEventLenToRead = 0U;
    uint32_t  PH_MEMLOC_REM dwEventReg = 0x0U;
    uint32_t  PH_MEMLOC_REM dwEvtStatusReg = 0x0U;
    uint8_t   PH_MEMLOC_REM bBackup = 0x0U;
    uint8_t   PH_MEMLOC_REM bHeaderLen = 0U;

    PH_CHECK_SUCCESS_FCT(status, phOsal_EventPend(
        (volatile phOsal_Event_t *)(&pDataParams->HwEventObj.EventHandle),
        E_OS_EVENT_OPT_PEND_SET_ANY | E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT,
        dwEvtsTimeOut,
        (E_PH_OSAL_EVT_SIG | E_PH_OSAL_EVT_RF),
        &tReceivedEvents));

    if ((tReceivedEvents & E_PH_OSAL_EVT_RF) != 0U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_HAL);
    }

    if ((tReceivedEvents & E_PH_OSAL_EVT_SIG) != 0U)
    {
        /* Reading length form the ISR buffer, in which we have read first part of response */
        wEventLength = pDataParams->sIrqResp.pIsrEvtBuffPtr[2];
        wEventLength <<= 8;
        wEventLength |= pDataParams->sIrqResp.pIsrEvtBuffPtr[3];

        /* Calculate the no of bytes to be read */
        bHeaderLen = TYPE_FIELD_LEN + LENGTH_FIELD_LEN + DIRECTION_BYTE_LEN;
        wEventLenToRead = wEventLength - (pDataParams->sIrqResp.bIsrBytesRead - bHeaderLen);

        wRemEventLength = (wEventLenToRead > 0) ? wEventLenToRead : 0x00U;

        if (wRemEventLength != 0)
        {
            bBackup = pDataParams->sIrqResp.pIsrEvtBuffPtr[pDataParams->sIrqResp.bIsrBytesRead - 1];
            /* By using wEventLength, We are reading One byte extra, for spi header. Actually
             * we should have deducted one byte from it.
             */
            (void)phhalHw_Pn5190_Read(
                pDataParams,
                &pDataParams->sIrqResp.pIsrEvtBuffPtr[pDataParams->sIrqResp.bIsrBytesRead - 1],
                (wRemEventLength + DIRECTION_BYTE_LEN));

            pDataParams->sIrqResp.pIsrEvtBuffPtr[pDataParams->sIrqResp.bIsrBytesRead - 1] = bBackup;
        }

        dwEventReg = (uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[4];
        dwEventReg |= ((uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[5]) << 8U ;
        dwEventReg |= ((uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[6]) << 16U;
        dwEventReg |= ((uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[7]) << 24U;
    }

    if ((dwEventReg & (PH_PN5190_EVT_IDLE | PH_PN5190_EVT_BOOT | PH_PN5190_EVT_GENERAL_ERROR | PH_PN5190_EVT_LPCD | PH_PN5190_EVT_AUTOCOLL)) != 0U)
    {
        /*Expected Event received successfully*/
        status = PH_ERR_SUCCESS;

        if ((dwEventReg & (PH_PN5190_EVT_LPCD | PH_PN5190_EVT_AUTOCOLL | PH_PN5190_EVT_BOOT)) != 0U)
        {
            /*check the status bit for LPCD and AUTOCOLL*/
            dwEvtStatusReg = (uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[8];
            dwEvtStatusReg |= ((uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[9]) << 8U ;
            dwEvtStatusReg |= ((uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[10]) << 16U;
            dwEvtStatusReg |= ((uint32_t) pDataParams->sIrqResp.pIsrEvtBuffPtr[11]) << 24U;

            if( ((dwEventReg == PH_PN5190_EVT_AUTOCOLL) && ((dwEvtStatusReg & 0x000000FF) != PH_ERR_PN5190_USER_CALNCELLED))
                    || ((dwEventReg == PH_PN5190_EVT_LPCD) && (dwEvtStatusReg != PHHAL_HW_LPCD_EVT_STATUS_HIF_ACTIVITY))
                    || ((dwEventReg == PH_PN5190_EVT_STANDBY_PREV) &&
                            ( (dwEvtStatusReg != PHHAL_HW_STANDBY_PREV_EVT_STATUS_PREV_TEMP) &&
                            (dwEvtStatusReg != PHHAL_HW_STANDBY_PREV_EVT_STATUS_SPI)) ) )
            {
                status = PH_ERR_INTERNAL_ERROR;
            }
        }
    }
    else
    {
        status = PH_EER_PN5190_UNEXPECTED_EVT;
    }

    if(dwEventReg & PH_PN5190_EVT_GENERAL_ERROR)
    {
        status = PH_ERR_INTERNAL_ERROR;
    }

    *dwEventsReceived = dwEventReg;
    pDataParams->dwEventStatus = dwEvtStatusReg;

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}
#endif  /*#ifndef _WIN32*/

#endif  /* NXPBUILD__PHHAL_HW_PN5190 */
