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

#ifdef NXPBUILD__PHHAL_HW_PN7462AU

#include <phFlashBoot.h>
#include <phhalTimer.h>

/*******************************************************************************
**   Function Declarations
*******************************************************************************/
void phApp_CPU_Init(void);

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phhalHw_PN7462AU_DataParams_t   * pHal;

#ifdef NXPBUILD__PHHAL_HW_TARGET
/* Parameters for L3 activation during Autocoll */
extern const uint8_t aSensRes[];
extern const uint8_t aNfcId1[];
extern const uint8_t bSelRes;
extern const uint8_t gphDta_SensFRes[];
extern const uint8_t gphDta_bSystemCode[];
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

   wStatus = phhalHw_PN7462AU_SetListenParam(
       pHal,
       (uint8_t *)&aSensRes[0], /* Pointer containing the address of 2 byte sens response for ISO14443- Type A */
       (uint8_t *)&aNfcId1[0],  /* Pointer containing the address of  3 byte UID for ISO14443- Type A, first byte being 0x08 */
       (uint8_t)bSelRes,        /* response to select i.e. sak for ISO14443- Type A */
       (uint8_t)PH_OFF,         /* Support for Felica T3T or NFC DEP */
       (uint8_t *)(&gphDta_SensFRes[4]),  /* Pointer containing the address of 6 byte NFCID2 for Felica */
       (uint8_t *)(&gphDta_SensFRes[10]), /* Pointer containing the address of 8 pad Bytes for Felica */
       (uint8_t *)gphDta_bSystemCode);    /* Pointer containing the address of 2 byte system code for Felica */
   if(wStatus != PH_ERR_SUCCESS)
   {
       DEBUG_PRINTF("\nphApp_HALConfigAutoColl: SetListenParameters Failed!");
       return wStatus;
   }
#endif /* NXPBUILD__PHHAL_HW_TARGET */

    return PH_ERR_SUCCESS;
}
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

/******************************************************************************
**                            End Of File
******************************************************************************/
