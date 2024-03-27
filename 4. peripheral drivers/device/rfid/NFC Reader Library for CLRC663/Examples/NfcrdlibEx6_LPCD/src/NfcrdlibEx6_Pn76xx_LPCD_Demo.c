/*----------------------------------------------------------------------------*/
/* Copyright 2022-2023 NXP                                                    */
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
* This application will configure Reader Library to provide demo on LPCD functionality of PN76XX NFC controller.
* Please refer Readme.txt file for Hardware Pin Configuration, Software Configuration and steps to build and
* execute the project which is present in the same project directory.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <NfcrdlibEx6_LPCD.h>

#ifdef NXPBUILD__PHHAL_HW_PN76XX
#include <phApp_Init.h>
#include "Clif.h"

#include "PN76xx.h"


/* Release Build Configuration to work with different Calibration options in Example */
#define NXPBUILD__PHHAL_HW_PN76XX_LPCD_CALIBRATE_ONCE

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phhalHw_Pn76xx_DataParams_t   * pHal;

uint32_t   dwLPCDCalibrateOption = 1;
uint32_t   dwLPCDDemoOption = 1;
uint32_t   dwLPCDRefValue;

/*******************************************************************************
**   Global Defines
*******************************************************************************/

/*******************************************************************************
**   Static Defines
*******************************************************************************/
static phStatus_t SW_LPCD_Calib(uint32_t * pdwRefValue);
static phStatus_t SW_LPCD_Demo(uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime);

/*******************************************************************************
**   Function Declarations
*******************************************************************************/

/*******************************************************************************
**   Function Definitions
*******************************************************************************/


/**
* This function will initialize NXP NFC Controller.
*/
void phApp_CPU_Init(void)
{
#ifdef SEGGER_RTT_ENABLE
    /* SEGGER RTT Init */
    memset((uint8_t *)PN76_SEGGER_RTT_MEMORY_ALLOCATION_ADDRESS, 0, 0x4b8);
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
#endif
   /* To enable NS RF IRQ */
   NVIC_EnableIRQ(CLIF_RF_IRQn);

   /* To enable NS TIMER IRQ */
   NVIC_EnableIRQ(TIMER_IRQn);

   /* Initialize PMU */
   PMU_Init();

   /* Initialize SMU */
   Smu_Init();

   /* Initialize CLKGEN  */
   CLKGEN_Init();
}


static phStatus_t SW_LPCD_Calib(uint32_t * pdwRefValue)
{
    phStatus_t  wStatus;

    /* Configure the Wake-up time to Turn ON the RF Field to measure reference values for SW LPCD Cycle */
    wStatus = phhalHw_Pn76xx_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS, 330U);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode to Default to perform SW LPCD Calibration Cycle */
    wStatus = phhalHw_Pn76xx_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_MODE, PHHAL_HW_PN76XX_LPCD_MODE_DEFAULT);
    CHECK_SUCCESS(wStatus);

    /* Perform LPCD Calibration Cycle */
    wStatus = phhalHw_Lpcd(pHal);
    CHECK_SUCCESS(wStatus);

    /* Get the Calibration value obtained during LPCD Calibration Cycle */
    return phhalHw_Pn76xx_LPCD_GetConfig(pHal, PHHAL_HW_CONFIG_LPCD_REF_VAL, pdwRefValue);
}

static phStatus_t SW_LPCD_Demo(uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime)
{
    phStatus_t  wStatus;

    /* Configure the Wake-up time between successive RF Field ON to measure and compare against the reference value
     * during SW LPCD Cycle. This can be fine tuned by User to obtain required Power efficiency. */
    wStatus = phhalHw_Pn76xx_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS, wLPCDWakeUpTime);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode to Power-down to perform SW LPCD Detection Cycle */
    wStatus = phhalHw_Pn76xx_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_MODE, PHHAL_HW_PN76XX_LPCD_MODE_POWERDOWN);
    CHECK_SUCCESS(wStatus);

    /* Perform LPCD Detection Cycle */
    return phhalHw_Lpcd(pHal);
}

phStatus_t Configure_LPCD(uint32_t *pdwLPCDCalibrateOption, uint32_t *pdwLPCDDemoOption)
{
#ifdef DEBUG
    uint32_t   iChoice = 0;

    /* Prompt user to either select SW LPCD or ULPCD */
    DEBUG_PRINTF("\nChoose below LPCD Demo Option ");
    DEBUG_PRINTF("\nSW LPCD with calibration always => 1 ");
    DEBUG_PRINTF("\nSW LPCD with calibration once   => 2 ");
    DEBUG_PRINTF("\nEnter LPCD Demo Option : ");
    DEBUG_SCANF("%X", &iChoice);

    *pdwLPCDCalibrateOption = (uint32_t)(iChoice & 0xF);
#else /* DEBUG */
#ifndef NXPBUILD__PHHAL_HW_PN76XX_LPCD_CALIBRATE_ONCE
    /* Calibration before every LPCD Cycle */
    *pdwLPCDCalibrateOption = 0x01U;
#else /* NXPBUILD__PHHAL_HW_PN76XX_LPCD_CALIBRATE_ONCE */
    /* Calibration Once before first LPCD Cycle */
    *pdwLPCDCalibrateOption = 0x02U;
#endif /* NXPBUILD__PHHAL_HW_PN76XX_LPCD_CALIBRATE_ONCE */
#endif /* DEBUG */

    return PH_ERR_SUCCESS;
}

/* Perform LPCD Calibration */
phStatus_t Calibrate_LPCD(uint32_t dwLPCDDemo, uint32_t *pdwLPCDRefValue)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;

    /* Software LPCD Calibration */
    wStatus = SW_LPCD_Calib(pdwLPCDRefValue);
    if(wStatus == PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nSW LPCD Calibration Successful. ");
    }
    else
    {
        DEBUG_PRINTF("\nSW LPCD Calibration Failed. ");
    }

    return wStatus;
}

/**
* With previous LPCD Calibration value, perform LPCD Detection cycle.
*/
phStatus_t DemoLPCD(uint32_t dwLPCDDemo, uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;

    /* SW LPCD Demo */
    wStatus = SW_LPCD_Demo(dwLPCDRefValue, wLPCDWakeUpTime);
    if(wStatus == PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!SW LPCD Exited due to Load Change.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
    }

    DEBUG_PRINTF("\n---------------Restart to try another option.---------------\n\n");

    return wStatus;
}

#endif /* NXPBUILD__PHHAL_HW_PN76XX */
