/*----------------------------------------------------------------------------*/
/* Copyright 2016-2021, 2023 NXP                                              */
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
* Example Source Header for NfcrdlibEx5_ISO15693 application.
*
* $Author: NXP $
* $Revision: $ (v07.09.00)
* $Date: $
*/

#ifndef INTFS_NFCRDLIBEX5_ISO15693_H_
#define INTFS_NFCRDLIBEX5_ISO15693_H_
#include <ph_Status.h>


#ifdef PH_OSAL_FREERTOS
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        #define ISO15693_DEMO_TASK_STACK        (1400/4)
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #if defined( __PN74XXXX__) || defined(__PN76XX__)
            #define ISO15693_DEMO_TASK_STACK    (1300/4)
        #else /* defined( __PN74XXXX__) || defined(__PN76XX__) */
            #define ISO15693_DEMO_TASK_STACK    (1250)
        #endif /* defined( __PN74XXXX__) || defined(__PN76XX__) */
    #endif
    #define ISO15693_DEMO_TASK_PRIO             4
#endif /* PH_OSAL_FREERTOS */

#ifdef PH_OSAL_LINUX
    #define ISO15693_DEMO_TASK_STACK    0x20000
    #define ISO15693_DEMO_TASK_PRIO     0
#endif /* PH_OSAL_LINUX */

#if defined(NXPBUILD__PHHAL_HW_PN5190) || \
    defined(NXPBUILD__PHHAL_HW_PN7462AU) || \
    defined(NXPBUILD__PHHAL_HW_PN76XX)
#define CUSTOM_HIGH_DATA_RATE_DEMO
#ifdef CUSTOM_HIGH_DATA_RATE_DEMO
    #define CUSTOM_HIGH_DATA_RATE_DEMO_TX106_RX106
    #ifndef NXPBUILD__PHHAL_HW_PN7462AU
    #ifndef CUSTOM_HIGH_DATA_RATE_DEMO_TX106_RX106
        #define CUSTOM_HIGH_DATA_RATE_DEMO_TX212_RX212
    #endif /* CUSTOM_HIGH_DATA_RATE_DEMO_TX106_RX106 */
    #endif /* NXPBUILD__PHHAL_HW_PN7462AU */
#endif /* CUSTOM_HIGH_DATA_RATE_DEMO */
#endif

#endif /* INTFS_NFCRDLIBEX5_ISO15693_H_ */
