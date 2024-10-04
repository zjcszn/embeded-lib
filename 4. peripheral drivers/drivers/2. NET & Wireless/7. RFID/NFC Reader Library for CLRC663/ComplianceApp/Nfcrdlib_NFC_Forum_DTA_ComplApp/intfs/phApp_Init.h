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

/** @file
 *
 * Interface file for the DTA Test Application initialization.
 *
 * $Author: NXP $
 * $Revision: $ (v07.09.00)
 * $Date: $
 */

#ifndef PHAPP_INIT_H
#define PHAPP_INIT_H

/* Status header */
#include <ph_Status.h>

#include <ph_NxpBuild_App.h>

/* Driver Header */
#include <phDriver.h>

/* NFCLIB Header */
#include <phNfcLib.h>

/* LLCP Header */
#include "phlnLlcp.h"

/* SNEP Header */
#include "phnpSnep.h"

#ifdef PH_PLATFORM_HAS_ICFRONTEND
#include "BoardSelection.h"
#endif

/* Check for K82 controller based boards. */
#if defined(PHDRIVER_FRDM_K82FPN5180_BOARD) || defined(PHDRIVER_FRDM_K82FRC663_BOARD) || defined(PHDRIVER_K82F_PNEV5190B_BOARD)
#define PHDRIVER_KINETIS_K82
#endif

/* Check for LPC1769 controller based boards. */
#if defined(PHDRIVER_LPC1769PN5180_BOARD) || defined (PHDRIVER_LPC1769RC663_BOARD) || defined(PHDRIVER_LPC1769PN5190_BOARD)
#define PHDRIVER_LPC1769
#include "board.h"
#endif

#ifdef PHDRIVER_KINETIS_K82
#   include <fsl_debug_console.h>
#   include <stdio.h>
#   include <fsl_gpio.h>
#   include <fsl_port.h>
#endif

#if !defined(__PN76XX__)
    #ifdef DEBUG
        #ifdef PHDRIVER_KINETIS_K82
            #if SDK_DEBUGCONSOLE==1
                #define  DEBUG_PRINTF   DbgConsole_Printf
            #else
                #define  DEBUG_PRINTF(...) printf(__VA_ARGS__);
            #endif
        #else /* PHDRIVER_KINETIS_K82 */
            #include <stdio.h>
            #define  DEBUG_PRINTF(...) printf(__VA_ARGS__); fflush(stdout)
        #endif /* PHDRIVER_KINETIS_K82 */
    #else /* DEBUG */
        #define  DEBUG_PRINTF(...)
    #endif /* DEBUG */
#else /* __PN76XX__ */
    #if (SEGGER_RTT_ENABLE == 1)
        #include "SEGGER_RTT.h"
        #define DEBUG_PRINTF(...) SEGGER_RTT_printf(0, __VA_ARGS__);
    #else
        #if DEBUG
            #include <fsl_debug_console.h>
            #define DEBUG_PRINTF(...) PRINTF(__VA_ARGS__);
        #else
            #define DEBUG_PRINTF(...)
        #endif /* DEBUG */
    #endif /* (SEGGER_RTT_ENABLE == 1) */
#endif /* __PN76XX__ */
/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
#ifdef PH_PLATFORM_HAS_ICFRONTEND
extern phbalReg_Type_t               sBalParams;             /* BAL component */
#endif /* PH_PLATFORM_HAS_ICFRONTEND */

#ifdef NXPBUILD__PHHAL_HW_PN5180
extern phhalHw_Pn5180_DataParams_t   * pHal;                 /* HAL generic pointer */
#endif /* NXPBUILD__PHHAL_HW_PN5180 */

#ifdef NXPBUILD__PHHAL_HW_PN5190
extern phhalHw_Pn5190_DataParams_t   * pHal;                 /* HAL generic pointer */
#endif /* NXPBUILD__PHHAL_HW_PN5190 */

#ifdef NXPBUILD__PHHAL_HW_RC663
extern phhalHw_Rc663_DataParams_t    * pHal;                 /* HAL generic pointer */
#endif /* NXPBUILD__PHHAL_HW_RC663 */

#ifdef NXPBUILD__PHHAL_HW_PN7462AU
extern phhalHw_PN7462AU_DataParams_t * pHal;                 /* HAL generic pointer */
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

#ifdef NXPBUILD__PHHAL_HW_PN76XX
extern phhalHw_Pn76xx_DataParams_t   * pHal;                 /* HAL generic pointer */
#endif /* NXPBUILD__PHHAL_HW_PN76XX */

#ifdef NXPBUILD__PHPAL_I14443P3A_SW
extern phpalI14443p3a_Sw_DataParams_t * ppalI14443p3a;      /* PAL ISO I14443-A component */
#endif /* NXPBUILD__PHPAL_I14443P3A_SW */

#ifdef NXPBUILD__PHPAL_I14443P3B_SW
extern phpalI14443p3b_Sw_DataParams_t * ppalI14443p3b;      /* PAL ISO I14443-B component */
#endif /* NXPBUILD__PHPAL_I14443P3B_SW */

#ifdef NXPBUILD__PHPAL_I14443P4_SW
extern phpalI14443p4_Sw_DataParams_t * ppalI14443p4;      /* PAL ISO I14443-4 component */
#endif /* NXPBUILD__PHPAL_I14443P4_SW */

#ifdef NXPBUILD__PHPAL_I14443P4A_SW
extern phpalI14443p4a_Sw_DataParams_t * ppalI14443p4a;      /* PAL ISO I14443-4A component */
#endif /* NXPBUILD__PHPAL_I14443P4A_SW */

#ifdef NXPBUILD__PHPAL_FELICA_SW
extern phpalFelica_Sw_DataParams_t * ppalFelica;         /* PAL Felica component */
#endif /* NXPBUILD__PHPAL_FELICA_SW */

#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
extern phpalI14443p4mC_Sw_DataParams_t * ppalI14443p4mC;
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

#ifdef NXPBUILD__PHAC_DISCLOOP_SW
extern phacDiscLoop_Sw_DataParams_t * pDiscLoop;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */

#ifdef NXPBUILD__PHAL_TOP_SW
extern phalTop_Sw_DataParams_t * palTop;
#endif /* NXPBUILD__PHAL_TOP_SW */

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
#ifdef NXPBUILD__PHHAL_HW_TARGET
extern phStatus_t phApp_HALConfigAutoColl(void);
#endif /* NXPBUILD__PHHAL_HW_TARGET */
extern void phApp_PrintTagInfo(phacDiscLoop_Sw_DataParams_t *pDataParams, uint16_t wNumberOfTags, uint16_t wTagsDetected);
extern phStatus_t phApp_Clif_Init( uint8_t *pTxBuffer, uint16_t wTxBufSize, uint8_t *pRxBuffer, uint16_t wRxBufSize);
extern void phApp_PrintTech(uint8_t TechType);
extern void phApp_Print_Buff(uint8_t *pBuff, uint8_t num);
extern void PrintErrorInfo(phStatus_t wStatus);
extern phStatus_t phApp_Configure_IRQ();
extern void phApp_Get_HAL_Callback(void * pHAL);
#endif /* PHAPP_INIT_H */
