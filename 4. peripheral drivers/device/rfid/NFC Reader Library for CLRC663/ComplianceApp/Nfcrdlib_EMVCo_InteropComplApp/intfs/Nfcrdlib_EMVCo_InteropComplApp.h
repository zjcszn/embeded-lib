/*
*                     Copyright 2017-2020, NXP
*
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

/** \file
* Example Source Header for Nfcrdlib_EMVCo_InteropComplApp.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#ifndef INTFS_NFCRDLIB_EMVCO_INTEROPCOMPLAPP_H_
#define INTFS_NFCRDLIB_EMVCO_INTEROPCOMPLAPP_H_
#include <ph_Status.h>

#define MIN_VALID_DATA_SIZE                     6
#define PHAC_EMVCO_MAX_BUFFSIZE               600               /**< Maximum buffer size for Emvco. */

typedef enum{
    eEmdRes_EOT = 0x70,
    eEmdRes_SW_0 = 0x90,
    eEmdRes_SW_1 = 0x00,
}eEmvcoRespByte;

#ifdef PH_OSAL_FREERTOS
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        #define EMVCO_INTEROP_TASK_STACK              (2200/4)
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #if  defined( __PN74XXXX__) || defined(__PN76XX__)
            #define EMVCO_INTEROP_TASK_STACK          (2000/4)
        #else /*  __PN74XXXX__*/
            #define EMVCO_INTEROP_TASK_STACK          (2050)
        #endif /*  __PN74XXXX__ OR __PN76XX__ */
    #endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
    #define EMVCO_INTEROP_TASK_PRIO                   4
#endif /* PH_OSAL_FREERTOS */

#ifdef PH_OSAL_LINUX
#define EMVCO_INTEROP_TASK_STACK              0x20000
#define EMVCO_INTEROP_TASK_PRIO               0
#endif /* PH_OSAL_LINUX */



#endif /* INTFS_NFCRDLIB_EMVCO_INTEROPCOMPLAPP_H_ */
