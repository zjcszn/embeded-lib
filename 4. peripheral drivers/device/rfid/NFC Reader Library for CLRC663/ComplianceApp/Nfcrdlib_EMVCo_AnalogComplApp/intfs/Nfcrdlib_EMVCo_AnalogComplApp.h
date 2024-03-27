/*----------------------------------------------------------------------------*/
/* Copyright 2016-2020,2023 NXP                                               */
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
* Example Source Header for Nfcrdlib_EMVCo_AnalogComplApp.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#ifndef INTFS_NFCRDLIB_EMVCO_ANALOGCOMPLAPP_H_
#define INTFS_NFCRDLIB_EMVCO_ANALOGCOMPLAPP_H_
#include <ph_Status.h>

#define PRETTY_PRINTING                                         /**< Enable pretty printing */
#define MIN_VALID_DATA_SIZE                     6
#define PHAC_EMVCO_MAX_BUFFSIZE               600               /**< Maximum buffer size for Emvco. */

typedef enum{
    eEmdRes_EOT = 0x70,
    eEmdRes_EOT_AnalogTests = 0x72,
    eEmdRes_SW_0 = 0x90,
    eEmdRes_SW_1 = 0x00,
}eEmvcoRespByte;

#ifdef PH_OSAL_FREERTOS
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        #define EMVCO_ANALOG_TASK_STACK              (2200/4)
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #if  defined( __PN74XXXX__) || defined(__PN76XX__)
            #define EMVCO_ANALOG_TASK_STACK          (2000/4)
        #else /*  __PN74XXXX__*/
            #define EMVCO_ANALOG_TASK_STACK          (2050)
        #endif /*  __PN74XXXX__*/
    #endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
    #define EMVCO_ANALOG_TASK_PRIO                   4
#endif /* NXPBUILD__PH_OSAL_FREERTOS */

#ifdef PH_OSAL_LINUX
#define EMVCO_ANALOG_TASK_STACK              0x20000
#define EMVCO_ANALOG_TASK_PRIO               0
#endif /* NXPBUILD__PH_OSAL_LINUX */

#include <stdio.h>

#define EMVCO_LOOPBACK_APP		1
#define EMVCO_TRANSSEND_TYPEA 	2
#define EMVCO_TRANSSEND_TYPEB 	3

#endif /* INTFS_NFCRDLIB_EMVCO_ANALOGCOMPLAPP_H_ */
