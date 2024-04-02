/*----------------------------------------------------------------------------*/
/* Copyright 2016-2023 NXP                                                    */
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
* Example Source abstracting component header specific to HW used in the examples
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

#ifndef PHAPP_INIT_H
#define PHAPP_INIT_H

/* Status header */
#include <ph_Status.h>

/* Driver Header */
#include <phDriver.h>

/* NFCLIB Header */
#include <phNfcLib.h>

/* LLCP header */
#include <phlnLlcp.h>

/* SNEP header */
#include <phnpSnep.h>

#include <stdio.h>
#define DEBUG_PRINTF(...)   printf(__VA_ARGS__)
#define DEBUG_SCANF(Input, data) *data = (SEGGER_RTT_WaitKey() - 0x30); SEGGER_RTT_WaitKey()

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/

#define PH_NFCRDLIB_EXAMPLE_LPCD_GUARDTIME      100           /* LPCD Guard time(T4) in milli-seconds configured by application for Rc663. */
#define PH_NFCRDLIB_EXAMPLE_LPCD_RFON_TIME      56            /* LPCD RFON time(T3) in micro-seconds configured by application for Rc663. */

/* HAL & BAL declarations */

#ifdef PH_PLATFORM_HAS_ICFRONTEND
extern phbalReg_Type_t sBalParams;
#endif /* PH_PLATFORM_HAS_ICFRONTEND */

#ifdef NXPBUILD__PHLN_LLCP_SW
extern phlnLlcp_Sw_DataParams_t           slnLlcp;            /* LLCP component */
#endif /* NXPBUILD__PHLN_LLCP_SW */

#ifdef NXPBUILD__PHHAL_HW_RC663
extern phhalHw_Rc663_DataParams_t *pHal;
#endif /* NXPBUILD__PHHAL_HW_RC663 */

/**************************************************Prints if error is detected**************************************************************/
/* Enable(1) / Disable(0) printing error/info */
#define DETECT_ERROR 0

#if DETECT_ERROR
#define DEBUG_ERROR_PRINT(x) x
#define PRINT_INFO(...) DEBUG_PRINTF(__VA_ARGS__)
#else
#define DEBUG_ERROR_PRINT(x)
#define PRINT_INFO(...)
#endif

#define CHECK_STATUS(x)                                      \
    if ((x) != PH_ERR_SUCCESS)                               \
{                                                            \
    DEBUG_PRINTF("Line: %d   Error - (0x%04X) has occurred : 0xCCEE CC-Component ID, EE-Error code. Refer-ph_Status.h\n", __LINE__, (x));    \
}

/* prints if error is detected */
#define CHECK_SUCCESS(x)              \
    if ((x) != PH_ERR_SUCCESS)        \
{                                     \
    DEBUG_PRINTF("\nLine: %d   Error - (0x%04X) has occurred : 0xCCEE CC-Component ID, EE-Error code. Refer-ph_Status.h\n ", __LINE__, (x)); \
    return (x);                       \
}

/* prints if error is detected */
#define CHECK_NFCLIB_STATUS(x)                               \
    if ((x) != PH_NFCLIB_STATUS_SUCCESS)                     \
{                                                            \
    DEBUG_PRINTF("\nLine: %d   Error - (0x%04X) has occurred in NFCLIB\n ", __LINE__, (x)); \
}


/*********************************************************************************************************************************************/

/*******************************************************************************
**   Function Declarations
*******************************************************************************/
extern void phApp_CPU_Init(void);
extern phStatus_t phApp_Comp_Init(void *pDiscLoopParams);
extern phStatus_t phApp_HALConfigAutoColl(void);
extern phStatus_t phApp_ConfigureLPCD(void);
extern void phApp_PrintTagInfo(phacDiscLoop_Sw_DataParams_t *pDataParams, uint16_t wNumberOfTags, uint16_t wTagsDetected);
extern void phApp_PrintTech(uint8_t TechType);
extern void phApp_Print_Buff(uint8_t *pBuff, uint8_t num);
extern void PrintErrorInfo(phStatus_t wStatus);
extern phStatus_t phApp_Configure_IRQ();

#ifdef PH_PLATFORM_HAS_ICFRONTEND
extern void CLIF_IRQHandler(void);
#endif /* PH_PLATFORM_HAS_ICFRONTEND */

#endif /* PHAPP_INIT_H */
