/*----------------------------------------------------------------------------*/
/* Copyright 2016-2021 NXP                                                    */
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
* Example Source Header for NfcrdlibEx1_DiscoveryLoop application.
*
* $Author: NXP $
* $Revision: $ (v07.09.00)
* $Date: $
*/


#ifndef NFCRDLIBEX1_DISCOVERYLOOP_H
#define NFCRDLIBEX1_DISCOVERYLOOP_H

#include <ph_Status.h>

#if defined (NXPBUILD__PHHAL_HW_PN5180)   || \
    defined (NXPBUILD__PHHAL_HW_PN5190)   || \
    defined (NXPBUILD__PHHAL_HW_RC663)    || \
    defined (NXPBUILD__PHHAL_HW_PN7462AU) || \
    defined (NXPBUILD__PHHAL_HW_PN7642)
        #define PH_EXAMPLE1_LPCD_ENABLE             /* If LPCD needs to be configured and used over HAL or over DiscLoop */
#endif

#define LISTEN_PHASE_TIME_MS              300       /* Listen Phase TIME */

/* Enables configuring of Discovery loop */
#define ENABLE_DISC_CONFIG

/* Enable EMVCO profile in discovery loop i.e. discovery loop will be configured to work in EMVCo mode
 * EMVCO is one of the profile in which Discovery loop can be configured to work. By default Discovery Loop will
 * work as per NFC Forum Activity Specification v2.2. which will configure the Reader in both POLL and LISTEN
 * (only for Universal device) modes of discovery loop.
 * So, to enable EMVCO profile, configurability of Discovery loop should be ENABLED by enabling
 * ENABLE_EMVCO_PROF macro.
*/
#ifdef ENABLE_DISC_CONFIG
//    #define ENABLE_EMVCO_PROF
#endif /* ENABLE_DISC_CONFIG */


#ifdef PH_OSAL_FREERTOS
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        #define DISC_DEMO_TASK_STACK         (1800/4)
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #if defined( __PN74XXXX__) || defined(__PN76XX__)
            #define DISC_DEMO_TASK_STACK     (1600/4)
        #else /* defined( __PN74XXXX__) || defined(__PN76XX__) */
            #define DISC_DEMO_TASK_STACK     (1650)
        #endif /* defined( __PN74XXXX__) || defined(__PN76XX__) */
    #endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
    #define DISC_DEMO_TASK_PRIO              4
#endif /* PH_OSAL_FREERTOS */

#ifdef PH_OSAL_LINUX
#define DISC_DEMO_TASK_STACK                0x20000
#define DISC_DEMO_TASK_PRIO                 0
#endif /* PH_OSAL_LINUX */

#endif /* NFCRDLIBEX1_DISCOVERYLOOP_H */
