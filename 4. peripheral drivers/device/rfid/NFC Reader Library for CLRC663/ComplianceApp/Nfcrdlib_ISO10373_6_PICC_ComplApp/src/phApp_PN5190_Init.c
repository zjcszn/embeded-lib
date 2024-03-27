/*----------------------------------------------------------------------------*/
/* Copyright 2020,2022-2023 NXP                                               */
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
* Example Source abstracting component data structure and code initialization and code specific to HW used in the examples
* This file shall be present in all examples. A customer does not need to touch/modify this file. This file
* purely depends on the phNxpBuild_Lpc.h or phNxpBuild_App.h
* The phAppInit.h externs the component data structures initialized here that is in turn included by the core examples.
* The core example shall not use any other variable defined here except the RdLib component data structures(as explained above)
* The RdLib component initialization requires some user defined data and function pointers.
* These are defined in the respective examples and externed here.
*
* Keystore and Crypto initialization needs to be handled by application.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

/* Status header */
#include <ph_Status.h>

#ifdef NXPBUILD__PHHAL_HW_PN5190
#include "phApp_Init.h"

#include "BoardSelection.h"

/* HAL specific headers */
#include <phhalHw_Pn5190_Instr.h>

/*******************************************************************************
**   Function Declarations
*******************************************************************************/
void CLIF_IRQHandler(void);

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phbalReg_Type_t                 sBalParams;
phhalHw_Pn5190_DataParams_t   * pHal;

#ifdef NXPBUILD__PHHAL_HW_TARGET
/* Parameters for L3 activation during Autocoll */
extern uint8_t  sens_res[2U]   ;
extern uint8_t  nfc_id1[3U]    ;
extern uint8_t  sel_res        ;
extern uint8_t  nfc_id3        ;
extern uint8_t  poll_res[18U]  ;
#endif /* NXPBUILD__PHHAL_HW_TARGET */

#ifdef DEBUG_PRINT_RAW_DATA
#define MAX_RAW_DATA_LENGTH    (1100)
uint8_t g_Raw_Data[MAX_RAW_DATA_LENGTH];
uint32_t g_Raw_Data_Index = 0;
#endif /* DEBUG_PRINT_RAW_DATA */

#define PN5190_DIRECTION_BYTE_LEN      0x01U
#define PN5190_TYPE_FIELD_LEN          0x01U
#define PN5190_LENGTH_FIELD_LEN        0x02U
#define PN5190_STATUS_FIELD_LEN        0x01U

/*******************************************************************************
**   Function Definitions
*******************************************************************************/

/**
* This function will initialize Hal Target Config
*/
phStatus_t phApp_HALConfigAutoColl(void)
{
#ifdef NXPBUILD__PHHAL_HW_TARGET
    phStatus_t wStatus;
    uint8_t baDynamicUidConfig[1U] = { 1U }; /* Enable Dynamic UID feature */
    uint8_t baReadEepromConfig[24U] = { 0U };

    /* Read Set Listen Parameters data from EEPROM */
    wStatus = phhalHw_Pn5190_Instr_ReadE2Prom(
        pHal,
        PHHAL_HW_PN5190_SET_LISTEN_EEPROM_ADDR,
        baReadEepromConfig,
        24U
        );
    CHECK_SUCCESS(wStatus);

    /* Verify EEPROM data and configure Set Listen Parameters if EEPROM data is not correct. */
    if ((memcmp(&baReadEepromConfig[0U], sens_res, 2U) != 0x00)  ||
        (memcmp(&baReadEepromConfig[2U], nfc_id1, 3U) != 0x00)   ||
        (memcmp(&baReadEepromConfig[5U], &sel_res, 1U) != 0x00)  ||
        (memcmp(&baReadEepromConfig[6U], poll_res, 18U) != 0x00))
    {
        /* Configure Set Listen Parameters. */
        wStatus = phhalHw_Pn5190_SetListenParameters(
            pHal,
            &sens_res[0],
            &nfc_id1[0],
            sel_res,
            &poll_res[0],
            nfc_id3
            );
        CHECK_SUCCESS(wStatus);
    }

    /* Read Dynamic UID configuration from EEPROM */
    wStatus = phhalHw_Pn5190_Instr_ReadE2Prom(
        pHal,
        PHHAL_HW_PN5190_DYNAMIC_UID_CONFG_ADDR,
        baReadEepromConfig,
        1U
        );
    CHECK_SUCCESS(wStatus);

    /* Verify EEPROM data and perform Dynamic UID configuration if EEPROM data is not correct. */
    if (baReadEepromConfig[0] != baDynamicUidConfig[0])
    {
        /* Configure Dynamic UID */
        wStatus = phhalHw_Pn5190_Instr_WriteE2Prom(
            pHal,
            PHHAL_HW_PN5190_DYNAMIC_UID_CONFG_ADDR,
            baDynamicUidConfig,
            1U
            );
        CHECK_SUCCESS(wStatus);
    }
#endif /* NXPBUILD__PHHAL_HW_TARGET */

    return PH_ERR_SUCCESS;
}

/* Read the data over SPI */
phStatus_t phApp_Pn5190_SPIRead(uint8_t * pRxBuffer,
        uint16_t wRxLength)
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint16_t    PH_MEMLOC_REM wResponseLen = 0U;

    /* Enable chip select connected to reader IC (NSS low). */
    phDriver_PinWrite(PHDRIVER_PIN_SSEL, PH_DRIVER_SET_LOW);

    /* Read back response */
    status =  phbalReg_Exchange(
            &sBalParams,
            PH_EXCHANGE_DEFAULT,
            NULL,
            0U,
            wRxLength,
            pRxBuffer,
            &wResponseLen);

    /* Disable chip select connected to reader IC (NSS high). */
    phDriver_PinWrite(PHDRIVER_PIN_SSEL, PH_DRIVER_SET_HIGH);

    if(wResponseLen != wRxLength)
    {
        status = PH_ERR_INTERNAL_ERROR;
    }

#ifdef DEBUG_PRINT_RAW_DATA
    uint16_t cnt = 0;

    if((g_Raw_Data_Index + wRxLength+4) < MAX_RAW_DATA_LENGTH)
    {
        g_Raw_Data[g_Raw_Data_Index++] = 'C';
        g_Raw_Data[g_Raw_Data_Index++] = 'C';
        for(cnt=0; cnt < wRxLength; cnt++)
        {
            g_Raw_Data[g_Raw_Data_Index++] = pRxBuffer[cnt];
        }
        g_Raw_Data[g_Raw_Data_Index++] = 'D';
        g_Raw_Data[g_Raw_Data_Index++] = 'D';
    }

    uint16_t cnt = 0;
    printf("\nRX: ");
    for(cnt=0; cnt < wRxLength; cnt++)
    {
        printf("%02X ", pRxBuffer[cnt]);
    }
#endif /* DEBUG_PRINT_RAW_DATA */
    return status;
}

volatile uint8_t bSkipEventPost = 0x0U;
/* Application callback to handle Asynchronous events which cannot be processed by RdLib */
void phApp_CallBack(void)
{
    uint32_t dwEventStatusReg = 0x0U;
    uint16_t wEventLen = 0x0U;
    uint8_t  bBackUp = 0U;
    uint8_t  bSPIReadLen = 0U;

    /* Event length is of 2 byte @bIndex  2 and 3 */
    wEventLen = (uint16_t) pHal->sIrqResp.pIsrEvtBuffPtr[2];
    wEventLen <<= 8;
    wEventLen |= ((uint16_t) pHal->sIrqResp.pIsrEvtBuffPtr[3]);

    dwEventStatusReg = (uint32_t) pHal->sIrqResp.pIsrEvtBuffPtr[4];
    dwEventStatusReg |= ((uint32_t) pHal->sIrqResp.pIsrEvtBuffPtr[5]) << 8U ;
    dwEventStatusReg |= ((uint32_t) pHal->sIrqResp.pIsrEvtBuffPtr[6]) << 16U;
    dwEventStatusReg |= ((uint32_t) pHal->sIrqResp.pIsrEvtBuffPtr[7]) << 24U;

    if ((dwEventStatusReg & PH_PN5190_EVT_TX_OVERCURRENT_ERROR) != 0U)
    {
        DEBUG_PRINTF ("\n\tReceived TX Over Current Event... \n");
        (void)phhalHw_AsyncAbort(pHal);
    }
    if ((dwEventStatusReg & PH_PN5190_EVT_CTS) != 0U)
    {
        DEBUG_PRINTF ("\n\tReceived CTS Event... \n");
        (void)phhalHw_SetConfig(pHal, PHHAL_HW_PN5190_CONFIG_CTS_EVENT_STATUS, PH_ON);
    }
    if ((dwEventStatusReg & PH_PN5190_EVT_RFON_DETECT) != 0U)
    {
        (void)phhalHw_SetConfig(pHal, PHHAL_HW_PN5190_CONFIG_RF_ON_EVENT_STATUS, PH_ON);
        bSkipEventPost = 0x1U;

        bSPIReadLen = (pHal->sIrqResp.bIsrBytesRead -
                (PN5190_DIRECTION_BYTE_LEN + PN5190_TYPE_FIELD_LEN + PN5190_LENGTH_FIELD_LEN));

        if (wEventLen > bSPIReadLen)
        {
            bBackUp = pHal->sIrqResp.pIsrEvtBuffPtr[wEventLen - 1];

            phApp_Pn5190_SPIRead(&pHal->sIrqResp.pIsrEvtBuffPtr[wEventLen - 1],
                    (uint16_t) (wEventLen - bSPIReadLen + PN5190_DIRECTION_BYTE_LEN));

            pHal->sIrqResp.pIsrEvtBuffPtr[wEventLen - 1] = bBackUp;
        }
    }
    if ((dwEventStatusReg & (0xFFFFFFFF << 0x0C)) != 0U)
    {
        DEBUG_PRINTF ("\n\tReceived Unexpected Event that should not be reported by PN5190... \n");
    }

}

/* Configure LPCD (for PN5190) */
phStatus_t phApp_ConfigureLPCD(void)
{
    phStatus_t status = PH_ERR_SUCCESS;

    status = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS, 100U);
    CHECK_SUCCESS(status);

    status = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_MODE, PHHAL_HW_PN5190_LPCD_MODE_DEFAULT);
    CHECK_SUCCESS(status);

    status = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_CONFIG, PHHAL_HW_PN5190_LPCD_CTRL_LPCD_CALIB);
    CHECK_SUCCESS(status);

    status = phhalHw_Lpcd(pHal);
    CHECK_SUCCESS(status);

    status = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS, 330U);
    CHECK_SUCCESS(status);

    status = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_MODE, PHHAL_HW_PN5190_LPCD_MODE_POWERDOWN);
    CHECK_SUCCESS(status);

    status = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_CONFIG, PHHAL_HW_PN5190_LPCD_CTRL_LPCD);
    CHECK_SUCCESS(status);

    return status;
}

void CLIF_IRQHandler(void)
{
    /* Read the interrupt status of external interrupt attached to the reader IC IRQ pin */
#ifndef PH_OSAL_LINUX
    if (phDriver_PinRead(PHDRIVER_PIN_IRQ, PH_DRIVER_PINFUNC_INTERRUPT))
#endif /* PH_OSAL_LINUX */
    {
        phDriver_PinClearIntStatus(PHDRIVER_PIN_IRQ);

        /* Call application registered callback. */
        if ((pHal->wId == (PH_COMP_HAL | PHHAL_HW_PN5190_ID)) && (pHal->pRFISRCallback != NULL))
        {
            /* Read the data over SPI */
            phApp_Pn5190_SPIRead(pHal->sIrqResp.pHandlerModeBuffPtr,
                    (uint16_t) pHal->sIrqResp.bIsrBytesRead);

            /* Handle events triggered by PN5190 for previous command execution */
            if( pHal->sIrqResp.pHandlerModeBuffPtr[1] == PH_PN5190_EVT_RSP )
            {
                pHal->sIrqResp.pIsrEvtBuffPtr = pHal->sIrqResp.pHandlerModeBuffPtr;

                /* Call application registered callback to handle PN5190 Asynchronous events  */
                phApp_CallBack();

                if (bSkipEventPost == 0x0U)
                {
                    /* Call application RF callback to handle PN5190 synchronous events  */
                    pHal->pRFISRCallback(pHal);
                }
                bSkipEventPost = 0x0U;
            }
            else
            {
                pHal->sIrqResp.pIsrBuffPtr = pHal->sIrqResp.pHandlerModeBuffPtr;

                pHal->pRFISRCallback(pHal);
            }

            if (pHal->sIrqResp.pHandlerModeBuffPtr == &pHal->sIrqResp.aISRReadBuf[0])
            {
                pHal->sIrqResp.pHandlerModeBuffPtr = &pHal->sIrqResp.aISRReadBuf2[0];
            }
            else
            {
                pHal->sIrqResp.pHandlerModeBuffPtr = &pHal->sIrqResp.aISRReadBuf[0];
            }
        }
    }
}
#endif /* NXPBUILD__PHHAL_HW_PN5190 */

/******************************************************************************
**                            End Of File
******************************************************************************/
