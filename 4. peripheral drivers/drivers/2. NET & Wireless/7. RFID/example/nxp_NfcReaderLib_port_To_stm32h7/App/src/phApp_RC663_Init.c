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

#ifdef NXPBUILD__PHHAL_HW_RC663

#include "BoardSelection.h"
/*******************************************************************************
**   Function Declarations
*******************************************************************************/

#ifdef PH_PLATFORM_HAS_ICFRONTEND
void CLIF_IRQHandler(void);
#endif

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phbalReg_Type_t sBalParams;
phhalHw_Rc663_DataParams_t *pHal;
phpalI14443p3a_Sw_DataParams_t *pI14443a;
phpalI14443p3b_Sw_DataParams_t *pI14443b;

phhalHw_Rc663_DataParams_t *pHal;

/*******************************************************************************
**   Function Definitions
*******************************************************************************/

/* Configure LPCD (for CLRC663) */
phStatus_t phApp_ConfigureLPCD(void) {
    phStatus_t status;
    uint8_t bValueI;
    uint8_t bValueQ;
    
    /* Configure LPCD calibration standby time (T4) and RF ON time (T3). */
    status = phhalHw_Rc663_Cmd_Lpcd_SetConfig(
        pHal,
        PHHAL_HW_RC663_CMD_LPCD_MODE_DEFAULT,
        0,
        0,
        PH_NFCRDLIB_EXAMPLE_LPCD_GUARDTIME,
        PH_NFCRDLIB_EXAMPLE_LPCD_RFON_TIME);
    
    /* Perform LPCD calibration to get I and Q values. */
    status = phhalHw_Rc663_Cmd_Lpcd_GetConfig(pHal, &bValueI, &bValueQ);
    CHECK_SUCCESS(status);
    
    /* Configure I and Q values for LPCD detection cycle. */
    status = phhalHw_Rc663_Cmd_Lpcd_SetConfig(
        pHal,
        PHHAL_HW_RC663_CMD_LPCD_MODE_POWERDOWN,
        bValueI,
        bValueQ,
        PH_NFCRDLIB_EXAMPLE_LPCD_GUARDTIME,
        PH_NFCRDLIB_EXAMPLE_LPCD_RFON_TIME);
    
    return status;
}

void CLIF_IRQHandler(void) {
    /* Read the interrupt status of external interrupt attached to the reader IC IRQ pin */
    if (phDriver_PinRead(PHDRIVER_PIN_IRQ, PH_DRIVER_PINFUNC_INTERRUPT)) {
        phDriver_PinClearIntStatus(PHDRIVER_PIN_IRQ);
        
        /* Call application registered callback. */
        if (pHal->pRFISRCallback != NULL) {
            pHal->pRFISRCallback(pHal);
        }
    }
}
#endif /* NXPBUILD__PHHAL_HW_RC663 */

/******************************************************************************
**                            End Of File
******************************************************************************/
