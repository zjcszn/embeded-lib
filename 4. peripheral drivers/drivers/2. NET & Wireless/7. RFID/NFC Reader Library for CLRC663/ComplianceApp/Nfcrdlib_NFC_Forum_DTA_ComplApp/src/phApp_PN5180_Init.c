/*----------------------------------------------------------------------------*/
/* Copyright 2020,2021,2023 NXP                                               */
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

#include "phApp_Init.h"

#ifdef NXPBUILD__PHHAL_HW_PN5180

/* HAL specific headers */
#include <phhalHw_Pn5180_Instr.h>

/*******************************************************************************
**   Function Declarations
*******************************************************************************/

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phbalReg_Type_t                 sBalParams;
phhalHw_Pn5180_DataParams_t   * pHal;

#ifdef NXPBUILD__PHHAL_HW_TARGET
/* Parameters for L3 activation during Autocoll */
extern const uint8_t aSensRes[];
extern const uint8_t aNfcId1[];
extern const uint8_t bSelRes;
extern const uint8_t nfc_id3;
extern const uint8_t poll_res[];
#endif /* NXPBUILD__PHHAL_HW_TARGET */

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
    uint8_t baDynamicUidConfig[1U];
    uint8_t baReadEepromConfig[24U] = { 0U };

    /* Read Set Listen Parameters data from EEPROM */
    wStatus = phhalHw_Pn5180_Instr_ReadE2Prom(
        pHal,
        PHHAL_HW_PN5180_SET_LISTEN_E2PROM_ADDR,
        baReadEepromConfig,
        24U
        );
    if(wStatus != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphApp_HALConfigAutoColl: ReadE2Prom Failed!");
        return wStatus;
    }

    /* Verify EEPROM data and configure Set Listen Parameters if EEPROM data is not correct. */
    if ((memcmp(&baReadEepromConfig[0U], aSensRes, 2U) != 0x00)  ||
        (memcmp(&baReadEepromConfig[2U], aNfcId1, 3U) != 0x00)   ||
        (memcmp(&baReadEepromConfig[5U], &bSelRes, 1U) != 0x00)  ||
        (memcmp(&baReadEepromConfig[6U], poll_res, 18U) != 0x00))
    {
        /* Configure Set Listen Parameters. */
        wStatus = phhalHw_Pn5180_SetListenParameters(
            pHal,
            (uint8_t *)&aSensRes[0],
            (uint8_t *)&aNfcId1[0],
            (uint8_t)bSelRes,
            (uint8_t *)&poll_res[0],
            (uint8_t)nfc_id3
            );
        if(wStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphApp_HALConfigAutoColl: SetListenParameters Failed!");
            return wStatus;
        }
    }

    /* Read Dynamic UID configuration from EEPROM */
    wStatus = phhalHw_Pn5180_Instr_ReadE2Prom(
        pHal,
        PHHAL_HW_PN5180_DYN_UID_CFG_E2PROM_ADDR,
        baDynamicUidConfig,
        0x01U
        );
    if(wStatus != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphApp_HALConfigAutoColl: ReadE2Prom Failed!");
        return wStatus;
    }

    if(baDynamicUidConfig[0] != 0U)
    {
        baDynamicUidConfig[0] = 0U; /* Disable Dynamic UID */
        wStatus = phhalHw_Pn5180_Instr_WriteE2Prom(
            pHal,
            PHHAL_HW_PN5180_DYN_UID_CFG_E2PROM_ADDR,
            baDynamicUidConfig,
            0x01U
            );
        if(wStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphApp_HALConfigAutoColl: WriteE2Prom Failed!");
            return wStatus;
        }
    }
#endif /* NXPBUILD__PHHAL_HW_TARGET */
    return PH_ERR_SUCCESS;
}

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

/******************************************************************************
**                            End Of File
******************************************************************************/
