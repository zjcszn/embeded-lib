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

#ifdef NXPBUILD__PHHAL_HW_PN7462AU
#include <phApp_Init.h>
#include <phFlashBoot.h>
#include <phhalTimer.h>
#include <PN7462AU_pcr.h>
#include <ph_Reg.h>

/* This macro will enable Calibration before every LPCD Detection Cycle. */
#define NXPBUILD__PHHAL_HW_PN7462AU_LPCD_CALIBRATE_ONCE

#ifdef NXPBUILD__PHHAL_HW_PN7462AU

/*******************************************************************************
**   Function Declarations
*******************************************************************************/
void phApp_CPU_Init(void);

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phhalHw_PN7462AU_DataParams_t * pHal;

/*******************************************************************************
**   Initialize PN74XX NFC Controller
*******************************************************************************/
/**
* This function will initialize NXP NFC Controller.
*/
void phApp_CPU_Init(void)
{
    phFlashBoot_Main();

    phhalTimer_Init();
}

#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

/*******************************************************************************
**   Global Defines
*******************************************************************************/

/*******************************************************************************
**   Static Defines
*******************************************************************************/

/*******************************************************************************
**   Function Definitions
*******************************************************************************/
phStatus_t Configure_LPCD(uint32_t *pdwLPCDCalibrateOption, uint32_t *pdwLPCDDemoOption)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;
    uint8_t    bLPCDThreshold = 10U;
    uint32_t   dwScanOption = 1;
#ifdef DEBUG

    /* Prompt user to either select LPCD Calibration once or always before LPCD Detection Cycle. */
    DEBUG_PRINTF("\nChoose below LPCD Demo Option ");
    DEBUG_PRINTF("\nLPCD with self calibration always => 1 ");
    DEBUG_PRINTF("\nLPCD with self calibration once   => 2 ");
    DEBUG_PRINTF("\nEnter LPCD Demo Option : ");
    DEBUG_SCAN("%X",&dwScanOption);

#else

#ifndef NXPBUILD__PHHAL_HW_PN7462AU_LPCD_CALIBRATE_ONCE
    /* Calibration before every LPCD Cycle */
    dwScanOption = 0x01U;
#else
    /* Calibration before every LPCD Cycle */
    dwScanOption = 0x02U;
#endif /* NXPBUILD__PHHAL_HW_PN7462AU_LPCD_CALIBRATE_ONCE */

#endif /* DEBUG */

    /* Configure the required Threshold required based on the changes in environment.
     * Lesser the Threshold, better the sensitivity.
     * But Higher the value results in less false detections. */
    wStatus = phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_PN7462AU_LPCD_THRESHOLD, bLPCDThreshold);
    CHECK_SUCCESS(wStatus);

    *pdwLPCDCalibrateOption = dwScanOption;
    *pdwLPCDDemoOption = 0;

    return PH_ERR_SUCCESS;
}

/* Perform LPCD Calibration and get LPCD reference value */
phStatus_t Calibrate_LPCD(uint32_t dwLPCDDemo, uint32_t *pdwLPCDRefValue)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;
    uint32_t dwGetLpcdRef = 0;
    uint32_t dwLPCDRefValue;
    uint16_t wLpcdValue;

    /* Provide LPCD Reference Value of 0 to LPCD API to perform LPCD Calibration
     * and get the LPCD Calibration Value. */
    dwLPCDRefValue = 0;
    wStatus = phhalRf_LPCD(0xFFFFU, dwLPCDRefValue, &dwGetLpcdRef);
    CHECK_SUCCESS(wStatus);

    wLpcdValue = (uint16_t)dwGetLpcdRef;
    if (wLpcdValue != 0x00)
    {
        /* Configure the LPCD Calibration value as a Reference value to perform LPCD Detection cycle. */
        wStatus = phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_PN7462AU_LPCD_REF_VALUE, wLpcdValue);
        CHECK_SUCCESS(wStatus);
    }

    DEBUG_PRINTF("\nLPCD Calibration Successful. ");

    return wStatus;
}

/**
* With previous LPCD Calibration value, perform LPCD Detection cycle.
*/
phStatus_t DemoLPCD(uint32_t dwLPCDDemo, uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;

    do
    {
        /* Configure the required Wait time before performing LPCD detection cycle once again
         * based on the power saving required by the User. */
        (void)phhalHw_Wait(pHal, PHHAL_HW_TIME_MILLISECONDS, wLPCDWakeUpTime);

        /* Perform LPCD Detection Cycle.
         * Unlike other Reader HAL's, PN7462AU will return immediately if the LPCD Detection cycle
         * is successful or not. In case the API returns an ERROR, we need to call the API once
         * again, until the API return success indicating Card Detection. */
        wStatus = phhalHw_Lpcd(pHal);
    }while(wStatus != PH_ERR_SUCCESS);

    DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!LPCD Exited due to Load Change.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");

    return wStatus;
}


#endif /* NXPBUILD__PHHAL_HW_PN7462AU */
