/*----------------------------------------------------------------------------*/
/* Copyright 2020 NXP                                                         */
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
* Example Source header for NfcrdlibEx6_LPCD.
* This application will configure Reader Library to provide demo on LPCD functionality of PN5180 Reader IC's.
* Please refer Readme.txt file  for  Hardware Pin Configuration, Software Configuration and steps to build and
* execute the project which is present in the same project directory.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <NfcrdlibEx6_LPCD.h>

#ifdef NXPBUILD__PHHAL_HW_PN5180
#include <phApp_Init.h>
#include "BoardSelection.h"

/* HAL specific headers */
#include <phhalHw_Pn5180_Instr.h>
#include <phhalHw_Pn5180_Reg.h>

#define NXPBUILD__PHHAL_HW_PN5180_LPCD_SELF_CALIBRATION
#ifndef NXPBUILD__PHHAL_HW_PN5180_LPCD_SELF_CALIBRATION
    #define NXPBUILD__PHHAL_HW_PN5180_LPCD_AUTO_CALIBRATION
#endif /* NXPBUILD__PHHAL_HW_PN5180_LPCD_SELF_CALIBRATION */

/* This macro will enable Calibration before every LPCD Detection Cycle. */
#define NXPBUILD__PHHAL_HW_PN5180_LPCD_CALIBRATE_ONCE

#define PN5180_LPCD_FIELD_ON_EEPROM_VALUE                  (LPCD_FIELD_ON_DURATION - 62U) / 8U

/**
LPCD_FIELD_ON_TIME address in E2PROM to configure the FIELD ON Time during LPCD
*/
#define PHHAL_HW_PN5180_LPCD_FIELD_ON_TIME_ADDR            0x36U

/**
LPCD_THRESHOLD address in E2PROM to configure the FIELD ON Time during LPCD
*/
#define PHHAL_HW_PN5180_LPCD_THRESHOLD_ADDR                0x37U

/*******************************************************************************
**   Global Defines
*******************************************************************************/

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phbalReg_Type_t                 sBalParams;
phhalHw_Pn5180_DataParams_t   * pHal;

/*******************************************************************************
**   Static Defines
*******************************************************************************/

/*******************************************************************************
**   Function Declaration
*******************************************************************************/

/*******************************************************************************
**   Function Definitions
*******************************************************************************/
uint32_t   dwLPCDCalibrateOption = 1;

phStatus_t Configure_LPCD(uint32_t *pdwLPCDCalibrateOption, uint32_t *pdwLPCDDemoOption)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;
    uint8_t    bEPromData[5];
    uint8_t    bEPromDPCControl;
//    uint16_t   wLPCDRefVal;
    uint8_t    bLPCDFieldONTime = PN5180_LPCD_FIELD_ON_EEPROM_VALUE;
    uint8_t    bLPCDThreshold = 0x10;
    uint8_t    bLPCD_Control;
    uint32_t   dwScanOption = 1;
#ifdef DEBUG

    /* Prompt user to either select SW LPCD or ULPCD */
    DEBUG_PRINTF("\nChoose below LPCD Demo Option ");
    DEBUG_PRINTF("\nLPCD with self calibration always => 1 ");
    DEBUG_PRINTF("\nLPCD with Auto calibration        => 2 ");
    DEBUG_PRINTF("\nEnter LPCD Demo Option : ");
    DEBUG_SCAN("%X",&dwScanOption);

#else

#ifdef NXPBUILD__PHHAL_HW_PN5180_LPCD_SELF_CALIBRATION
#ifndef NXPBUILD__PHHAL_HW_PN5180_LPCD_CALIBRATE_ONCE
    /* Calibration before every LPCD Cycle */
    dwScanOption = 0x01U;
#else
    /* Calibration before every LPCD Cycle */
    dwScanOption = 0x02U;
#endif /* NXPBUILD__PHHAL_HW_PN5180_LPCD_CALIBRATE_ONCE */
#else
    /* Auto Calibration before LPCD Cycle */
    dwScanOption = 0x03U;
#endif /* NXPBUILD__PHHAL_HW_PN5180_LPCD_SELF_CALIBRATION */

#endif /* DEBUG */

    /* Configure the LPCD Mode in EEPROM based on the Auto/Self Calibration mechanism */

    /* Read LPCD Configuration settings and values from EEPROM */
    wStatus = phhalHw_Pn5180_Instr_ReadE2Prom(pHal, PHHAL_HW_PN5180_LPCD_REFERENCE_VALUE_ADDR, bEPromData,  5U);
    CHECK_SUCCESS(wStatus);

    wStatus = phhalHw_Pn5180_Instr_ReadE2Prom(pHal, PHHAL_HW_PN5180_OCPROT_CONTROL_ADDR, &bEPromDPCControl,  1U);
    CHECK_SUCCESS(wStatus);

//    wLPCDRefVal = (bEPromData[0] | (bEPromData[1] < 8));
    bLPCD_Control = bEPromData[4];

    if (dwScanOption != 2)
    {
        if ((bLPCD_Control & LPCD_REFVAL_CONTROL_MASK) != 0x01U)
        {
            bLPCD_Control = ((bLPCD_Control & (uint8_t)~LPCD_REFVAL_CONTROL_MASK) | 0x01U);
        }

        if ((bEPromDPCControl & 0x01U) != 0U)
        {
            bEPromDPCControl = (bEPromDPCControl & (uint8_t)~0x01U);
            wStatus = phhalHw_Pn5180_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5180_OCPROT_CONTROL_ADDR, &bEPromDPCControl,  1U);
            CHECK_SUCCESS(wStatus);
        }
    }
    else
    {
        if ((bLPCD_Control & LPCD_REFVAL_CONTROL_MASK) != 0x00U)
        {
            bLPCD_Control = (bLPCD_Control & (uint8_t)~LPCD_REFVAL_CONTROL_MASK);
        }
    }

    /* Configuring LPCD_GPO_REFVAL_CONTROL_ADDR to either configure Self or Auto Calibration.
     * Based on user input. */
    if (bLPCD_Control != bEPromData[4])
    {
        /* Write LPCD_REFVAL_GPO_CONTROL setting to SELF Calibration in EEPROM */
        wStatus = phhalHw_Pn5180_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5180_LPCD_GPO_REFVAL_CONTROL_ADDR, &bLPCD_Control,  1U);
        CHECK_SUCCESS(wStatus);
    }

    /* Configuring Higher Field ON time leads to higher Power consumption.
     * This values needs to configured to achieve best power requirement of User. */
    if (bLPCDFieldONTime != bEPromData[2])
    {
        /* Configure required LPCD FIELD ON Time in EEPROM used during LPCD Calibration and Detection cycle. */
        wStatus = phhalHw_Pn5180_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5180_LPCD_FIELD_ON_TIME_ADDR, &bLPCDFieldONTime,  1U);
        CHECK_SUCCESS(wStatus);
    }

    /* Configuring Higher Threshold requires higher Antenna De-tuning to exit LPCD Detection Cycle.
     * Lower the values needs to configured to achieve best power requirement of User. */
    if (bLPCDThreshold != bEPromData[3])
    {
        /* Configure required LPCD threshold in EEPROM based on the required range */
        wStatus = phhalHw_Pn5180_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5180_LPCD_THRESHOLD_ADDR, &bLPCDThreshold,  1U);
        CHECK_SUCCESS(wStatus);
    }

    dwLPCDCalibrateOption = dwScanOption;
    *pdwLPCDCalibrateOption = dwScanOption;
    *pdwLPCDDemoOption = 0;

    return PH_ERR_SUCCESS;
}

/* Perform LPCD Calibration */
phStatus_t Calibrate_LPCD(uint32_t dwLPCDDemo, uint32_t *pdwLPCDRefValue)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;
    uint32_t   dwAgcRefReg;

    if (dwLPCDCalibrateOption != 2U)
    {
        /* Read the current AGC Gear and AGC measurement */
        wStatus = phhalHw_Pn5180_Instr_ReadRegister(pHal, AGC_REF_CONFIG, &dwAgcRefReg);
        CHECK_SUCCESS(wStatus);
        /* Write back AGC Gear and AGC measurement */
        wStatus = phhalHw_Pn5180_Instr_WriteRegister(pHal, AGC_REF_CONFIG, dwAgcRefReg);
        CHECK_SUCCESS(wStatus);

        DEBUG_PRINTF("\nLPCD Calibration Successful. ");
    }
    else
    {
        DEBUG_PRINTF("\nLPCD Calibration not required as Self Calibration is been selected. ");
    }

    return wStatus;
}

/**
*/
phStatus_t DemoLPCD(uint32_t dwLPCDDemo, uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;

    wStatus = phhalHw_Pn5180_Int_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS, wLPCDWakeUpTime);
    CHECK_SUCCESS(wStatus);

    wStatus = phhalHw_Pn5180_Int_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_MODE, PHHAL_HW_PN5180_LPCD_MODE_POWERDOWN);
    CHECK_SUCCESS(wStatus);

    /* Perform LPCD Detection Cycle */
    wStatus = phhalHw_Lpcd(pHal);
    if(wStatus == PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!LPCD Exited due to Load Change.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
    }
    else
    {
        DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!LPCD Exited due to error.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
    }

    return wStatus;
}


/*******************************************************************************
**   CLIF ISR Handler
*******************************************************************************/
void CLIF_IRQHandler(void)
{
    /* Read the interrupt status of external interrupt attached to the reader IC IRQ pin */
    if (phDriver_PinRead(PHDRIVER_PIN_IRQ, PH_DRIVER_PINFUNC_INTERRUPT))
    {
        phDriver_PinClearIntStatus(PHDRIVER_PIN_IRQ);

        /* Call application registered callback. */
        if (pHal->pRFISRCallback != NULL)
        {
            pHal->pRFISRCallback(pHal);
        }
    }
}

#endif /* NXPBUILD__PHHAL_HW_PN5180 */
