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

#ifdef NXPBUILD__PHHAL_HW_PN7462AU

#include <phOsal.h>

#include <phFlashBoot.h>
#include <phhalTimer.h>

/*******************************************************************************
**   Function Declarations
*******************************************************************************/
void phApp_CPU_Init(void);

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phhalHw_PN7462AU_DataParams_t * pHal;

#ifdef NXPBUILD__PHHAL_HW_TARGET
/* Parameters for L3 activation during Autocoll */
extern uint8_t  sens_res[2]    ;
extern uint8_t  nfc_id1[3]     ;
extern uint8_t  sel_res        ;
extern uint8_t  nfc_id3        ;
extern uint8_t  poll_res[18]   ;
#endif /* NXPBUILD__PHHAL_HW_TARGET */

/*******************************************************************************
**   Function Definitions
*******************************************************************************/
/**
* This function will initialize NXP NFC Controller.
*/
void phApp_CPU_Init(void)
{
    phFlashBoot_Main();

    phhalTimer_Init();
}

/**
* This function will initialize Hal Target Config
*/
phStatus_t phApp_HALConfigAutoColl(void)
{
#ifdef NXPBUILD__PHHAL_HW_TARGET
    phStatus_t wStatus;
    uint8_t bSystemCode[] = {0xFF, 0xFF};

    /* Set Listen Parameters in HAL Buffer used during Autocoll */
    wStatus = phhalHw_PN7462AU_SetListenParam(
       pHal,
       &sens_res[0],
       &nfc_id1[0],
       sel_res,
       (poll_res[0] == 0x02 ? PH_ON : PH_OFF),
       &poll_res[2],
       &poll_res[8],
       bSystemCode);
    CHECK_SUCCESS(wStatus);
#endif /* NXPBUILD__PHHAL_HW_TARGET */

    return PH_ERR_SUCCESS;
}

/* Configure LPCD (for PN7462AU) */
phStatus_t phApp_ConfigureLPCD(void)
{
    phStatus_t status;
    uint16_t wLpcdValue;

    wLpcdValue = 0;

    status = phhalHw_GetConfig(pHal, PHHAL_HW_CONFIG_PN7462AU_LPCD_NEW_VALUE, &wLpcdValue);
    CHECK_SUCCESS(status);

    if (wLpcdValue != 0x00)
    {
        status = phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_PN7462AU_LPCD_REF_VALUE, wLpcdValue);
        CHECK_SUCCESS(status);
    }

    return status;
}
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

/******************************************************************************
**                            End Of File
******************************************************************************/
