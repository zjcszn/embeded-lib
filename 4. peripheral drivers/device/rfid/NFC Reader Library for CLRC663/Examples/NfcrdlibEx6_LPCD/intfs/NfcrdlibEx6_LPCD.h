/*----------------------------------------------------------------------------*/
/* Copyright 2021 NXP                                                         */
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
* Example Source header for NfcrdlibEx6_LPCD.
* This application will configure Reader Library to provide demo on LPCD functionality of NXP Reader IC's.
* Please refer Readme.txt file  for  Hardware Pin Configuration, Software Configuration and steps to build and
* execute the project which is present in the same project directory.
*
* $Author: NXP $
* $Revision: $ (v07.09.00)
* $Date: $
*/

#ifndef INTFS_NFCRDLIBEX6_LPCD_H_
#define INTFS_NFCRDLIBEX6_LPCD_H_

#include <ph_Status.h>

#define PRETTY_PRINTING                                         /**< Enable pretty printing */
#define MiN_VALID_DATA_SIZE          6

#ifdef PH_OSAL_FREERTOS
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        #define LPCD_TASK_STACK          (2200/4)
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #if defined(__PN74XXXX__) || defined(__PN76XX__)
            #define LPCD_TASK_STACK      (2000/4)
        #else /*  __PN74XXXX__ || __PN76XX__ */
            #define LPCD_TASK_STACK      (2050)
        #endif /*  __PN74XXXX__*/
    #endif /*  PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION*/
    #define LPCD_TASK_PRIO               4
#endif /* PH_OSAL_FREERTOS */

#ifdef PH_OSAL_LINUX
    #define LPCD_TASK_STACK              0x20000
    #define LPCD_TASK_PRIO               0
#endif /* PH_OSAL_LINUX */

/*******************************************************************************
**   Global Defines
*******************************************************************************/
/* User can use suitable PowerDown Time between successive Field ON cycle used for measurement and comparison with reference values */
#define LPCD_POWERDOWN_TIME           330U

/* User can use suitable Field ON time in micro seconds used for measurement of Load Change values.
 * NOTE : This value is suitable for all NXP NFC Reader IC's and should not be reduced to a lesser value
 * than the default value provided here, as this is leads to false LPCD detection cycles. */
#define LPCD_FIELD_ON_DURATION        102U

/*******************************************************************************
**   Function Declarations
*******************************************************************************/
phStatus_t Configure_LPCD(uint32_t *pdwLPCDCalibrateOption, uint32_t *pdwLPCDDemoOption);
phStatus_t Calibrate_LPCD(uint32_t dwLPCDDemo, uint32_t *pdwLPCDRefValue);
phStatus_t DemoLPCD(uint32_t dwLPCDDemo, uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime);

#endif /* INTFS_NFCRDLIBEX6_LPCD_H_ */
