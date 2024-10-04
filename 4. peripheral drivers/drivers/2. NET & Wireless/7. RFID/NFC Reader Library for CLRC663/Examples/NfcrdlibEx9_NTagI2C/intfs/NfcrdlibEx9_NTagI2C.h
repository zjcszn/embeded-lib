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
* Example Source Header for NTag I2C application.
*
* $Author: NXP $
* $Revision: $ (v07.09.00)
* $Date: $
*/

#ifndef INTFS_NFCRDLIBEX9_NTAGI2C_H_
#define INTFS_NFCRDLIBEX9_NTAGI2C_H_
#include <ph_Status.h>

#define DATA_BUFFER_LEN         16         /* Buffer length */

/* Tag Version info macros */
#define NTAG_I2C_1K_VERSION     0x13       /* Tag Version: 1k */
#define NTAG_I2C_2K_VERSION     0x15       /* Tag Version: 2k */

#define PTHRU_ON_OFF            (1 << 6)   /* NC_REG Pass-through bit */

#ifdef PH_OSAL_FREERTOS
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        #define NTAGI2C_TASK_STACK          (1400/4)
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #if defined( __PN74XXXX__) || defined(__PN76XX__)
            #define NTAGI2C_TASK_STACK      (1200/4)
        #else /* defined( __PN74XXXX__) || defined(__PN76XX__) */
            #define NTAGI2C_TASK_STACK      (1250)
        #endif /* defined( __PN74XXXX__) || defined(__PN76XX__) */
    #endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
    #define NTAGI2C_TASK_PRIO               4
#endif /* PH_OSAL_FREERTOS */

#ifdef PH_OSAL_LINUX
    #define NTAGI2C_TASK_STACK          0x20000
    #define NTAGI2C_TASK_PRIO           0
#endif /* PH_OSAL_LINUX */


#endif /* INTFS_NFCRDLIBEX9_NTAGI2C_H_ */
