/*----------------------------------------------------------------------------*/
/* Copyright 2019-2022 NXP                                                    */
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
* PN5190 generic HAL interface.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <ph_Status.h>
#include <phhalHw.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PH_KEYSTORE
#include <phKeyStore.h>
#endif

#ifdef NXPBUILD__PHHAL_HW_PN5190

#include <phDriver.h>
#include <phTools.h>
#include "phhalHw_Pn5190.h"
#include "phhalHw_Pn5190_Instr.h"
#include "phhalHw_Pn5190_InstrMngr.h"
#include "phhalHw_Pn5190_Reg.h"
#include "phhalHw_Pn5190_Int.h"
#include "phhalHw_Pn5190_Wait.h"
#include "BoardSelection.h"

#define PH_RF_EXCH_CONF_ENABLE_RSP_RX_DATA   0x08
#define PH_RF_EXCH_CONF_ENABLE_EVT_STATUS    0x04
#define PH_RF_EXCH_CONF_ENABLE_RF_STATUS     0x02
#define PH_RF_EXCH_CONF_ENABLE_RX_STATUS     0x01

#define PH_RECEIVE_RF_ENABLE_EVT_STATUS   0x04
#define PH_RECEIVE_RF_ENABLE_RF_STATUS    0x02
#define PH_RECEIVE_RF_ENABLE_RX_STATUS    0x01

#define SET_PN5190_SHADOW(USED_SHADOW)   \
    pShadowDefault = &(USED_SHADOW)[0][0]; \
    wShadowCount = (uint16_t)(sizeof((USED_SHADOW)) / (sizeof((USED_SHADOW)[0])))

#define EVTS_HEADER_BYTES       3
#define EVENT_LSB_INDEX         1
#define TLV_TYPE_FIELD_INDEX    1
#define TLV_LENGTH_MSB_INDEX    2
#define TLV_LENGTH_LSB_INDEX    3

/* Default shadow for ISO14443-3A Mode */
static const uint16_t PH_MEMLOC_CONST_ROM wPn5190_DefaultShadow_I14443a[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_ON},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXWAIT_US,             76U},
    {PHHAL_HW_CONFIG_TXWAIT_US,             85U},
    {PHHAL_HW_CONFIG_TXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_106 | PHHAL_HW_RF_TYPE_A_FRAMING},
    {PHHAL_HW_CONFIG_RXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_106 | PHHAL_HW_RF_TYPE_A_FRAMING},
    {PHHAL_HW_CONFIG_ASK100,                PH_ON}
};

/* Default shadow for ISO14443-3B Mode */
static const uint16_t PH_MEMLOC_CONST_ROM wPn5190_DefaultShadow_I14443b[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXWAIT_US,             76U},
    {PHHAL_HW_CONFIG_TXWAIT_US,             85U},
    {PHHAL_HW_CONFIG_TXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_106 | PHHAL_HW_RF_TYPE_B_FRAMING},
    {PHHAL_HW_CONFIG_RXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_106 | PHHAL_HW_RF_TYPE_B_FRAMING},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

/* Default shadow for FeliCa Mode */
static const uint16_t PH_MEMLOC_CONST_ROM wPn5190_DefaultShadow_Felica[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXWAIT_US,             88U},
    {PHHAL_HW_CONFIG_TXWAIT_US,             160U},
    {PHHAL_HW_CONFIG_TXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_212 | PHHAL_HW_RF_TYPE_F_FRAMING},
    {PHHAL_HW_CONFIG_RXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_212 | PHHAL_HW_RF_TYPE_F_FRAMING},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

/* Default shadow for FeliCa Mode at 424 baud rate */
static const uint16_t PH_MEMLOC_CONST_ROM wPn5190_DefaultShadow_Felica_424[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXWAIT_US,             85U},
    {PHHAL_HW_CONFIG_TXWAIT_US,             160U},
    {PHHAL_HW_CONFIG_TXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_424 | PHHAL_HW_RF_TYPE_F_FRAMING},
    {PHHAL_HW_CONFIG_RXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_424 | PHHAL_HW_RF_TYPE_F_FRAMING},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

/* Default shadow for ISO15693 Mode */
static const uint16_t PH_MEMLOC_CONST_ROM wPn5190_DefaultShadow_I15693[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXWAIT_US,             300U},
    {PHHAL_HW_CONFIG_TXWAIT_US,             500U},
    {PHHAL_HW_CONFIG_TXDATARATE_FRAMING,    PHHAL_HW_RF_TX_DATARATE_1_OUT_OF_4},
    {PHHAL_HW_CONFIG_RXDATARATE_FRAMING,    PHHAL_HW_RF_RX_DATARATE_HIGH},
    {PHHAL_HW_CONFIG_SUBCARRIER,            PHHAL_HW_SUBCARRIER_SINGLE},
    {PHHAL_HW_CONFIG_ASK100,                PH_ON}
};


/* Default shadow for ISO18000-3 Mode3 */
static const uint16_t PH_MEMLOC_CONST_ROM wPn5190_DefaultShadow_I18000p3m3[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXWAIT_US,             76U},
    {PHHAL_HW_CONFIG_TXWAIT_US,             300U},
    {PHHAL_HW_CONFIG_TXDATARATE_FRAMING,    PHHAL_HW_RF_TX_DATARATE_I18000P3M3},
    {PHHAL_HW_CONFIG_RXDATARATE_FRAMING,    PHHAL_HW_RX_I18000P3M3_FL_847_MAN4},
    {PHHAL_HW_CONFIG_SUBCARRIER,            PHHAL_HW_SUBCARRIER_DUAL},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};


/* Default shadow for ISO18092 Active Initiator Mode at 106kbps. */
static const uint16_t PH_MEMLOC_CONST_ROM wPn5190_DefShadow_I18092m_Active[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_ON},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_TXWAIT_US,             0U},
    {PHHAL_HW_CONFIG_RXWAIT_US,             29U},
    {PHHAL_HW_CONFIG_TXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_106 | PHHAL_HW_RF_TYPE_ACTIVE_FRAMING},
    {PHHAL_HW_CONFIG_RXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_106 | PHHAL_HW_RF_TYPE_ACTIVE_FRAMING},
    {PHHAL_HW_CONFIG_ASK100,                PH_ON}
};

/* Default shadow for ISO18092 Active Initiator Mode at 212kbps. */
static const uint16_t PH_MEMLOC_CONST_ROM wPn5190_DefShadow_I18092m_212_Active[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_TXWAIT_US,             0U},
    {PHHAL_HW_CONFIG_RXWAIT_US,             15U},
    {PHHAL_HW_CONFIG_TXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_212 | PHHAL_HW_RF_TYPE_ACTIVE_FRAMING},
    {PHHAL_HW_CONFIG_RXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_212 | PHHAL_HW_RF_TYPE_ACTIVE_FRAMING},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

/* Default shadow for ISO18092 Active Initiator Mode at 424kbps. */
static const uint16_t PH_MEMLOC_CONST_ROM wPn5190_DefShadow_I18092m_424_Active[][2] =
{
    {PHHAL_HW_CONFIG_PARITY,                PH_OFF},
    {PHHAL_HW_CONFIG_TXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_RXCRC,                 PH_ON},
    {PHHAL_HW_CONFIG_TXWAIT_US,             0U},
    {PHHAL_HW_CONFIG_RXWAIT_US,             8U},
    {PHHAL_HW_CONFIG_TXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_424 | PHHAL_HW_RF_TYPE_ACTIVE_FRAMING},
    {PHHAL_HW_CONFIG_RXDATARATE_FRAMING,    PHHAL_HW_RF_DATARATE_424 | PHHAL_HW_RF_TYPE_ACTIVE_FRAMING},
    {PHHAL_HW_CONFIG_ASK100,                PH_OFF}
};

static const uint8_t bHalEventName[] = "Hal";
phOsal_Event_t xEventHandle;

#ifndef _WIN32
static phStatus_t phhalHw_Pn5190_EnBootNormalMode(phhalHw_Pn5190_DataParams_t * pDataParams);
#endif /* _WIN32 */

phStatus_t phhalHw_Pn5190_Init(
                               phhalHw_Pn5190_DataParams_t * pDataParams,
                               uint16_t wSizeOfDataParams,
                               void * pBalDataParams,
                               void * pKeyStoreDataParams,
                               uint8_t * pTxBuffer,
                               uint16_t wTxBufSize,
                               uint8_t * pRxBuffer,
                               uint16_t wRxBufSize
                               )
{
    phStatus_t PH_MEMLOC_REM statusTmp;
#ifndef _WIN32
    phDriver_Pin_Config_t PH_MEMLOC_REM pinCfg;
#endif

    if (sizeof(phhalHw_Pn5190_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_HAL);
    }

    /* The buffers cannot be zero */
    if (wTxBufSize == 0U || wRxBufSize == 0U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Check all the pointers */
    PH_ASSERT_NULL(pDataParams);
    PH_ASSERT_NULL(pBalDataParams);
    PH_ASSERT_NULL(pTxBuffer);
    PH_ASSERT_NULL(pRxBuffer);

    /* The first bytes are reserved for the SPI commands */
    pDataParams->pTxBuffer                = &pTxBuffer[1];
    pDataParams->wTxBufSize               = wTxBufSize - 1U;
    pDataParams->pRxBuffer                = &pRxBuffer[2];
    pDataParams->wRxBufSize               = wRxBufSize - 2U;

    /* Initialize the private data */
    pDataParams->wId                      = PH_COMP_HAL | PHHAL_HW_PN5190_ID;
    pDataParams->pBalDataParams           = pBalDataParams;
    pDataParams->pKeyStoreDataParams      = pKeyStoreDataParams;

    pDataParams->wRxBufLen              = 0U;
    pDataParams->wRxBufStartPos         = 0U;

    pDataParams->wTxBufLen              = 0U;
    pDataParams->wTxBufStartPos         = 0U;
    pDataParams->wTxFwDataLen           = 0U;

    pDataParams->bActiveMode            = PH_OFF;
    pDataParams->bCardType              = PHHAL_HW_CARDTYPE_ISO14443A;
    pDataParams->bTimeoutUnit           = PHHAL_HW_TIME_MICROSECONDS;
    pDataParams->wTimingMode            = PHHAL_HW_TIMING_MODE_OFF;
    pDataParams->dwTimingUs             = 0U;
    pDataParams->wFieldOffTime          = PHHAL_HW_FIELD_OFF_DEFAULT;
    pDataParams->wFieldRecoveryTime     = PHHAL_HW_FIELD_RECOVERY_DEFAULT;
    pDataParams->bSymbolStart           = PH_OFF;
    pDataParams->bSymbolEnd             = PH_OFF;

    pDataParams->bRfResetAfterTo        = PH_OFF;
    pDataParams->bOpeMode               = RD_LIB_MODE_NFC;
    pDataParams->dwFelicaEmdReg         = 0U;
    pDataParams->bRxMultiple            = PH_OFF;
    pDataParams->bJewelActivated        = PH_OFF;
    pDataParams->bLpcdConfig            = 0U;
    pDataParams->wWakeupCounterInMs     = 1000U;
    pDataParams->wLPCDWakeupCounterInMs = 1000U;
    pDataParams->bLpcdWakeUpCtrl        = 0U;
    pDataParams->bRfca                  = PH_ON;
    pDataParams->wTargetMode            = PH_OFF;
    pDataParams->dwTxWaitMs             = 0U;
    pDataParams->bMfcCryptoEnabled      = PH_OFF;
    pDataParams->bCardMode              = PH_OFF;
    pDataParams->bCmdAborted            = PH_OFF;
    pDataParams->bBalConnectionType     = PHHAL_HW_BAL_CONNECTION_SPI;
    pDataParams->wAdditionalInfo        = 0U;
    pDataParams->dwExpectedEvent        = 0U;
#ifndef _WIN32
    pDataParams->pTimerISRCallBack      = &phhalHw_Pn5190_Int_GuardTimeCallBck;
#endif

    pDataParams->bPollGuardTimeFlag     = PH_OFF;
    pDataParams->sIrqResp.bIsrBytesRead = 9;
    pDataParams->bCTSEvent              = PH_OFF;
    pDataParams->bRFONEvent             = PH_OFF;
    pDataParams->dwEventStatus          = 0U;
    pDataParams->dwEventReceived        = 0U;
    pDataParams->dwLpcdRefVal           = 0U;
    pDataParams->dwGenError             = 0x00U;
    pDataParams->bNfcipMode             = PH_OFF;
    pDataParams->bFeliCaRxOnly          = PH_OFF;
    pDataParams->bAbortCMD              = PH_PN5190_SWITCH_MODE_NORMAL_2_1;
    pDataParams->bTimerDeviation        = 3;
    pDataParams->sIrqResp.pHandlerModeBuffPtr = &pDataParams->sIrqResp.aISRReadBuf[0];

    memset(&pDataParams->sIrqResp.aISRReadBuf[0], 0, MAX_ISR_READ_BUFFER_SIZE);

#ifndef _WIN32
    /* Configure Reset pin as output and set to high. */
    pinCfg.bPullSelect = PHDRIVER_PIN_RESET_PULL_CFG;
    pinCfg.bOutputLogic = RESET_POWERUP_LEVEL;

    PH_CHECK_SUCCESS_FCT(statusTmp, phDriver_PinConfig(
        PHDRIVER_PIN_RESET,
        PH_DRIVER_PINFUNC_OUTPUT,
        &pinCfg));

    if(((phbalReg_Type_t *)pBalDataParams)->bBalType == PHBAL_REG_TYPE_SPI)
    {
        /* Configure NSS (Chip Select) pin as GPIO output and set to high. */
        pinCfg.bPullSelect = PHDRIVER_PIN_NSS_PULL_CFG;
        pinCfg.bOutputLogic = PH_DRIVER_SET_HIGH;

        PH_CHECK_SUCCESS_FCT(statusTmp, phDriver_PinConfig(
            PHDRIVER_PIN_SSEL,
            PH_DRIVER_PINFUNC_OUTPUT,
            &pinCfg));
    }

    pinCfg.bPullSelect = PHDRIVER_PIN_DWL_PULL_CFG;
    pinCfg.bOutputLogic = PH_DRIVER_SET_LOW;
    phDriver_PinConfig(PHDRIVER_PIN_DWL, PH_DRIVER_PINFUNC_OUTPUT, &pinCfg);

    if (((phbalReg_Type_t *)pBalDataParams)->bBalType != PHBAL_REG_TYPE_KERNEL_SPI)
    {
        /* Busy pin (nothing but IRQ for PN5190) as input and pullup enabled.
         * Helps during poll operation, Do Not Remove. */
        pinCfg.bPullSelect = PHDRIVER_PIN_BUSY_PULL_CFG;
        phDriver_PinConfig(PHDRIVER_PIN_BUSY, PH_DRIVER_PINFUNC_INPUT, &pinCfg);
    }

    /* Reset Pn5190 Front-end. */
    (void)phhalHw_Pn5190_Int_Reset();

    statusTmp = phhalHw_Pn5190_EnBootNormalMode(pDataParams);
    /* Switch to Normal Mode and Read Ideal Event*/
    if( PH_ERR_SUCCESS != (statusTmp & PH_ERR_MASK ))
    {
        return statusTmp;
    }

#endif

    /* Create the event. */
    pDataParams->HwEventObj.pEvtName = (uint8_t *)bHalEventName;
    pDataParams->HwEventObj.intialValue = 0U;
    PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_EventCreate(&pDataParams->HwEventObj.EventHandle, &pDataParams->HwEventObj));

    /*clear previously occurred unwanted events*/
    (void)phOsal_EventClear(&pDataParams->HwEventObj.EventHandle, E_OS_EVENT_OPT_NONE, (E_PH_OSAL_EVT_RF | E_PH_OSAL_EVT_SIG), NULL);
    xEventHandle = pDataParams->HwEventObj.EventHandle;

#ifndef _WIN32
    /* This assignment has to be done after PN5190 is confirmed to
     * be normal mode */
    pDataParams->pRFISRCallback         = &phhalHw_Pn5190_ISR_Callback;
#endif

    return PH_ERR_SUCCESS;
}

#ifndef _WIN32
static phStatus_t phhalHw_Pn5190_EnBootNormalMode(phhalHw_Pn5190_DataParams_t * pDataParams)
{
    uint32_t   PH_MEMLOC_REM dwEventStatusReg = 0x0U;
    phStatus_t PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint8_t    PH_MEMLOC_REM bEvent = 0x0U;
    uint16_t   PH_MEMLOC_REM wPktLength = 0x0U;

    PH_CHECK_SUCCESS_FCT(status, phhalHw_Pn5190_Instr_SwitchModeBootNormal(pDataParams));
    /* Wait till IDEL EVENT is been triggered by PN5190. */
    phDriver_IRQPinPoll(PHDRIVER_PIN_IRQ, PH_DRIVER_PINFUNC_INPUT, PIN_IRQ_TRIGGER_TYPE);

    /*Doing split read because we don't know whether ERROR also occurred, accordingly
     * the length of response will change  */
    PH_CHECK_SUCCESS_FCT(status, phhalHw_Pn5190_Read(
        pDataParams,
        &pDataParams->sIrqResp.aISRReadBuf[0],
        EVTS_HEADER_BYTES + 1
        ));

    bEvent = pDataParams->sIrqResp.aISRReadBuf[TLV_TYPE_FIELD_INDEX];

    wPktLength = pDataParams->sIrqResp.aISRReadBuf[TLV_LENGTH_MSB_INDEX];
    wPktLength <<=  8;
    wPktLength |= pDataParams->sIrqResp.aISRReadBuf[TLV_LENGTH_LSB_INDEX];

    if (!( (wPktLength == 0x08) || (wPktLength == 0x0C) ))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERFACE_ERROR, PH_COMP_HAL);
    }

    PH_CHECK_SUCCESS_FCT(status, phhalHw_Pn5190_Read(
        pDataParams,
        &pDataParams->sIrqResp.aISRReadBuf[0],
        wPktLength + 1
        ));

    dwEventStatusReg = (uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX];
    dwEventStatusReg |= ((uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 1]) << 8U ;
    dwEventStatusReg |= ((uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 2]) << 16U;
    dwEventStatusReg |= ((uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 3]) << 24U;

    pDataParams->dwEventStatus = (uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 4];
    pDataParams->dwEventStatus |= ((uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 5]) << 8U ;
    pDataParams->dwEventStatus |= ((uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 6]) << 16U;
    pDataParams->dwEventStatus |= ((uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 7]) << 24U;


    /* Check if PN5190 Booted correctly*/
    if (!((bEvent == 0x80) && (dwEventStatusReg == PH_PN5190_EVT_BOOT)))
    {
        if(dwEventStatusReg & PH_PN5190_EVT_GENERAL_ERROR)
        {
            pDataParams->dwGenError = (uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 8];
            pDataParams->dwGenError |= ((uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 9]) << 8U ;
            pDataParams->dwGenError |= ((uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 10]) << 16U;
            pDataParams->dwGenError |= ((uint32_t) pDataParams->sIrqResp.aISRReadBuf[EVENT_LSB_INDEX + 11]) << 24U;
        }
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERFACE_ERROR, PH_COMP_HAL);
    }

    return status;
}
#endif /* _WIN32 */

phStatus_t phhalHw_Pn5190_DeInit(
                                 phhalHw_Pn5190_DataParams_t * pDataParams
                                 )
{
    /* De-Initialize data parameters */
    pDataParams->wId                    = 0U;
    pDataParams->pBalDataParams         = NULL;
    pDataParams->pKeyStoreDataParams    = NULL;
    pDataParams->pTxBuffer              = NULL;
    pDataParams->wTxBufSize             = 0U;
    pDataParams->wTxBufLen              = 0U;
    pDataParams->pRxBuffer              = NULL;
    pDataParams->wRxBufSize             = 0U;
    pDataParams->wRxBufLen              = 0U;
    pDataParams->wRxBufStartPos         = 0U;
    pDataParams->wTxBufStartPos         = 0U;
    pDataParams->bCardType              = PHHAL_HW_CARDTYPE_UNKNOWN;
    pDataParams->bTimeoutUnit           = PHHAL_HW_TIME_MICROSECONDS;
    pDataParams->wFieldOffTime          = PHHAL_HW_FIELD_OFF_DEFAULT;
    pDataParams->wFieldRecoveryTime     = PHHAL_HW_FIELD_RECOVERY_DEFAULT;
    pDataParams->bSymbolStart           = PH_OFF;
    pDataParams->bSymbolEnd             = PH_OFF;
    pDataParams->wAdditionalInfo        = 0U;
    pDataParams->bRfResetAfterTo        = PH_OFF;
    pDataParams->bRxMultiple            = PH_OFF;
    pDataParams->bActiveMode            = PH_OFF;
    pDataParams->bRfca                  = PH_ON;
    pDataParams->wTargetMode            = PH_OFF;
    pDataParams->bCmdAborted            = PH_OFF;
    pDataParams->bOpeMode               = RD_LIB_MODE_NFC;
    pDataParams->dwFelicaEmdReg         = 0U;
    pDataParams->bJewelActivated        = PH_OFF;
    pDataParams->bLpcdConfig            = 0U;
    pDataParams->wWakeupCounterInMs     = 0U;
    pDataParams->wLPCDWakeupCounterInMs = 0U;
    pDataParams->bCTSEvent              = PH_OFF;
    pDataParams->bRFONEvent             = PH_OFF;
    pDataParams->dwEventStatus          = 0U;
    pDataParams->dwEventReceived        = 0U;
    pDataParams->dwLpcdRefVal           = 0U;
    pDataParams->dwTxWaitMs             = 0U;
    pDataParams->bMfcCryptoEnabled      = PH_OFF;
    pDataParams->bCardMode              = PH_OFF;
    pDataParams->bPollGuardTimeFlag     = PH_OFF;
    pDataParams->dwExpectedEvent        = 0U;
#ifndef _WIN32
    pDataParams->pRFISRCallback         = NULL;
    pDataParams->pTimerISRCallBack      = NULL;
#endif
    pDataParams->bNfcipMode             = PH_OFF;
    pDataParams->bFeliCaRxOnly          = PH_OFF;
    pDataParams->bAbortCMD              = PH_PN5190_SWITCH_MODE_NORMAL_2_1;
    pDataParams->bTimerDeviation        = 0U;

    pDataParams->sIrqResp.pHandlerModeBuffPtr =  NULL;
#ifndef _WIN32
    /* De-initialize local event handle */
    xEventHandle = NULL;

    /* Delete event */
    (void)phOsal_EventDelete(&(pDataParams->HwEventObj.EventHandle));
#endif
    return PH_ERR_SUCCESS;
}

phStatus_t phhalHw_Pn5190_AsyncAbort(
                                     phhalHw_Pn5190_DataParams_t * pDataParams
                                     )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_SUCCESS;

    /* Post Abort Event. */
#ifndef _WIN32
    PH_CHECK_SUCCESS_FCT(status, phOsal_EventPost(
        &pDataParams->HwEventObj.EventHandle,
        E_OS_EVENT_OPT_NONE,
        E_PH_OSAL_EVT_ABORT,
        NULL));
#else
    /* satisfy compiler */
    if (0U != (pDataParams));
#endif
    return PH_ERR_SUCCESS;
}

phStatus_t phhalHw_Pn5190_FieldOn(
                                  phhalHw_Pn5190_DataParams_t * pDataParams
                                  )
{
    phStatus_t PH_MEMLOC_REM statusTmp;
    uint8_t    PH_MEMLOC_REM bRFONconfig = 0U;

    if(pDataParams->bRfca == PH_OFF)
    {
        bRFONconfig |= 0x01U;
    }

    if(pDataParams->bActiveMode == PH_ON)
    {
        bRFONconfig |= 0x02U;
    }

    statusTmp = phhalHw_Pn5190_Instr_RfOn(
       pDataParams,
       bRFONconfig);

    if ((statusTmp & PH_ERR_MASK) == PH_ERR_PN5190_EXTERNAL_FIELD_ERROR)
    {
        statusTmp = PH_ADD_COMPCODE_FIXED(PH_ERR_RF_ERROR, PH_COMP_HAL);
    }

    return statusTmp;
}

phStatus_t phhalHw_Pn5190_FieldOff(
                                   phhalHw_Pn5190_DataParams_t * pDataParams
                                   )
{
    phStatus_t PH_MEMLOC_REM statusTmp;

    /* Disable the EMD. */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig(pDataParams, PHHAL_HW_CONFIG_SET_EMD, PH_OFF));

    return phhalHw_Pn5190_Instr_RfOff(pDataParams);
}

phStatus_t phhalHw_Pn5190_FieldReset(
                                     phhalHw_Pn5190_DataParams_t * pDataParams
                                     )
{
    phStatus_t PH_MEMLOC_REM statusTmp;

    /* Switch off the field */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_FieldOff(pDataParams));

    /* Wait for field-off time-out */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Wait(
        pDataParams,
        PHHAL_HW_TIME_MILLISECONDS,
        (pDataParams->wFieldOffTime)));

    /* In Active mode FW will enter idle state when the RF OFF CMD is seen during active communication.
     * So, apply protocol settings/LoadRf during Field reset. */
    if(pDataParams->bActiveMode == PH_ON)
    {
        if(pDataParams->bCardType == PHHAL_HW_CARDTYPE_UNKNOWN)
        {
            pDataParams->bCardType = PHHAL_HW_CARDTYPE_I18092M_ACTIVE_106;
        }
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_ApplyProtocolSettings(pDataParams, pDataParams->bCardType));
    }

    /* Switch on the field again */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_FieldOn(pDataParams));

    /* Wait for field-recovery time-out */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Wait(
        pDataParams,
        PHHAL_HW_TIME_MILLISECONDS,
        (pDataParams->wFieldRecoveryTime)));

    return PH_ERR_SUCCESS;
}

phStatus_t phhalHw_Pn5190_ApplyProtocolSettings(
                                                phhalHw_Pn5190_DataParams_t * pDataParams,
                                                uint8_t bCardType
                                                )
{
    phStatus_t PH_MEMLOC_REM statusTmp;
    const uint16_t * PH_MEMLOC_REM pShadowDefault;
    uint16_t   PH_MEMLOC_REM wShadowCount;
    uint8_t    PH_MEMLOC_REM bUseDefaultShadow;
    uint16_t   PH_MEMLOC_COUNT wIndex;
    uint16_t   PH_MEMLOC_REM wConfig;

    /* Disable the EMD. */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig(pDataParams, PHHAL_HW_CONFIG_SET_EMD, PH_OFF));

    /* Store new card type */
    if (bCardType != PHHAL_HW_CARDTYPE_CURRENT)
    {
        pDataParams->bCardType = bCardType;
        pDataParams->bTimeoutUnit = PHHAL_HW_TIME_MICROSECONDS;
        pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_US] = 0;
        pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS] = 0;
        bUseDefaultShadow = 1U;
    }
    else
    {
        bUseDefaultShadow = 0U;
    }

    pDataParams->bActiveMode       = PH_OFF;
    pDataParams->wTargetMode       = PH_OFF;
    pDataParams->bJewelActivated   = PH_OFF;
    pDataParams->bNfcipMode        = PH_OFF;
    pDataParams->bCmdAborted       = PH_OFF;

    /* Configure reader IC for current card */
    switch (pDataParams->bCardType)
    {
        /* configure hardware for ISO 14443A */
    case PHHAL_HW_CARDTYPE_ISO14443A:
        /* Use 14443a default shadow */
        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, PHHAL_HW_PN5190_RF_TX_ISO14443A_106_MILLER, PHHAL_HW_PN5190_RF_RX_ISO14443A_106_MANCH_SUBC));  /* Masking RxDataRate with Minimum Configuration Index */
        SET_PN5190_SHADOW(wPn5190_DefaultShadow_I14443a);
        break;

    case PHHAL_HW_CARDTYPE_ISO14443B:
        /* Use 14443b shadow */
        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, PHHAL_HW_PN5190_RF_TX_ISO14443B_106_NRZ, PHHAL_HW_PN5190_RF_RX_ISO14443B_106_BPSK));  /* Masking RxDataRate with Minimum Configuration Index */
        SET_PN5190_SHADOW(wPn5190_DefaultShadow_I14443b);
        break;

    case PHHAL_HW_CARDTYPE_FELICA_212:
        /* Use Felica shadow */
        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, PHHAL_HW_PN5190_RF_TX_FELICA_212, PHHAL_HW_PN5190_RF_RX_FELICA_212));  /* Masking RxDataRate with Minimum Configuration Index */

        SET_PN5190_SHADOW(wPn5190_DefaultShadow_Felica);
        break;

    case PHHAL_HW_CARDTYPE_FELICA_424:
        /* Use Felica shadow for 424*/
        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, PHHAL_HW_PN5190_RF_TX_FELICA_424, PHHAL_HW_PN5190_RF_RX_FELICA_424));  /* Masking RxDataRate with Minimum Configuration Index */

        SET_PN5190_SHADOW(wPn5190_DefaultShadow_Felica_424);
        break;

    case PHHAL_HW_CARDTYPE_I18092M_ACTIVE_106:
        /* Use I18092m_Active shadow */
        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, PHHAL_HW_PN5190_RF_TX_NFC_AI_106_106, PHHAL_HW_PN5190_RF_RX_NFC_AI_106));  /* Masking RxDataRate with Minimum Configuration Index */

        SET_PN5190_SHADOW(wPn5190_DefShadow_I18092m_Active);
        /* Enable active mode */
        pDataParams->bActiveMode = PH_ON;
        pDataParams->bNfcipMode  = PH_ON;
        break;

    case PHHAL_HW_CARDTYPE_I18092M_ACTIVE_212:
        /* Use I18092m_Active_212 shadow */
        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, PHHAL_HW_PN5190_RF_TX_NFC_AI_212_212, PHHAL_HW_PN5190_RF_RX_NFC_AI_212));  /* Masking RxDataRate with Minimum Configuration Index */

        SET_PN5190_SHADOW(wPn5190_DefShadow_I18092m_212_Active);
        /* Enable active mode */
        pDataParams->bActiveMode = PH_ON;
        break;

    case PHHAL_HW_CARDTYPE_I18092M_ACTIVE_424:
        /* Use I18092m_Active_424 shadow */
        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, PHHAL_HW_PN5190_RF_TX_NFC_AI_424_424, PHHAL_HW_PN5190_RF_RX_NFC_AI_424));  /* Masking RxDataRate with Minimum Configuration Index */

        SET_PN5190_SHADOW(wPn5190_DefShadow_I18092m_424_Active);
        /* Enable active mode */
        pDataParams->bActiveMode = PH_ON;
        break;

    case PHHAL_HW_CARDTYPE_ISO15693:
        /* Use 15693 shadow */
        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, PHHAL_HW_PN5190_RF_TX_ISO15693_26_1OF4_ASK100, PHHAL_HW_PN5190_RF_RX_ISO15693_26_SC));  /* Masking RxDataRate with Minimum Configuration Index */

        SET_PN5190_SHADOW(wPn5190_DefaultShadow_I15693);

        break;

    case PHHAL_HW_CARDTYPE_I18000P3M3:
        /* Use 18000p3m3 shadow */
        PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, PHHAL_HW_PN5190_RF_TX_ISO180003M3_TARI_9_44_ASK, PHHAL_HW_PN5190_RF_RX_ISO180003M3_MANCH848_4_PERIOD));  /* Masking RxDataRate with Minimum Configuration Index */

        SET_PN5190_SHADOW(wPn5190_DefaultShadow_I18000p3m3);
        break;

    default:
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Copy over default shadow contents into current shadow. */
    if (0U != bUseDefaultShadow)
    {
        /* Initialize config shadow */
        (void)memset(pDataParams->wCfgShadow, 0x00U, (sizeof(uint16_t) * PHHAL_HW_PN5190_SHADOW_COUNT));

        for (wIndex = 0U; wIndex < wShadowCount; ++wIndex)
        {
            wConfig = pShadowDefault[wIndex << 1U];

            /* Updated TxCRC and RxCRC in shadow due to LoadRFConfig */
            if ((wConfig == PHHAL_HW_CONFIG_TXCRC) ||
                (wConfig == PHHAL_HW_CONFIG_RXCRC))
            {
                if (pDataParams->bCardType == PHHAL_HW_CARDTYPE_ISO14443B)
                {
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig(pDataParams, wConfig, pShadowDefault[(wIndex << 1U) + 1U]));
                }
                else
                {
                    /* Write config data into shadow */
                    pDataParams->wCfgShadow[wConfig] = pShadowDefault[(wIndex << 1U) + 1U];
                }
            }

            /* Configure the RxWait using Shadow values */
            if (wConfig == PHHAL_HW_CONFIG_RXWAIT_US)
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig(pDataParams, wConfig, pShadowDefault[(wIndex << 1U) + 1U]));
            }

            /* Configure the TxWait using Shadow values */
            if(wConfig == PHHAL_HW_CONFIG_TXWAIT_US)
            {
                if (((pDataParams->bCardType == PHHAL_HW_CARDTYPE_ISO14443A) || (pDataParams->bCardType == PHHAL_HW_CARDTYPE_ISO14443B)) &&
                    (pDataParams->bOpeMode == RD_LIB_MODE_EMVCO))
                {
                    /* Configure TxWait of 500 micro seconds in EMVCo mode. */
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig(pDataParams, wConfig, 500U));
                }
                else
                {
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig(pDataParams, wConfig, pShadowDefault[(wIndex << 1U) + 1U]));
                }
            }
            pDataParams->wCfgShadow[wConfig] = pShadowDefault[(wIndex << 1U) + 1U];
        }
    }

    /* MIFARE Crypto1 state is disabled by default */
    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(pDataParams, PHHAL_HW_CONFIG_DISABLE_MF_CRYPTO1, PH_ON));

    return PH_ERR_SUCCESS;
}

phStatus_t phhalHw_Pn5190_SetConfig(
                                    phhalHw_Pn5190_DataParams_t * pDataParams,
                                    uint16_t wConfig,
                                    uint16_t wValue
                                    )
{
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint16_t    PH_MEMLOC_REM wSizeOfRegTypeValueSets = 0U;
    uint32_t    PH_MEMLOC_REM dwRegister = 0U;
    uint32_t    PH_MEMLOC_REM dwTemp = 0U;
    uint32_t    PH_MEMLOC_REM dwValue = 0U;
    float32_t   PH_MEMLOC_REM fTime = 0.0;
    uint8_t     PH_MEMLOC_BUF wRegTypeValueSets[12] = {0};
    uint8_t *   PH_MEMLOC_REM pBuffer = NULL;
    uint16_t    PH_MEMLOC_REM wBufferSize;
    uint16_t    PH_MEMLOC_REM wBufferLen = 0U;

    /* Check if Apply Protocol is already done. */
    if (pDataParams->bCardType == PHHAL_HW_CARDTYPE_UNKNOWN)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_HAL);
    }

    switch(wConfig)
    {
        case PHHAL_HW_CONFIG_TXBUFFER_OFFSET:
            /* Update TX buffer offset based on the command used to buffer the data in HAL. */
            if(*pDataParams->pTxBuffer == PHHAL_HW_PN5190_INSTR_EXCHANGE_RF_DATA)
            {
                pDataParams->wAdditionalInfo = wValue + PHHAL_HW_PN5190_EXCHANGE_HEADER_SIZE;
            }
            else
            {
                pDataParams->wAdditionalInfo = wValue + PHHAL_HW_PN5190_TRANSMIT_HEADER_SIZE;
            }

            if ((pDataParams->bActiveMode != PH_ON) && (pDataParams->bNfcipMode == PH_ON))
            {
                 pDataParams->wAdditionalInfo += 1U;
            }
            break;

        case PHHAL_HW_CONFIG_RXBUFFER_STARTPOS:
            /* Retrieve RxBuffer parameters */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_GetRxBuffer(pDataParams, &pBuffer, &wBufferLen, &wBufferSize));

            /* Boundary check */
            if (wValue >= pDataParams->wRxBufSize)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_HAL);
            }

            /* Set start position */
            pDataParams->wRxBufStartPos = wValue;
            break;

        case PHHAL_HW_CONFIG_TXBUFFER_LENGTH:

            /* Retrieve TxBuffer parameters */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_GetTxBuffer(pDataParams, &pBuffer, &wBufferLen, &wBufferSize));

            /* Check parameter, must not exceed TxBufferSize */
            if (wValue > pDataParams->wTxBufSize)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_HAL);
            }

            if ((pDataParams->bActiveMode != PH_ON) && (pDataParams->bNfcipMode == PH_ON))
            {
                wValue++;
            }

            /* set buffer length */
            pDataParams->wTxBufLen = wValue;
            break;

        case PHHAL_HW_CONFIG_TXBUFFER:

            /* Retrieve TxBuffer parameters */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_GetTxBuffer(pDataParams, &pBuffer, &wBufferLen, &wBufferSize));

            /* Check parameter, must not exceed TxBufferSize */
            if (pDataParams->wAdditionalInfo >= pDataParams->wTxBufSize)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_HAL);
            }

            pBuffer[pDataParams->wAdditionalInfo] = (uint8_t)wValue;

            break;

        case PHHAL_HW_CONFIG_TXDATARATE_FRAMING:

            PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Int_SetTxDataRateFraming(pDataParams,wConfig,wValue));
            break;

        case PHHAL_HW_CONFIG_RXDATARATE_FRAMING:

            PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Int_SetRxDataRateFraming(pDataParams,wConfig,wValue));

            if (((pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXDATARATE_FRAMING] & PHHAL_HW_RF_DATARATE_OPTION_MASK) != PHHAL_HW_RF_DATARATE_106)
                && ((pDataParams->wCfgShadow[wConfig] & PHHAL_HW_RF_DATARATE_OPTION_MASK) == PHHAL_HW_RF_DATARATE_106))
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig_Int(pDataParams, PHHAL_HW_CONFIG_RXWAIT_US, 70U));
            }
            break;

        case PHHAL_HW_CONFIG_RFCA:

            if ((wValue != PH_ON) && (wValue != PH_OFF))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            pDataParams->bRfca = (uint8_t)wValue;
            break;


        case PHHAL_HW_CONFIG_SUBCARRIER:

            if (pDataParams->wCfgShadow[wConfig] != wValue)
            {
                /* Parameter check */
                if ((pDataParams->bCardType != PHHAL_HW_CARDTYPE_ISO15693) &&
                    (pDataParams->bCardType != PHHAL_HW_CARDTYPE_I18000P3M3))
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_HAL);
                }

                /* Only single subcarrier to be used while reception by ISO15693 */
                if ((pDataParams->bCardType == PHHAL_HW_CARDTYPE_ISO15693) && (wValue != PHHAL_HW_SUBCARRIER_SINGLE))
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_HAL);
                }

                /* Evaluate hardware settings */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_SetCardMode(
                        pDataParams,
                        ((pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXDATARATE_FRAMING]) & PHHAL_HW_RF_DATARATE_OPTION_MASK),
                        ((pDataParams->wCfgShadow[PHHAL_HW_CONFIG_RXDATARATE_FRAMING]) & PHHAL_HW_RF_DATARATE_OPTION_MASK),
                        wValue));

                /* Update Subcarrier setting */
                pDataParams->wCfgShadow[PHHAL_HW_CONFIG_SUBCARRIER] = wValue;
            }

            break;

        case PHHAL_HW_CONFIG_POLL_GUARD_TIME_US:
            if (wValue == 0x00U)
            {
                return PH_ERR_SUCCESS;
            }

            pDataParams->bPollGuardTimeFlag = PH_ON;
#ifndef _WIN32
            (void)phOsal_EventClear(
                &(((phhalHw_Pn5190_DataParams_t *)(pDataParams))->HwEventObj.EventHandle),
                E_OS_EVENT_OPT_NONE,
                E_PH_OSAL_EVT_GT_EXP,
                NULL);

            PH_CHECK_SUCCESS_FCT(statusTmp, phDriver_TimerStart(
                PH_DRIVER_TIMER_MICRO_SECS,
                wValue,
                phhalHw_Pn5190_Int_GuardTimeCallBck));
#endif
            break;

        case PHHAL_HW_CONFIG_OPE_MODE:
            pDataParams->bOpeMode = (uint8_t)wValue;
            break;

        case PHHAL_HW_CONFIG_RFRESET_ON_TIMEOUT:

            if (wValue == PH_OFF)
            {
                pDataParams->bRfResetAfterTo = PH_OFF;
            }
            else
            {
                pDataParams->bRfResetAfterTo = PH_ON;
            }

            break;

        case PHHAL_HW_CONFIG_PARITY:
        case PHHAL_HW_CONFIG_ASK100:
        case PHHAL_HW_CONFIG_RXWAIT_US:
            if (pDataParams->wCfgShadow[wConfig] != wValue)
            {
                /* Use internal set config */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig_Int(pDataParams, wConfig, wValue));

                /* Write config data into shadow */
                pDataParams->wCfgShadow[wConfig] = wValue;
            }
            break;

        case PHHAL_HW_CONFIG_TXCRC:
        case PHHAL_HW_CONFIG_RXCRC:
            /* Check shadow while performing configuration in EMVCo mode, to have good timing performance.
             * As in EMVCo mode ApplyProtocolSettings shall always be followed by FieldOn function. */
            if (pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
            {
                if (pDataParams->wCfgShadow[wConfig] != wValue)
                {
                    /* Use internal set config */
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig_Int(pDataParams, wConfig, wValue));
                }
            }
            else
            {
                /* NOTE : Do not perform shadow check, as FieldOn function of PN5190 FW performs
                 * LoadRFConfiguration of previously applied Protocol configuration and the shadow
                 * value of HAL for Tx and Rx CRC doesn't have the value that is configured on CLIF. */

                /* Use internal set config */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig_Int(pDataParams, wConfig, wValue));
            }

            /* Write config data into shadow */
            pDataParams->wCfgShadow[wConfig] = wValue;
            break;

        case PHHAL_HW_CONFIG_TXLASTBITS:

            if (pDataParams->wCfgShadow[wConfig] != wValue)
            {
                /* Check parameter */
                if (wValue > MASK_TX_LASTBITS)
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
                }

                /* Write config data into shadow */
                pDataParams->wCfgShadow[wConfig] = wValue;
            }
            break;

        case PHHAL_HW_CONFIG_RXALIGN:

            if (pDataParams->wCfgShadow[wConfig] != wValue)
            {
                /* Check the parameter */
                if (wValue >  MASK_RX_LASTBITS)
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
                }

                PH_CHECK_SUCCESS_FCT(statusTmp,phhalHw_Pn5190_Int_ClearNSetRegFields(pDataParams,
                        CLIF_RX_PROTOCOL_CONFIG, CLIF_RX_PROTOCOL_CONFIG_RX_BIT_ALIGN_MASK,
                        CLIF_RX_PROTOCOL_CONFIG_RX_BIT_ALIGN_POS, wValue));


                /* Write config data into shadow */
                pDataParams->wCfgShadow[wConfig] = wValue;
            }
            break;

        case PHHAL_HW_CONFIG_TXWAIT_US:

            if (pDataParams->wCfgShadow[wConfig] != wValue)
            {
                /* Set TxWait */
                fTime = 0.0;
                dwValue = PHHAL_HW_Pn5190_TR_RX_PRESCALAR;
                dwTemp = wValue;

                if(0U != dwTemp)
                {
                    fTime = (float32_t)(((float32_t)dwTemp * 13.56) / (float32_t)(dwValue));
                    dwTemp = (uint32_t)fTime;
                    dwValue |= (uint32_t)((dwTemp ) << CLIF_TX_WAIT_TX_WAIT_VALUE_POS);
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegister(pDataParams, CLIF_TX_WAIT, dwValue));
                }
                else
                {
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegister(pDataParams, CLIF_TX_WAIT, dwValue));
                }

                /* Write configuration data into shadow */
                pDataParams->wCfgShadow[wConfig] = wValue;
            }
            break;

        case PHHAL_HW_CONFIG_DISABLE_MF_CRYPTO1:
            /* Disable crypto, enabling is not supported */
            if ((wValue != PH_OFF) && ( pDataParams->bMfcCryptoEnabled == PH_ON))
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams, SYSTEM_CONFIG, (uint32_t)(~(SYSTEM_CONFIG_MFC_CRYPTO_ON_MASK))));
                pDataParams->bMfcCryptoEnabled = PH_OFF;
            }
            break;

        case PHHAL_HW_CONFIG_TIMEOUT_VALUE_US:
            if ((pDataParams->wCfgShadow[wConfig] != wValue) || (pDataParams->bTimeoutUnit != PHHAL_HW_TIME_MICROSECONDS) || (pDataParams->bOpeMode != RD_LIB_MODE_EMVCO))
            {
                PH_CHECK_SUCCESS_FCT(statusTmp,
                                     phhalHw_Pn5190_Int_SetTmo(
                                             pDataParams,
                                             wValue,
                                             PHHAL_HW_TIME_MICROSECONDS));
                /* Write config data into shadow */
                pDataParams->wCfgShadow[wConfig] = wValue;
                pDataParams->bTimeoutUnit = PHHAL_HW_TIME_MICROSECONDS;
            }
            break;

        case PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS:
            if ((pDataParams->wCfgShadow[wConfig] != wValue) || (pDataParams->bTimeoutUnit != PHHAL_HW_TIME_MILLISECONDS) || (pDataParams->bOpeMode != RD_LIB_MODE_EMVCO))
            {
                PH_CHECK_SUCCESS_FCT(statusTmp,
                                     phhalHw_Pn5190_Int_SetTmo(
                                             pDataParams,
                                             wValue,
                                             PHHAL_HW_TIME_MILLISECONDS));
                /* Write config data into shadow */
                pDataParams->wCfgShadow[wConfig] = wValue;
                pDataParams->bTimeoutUnit = PHHAL_HW_TIME_MILLISECONDS;
            }
            break;

        case PHHAL_HW_CONFIG_NFCIP_STARTBYTE:
            if((wValue != PH_ON) && (wValue != PH_OFF))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            pDataParams->bNfcipMode = (uint8_t)wValue;
            break;

        case PHHAL_HW_CONFIG_JEWEL_MODE:

            if (wValue == PH_ON)
            {
                pDataParams->bJewelActivated = (uint8_t)wValue;
                /* Enable FrameStep in Transceive Control Register. */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterOrMask( pDataParams, CLIF_TRANSCEIVE_CONTROL, CLIF_TRANSCEIVE_CONTROL_TX_FRAMESTEP_ENABLE_MASK));
            }
            else if (wValue == PH_OFF)
            {
                pDataParams->bJewelActivated = (uint8_t)wValue;
                /* Disable FrameStep in Transceive Control Register. */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask( pDataParams, CLIF_TRANSCEIVE_CONTROL, (uint32_t )~(CLIF_TRANSCEIVE_CONTROL_TX_FRAMESTEP_ENABLE_MASK)));
            }
            else
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            break;

        case PHHAL_HW_CONFIG_SET_EMD:
            pDataParams->bEmdFlag = (uint8_t)wValue;
            if(wValue == 0x1U)
            {
                dwRegister = 0U;

                /* Read the EMD CONTROL Register to get RM EMD SENSITIVITY field */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_ReadRegister(pDataParams, EMD_CONTROL, &dwRegister));

                /* Retain only the RM EMD SENSITIVITY value field. */
                dwRegister &= (uint32_t)(EMD_CONTROL_EMD_RM_EMD_SENSITIVITY_MASK);

                if(pDataParams->bOpeMode != RD_LIB_MODE_ISO)
                {
                    dwRegister |= PHHAL_HW_PN5190_EMD_3P0;
                }
                else
                {
                    dwRegister |= PHHAL_HW_PN5190_ISO_EMD;
                }
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegister(pDataParams, EMD_CONTROL, dwRegister));

                /* Set the RM RESYNC ENABLE in SIGPRO RM ENABLE Register. */
                if (pDataParams->bCardType == PHHAL_HW_CARDTYPE_ISO14443A)
                {
                    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterOrMask(pDataParams, CLIF_SIGPRO_RM_ENABLES, CLIF_SIGPRO_RM_ENABLES_RM_RESYNC_RESET_ENABLE_MASK));
                }
            }
            else
            {
                /* Clear EMD Enable bit in EMD Control Register */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams, EMD_CONTROL,(uint32_t)(~EMD_CONTROL_EMD_ENABLE_MASK)));
            }
            break;

        case PHHAL_HW_CONFIG_RXMULTIPLE:

            if (wValue == PH_ON)
            {
                pDataParams->bRxMultiple = PH_ON;
                /* Enable Rx Multiple in Transceive Control Register. */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterOrMask( pDataParams, CLIF_TRANSCEIVE_CONTROL, CLIF_TRANSCEIVE_CONTROL_RX_MULTIPLE_ENABLE_MASK));
                /* Reset Timer1 (FDT Timer) stop condition. */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams, CLIF_TIMER1_CONFIG, (uint32_t )~(CLIF_TIMER1_CONFIG_T1_STOP_ON_RX_STARTED_MASK)));
            }
            else
            {
                pDataParams->bRxMultiple = PH_OFF;
                /* Disable Rx Multiple in Transceive Control Register. */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask( pDataParams, CLIF_TRANSCEIVE_CONTROL, (uint32_t )~(CLIF_TRANSCEIVE_CONTROL_RX_MULTIPLE_ENABLE_MASK)));
                /* Set Timer1 (FDT Timer) stop condition to Stop on RX Start condition. */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterOrMask(pDataParams, CLIF_TIMER1_CONFIG, CLIF_TIMER1_CONFIG_T1_STOP_ON_RX_STARTED_MASK));
            }
            break;

        case PHHAL_HW_CONFIG_MFHALTED:
            if((wValue != PH_ON) && (wValue != PH_OFF))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            if(wValue == PH_ON)
            {
               PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_Pn5190_Instr_WriteRegisterOrMask(pDataParams, SYSTEM_CONFIG, SYSTEM_CONFIG_AUTOCOLL_STATE_A_MASK));
               pDataParams->bCardMode  = PH_ON;
            }
            else
            {
                PH_CHECK_SUCCESS_FCT(statusTmp,
                    phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams, SYSTEM_CONFIG, (uint32_t)~SYSTEM_CONFIG_AUTOCOLL_STATE_A_MASK));
                pDataParams->bCardMode  = PH_OFF;
            }
            break;

        case PHHAL_HW_CONFIG_FIELD_OFF_TIME:
            /* Off time shall not be zero. */
            if (wValue == 0U)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            /* Store off time. */
            pDataParams->wFieldOffTime = wValue;
            break;

        case PHHAL_HW_CONFIG_FIELD_RECOVERY_TIME:
            /* Store recovery time. */
            pDataParams->wFieldRecoveryTime = wValue;
            break;

        case PHHAL_HW_CONFIG_SET_SYMBOL_SEND:

            /* check parameter */
            if ((wValue != PH_ON) && (wValue != PH_OFF))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            /* PH_ON implies Clearing Bit-3 (DataEn Bit in TxDataNum). */
            if (wValue == PH_ON)
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams, CLIF_TX_FRAME_CONFIG,  ~(uint32_t)CLIF_TX_FRAME_CONFIG_TX_DATA_ENABLE_MASK));
            }
            /* PH_OFF implies Setting Bit-3 (DataEn Bit in TxDataNum). */
            else
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterOrMask(pDataParams, CLIF_TX_FRAME_CONFIG, CLIF_TX_FRAME_CONFIG_TX_DATA_ENABLE_MASK));
            }
            break;


        case PHHAL_HW_CONFIG_SYMBOL_START:

            /* Parameter check */
            if ((pDataParams->bCardType != PHHAL_HW_CARDTYPE_ISO15693) &&
                (pDataParams->bCardType != PHHAL_HW_CARDTYPE_I18000P3M3))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_HAL);
            }

            /* Parameter check #2 */
            switch (wValue)
            {
                case PHHAL_HW_SYMBOL_I15693_SOF:
                    if (pDataParams->bCardType != PHHAL_HW_CARDTYPE_ISO15693)
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
                    }
                    break;
                case PHHAL_HW_SYMBOL_I18000P3M3_PREAMBLE:
                case PHHAL_HW_SYMBOL_I18000P3M3_FSYNC:
                    if (pDataParams->bCardType != PHHAL_HW_CARDTYPE_I18000P3M3)
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
                    }
                    break;
                case PH_OFF:
                    break;
                default:
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_HAL);
            }

            wSizeOfRegTypeValueSets = 0U;

            /*Clear the Bits of TX_CONFIG_TX_START_SYMBOL_MASK & DATA_ENABLE*/
            if(wValue == PH_OFF)
            {
                dwTemp = (uint32_t) ~( CLIF_TX_FRAME_CONFIG_TX_START_SYMBOL_MASK | CLIF_TX_FRAME_CONFIG_TX_DATA_ENABLE_MASK);
            }
            else
            {
                dwTemp = (uint32_t) ~( CLIF_TX_FRAME_CONFIG_TX_START_SYMBOL_MASK);
            }


            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = CLIF_TX_FRAME_CONFIG;
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = PHHAL_HW_PN5190_WRITE_MULTIPLE_TYPE_WRITE_AND_MASK;
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>8U);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>16U);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>24U);

            dwValue = 0U;

            /* Set new symbol */
            switch (wValue)
            {
            case PHHAL_HW_SYMBOL_I15693_SOF:
                dwValue = 0x3U;
                break;
            case PHHAL_HW_SYMBOL_I18000P3M3_PREAMBLE:
                dwValue = 0x1U;
                break;
            case PHHAL_HW_SYMBOL_I18000P3M3_FSYNC:
                dwValue = 0x2U;
                break;
            default:
                break;
            }

            /*Set the new value  */
            dwTemp = (uint32_t)((dwValue << CLIF_TX_FRAME_CONFIG_TX_START_SYMBOL_POS) & CLIF_TX_FRAME_CONFIG_TX_START_SYMBOL_MASK);
            dwTemp |= (uint32_t)((dwValue << CLIF_TX_FRAME_CONFIG_TX_DATA_ENABLE_POS) & CLIF_TX_FRAME_CONFIG_TX_DATA_ENABLE_MASK);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = CLIF_TX_FRAME_CONFIG;
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = PHHAL_HW_PN5190_WRITE_MULTIPLE_TYPE_WRITE_OR_MASK;
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>8U);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>16U);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>24U);

            /*Send the array to the IC*/
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterMultiple(pDataParams,  wRegTypeValueSets, wSizeOfRegTypeValueSets));

            break;

        case PHHAL_HW_CONFIG_CRCTYPE:

            if(!(wValue & (PHHAL_HW_CRCTYPE_CRC5 | PHHAL_HW_CRCTYPE_CRC16 | PHHAL_HW_CRCOPTION_INVERTED)))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            dwValue = 0U;
            /*dwRegister will contain all the bits that need to be zeroed out*/
            dwRegister = 0U;

            if(0U != (wValue & PHHAL_HW_CRCTYPE_CRC16))
            {
                dwRegister |= CLIF_CRC_TX_CONFIG_TX_CRC_TYPE_MASK;
            }
            else
            {
                /* QAC */
            }

            if(0U != (wValue & PHHAL_HW_CRCOPTION_INVERTED))
            {
                dwValue |= CLIF_CRC_TX_CONFIG_TX_CRC_INV_MASK;
            }
            else
            {
                dwRegister |= CLIF_CRC_TX_CONFIG_TX_CRC_INV_MASK;
            }

            if ((pDataParams->bCardType == PHHAL_HW_CARDTYPE_I18000P3M3) && (0U != ((wValue & PHHAL_HW_CRCTYPE_CRC16))))
            {
                dwRegister |= CLIF_CRC_TX_CONFIG_TX_CRC_PRESET_SEL_MASK;
                dwValue |= (0x3U << CLIF_CRC_TX_CONFIG_TX_CRC_PRESET_SEL_POS);
            }


            wSizeOfRegTypeValueSets = 0U;

            /*Clear the Bits */
            dwTemp = (uint32_t) ~( dwRegister );
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = CLIF_CRC_TX_CONFIG;
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = PHHAL_HW_PN5190_WRITE_MULTIPLE_TYPE_WRITE_AND_MASK;
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>8U);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>16U);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>24U);

            /*Set the new value  */
            dwTemp = dwValue;
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = CLIF_CRC_TX_CONFIG;
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = PHHAL_HW_PN5190_WRITE_MULTIPLE_TYPE_WRITE_OR_MASK;
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>8U);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>16U);
            wRegTypeValueSets[wSizeOfRegTypeValueSets++] = (uint8_t)(dwTemp>>24U);

            /*Send the array to the IC*/
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterMultiple(pDataParams,  wRegTypeValueSets, wSizeOfRegTypeValueSets));

            break;

        case PHHAL_HW_CONFIG_TXWAIT_MS:
            /* Parameter checking */
            if (wValue > 650U) /* To set Max value for wValue in uS i.e, 0xFFFFU )*/
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            /* Store the value in terms of uS in shadow table*/
            pDataParams->dwTxWaitMs =  wValue;

            /* Set TxWait */
            fTime = 0.0;
            dwValue = PHHAL_HW_Pn5190_TR_RX_PRESCALAR;
            dwTemp = wValue*1000U;
            if(0U != dwTemp)
            {
                fTime = (float32_t)(((float32_t)dwTemp * 13.56) / (float32_t)(dwValue));
                dwTemp = (uint32_t)fTime;
                dwValue |= (uint32_t)((dwTemp ) << CLIF_TX_WAIT_TX_WAIT_VALUE_POS);
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegister(pDataParams, CLIF_TX_WAIT, dwValue));
            }
            else
            {
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegister(pDataParams, CLIF_TX_WAIT, dwValue));
            }
            break;

        case PHHAL_HW_CONFIG_SETMINFDT:

            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_SetMinFdt(pDataParams, wValue));
            break;

        case PHHAL_HW_CONFIG_RFON_INTERRUPT:
            if((wValue != PH_ON) && (wValue != PH_OFF))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            if(wValue == PH_ON)
            {
                /* This will Enable RF-ON Interrupt in PN5190. PN5190 will send PH_PN5190_EVT_RFON_DETECT
                 * after RF-ON interrupt*/
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegister(pDataParams,
                        EVENT_ENABLE, EVENT_ENABLE_RFON_DET_EVENT_ENABLE_MASK));
                pDataParams->dwExpectedEvent = PH_PN5190_EVT_RFON_DETECT;
            }
            else
            {
                /* Disable RF-ON Interrupt*/
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams,
                        EVENT_ENABLE,  ~(uint32_t)EVENT_ENABLE_RFON_DET_EVENT_ENABLE_MASK));
            }
            break;

        case PHHAL_HW_CONFIG_TIMING_MODE:
        {
            /* Check supported option bits */
            switch (wValue & PHHAL_HW_TIMING_MODE_OPTION_MASK)
            {
            case PHHAL_HW_TIMING_MODE_OPTION_DEFAULT:
            case PHHAL_HW_TIMING_MODE_OPTION_AUTOCLEAR:
                break;
            default:
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            /* Check supported timing modes */
            switch (wValue & (uint16_t)~(uint16_t)PHHAL_HW_TIMING_MODE_OPTION_MASK)
            {
            case PHHAL_HW_TIMING_MODE_OFF:
            case PHHAL_HW_TIMING_MODE_FDT:
                pDataParams->dwTimingUs = 0U;
                pDataParams->wTimingMode = wValue;
                break;
            case PHHAL_HW_TIMING_MODE_COMM:
                return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_HAL);
            default:
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            break;
        }

        case PHHAL_HW_CONFIG_TIMER_DEVIATION_PROXIMITY_CHECK:
            /* Max timer deviation should be 5 percentage */
            if(wValue > 5)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            pDataParams->bTimerDeviation = wValue;
        break;

        case PHHAL_HW_PN5190_CONFIG_CTS_EVENT_STATUS:
            /* check parameter */
            if ((wValue != PH_ON) && (wValue != PH_OFF))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            pDataParams->bCTSEvent = wValue;
            break;

        case PHHAL_HW_PN5190_CONFIG_CTS_EVENT_ENABLE:
            if(wValue == PH_ON)
            {
                /* Enable CTS Event Interrupt*/
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegister(pDataParams, EVENT_ENABLE, EVENT_ENABLE_CTS_EVENT_ENABLE_MASK));
            }
            else if (wValue == PH_OFF)
            {
                /* Disable CTS Event Interrupt*/
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams, EVENT_ENABLE,  ~(uint32_t)EVENT_ENABLE_CTS_EVENT_ENABLE_MASK));
            }
            else /* check parameter */
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            break;

        case PHHAL_HW_PN5190_CONFIG_LPCD_EVENT_ENABLE:
            if(wValue == PH_ON)
            {
                /* Enable LPCD Event */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterOrMask(pDataParams, EVENT_ENABLE, EVENT_ENABLE_LPCD_EVENT_ENABLE_MASK));
            }
            else if (wValue == PH_OFF)
            {
                /* Disable LPCD Event */
                PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams, EVENT_ENABLE, (uint32_t)~(EVENT_ENABLE_LPCD_EVENT_ENABLE_MASK)));
            }
            else
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }
            break;

        case PHHAL_HW_PN5190_CONFIG_EXPECTED_EVENT:
            if ((wValue < PH_PN5190_EVT_BOOT) || (wValue >= PH_PN5190_EVT_RFU))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            pDataParams->dwExpectedEvent = (uint32_t) wValue;
            break;

        case PHHAL_HW_PN5190_CONFIG_RF_ON_EVENT_STATUS:
            /* check parameter */
            if ((wValue != PH_ON) && (wValue != PH_OFF))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            pDataParams->bRFONEvent = wValue;
            break;

        case PHHAL_HW_PN5190_CONFIG_ABORT_CMD:
            /* check parameter */
            if ((wValue != PH_PN5190_SWITCH_MODE_NORMAL_2_1) && (wValue != PH_PN5190_SWITCH_MODE_NORMAL_2_2))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            pDataParams->bAbortCMD = wValue;
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD:
            if ((wValue == PH_ON) && (pDataParams->bOpeMode != RD_LIB_MODE_FELICA))
            {
                /* Before Setting FELICA_EMD bit, OpeMode need to be configured to RD_LIB_MODE_FELICA */
                return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_HAL);
            }

            /* Configure FeliCa EMD enable bit in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegBit(pDataParams, wValue, (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_ENABLE_MASK));
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD_LEN_CHECK:
            /* Configure EMD length byte check enable bit in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegBit(pDataParams, wValue, (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_LEN_CHECK_ENABLE_MASK));
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD_RC_CHECK:
            /* Configure RC byte check enable bit in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegBit(pDataParams, wValue, (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_RC_CHECK_ENABLE_MASK));
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD_PROTOCOL_ERR_CHECK:
            /* Configure Protocol error check enable bit in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegBit(pDataParams, wValue, (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_PROTOCOL_ERR_CHECK_ENABLE_MASK));
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD_INTEGRITY_ERR_CHECK:
            /* Configure Integrity error check enable bit in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegBit(pDataParams, wValue, (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_INTEGRITY_ERR_CHECK_ENABLE_MASK));
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD_RC_CHECK_ON_CRC_CORRECT:
            /* Configure RC check on correct CRC enable bit in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegBit(pDataParams, wValue, (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_RC_CHECK_ON_CRC_CORRECT_ENABLE_MASK));
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD_LOG_ENABLE:
            /* Configure Log enable bit in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegBit(pDataParams, wValue, (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_LOG_ENABLE_MASK));
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_RX_ONLY:
            /* check parameter */
            if ((wValue != PH_ON) && (wValue != PH_OFF))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
            }

            pDataParams->bFeliCaRxOnly = wValue;
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD_LEN_BYTE_MIN:
            /* Configure min length byte value in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegByte(
                    pDataParams,
                    wValue,
                    (uint8_t)FELICA_EMD_CONTROL_FELICA_EMD_LENGTH_BYTE_MIN_POS,
                    (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_LENGTH_BYTE_MIN_MASK));
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD_LEN_BYTE_MAX:
            /* Configure max length byte value in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegByte(
                    pDataParams,
                    wValue,
                    (uint8_t)FELICA_EMD_CONTROL_FELICA_EMD_LENGTH_BYTE_MAX_POS,
                    (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_LENGTH_BYTE_MAX_MASK));
            break;

        case PHHAL_HW_CONFIG_SET_FELICA_EMD_RC_BYTE_VAL:
            /* Configure RC byte value in FELICA_EMD_CONTROL Register and shadow register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Int_SetConfig_FelicaEmdRegByte(
                    pDataParams,
                    wValue,
                    (uint8_t)FELICA_EMD_CONTROL_FELICA_EMD_RC_BYTE_VALUE_POS,
                    (uint32_t)FELICA_EMD_CONTROL_FELICA_EMD_RC_BYTE_VALUE_MASK));
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_HAL);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phhalHw_Pn5190_GetConfig(
                                    phhalHw_Pn5190_DataParams_t * pDataParams,
                                    uint16_t wConfig,
                                    uint16_t * pValue
                                    )
{
    phStatus_t PH_MEMLOC_REM statusTmp;
    uint16_t   PH_MEMLOC_REM wBufferLen;
    uint16_t   PH_MEMLOC_REM wBufferSize;
    uint32_t   PH_MEMLOC_REM dwRegister;
    uint8_t *  PH_MEMLOC_REM pBuffer;

    /* Check all the pointers */
    PH_ASSERT_NULL(pValue);

    switch(wConfig)
    {
        case PHHAL_HW_CONFIG_OPE_MODE:
            *pValue = pDataParams->bOpeMode;
            break;

        case PHHAL_HW_CONFIG_SET_EMD:
            *pValue = pDataParams->bEmdFlag;
            break;
        case PHHAL_HW_CONFIG_PARITY:
        case PHHAL_HW_CONFIG_TXCRC:
        case PHHAL_HW_CONFIG_RXCRC:
        case PHHAL_HW_CONFIG_TXLASTBITS:
        case PHHAL_HW_CONFIG_TXDATARATE_FRAMING:
        case PHHAL_HW_CONFIG_RXDATARATE_FRAMING:
        case PHHAL_HW_CONFIG_ASK100:
        case PHHAL_HW_CONFIG_SUBCARRIER:
        case PHHAL_HW_CONFIG_TXWAIT_US:
        case PHHAL_HW_CONFIG_RXWAIT_US:

          /* Read config from shadow */
          *pValue = pDataParams->wCfgShadow[wConfig];
          break;

        case PHHAL_HW_CONFIG_TXBUFFER_OFFSET:
        case PHHAL_HW_CONFIG_RXLASTBITS:

            *pValue = pDataParams->wAdditionalInfo;
            break;

        case PHHAL_HW_CONFIG_TIMEOUT_VALUE_US:

            if (pDataParams->bTimeoutUnit == PHHAL_HW_TIME_MICROSECONDS)
            {
                *pValue = pDataParams->wCfgShadow[wConfig];
            }
            else
            {
                if (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS] > (0xFFFFU / 1000U))
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_PARAMETER_OVERFLOW, PH_COMP_HAL);
                }
                *pValue = pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS] * 1000U;
            }
            break;

        case PHHAL_HW_CONFIG_TIMEOUT_VALUE_MS:

            if (pDataParams->bTimeoutUnit == PHHAL_HW_TIME_MILLISECONDS)
            {
                *pValue = pDataParams->wCfgShadow[wConfig];
            }
            else
            {
                *pValue = pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TIMEOUT_VALUE_US] / 1000U;
            }
            break;

        case PHHAL_HW_CONFIG_FIELD_OFF_TIME:

            *pValue = pDataParams->wFieldOffTime;
            break;

        case PHHAL_HW_CONFIG_FIELD_RECOVERY_TIME:

            *pValue = pDataParams->wFieldRecoveryTime;
            break;

        case PHHAL_HW_CONFIG_JEWEL_MODE:

            /* Write configuration data into shadow */
            *pValue = pDataParams->bJewelActivated;
            break;

        case PHHAL_HW_CONFIG_RFRESET_ON_TIMEOUT:

            *pValue = pDataParams->bRfResetAfterTo;
            break;

        case PHHAL_HW_CONFIG_DISABLE_MF_CRYPTO1:

            /* Check Crypto1On Status */
            if ( pDataParams->bMfcCryptoEnabled == PH_ON)
            {
                *pValue = PH_OFF; /* OFF in this case means "Crypto1 not disabled --> enabled" */
            }
            else
            {
                *pValue = PH_ON; /* ON in this case means "Crypto1 is disabled" */
            }
            break;


        case PHHAL_HW_CONFIG_RXBUFFER_STARTPOS:

            /* Return parameter */
            *pValue = pDataParams->wRxBufStartPos;
            break;

        case PHHAL_HW_CONFIG_RXBUFFER_BUFSIZE:

            /* Retrieve RxBuffer parameters */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_GetRxBuffer(pDataParams, &pBuffer, &wBufferLen, &wBufferSize));

            /* Return parameter */
            *pValue = wBufferSize;
            break;

        case PHHAL_HW_CONFIG_TXBUFFER_BUFSIZE:

            /* Retrieve TxBuffer parameters */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_GetTxBuffer(pDataParams, &pBuffer, &wBufferLen, &wBufferSize));

            /* Return parameter */
            *pValue = wBufferSize;
            break;

        case PHHAL_HW_CONFIG_TXBUFFER_LENGTH:

            /* Retrieve TxBuffer parameters */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_GetTxBuffer(pDataParams, &pBuffer, &wBufferLen, &wBufferSize));
            if ((pDataParams->bActiveMode != PH_ON) && (pDataParams->bNfcipMode == PH_ON))
            {
                if (wBufferLen != 0)
                {
                    wBufferLen--;
                }
            }
            /* Return parameter */
            *pValue = wBufferLen;
            break;

        case PHHAL_HW_CONFIG_TXBUFFER:

            /* Retrieve TxBuffer parameters */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_GetTxBuffer(pDataParams, &pBuffer, &wBufferLen, &wBufferSize));

            /* Check additional info parameter */
            if (pDataParams->wAdditionalInfo >= pDataParams->wTxBufSize)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_HAL);
            }

            /* Retrieve TxBuffer byte */
            *pValue = (uint16_t)pBuffer[pDataParams->wAdditionalInfo];
            break;

        case PHHAL_HW_CONFIG_RXMULTIPLE:
            *pValue =  pDataParams->bRxMultiple;
            break;

        case PHHAL_HW_CONFIG_NFCIP_STARTBYTE:
            *pValue = pDataParams->bNfcipMode;
            break;

        case PHHAL_HW_CONFIG_TARGET_DATARATE_FRAMING:
            *pValue = pDataParams->wTargetMode;
            break;

        case PHHAL_HW_CONFIG_ACTIVEMODE:
            /* Return parameter */
            *pValue = (uint16_t)pDataParams->bActiveMode;
            break;

        case PHHAL_HW_CONFIG_CARD_TYPE:
            *pValue = pDataParams->bCardType;
            break;

        case PHHAL_HW_CONFIG_EXT_RF_ON:
            /* Retrieve Status register */
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Instr_ReadRegister(pDataParams, CLIF_STATUS, &dwRegister));

            /* Check EXT_RF_ON bit */
            if (0U != (dwRegister & CLIF_STATUS_RF_DET_STATUS_MASK))
            {
                *pValue = PH_ON; /* ON if external RF field is detected */
            }
            else
            {
                *pValue = PH_OFF; /* OFF if no external RF field is detected */
            }
            break;

        case PHHAL_HW_CONFIG_MFHALTED:
            PH_CHECK_SUCCESS_FCT(statusTmp,
                phhalHw_Pn5190_Instr_ReadRegister(pDataParams, SYSTEM_CONFIG, &dwRegister));
            if(0U != (dwRegister & SYSTEM_CONFIG_AUTOCOLL_STATE_A_MASK))
            {
                *pValue = PH_ON;
            }
            else
            {
                *pValue = PH_OFF;
            }
            break;

        case PHHAL_HW_CONFIG_INT_RF_ON:

            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_ReadRegister(pDataParams, CLIF_STATUS, &dwRegister));
            if((dwRegister & CLIF_STATUS_TX_RF_STATUS_MASK ) == CLIF_STATUS_TX_RF_STATUS_MASK)
            {
                *pValue = PH_ON;
            }
            else
            {
                *pValue = PH_OFF;
            }
            break;

        case PHHAL_HW_CONFIG_TIMING_MODE:
        {
            *pValue = pDataParams->wTimingMode;
            break;
        }

        case PHHAL_HW_CONFIG_TIMING_US:
        {
            if (pDataParams->dwTimingUs > 0xFFFFU)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_PARAMETER_OVERFLOW, PH_COMP_HAL);
            }

            *pValue = (uint16_t)pDataParams->dwTimingUs;
            pDataParams->dwTimingUs = 0U;
            break;
        }

        case PHHAL_HW_CONFIG_TIMING_MS:
        {
            if (pDataParams->dwTimingUs > (0xFFFFU * 1000U))
            {
                pDataParams->dwTimingUs = 0U;
                return PH_ADD_COMPCODE_FIXED(PH_ERR_PARAMETER_OVERFLOW, PH_COMP_HAL);
            }

            *pValue = (uint16_t)(pDataParams->dwTimingUs / 1000U);
            pDataParams->dwTimingUs = 0U;
            break;
        }

        case PHHAL_HW_CONFIG_TIMER_DEVIATION_PROXIMITY_CHECK:
        {
            *pValue = pDataParams->bTimerDeviation;
            break;
        }

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_PARAMETER, PH_COMP_HAL);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phhalHw_Pn5190_SetListenParameters(
                                              phhalHw_Pn5190_DataParams_t * pDataParams,
                                              uint8_t * pSensRes,
                                              uint8_t * pNfcId1,
                                              uint8_t SelRes,
                                              uint8_t * pPollingResp,
                                              uint8_t bNfcId3
                                              )
{
    /* Check all the pointers */
    PH_ASSERT_NULL(pSensRes);
    PH_ASSERT_NULL(pNfcId1);
    PH_ASSERT_NULL(pPollingResp);

    uint8_t PH_MEMLOC_BUF aCmd[PHHAL_HW_PN5190_SET_LISTEN_LEN];
    if(0U != bNfcId3)
    {
        /*Coverity*/;
    }

    (void)memcpy(&aCmd[0], pSensRes, PHHAL_HW_PN5190_SET_LISTEN_SENRES_LEN);
    (void)memcpy(&aCmd[2], pNfcId1, PHHAL_HW_PN5190_SET_LISTEN_NFCID1_LEN);
    (void)memcpy(&aCmd[5], &SelRes, PHHAL_HW_PN5190_SET_LISTEN_SELRES_LEN);
    (void)memcpy(&aCmd[6], pPollingResp, PHHAL_HW_PN5190_SET_LISTEN_POLLRES_LEN);

    return phhalHw_Pn5190_Instr_WriteE2Prom(pDataParams, PHHAL_HW_PN5190_SET_LISTEN_EEPROM_ADDR, aCmd, PHHAL_HW_PN5190_SET_LISTEN_LEN);
}

phStatus_t phhalHw_Pn5190_Exchange(
                                   phhalHw_Pn5190_DataParams_t * pDataParams,
                                   uint16_t wOption,
                                   uint8_t * pTxBuffer,
                                   uint16_t wTxLength,
                                   uint8_t ** ppRxBuffer,
                                   uint16_t * pRxLength
                                   )
{
    phStatus_t         PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    phStatus_t         PH_MEMLOC_REM statusTmp = PH_ERR_SUCCESS;
    uint8_t            PH_MEMLOC_REM bTxLastBits = 0x00U;
    uint8_t            PH_MEMLOC_REM bRespConfig = 0x00U;
    phOsal_EventBits_t PH_MEMLOC_REM dwEventFlags = 0;
    uint8_t            PH_MEMLOC_REM bSyncByte = 0xf0;
    uint32_t           PH_MEMLOC_REM dwTimingSingle = 0;

    /* Check all the pointers */
    if (0U != (wTxLength)) PH_ASSERT_NULL_PARAM(pTxBuffer, PH_COMP_HAL);

    do
    {
        if (pDataParams->wTargetMode != PH_OFF)
        {
            /* If wTargetMode is changed then Target is activated and Exchange should not be used to perform Tx Operation. */
            statusTmp = (PH_ERR_USE_CONDITION | PH_COMP_HAL);
            break;
        }

        /* Check options */
        if (0U != (wOption & (uint16_t)~(uint16_t)(PH_EXCHANGE_BUFFERED_BIT | PH_EXCHANGE_LEAVE_BUFFER_BIT)))
        {
            statusTmp = (PH_ERR_INVALID_PARAMETER | PH_COMP_HAL);
            break;
        }

        /* Validate input parameters. */
        if (((ppRxBuffer == NULL) || (pRxLength == NULL)) && ((wOption & PH_EXCHANGE_BUFFERED_BIT) == 0U))
        {
            statusTmp = (PH_ERR_INVALID_PARAMETER | PH_COMP_HAL);
            break;
        }

        /* Validate received parameter */
        if (pRxLength != NULL)
        {
            *pRxLength = 0U;
        }

        if (pDataParams->bPollGuardTimeFlag == PH_ON)
        {
          pDataParams->bPollGuardTimeFlag = PH_OFF;

          /* When there is no errors in preconditions wait till timer expire. */
          if (statusTmp == PH_ERR_SUCCESS)
          {
              /* Wait infinitely for the Poll Guard Time to Expire. */
              (void)phOsal_EventPend(&xEventHandle, E_OS_EVENT_OPT_PEND_SET_ANY | E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT,
                      PHOSAL_MAX_DELAY, E_PH_OSAL_EVT_GT_EXP | E_PH_OSAL_EVT_ABORT, &dwEventFlags);
              if(0U != (dwEventFlags & E_PH_OSAL_EVT_ABORT))
              {
                  statusTmp = PH_ERR_ABORTED;
              }
          }
          /* Disable Timer */
          PH_CHECK_SUCCESS_FCT(status, phDriver_TimerStop());
        }
    }
    while (0);

    if (statusTmp != PH_ERR_SUCCESS)
    {
      return statusTmp;
    }

#ifndef _WIN32
    (void)phOsal_EventClear(&pDataParams->HwEventObj.EventHandle, E_OS_EVENT_OPT_NONE, E_PH_OSAL_EVT_RF, NULL);
#endif

    if (pDataParams->bJewelActivated == PH_OFF)
    {
        bTxLastBits = (uint8_t)pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXLASTBITS];
    }
    else
    {
        bTxLastBits = 0x07U;

        /* Set Timer1 (FDT Timer) start condition to Tx Frame Step detected instead of Tx Ended. */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(
            pDataParams,
            CLIF_TIMER1_CONFIG,
            (uint32_t )~(CLIF_TIMER1_CONFIG_T1_START_ON_TX_ENDED_MASK)));

        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterOrMask(
            pDataParams,
            CLIF_TIMER1_CONFIG,
            CLIF_TIMER1_CONFIG_T1_START_ON_TX_FRAMESTEP_MASK));
    }

    bRespConfig = PH_RF_EXCH_CONF_ENABLE_RF_STATUS | PH_RF_EXCH_CONF_ENABLE_RX_STATUS | PH_RF_EXCH_CONF_ENABLE_EVT_STATUS;

    if ((pDataParams->bActiveMode != PH_ON) && (pDataParams->bNfcipMode == PH_ON))
    {
        if((wOption == PH_EXCHANGE_BUFFER_FIRST) || (wOption == PH_EXCHANGE_DEFAULT))
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_ExchangeRFData(
                pDataParams,
                PH_EXCHANGE_BUFFER_FIRST,
                0,
                bRespConfig,
                &bSyncByte,
                1,
                NULL,
                0));

            if(wOption == PH_EXCHANGE_BUFFER_FIRST)
            {
                wOption = PH_EXCHANGE_BUFFER_CONT;
            }
            else
            {
                wOption = PH_EXCHANGE_BUFFER_LAST;
            }
        }
    }

    status = phhalHw_Pn5190_Instr_ExchangeRFData(pDataParams, wOption, bTxLastBits, bRespConfig,
            pTxBuffer, wTxLength, ppRxBuffer, pRxLength);
    PH_CHECK_ABORT(status);

    wTxLength = 0U;

    if ((status & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT)
    {
        /* Reset after timeout behavior */
        if (pDataParams->bRfResetAfterTo != PH_OFF)
        {
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_FieldReset(pDataParams));
        }
    }

    /* When buffering is enabled, return */
    if ( (status == PH_ERR_SUCCESS) &&
        ( 0U != ( wOption & PH_EXCHANGE_BUFFERED_BIT ) ) )
    {
        return PH_ERR_SUCCESS;
    }

    /* Reset TxLastBits */
    if (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXLASTBITS] != 0U)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig(pDataParams, PHHAL_HW_CONFIG_TXLASTBITS, 0x00U));
    }

    if (status == PH_ERR_SUCCESS)
    {
        status  = phhalHw_Pn5190_Int_Retrieve_Data(pDataParams, ppRxBuffer, pRxLength);
        PH_CHECK_ABORT(status);
    }

    if((status & PH_ERR_MASK) != PH_ERR_IO_TIMEOUT)
    {
        /* Timing */
        if ((pDataParams->wTimingMode & (uint16_t)~(uint16_t)PHHAL_HW_TIMING_MODE_OPTION_MASK) == PHHAL_HW_TIMING_MODE_FDT)
        {
            /* Calculate FDT Value */
            PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Int_PCD_GetExchgFdt(pDataParams, &dwTimingSingle));

            /* FDT single -> replace the previous value */
            if (0U != (pDataParams->wTimingMode & PHHAL_HW_TIMING_MODE_OPTION_AUTOCLEAR))
            {
                pDataParams->dwTimingUs = dwTimingSingle;
            }
            /* FDT -> add current value to current count */
            else
            {
                /* Check for overflow */
                if ((0xFFFFFFFFU - pDataParams->dwTimingUs) < dwTimingSingle)
                {
                    pDataParams->dwTimingUs = 0xFFFFFFFFU;
                }
                /* Update global timing value */
                else
                {
                    pDataParams->dwTimingUs = pDataParams->dwTimingUs + dwTimingSingle;
                }
            }
        }
    }

    if (pDataParams->bJewelActivated == PH_ON)
    {
        /* Reset Timer1 (FDT Timer) start condition to Tx Ended instead of Tx Frame Step detected. */
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams,
                CLIF_TIMER1_CONFIG, (uint32_t )~(CLIF_TIMER1_CONFIG_T1_START_ON_TX_FRAMESTEP_MASK)));
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterOrMask(pDataParams,
                CLIF_TIMER1_CONFIG, CLIF_TIMER1_CONFIG_T1_START_ON_TX_ENDED_MASK));
    }

    if (((status & PH_ERR_MASK) == PH_ERR_PN5190_RX_TIMEOUT_ERROR) ||
        ((status & PH_ERR_MASK ) == PH_ERR_PN5190_TX_FAILURE))
    {
        /* Map equivalent PN5190 error codes to HAL Generic error code. */
        return PH_ADD_COMPCODE_FIXED(PH_ERR_IO_TIMEOUT, PH_COMP_HAL);
    }

    return status;
}

phStatus_t phhalHw_Pn5190_Transmit(
                                   phhalHw_Pn5190_DataParams_t * pDataParams,
                                   uint16_t wOption,
                                   uint8_t * pTxBuffer,
                                   uint16_t wTxLength
                                   )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM statusTmp;
    uint8_t     PH_MEMLOC_REM bNumberOfValidBits = 0U;
    uint8_t     PH_MEMLOC_REM bSyncByte = 0xf0;

    if (0U != (wTxLength))
    {
        if ((PH_TRANSMIT_PREV_FRAME & wOption) == 0U)
        {
            PH_ASSERT_NULL(pTxBuffer);
        }
    }

    /* Check options */
    if (wOption & (uint16_t)~(uint16_t)(PH_TRANSMIT_BUFFERED_BIT | PH_TRANSMIT_LEAVE_BUFFER_BIT | PH_TRANSMIT_PREV_FRAME))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Re-send option should not be combined with buffer options. */
    if ((wOption & PH_TRANSMIT_PREV_FRAME) && (wOption & (uint16_t)~(uint16_t)(PH_TRANSMIT_PREV_FRAME)))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* In Target Mode it's not allowed to have same buffers for transmission and reception
    *  since the data to transmit would get overwritten by received data. */
    if ((pDataParams->wTargetMode != PH_OFF) &&
        (pDataParams->pTxBuffer == pDataParams->pRxBuffer))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_HAL);
    }

    if ((PH_TRANSMIT_PREV_FRAME & wOption) == 0U)
    {
        bNumberOfValidBits = (uint8_t)pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXLASTBITS];
    }
    else
    {
        if(wTxLength == 0x00U)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
        }
        pDataParams->wTxBufLen = wTxLength;

        if ((pDataParams->bActiveMode != PH_ON) && (pDataParams->bNfcipMode == PH_ON))
        {
            pDataParams->wTxBufLen += 1;
        }
    }

    if ((pDataParams->bActiveMode != PH_ON) && (pDataParams->bNfcipMode == PH_ON))
    {
        if((wOption == PH_TRANSMIT_BUFFER_FIRST) || (wOption == PH_TRANSMIT_DEFAULT))
        {
            PH_CHECK_SUCCESS_FCT(status, phhalHw_Pn5190_Instr_TransmitRFData(
                pDataParams,
                PH_TRANSMIT_BUFFER_FIRST,
                bNumberOfValidBits,
                &bSyncByte,
                1));

            if(wOption == PH_TRANSMIT_BUFFER_FIRST)
            {
                wOption = PH_TRANSMIT_BUFFER_CONT;
            }
            else
            {
                wOption = PH_TRANSMIT_BUFFER_LAST;
            }
        }
    }

    status =  phhalHw_Pn5190_Instr_TransmitRFData(pDataParams, wOption, bNumberOfValidBits, pTxBuffer, wTxLength);
    PH_CHECK_ABORT(status);

    /* Reset TxLastBits */
    if (pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXLASTBITS] != 0)
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_SetConfig(pDataParams, PHHAL_HW_CONFIG_TXLASTBITS, 0x00));
    }

    if ((status & PH_ERR_MASK ) == PH_ERR_PN5190_TX_FAILURE)
    {
        /* Map equivalent PN5190 error codes to HAL Generic error code. */
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_HAL);
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Receive(
                                  phhalHw_Pn5190_DataParams_t * pDataParams,
                                  uint16_t wOption,
                                  uint8_t ** ppRxBuffer,
                                  uint16_t * pRxLength
                                  )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_UNSUPPORTED_COMMAND;
    uint8_t  PH_MEMLOC_REM  bRespControl = 0;
    uint8_t  PH_MEMLOC_REM  bRfTxConfig;
    uint8_t  PH_MEMLOC_REM  bRfRxConfig;

    /* Validate input parameters. */
    if ((wOption != 0U) || (ppRxBuffer == NULL) || (pRxLength == NULL))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    if (pDataParams->bCmdAborted == PH_ON)
    {
        pDataParams->bCmdAborted = PH_OFF;

        switch (pDataParams->wTargetMode & PHHAL_HW_RF_FRAMING_OPTION_MASK)
        {
        case PHHAL_HW_RF_TYPE_A_FRAMING:
            /* Update with Passive Type A Card mode settings. */
            bRfTxConfig = PHHAL_HW_PN5190_RF_TX_ISO14443A_PICC_106_MANCH_SUBC;
            bRfRxConfig = PHHAL_HW_PN5190_RF_RX_ISO14443A_PICC_106_MILLER;
            break;
        case PHHAL_HW_RF_TYPE_F_FRAMING:
            /* Update with Passive Type F Card mode settings. */
            if ((pDataParams->wTargetMode & PHHAL_HW_RF_DATARATE_OPTION_MASK) == PHHAL_HW_RF_DATARATE_212)
            {
                bRfTxConfig = PHHAL_HW_PN5190_RF_TX_NFC_PT_212;
                bRfRxConfig = PHHAL_HW_PN5190_RF_RX_NFC_PT_212;
            }
            else
            {
                bRfTxConfig = PHHAL_HW_PN5190_RF_TX_NFC_PT_424;
                bRfRxConfig = PHHAL_HW_PN5190_RF_RX_NFC_PT_424;
            }
            break;
        default :
            return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_HAL);
        }

        /* Load the required Card mode settings. */
        status = phhalHw_Pn5190_Instr_LoadRfConfiguration(pDataParams, bRfTxConfig, bRfRxConfig);
        if (status != PH_ERR_SUCCESS)
        {
            return PH_ADD_COMPCODE(status, PH_COMP_HAL);
        }
    }

    bRespControl = PH_RECEIVE_RF_ENABLE_EVT_STATUS |
            PH_RECEIVE_RF_ENABLE_RF_STATUS | PH_RECEIVE_RF_ENABLE_RX_STATUS;

    /* Wait for the Data to be received */
    status = phhalHw_Pn5190_Instr_ReceiveRFData(pDataParams, bRespControl, ppRxBuffer, pRxLength);

    if (status == PH_ERR_SUCCESS)
    {
        status = phhalHw_Pn5190_Int_Retrieve_Data(pDataParams, ppRxBuffer, pRxLength);
    }
    else if ((status & PH_ERR_MASK) == PH_ERR_PN5190_RX_TIMEOUT_ERROR)
    {
        /* Map equivalent PN5190 error codes to HAL Generic error code. */
        return PH_ADD_COMPCODE_FIXED(PH_ERR_IO_TIMEOUT, PH_COMP_HAL);
    }
    else
    {
        /* Do nothing */
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Autocoll(
                                   phhalHw_Pn5190_DataParams_t * pDataParams,
                                   uint16_t wMode,
                                   uint8_t ** ppRxBuffer,
                                   uint16_t * pRxLength,
                                   uint16_t * pProtParams
                                   )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint32_t   PH_MEMLOC_REM dwRegister = 0;
    uint8_t     PH_MEMLOC_REM offsetPos = 0U;
    uint8_t     PH_MEMLOC_REM offsetLen = 0U;

    /* Check all the pointers */
    PH_ASSERT_NULL(ppRxBuffer);
    PH_ASSERT_NULL(pRxLength);
    PH_ASSERT_NULL(pProtParams);

    /* Disable the EMD. */
    PH_CHECK_SUCCESS_FCT(status, phhalHw_Pn5190_SetConfig(pDataParams, PHHAL_HW_CONFIG_SET_EMD, PH_OFF));

    uint8_t * PH_MEMLOC_REM pEvtPayload = NULL;
    uint32_t PH_MEMLOC_REM dwEventReceived = 0x0U;
    uint8_t  PH_MEMLOC_REM bRfTechnology = 0U;

    uint8_t     PH_MEMLOC_REM bTypeA_CRC = 0;
    uint8_t     PH_MEMLOC_REM bTypeF_CRC = 0;
    uint16_t    PH_MEMLOC_REM wCrcIn = 0;
    uint16_t    PH_MEMLOC_REM wCrcCalc = 0;

    if(pDataParams->bCardMode == PH_ON)
    {
        bRfTechnology = PHHAL_HW_PN5190_AUTOCOLL_RF_TECHNOLOGY_MASK_NFC_A;
    }
    else
    {
        /*RFTech A*/
        if((wMode & A_MODE) == A_MODE)
        {
            bRfTechnology |= PHHAL_HW_PN5190_AUTOCOLL_RF_TECHNOLOGY_MASK_NFC_A;
        }
        /*RFTech F*/
        if((wMode & F_MODE) == F_MODE)
        {
            bRfTechnology |= PHHAL_HW_PN5190_AUTOCOLL_RF_TECHNOLOGY_MASK_NFC_F;
        }
        /*RFTech Active A*/
        if((wMode & ACTIVE_A_MODE) == ACTIVE_A_MODE)
        {
            bRfTechnology |= PHHAL_HW_PN5190_AUTOCOLL_RF_TECHNOLOGY_MASK_NFC_A_ACTIVE;
        }
        /*RFTech Active F*/
        if((wMode & ACTIVE_F_MODE) == ACTIVE_F_MODE)
        {
            bRfTechnology |= PHHAL_HW_PN5190_AUTOCOLL_RF_TECHNOLOGY_MASK_NFC_F_ACTIVE;
        }
        /*RFTech B and Active B*/
        if(((wMode & B_MODE) == B_MODE)||((wMode & ACTIVE_B_MODE) == ACTIVE_B_MODE))
        {
            /* Do nothing, ignore -- NFCP576 -- as the NFC forum DTA application expects success for Listen B */
            return PH_ERR_SUCCESS;
        }
    }
    /* set the receive length */
    *pRxLength = 0U;

    pDataParams->wRxBufLen = 0U;

    /* Reset the Flag used to indicate if Passive Target is activated by ISO/IEC 18092 Frame */
    pDataParams->wTargetMode = PH_OFF;
    pDataParams->bActiveMode = PH_OFF;
    pDataParams->bNfcipMode  = PH_OFF;
    /* Reset bCmdAborted flag */
    pDataParams->bCmdAborted = PH_OFF;

    PH_CHECK_ABORT_FCT(status, phhalHw_Pn5190_Instr_SwitchModeAutocoll(pDataParams,bRfTechnology,PHHAL_HW_PN5190_AUTOCOLL_MODE_TERMINATE_ON_RF_OFF));

    if(status == PH_ERR_SUCCESS)
    {
        /*Wait for event*/
        status = phhalHw_Pn5190_WaitForEvent(
                     pDataParams,
                     PH_PN5190_EVT_AUTOCOLL | PH_PN5190_EVT_GENERAL_ERROR,
                     PHOSAL_MAX_DELAY,
                     true,
                     &dwEventReceived,
                     &pEvtPayload);

        if (status == PH_ERR_SUCCESS)
        {
            if(dwEventReceived == PH_PN5190_EVT_AUTOCOLL)
            {
                if(pEvtPayload[PHHAL_HW_PN5190_AUTOCOLL_STATUS_INDEX] == PHHAL_HW_PN5190_AUTOCOLL_ACTIVATED)
                {
                    PH_CHECK_SUCCESS_FCT(status,  phhalHw_Pn5190_Instr_ReadRegister(pDataParams, CLIF_RX_STATUS_ERROR, &dwRegister));

                    if ((dwRegister & CLIF_RX_STATUS_ERROR_RX_CL_ERROR_MASK) == 0U)
                    {
                        switch(pEvtPayload[PHHAL_HW_PN5190_AUTOCOLL_TECH_ACT_INDEX])
                        {
                        case PHHAL_HW_PN5190_AUTOCOLL_TECH_PASSIVE_A_106:
                            *pProtParams = PHHAL_HW_RF_TYPE_A_FRAMING | PHHAL_HW_RF_DATARATE_106;
                            break;
                        case PHHAL_HW_PN5190_AUTOCOLL_TECH_PASSIVE_F_212:
                            *pProtParams = PHHAL_HW_RF_TYPE_F_FRAMING | PHHAL_HW_RF_DATARATE_212;
                            break;
                        case PHHAL_HW_PN5190_AUTOCOLL_TECH_PASSIVE_F_424:
                            *pProtParams = PHHAL_HW_RF_TYPE_F_FRAMING | PHHAL_HW_RF_DATARATE_424;
                            break;
                        case PHHAL_HW_PN5190_AUTOCOLL_TECH_ACTIVE_106:
                            *pProtParams = PHHAL_HW_RF_TYPE_ACTIVE_FRAMING | PHHAL_HW_RF_DATARATE_106;
                            bTypeA_CRC = 1;
                            break;
                        case PHHAL_HW_PN5190_AUTOCOLL_TECH_ACTIVE_212:
                            *pProtParams = PHHAL_HW_RF_TYPE_ACTIVE_FRAMING | PHHAL_HW_RF_DATARATE_212;
                            bTypeF_CRC = 1;
                            break;
                        case PHHAL_HW_PN5190_AUTOCOLL_TECH_ACTIVE_424:
                            *pProtParams = PHHAL_HW_RF_TYPE_ACTIVE_FRAMING | PHHAL_HW_RF_DATARATE_424;
                            bTypeF_CRC = 1;
                            break;
                        default:
                            /* Invalid Framing as PN5190 will not emulate other than Type-A, Type-F and Active Frame */
                            return PH_ADD_COMPCODE_FIXED(PH_ERR_FRAMING_ERROR, PH_COMP_HAL);
                        }

                        if ((pEvtPayload[PHHAL_HW_PN5190_AUTOCOLL_TECH_ACT_INDEX]
                            & PHHAL_HW_PN5190_AUTOCOLL_ACTIVATION_MODE_MASK) == PHHAL_HW_PN5190_AUTOCOLL_ACTIVATION_PASSIVE_MODE)

                        /* Disable active mode */
                        {
                            if (pEvtPayload[PHHAL_HW_PN5190_AUTOCOLL_TECH_STATUS_INDEX] == 0U)
                            {
                                return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_HAL);
                            }
                        }

                        /* Read the RF Rx Buffer contents. */
                        status = phhalHw_Pn5190_Instr_RetrieveRFData(
                                pDataParams,
                                ppRxBuffer,
                                pRxLength);

                        /* Disable active mode */
                        if(((*pProtParams & PHHAL_HW_RF_DATARATE_OPTION_MASK) == PHHAL_HW_RF_DATARATE_106) && (**ppRxBuffer == PHHAL_HW_PN5190_I18092MT_START_BYTE))
                        {
                            /* Offset for StartByte as it included if Passive Communication in ISO/IEC 18092 */
                            offsetPos = 1U;
                            offsetLen = 1U;
                        }

                        if((*pProtParams & PHHAL_HW_RF_FRAMING_OPTION_MASK) == PHHAL_HW_RF_TYPE_ACTIVE_FRAMING)
                        {
                            pDataParams->bActiveMode = PH_ON;
                        }

                        /* At-least a byte data + 2 bytes of crc.  */
                        if(*pRxLength >= 3)
                        {
                            if ((bTypeA_CRC == 0x1U) || (bTypeF_CRC == 0x1U))
                            {
                                if (bTypeA_CRC == 0x1U)
                                {
                                    /* Retrieve CRC */
                                    wCrcIn  = (uint16_t)(((uint16_t)(*ppRxBuffer)[(*pRxLength) - 1U]) << 8U);
                                    wCrcIn |= (uint16_t)((*ppRxBuffer)[(*pRxLength) - 2U]);

                                    /* Remove CRC from input data */
                                    *pRxLength -= 2U;

                                    /* Calculate CRC */
                                    PH_CHECK_SUCCESS_FCT(status, phTools_CalculateCrc16(
                                        PH_TOOLS_CRC_OPTION_DEFAULT,
                                        PH_TOOLS_CRC16_PRESET_ISO14443A,
                                        PH_TOOLS_CRC16_POLY_ISO14443,
                                        *ppRxBuffer,
                                        *pRxLength,
                                        &wCrcCalc));
                                }
                                else if (bTypeF_CRC == 0x1U)
                                {
                                    /* Retrieve CRC */
                                    wCrcIn = (uint16_t)((*ppRxBuffer)[(*pRxLength) - 1U]);
                                    wCrcIn |= (uint16_t)(((uint16_t)(*ppRxBuffer)[(*pRxLength) - 2U]) << 8U);

                                    /* Remove CRC from input data */
                                    *pRxLength -= 2U;

                                    /* Calculate CRC */
                                    PH_CHECK_SUCCESS_FCT(status, phTools_CalculateCrc16(
                                        PH_TOOLS_CRC_OPTION_MSB_FIRST,
                                        PH_TOOLS_CRC16_PRESET_FELICA,
                                        PH_TOOLS_CRC16_POLY_FELICA,
                                        *ppRxBuffer,
                                        *pRxLength,
                                        &wCrcCalc));
                                }
                                /* CRC Check -> Compare input and calculated crc */
                                if (wCrcIn == wCrcCalc)
                                {
                                    status = PH_ERR_SUCCESS;
                                }
                                else
                                {
                                    return PH_ADD_COMPCODE_FIXED(PH_ERR_INTEGRITY_ERROR, PH_COMP_HAL);
                                }

                            }
                            else
                            {
                                /* Remove CRC from input data */
                                *pRxLength -=2;
                            }

                            /* Store received data length in dataparams */
                            pDataParams->wRxBufLen = *pRxLength;

                            /* Return RxBuffer pointer */
                            /* Offset "F0" incase of Active communication */
                            *ppRxBuffer = pDataParams->pRxBuffer + offsetPos;

                            /* Return RxBuffer length */
                            /* Offset "F0" + CRC incase of Active communication */
                            *pRxLength = pDataParams->wRxBufLen - offsetLen;

                            pDataParams->wTargetMode = *pProtParams;

                            pDataParams->wCfgShadow[PHHAL_HW_CONFIG_RXDATARATE_FRAMING] = *pProtParams;
                            pDataParams->wCfgShadow[PHHAL_HW_CONFIG_TXDATARATE_FRAMING] = *pProtParams;

                        }
                        else
                        {
                            return PH_ADD_COMPCODE_FIXED(PH_ERR_INTEGRITY_ERROR, PH_COMP_HAL);
                        }
                    }
                    else
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTEGRITY_ERROR, PH_COMP_HAL);
                    }
                }
                else if(pEvtPayload[PHHAL_HW_PN5190_AUTOCOLL_STATUS_INDEX] == PHHAL_HW_PN5190_AUTOCOLL_EXTRF_OFF)
                {
                    status = PH_ERR_EXT_RF_ERROR;
                }
                else
                {
                    /* Unexpected status return from FE */
                    status = PH_ERR_INTERNAL_ERROR;
                }
            }
            if(dwEventReceived & PH_PN5190_EVT_GENERAL_ERROR)
            {
                pDataParams->dwGenError = (uint32_t) pEvtPayload[4];
                pDataParams->dwGenError |= ((uint32_t) pEvtPayload[5]) << 8U ;
                pDataParams->dwGenError |= ((uint32_t) pEvtPayload[6]) << 16U;
                pDataParams->dwGenError |= ((uint32_t) pEvtPayload[7]) << 24U;

                status = PH_ERR_INTERNAL_ERROR;
            }
        }
    }
    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Wait(
                               phhalHw_Pn5190_DataParams_t * pDataParams,
                               uint8_t bUnit,
                               uint16_t wTimeout
                               )
{
    phStatus_t PH_MEMLOC_REM wStatus;
    phStatus_t PH_MEMLOC_REM statusTmp;
    uint32_t    PH_MEMLOC_REM dwLoadValue;
    uint32_t    PH_MEMLOC_REM wPrescaler;
    uint32_t    PH_MEMLOC_REM wFreq;

    /* Parameter check */
    if ((bUnit != PHHAL_HW_TIME_MICROSECONDS) && (bUnit != PHHAL_HW_TIME_MILLISECONDS))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* If time-out value is zero, there is no wait needed */
    if (wTimeout == 0U)
    {
        return PH_ERR_SUCCESS;
    }

    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegister(
        pDataParams,
        EVENT_ENABLE,
        EVENT_ENABLE_TIMER0_EVENT_ENABLE_MASK));

    do
    {
        /* Setting the Prescaler frequency according to wTimeout */
        if (bUnit == PHHAL_HW_TIME_MILLISECONDS)
        {
            wFreq = PHHAL_HW_PN5190_MIN_FREQ;
            wPrescaler = 0x3CU;
            while(wTimeout > PHHAL_HW_PN5190_MAX_TIME_DELAY_MS)
            {
                wTimeout -= PHHAL_HW_PN5190_MAX_TIME_DELAY_MS;
                /* Calculating the load value */
                dwLoadValue =(uint32_t) (  PHHAL_HW_PN5190_MAX_TIME_DELAY_MS * ( wFreq  / PHHAL_HW_PN5190_CONVERSION_MS_SEC) );
                PH_CHECK_ABORT_FCT(wStatus, phhalHw_Pn5190_Int_TimerStart(
                    pDataParams,
                    CLIF_TIMER0_CONFIG,
                    CLIF_TIMER0_CONFIG_T0_START_NOW_MASK,
                    CLIF_TIMER0_CONFIG_T0_ONE_SHOT_MODE_MASK,
                    wPrescaler,
                    dwLoadValue));
            }

            dwLoadValue =(uint32_t) (  wTimeout * ( wFreq  / PHHAL_HW_PN5190_CONVERSION_MS_SEC) );
        }
        else
        {
            /* here wTimeout will be in uS and not be Zero */
            wFreq = PHHAL_HW_PN5190_MAX_FREQ;
            wPrescaler = 0x00U;
            /*Reducing the division by 2 digits to retain the 2 digit decimal places which were getting wiped out*/
            dwLoadValue =(uint32_t) ( wFreq / (PHHAL_HW_PN5190_CONVERSION_US_SEC/100));
            /*Restoring the division done in the earlier step*/
            dwLoadValue =(uint32_t) ((wTimeout * dwLoadValue)/100);
            wTimeout = (wTimeout/1000) + 1;
        }
        PH_CHECK_ABORT_FCT(wStatus, phhalHw_Pn5190_Int_TimerStart(
            pDataParams,
            CLIF_TIMER0_CONFIG,
            CLIF_TIMER0_CONFIG_T0_START_NOW_MASK,
            CLIF_TIMER0_CONFIG_T0_ONE_SHOT_MODE_MASK,
            wPrescaler,
            dwLoadValue));
    }while(0);

    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(
        pDataParams,
        EVENT_ENABLE,
        ~(uint32_t) EVENT_ENABLE_TIMER0_EVENT_ENABLE_MASK));

    return PH_ADD_COMPCODE(wStatus, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_Lpcd(
                               phhalHw_Pn5190_DataParams_t * pDataParams
                               )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_USE_CONDITION;
    uint32_t PH_MEMLOC_REM dwEventReceived = 0x0U;
    uint32_t PH_MEMLOC_REM dwEventToWait = 0x00U;

    if((pDataParams->bLpcdMode == PHHAL_HW_PN5190_LPCD_MODE_DEFAULT) ||
       (pDataParams->bLpcdMode == PHHAL_HW_PN5190_LPCD_MODE_POWERDOWN))
    {
        if (pDataParams->bLpcdMode == PHHAL_HW_PN5190_LPCD_MODE_DEFAULT)
        {
            if (pDataParams->bLpcdConfig == PHHAL_HW_PN5190_LPCD_CTRL_SINGLEMODE_LPCD)
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_HAL);
            }

            /* Reset the reference value. */
            pDataParams->dwLpcdRefVal = 0U;
        }

        if ((pDataParams->bLpcdConfig == PHHAL_HW_PN5190_LPCD_CTRL_ULPCD_CALIB) ||
            (pDataParams->bLpcdConfig == PHHAL_HW_PN5190_LPCD_CTRL_ULPCD))
        {
            /* Use the HF ATT reference value provided by User. */
            pDataParams->dwLpcdRefVal |= (uint32_t)(pDataParams->bHFATTVal << 16U);
        }

        /* Call LPCD Command to exchange LPCD TLV. */
        status = phhalHw_Pn5190_Instr_SwitchModeLpcd(pDataParams, pDataParams->bLpcdConfig, pDataParams->bLpcdWakeUpCtrl,
                pDataParams->dwLpcdRefVal, pDataParams->wLPCDWakeupCounterInMs);

        if (pDataParams->bLpcdMode == PHHAL_HW_PN5190_LPCD_MODE_DEFAULT )
        {
            dwEventToWait = PH_PN5190_EVT_LP_CALIB | PH_PN5190_EVT_GENERAL_ERROR;
        }
        else
        {
            dwEventToWait = PH_PN5190_EVT_LPCD | PH_PN5190_EVT_GENERAL_ERROR;
        }


        if(status == PH_ERR_SUCCESS)
        {
            /* Once LPCD Command exchange is successful, wait for LPCD Event from PN5190. */
            status = phhalHw_Pn5190_Int_EventWait(
                    pDataParams,
                    dwEventToWait,
                    PHOSAL_MAX_DELAY,
                    true,
                    &dwEventReceived);

            if(status == PH_ERR_SUCCESS)
            {
                /* Check for LPCD Event and expectation is to receive only LPCD event. */
                if (dwEventReceived == PH_PN5190_EVT_LPCD)
                {
                    if (pDataParams->dwEventStatus == PHHAL_HW_LPCD_EVT_STATUS_CARD_DETECTED)
                    {
                        /* Received LPCD event with card detection event bit set. */
                        status = PH_ERR_SUCCESS;
                    }
                    else
                    {
                        /* Received LPCD event and but other than card detection event bit set. */
                        status = PH_ERR_LPCD_ABORTED;
                    }
                }
                else if(dwEventReceived == PH_PN5190_EVT_LP_CALIB)
                {
                    /* Store the LPCD Calibration data, which should be used during card detection phase. */
                    pDataParams->dwLpcdRefVal = pDataParams->dwEventStatus;
                    status = PH_ERR_SUCCESS;
                }
                else
                {
                    status = PH_ERR_INTERNAL_ERROR;
                }
            }
        }
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_MfcAuthenticateKeyNo(
                                               phhalHw_Pn5190_DataParams_t * pDataParams,
                                               uint8_t bBlockNo,
                                               uint8_t bKeyType,
                                               uint16_t wKeyNo,
                                               uint16_t wKeyVersion,
                                               uint8_t * pUid
                                               )
{
#if defined(NXPBUILD__PH_KEYSTORE)
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_BUF aKey[PHHAL_HW_MFC_KEY_LENGTH * 2];
    uint8_t *  PH_MEMLOC_REM pKey;
    uint16_t   PH_MEMLOC_REM bKeystoreKeyType = 0U;

    /* Bail out if we haven't got a key-store */
    if (pDataParams->pKeyStoreDataParams == NULL)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_HAL);
    }

    /* Retrieve KeyA & KeyB from key-store */
    status = phKeyStore_GetKey(
        pDataParams->pKeyStoreDataParams,
        wKeyNo,
        wKeyVersion,
        sizeof(aKey),
        aKey,
        &bKeystoreKeyType);

    /* Change component code for invalid parameter */
    if ((status & PH_ERR_MASK) == PH_ERR_INVALID_PARAMETER)
    {
        return PH_ADD_COMPCODE((status & PH_ERR_MASK), PH_COMP_HAL);
    }
    else
    {
        PH_CHECK_SUCCESS(status);
    }

    /* Check key type */
    if (bKeystoreKeyType != PH_KEYSTORE_KEY_TYPE_MIFARE)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Evaluate which key to use */
    if (bKeyType == PHHAL_HW_MFC_KEYA)
    {
        /* Use KeyA */
        pKey = aKey;
    }
    else if (bKeyType == PHHAL_HW_MFC_KEYB)
    {
        /* Use KeyB */
        pKey = &aKey[PHHAL_HW_MFC_KEY_LENGTH];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_HAL);
    }

    /* Call the authentication function using the key */
    return phhalHw_Pn5190_MfcAuthenticate(
        pDataParams,
        bBlockNo,
        bKeyType,
        pKey,
        pUid);
#else /* NXPBUILD__PH_KEYSTORE */
    PH_UNUSED_VARIABLE(pDataParams);
    PH_UNUSED_VARIABLE(bBlockNo);
    PH_UNUSED_VARIABLE(bKeyType);
    PH_UNUSED_VARIABLE(wKeyNo);
    PH_UNUSED_VARIABLE(wKeyVersion);
    PH_UNUSED_VARIABLE(pUid);
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_HAL);
#endif /* NXPBUILD__PH_KEYSTORE */
}

phStatus_t phhalHw_Pn5190_MfcAuthenticate(
                                          phhalHw_Pn5190_DataParams_t * pDataParams,
                                          uint8_t bBlockNo,
                                          uint8_t bKeyType,
                                          uint8_t * pKey,
                                          uint8_t * pUid
                                          )
{
    return phhalHw_Pn5190_Instr_MifareAuthenticate(
        pDataParams,
        pKey,
        bKeyType,
        bBlockNo,
        pUid);
}

phStatus_t phhalHw_Pn5190_I18000p3m3Inventory(
                                              phhalHw_Pn5190_DataParams_t * pDataParams,
                                              uint8_t * pSelCmd,
                                              uint8_t bSelCmdLen,
                                              uint8_t bNumValidBitsinLastByte,
                                              uint8_t * pBeginRndCmd,
                                              uint8_t bTSprocessing,
                                              uint8_t ** ppRxBuffer,
                                              uint16_t * wRxBufferLen
                                              )
{
    phStatus_t  PH_MEMLOC_REM statusTmp = PH_ERR_SUCCESS;
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint16_t    PH_MEMLOC_REM wResultSize;
    uint8_t     PH_MEMLOC_REM bTarget = 0U;
    uint8_t     PH_MEMLOC_REM bSession = 0U;
    phOsal_EventBits_t PH_MEMLOC_REM dwEventFlags = 0;
    uint8_t     PH_MEMLOC_COUNT bQ = 1;
    uint8_t     PH_MEMLOC_REM bBuff1, bBuff2;
    uint16_t    PH_MEMLOC_REM bIndex = 0;
    uint16_t    PH_MEMLOC_REM wNumOfSlots = 0;

    /* Check all the pointers */
    PH_ASSERT_NULL(pSelCmd);
    PH_ASSERT_NULL(pBeginRndCmd);
    PH_ASSERT_NULL(ppRxBuffer);
    PH_ASSERT_NULL(wRxBufferLen);

    /* Reset the Rx Buffer Start Position. */
    pDataParams->wRxBufStartPos = 0;
    pDataParams->wCurrentSlotCount = 0;

    if(bSelCmdLen > 0U)
    {
        bTarget = pSelCmd[0];
        bTarget = bTarget>> 1U;
        bTarget = bTarget&0x07U;

        bSession = pBeginRndCmd[1]>> 4U;
        bSession &= 0x03U;

        switch(bSession)
        {
        case 0:
        case 2:
        {
            if(bSession != bTarget)
            {
                statusTmp = (PH_ERR_INVALID_PARAMETER | PH_COMP_HAL);
            }
            break;
        }
        default:
            statusTmp = (PH_ERR_INVALID_PARAMETER | PH_COMP_HAL);
        }

    }

    if (pDataParams->bPollGuardTimeFlag == PH_ON)
    {
        pDataParams->bPollGuardTimeFlag = PH_OFF;

        /* When there is no errors in preconditions wait till timer expire. */
        if (statusTmp == PH_ERR_SUCCESS)
        {
            /* Wait for the Poll Guard Time to Expire. */
            (void)phOsal_EventPend(&xEventHandle, (phOsal_EventOpt_t)(E_OS_EVENT_OPT_PEND_SET_ANY | E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT),
                    PHOSAL_MAX_DELAY, E_PH_OSAL_EVT_GT_EXP | E_PH_OSAL_EVT_ABORT, &dwEventFlags);
            if(0U != (dwEventFlags & E_PH_OSAL_EVT_ABORT))
            {
                statusTmp = PH_ERR_ABORTED;
            }
        }

        /* Stop Timer */
        PH_CHECK_SUCCESS_FCT(status, phDriver_TimerStop());
    }

    if (statusTmp != PH_ERR_SUCCESS)
    {
        return statusTmp;
    }

    if (bTSprocessing == PHHAL_HW_I18000P3M3_GET_MAX_RESPS)
    {
        bSelCmdLen = 0U;
    }

    status = phhalHw_Pn5190_Instr_EpcGen2Inventory(
            pDataParams,
            pSelCmd,
            bSelCmdLen,
            bNumValidBitsinLastByte,
            pBeginRndCmd,
            bTSprocessing,
            ppRxBuffer,
            &wResultSize
            );
    PH_CHECK_ABORT(status);

    if (status == PH_ERR_SUCCESS)
    {
        /*Retrieve the result size and result*/
        *wRxBufferLen = wResultSize;

        if (wResultSize > 0U)
        {
            /* Response length is 3bytes by default. With 1st byte indicating Response status.
             * If 1st Byte has value '2' it indicates there is no response in any Timeslot,
             * all other values indicate response availability.(As specified in PN5190 Instruction Layer document) */

            /* Extract bQ value from pBeginRndCmd */
            bBuff1 = *(pBeginRndCmd + 1) & (uint8_t)0x7;
            bBuff2 = *(pBeginRndCmd + 2) & (uint8_t)0x80;

            bQ = (bBuff1 << 1) | (bBuff2 >> 7);

            wNumOfSlots = 1 << bQ;

            while (0U != wNumOfSlots)
            {
                /* Do not proceed if available buffer is not enough to accomodate further exchanges */
                if (bIndex >= wResultSize)
                {
                    /* Backup the relevent data for Resume Inventory */
                    pDataParams->wCurrentSlotCount = wNumOfSlots;

                    status = PH_ERR_SUCCESS_CHAINING;
                    break;
                }
                else
                {
                    if ((*ppRxBuffer)[bIndex] == 0U)
                    {
                        bIndex += (*ppRxBuffer)[bIndex + 1];
                    }
                    else if (((*ppRxBuffer)[bIndex] == 2U)
                        && ((*ppRxBuffer)[bIndex + 1] == 0U)
                        && ((*ppRxBuffer)[bIndex + 2] == 0U))
                    {
                        status = PH_ERR_IO_TIMEOUT;
                    }
                    else if (((*ppRxBuffer)[bIndex] == 3U)
                        && ((*ppRxBuffer)[bIndex + 1] == 0U)
                        && ((*ppRxBuffer)[bIndex + 2] == 0U))
                    {
                        status = PH_ERR_COLLISION_ERROR;

                        if((bTSprocessing & PHHAL_HW_I18000P3M3_ONE_TS_ONLY) ||
                            (bTSprocessing & PHHAL_HW_I18000P3M3_GET_TAG_HANDLE))
                        {
                            break;
                        }
                    }
                    else if ((*ppRxBuffer)[bIndex] == 1U)
                    {
                        bIndex += (*ppRxBuffer)[bIndex + 1];
                    }
                    else
                    {
                        status = PH_ERR_IO_TIMEOUT;
                    }

                    bIndex += 3U;
                    wNumOfSlots--;
                }
            }
        }
        else
        {
            status = PH_ERR_INTERNAL_ERROR;
        }
    }
    else
    {
        status = PH_ERR_IO_TIMEOUT;
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_18000p3m3ResumeInventory(
                                                phhalHw_Pn5190_DataParams_t * pDataParams,  /**< [In] Pointer to this layer's parameter structure. */
                                                uint8_t ** ppRxBuffer,                      /**< [In] Pointer to HAL Rx Buffer containing response from single/multiple tags. */
                                                uint16_t * wRxBufferLen                     /**< [In] Length of response in hal Rx Buffer. */
                                                   )
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_IO_TIMEOUT;
    uint16_t    PH_MEMLOC_REM wResultSize;
    uint16_t    PH_MEMLOC_REM bIndex = 0;
    uint16_t    PH_MEMLOC_REM wNumOfSlots = 0;

    /* Check all the pointers */
    PH_ASSERT_NULL(ppRxBuffer);
    PH_ASSERT_NULL(wRxBufferLen);

    status = phhalHw_Pn5190_Instr_EpcGen2ResumeInventory(
            pDataParams,
            ppRxBuffer,
            &wResultSize);
    PH_CHECK_ABORT(status);

    if (status == PH_ERR_SUCCESS)
    {
        /*Retrieve the result size and result*/
        *wRxBufferLen = wResultSize;

        if (wResultSize > 0U)
        {
            /* Response length is 3bytes by default. With 1st byte indicating Response status.
             * If 1st Byte has value '2' it indicates there is no response in any Timeslot,
             * all other values indicate response availability.(As specified in PN5190 Instruction Layer document) */
            wNumOfSlots = pDataParams->wCurrentSlotCount;
            pDataParams->wCurrentSlotCount = 0;

            while (0U != wNumOfSlots)
            {
                /* Do not proceed if available buffer is not enough to accomodate further exchanges */
                if (bIndex >= wResultSize)
                {
                    /* Backup the relevent data for Resume Inventory */
                    pDataParams->wCurrentSlotCount = wNumOfSlots;

                    status = PH_ERR_SUCCESS_CHAINING;
                    break;
                }
                else
                {
                    if ((*ppRxBuffer)[bIndex] == 0U)
                    {
                        bIndex += (*ppRxBuffer)[bIndex + 1];
                    }
                    else if (((*ppRxBuffer)[bIndex] == 2)
                        && ((*ppRxBuffer)[bIndex + 1] == 0)
                        && ((*ppRxBuffer)[bIndex + 2] == 0))
                    {
                        status = PH_ERR_IO_TIMEOUT;
                    }
                    else if (((*ppRxBuffer)[bIndex] == 3)
                        && ((*ppRxBuffer)[bIndex + 1] == 0)
                        && ((*ppRxBuffer)[bIndex + 2] == 0))
                    {
                        status = PH_ERR_COLLISION_ERROR;
                    }
                    else
                    {
                        status = PH_ERR_IO_TIMEOUT;
                    }

                    bIndex += 3U;
                    wNumOfSlots--;
                }
            }
        }
        else
        {
            status = PH_ERR_INTERNAL_ERROR;
        }
    }
    else
    {
        status = PH_ERR_IO_TIMEOUT;
    }

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_EventWait(
                                    phhalHw_Pn5190_DataParams_t * pDataParams,
                                    uint32_t dwEventTimeout
                                    )
{
    phStatus_t PH_MEMLOC_REM status;
    uint32_t PH_MEMLOC_REM dwEventReceived = 0x0U;
    phStatus_t PH_MEMLOC_REM statusTmp;
    uint32_t PH_MEMLOC_REM dwTOLoopCnt = 0x0U;

    if (pDataParams->dwExpectedEvent == 0U)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_HAL);
    }

    pDataParams->dwEventReceived = 0U;

    if (pDataParams->dwExpectedEvent == PH_PN5190_EVT_RFON_DETECT)
    {
        if (pDataParams->bRFONEvent != 0x01U)
        {
            status = PH_ERR_IO_TIMEOUT;
            do
            {
                (void)phOsal_ThreadDelay(1);
                if (pDataParams->bRFONEvent == 0x01U)
                {
                    break;
                }
                dwTOLoopCnt++;
            }while(dwTOLoopCnt < dwEventTimeout);
        }

        if (pDataParams->bRFONEvent == 0x01U)
        {
            pDataParams->dwEventReceived = PH_PN5190_EVT_RFON_DETECT;
            pDataParams->bRFONEvent = 0x0U;
            status = PH_ERR_SUCCESS;
        }
    }
    else
    {
        /* Wait for event, configured by HOST using Setconfig of PHHAL_HW_PN5190_CONFIG_EXPECTED_EVENT. */
        status = phhalHw_Pn5190_Int_EventWait(
            pDataParams,
            pDataParams->dwExpectedEvent,
            dwEventTimeout,
            true,
            &dwEventReceived);

        pDataParams->dwEventReceived = dwEventReceived;
    }

    /* Only on successful External RF ON event, FW shall clear EVENT ENABLE Register.
     * But in case of Timeout, EVENT_ENABLE bit for RFON_DET_EVENT needs to be reset by HOST. */
    if ((pDataParams->dwExpectedEvent == PH_PN5190_EVT_RFON_DETECT) && ((status & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT))
    {
        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_Pn5190_Instr_WriteRegisterAndMask(pDataParams,
            EVENT_ENABLE, ~(uint32_t)EVENT_ENABLE_RFON_DET_EVENT_ENABLE_MASK));
    }

    if (((status & PH_ERR_MASK) != PH_ERR_IO_TIMEOUT) && ((status & PH_ERR_MASK) != PH_ERR_ABORTED))
    {
        if ((pDataParams->dwExpectedEvent & pDataParams->dwEventReceived) == 0U)
        {
            status = PH_EER_PN5190_UNEXPECTED_EVT;
        }
    }

    pDataParams->dwExpectedEvent = 0;

    return PH_ADD_COMPCODE(status, PH_COMP_HAL);
}

phStatus_t phhalHw_Pn5190_EventConsume(
                                       phhalHw_Pn5190_DataParams_t * pDataParams
                                       )
{
    phStatus_t PH_MEMLOC_REM status;
    status = phOsal_EventClear(
        &(pDataParams->HwEventObj.EventHandle),
        E_OS_EVENT_OPT_NONE,
        (E_PH_OSAL_EVT_RF | E_PH_OSAL_EVT_ABORT),
        NULL);
    return status;
}

#ifdef _WIN32
phStatus_t phhalHw_Pn5190_GetDataParamsSize(
                                            uint16_t * pDataParamsSize
                                            )
{
    if (pDataParamsSize == NULL)
    {
        return PH_ERR_INVALID_PARAMETER;
    }

    *pDataParamsSize = (uint16_t)sizeof(phhalHw_Pn5190_DataParams_t);

    return PH_ERR_SUCCESS;
}
#endif

#endif  /* NXPBUILD__PHHAL_HW_PN5190 */
