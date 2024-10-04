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
* Example Source Header for Nfcrdlib_EMVCo_LoopBackComplApp.
*
* $Author: NXP $
* $Revision: $ (v07.09.00)
* $Date: $
*/

#ifndef INTFS_NFCRDLIB_EMVCO_LOOPBACKCOMPLAPP_H_
#define INTFS_NFCRDLIB_EMVCO_LOOPBACKCOMPLAPP_H_
#include <ph_Status.h>

#define PRETTY_PRINTING                             /**< Enable pretty printing */
#define MIN_VALID_DATA_SIZE                   6
#if defined(NXPBUILD__PHHAL_HW_RC663) || defined(NXPBUILD__PHHAL_HW_PN5190) || defined(NXPBUILD__PHHAL_HW_PN76XX)
  #define PHAC_EMVCO_MAX_BUFFSIZE             1030  /**< Maximum buffer size for EMVCo as FSD is configured to 10 in case of RC663, PN5190 and PN76XX. */
#else
  #define PHAC_EMVCO_MAX_BUFFSIZE             600   /**< Maximum buffer size for EMVCo as FSD is configured to 8 in case of PN5180 and PN7462AU. */
#endif /* NXPBUILD__PHHAL_HW_RC663 or NXPBUILD__PHHAL_HW_PN5190 or NXPBUILD__PHHAL_HW_PN76XX */

typedef enum{
    eEmdRes_EOT = 0x70,
    eEmdRes_EOT_AnalogTests = 0x72,
    eEmdRes_SW_0 = 0x90,
    eEmdRes_SW_1 = 0x00,
}eEmvcoRespByte;

#ifdef PH_OSAL_FREERTOS
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        #define EMVCO_LOOPBACK_TASK_STACK              (2200/4)
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #if  defined( __PN74XXXX__) || defined(__PN76XX__)
            #define EMVCO_LOOPBACK_TASK_STACK          (2000/4)
        #else /*  __PN74XXXX__*/
            #define EMVCO_LOOPBACK_TASK_STACK          (2050)
        #endif /*  __PN74XXXX__ OR __PN76XX__ */
    #endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
    #define EMVCO_LOOPBACK_TASK_PRIO                   4
#endif /* PH_OSAL_FREERTOS */

#ifdef PH_OSAL_LINUX
#define EMVCO_LOOPBACK_TASK_STACK              0x20000
#define EMVCO_LOOPBACK_TASK_PRIO               0
#endif /* PH_OSAL_LINUX */

#endif /* INTFS_NFCRDLIB_EMVCO_LOOPBACKCOMPLAPP_H_ */
