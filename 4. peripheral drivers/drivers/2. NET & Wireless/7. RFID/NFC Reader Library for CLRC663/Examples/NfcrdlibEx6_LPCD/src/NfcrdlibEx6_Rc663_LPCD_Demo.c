/*----------------------------------------------------------------------------*/
/* Copyright 2020 - 2022 NXP                                                  */
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
* This application will configure Reader Library to provide demo on LPCD functionality of RC663 Reader IC's.
* Please refer Readme.txt file  for  Hardware Pin Configuration, Software Configuration and steps to build and
* execute the project which is present in the same project directory.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <NfcrdlibEx6_LPCD.h>

#ifdef NXPBUILD__PHHAL_HW_RC663
#include <phApp_Init.h>
#include "BoardSelection.h"

#define NXPBUILD__PHHAL_HW_RC663_LPCD_CALIBRATE_ONCE
#define NXPBUILD__PHHAL_HW_RC663_LPCD_ENABLE_USER_OPTION


/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phbalReg_Type_t                 sBalParams;
phhalHw_Rc663_DataParams_t    * pHal;

/*******************************************************************************
**   Global Defines
*******************************************************************************/
#define PHHAL_HW_CLRC663PLUS_VERSION        0x1AU   /**< Version of RC66303 (RC663 Plus). Refer VERSION Register (Address : 0x7F) for more information. */

/*******************************************************************************
**   Function Declarations
*******************************************************************************/
void CLIF_IRQHandler(void);

/*******************************************************************************
**   Function Definitions
*******************************************************************************/

uint32_t   dwLPCDCalibrateOption = 1;
uint32_t   dwLPCDDemoOption = 1;
uint32_t   dwLPCDRefValue;

phStatus_t Configure_LPCD(uint32_t *pdwLPCDCalibrateOption, uint32_t *pdwLPCDDemoOption)
{
#ifdef DEBUG

    uint32_t   dwScanOption = 1;

#ifdef NXPBUILD__PHHAL_HW_RC663_LPCD_ENABLE_USER_OPTION
    /* Prompt user to either select SW LPCD or ULPCD */
    DEBUG_PRINTF("\nChoose below LPCD Demo Option ");
    DEBUG_PRINTF("\nLPCD with calibration always => 1 ");
    DEBUG_PRINTF("\nLPCD with calibration once   => 2 ");
    DEBUG_PRINTF("\nEnter LPCD Demo Option : ");
    DEBUG_SCAN("%X",&dwScanOption);
#endif /* NXPBUILD__PHHAL_HW_RC663_LPCD_ENABLE_USER_OPTION */
    *pdwLPCDCalibrateOption = dwScanOption;
    *pdwLPCDDemoOption = 0;
#else /* DEBUG */

#ifdef NXPBUILD__PHHAL_HW_RC663_LPCD_CALIBRATE_ONCE
    /* Calibration Once before first LPCD Cycle */
    *pdwLPCDCalibrateOption = 0x02U;
#else
    /* Calibration before every LPCD Cycle */
    *pdwLPCDCalibrateOption = 0x01U;
#endif /* NXPBUILD__PHHAL_HW_RC663_LPCD_CALIBRATE_ONCE */

#endif /* DEBUG */

    return PH_ERR_SUCCESS;
}

/* Perform LPCD Calibration */
phStatus_t Calibrate_LPCD(uint32_t dwLPCDDemo, uint32_t *pdwLPCDRefValue)
{
    phStatus_t wStatus;
    uint8_t    bValueI = 0;
    uint8_t    bValueQ = 0;
    uint8_t    bVersion = 0;
    uint8_t    bDigitalFilter = PH_ON;
    uint16_t   wDetectionOption = PHHAL_HW_RC663_FILTER_OPTION1;
    uint16_t   wPowerDownTimeMs = LPCD_POWERDOWN_TIME;
    uint16_t   wDetectionTimeUs = LPCD_FIELD_ON_DURATION;

    /* Configure the LPCD Mode Default to perform LPCD Calibration cycle.
     * Provide Input Reference I and Q value of Zero and provide the Detection time to
     * turn ON RF for this duration to measure the Load change using I and Q Channel Values.
     * Configure suitable Power Down time to save power consumed by RC663 Reader IC. */
    wStatus = phhalHw_Rc663_Cmd_Lpcd_SetConfig(pHal, PHHAL_HW_RC663_CMD_LPCD_MODE_DEFAULT,
        bValueI, bValueQ, wPowerDownTimeMs, wDetectionTimeUs);
    CHECK_STATUS(wStatus);

    /* Read the Version register to differentiate between RC663 Old(RC66301, RC66302) and RC663 Plus (RC66303). */
    wStatus = phhalHw_Rc663_ReadRegister(pHal, PHHAL_HW_RC663_REG_VERSION, &bVersion);
    CHECK_STATUS(wStatus);
    if (bVersion == PHHAL_HW_CLRC663PLUS_VERSION)
    {
        /* Configure the Charge Pump to have high detection range.
         * Enabling the charge pump increases the power level during the detection phase increasing
         * the detection range. This leads to increase in average current consumption. */
        wStatus = phhalHw_SetConfig(pHal, PHHAL_HW_RC663_CONFIG_LPCD_CHARGEPUMP, PH_ON);
        CHECK_STATUS(wStatus);

        /* Configuring the Digital Filter triggers the RC663 Plus algorithm to get average of multiple
         * samples to detect change in I and Q values. This increases the RF ON time duration in turn
         * increasing the average current consumption. */
        wStatus = phhalHw_SetConfig(pHal, PHHAL_HW_RC663_CONFIG_LPCD_FILTER,
            ((uint16_t)(bDigitalFilter) | wDetectionOption));
        CHECK_STATUS(wStatus);
    }

    /* Perform LPCD Calibration cycle and get the LPCD reference I and Q Values. */
    wStatus = phhalHw_Rc663_Cmd_Lpcd_GetConfig(pHal, &bValueI, &bValueQ);
    CHECK_STATUS(wStatus);

    *pdwLPCDRefValue = ((bValueI) | (bValueQ << 0x8));

    DEBUG_PRINTF("\nLPCD Calibration Successful. ");

    return PH_ERR_SUCCESS;
}

/**
*/
phStatus_t DemoLPCD(uint32_t dwLPCDDemo, uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;
    uint16_t   wPowerDownTimeMs = wLPCDWakeUpTime;
    uint16_t   wDetectionTimeUs = LPCD_FIELD_ON_DURATION;
    uint8_t    bValueI, bValueQ;

    bValueI = (dwLPCDRefValue & 0xFF);
    bValueQ = ((dwLPCDRefValue >> 0x8) & 0xFF);

    /* Configure the LPCD Mode Powerdown mode to perform LPCD detection cycle.
     * Provide Input Reference I and Q value either obtained using previous LPCD Calibration phase
     * or provided by use.
     * Provide the Detection time to turn ON RF for this duration to measure the
     * Load change using I and Q Channel Values.
     * Configure suitable Power Down time to save power consumed by RC663 Reader IC. */
    wStatus = phhalHw_Rc663_Cmd_Lpcd_SetConfig(pHal, PHHAL_HW_RC663_CMD_LPCD_MODE_POWERDOWN,
        bValueI, bValueQ, wPowerDownTimeMs, wDetectionTimeUs);
    CHECK_STATUS(wStatus);

    /* Perform LPCD Detection Cycle */
    wStatus = phhalHw_Lpcd(pHal);
    if(wStatus == PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!LPCD Exited due to Load Change.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
    }
    else
    {
        DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!LPCD Exited due to issue.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
    }

    wStatus = phhalHw_Wait(pHal, PHHAL_HW_TIME_MILLISECONDS, 500);
    CHECK_STATUS(wStatus);

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
#endif /* NXPBUILD__PHHAL_HW_RC663 */
